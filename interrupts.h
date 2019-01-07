#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H
#include "types.h"

class InterruptManager{
    // esp is the current stack pointer(assembler code gives us the current stack pointer)
    public:
        static uint32_t handleInterrupt(uint8_t interruptNumber, uint32_t esp);
};

#endif