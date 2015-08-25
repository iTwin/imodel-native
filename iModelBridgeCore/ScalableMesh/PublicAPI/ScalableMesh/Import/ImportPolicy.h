/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ImportPolicy.h $
|    $RCSfile: ImportPolicy.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/19 13:49:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
struct MemoryAllocator;
END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct DataTypeFamily;
struct DataType;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeSelectionPolicy : private Unassignable
    {
private:
    virtual TypeSelectionPolicy*            _Clone                     () const = 0;
    virtual const DataType*                 _Select                    (const DataTypeFamily&           typeFamily,
                                                                        const DataType*                 typesBegin,
                                                                        const DataType*                 typesEnd) const = 0;
protected:
    IMPORT_DLLE explicit                    TypeSelectionPolicy        ();
    IMPORT_DLLE                             TypeSelectionPolicy        (const TypeSelectionPolicy&      rhs);
public:
    IMPORT_DLLE virtual                     ~TypeSelectionPolicy       () = 0;

    TypeSelectionPolicy*                    Clone                      () const;
    const DataType*                         Select                     (const DataTypeFamily&           typeFamily,
                                                                        const DataType*                 typesBegin,
                                                                        const DataType*                 typesEnd) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
typedef MemoryAllocator                     MemoryAllocationPolicy;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportPolicy
    {
private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_implP;
public:
    IMPORT_DLLE static const TypeSelectionPolicy&  
                                            GetDefaultSourceTypeSelectionPolicy    ();
    IMPORT_DLLE static const TypeSelectionPolicy&  
                                            GetDefaultTargetTypeSelectionPolicy    ();

    IMPORT_DLLE static const MemoryAllocationPolicy&
                                            GetDefaultMemoryAllocationPolicy       ();

    IMPORT_DLLE explicit                    ImportPolicy                           ();

    IMPORT_DLLE explicit                    ImportPolicy                           (const MemoryAllocationPolicy&   memoryAllocationPolicy);

    IMPORT_DLLE explicit                    ImportPolicy                           (const TypeSelectionPolicy&      sourceTypeSelectPolicy,
                                                                                    const TypeSelectionPolicy&      targetTypeSelectPolicy);

    IMPORT_DLLE explicit                    ImportPolicy                           (const MemoryAllocationPolicy&   memoryAllocationPolicy,
                                                                                    const TypeSelectionPolicy&      sourceTypeSelectPolicy,
                                                                                    const TypeSelectionPolicy&      targetTypeSelectPolicy);


    IMPORT_DLLE                             ~ImportPolicy                          ();

    IMPORT_DLLE                             ImportPolicy                           (const ImportPolicy&             rhs);
    IMPORT_DLLE ImportPolicy&               operator=                              (const ImportPolicy&             rhs);

    const MemoryAllocationPolicy&           GetMemoryAllocationPolicy              () const;

    const TypeSelectionPolicy&              GetSourceTypeSelection                 () const;
    const TypeSelectionPolicy&              GetTargetTypeSelection                 () const;
    };

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
