#include "keyboard.h"

KeyboardDriver :: KeyboardDriver(InterruptManager* manager){

}
KeyboardDriver :: ~KeyboardDriver(){

}
uint32_t KeyboardDriver :: HandleInterrupt(uint32_t esp)
{
    return esp;
}
