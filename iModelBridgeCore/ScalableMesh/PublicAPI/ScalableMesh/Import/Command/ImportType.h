/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/ImportType.h $
|    $RCSfile: ImportType.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:58:23 $
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
struct ImportTypeCommand : public ImportCommandMixinBase<ImportTypeCommand>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    DataTypeFamily                      m_sourceType;

public:
    IMPORT_DLLE explicit                ImportTypeCommand                  (const DataTypeFamily&                   sourceType);
    IMPORT_DLLE virtual                 ~ImportTypeCommand                 ();
    
    IMPORT_DLLE                         ImportTypeCommand                  (const ImportTypeCommand&                rhs);


    const DataTypeFamily&               GetSourceType                      () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
