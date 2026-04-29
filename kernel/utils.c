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

int compareArraySize(char* str1, char* str2, int amount)
{
    for(int i = 0; i < amount; i++)
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
    if(str1[amount - 1] == str2[amount - 1])
    {
        return 1;
    }
    return 0;
}

void memSet(uint8_t* addr, char c, size_t amount)
{
    for(size_t i = 0; i < amount; i++)
    {
        addr[i] = c;
    }
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
        else if(in[0] == splitter)
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
void splitN(char* in, char splitter, char* out1, char* out2)
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
        else if(in[0] == splitter)
        {
            hasSplit = 1;
            in++;
            continue;
        }
        out1[0] = in[0];
        out1++;
        in++;
    }

    return;
}

int count(char* str, char c)
{
    int num = 0;
    while(str[0] != 0)
    {
        if(str[0] == c)
        {
            num++;
        }
        str++;
    }
    return num;
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