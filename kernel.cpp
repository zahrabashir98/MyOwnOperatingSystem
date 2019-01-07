#include "types.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"

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

    printf("http://google.com");
    GlobalDescriptorTable gdt;
    // first intialize hardware and others and finally interrupts
    InterruptManager interrupts(&gdt);
    KeyboardDriver keyboard(&interrupts);
    MouseDriver mouse(&interrupts);

    interrupts.Activate();
    // kernel shouldn't stop
    while(1);
}