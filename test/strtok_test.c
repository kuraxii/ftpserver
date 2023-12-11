/*********************************************
* @FileName: strtok_test.c
* @Author: Null-zzj
* @Mail: zj.zhu.cn@gmail.com
* @CreatedTime: 周一 12月 11 17:08:51 2023
* @Descript:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
char *strtok_once(char *s, const char *delim, char **ptrptr)
{
    char *tmp = NULL;

    if (s == NULL)
    {
        s = *ptrptr;
    }
    printf("111   %s\n", s);
    s += strspn(s, delim);
    printf("222\n");

    if (*s)
    {
        tmp = s;
        s += strcspn(s, delim);

        if (*s)
        {
            *s++ = 0;
            *ptrptr = s;
        }
        else
        {
            *ptrptr = NULL; // Reach the end after the first token
        }
    }
    else
    {
        *ptrptr = NULL; // Reach the end without any token found
    }

    return tmp;
}

int main(int argc, char *argv[])
{
    char str1[] = "SYNC";
    char str2[] = "ABC DEF";
    char str3[] = "123 456 789";
    char* rest;
    char* ptrptr;
    char* token;


    

    rest = str1;
    ptrptr = NULL;
    while((token = strtok_once(rest, " ", &ptrptr)))
    {
        printf("token: %s\n", token);
        rest = NULL;
    }

    rest = str2;
    ptrptr = NULL;
    while((token = strtok_once(rest, " ", &ptrptr)))
    {
        printf("token: %s\n", token);
        rest = NULL;
    }


    rest = str3;
    ptrptr = NULL;
    while((token = strtok_once(rest, " ", &ptrptr)))
    {
        printf("token: %s\n", token);
        rest = NULL;
    }

    return 0;
}