/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PointCloudHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#if defined (NEEDS_WORK_DGNITEM)

/** @cond BENTLEY_SDK_Internal */
#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/IPointCloud.h>
#include <DgnPlatform/DgnHandlers/PointCloudClipHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
#define PointCloudChannels_Is_Point_Visible(bValue)     ( (bValue &  0x07) != 0)  //if one of the first 3 bits On
#define PointCloudChannels_Is_Point_Hidden(bValue)      (!PointCloudChannels_Is_Point_Visible (bValue))
#define PointCloudChannels_Hide_Point(bValue)           (bValue  &= ~0x07)   //turn off first 3 bits
#define PointCloudChannels_Show_Point(bValue)           (bValue  |= 0x01)   //turn on 2 bit which is layer 1
#define PointCloudChannels_Is_Point_Selected(bValue)    (((bValue) & IPointCloudDataQuery::POINT_SELECTED) != 0)
#define PointCloudChannels_Select_Point(bValue)         (bValue  |= IPointCloudDataQuery::POINT_SELECTED)
#define PointCloudChannels_Unselect_Point(bValue)       (bValue  &= ~IPointCloudDataQuery::POINT_SELECTED)

/*=================================================================================**//**
* The collection of all point cloud element in a model
* Note that it will scan all graphic elements in order to find point cloud element.
* Example:
*\code
    DgnModelR model = ...

    for each (ElementRefP pointCloudElmRefP in PointCloudCollection (model))
        {
        ...
        }
\endcode
* @bsiclass                                                     Mathieu.Marchand  06/2011
+===============+===============+===============+===============+===============+======*/
struct  PointCloudCollection
{
private:
    DgnModelR m_model;

public:
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct PointCloudIterator
    {
    friend struct PointCloudCollection;
    private:
        PersistentElementRefListIterator    m_it;
        ElementRefP                         m_elmRef;

        DGNPLATFORM_EXPORT void ToNext ();
        PointCloudIterator (PersistentElementRefList* l){m_elmRef = NULL; m_it = l->begin(); ToNext();}
        PointCloudIterator () {m_elmRef = NULL;}

    public:
        PointCloudIterator& operator++(){++m_it; ToNext(); return *this;}
        bool                operator==(PointCloudIterator const& rhs) const {return m_elmRef == rhs.m_elmRef;}
        bool                operator!=(PointCloudIterator const& rhs) const {return !(*this == rhs);}
        ElementRefP         operator* () const {return m_elmRef;}
    };

    typedef PointCloudIterator    iterator;

    PointCloudCollection (DgnModelR model):m_model(model){}

    PointCloudIterator begin () const {return PointCloudIterator (m_model.GetGraphicElementsP());}
    PointCloudIterator end   () const {return PointCloudIterator ();}

}; // PointCloudCollection

//__PUBLISH_SECTION_START__

