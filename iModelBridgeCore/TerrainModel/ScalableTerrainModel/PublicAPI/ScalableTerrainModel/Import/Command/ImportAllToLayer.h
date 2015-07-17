/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Command/ImportAllToLayer.h $
|    $RCSfile: ImportAllToLayer.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:58:06 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Command/Base.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportAllToLayerCommand : public ImportCommandMixinBase<ImportAllToLayerCommand>
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    UInt                                m_targetLayer;
public:
    IMPORT_DLLE explicit                ImportAllToLayerCommand            (UInt                            targetLayer);



    IMPORT_DLLE virtual                 ~ImportAllToLayerCommand           ();

    IMPORT_DLLE                         ImportAllToLayerCommand            (const ImportAllToLayerCommand&  rhs);

    UInt                                GetTargetLayer                     () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE