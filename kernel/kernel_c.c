typedef unsigned long long uintptr_t;

extern void print(char* string);
extern void newLine();

extern void processBuffer();

extern void initPMM();

int commandReady = 0;

void main()
{
    initPMM();

    print("Kernel Loaded");
    newLine();

    while(1 == 1)
    {
        if(commandReady == 1)
        {
            processBuffer();
            commandReady = 0;
        }
        else
        {
            __asm__ volatile ("hlt");
        }
    }

    return;
}