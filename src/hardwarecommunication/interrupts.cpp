#include <hardwarecommunication/interrupts.h>
using namespace myos::common;
using namespace myos::hardwarecommunication;


void printf(char* str);
void printfHex(uint8_t);


InterruptHandler :: InterruptHandler(uint8_t interruptNumber, InterruptManager* interruptManager)
{
    this->interruptNumber = interruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[interruptNumber] = this;
}

InterruptHandler :: ~InterruptHandler()
{
    if (interruptManager->handlers[interruptNumber] == this)
        interruptManager->handlers[interruptNumber] = 0;
}

uint32_t InterruptHandler :: HandleInterrupt(uint32_t esp)
{
    return esp;
}



InterruptManager :: GateDescriptor InterruptManager:: interruptDescriptorTable[256];

InterruptManager* InterruptManager :: ActiveInterruptManager = 0;

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
  
InterruptManager :: InterruptManager(uint16_t hardwareInterruptOffset, GlobalDescriptorTable* gdt, TaskManager* taskManager)
: picMasterCommand(0x20),
  picMasterData(0x21),
  picSalveCommand(0xA0),
  picSalveData(0xA1)
{
    // set all entries to interrupt ignore 
    this->hardwareInterruptOffset = hardwareInterruptOffset;
    this->taskManager = taskManager;
    uint32_t CodeSegment = gdt->codeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE =0xE; // type


    for (uint16_t i=0; i<256;i++){
        handlers[i] = 0;
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0,IDT_INTERRUPT_GATE);
    }
    
    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0,IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0,IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0,IDT_INTERRUPT_GATE);
  
    picMasterCommand.Write(0x11);
    picSalveCommand.Write(0x11);
    // we tell the master pic if you get any inte at 0x20 (each have 8 interrupts)
    picMasterData.Write(0x20);
    picSalveData.Write(0x28);

    picMasterData.Write(0x04);
    picSalveData.Write(0x02);

    picMasterData.Write(0x01);
    picSalveData.Write(0x01);

    picMasterData.Write(0x00);
    picSalveData.Write(0x00);


    /*****************************/
    // tell processor to use IDT 
    interruptDescriptorTablePointer idt;
    idt.size = 256*sizeof(GateDescriptor)-1;
    idt.base = (uint32_t)interruptDescriptorTable;
    asm volatile("lidt %0" : : "m"(idt));
}
InterruptManager :: ~InterruptManager(){

}
void InterruptManager :: Activate()
{

    if(ActiveInterruptManager != 0)
        ActiveInterruptManager->Deactivate();
    ActiveInterruptManager = this;
    // Start Interrupts
    asm("sti");
    
}
void InterruptManager :: Deactivate()
{

    if(ActiveInterruptManager == this){
        ActiveInterruptManager = 0;
        asm("cli");
    }
       
    
}
uint32_t InterruptManager :: handleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    // use active... pointer and call DoHandleInterrupt on it
    if (ActiveInterruptManager != 0){
        return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
    }
    // cause no multiple processes just return it
    return esp;

}

// here we send answer to  hardware interrupt (we dont want os to halt here)
uint32_t InterruptManager :: DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{

    if (handlers[interruptNumber]!=0)
    {
        esp = handlers[interruptNumber]->HandleInterrupt(esp);
    }

    else if (interruptNumber != 0x20){
        printf("UNHANDLED INTERRUPT 0x");
        printfHex(interruptNumber);
        // // if it is not timer interrupt
        // char* foo = "UNHANDLED INTERRUPT 0x";
        // char* hex = "0123456789ABCDEF";
        // foo[22] = hex[(interruptNumber >> 4) & 0x0F];
        // foo[23] = hex[interruptNumber & 0x0F];
        // printf(foo);
    }


    if (interruptNumber == 0x20)
    {
        esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
    }
    //  this case we have to send answer
    if (0x20 <= interruptNumber < 0x30){
        picMasterCommand.Write(0x20);

        if (0x28 <= interruptNumber){
            // slaves only
            picSalveCommand.Write(0x20);}
    }
    return esp;

}

//now we need to tell the processor to use this
// now we need to tell pic
// we need port for Pic communication (in interrupts.h avval)