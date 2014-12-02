/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IPointCloud.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

DGNPLATFORM_TYPEDEFS(PointCloudProperties)
DGNPLATFORM_TYPEDEFS(PointCloudClipProperties)

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS(PointCloudClipReference)
DGNPLATFORM_REF_COUNTED_PTR(PointCloudClipReference)
DGNPLATFORM_REF_COUNTED_PTR(UnknownAttributeData)

// Forward declarations
BEGIN_BENTLEY_POINTCLOUD_NAMESPACE
struct PointCloudGcsFacility;
END_BENTLEY_POINTCLOUD_NAMESPACE

#define STORAGE_SCHEMETYPE L"PTSS://"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
//NEVER change this enum since they are persistent in the dgn.
enum PointCloudXAttributesMinorId
    {
    PointCloudMinorId_Handler                    =  1,       
    PointCloudMinorId_Attributes                 =  2,       
    PointCloudMinorId_ClipAttributes             =  3,       
    PointCloudMinorId_ViewSettings               =  4,       
    PointCloudMinorId_ClassificationViewSettings =  5,
    PointCloudMinorId_IModelPublishingSource     =  6,
    PointCloudMinorId_ClipReference              =  7,
    PointCloudMinorId_ClipBoxAtribute            =  8,
    PointCloudMinorId_ClipPolygonAttribute       =  9,
    PointCloudMinorId_PointCloudClipElement      = 10,
    PointCloudMinorId_PCDSListHandler             = 13, 
    PointCloudMinorId_PCDSXAttId                  = 14, 
    PointCloudMinorId_PCClassifXAttId             = 17, 
    PointCloudMinorId_ChannelFileXAttributeHandler  = 18, 
    PointCloudMinorId_ClashAttributes               = 19, 
    };
//__PUBLISH_SECTION_START__


/// @addtogroup Pointcloud
/// @beginGroup
typedef RefCountedPtr<PointCloudProperties>             PointCloudPropertiesPtr;
typedef RefCountedPtr<PointCloudClipProperties>         PointCloudClipPropertiesPtr;

/*=================================================================================**//**
* Provides methods for inspecting Point Cloud properties.
* @bsiclass                                                    Mathieu.Marchand  06/2011
+===============+===============+===============+===============+===============+======*/
struct PointCloudProperties: public RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct BentleyApi::PointCloud::PointCloudGcsFacility;

private:

    typedef union
        {
        UInt32 s;
        struct 
            {
            UInt32     ignoreLocate:1;
            UInt32     unlockedGeoRef:1;
            UInt32     reserved:30;
            } b;
        } AttributeFlags;

    typedef bvector<UnknownAttributeDataPtr> UnknownAttributes;

    Transform               m_transform;        // In UOR coord.
    AttributeFlags          m_flags;
    WString                 m_description;      // Description of the Point Cloud file/attachment
    Byte                    m_viewStates;
    float                   m_viewDensity;      // [0,1], default 1.0.
    double                  m_uorPerMeter;
    DPoint3d                m_globalOrigin;
    UInt32                  m_reserved[4];
    WString                 m_wktString;        // Well Known Text string representing the spatial reference
    UnknownAttributes       m_unknownAttributes;

    PointCloudProperties();
    PointCloudProperties(PointCloudPropertiesCR);

    void Clear(DgnModelCP modelP);

private:
    //! Sets the point cloud spatial reference.
    DGNPLATFORM_EXPORT void      SetSpatialReferenceWkt(WStringCR wktString);

public:
    //! Non-published method used to recreate already created point cloud element.
    static PointCloudPropertiesPtr Create(ElementHandleCR pointCloudEh);

    //! Non-published method used to initialize point cloud element.
    DGNPLATFORM_EXPORT StatusInt InitFromRawData(DataInternalizer& dataInternalizer, DgnModelP modelRef);

    //! Load attachment information from the provided ElementHandle into this PointCloudProperties.
    DGNPLATFORM_EXPORT StatusInt LoadFromElement(ElementHandleCR eh);
    
    //! Store this PointCloudProperties information to the provided EditElemHandle.
    DGNPLATFORM_EXPORT StatusInt StoreToElement(EditElementHandleR eeh) const;

    //! Compute point cloud transformation using global origin, transformation matrix, UOR per meter
    DGNPLATFORM_EXPORT Transform ComputeCloudTransform() const;

    //! Return the point cloud spatial reference.
    DGNPLATFORM_EXPORT WStringCR GetSpatialReferenceWkt() const;

    //! test if the point cloud has spatial reference.
    DGNPLATFORM_EXPORT bool      HasSpatialReferenceWkt() const;

    //! Set the transformation matrix.
    DGNPLATFORM_EXPORT void SetTransform (TransformCR trans);

//__PUBLISH_SECTION_START__
public:
    //! Creates an instance of a PointCloudProperties.
    //! @return     An instance of a PointCloudProperties.
    DGNPLATFORM_EXPORT static PointCloudPropertiesPtr Create();

    //! Creates an instance of a PointCloudProperties from an existing persistent element.
    //! @param[in]      eRef    The element from which the instance of a PointCloudProperties is created.
    //! @return         An instance of a PointCloudProperties.
    DGNPLATFORM_EXPORT static PointCloudPropertiesPtr Create(ElementRefP eRef);

    //! Gets the transformation matrix. 
    //! @return         A transformation matrix.
    DGNPLATFORM_EXPORT TransformCR GetTransform () const;

    //! Indicates if the point cloud is locatable.
    //! @return         true if the point cloud is locatable; false otherwise.
    DGNPLATFORM_EXPORT bool GetLocate () const;

    //! Sets locate state.
    //! @param[in]      isOn    If true, sets the point cloud as locatable.
    DGNPLATFORM_EXPORT void SetLocate (bool isOn);

    //! Returns Indicates if the point cloud's geoReference can be changed.
    //! @return         true if the point cloud's geoReference can be changed; false otherwise.
    DGNPLATFORM_EXPORT bool GetLockedGeoReference () const;

    //! Sets georeference lock state.
    //! @param[in]      isLocked    If true, locks the point cloud's geoReference.
    DGNPLATFORM_EXPORT void SetLockedGeoReference (bool isLocked);

    //! Gets the description of the point cloud.
    //! @return         Description of the point cloud.
    DGNPLATFORM_EXPORT WStringCR GetDescription () const;

    //! Sets the description of the point cloud.
    //! @param[in]      description     Description of the point cloud.
    DGNPLATFORM_EXPORT void SetDescription (WStringCR description);

    //! Indicates if the point cloud is displayed in a specific view.
    //! @param[in] viewNumber   The view number to query [0..7].
    //! @return    true if the point cloud is displayed in the specified view.
    DGNPLATFORM_EXPORT bool GetViewState(int viewNumber) const;

    //! Sets if the point cloud is displayed in a specific view.
    //! @param[in]  state       Set to true to display in the view.
    //! @param[in]  viewNumber  The view number to query [0..7]. 
    DGNPLATFORM_EXPORT void SetViewState(int viewNumber, bool state); 

    //! Gets the density of a point cloud. This represents the density of points displayed for this point cloud.
    //! @return    The density (a float value between 0.0 and 1.0).
    DGNPLATFORM_EXPORT float GetViewDensity() const;

    //! Sets the density of a point cloud. This represents the density of points displayed for this point cloud. Default is 1.0.
    //! @param[in]  density  The view density expressed as percentage (a float value between 0.0 and 1.0).
    DGNPLATFORM_EXPORT void SetViewDensity(float density);

    //! Gets scale factor between points coordinates and UOR.
    //! @return    UOR per meter.
    DGNPLATFORM_EXPORT double GetUorPerMeter() const;

    //! Sets scale factor between points coordinates and UOR.
    //! @param[in]  val     UOR per meter.
    DGNPLATFORM_EXPORT void SetUorPerMeter(double val);

    //! Gets translation factor between points coordinates and UOR.
    //! @return    Global origin.
    DGNPLATFORM_EXPORT DPoint3dCR GetGlobalOrigin() const;

    //! Sets translation factor between points coordinates and UOR.
    //! @param[in]  val     Global origin.
    DGNPLATFORM_EXPORT void SetGlobalOrigin(DPoint3dCR val);
};

/*---------------------------------------------------------------------------------**//**
* Interface to query persistent information about a point cloud element.
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudQuery
{
//__PUBLISH_SECTION_END__
protected:
    virtual PointCloudPropertiesPtr     _GetPointCloudProperties(ElementHandleCR eh) const = 0;
    virtual PointCloudClipPropertiesPtr _GetPointCloudClipProperties(ElementHandleCR eh) const = 0;
    virtual PointCloudClipReferencePtr  _GetClipReference(ElementHandleCR eh) const = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Gets the PointCloudProperties associated to the provided element.
    //! @param[in]  eh  Element from which the PointCloudProperties are extracted.
    //! @return    Point cloud properties.
    DGNPLATFORM_EXPORT PointCloudPropertiesPtr      GetPointCloudProperties(ElementHandleCR eh) const;

    //! Gets the  PointCloudClipProperties associated to the provided element.
    //! @param[in]  eh  Element from which the PointCloudClipProperties are extracted.
    //! @return    Point cloud clip properties.
    DGNPLATFORM_EXPORT PointCloudClipPropertiesPtr  GetPointCloudClipProperties(ElementHandleCR eh) const;

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT PointCloudClipReferencePtr   GetClipReference(ElementHandleCR eh) const;
//__PUBLISH_SECTION_START__

}; // IPointCloudQuery

/*---------------------------------------------------------------------------------**//**
* Interface to edit persistent information about a point cloud element.
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudEdit : public IPointCloudQuery
{
//__PUBLISH_SECTION_END__
protected:
    virtual StatusInt _SetPointCloudProperties(EditElementHandleR eeh, PointCloudPropertiesCR props) = 0;
    virtual StatusInt _SetPointCloudClipProperties(EditElementHandleR eeh, PointCloudClipPropertiesCR props) = 0;
    virtual StatusInt _SetClipReference(EditElementHandleR eh, PointCloudClipReferenceCR clipRef) = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Sets the PointCloudProperties on an element.
    //! @param[in]  eeh     Element on which the PointCloudProperties are set.
    //! @param[in]  props   PointCloudProperties to set on the element.
    //! @return     SUCCESS or ERROR.
    DGNPLATFORM_EXPORT StatusInt SetPointCloudProperties(EditElementHandleR eeh, PointCloudPropertiesCR props);

    //! Sets the PointCloudClipProperties on an element.
    //! @param[in]  eeh     Element on which the PointCloudClipProperties are set.
    //! @param[in]  props   PointCloudClipProperties to set on the element.
    //! @return     SUCCESS or ERROR.
    DGNPLATFORM_EXPORT StatusInt SetPointCloudClipProperties(EditElementHandleR eeh, PointCloudClipPropertiesCR props);

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT StatusInt SetClipReference(EditElementHandleR eh, PointCloudClipReferenceCR clipRef);
//__PUBLISH_SECTION_START__

}; // IPointCloudEdit

/// @endGroup


//__PUBLISH_SECTION_END__
/*---------------------------------------------------------------------------------**//**
* PointCloudElementFacility
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudElementFacility
    {
    public:
        DGNPLATFORM_EXPORT static void CreateBox(DPoint3d box[8], DRange3d const& range);
        DGNPLATFORM_EXPORT static void CreateBox(DPoint3d box[8], DVector3d const& range);


        DGNPLATFORM_EXPORT static void GetRangeBox(DPoint3d box[8], ElementHandleCR eh, bool clipped=true);
        DGNPLATFORM_EXPORT static void DrawBoundingBox(ViewContextR context, ElementHandleCR eh);
        DGNPLATFORM_EXPORT static void DrawBoundingBoxFaces(ViewContextR context, ElementHandleCR eh, bool clipped=true);

    };
//__PUBLISH_SECTION_START__


//__PUBLISH_SECTION_END__
/*---------------------------------------------------------------------------------**//**
* MakeDgnModelWritable                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
struct MakeDgnModelWritable
    {
    public:
        DGNPLATFORM_EXPORT MakeDgnModelWritable(DgnModelR cacheP);
        DGNPLATFORM_EXPORT ~MakeDgnModelWritable();

    private:
        MakeDgnModelWritable(MakeDgnModelWritable const&);  // disabled
        MakeDgnModelWritable& operator=(MakeDgnModelWritable const&);   // disabled

        DgnModelR   m_dgnCache;
        bool        m_wasReadOnly;
    };
//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
