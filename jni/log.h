#include <stdio.h>
#include <time.h>

char *logfile = "/storage/emulated/0/AppProjects/clog.txt";

int mlog( const char* info,int code, void* pin )
{
    time_t nowtime=time(NULL); 
    char tmp[64];
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S",localtime(&nowtime));

   FILE * fp;
   fp = fopen ( logfile, "a+" );
   fprintf( fp, "%s  [ %p ]   %d  %s\n", tmp, pin, code, info );
   fclose( fp ); 
   return 0;
}

void clean_log()
 {
    FILE *fp;
    fp = fopen ( logfile, "w" );
    fclose( fp );
    return ;
 }
