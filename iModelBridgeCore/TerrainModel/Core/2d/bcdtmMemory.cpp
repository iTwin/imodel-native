/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmMemory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "malloc.h"
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
//#pragma optimize( "p", on )
#include "bcDTMElement.h"

//#define DEBUGMEM

#ifdef DEBUGMEM

#include <stdarg.h>

#undef bcdtmMemory_allocate
#undef bcdtmMemory_reallocate
#undef bcdtmMemory_free
/*
* MOSSComms::write_to_log
*
* Output data to the MOSS Comms logfile.
*/
void write_to_log(char* format ...)
    {
    //   va_list va;
    //static int first = 1;
    //   static char File[512];
    //FILE* fp;

    //va_start(va, format);

    //if(first)
    //{
    //    first = 0;

    //       strcpy (File, "d:\\log.log");

    //       remove(File);
    //       fp = fopen(File,"w");
    //}
    //else
    //	fp = fopen(File,"a");

    //   if(fp)
    //   {
    //    vfprintf(fp, format, va);
    //       fprintf(fp, "\n");

    //    fclose(fp);
    //   }

    //va_end(va);
    }

struct memstate
    {
    bool valid;
    size_t size;
    char* allocFile;
    int allocLine;
    char* freeFile;
    int freeLine;
    };
bmap<void*, memstate> s_allocations;

void HasError()
    {
    OutputDebugString ("Error\n");
    }

void CheckMemory (void * l)
    {
    if (s_allocations.find (l) == s_allocations.end())
        {
        // Error...
        HasError();
        }
    else
        {
        memstate* state = &s_allocations[l];
        if (!s_allocations[l].valid)
            HasError();
        }
    }

void checkAndFree (void* l, char* file, int line)
    {
    if (s_allocations.find (l) != s_allocations.end())
        {
        write_to_log ("Free %x %s %d\n", l, file, line);
        memstate* state = &s_allocations[l];
        if (!state->valid)
            HasError();
        else
            free (l);
        state->freeFile = file;
        state->freeLine = line;
        state->valid = false;
        }
    else
        {
        // Error...
        write_to_log ("Free %x %s %d <-- Error\n", l, file, line);
        HasError();
        }
    }

void* checkMAlloc(size_t size, char* file, int line)
    {
    void* l = malloc(size);
    if (l != NULL)
        {
        write_to_log ("Malloc %x, %s %d", l, file, line);
        memstate state;
        state.size = size;
        state.valid = true;
        state.allocFile = file;
        state.allocLine = line;
        state.freeFile = 0;
        state.freeLine = 0;
        s_allocations[l] = state;
        }
    else
        HasError();
    return l;
    }

void* checkReAlloc (void* l, size_t size, char* file, int line)
    {
    void* l2;
    if (s_allocations.find (l) != s_allocations.end())
        {
        memstate* state = &s_allocations[l];
        if (!state->valid)
            HasError();
        state->valid = false;
        state->freeFile = file;
        state->freeLine = line;
        l2 = realloc (l, size);
        }
    else
        {
        // Error...
        HasError();
        write_to_log ("Realloc %x, %s %d <--Error", l, file, line);
        l2 = realloc (l, size);
        }

    if (l2 != NULL)
        {
        write_to_log ("Realloc %x (%x) %s %d", l2, l, file, line);
        memstate state;
        state.size = size;
        state.valid = true;
        state.allocFile = file;
        state.allocLine = line;
        state.freeFile = 0;
        state.freeLine = 0;
        s_allocations[l2] = state;
        }
    else
        HasError();
    return l2;
    }
#endif

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void* bcdtmMemory_allocatePartition(
    BC_DTM_OBJ *dtmP,
    DTMPartition type,
    int partitionNumber,
    size_t size)

    // Called when the DTM needs to allocate memory for one of the main DTM partitions.
    // If it is a normal dtm it calls malloc and returns, otherwise it calls the DTMElement Partition Allocation method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        char *memP = NULL ;
        memP = ( char *) malloc( size * sizeof( char)) ;
        return (void *) memP  ;
        }
    IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

    return mem->AllocateMemory(type, partitionNumber, size);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void* bcdtmMemory_reallocatePartition(
    BC_DTM_OBJ *dtmP,
    DTMPartition type,
    int partitionNumber,
    void* pointer,
    size_t size)
    // Called when the DTM needs to reallocate memory for one of the main DTM partitions.
    // If it is a normal dtm it calls realloc and returns, otherwise it calls the DTMElement Partition Reallocation method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        //            dtmP->memoryUsed += size - _msize(pointer);
        return realloc(pointer, size);
        }
    IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

    return mem->ReallocateMemory(type, partitionNumber, pointer, size);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void bcdtmMemory_freePartition(
    BC_DTM_OBJ *dtmP,
    DTMPartition type,
    int partitionNumber,
    void* pointer)
    // Called when the DTM needs to free memory for one of the main DTM partitions.
    // If it is a normal dtm it calls free and returns, otherwise it calls the DTMElement Partition Free method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        //            dtmP->memoryUsed -= _msize(pointer);
        free(pointer);
        }
    else
        {
        IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;
        if (nullptr != mem)
            mem->FreeMemory (type, partitionNumber, pointer);
        }
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
// otherwise it need to translate partitionNumber in to the pointer to the memory.
BENTLEYDTM_EXPORT void* bcdtmMemory_getPointer(
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber)
    // Called when the DTM needs to get a pointer to a section of allocated memory.
    // If it is a normal dtm it calls the partitionNumber is the pointer to the data so it returns this.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
