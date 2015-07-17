/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Memory/Allocation.h $
|    $RCSfile: Allocation.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/07 14:20:31 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Memory/Definitions.h>


BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MemoryAllocator
    {
private:
    MemoryAllocator&                    operator=                          (const MemoryAllocator&);

    virtual void*                       _Allocate                          (size_t              capacity) const = 0;
    virtual void                        _Deallocate                        (void*               memory) const = 0;

    virtual MemoryAllocator*            _Clone                             () const = 0;

protected:
    MEMORY_DLLE explicit                MemoryAllocator                    ();

    MEMORY_DLLE                         MemoryAllocator                    (const MemoryAllocator& 
                                                                                                rhs);

    // NTERAY: Disable public use of operator new?

public:
    MEMORY_DLLE virtual                 ~MemoryAllocator                   () = 0;

    MEMORY_DLLE void*                   Allocate                           (size_t              capacity) const;
    MEMORY_DLLE void                    Deallocate                         (void*               memory) const;

    MEMORY_DLLE MemoryAllocator*        Clone                              () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* NOTE: Not designed to be used as a base class
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultMemoryAllocator : public MemoryAllocator
    {
private:
    // NTERAY: Disable public use of operator new?


    virtual void*                       _Allocate                          (size_t              capacity) const override;
    virtual void                        _Deallocate                        (void*               memory) const override;

    virtual MemoryAllocator*            _Clone                             () const override;
public:
    MEMORY_DLLE explicit                DefaultMemoryAllocator             ();
    MEMORY_DLLE virtual                 ~DefaultMemoryAllocator            ();
    };


END_BENTLEY_MRDTM_MEMORY_NAMESPACE