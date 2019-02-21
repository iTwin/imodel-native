/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Command/ImportLayerType.h $
|    $RCSfile: ImportLayerType.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:58:17 $
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
struct ImportLayerTypeCommand : public ImportCommandMixinBase<ImportLayerTypeCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    UInt                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
public:
    IMPORT_DLLE explicit                ImportLayerTypeCommand             (UInt                            sourceLayer,
                                                                            const DataTypeFamily&           sourceType);

    IMPORT_DLLE virtual                 ~ImportLayerTypeCommand            ();

    IMPORT_DLLE                         ImportLayerTypeCommand             (const ImportLayerTypeCommand&   rhs);

    UInt                                GetSourceLayer                     () const;
    const DataTypeFamily&               GetSourceType                      () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE