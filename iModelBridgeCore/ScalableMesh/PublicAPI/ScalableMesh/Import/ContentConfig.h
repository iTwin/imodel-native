/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ContentConfig.h $
|    $RCSfile: ContentConfig.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/18 15:51:20 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Config/Content/All.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

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
    typedef RefCountedPtr<Impl>
                                        ImplPtr;

    ImplPtr                             m_pImpl;

public:


    IMPORT_DLLE explicit                ContentConfig                  ();
    IMPORT_DLLE                         ~ContentConfig                 ();

    IMPORT_DLLE                         ContentConfig                  (const ContentConfig&                    rhs);
    IMPORT_DLLE ContentConfig&          operator=                      (const ContentConfig&                    rhs);


    IMPORT_DLLE const GCSConfig&       GetGCSConfig() const;
    IMPORT_DLLE const TypeConfig&       GetTypeConfig() const;
    IMPORT_DLLE const ScalableMeshConfig&       GetScalableMeshConfig() const;


    IMPORT_DLLE bool      IsEmpty() const
        {
        return !GetGCSConfig().IsSet() && !GetTypeConfig().IsSet() && !GetScalableMeshConfig().IsSet();
        }

    IMPORT_DLLE void       SetGCSConfig(const GCSConfig& gcsConfig);
    IMPORT_DLLE void       SetTypeConfig(const TypeConfig& typeConfig);
    IMPORT_DLLE void       SetScalableMeshConfig(const ScalableMeshConfig& scalableMeshConfig);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigPolicy
    {
    uint32_t                                  m_flags;
    const void*                             m_implP; // Reserved some space for further use.

public:
    IMPORT_DLLE    explicit                 ContentConfigPolicy                    ();
    IMPORT_DLLE                             ~ContentConfigPolicy                   ();
    IMPORT_DLLE                             ContentConfigPolicy                    (const ContentConfigPolicy&       rhs);
    IMPORT_DLLE    ContentConfigPolicy&     operator=                              (const ContentConfigPolicy&       rhs);
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
