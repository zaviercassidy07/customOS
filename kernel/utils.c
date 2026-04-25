#include "utils.h"

int compareArray(char* str1, char* str2)
{
    for(int i = 0; i < 128; i++)
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

uint64_t convInt(char* input)
{
    uint64_t total = 0;
    uint64_t index = 0;

    while(input[index] != '\0')
    {
        total = (total * 10) + (input[index] - '0');
        index++;
    }

    return total;
}