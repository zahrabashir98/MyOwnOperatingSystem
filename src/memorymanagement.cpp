
#include <memorymanagement.h>

using namespace myos;
using namespace myos::common;


MemoryManager* MemoryManager::activeMemoryManager = 0;
        
MemoryManager::MemoryManager(size_t start, size_t size)
{
    activeMemoryManager = this;

    // if size is not sufficent
    if(size < sizeof(MemoryChunk))
    {
        first = 0;
    }
    else
    {
        // start if the offset where the memory manager is going to start with and make chunks
        first = (MemoryChunk*)start;
        
        first -> allocated = false;
        first -> prev = 0;
        first -> next = 0;
        first -> size = size - sizeof(MemoryChunk);
    }
}

MemoryManager::~MemoryManager()
{
    if(activeMemoryManager == this)
        activeMemoryManager = 0;
}
        
void* MemoryManager::malloc(size_t size)
{
    MemoryChunk *result = 0;
    // look for chunk that is large enough and free
    for(MemoryChunk* chunk = first; chunk != 0 && result == 0; chunk = chunk->next)
        if(chunk->size > size && !chunk->allocated)
            result = chunk;
        
    if(result == 0)
        return 0;
    // we have != 0 result so: (if chunk is too small)
    // if result is larger than what we need
    if(result->size >= size + sizeof(MemoryChunk) + 1)
    {
        MemoryChunk* temp = (MemoryChunk*)((size_t)result + sizeof(MemoryChunk) + size);
        
        temp->allocated = false;
        temp->size = result->size - size - sizeof(MemoryChunk);
        temp->prev = result;
        temp->next = result->next;
        if(temp->next != 0)
            temp->next->prev = temp;
        
        result->size = size;
        result->next = temp;
    }
    
    result->allocated = true;
    //result pointer points to the begining of result
    return (void*)(((size_t)result) + sizeof(MemoryChunk));
}

void MemoryManager::free(void* ptr)
{
    //we subtract size of memory chunk from it
    MemoryChunk* chunk = (MemoryChunk*)((size_t)ptr - sizeof(MemoryChunk));
    
    chunk -> allocated = false;
    
    if(chunk->prev != 0 && !chunk->prev->allocated)
    {
        chunk->prev->next = chunk->next;
        // add size of memory chunk we freed to the chunk and left is allocated
        chunk->prev->size += chunk->size + sizeof(MemoryChunk);
        if(chunk->next != 0)
            chunk->next->prev = chunk->prev;
        
        chunk = chunk->prev;
    }
    // Right is free and we have to merge
    if(chunk->next != 0 && !chunk->next->allocated)
    {
        chunk->size += chunk->next->size + sizeof(MemoryChunk);
        chunk->next = chunk->next->next;
        if(chunk->next != 0)
            chunk->next->prev = chunk;
    }
    
}




// void* operator new(unsigned size)
// {
//     if(myos::MemoryManager::activeMemoryManager == 0)
//         return 0;
//     return myos::MemoryManager::activeMemoryManager->malloc(size);
// }

// void* operator new[](unsigned size)
// {
//     if(myos::MemoryManager::activeMemoryManager == 0)
//         return 0;
//     return myos::MemoryManager::activeMemoryManager->malloc(size);
// }

// void* operator new(unsigned size, void* ptr)
// {
//     return ptr;
// }

// void* operator new[](unsigned size, void* ptr)
// {
//     return ptr;
// }

// void operator delete(void* ptr)
// {
//     if(myos::MemoryManager::activeMemoryManager != 0)
//         myos::MemoryManager::activeMemoryManager->free(ptr);
// }

// void operator delete[](void* ptr)
// {
//     if(myos::MemoryManager::activeMemoryManager != 0)
//         myos::MemoryManager::activeMemoryManager->free(ptr);
// }