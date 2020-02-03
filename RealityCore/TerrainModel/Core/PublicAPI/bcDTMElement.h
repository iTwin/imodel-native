/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

struct IDTMElementMemoryAllocator
    {
    public:
        virtual void* AllocateMemory (DTMPartition type, int partitionNumber, size_t size) = 0;
        virtual void* ReallocateMemory (DTMPartition type, int partitionNumber, void *pointer, size_t size) = 0;
        virtual void FreeMemory (DTMPartition type, int partitionNumber, void* pointer) = 0;
        virtual int AllocateMemory(size_t size) = 0;
        virtual int ReallocateMemory(int id, size_t size) = 0;
        virtual void FreeMemory(int id) = 0;
        virtual void* GetMemoryPointer(int id) = 0;
        virtual void Release() = 0;
        virtual int SetMemoryAccess (BC_DTM_OBJ* dtm, DTMAccessMode accessMode) = 0;
    };