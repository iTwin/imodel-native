/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/ContentConfig.h $
|    $RCSfile: ContentConfig.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/18 15:51:20 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ILayerConfigVisitor;
struct IContentConfigVisitor;
struct ContentConfigComponent;
struct ContentConfigComponentBase;


/*---------------------------------------------------------------------------------**//**
* @description  
*
* NOTE : - Not designed to be used as a base class.
*        - This is a copy on write implementation, so there is no cost copying instances
*          of this object.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfig
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;
    ImplPtr                             m_pImpl;

public:
    typedef ContentConfigComponent      value_type;
    typedef const value_type&           const_reference;
    typedef value_type&                 reference;

    typedef const std::type_info*       ComponentClassID;

    IMPORT_DLLE explicit                ContentConfig                  ();
    IMPORT_DLLE                         ~ContentConfig                 ();

    IMPORT_DLLE                         ContentConfig                  (const ContentConfig&                    rhs);
    IMPORT_DLLE ContentConfig&          operator=                      (const ContentConfig&                    rhs);

    IMPORT_DLLE void                    push_back                      (const ContentConfigComponent&           config);
    IMPORT_DLLE void                    push_back                      (const ContentConfigComponentBase&       config);

    IMPORT_DLLE void                    Accept                         (IContentConfigVisitor&                   visitor) const;

    IMPORT_DLLE bool                    IsEmpty                        () const;
    IMPORT_DLLE size_t                  GetCount                       () const;

    IMPORT_DLLE void                    RemoveAllOfType                (ComponentClassID                        classID);
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigComponent
    {
private:
    typedef const std::type_info*           ClassID;
    typedef SharedPtrTypeTrait<const ContentConfigComponentBase>::type       
                                            BaseCPtr;


    BaseCPtr                                m_basePtr;
    ClassID                                 m_classID;

public:
    explicit                                ContentConfigComponent             (const ContentConfigComponentBase&           config);

    IMPORT_DLLE                             ~ContentConfigComponent            ();

    IMPORT_DLLE                             ContentConfigComponent             (const ContentConfigComponent&               rhs);    
    IMPORT_DLLE ContentConfigComponent&     operator=                          (const ContentConfigComponent&               rhs);  

    ClassID                                 GetClassID                         () const { return m_classID; }

    IMPORT_DLLE void                        Accept                             (IContentConfigVisitor&                       visitor) const;
    IMPORT_DLLE void                        Accept                             (ILayerConfigVisitor&                         visitor) const;


    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigPolicy
    {
    UInt32                                  m_flags;
    const void*                             m_implP; // Reserved some space for further use.

public:
    IMPORT_DLLE    explicit                 ContentConfigPolicy                    ();
    IMPORT_DLLE                             ~ContentConfigPolicy                   ();
    IMPORT_DLLE                             ContentConfigPolicy                    (const ContentConfigPolicy&       rhs);
    IMPORT_DLLE    ContentConfigPolicy&     operator=                              (const ContentConfigPolicy&       rhs);
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE