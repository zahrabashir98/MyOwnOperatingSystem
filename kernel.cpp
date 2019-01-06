#include "types.h"
#include "gdt.h"
void printf(char* str){
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    // copy string to this location
    for(int i=0 ; str[i] != '\0'; ++i){
        // seperate high bytes to avoid overriding
        VideoMemory[i] = (VideoMemory[i] & 0xFF00) | str[i];
    }

}

typedef void(*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
// jumps into function pointers
extern "C" void callConstructors()
{
    for (constructor* i=&start_ctors; i != end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*magicnumber*/){

    printf("http://google.com");
    GlobalDescriptorTable gdt;
    // kernel shouldn't stop
    while(1);
}