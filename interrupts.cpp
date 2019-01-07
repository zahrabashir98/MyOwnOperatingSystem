#include "interrupts.h"

void printf(char* str);

InterruptManager :: GateDescriptor InterruptManager:: interruptDescriptorTable[256];
void InterruptManager :: SetInterruptDescriptorTableEntry(
            uint8_t interruptNumber,
            uint16_t gdt_codeSegmentSelectorOffset,
            void (*handler)(),
            uint8_t DescriptorPrivilegeLevel,
            uint8_t DescriptorType)
{
    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = gdt_codeSegmentSelectorOffset;
    interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | DescriptorType | ((DescriptorPrivilegeLevel&3) <<5);
    interruptDescriptorTable[interruptNumber].reserved = 0;
}
  
InterruptManager :: InterruptManager(GlobalDescriptorTable* gdt){
    // set all entries to interrupt ignore 
    uint16_t CodeSegment = gdt->codeSegmentSelector();
    // type
    const uint8_t IDT_INTERRUPT_GATE =0xE;
    for (uint16_t i=0; i<256;i++)
        SetInterruptDescriptorTableEntry(i, CodeSegment, &InterruptIgnore, 0,IDT_INTERRUPT_GATE);
    
    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0,IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0,IDT_INTERRUPT_GATE);

}
InterruptManager :: ~InterruptManager(){

}

uint32_t InterruptManager :: handleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    printf("INTERRUPT");
    // cause no multiple processes just return it
    return esp;

}
