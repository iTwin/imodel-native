/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ContentDescriptor.h $
|    $RCSfile: ContentDescriptor.h,v $
|   $Revision: 1.12 $
|       $Date: 2011/11/18 15:51:23 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct TypeDescriptor;
struct LayerDescriptor;
struct SourceRef;
struct ContentConfig;
struct LayerConfig;
struct ContentDescriptorImpl;
struct LayerDescriptorImpl;
struct AttachmentRecord;
struct MetadataRecord;
struct ContentConfigPolicy;


/*---------------------------------------------------------------------------------**//**
* @description  Describes the content of a dtm source. Includes all layer descriptors for
*               this file.
*
* @see LayerDescriptor
*
* 
*
* NOTE:     - Not designed to be a base class. 
*           - Optimized with copy on write.
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentDescriptor
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR, 
        S_QTY,
        };

private:
    SharedPtrTypeTrait<ContentDescriptorImpl>::type 
                                                m_pImpl;

public:
    typedef LayerDescriptor                     value_type;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;

    typedef const value_type*                   const_iterator;
    // NEEDS_WORK_SM : easy way create iterator to overite layer, maybe a better way ?
    typedef value_type*                            iterator;

    typedef UInt                                LayerID;

    IMPORT_DLLE explicit                        ContentDescriptor                  (const WChar*                      name);
    IMPORT_DLLE explicit                        ContentDescriptor                  (const WChar*                      name,
                                                                                    const LayerDescriptor&            layer, 
                                                                                    bool                              canRepresent3dData = false);
    IMPORT_DLLE                                 ~ContentDescriptor                 ();

    IMPORT_DLLE                                 ContentDescriptor                  (const ContentDescriptor&            rhs);
    IMPORT_DLLE ContentDescriptor&              operator=                          (const ContentDescriptor&            rhs);

    IMPORT_DLLE Status                          Configure                          (const ContentConfig&                config,
                                                                                    Log&                                log = GetDefaultLog());

    IMPORT_DLLE Status                          Configure                          (const ContentConfig&                config,
                                                                                    const ContentConfigPolicy&          policy,
                                                                                    Log&                                log = GetDefaultLog());

    IMPORT_DLLE void                            Add                                (const LayerDescriptor&              layer);
    IMPORT_DLLE void                            push_back                          (const LayerDescriptor&              layer);

    IMPORT_DLLE const WString&               GetName                            () const;
    IMPORT_DLLE const WChar*                 GetNameCStr                        () const;

    IMPORT_DLLE UInt                            GetLayerCount                      () const;

    IMPORT_DLLE const_iterator                  LayersBegin                        () const;
    IMPORT_DLLE const_iterator                  LayersEnd                          () const;

    IMPORT_DLLE const_iterator                  FindLayerFor                       (LayerID                             layerID) const;
    IMPORT_DLLE iterator                        FindLayerFor                       (LayerID layerID);
    IMPORT_DLLE LayerID                         GetLayerIDFor                      (const_iterator                      iter) const;

    IMPORT_DLLE bool                            HoldsIncompleteTypes               () const;

    IMPORT_DLLE bool                            IsPod                              () const;

    IMPORT_DLLE bool                            CanRepresent3dData () const;

    IMPORT_DLLE const MetadataRecord&           GetMetadataRecord                  () const;
    IMPORT_DLLE MetadataRecord&                 EditMetadataRecord                 ();


    // TDORAY: Add an HasExtendedAttachments (for user to know whether he want to fetch it or not)
    // TDORAY: Add an HasExtendedMetadata (for user to know whether he want to fetch it or not)
    };


/*---------------------------------------------------------------------------------**//**
* @description  Describes the content of a layer of dtm data.
* NOTE:     - Not designed to be a base class.
*           - Optimized with copy on write.
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerDescriptor
    {
private:
    friend struct                               ContentDescriptor;

    struct                                      Impl;
    SharedPtrTypeTrait<LayerDescriptorImpl>::type
                                                m_pImpl;

public:
    typedef TypeDescriptor                      value_type;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;

    typedef const value_type*                   const_iterator;


    IMPORT_DLLE explicit                        LayerDescriptor                    (const WChar*                        name,
                                                                                    const DataTypeSet&                  storedTypes,
                                                                                    const GCS&                          rGCS,
                                                                                    const DRange3d*                     extentP,
                                                                                    const ScalableMeshData&             rData);


    IMPORT_DLLE                                 ~LayerDescriptor                   ();

    IMPORT_DLLE                                 LayerDescriptor                    (const LayerDescriptor&              right);
    IMPORT_DLLE LayerDescriptor&                operator=                          (const LayerDescriptor&              right);

    IMPORT_DLLE const WString&               GetName                            () const;
    IMPORT_DLLE const WChar*                 GetNameCStr                        () const;

    IMPORT_DLLE UInt                            GetTypeCount                       () const;

    IMPORT_DLLE const_iterator                  TypesBegin                         () const;
    IMPORT_DLLE const_iterator                  TypesEnd                           () const;

    IMPORT_DLLE const_iterator                  FindTypeFor                        (const DataTypeFamily&               typeFamily) const;


    IMPORT_DLLE bool                            HoldsIncompleteTypes               () const;

    IMPORT_DLLE bool                            HasExtent                          () const;
    IMPORT_DLLE const DRange3d&                 GetExtent                          () const;

    IMPORT_DLLE void                            SetScalableMeshData (ScalableMeshData& data);
    IMPORT_DLLE const ScalableMeshData&         GetScalableMeshData() const;

    IMPORT_DLLE const GCS&                      GetGCS                             () const;

    IMPORT_DLLE const AttachmentRecord&         GetAttachmentRecord                () const;
    IMPORT_DLLE AttachmentRecord&               EditAttachmentRecord               ();

    IMPORT_DLLE const MetadataRecord&           GetMetadataRecord                  () const;
    IMPORT_DLLE MetadataRecord&                 EditMetadataRecord                 ();


    // TDORAY: Add an HasExtendedAttachments (for user to know whether he want to fetch it or not)
    // TDORAY: Add an HasExtendedMetadata (for user to know whether he want to fetch it or not)
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeDescriptor
    {
private:
    friend struct                               LayerDescriptor;

    DataType                                    m_type;
    const void*                                 m_implP; // Reserve some space for further use
public:
    // Implicitly convert from DataType
    IMPORT_DLLE                                 TypeDescriptor                     (const DataType&                     type);


    IMPORT_DLLE                                 ~TypeDescriptor                    ();

    IMPORT_DLLE                                 TypeDescriptor                     (const TypeDescriptor&               rhs);
    IMPORT_DLLE TypeDescriptor&                 operator=                          (const TypeDescriptor&               rhs);
    IMPORT_DLLE TypeDescriptor&                 operator=                          (const DataType&                     type);

    // Implicitly convert to data type
                                                operator DataType                  () const; 

    const DataType&                             GetType                            () const;

    bool                                        operator<                          (const TypeDescriptor&               rhs) const;
    bool                                        operator==                         (const TypeDescriptor&               rhs) const;
    };

bool                                            operator==                         (const TypeDescriptor&               lhs,
                                                                                    const DataType&                     rhs);
bool                                            operator==                         (const DataType&                     lhs,
                                                                                    const TypeDescriptor&               rhs);

bool                                            operator==                         (const TypeDescriptor&               lhs,
                                                                                    const DataTypeFamily&               rhs);
bool                                            operator==                         (const DataTypeFamily&               lhs,
                                                                                    const TypeDescriptor&               rhs);


bool                                            operator<                          (const TypeDescriptor&               lhs,
                                                                                    const DataType&                     rhs);
bool                                            operator<                          (const DataType&                     lhs,
                                                                                    const TypeDescriptor&               rhs);

bool                                            operator<                          (const TypeDescriptor&               lhs,
                                                                                    const DataTypeFamily&               rhs);
bool                                            operator<                          (const DataTypeFamily&               lhs,
                                                                                    const TypeDescriptor&               rhs);






#include <ScalableMesh/Import/ContentDescriptor.hpp>


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
