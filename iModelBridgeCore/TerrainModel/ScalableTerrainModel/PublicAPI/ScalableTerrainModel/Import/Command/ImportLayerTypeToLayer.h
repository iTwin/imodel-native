/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Command/ImportLayerTypeToLayer.h $
|    $RCSfile: ImportLayerTypeToLayer.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:58:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Command/Base.h>
#include <ScalableTerrainModel/Import/DataTypeFamily.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

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
    UInt                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
    UInt                                m_targetLayer;

public:
    IMPORT_DLLE explicit                ImportLayerTypeToLayerCommand      (UInt                            sourceLayer,
                                                                            const DataTypeFamily&           sourceType,
                                                                            UInt                            targetLayer);

    IMPORT_DLLE virtual                 ~ImportLayerTypeToLayerCommand     ();

    IMPORT_DLLE                         ImportLayerTypeToLayerCommand      (const ImportLayerTypeToLayerCommand& 
                                                                                                            rhs);

    UInt                                GetSourceLayer                     () const;
    const DataTypeFamily&               GetSourceType                      () const;

    UInt                                GetTargetLayer                     () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE