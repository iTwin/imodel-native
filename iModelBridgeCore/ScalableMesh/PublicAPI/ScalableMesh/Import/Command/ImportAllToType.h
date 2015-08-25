/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportAllToType.h $
|    $RCSfile: ImportAllToType.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:58:09 $
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
struct ImportAllToTypeCommand : public ImportCommandMixinBase<ImportAllToTypeCommand>
    {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    DataTypeFamily                      m_targetType;

public:
    IMPORT_DLLE explicit                ImportAllToTypeCommand             (const DataTypeFamily&                   targetType);

    IMPORT_DLLE virtual                 ~ImportAllToTypeCommand            ();

    IMPORT_DLLE                         ImportAllToTypeCommand             (const ImportAllToTypeCommand&           rhs);

    const DataTypeFamily&               GetTargetType                      () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
