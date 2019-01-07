#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

#include "types.h"
#include "port.h"
#include "gdt.h"
class InterruptManager;

class InterruptHandler{
    protected:
        uint8_t interruptNumber;
        InterruptManager* interruptManager;
        InterruptHandler(uint8_t interruptNumber, InterruptManager* interruptManager);
        ~InterruptHandler();
    public:
        virtual uint32_t HandleInterrupt(uint32_t esp);
};
class InterruptManager{
    
    friend class InterruptHandler;
    protected:

        static InterruptManager* ActiveInterruptManager;
        InterruptHandler* handlers[256];
        struct GateDescriptor
        {
            uint16_t handlerAddressLowBits;
            uint16_t gdt_codeSegmentSelector;
            uint8_t reserved;
            uint8_t access;
            uint16_t handlerAddressHighBits;

        }__attribute__((packed));

        static GateDescriptor interruptDescriptorTable[256];
        struct interruptDescriptorTablePointer{
                uint32_t base;
                uint16_t size;

        }__attribute__((packed));

        static void SetInterruptDescriptorTableEntry(
            uint8_t interruptNumber,
            uint16_t gdt_codeSegmentSelectorOffset,
            void (*handler)(),
            uint8_t DescriptorPrivilegeLevel,
            uint8_t DescriptorType);

        Port8BitSlow picMasterCommand;
        Port8BitSlow picMasterData;
        Port8BitSlow picSalveCommand;
        Port8BitSlow picSalveData;

    public:
        InterruptManager(GlobalDescriptorTable* gdt);
        ~InterruptManager();

        void Activate();
        void Deactivate();

        // esp is the current stack pointer(assembler code gives us the current stack pointer)
        static uint32_t handleInterrupt(uint8_t interruptNumber, uint32_t esp);
        uint32_t DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp);
        static void IgnoreInterruptRequest();
        // timer interrupts
        static void HandleInterruptRequest0x00();
        // keyboard interrupts
        static void HandleInterruptRequest0x01();
        static void HandleInterruptRequest0x0C();


};

#endif