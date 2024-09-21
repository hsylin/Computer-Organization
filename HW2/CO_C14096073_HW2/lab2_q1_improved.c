

    "addi x6, x0, 16\n\t" 
    "addi %[arith_cnt], %[arith_cnt], 1\n\t"
    "vsetvli x5, x6, e16\n\t"
    "add %[others_cnt], %[others_cnt], 1\n\t"
    

    "vle16.v v0, (%[h])\n\t"
    "vle16.v v1, (%[x])\n\t"
    
    
    "vadd.vv v2, v0, v1\n\t"
    "vse16.v v2, (%[y])\n\t"
    "add %[h], %[h], x6\n\t"
    "add %[x], %[x], x6\n\t"
    "add %[y], %[y], x6\n\t"
    
    "vle16.v v0, (%[h])\n\t"
    "vle16.v v1, (%[x])\n\t"
    "vadd.vv v2, v0, v1\n\t"
    "vse16.v v2, (%[y])\n\t"

    "add %[lw_cnt], %[lw_cnt], 4\n\t"
    "add %[sw_cnt], %[sw_cnt], 2\n\t"
    "add %[arith_cnt], %[arith_cnt], 5\n\t"
    
    


    

    


    