/*---------------------------------------------------------------------------------**//**
* Handler for a Point Cloud element
* @ingroup Pointcloud
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudHandler : public BentleyApi::DgnPlatform::DisplayHandler,
//                           public BentleyApi::DgnPlatform::ITransactionHandler, removed in graphite
                           public BentleyApi::DgnPlatform::IPointCloudEdit
{
    DEFINE_T_SUPER(DgnPlatform::DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (PointCloudHandler, DGNPLATFORM_EXPORT)

private:

//__PUBLISH_SECTION_END__
        OvrMatSymbP     GetSymbologyOverride(ElementHandleCR eh, ViewContextR context, bool isHilited);
protected:

//    StatusInt ProcessAttributesChange(XAttributeHandleCR xAttr, ChangeTrackAction action, bool* cantBeUndoneFlag) const; removed in graphite

    // DisplayHandler
    DGNPLATFORM_EXPORT virtual void            _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
    DGNPLATFORM_EXPORT virtual void            _Draw (ElementHandleCR, ViewContextR) override;

    // Handler
    DGNPLATFORM_EXPORT virtual bool            _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;
    DGNPLATFORM_EXPORT virtual void            _GetTypeName (WStringR string, UInt32 desiredLength) override;
    DGNPLATFORM_EXPORT virtual void            _GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength) override;
    DGNPLATFORM_EXPORT virtual StatusInt       _OnTransform (EditElementHandleR, TransformInfoCR) override;
//    DGNPLATFORM_EXPORT virtual void            _OnElementLoaded (ElementHandleCR eh) override; removed in graphite
    DGNPLATFORM_EXPORT virtual ReprojectStatus _OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain) override;

    // ITransactionHandler
//    DGNPLATFORM_EXPORT virtual ITransactionHandlerP    _GetITransactionHandler() override;
//    DGNPLATFORM_EXPORT virtual void                    _OnAdded (ElementHandleP elHandle) override;
//    DGNPLATFORM_EXPORT virtual void                    _OnDeleted (ElementHandleP elHandle) override;
//    DGNPLATFORM_EXPORT virtual void                    _OnXAttributeChanged (XAttributeHandleCR xAttr, Bentley::DgnPlatform::ChangeTrackAction action, bool* cantBeUndoneFlag);
//    DGNPLATFORM_EXPORT virtual void                    _OnUndoRedoXAttributeChange (XAttributeHandleCR xAttr, Bentley::DgnPlatform::ChangeTrackAction action, bool isUndo, Bentley::DgnPlatform::ChangeTrackSource source);
//    DGNPLATFORM_EXPORT virtual void                    _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, Bentley::DgnPlatform::ChangeTrackAction action,
//                                                                    bool isUndo, Bentley::DgnPlatform::ChangeTrackSource source);

//    DGNPLATFORM_EXPORT virtual ITransactionHandler::PreActionStatus _OnAdd (EditElementHandleR eeh);
  
    // IPointCloudQuery
    DGNPLATFORM_EXPORT virtual PointCloudPropertiesPtr     _GetPointCloudProperties(ElementHandleCR eh) const override;
    DGNPLATFORM_EXPORT virtual PointCloudClipPropertiesPtr _GetPointCloudClipProperties(ElementHandleCR eh) const override;
    DGNPLATFORM_EXPORT virtual PointCloudClipReferencePtr  _GetClipReference(ElementHandleCR eh) const;

    // IPointCloudEdit
    DGNPLATFORM_EXPORT virtual StatusInt _SetPointCloudProperties(EditElementHandleR eeh, PointCloudPropertiesCR props) override;
    DGNPLATFORM_EXPORT virtual StatusInt _SetPointCloudClipProperties(EditElementHandleR eeh, PointCloudClipPropertiesCR props) override;
    DGNPLATFORM_EXPORT virtual StatusInt _SetClipReference(EditElementHandleR eeh, PointCloudClipReferenceCR clipRef) override;

public:
    DGNPLATFORM_EXPORT static ElementHandlerId GetElemHandlerId ();
    DGNPLATFORM_EXPORT static StatusInt        RegisterHandlers ();
    
    //!Return the base path used for relative searches. A search path may contain one or more configuration variables, directory paths, and/or file paths. Paths are separated by ';'.
    //! @See DgnDocumentMoniker::Create
    DGNPLATFORM_EXPORT static WString       GetSearchPath(DgnModelP modelRefP);

    DGNPLATFORM_EXPORT static size_t          GetNbPointClouds (DgnModelP modelRefP);

//__PUBLISH_SECTION_START__
public:
    /*---------------------------------------------------------------------------------**//**
    * Create a new point cloud element using the supplied parameters. 
    * @param[out] eeh                       The new element.
    * @param[in]  modelRef                  Model to associate this element with. Required to compute range.
    * @param[in]  pointCloudProperties      Properties to assign to the new point cloud element. Use PointCloudProperties::Create to create these properties. 
    * @param[in]  range                     Range of the element to create. It's important to set the actual range of the point cloud element to create
    *                                       because this method does not open the point cloud file and thus does not read the range from the file.
    * @return                               SUCCESS or ERROR if the range of the element can't be validated.
    * 
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static StatusInt CreatePointCloudElement(EditElementHandleR eeh, DgnModelR modelRef, PointCloudPropertiesCR pointCloudProperties, DRange3d range);

    /*---------------------------------------------------------------------------------**//**
    * Get the set of point cloud elements in the model. 
    * @param[in]  modelR                  Model for which the point clouds set is requested.
    * @return A set of point cloud elementRef.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static T_StdElementRefSet    GetPointCloudElementRefsInModel(DgnModelR modelR);

};


END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif

/** @endcond */
