/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ContentDescriptor.h $
|    $RCSfile: ContentDescriptor.h,v $
|   $Revision: 1.12 $
|       $Date: 2011/11/18 15:51:23 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct TypeDescriptor;
struct ILayerDescriptor;
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
    typedef RefCountedPtr<ILayerDescriptor>                     value_type;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;

    typedef const value_type*                   const_iterator;
    // NEEDS_WORK_SM : easy way create iterator to overite layer, maybe a better way ?
    typedef value_type*                            iterator;

    typedef uint32_t                                LayerID;

    IMPORT_DLLE explicit                        ContentDescriptor                  (const WChar*                      name);
    IMPORT_DLLE explicit                        ContentDescriptor                  (const WChar*                      name,
                                                                                    const RefCountedPtr<ILayerDescriptor>&            layer,
                                                                                    bool                              canRepresent3dData = false);
    IMPORT_DLLE                                 ~ContentDescriptor                 ();

    IMPORT_DLLE                                 ContentDescriptor                  (const ContentDescriptor&            rhs);
    IMPORT_DLLE ContentDescriptor&              operator=                          (const ContentDescriptor&            rhs);

    IMPORT_DLLE Status                          Configure                          (const ContentConfig&                config,
                                                                                    Log&                                log = GetDefaultLog());

    IMPORT_DLLE Status                          Configure                          (const ContentConfig&                config,
                                                                                    const ContentConfigPolicy&          policy,
                                                                                    Log&                                log = GetDefaultLog());

    IMPORT_DLLE void                            Add                                (const RefCountedPtr<ILayerDescriptor>&              layer);
    IMPORT_DLLE void                            push_back(const RefCountedPtr<ILayerDescriptor>&              layer);

    IMPORT_DLLE const WString&               GetName                            () const;
    IMPORT_DLLE const WChar*                 GetNameCStr                        () const;

    IMPORT_DLLE uint32_t                            GetLayerCount                      () const;

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


struct ILayerDescriptor : RefCountedBase
    {

protected:
    virtual const WString&               _GetName() const = 0;

    virtual const bvector<TypeDescriptor>& _GetTypes() const = 0;

    virtual bool                            _HoldsIncompleteTypes() const = 0;

    virtual const DRange3d&                 _GetExtent() const = 0;

    virtual const ScalableMeshData&         _GetScalableMeshData() const = 0;

    virtual const GCS&                      _GetGCS() const = 0;

    virtual const AttachmentRecord&         _GetAttachmentRecord() const = 0;

    virtual const MetadataRecord&           _GetMetadataRecord() const = 0;


    virtual ILayerDescriptor&               _SetScalableMeshData(const ScalableMeshData& data) = 0;

    virtual ILayerDescriptor&         _SetGCS(const GCS& gcs) = 0;

    virtual ILayerDescriptor&               _SetAttachmentRecord(const AttachmentRecord& attachments) = 0;

    virtual ILayerDescriptor&               _SetMetadataRecord(const MetadataRecord& metadata) = 0;

public:

    IMPORT_DLLE const WString&               GetName() const
        {
        return _GetName();
        }

    IMPORT_DLLE const bvector<TypeDescriptor>& GetTypes() const
        {
        return _GetTypes();
        }

    IMPORT_DLLE bool                            HoldsIncompleteTypes() const
        {
        return _HoldsIncompleteTypes();
        }

    IMPORT_DLLE const DRange3d&                 GetExtent() const
        {
        return _GetExtent();
        }

    IMPORT_DLLE const ScalableMeshData&         GetScalableMeshData() const
        {
        return _GetScalableMeshData();
        }

    IMPORT_DLLE const GCS&                      GetGCS() const
        {
        return _GetGCS();
        }

    IMPORT_DLLE const AttachmentRecord&         GetAttachmentRecord() const
        {
        return _GetAttachmentRecord();
        }

    IMPORT_DLLE const MetadataRecord&           GetMetadataRecord() const
        {
        return _GetMetadataRecord();
        }


    IMPORT_DLLE ILayerDescriptor&               SetScalableMeshData(const ScalableMeshData& data)
        {
        return _SetScalableMeshData(data);
        }

    IMPORT_DLLE ILayerDescriptor&               SetAttachmentRecord(const AttachmentRecord& attachments)
        {
        return _SetAttachmentRecord(attachments);
        }

    IMPORT_DLLE ILayerDescriptor&               SetGCS(const GCS& gcs)
        {
        return _SetGCS(gcs);
        }

    IMPORT_DLLE ILayerDescriptor&               SetMetadataRecord(const MetadataRecord& metadata)
        {
        return _SetMetadataRecord(metadata);
        }


    IMPORT_DLLE void               Configure(const ContentConfig& config)
        {
        if (config.GetTypeConfig().IsSet())
            {
            const DataType& type = config.GetTypeConfig().GetType();

            auto foundTypeIt = std::find(GetTypes().begin(), GetTypes().end(), type.GetFamily());

            if (GetTypes().end() == foundTypeIt)
                //throw CustomException(L"Replacement type not found!");
                return;

            if (foundTypeIt->GetType().GetDimensionOrgCount() != type.GetDimensionOrgCount())
                //throw CustomException(L"Mismatching dimension org count!");
                return;

            const_cast<TypeDescriptor&>(*foundTypeIt) = type;
            }
        if (config.GetScalableMeshConfig().IsSet())
            {
            SetScalableMeshData(config.GetScalableMeshConfig().GetScalableMeshData());
            }


        if (config.GetGCSConfig().IsSet())
            {
            if (GetGCS().HasGeoRef() && config.GetGCSConfig().IsExistingPreservedIfGeoreferenced())
                return;

            if (!GetGCS().IsNull() && config.GetGCSConfig().IsExistingPreservedIfLocalCS())
                return;

            if (!GetGCS().HasLocalTransform() || !config.GetGCSConfig().IsPrependedToExistingLocalTransform())
                {
                SetGCS(config.GetGCSConfig().GetGCS());
                return;
                }

            // We suppose that user has already taken care of adapting his gcs to the input units
            // of the existing transform
            GCS newGCS(config.GetGCSConfig().GetGCS());

            if (GCS::S_SUCCESS != newGCS.AppendLocalTransform(GetGCS().GetLocalTransform()))
                //throw CustomException(L"Error creating GCS!");
                return;

            SetGCS(newGCS);
            }
        }

    IMPORT_DLLE static RefCountedPtr<ILayerDescriptor>  CreateLayerDescriptor(const WChar*                        name,
                                                                       const DataTypeSet&                  storedTypes,
                                                                       const GCS&                          rGCS,
                                                                       const DRange3d*                     extentP,
                                                                       const ScalableMeshData&             rData);

    IMPORT_DLLE static RefCountedPtr<ILayerDescriptor>  CreateLayerDescriptor(const ILayerDescriptor& desc);

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
