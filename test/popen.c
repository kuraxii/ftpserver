#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    FILE* fp = popen("ls -l /home/zzj/desktop", "r");
    char buf[255];
    while(1)
    {
        fgets(buf, sizeof(buf), fp);
        if(feof(fp))
        {
            break;
        }

        fputs(buf, stdout);
    }




    return 0;
}