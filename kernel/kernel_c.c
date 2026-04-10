extern void print(char* string);
extern void newLine();

extern void processBuffer();

int commandReady = 0;

void main()
{
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