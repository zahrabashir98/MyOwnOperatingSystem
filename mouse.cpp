#include "mouse.h"

MouseDriver :: MouseDriver(InterruptManager* manager, MouseEventHandler* handler)
: InterruptHandler(0x2C, manager),
  dataport(0x60),
  commandport(0x64)
{

    this->handler = handler;
// raft too activate
    // offset = 0;
    // buttons = 0;

    // static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    // VideoMemory[80*12+40] = ((VideoMemory[80*12+40] & 0xF000 ) >> 4)
    //                     | ((VideoMemory[80*12+40] & 0x0F00 ) << 4)
    //                     | ((VideoMemory[80*12+40] & 0x00FF ));

    // commandport.Write(0xAE); // // activate interrupts...tells the pic to start sending keyboard interrupt
    // commandport.Write(0x20); // get current state from pic
    // uint8_t status = (dataport.Read() |2); // we set second bits to true
    // commandport.Write(0x60); //set state (tells the keyboard to change the current state)
    // dataport.Write(status);
    // commandport.Write(0xD4);
    // dataport.Write(0xF4); // activate
    // dataport.Read();

}


MouseDriver :: ~MouseDriver(){

}

void printf(char*);

void MouseDriver::Activate()
{
    offset = 0;
    buttons = 0;

    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    VideoMemory[80*12+40] = ((VideoMemory[80*12+40] & 0xF000 ) >> 4)
                        | ((VideoMemory[80*12+40] & 0x0F00 ) << 4)
                        | ((VideoMemory[80*12+40] & 0x00FF ));

    commandport.Write(0xAE); // // activate interrupts...tells the pic to start sending keyboard interrupt
    commandport.Write(0x20); // get current state from pic
    uint8_t status = (dataport.Read() |2); // we set second bits to true
    commandport.Write(0x60); //set state (tells the keyboard to change the current state)
    dataport.Write(status);
    commandport.Write(0xD4);
    dataport.Write(0xF4); // activate
    dataport.Read();

}


uint32_t MouseDriver :: HandleInterrupt(uint32_t esp)
{
    // read status from commnad port
    uint8_t status = commandport.Read();

    //test if it is data for us
    if (!(status & 0x20)) //if six bits are 1 -> then there is data to read
        return esp;

    static int8_t x=40, y=12;
    buffer[offset] = dataport.Read();
    offset = (offset+1) %3;

    // For movement
    if (offset ==0 )
    {
        // Display cursor on screen
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;

        VideoMemory[80*y+x] = ((VideoMemory[80*y+x] & 0xF000 ) >> 4)
                            | ((VideoMemory[80*y+x] & 0x0F00 ) << 4)
                            | ((VideoMemory[80*y+x] & 0x00FF ));

        x += buffer[1];
        y -= buffer[2];

        // we dont want the mouse to run out of the screen
        if(x<0) x=0;
        if(x>=80) x=79;
        if (y<0) y=0;
        if (y>=25) y=24;

        // I want the cursor to switch( high first) (after we move cursor)
        VideoMemory[80*y+x] = ((VideoMemory[80*y+x] & 0xF000 ) >> 4)
                            | ((VideoMemory[80*y+x] & 0x0F00 ) << 4)
                            | ((VideoMemory[80*y+x] & 0x00FF ));

        // look if the button has been presses(compare old and new state)
        for (uint8_t i=0; i<3; i++)
        {   
            if ((buffer[0] & (0x01 <<i)) != (buttons & (0x01<<i))){
                    // invert color
                    VideoMemory[80*y+x] = ((VideoMemory[80*y+x] & 0xF000 ) >> 4)
                                        | ((VideoMemory[80*y+x] & 0x0F00 ) << 4)
                                        | ((VideoMemory[80*y+x] & 0x00FF ));
            }
        }
        // when we are done we copy buffer[0]
        buttons = buffer[0];
    }
    return esp;
}
