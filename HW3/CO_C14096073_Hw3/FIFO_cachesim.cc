// See LICENSE for license details.

#include "cachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>

cache_sim_t::cache_sim_t(size_t _sets, size_t _ways, size_t _linesz, const char* _name)
: sets(_sets), ways(_ways), linesz(_linesz), name(_name), log(false)
{
  init();
}

static void help()
{
  std::cerr << "Cache configurations must be of the form" << std::endl;
  std::cerr << "  sets:ways:blocksize" << std::endl;
  std::cerr << "where sets, ways, and blocksize are positive integers, with" << std::endl;
  std::cerr << "sets and blocksize both powers of two and blocksize at least 8." << std::endl;
  exit(1);
}

cache_sim_t* cache_sim_t::construct(const char* config, const char* name)
{
  const char* wp = strchr(config, ':');// seearch for : location
  if (!wp++) help();
  const char* bp = strchr(wp, ':');// ? ':'
  if (!bp++) help();

  size_t sets = atoi(std::string(config, wp).c_str());// ? std::string(config, wp)
  size_t ways = atoi(std::string(wp, bp).c_str());
  size_t linesz = atoi(bp);// block size

  if (ways > 4 /* empirical */ && sets == 1)//?
    return new fa_cache_sim_t(ways, linesz, name);//  fa_cache_sim_t(ways, linesz, name) for y associate
  return new cache_sim_t(sets, ways, linesz, name);
}

void cache_sim_t::init()
{
  if (sets == 0 || (sets & (sets-1)))//2^n 
    help();
  if (linesz < 8 || (linesz & (linesz-1)))//2^n
    help();

  idx_shift = 0;
  for (size_t x = linesz; x>1; x >>= 1)// log2(linesz) = idx_shift , space for data
    idx_shift++;

  tags = new uint64_t[sets*ways]();
  lfsr.new_array(ways,sets);
  read_accesses = 0;
  read_misses = 0;
  bytes_read = 0;
  write_accesses = 0;
  write_misses = 0;
  bytes_written = 0;
  writebacks = 0;

  miss_handler = NULL;
}

cache_sim_t::cache_sim_t(const cache_sim_t& rhs)// copy cache
 : sets(rhs.sets), ways(rhs.ways), linesz(rhs.linesz),
   idx_shift(rhs.idx_shift), name(rhs.name), log(false)
{
  tags = new uint64_t[sets*ways];
  memcpy(tags, rhs.tags, sets*ways*sizeof(uint64_t));//copy content from rhs.tags to tags
}

cache_sim_t::~cache_sim_t()
{
  print_stats();
  delete [] tags;
}

void cache_sim_t::print_stats()
{
  if (read_accesses + write_accesses == 0)//no operation
    return;

  float mr = 100.0f*(read_misses+write_misses)/(read_accesses+write_accesses);

  std::cout << std::setprecision(3) << std::fixed;
  std::cout << name << " ";
  std::cout << "Bytes Read:            " << bytes_read << std::endl;
  std::cout << name << " ";
  std::cout << "Bytes Written:         " << bytes_written << std::endl;
  std::cout << name << " ";
  std::cout << "Read Accesses:         " << read_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Accesses:        " << write_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Read Misses:           " << read_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Misses:          " << write_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Writebacks:            " << writebacks << std::endl;
  std::cout << name << " ";
  std::cout << "Miss Rate:             " << mr << '%' << std::endl;
}

uint64_t* cache_sim_t::check_tag(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);// & (sets-1) = a mask for get set (addr >> idx_shift) ,no data only tag+set_index
  size_t tag = (addr >> idx_shift) | VALID; // VALID in need to enter the tags, addr in tags must have VALID

  for (size_t i = 0; i < ways; i++)//find all way in tags
    if (tag == (tags[idx*ways + i] & ~DIRTY))//tag is not dirty
      return &tags[idx*ways + i];

  return NULL;
}

uint64_t cache_sim_t::victimize(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t way = lfsr.FIFO(ways);
  uint64_t victim = tags[idx*ways + way];// get victim
  tags[idx*ways + way] = (addr >> idx_shift) | VALID;// set tags[idx*ways + way] to (addr >> idx_shift) | VALID 
  return victim;
}

void cache_sim_t::access(uint64_t addr, size_t bytes, bool store)
{
  store ? write_accesses++ : read_accesses++;//write_accesses is written but must in cache
  (store ? bytes_written : bytes_read) += bytes;

  uint64_t* hit_way = check_tag(addr);
  if (likely(hit_way != NULL))// faster if((hit_way != NULL))
  {
    if (store)
      *hit_way |= DIRTY;
    return;
  }

  store ? write_misses++ : read_misses++;
  if (log)
  {
    std::cerr << name << " "
              << (store ? "write" : "read") << " miss 0x"
              << std::hex << addr << std::endl;
  }

  uint64_t victim = victimize(addr);

  if ((victim & (VALID | DIRTY)) == (VALID | DIRTY))// victim is dirty&&vaild needs write back
  {
    uint64_t dirty_addr = (victim & ~(VALID | DIRTY)) << idx_shift;
    if (miss_handler)//dont care
      miss_handler->access(dirty_addr, linesz, true);
    writebacks++;
  }

  if (miss_handler)//dont care
    miss_handler->access(addr & ~(linesz-1), linesz, false);

  if (store)//write allocate
    *check_tag(addr) |= DIRTY;
}

void cache_sim_t::clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
{
  uint64_t start_addr = addr & ~(linesz-1); //set data to 0
  uint64_t end_addr = (addr + bytes + linesz-1) & ~(linesz-1);
  uint64_t cur_addr = start_addr;
  while (cur_addr < end_addr) {
    uint64_t* hit_way = check_tag(cur_addr);
    if (likely(hit_way != NULL))
    {
      if (clean) {
        if (*hit_way & DIRTY) {//set dirty to 0 and writebacks++
          writebacks++;
          *hit_way &= ~DIRTY;
        }
      }

      if (inval)//set valid to 0
        *hit_way &= ~VALID;
    }
    cur_addr += linesz;
  }
  if (miss_handler)
    miss_handler->clean_invalidate(addr, bytes, clean, inval);
}

fa_cache_sim_t::fa_cache_sim_t(size_t ways, size_t linesz, const char* name)
  : cache_sim_t(1, ways, linesz, name)
{
}

uint64_t* fa_cache_sim_t::check_tag(uint64_t addr)
{

  auto it = tags.find(addr >> idx_shift);// tags(map<addr(no data) , addr(no data)|valid>) 
  return it == tags.end() ? NULL : &it->second;
}

uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  uint64_t old_tag = 0;
  uint64_t locate = addr >> idx_shift;
  if (tags.size() == ways)//if full
  {
    auto it = tags.begin();
    auto index =lfsr.FIFO_fa(ways,locate);
    std::advance(it,index); 
    old_tag = it->second;
    tags.erase(it);//and pop it
  }
  lfsr.insert(locate);
  
 
  tags[addr >> idx_shift] = (addr >> idx_shift) | VALID;
  return old_tag;
}
