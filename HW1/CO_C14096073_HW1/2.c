#include<stdio.h>
int main ()
{
FILE *fp;

fp = fopen("../input/2.txt", "r"); 
if (fp == NULL)
    {
        printf("File does not exists \n");
        return 0;
    }
int a[10] = {0}, b[10]= {0}, c[10] = {0};
int i, arr_size = 10;
        for(i = 0; i<arr_size; i++) fscanf(fp,"%d", &a[i]);
        for(i = 0; i<arr_size; i++) fscanf(fp,"%d", &b[i]);
        for(i = 0; i<arr_size; i++) fscanf(fp,"%d", &c[i]);
fclose(fp);       
        int *p_a = &a[0];
        int *p_b = &b[0];
        int *p_c = &c[0];

        for (int i = 0; i < arr_size; i++)
            asm volatile("div %[C], %[A], %[B]\n\t" 
                        :[C] "+r"(c[i]) 
                        :[A] "r"(a[i]) , [B] "r"(b[i])  
                        );
                p_c = &c[0];
        for (int i = 0; i < arr_size; i++)
printf("%d ", *p_c++);
return 0;
}
