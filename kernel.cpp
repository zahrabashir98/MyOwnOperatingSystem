void printf(char* str){
    unsigned short* VideoMemory = (unsigned short*)0xb8000;
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

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber){

    printf("Hello World!.......http://google.com");
    // kernel shouldn't stop
    while(1);
}