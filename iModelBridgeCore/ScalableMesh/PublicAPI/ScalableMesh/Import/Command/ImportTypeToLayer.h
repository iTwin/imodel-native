/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportTypeToLayer.h $
|    $RCSfile: ImportTypeToLayer.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/22 21:58:25 $
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
struct ImportTypeToLayerCommand : public ImportCommandMixinBase<ImportTypeToLayerCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    DataTypeFamily                      m_sourceType;
    UInt                                m_targetLayer;

public:
    IMPORT_DLLE explicit                ImportTypeToLayerCommand           (const DataTypeFamily&                   sourceType,
                                                                            UInt                                    targetLayer);
    IMPORT_DLLE virtual                 ~ImportTypeToLayerCommand          ();

    IMPORT_DLLE                         ImportTypeToLayerCommand           (const ImportTypeToLayerCommand&         rhs);

    const DataTypeFamily&               GetSourceType                      () const;
    UInt                                GetTargetLayer                     () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
