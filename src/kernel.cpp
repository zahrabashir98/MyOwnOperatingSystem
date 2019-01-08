#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <hardwarecommunication/pci.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;


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


class MouseToConsole : public MouseEventHandler
{
    int8_t x,y;

public:


    MouseToConsole()
    {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    x = 40;
    y = 12;
    VideoMemory[80*12+40] = ((VideoMemory[80*12+40] & 0xF000 ) >> 4)
                        | ((VideoMemory[80*12+40] & 0x0F00 ) << 4)
                        | ((VideoMemory[80*12+40] & 0x00FF ));

    }
    void OnMouseMove(int xoffset , int yoffset)
    {
        // Display cursor on screen
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;

        VideoMemory[80*y+x] = ((VideoMemory[80*y+x] & 0xF000 ) >> 4)
                            | ((VideoMemory[80*y+x] & 0x0F00 ) << 4)
                            | ((VideoMemory[80*y+x] & 0x00FF ));

        x += xoffset;
        y += yoffset;

        // we dont want the mouse to run out of the screen
        if(x<0) x=0;
        if(x>=80) x=79;
        if (y<0) y=0;
        if (y>=25) y=24;

        // I want the cursor to switch( high first) (after we move cursor)
        VideoMemory[80*y+x] = ((VideoMemory[80*y+x] & 0xF000 ) >> 4)
                            | ((VideoMemory[80*y+x] & 0x0F00 ) << 4)
                            | ((VideoMemory[80*y+x] & 0x00FF ));

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

        MouseToConsole mousehandler;
        MouseDriver mouse(&interrupts, &mousehandler);
        drvManager.AddDriver(&mouse);


        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager);
    printf("initializating hardware, stage2\n");
        drvManager.ActivateAll();

    printf("initializating hardware, stage3\n");
    interrupts.Activate();
    // kernel shouldn't stop
    while(1);
}