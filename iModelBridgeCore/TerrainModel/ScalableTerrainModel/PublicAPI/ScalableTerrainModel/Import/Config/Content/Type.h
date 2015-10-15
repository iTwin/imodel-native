/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Content/Type.h $
|    $RCSfile: Type.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/22 21:58:02 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Config/Content/Base.h>
#include <ScalableTerrainModel/Import/DataType.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConfig : public ContentConfigComponentMixinBase<TypeConfig>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    DataType                            m_type;
    uint32_t                              m_flags;
    void*                               m_implP; // Reserve some space for further use
public:
    IMPORT_DLLE static ClassID          s_GetClassID                       ();

    IMPORT_DLLE explicit                TypeConfig                         (const DataType&             type);
    IMPORT_DLLE virtual                 ~TypeConfig                        ();

    IMPORT_DLLE                         TypeConfig                         (const TypeConfig&           rhs);

    const DataType&                     GetType                            () const;
    };


inline const DataType& TypeConfig::GetType () const
    { return m_type; }


END_BENTLEY_MRDTM_IMPORT_NAMESPACE