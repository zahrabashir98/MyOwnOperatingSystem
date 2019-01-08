#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <hardwarecommunication/pci.h>
#include <drivers/vga.h>
#include <multitasking.h>

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






void taskA()
{
    while(true)
        printf("A");
}
void taskB()
{
    while(true)
        printf("B");
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

    printf("http://google.com\n");
    GlobalDescriptorTable gdt;

    // use multiboot info
    // memupper tells us the size of ram
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024; //10 MiB
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);
    
    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");

    /************************/
    TaskManager taskManager;
    Task task1(&gdt, taskA);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
   /******************************/

    // first intialize hardware and others and finally interrupts
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    printf("initializating hardware, stage1\n");

    DriverManager drvManager;

        PrintfKeyboardEventHandler kbhandler;
        KeyboardDriver keyboard(&interrupts, &kbhandler);
        drvManager.AddDriver(&keyboard);

        MouseToConsole mousehandler;
        MouseDriver mouse(&interrupts, &mousehandler);
        drvManager.AddDriver(&mouse);


        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);


        VideoGraphicsArray vga;

    printf("initializating hardware, stage2\n");
        drvManager.ActivateAll();

    printf("initializating hardware, stage3\n");
    interrupts.Activate();
    // evrey thing after this might not be executed

    vga.SetMode(300, 200,8);
    //draw a blue rectangle
    for( int32_t y=0; y<200;y++)
        for( int32_t x=0; x<320;x++)
            vga.PutPixel(x, y, 0x00, 0x00, 0xA8);
    // kernel shouldn't stop
    while(1);
}