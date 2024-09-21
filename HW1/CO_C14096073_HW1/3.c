#include<stdio.h>
int main()
{
FILE *fp;
fp = fopen("../input/3.txt", "r"); 
if (fp == NULL)
    {
        printf("File does not exists \n");
        return 0;
    }
 int f,i=0;
 int h[9]={0}, x[3]={0}, y[3]={0};
 for(i = 0; i<9; i++) fscanf(fp,"%d", &h[i]);
 for(i = 0; i<3; i++) fscanf(fp,"%d", &x[i]);
 for(i = 0; i<3; i++) fscanf(fp,"%d", &y[i]);
 fclose(fp); 
 int *p_x = &x[0] ;
 int *p_h = &h[0] ;
 int *p_y = &y[0] ;

 for (i = 0 ; i < 3; i++)
 {

 p_x = &x[0] ;
 for (f = 0 ; f < 3; f++)
            asm volatile("MUL %[F], %[X], %[H]\n\t" 
                        "add %[Y], %[Y], %[F]\n\t" 
                    :[Y] "+r"(y[i]) 
                    :[X] "r"(x[f]) , [H] "r"(h[(i*3)+f]) ,[F] "r"(h[0]) 
                    );
 }
 p_y = &y[0];
 for(i = 0; i<3; i++)
 printf("%d \n", *p_y++);
 return(0) ;

}
