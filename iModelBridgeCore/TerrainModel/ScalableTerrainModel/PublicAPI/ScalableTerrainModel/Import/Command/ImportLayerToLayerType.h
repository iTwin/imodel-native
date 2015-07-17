/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Command/ImportLayerToLayerType.h $
|    $RCSfile: ImportLayerToLayerType.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:58:14 $
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
struct ImportLayerToLayerTypeCommand : public ImportCommandMixinBase<ImportLayerToLayerTypeCommand>
    {

    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    UInt                                m_sourceLayer;
    UInt                                m_targetLayer;
    DataTypeFamily                      m_targetType;

public:
    IMPORT_DLLE explicit                ImportLayerToLayerTypeCommand      (UInt                                    sourceLayer,
                                                                            UInt                                    targetLayer,
                                                                            const DataTypeFamily&                   targetType);

    IMPORT_DLLE virtual                 ~ImportLayerToLayerTypeCommand     ();

    IMPORT_DLLE                         ImportLayerToLayerTypeCommand      (const ImportLayerToLayerTypeCommand&    rhs);

    UInt                                GetSourceLayer                     () const;
    UInt                                GetTargetLayer                     () const;
    const DataTypeFamily&               GetTargetType                      () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE