/*--------------------------------------------------------------------------------------+
|
|     $Source: Memory/MemoryAllocation.cpp $
|    $RCSfile: MemoryAllocation.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/19 13:50:32 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Memory/Allocation.h>


BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator::MemoryAllocator () 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator::MemoryAllocator (const MemoryAllocator&) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator::~MemoryAllocator ()
    {

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void* MemoryAllocator::Allocate (size_t pi_capacity) const
    { 
    return _Allocate(pi_capacity); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void MemoryAllocator::Deallocate (void* pi_memory) const
    { 
    _Deallocate(pi_memory); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator* MemoryAllocator::Clone () const
    { 
    return _Clone (); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultMemoryAllocator::DefaultMemoryAllocator ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultMemoryAllocator::~DefaultMemoryAllocator ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void* DefaultMemoryAllocator::_Allocate (size_t pi_capacity) const
    {
    return new byte[pi_capacity];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultMemoryAllocator::_Deallocate (void* pi_memory) const
    {
    delete [] (byte*)pi_memory;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryAllocator* DefaultMemoryAllocator::_Clone () const
    {
    return new DefaultMemoryAllocator(*this);
    }



END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
