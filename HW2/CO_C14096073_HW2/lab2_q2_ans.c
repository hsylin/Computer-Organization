	"addi x31, x0, 16\n\t" 
	"vsetvli x5, x31, e16\n\t"
	"vle16.v v0, (%[p_x])\n\t"
	"vle16.v v2, (%[p_x])\n\t"

	
	"add %[flag],x0,x0\n\t"	
	"addi x6, x0, 0\n\t" 
	"addi x30, x0, 0\n\t" 
	"addi x31,x0,8\n\t"


	
	"leftshift:\n\t"
	"vadd.vv v3, v0, v2\n\t"
	"vmseq.vx v4, v3, %[target] \n\t"
	"vfirst.m x7, v4\n\t"
	"bge x7,x0,flagisone\n\t"
	"vslidedown.vi v0,v0,1\n\t"
	"addi x6, x6,1\n\t"
	"blt x6,x31,leftshift\n\t"
	"beq x0,x0,rightshift\n\t"
	
	 "vle16.v v0, (%[p_x])\n\t"
	 
	"rightshift:\n\t"
	"vadd.vv v3, v0, v2\n\t"
	"vmseq.vx v4, v3, %[target] \n\t"
	"vfirst.m x7, v4\n\t"
	"bge x7,x0,flagisone\n\t"
	"addi %[p_x], %[p_x], -2\n\t"
	"vle16.v v0, (%[p_x])\n\t"	
	"addi x30, x30,1\n\t"	
	"blt x30,x31,rightshift\n\t"
	"beq x0,x0,exit\n\t"
	
	"flagisone:\n\t"
	"addi %[flag],x0,1\n\t"
	
	"exit:\n\t"


