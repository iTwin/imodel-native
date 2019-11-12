/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Command/Base.h>
#include <ScalableTerrainModel/Import/DataTypeFamily.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportLayerTypeToLayerTypeCommand : public ImportCommandMixinBase<ImportLayerTypeToLayerTypeCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    UInt                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
    UInt                                m_targetLayer;
    DataTypeFamily                      m_targetType;

public:
    IMPORT_DLLE explicit                ImportLayerTypeToLayerTypeCommand  (UInt                            sourceLayer,
                                                                            const DataTypeFamily&           sourceType,
                                                                            UInt                            targetLayer,
                                                                            const DataTypeFamily&           targetType);

    IMPORT_DLLE virtual                 ~ImportLayerTypeToLayerTypeCommand ();

    IMPORT_DLLE                         ImportLayerTypeToLayerTypeCommand  (const ImportLayerTypeToLayerTypeCommand& 
                                                                                                            rhs);


    UInt                                GetSourceLayer                     () const;
    const DataTypeFamily&               GetSourceType                      () const;

    UInt                                GetTargetLayer                     () const;
    const DataTypeFamily&               GetTargetType                      () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE