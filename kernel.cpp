#include "types.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"
#include "driver.h"

void printf(char* str){

    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    
    // resolve stupidity of printf ^__^
    static uint8_t x=0, y=0;

    // copy string to this location
    for(int i=0 ; str[i] != '\0'; ++i){
        // \ handling(line fit)
        switch(str[i])
        {
            case '\n':
                y++;
                x = 0;
                break;
                
            default:
                // seperate high bytes to avoid overriding
                VideoMemory[80*y +x] = (VideoMemory[80*y +x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if (x>=80)
        {
            y++;
            x = 0;
        }

        if (y>=25)
        {
            for (y=0; y<25; y++)
                for(x=0 ; x<80; x++)
                    VideoMemory[80*y +x] = (VideoMemory[80*y +x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        
        }

    }

}
void printfHex(uint8_t key){

    // prints the key
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0x0F];
    foo[1] = hex[key & 0x0F];
    printf(foo);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }


};



typedef void(*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
// jumps into function pointers
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*magicnumber*/){

    printf("http://google.com\n");
    GlobalDescriptorTable gdt;
    // first intialize hardware and others and finally interrupts
    InterruptManager interrupts(&gdt);
    
    printf("initializating hardware, stage1\n");

    DriverManager drvManager;

        PrintfKeyboardEventHandler kbhandler;
        KeyboardDriver keyboard(&interrupts, &kbhandler);
        drvManager.AddDriver(&keyboard);

        MouseDriver mouse(&interrupts);
        drvManager.AddDriver(&mouse);

    printf("initializating hardware, stage2\n");
        drvManager.ActivateAll();

    printf("initializating hardware, stage3\n");
    interrupts.Activate();
    // kernel shouldn't stop
    while(1);
}