#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

#define LOG_FILE "/storage/emulated/0/AppProjects/clog.txt"

int mlog( const char* info,int code, void* pin )
{
    time_t nowtime=time(NULL); 
    char tmp[64];
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S",localtime(&nowtime));

   FILE * fp;
   fp = fopen ( LOG_FILE, "a+" );
   if(!fp) return 1;
   fprintf( fp, "%s  [ %p ]   %d  %s\n", tmp, pin, code, info );
   fclose( fp ); 
   return 0;
}

int mlogI(int code)
{
   FILE * fp;
   fp = fopen ( LOG_FILE, "a+" );
   fprintf( fp, "{%d}", code);
   fclose( fp ); 
   return 0;
}

int mlogS(char* str)
{
   FILE * fp;
   fp = fopen ( LOG_FILE, "a+" );
   fprintf( fp, "%s ", str);
   fclose( fp ); 
   return 0;
}

void clean_log()
 {
    FILE *fp;
    fp = fopen ( LOG_FILE, "w" );
	 if(!fp) return ;
    fclose( fp );
    return ;
 }

 #endif LOG_H
