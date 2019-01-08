#include <drivers/keyboard.h>
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;



KeyboardEventHandler::KeyboardEventHandler()
{

}
void KeyboardEventHandler::OnKeyDown(char)
{

}
 void KeyboardEventHandler::OnKeyUp(char)
 {

 }

KeyboardDriver :: KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler)
: InterruptHandler(0x21, manager),
  dataport(0x60),
  commandport(0x64)
{
    this->handler = handler;
    // raft too activate 
    // // wait for you stop pressing a key
    // while(commandport.Read() & 0x1)
    //     dataport.Read();
    
    // commandport.Write(0xAE); // // activate interrupts...tells the pic to start sending keyboard interrupt
    // commandport.Write(0x20); // get current state
    // uint8_t status = (dataport.Read() |1) & ~0x10; // we set the rightmost bit to 1 and clear the 5th bit
    // commandport.Write(0x60); //set state (tells the keyboard to change the current state)
    // dataport.Write(status);
    // dataport.Write(0xF4); // activate keyboard

}


KeyboardDriver :: ~KeyboardDriver(){

}

void printf(char*);
void printfHex(uint8_t);
void KeyboardDriver::Activate()
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
uint32_t KeyboardDriver :: HandleInterrupt(uint32_t esp)
{
    // if there is a key strike we have to fetch it
    uint8_t key = dataport.Read();

    if(handler==0){
        return esp;
    }


    // release?
    if(key < 0x80)
    {

        switch(key){
            // replace printf with handler->onkeydown
            case 0x02: handler->OnKeyDown('1'); break;
            case 0x03: handler->OnKeyDown('2'); break;
          
            //ignores
            case 0xFA : break;
            case 0x1E: handler->OnKeyDown('a'); break;
            // numlock
            case 0x45: case 0xC5: break;

            default:
            {
                printf("KEYBOARD 0x");
                printfHex(key);
                break;
            }
        }
    }
    return esp;
}
