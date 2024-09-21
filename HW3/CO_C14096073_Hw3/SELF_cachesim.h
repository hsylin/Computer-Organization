// See LICENSE for license details.

#ifndef _RISCV_CACHE_SIM_H
#define _RISCV_CACHE_SIM_H

#include "memtracer.h"
#include "common.h"
#include <cstring>
#include <string>
#include <map>
#include <unordered_map>
#include <cstdint>
using namespace std;
class lfsr_t
{
 public:
  lfsr_t() : reg(1) {}
  lfsr_t(const lfsr_t& lfsr) : reg(lfsr.reg) {}
  void new_array(size_t ways, size_t sets)
  {
  	LMU_array = new uint32_t[sets*ways](); 
  	num_array = new long unsigned int[sets]();
  	int k=0;
  	for(long unsigned int i=0;i<sets;i++)
  	{
  		num_array[i] = 0;
  		for(long unsigned int j=0;j<ways;j++)
  		{
  			LMU_array[k] = 0;
  			k++;
		}
	}
  }
  uint32_t next() { return reg = (reg>>1)^(-(reg&1) & 0xd0000001); }
  uint32_t LMU(size_t ways, size_t idx)
  {
  	uint32_t index=0;
  	max_ren++;
  	  if(num_array[idx]==ways)
  	  {
  	  	uint32_t max=LMU_array[idx*ways];
  	  	for(uint32_t i=1;i<ways;i++)
  	  	{
  	  		if(max<LMU_array[idx*ways+i])
  	  		{
  	  			index = i;
  	  			max = LMU_array[idx*ways+i];
			}
		}
		
		LMU_array[idx*ways+index] = max_ren;
	  }
	  else
	  {
	  	index = num_array[idx];
	  	LMU_array[ways*idx+index] = max_ren;
	  	num_array[idx]++;
	  }
	  return index;
  }
  uint64_t LMU_fa(size_t ways,uint64_t locate)
  {
    	max_ren++;
  	  	map<uint64_t, int>::iterator start=ma.begin();
  	  	int max=start->second;
  	  	uint64_t index=start->first;

  	  	int temp =0;
  	  	for(map<uint64_t, int>::iterator i=ma.begin();i!=ma.end();i++)
  	  	{
  	  		if(max<i->second)
  	  		{
  	  			index = i->first;
  	  			max = i->second;
  	  			temp =distance(start,i);
			}
		}
		


  		ma.erase(index);

	   return temp;

  }
  void update(size_t ways, size_t idx, size_t way)
  {
  	max_ren++;

	LMU_array[ways*idx+way] = max_ren;
	
  }
  void update(uint64_t line)
  {
  	max_ren++;

	ma[line] = max_ren;

  }
      void insert(uint64_t line)
  {
	max_ren++;
	ma[line]=max_ren;
  }
 private:
  uint32_t reg;
  int max_ren = 0;
  uint32_t* LMU_array;
  map<uint64_t, int> ma;
  long unsigned int* num_array;
  uint64_t full = 0;
};

class cache_sim_t
{
 public:
  cache_sim_t(size_t sets, size_t ways, size_t linesz, const char* name);
  cache_sim_t(const cache_sim_t& rhs);
  virtual ~cache_sim_t();

  void access(uint64_t addr, size_t bytes, bool store);
  void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval);
  void print_stats();
  void set_miss_handler(cache_sim_t* mh) { miss_handler = mh; }
  void set_log(bool _log) { log = _log; }

  static cache_sim_t* construct(const char* config, const char* name);

 protected:
  static const uint64_t VALID = 1ULL << 63;
  static const uint64_t DIRTY = 1ULL << 62;

  virtual uint64_t* check_tag(uint64_t addr);
  virtual uint64_t victimize(uint64_t addr);

  lfsr_t lfsr;
  cache_sim_t* miss_handler;

  size_t sets;
  size_t ways;
  size_t linesz;
  size_t idx_shift;

  uint64_t* tags;
  
  uint64_t read_accesses;
  uint64_t read_misses;
  uint64_t bytes_read;
  uint64_t write_accesses;
  uint64_t write_misses;
  uint64_t bytes_written;
  uint64_t writebacks;

  std::string name;
  bool log;

  void init();
};

class fa_cache_sim_t : public cache_sim_t
{
 public:
  fa_cache_sim_t(size_t ways, size_t linesz, const char* name);
  uint64_t* check_tag(uint64_t addr);
  uint64_t victimize(uint64_t addr);
 private:
  static bool cmp(uint64_t a, uint64_t b);
  std::map<uint64_t, uint64_t> tags;
};

class cache_memtracer_t : public memtracer_t
{
 public:
  cache_memtracer_t(const char* config, const char* name)
  {
    cache = cache_sim_t::construct(config, name);
  }
  ~cache_memtracer_t()
  {
    delete cache;
  }
  void set_miss_handler(cache_sim_t* mh)
  {
    cache->set_miss_handler(mh);
  }
  void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
  {
    cache->clean_invalidate(addr, bytes, clean, inval);
  }
  void set_log(bool log)
  {
    cache->set_log(log);
  }

 protected:
  cache_sim_t* cache;
};

class icache_sim_t : public cache_memtracer_t
{
 public:
  icache_sim_t(const char* config) : cache_memtracer_t(config, "I$") {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == FETCH;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == FETCH) cache->access(addr, bytes, false);
  }
};

class dcache_sim_t : public cache_memtracer_t
{
 public:
  dcache_sim_t(const char* config) : cache_memtracer_t(config, "D$") {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == LOAD || type == STORE;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == LOAD || type == STORE) cache->access(addr, bytes, type == STORE);
  }
};

#endif