#ifdef DEBUGMEM
        CheckMemory ((void*)partitionNumber);
#endif
        return (void*)partitionNumber;
        }
    IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

    return (void*)mem->GetMemoryPointer((int)partitionNumber);
    }


#ifdef DEBUGMEM
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
DTMMemPnt bcdtmMemory_allocateDebug(
    BC_DTM_OBJ *dtmP,
    size_t size,
    char* location, int line
    )
    // Called when the DTM needs to allocate a section of memory.
    // If it is a normal dtm it calls malloc and returns, otherwise it calls the DTMElement Section Allocation method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        //            dtmP->memoryUsed += size;
        return (DTMMemPnt)checkMAlloc(size, location, line);
        }
    IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

    return mem->AllocateMemory(size);
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
DTMMemPnt bcdtmMemory_reallocateDebug(
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber,
    size_t size,
    char* file,
    int line)
    // Called when the DTM needs to reallocate a section of memory.
    // If it is a normal dtm it calls realloc and returns, otherwise it calls the DTMElement Section Reallocation method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        //            dtmP->memoryUsed += size - _msize((void*)partitionNumber);
        return (DTMMemPnt)checkReAlloc((void*)partitionNumber, size, file, line);
        }
    IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

    return (DTMMemPnt)mem->ReallocateMemory((int)partitionNumber, size);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
void bcdtmMemory_freeDebug(
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber,
    char* file,
    int line)
    // Called when the DTM needs to free a section of memory.
    // If it is a normal dtm it calls free and returns, otherwise it calls the DTMElement Section Free method.
    {
    if(dtmP->dtmObjType != BC_DTM_ELM_TYPE)
        {
        //            dtmP->memoryUsed -= _msize((void*)partitionNumber);
        checkAndFree((void*)partitionNumber, file, line);
        }
    else
        {
        IDTMElementMemoryAllocator* mem = (IDTMElementMemoryAllocator*)dtmP->DTMAllocationClass;

        mem->FreeMemory((int)partitionNumber);
        }
    }
BENTLEYDTM_EXPORT DTMMemPnt bcdtmMemory_allocate(BC_DTM_OBJ *dtmP, size_t size)
    {
    return bcdtmMemory_allocateDebug (dtmP, size, "", 0);
    }
BENTLEYDTM_EXPORT DTMMemPnt bcdtmMemory_reallocate(
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber,
    size_t size)
    {
    return bcdtmMemory_reallocateDebug(dtmP, partitionNumber, size, "", 0);
    }
BENTLEYDTM_EXPORT void bcdtmMemory_free(
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber)
    {
    bcdtmMemory_freeDebug (dtmP, partitionNumber, "", 0);
    }
#else
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT DTMMemPnt bcdtmMemory_allocate(
    BC_DTM_OBJ *dtmP,
    size_t size)
    // Called when the DTM needs to allocate a section of memory.
    // If it is a normal dtm it calls malloc and returns, otherwise it calls the DTMElement Section Allocation method.
    {
    if (!dtmP->DTMAllocationClass)
        return (DTMMemPnt)malloc(size);

    return dtmP->DTMAllocationClass->AllocateMemory(size);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT DTMMemPnt bcdtmMemory_reallocate (
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber,
    size_t size)
    // Called when the DTM needs to reallocate a section of memory.
    // If it is a normal dtm it calls realloc and returns, otherwise it calls the DTMElement Section Reallocation method.
    {
    if (!dtmP->DTMAllocationClass)
        return (DTMMemPnt)realloc((void*)partitionNumber, size);
    return (DTMMemPnt)dtmP->DTMAllocationClass->ReallocateMemory((int)partitionNumber, size);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void bcdtmMemory_free (
    BC_DTM_OBJ *dtmP,
    DTMMemPnt partitionNumber)
    // Called when the DTM needs to free a section of memory.
    // If it is a normal dtm it calls free and returns, otherwise it calls the DTMElement Section Free method.
    {
    if (!dtmP->DTMAllocationClass)
        free((void*)partitionNumber);
    else
        dtmP->DTMAllocationClass->FreeMemory((int)partitionNumber);
    }
#endif

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT void bcdtmMemory_release (BC_DTM_OBJ *dtmP)
    // This is called when the element is being release.
    {
    if(dtmP->DTMAllocationClass)
        {
        dtmP->DTMAllocationClass->Release();
        dtmP->DTMAllocationClass = nullptr;
        }
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmMemory_setMemoryAccess (BC_DTM_OBJ *dtmP, DTMAccessMode accessMode)
    {
    if(dtmP->DTMAllocationClass)
        return dtmP->DTMAllocationClass->SetMemoryAccess(dtmP, accessMode);
    return DTM_SUCCESS;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+--------------------------------------------------------------------*/
BENTLEYDTM_EXPORT IDTMElementMemoryAllocator* bcdtmMemory_getAllocator(
    BC_DTM_OBJ *dtmP)
    // Called when the DTM needs to free a section of memory.
    // If it is a normal dtm it calls free and returns, otherwise it calls the DTMElement Section Free method.
    {
    return dtmP->DTMAllocationClass;
    }


