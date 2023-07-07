#include <stdio.h>

int text(const char* filename, float x[])
{
   FILE * fp;
   int n = 1024;
   char str[n];
   int j = 1;
   float *px = x;
   
   fp = fopen (filename, "r");
   
   while(!feof(fp)){
	   
	   fgets(str, n, fp);
	   printf("%s\n", str);
	   int i = 1;
	   char* pc = str;
	   
	   while((*pc)!='\0'){
		   if((*pc)=='o'){
			   *px++ = i;
			   *px++ = -j;
		   }
		   pc++;
		   i++;
	   }
	   j++;
   }
   
   fclose(fp); 
   return 0;
}

int getf(float x[], float xv, float yv){
	text("/storage/emulated/0/AppProjects/opengl/jni/hi.h", x);
	float* p = x;
	int i = 0;
	while(*p!=0){
		*p *= xv; //*p += 0.2;
		p++;
		*p *= yv; //*p += 0.2;
		p++;
		i++;
	}
	return i;
}
