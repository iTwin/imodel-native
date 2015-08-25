/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Content/ScalableMeshConfig.h $
|    $RCSfile: ScalableMeshConfig.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/22 21:58:02 $
|     $Author: Thomas.Butzbach $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/Config/Content/Base.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScalableMeshConfig : public ContentConfigComponentMixinBase < ScalableMeshConfig >
{
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
    void operator delete [](void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

private:
    // NEEDS_WORK_SM : All config have some data in stuct (in public API) => refactoring : only m_implP
    ScalableMeshData                    m_smData;
    uint32_t                              m_flags;
    void*                               m_implP; // Reserve some space for further use
public:
    IMPORT_DLLE static ClassID          s_GetClassID();

    IMPORT_DLLE explicit                ScalableMeshConfig(const ScalableMeshData&             data);
    IMPORT_DLLE virtual                 ~ScalableMeshConfig();

    IMPORT_DLLE                         ScalableMeshConfig(const ScalableMeshConfig&           rhs);

    const ScalableMeshData&             GetScalableMeshData() const;
};


inline const ScalableMeshData& ScalableMeshConfig::GetScalableMeshData() const
{
    return m_smData;
}


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
