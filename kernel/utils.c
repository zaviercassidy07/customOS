#include "utils.h"

int compareArray(char* str1, char* str2)
{
    for(int i = 0; i <= 128; i++)
    {
        if(str1[i] == str2[i])
        {
            if(str1[i] == 0)
            {
                return 1;
            }
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void strCopy(char* str, char* addr)
{
    while(str[0] != 0)
    {
        addr[0] = str[0];
        str++;
        addr++;
    }
    addr[0] = 0;
    return;
}