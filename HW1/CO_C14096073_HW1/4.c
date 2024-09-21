#include<stdio.h>
int main()
{
 int i=0;
 int h[9]={0}, x[3]={0}, y[3]={0};
 FILE *fp;
fp = fopen("../input/4.txt", "r"); 
if (fp == NULL)
    {
        printf("File does not exists \n");
        return 0;
    }
 for(i = 0; i<9; i++) fscanf(fp,"%d", &h[i]);
 for(i = 0; i<3; i++) fscanf(fp,"%d", &x[i]);
 for(i = 0; i<3; i++) fscanf(fp,"%d", &y[i]);
 fclose(fp);   
 int *p_x = &x[0] ;
 int *p_h = &h[0] ;
 int *p_y = &y[0] ;
 asm volatile("MUL %[H0], %[X0], %[H0]\n\t" 
              "add %[Y0], %[Y0], %[H0]\n\t"
              "MUL %[H0], %[X1], %[H1]\n\t" 
              "add %[Y0], %[Y0], %[H0]\n\t"
              "MUL %[H0], %[X2], %[H2]\n\t" 
              "add %[Y0], %[Y0], %[H0]\n\t"
              
              "MUL %[H0], %[X0], %[H3]\n\t" 
              "add %[Y1], %[Y1], %[H0]\n\t"
              "MUL %[H0], %[X1], %[H4]\n\t" 
              "add %[Y1], %[Y1], %[H0]\n\t"
              "MUL %[H0], %[X2], %[H5]\n\t" 
              "add %[Y1], %[Y1], %[H0]\n\t"
              
              "MUL %[H0], %[X0], %[H6]\n\t" 
              "add %[Y2], %[Y2], %[H0]\n\t"
              "MUL %[H0], %[X1], %[H7]\n\t" 
              "add %[Y2], %[Y2], %[H0]\n\t"
              "MUL %[H0], %[X2], %[H8]\n\t" 
              "add %[Y2], %[Y2], %[H0]\n\t"
              
                    :[Y0] "+r"(y[0]) ,[Y1] "+r"(y[1]), [Y2] "+r"(y[2]) 
                    :[X0] "r"(x[0]),[X1] "r"(x[1]) , [X2] "r"(x[2]) ,
                    [H0] "r"(h[0]),[H1] "r"(h[1]) , [H2] "r"(h[2]) ,[H3] "r"(h[3]),[H4] "r"(h[4]) , [H5] "r"(h[5]) ,[H6] "r"(h[6]),[H7] "r"(h[7]) , [H8] "r"(h[8]) 
                    );
 p_y = &y[0];
 for(i = 0; i<3; i++)
 printf("%d \n", *p_y++);
 return(0) ;

}
