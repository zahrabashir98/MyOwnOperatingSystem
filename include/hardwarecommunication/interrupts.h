#ifndef __MYOS_HARDWARECOMMUNICATION__INTERRUPTS_H
#define __MYOS_HARDWARECOMMUNICATION__INTERRUPTS_H

#include <common/types.h>
#include <multitasking.h>
#include <hardwarecommunication/port.h>
#include <gdt.h>


namespace myos
{
    namespace hardwarecommunication
    {
        class InterruptManager;

        class InterruptHandler{
            protected:
                myos::common::uint8_t interruptNumber;
                InterruptManager* interruptManager;
                InterruptHandler(myos::common::uint8_t interruptNumber, InterruptManager* interruptManager);
                ~InterruptHandler();
            public:
                virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
        };
        class InterruptManager{
            
            friend class InterruptHandler;
            protected:

                static InterruptManager* ActiveInterruptManager;
                InterruptHandler* handlers[256];
                TaskManager *taskManager;

                struct GateDescriptor
                {
                    myos::common::uint16_t handlerAddressLowBits;
                    myos::common::uint16_t gdt_codeSegmentSelector;
                    myos::common::uint8_t reserved;
                    myos::common::uint8_t access;
                    myos::common::uint16_t handlerAddressHighBits;

                }__attribute__((packed));

                static GateDescriptor interruptDescriptorTable[256];
                struct interruptDescriptorTablePointer{
                        myos::common::uint32_t base;
                        myos::common::uint16_t size;

                }__attribute__((packed));

                static void SetInterruptDescriptorTableEntry(
                    myos::common::uint8_t interruptNumber,
                    myos::common::uint16_t gdt_codeSegmentSelectorOffset,
                    void (*handler)(),
                    myos::common::uint8_t DescriptorPrivilegeLevel,
                    myos::common::uint8_t DescriptorType);

                Port8BitSlow picMasterCommand;
                Port8BitSlow picMasterData;
                Port8BitSlow picSalveCommand;
                Port8BitSlow picSalveData;

            public:
                InterruptManager(myos::common::uint16_t hardwareInterruptOffset, GlobalDescriptorTable* gdt, myos::TaskManager* taskManager);
                ~InterruptManager();

                void Activate();
                void Deactivate();

                // esp is the current stack pointer(assembler code gives us the current stack pointer)
                static myos::common::uint32_t handleInterrupt(myos::common::uint8_t interruptNumber, myos::common::uint32_t esp);
                myos::common::uint32_t DoHandleInterrupt(myos::common::uint8_t interruptNumber, myos::common::uint32_t esp);
                static void IgnoreInterruptRequest();
                // timer interrupts
                static void HandleInterruptRequest0x00();
                // keyboard interrupts
                static void HandleInterruptRequest0x01();
                static void HandleInterruptRequest0x0C();


        };
    }
}

#endif