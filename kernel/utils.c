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

size_t strLen(char* str)
{
    size_t size = 0;

    while(str[size] != 0)
    {
        size++;
    }
    return size + 1; // plus one accounts for null terminator
}

void split(char* in, char splitter, char* out1, char* out2)
{
    int hasSplit = 0;
    while(in[0] != 0)
    {
        if(hasSplit == 1)
        {
            out2[0] = in[0];

            out2++;
            in++;
            continue;
        }
        else if(in[0] == ' ')
        {
            hasSplit = 1;
            in++;
            continue;
        }
        out1[0] = in[0];
        out1++;
        in++;
    }
    out1[0] = 0;
    out2[0] = 0;
    return;
}

uint64_t convInt(char* input)
{
    uint64_t total = 0;
    uint64_t index = 0;

    while(input[index] != 0)
    {
        total = (total * 10) + (input[index] - '0');
        index++;
    }

    return total;
}