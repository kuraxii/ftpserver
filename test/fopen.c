#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    int ret = access("/home/zzj/desktop", R_OK);
    printf("ret = %d\n", ret);

    FILE* fp = fopen("/home/zzj/desktop", "r");
    printf("fp = %p\n", fp);

    

    return 0;
}