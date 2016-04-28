/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ImportSequence.h $
|    $RCSfile: ImportSequence.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:58:05 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/DataTypeFamily.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct IImportSequenceVisitor;
struct ImportCommand;
struct ImportCommandBase;

/*---------------------------------------------------------------------------------**//**
* @description  
*
* NOTE : - Not designed to be used as a base class.
*        - This is a copy on write implementation, so there is no cost copying instances
*          of this object.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportSequence
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;
    ImplPtr                             m_pImpl;

public:
    typedef ImportCommand               value_type;
    typedef const value_type&           const_reference;
    typedef value_type&                 reference;

    IMPORT_DLLE explicit                ImportSequence                 ();
    IMPORT_DLLE                         ~ImportSequence                ();

    IMPORT_DLLE                         ImportSequence                 (const ImportSequence&                   rhs);
    IMPORT_DLLE ImportSequence&         operator=                      (const ImportSequence&                   rhs);

    IMPORT_DLLE void                    push_back                      (const ImportCommand&                    command);
    IMPORT_DLLE void                    push_back                      (const ImportCommandBase&                command);


    IMPORT_DLLE bvector<ImportCommand>&  GetCommands                   () const;

    IMPORT_DLLE bool                    IsEmpty                        () const;
    IMPORT_DLLE size_t                  GetCount                       () const;
   
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportCommand
    {
private:
    typedef const std::type_info*           ClassID;
    typedef SharedPtrTypeTrait<const ImportCommandBase>::type       
                                            BaseCPtr;
public:
    BaseCPtr                                m_basePtr;
    explicit                                ImportCommand                      (const ImportCommandBase&            command);

    IMPORT_DLLE                             ~ImportCommand                     ();

    IMPORT_DLLE                             ImportCommand                      (const ImportCommand&                rhs);    
    IMPORT_DLLE ImportCommand&              operator=                          (const ImportCommand&                rhs);  

    IMPORT_DLLE ClassID                     GetClassID                         () const;



    IMPORT_DLLE uint32_t                                 GetSourceLayer         () const;
    IMPORT_DLLE uint32_t                                 GetTargetLayer() const;

    IMPORT_DLLE const DataTypeFamily&                    GetSourceType()const;
    IMPORT_DLLE const DataTypeFamily&                    GetTargetType()const;

    IMPORT_DLLE bool                                 IsSourceLayerSet()const;
    IMPORT_DLLE bool                                 IsTargetLayerSet()const;

    IMPORT_DLLE bool                                 IsSourceTypeSet()const;
    IMPORT_DLLE bool                                 IsTargetTypeSet()const;


    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
