/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportLayerToLayer.h $
|    $RCSfile: ImportLayerToLayer.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/11/22 21:58:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/Command/Base.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportLayerToLayerCommand : public ImportCommandMixinBase<ImportLayerToLayerCommand>
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    uint32_t                                m_sourceLayer;
    uint32_t                                m_targetLayer;
public:
    IMPORT_DLLE explicit                ImportLayerToLayerCommand          (uint32_t                                sourceLayer,
                                                                            uint32_t                                targetLayer);


    IMPORT_DLLE virtual                 ~ImportLayerToLayerCommand         ();

    IMPORT_DLLE                         ImportLayerToLayerCommand          (const ImportLayerToLayerCommand&    rhs);

    uint32_t                                GetSourceLayer                     () const;
    uint32_t                                GetTargetLayer                     () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
