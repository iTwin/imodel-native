/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportLayerTypeToLayer.h $
|    $RCSfile: ImportLayerTypeToLayer.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:58:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/Command/Base.h>
#include <ScalableMesh/Import/DataTypeFamily.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportLayerTypeToLayerCommand : public ImportCommandMixinBase<ImportLayerTypeToLayerCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    uint32_t                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
    uint32_t                                m_targetLayer;

public:
    IMPORT_DLLE explicit                ImportLayerTypeToLayerCommand      (uint32_t                            sourceLayer,
                                                                            const DataTypeFamily&           sourceType,
                                                                            uint32_t                            targetLayer);

    IMPORT_DLLE virtual                 ~ImportLayerTypeToLayerCommand     ();

    IMPORT_DLLE                         ImportLayerTypeToLayerCommand      (const ImportLayerTypeToLayerCommand& 
                                                                                                            rhs);

    uint32_t                                GetSourceLayer                     () const;
    const DataTypeFamily&               GetSourceType                      () const;

    uint32_t                                GetTargetLayer                     () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
