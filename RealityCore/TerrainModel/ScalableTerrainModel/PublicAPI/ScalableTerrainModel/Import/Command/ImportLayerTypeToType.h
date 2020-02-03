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
struct ImportLayerTypeToTypeCommand : public ImportCommandMixinBase<ImportLayerTypeToTypeCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    UInt                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
    DataTypeFamily                      m_targetType;

public:
    IMPORT_DLLE explicit                ImportLayerTypeToTypeCommand       (UInt                            sourceLayer,
                                                                            const DataTypeFamily&           sourceType,
                                                                            const DataTypeFamily&           targetType);

    IMPORT_DLLE virtual                 ~ImportLayerTypeToTypeCommand      ();

    IMPORT_DLLE                         ImportLayerTypeToTypeCommand       (const ImportLayerTypeToTypeCommand& 
                                                                                                            rhs);

    UInt                                GetSourceLayer                     () const;
    const DataTypeFamily&               GetSourceType                      () const;

    const DataTypeFamily&               GetTargetType                      () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE