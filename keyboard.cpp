#include "keyboard.h"

KeyboardDriver :: KeyboardDriver(InterruptManager* manager)
: InterruptHandler(0x21, manager),
  dataport(0x60),
  commandport(0x64)
{
    // wait for you stop pressing a key
    while(commandport.Read() & 0x1)
        dataport.Read();
    
    commandport.Write(0xAE); // // activate interrupts...tells the pic to start sending keyboard interrupt
    commandport.Write(0x20); // get current state
    uint8_t status = (dataport.Read() |1) & ~0x10; // we set the rightmost bit to 1 and clear the 5th bit
    commandport.Write(0x60); //set state (tells the keyboard to change the current state)
    dataport.Write(status);
    dataport.Write(0xF4); // activate keyboard

}


KeyboardDriver :: ~KeyboardDriver(){

}

void printf(char*);

uint32_t KeyboardDriver :: HandleInterrupt(uint32_t esp)
{
    // if there is a key strike we have to fetch it
    uint8_t key = dataport.Read();
    // release?
    if(key < 0x80)
    {

        switch(key){

            case 0xFA : break;
            case 0x1E: printf("a"); break;
            // numlock - ignore
            case 0x45: case 0xC5: break;

            default:
                // prints the key
                char* foo = "KEYBOARD 0x00";
                char* hex = "0123456789ABCDEF";
                foo[11] = hex[(key >> 4) & 0x0F];
                foo[12] = hex[key & 0x0F];
                printf(foo);
        }
    }
    return esp;
}
