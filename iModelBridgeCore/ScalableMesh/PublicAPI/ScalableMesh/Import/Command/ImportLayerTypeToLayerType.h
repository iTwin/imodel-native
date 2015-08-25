/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportLayerTypeToLayerType.h $
|    $RCSfile: ImportLayerTypeToLayerType.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:58:20 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
