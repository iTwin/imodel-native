/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/ImportConfig.h $
|    $RCSfile: ImportConfig.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:58:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct IImportConfigVisitor;

struct ImportConfigComponent;
struct ImportConfigComponentBase;

/*---------------------------------------------------------------------------------**//**
* @description  Object used to store the configuration specifying how to import dtm
*               data. 
*               
* NOTE : - Not designed to be used as a base class.
*        - This is a copy on write implementation, so there is no cost copying instances
*          of this object.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfig
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;

    ImplPtr                             m_pImpl;

public:
    typedef ImportConfigComponent       value_type;
    typedef const value_type&           const_reference;
    typedef value_type&                 reference;

    typedef const std::type_info*       ComponentClassID;

    IMPORT_DLLE explicit                ImportConfig                   ();
    IMPORT_DLLE                         ~ImportConfig                  ();

    IMPORT_DLLE                         ImportConfig                   (const ImportConfig&                     rhs);
    IMPORT_DLLE ImportConfig&           operator=                      (const ImportConfig&                     rhs);

    IMPORT_DLLE void                    push_back                      (const ImportConfigComponent&            config);
    IMPORT_DLLE void                    push_back                      (const ImportConfigComponentBase&        config);

    IMPORT_DLLE void                    Accept                         (IImportConfigVisitor&                   visitor) const;

    IMPORT_DLLE void                    RemoveAllOfType                (ComponentClassID                        classID);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*                        
* @see ImportConfig
* @see ImportConfigComponentBase
*
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfigComponent
    {
private:
    typedef const std::type_info*       ClassID;
    typedef SharedPtrTypeTrait<const ImportConfigComponentBase>::type       
                                        BaseCPtr;
    BaseCPtr                            m_basePtr;
    ClassID                             m_classID;
public:
    explicit                            ImportConfigComponent          (const ImportConfigComponentBase&        config);

    IMPORT_DLLE                         ~ImportConfigComponent         ();

    IMPORT_DLLE                         ImportConfigComponent          (const ImportConfigComponent&            rhs);    
    IMPORT_DLLE ImportConfigComponent&  operator=                      (const ImportConfigComponent&            rhs);  

    ClassID                             GetClassID                     () const { return m_classID; }

    IMPORT_DLLE void                    Accept                         (IImportConfigVisitor&                   visitor) const;

    };



END_BENTLEY_MRDTM_IMPORT_NAMESPACE