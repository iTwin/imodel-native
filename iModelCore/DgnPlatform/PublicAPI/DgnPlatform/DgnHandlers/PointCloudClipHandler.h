/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PointCloudClipHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS(PointCloudClipBase)
DGNPLATFORM_TYPEDEFS(PointCloudClipBox)
DGNPLATFORM_TYPEDEFS(PointCloudClipPolygon)

DGNPLATFORM_REF_COUNTED_PTR(PointCloudClipBase)
DGNPLATFORM_REF_COUNTED_PTR(PointCloudClipBox)
DGNPLATFORM_REF_COUNTED_PTR(PointCloudClipPolygon)

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

struct PointCloudClipReferenceHandler;
#ifdef WIP_VANCOUVER_MERGE // pointcloud
struct PointCloudClipFacility;
struct PointCloudClipElementHandler;
#endif
struct PointCloudHandler;

typedef bvector<DPoint3d>                              PointCloudPolygon;
typedef BentleyApi::DgnPlatform::PointCloudPolygon&       PointCloudPolygonR;
typedef BentleyApi::DgnPlatform::PointCloudPolygon const& PointCloudPolygonCR;

//__PUBLISH_SECTION_START__

/// @addtogroup Pointcloud
/// @beginGroup

/*---------------------------------------------------------------------------------**//**
* Structure used to define clips on point clouds.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct OrientedBox
    {
    private:
        DVec3d      m_xVec;
        DVec3d      m_yVec;
        DVec3d      m_zVec;
        DPoint3d    m_origin;

    public:
        
        DGNPLATFORM_EXPORT OrientedBox ();

        /*---------------------------------------------------------------------------------**//**
        * OrientedBox constructor to use with the clip boundary and mask methods
        * A OrientedBox can only be built using X Y Z directional vectors that are perpendicular to one-another.
        * @param xVec IN    X directional vector from the origin and with a magnitude of the width of the box
        * @param yVec IN    Y directional vector from the origin and with a magnitude of the height of the box  
        * @param zVec IN    Z directional vector from the origin and with a magnitude of the depth of the box
        * @param origin IN  the origin point of the box, can be any of the 8 points    
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT OrientedBox (DVec3d& xVec, DVec3d& yVec, DVec3d& zVec, DPoint3d& origin);

        //!OrientedBox destructor
        DGNPLATFORM_EXPORT ~OrientedBox ();

        //! Get the X vector of this OrientedBox.
        //! @return X vector
        DGNPLATFORM_EXPORT DVec3d const&   GetXVec() const;
        //! Get the Y vector of this OrientedBox.
        //! @return Y vector
        DGNPLATFORM_EXPORT DVec3d const&   GetYVec() const;
        //! Get the Z vector of this OrientedBox.
        //! @return Z vector
        DGNPLATFORM_EXPORT DVec3d const&   GetZVec() const;
        //! Get the origin of this OrientedBox.
        //! @return origin
        DGNPLATFORM_EXPORT DPoint3d const& GetOrigin() const;
/*__PUBLISH_SECTION_END__*/
        DGNPLATFORM_EXPORT void         ApplyTransform(TransformCR trn);
        DGNPLATFORM_EXPORT void         Init(DVec3dCR xVec, DVec3dCR yVec, DVec3dCR zVec, DPoint3dCR origin);
        DGNPLATFORM_EXPORT static void  ComputeCornersFromOrientedBox (DPoint3d corners[8], DgnPlatform::OrientedBox const& clipBox, bool ccwOrder = false);
/*__PUBLISH_SECTION_START__*/
    };

typedef OrientedBox&            OrientedBoxR;
typedef OrientedBox const&      OrientedBoxCR;
typedef bvector<OrientedBox>    OrientedBoxList;
typedef OrientedBoxList&        OrientedBoxListR;
typedef OrientedBoxList const&  OrientedBoxListCR;

/*=================================================================================**//**
* Provides methods for inspecting Point Cloud clip properties.
* @bsiclass                                                    Eric.Paquet  06/2011
+===============+===============+===============+===============+===============+======*/
struct PointCloudClipProperties: public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    PointCloudClipProperties() ;
    ~PointCloudClipProperties();

    void    Clear();

    StatusInt                           AddClipBoundaryPolygon(bvector<DPoint3d> const& polygonPoints);
    StatusInt                           AddClipBoundary(OrientedBox const& clipBox);
    StatusInt                           AddClipMaskPolygon(bvector<DPoint3d> const& polygonPoints);
    StatusInt                           AddClipMask(OrientedBox const& clipBox);

    OrientedBoxList const&              GetClipMask() const;
    bvector<bvector<DPoint3d> > const&  GetClipMaskPolygon() const;

    OrientedBoxList                     m_boundary;
    OrientedBoxList                     m_mask;
    bvector<bvector<DPoint3d> >         m_boundaryPolygon;
    bvector<bvector<DPoint3d> >         m_maskPolygon;

public:
    //! Non-published method used to recreate already created point cloud element.
    static PointCloudClipPropertiesPtr Create(ElementHandleCR rasterEh);

    // NEVER CHANGE THIS ENUM NUMBERS SINCE THEY ARE STORED IN FILE
    enum ClipFormat
        {
        Format_ClipBoxBoundary = 1,
        Format_ClipBoxMask = 2,
        Format_ClipPolygonBoundary=3,
        Format_ClipPolygonMask=4
        };

    static OrientedBox  LoadClipBox(DataInternalizer& dataInternalizer);
    static void         StoreClipBox(OrientedBox const& box, DataExternalizer& dataExternalizer);
    static void         LoadClipPolygon(DataInternalizer& dataInternalizer, bvector<DPoint3d>& polygon);
    static void         StoreClipPolygon(bvector<DPoint3d> const& polygon, DataExternalizer& dataExternalizer);

    StatusInt           LoadFromElement(ElementHandleCR eh);
    StatusInt           LoadFromXAttribute(BentleyApi::DgnPlatform::ElementHandle::XAttributeIter  const& xAttr);
    StatusInt           Load(DataInternalizer& dataInternalizer);
    StatusInt           StoreToElement(EditElementHandleR eeh) const;

//__PUBLISH_SECTION_START__
public:
    //! Create a new PointCloudClipProperties object.
    //! @return     Pointer to a PointCloudClipProperties object
    DGNPLATFORM_EXPORT static PointCloudClipPropertiesPtr   Create();

    //! Get the list of clip boundaries of a point cloud.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip boundary is requested.
    //! @param[out] clipBox     Clip boundary box
    //! @return     ERROR if there is no clip boundary box applied to the point cloud. SUCCESS otherwise.
    DGNPLATFORM_EXPORT StatusInt                            GetClipBoundary(PointCloudPropertiesCR props, OrientedBoxR clipBox) const;

    //! Delete all the clip boundaries of a point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            ClearClipBoundary ();

    //! Set the clip boundary of a point cloud after removing any existing clip boundary.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip boundary is set.
    //! @param[in]  clipBox     OrientedBox that defines the clip boundary.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            SetClipBoundary(PointCloudPropertiesCR props, OrientedBoxCR clipBox);

    //! Set the clip boundary of a point cloud after removing any existing clip boundary.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip boundary is set.
    //! @param[in]  polygon     Polygon that defines the clip boundary.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            SetClipBoundary(PointCloudPropertiesCR props, bvector<DPoint3d> const& polygon);

    //! Get the polygon clip boundaries of a point cloud.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip boundary is requested.
    //! @param[out] clipPolygon Clip boundary polygon
    //! @return     ERROR if there is no clip boundary polygon applied to the point cloud. SUCCESS otherwise.
    DGNPLATFORM_EXPORT StatusInt                            GetClipBoundaryPolygon(PointCloudPropertiesCR props, bvector<DPoint3d>& clipPolygon) const;

    //! Delete the polygon clip boundaries of a point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            ClearClipBoundaryPolygon();

    //! Delete the polygon clip masks of a point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            ClearClipMaskPolygon();

    //! Delete all the clip masks of a point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            ClearClipMask ();

    //! Get the list of clip masks of a point cloud.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip masks are requested.
    //! @param[out] clipBoxList List of clip masks of the point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            GetClipMaskList(PointCloudPropertiesCR props, OrientedBoxListR clipBoxList) const;

    //! Get the list of clip polygons of a point cloud.
    //! @param[in]  props           PointCloudProperties of the point cloud for which the clip polygons are requested.
    //! @param[out] clipPolygonList List of clip polygons of the point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            GetClipMaskList(PointCloudPropertiesCR props, bvector<bvector<DPoint3d> >& clipPolygonList) const;

    //! Set the list of clip masks of a point cloud.
    //! @param[in]  props       PointCloudProperties of the point cloud for which the clip masks are set.
    //! @param[in]  clipBoxList List of clip masks to set on the point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            SetClipMaskList (PointCloudPropertiesCR props, OrientedBoxListCR clipBoxList);

    //! Set the list of clip polygons of a point cloud.
    //! @param[in]  props           PointCloudProperties of the point cloud for which the clip polygons are set.
    //! @param[in]  clipPolygonList List of clip polygons to set on the point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            SetClipMaskList (PointCloudPropertiesCR props, bvector<bvector<DPoint3d> > const& clipPolygonList);

    //! Delete all the clip masks and clip boundaries of a point cloud.
    //! @return     always SUCCESS
    DGNPLATFORM_EXPORT StatusInt                            ClearAllClip ();
};  //PointCloudClipProperties

/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* PointCloudClipReference
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClipReference : public RefCountedBase
    {
    friend struct PointCloudClipReferenceHandler;
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    friend struct PointCloudClipElementHandler;
#endif
    friend struct PointCloudHandler;

    public:
        typedef bvector<BentleyApi::DgnPlatform::PersistentElementPath>  PersistentElementPathCollection;
        typedef PersistentElementPathCollection const&                PersistentElementPathCollectionCR;
        typedef bvector<BentleyApi::DgnPlatform::ElementHandle>          ElementHandleCollection;
        typedef bvector<UnknownAttributeDataPtr>                      UnknownAttributes;

    private:
        struct AttributeFlags
            {
            AttributeFlags()    {s = 0; b.active = 1;}
            void Clear()        {s = 0; b.active = 1;}

            union
                {
                uint32_t s;
                struct 
                    {
                    uint32_t   active:1;
                    uint32_t   reserved:31;
                    } b;
                };
            };

        uint32_t                        m_id;
        AttributeFlags                  m_flags;
        WString                         m_name;
        PersistentElementPathCollection m_pep;
        UnknownAttributes               m_attrData;

        StatusInt LoadFrom(ElementHandleCR eh);
        StatusInt LoadFrom(BentleyApi::DgnPlatform::ElementHandle::XAttributeIter  const& xAttr);
        StatusInt LoadFrom(XAttributeHandleCR xa);

        StatusInt Load(DataInternalizer& dataInternalizer, uint32_t id);
        StatusInt Store(DataExternalizer& dataExternalizer) const;
        StatusInt ScheduleWriteXAttribute(EditElementHandleR hostElement) const;

        uint32_t GetId() const;
        void   SetId(uint32_t id);


    protected:
        PointCloudClipReference();
        ~PointCloudClipReference();

        /*---------------------------------------------------------------------------------**//**
        * Create clip reference and init its content from on the given XAttributeHandle.
        * @param eh IN XAttributeHandle that contains externalized PointCloudClipReference data.
        * @return    Reference counted pointer to a PointCloudClipReference instance. NULL if an error occured in the creation.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT static PointCloudClipReferencePtr Create(XAttributeHandleCR xa);

    public:
        /*---------------------------------------------------------------------------------**//**
        * Create an empty clip reference
        * @return    Reference counted pointer to a PointCloudClipReference instance. NULL if an error occured in the creation.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT static PointCloudClipReferencePtr Create();

        /*---------------------------------------------------------------------------------**//**
        * Create clip reference and init its content from on the given Elementhandle.
        * @param eh IN PointCloud Element.
        * @return    Reference counted pointer to a PointCloudClipReference instance. NULL if an error occured in the creation.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT static PointCloudClipReferencePtr Create(ElementHandleCR eh);

        /*---------------------------------------------------------------------------------**//**
        * @return    Return a const reference to a colection of PersistentElementPath.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT PersistentElementPathCollection const& GetCollection() const;

        /*---------------------------------------------------------------------------------**//**
        * @return    Return a const reference to a colection of PersistentElementPath.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT ElementHandleCollection GetElementHandleCollection(ElementHandleCR hostElement) const;

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT WStringCR GetName() const;

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      SetName(WStringCR name);

        /*---------------------------------------------------------------------------------**//**
        * Set the state of the clip reference.
        * @param active IN The new state.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      SetActive (bool active);

        /*---------------------------------------------------------------------------------**//**
        * @return    Return the state of the clip reference.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT bool      GetActive () const;

        /*---------------------------------------------------------------------------------**//**
        * Add a clip.
        * @param eh IN Path to a clip element.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      AddElementPath(ElementHandleCR host, ElementHandleCR clipElm);

        /*---------------------------------------------------------------------------------**//**
        * Remove a clip.
        * @param eh IN Path to a clip element.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      RemoveElementPath(ElementHandleCR host, ElementHandleCR clipElm);

        /*---------------------------------------------------------------------------------**//**
        * Remove all clips.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      Clear();

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT StatusInt OnPreprocessCopyRemapIds (ElementHandleCR eh);

//        DGNPLATFORM_EXPORT bool      OnElementIDsChanged (ElementAndModelIdRemappingCR remapTable, ElementHandleCR eh); removed in graphite

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 06/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT void      DisclosePointers (T_StdElementRefSet* refs,DgnModelP  homeModel);

//        DGNPLATFORM_EXPORT bool      RootsChanged (ElementHandleR dependent, bvector<Bentley::DgnPlatform::IDependencyHandler::RootChange> const& rootsChanged, bvector<Bentley::DgnPlatform::XAttributeHandle> const& xAttrsAffected);  removed in graphite
//        DGNPLATFORM_EXPORT void      UndoRedoRootsChanged  (ElementHandleR  dependent, bvector<Bentley::DgnPlatform::IDependencyHandler::RootChange> const& rootsChanged, bvector<Bentley::DgnPlatform::XAttributeHandle> const&  xAttrsAffected) const;  removed in graphite
    
        /*---------------------------------------------------------------------------------**//**
        * @return   XAttributeHandlerId
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::XAttributeHandlerId GetXAttributeHandlerId ();

    }; // PointCloudClipReference


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClipReferenceHandler 
// : public Bentley::DgnPlatform::IXAttributePointerContainerHandler, removed in graphite
//                                        public Bentley::DgnPlatform::IDependencyHandler removed in graphite
    {
    // IXAttributePointerContainerHandler
//    virtual StatusInt           _OnPreprocessCopyRemapIds   (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xa, ElementHandleCR eh) override; removed in graphite
//    virtual void                _OnElementIDsChanged    (XAttributeHandleR xa, ElementAndModelIdRemappingCR remapTable, ElementHandleCR eh) override; removed in graphite
//    virtual void                _DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xa, DgnModelP homeModel) override;
//    virtual IDependencyHandler* _GetIDependencyHandler () override; removed in graphite

    // IDependencyHandler
//    virtual void _OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const& xAttrsAffected) override; removed in graphite
//    virtual void _OnUndoRedoRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const& xAttrsAffected) const override; removed in graphite
    
    static BentleyApi::DgnPlatform::XAttributeHandlerId  GetXAttributeHandlerId ();
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // pointcloud
struct PointCloudClipElementHandler : BentleyApi::DgnPlatform::Handler
//, Bentley::DgnPlatform::ITransactionHandler removed in graphite
    {
    DEFINE_T_SUPER(BentleyApi::DgnPlatform::Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (PointCloudClipElementHandler, DGNPLATFORM_EXPORT)

    private:
        // handler
        virtual void                    _GetTypeName (WStringR string, uint32_t desiredLength) override;

//        virtual void                    _OnElementLoaded (ElementHandleCR eh) override; removed in graphite
//        virtual ITransactionHandlerP    _GetITransactionHandler() override; removed in graphite
//        virtual void                    _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override; removed in graphite
//        virtual void                    _OnAdded (ElementHandleP elHandle) override; removed in graphite
//        virtual void                    _OnDeleted (ElementHandleP elHandle) override; removed in graphite
//        virtual void                    _OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag) override; removed in graphite
        virtual ReprojectStatus         _OnGeoCoordinateReprojection (EditElementHandleR eeh, IGeoCoordinateReprojectionHelper& helper, bool inChain) override;

    public:
        typedef bvector<PointCloudClipPolygonPtr> ClipPolygonPtrCollection;
        typedef bvector<PointCloudClipBoxPtr>     ClipBoxPtrCollection;
 
        ~PointCloudClipElementHandler();

        DGNPLATFORM_EXPORT PointCloudClipBasePtr      CreateBaseClip(ElementHandleCR eh);

        DGNPLATFORM_EXPORT StatusInt                  SetClip(EditElementHandleR eeh, PointCloudClipBoxCR clip);
        DGNPLATFORM_EXPORT StatusInt                  SetClip(EditElementHandleR eeh, PointCloudClipPolygonCR clip);

        DGNPLATFORM_EXPORT ClipPolygonPtrCollection   GetActiveClipPolygons(ElementHandleCR hostElement) const;
        DGNPLATFORM_EXPORT ClipBoxPtrCollection       GetActiveClipBoxes(ElementHandleCR hostElement) const;

        DGNPLATFORM_EXPORT static StatusInt                              CreateElement(EditElementHandleR eeh, DgnModelP modelRef);
        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::DgnClassId  GetHandlerId ();

        DGNPLATFORM_EXPORT static T_StdElementRefSet GetClips(DgnModelR model);

    private:
        void DropClippedRangeAppData (DgnModelP modelrefP);
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClipBase : RefCountedBase
    {
    private:
    protected:
        struct AttributeFlags
            {
            AttributeFlags()    {s = 0; b.active = 1;}
            void Clear()        {s = 0; b.active = 1;}

            union
                {
                uint32_t s;
                struct 
                    {
                    uint32_t   active:1;
                    uint32_t   outside:1;
                    uint32_t   reserved:30;
                    } b;
                };
            };

        PointCloudClipBase();
        virtual ~PointCloudClipBase();

        virtual void                    _Clear() = 0;
        virtual StatusInt               _OnTransform (TransformCR trn) = 0;
        virtual DRange3d                _GetRange() const = 0;
        virtual void                    _SetActive (bool active) = 0;
        virtual bool                    _GetActive () const = 0;
        virtual void                    _SetOutside(bool outside) = 0;
        virtual bool                    _GetOutside() const = 0;
        virtual WStringCR               _GetName() const = 0;
        virtual void                    _SetName(WStringCR name) = 0;

    public:
        DGNPLATFORM_EXPORT void         SetActive (bool active);
        DGNPLATFORM_EXPORT bool         GetActive () const;
        DGNPLATFORM_EXPORT bool         GetOutside() const;
        DGNPLATFORM_EXPORT void         SetOutside(bool outside);
        DGNPLATFORM_EXPORT WStringCR    GetName() const;
        DGNPLATFORM_EXPORT void         SetName(WStringCR name);
        DGNPLATFORM_EXPORT void         Clear();
        DGNPLATFORM_EXPORT StatusInt    OnTransform (TransformCR trn);
        DGNPLATFORM_EXPORT DRange3d     GetRange() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClipBox : public PointCloudClipBase
    {
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    friend BentleyApi::DgnPlatform::PointCloudClipElementHandler;
#endif

    private:
        uint32_t        m_id;
        AttributeFlags  m_flags;
        WString         m_name;
        OrientedBox     m_box;

        uint32_t GetId() const;
        void   SetId(uint32_t id);

    protected:
        PointCloudClipBox();
        PointCloudClipBox(WStringCR name, OrientedBoxCR box);
        ~PointCloudClipBox();

        StatusInt Load(DataInternalizer& dataInternalizer, uint32_t id);
        StatusInt Load(BentleyApi::DgnPlatform::ElementHandle::XAttributeIter const& xAttr);
        StatusInt Store(DataExternalizer& dataExternalizer) const;
        StatusInt ScheduleWriteXAttribute(EditElementHandleR hostElement) const;

        // from PointCloudClipBase
        virtual void                    _Clear();
        virtual StatusInt               _OnTransform (TransformCR trn);
        virtual DRange3d                _GetRange() const;
        virtual void                    _SetActive (bool active);
        virtual bool                    _GetActive () const;
        virtual void                    _SetOutside(bool outside);
        virtual bool                    _GetOutside() const;
        virtual WStringCR               _GetName() const;
        virtual void                    _SetName(WStringCR name);

    public:

        DGNPLATFORM_EXPORT static PointCloudClipBoxPtr Create(WStringCR name, OrientedBoxCR box);
        DGNPLATFORM_EXPORT static PointCloudClipBoxPtr Create(ElementHandleCR hostElement);
        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::XAttributeHandlerId GetXAttributeHandlerId ();

        DGNPLATFORM_EXPORT OrientedBoxCR GetBox() const;
        DGNPLATFORM_EXPORT void          SetBox(OrientedBoxCR box);
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClipPolygon : public PointCloudClipBase
    {
#ifdef WIP_VANCOUVER_MERGE // pointcloud
    friend BentleyApi::DgnPlatform::PointCloudClipElementHandler;
#endif

    private:
        uint32_t            m_id;
        AttributeFlags      m_flags;
        WString             m_name;
        PointCloudPolygon   m_polygon;

        uint32_t GetId() const;
        void   SetId(uint32_t id);

    protected:
        PointCloudClipPolygon();
        PointCloudClipPolygon(WStringCR name, PointCloudPolygonCR polygon);
        ~PointCloudClipPolygon();

        StatusInt Load(DataInternalizer& dataInternalizer, uint32_t id);
        StatusInt Load(BentleyApi::DgnPlatform::ElementHandle::XAttributeIter const& xAttr);
        StatusInt Store(DataExternalizer& dataExternalizer) const;
        StatusInt ScheduleWriteXAttribute(EditElementHandleR hostElement) const;

        // from PointCloudClipBase
        virtual void                    _Clear();
        virtual StatusInt               _OnTransform (TransformCR trn);
        virtual DRange3d                _GetRange() const;
        virtual void                    _SetActive (bool active);
        virtual bool                    _GetActive () const;
        virtual void                    _SetOutside(bool outside);
        virtual bool                    _GetOutside() const;
        virtual WStringCR               _GetName() const;
        virtual void                    _SetName(WStringCR name);

    public:
        DGNPLATFORM_EXPORT static PointCloudClipPolygonPtr Create(WStringCR name, PointCloudPolygonCR polygon);
        DGNPLATFORM_EXPORT static PointCloudClipPolygonPtr Create(ElementHandleCR hostElement);
        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::XAttributeHandlerId  GetXAttributeHandlerId ();

        DGNPLATFORM_EXPORT bool                 IsBlock() const;
        DGNPLATFORM_EXPORT PointCloudPolygonCR  GetPolygon() const;
        DGNPLATFORM_EXPORT void                 SetPolygon(PointCloudPolygonCR poly);
    };

/*---------------------------------------------------------------------------------**//**
* FaceAndNormal
+---------------+---------------+---------------+---------------+---------------+------*/
enum PointCloudBoxCorners
    {
    // lower surface
    Front_Lower_Left    = 0,
    Front_Lower_Right   = 1,
    Back_Lower_Right    = 2,
    Back_Lower_Left     = 3,

    //upper surface
    Front_Upper_Left    = 4,
    Back_Upper_Right    = 5,
    Back_Upper_Left     = 6,
    Front_Upper_Right   = 7,
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // pointcloud
struct PointCloudClipFacility
    {
    public:
        DGNPLATFORM_EXPORT static void GetEffectiveRange(DRange3d& globalRange, ElementHandleCR eh);

        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::PointCloudClipElementHandler::ClipBoxPtrCollection GetActiveClipBoxes(ElementHandleCR hostElement);
        DGNPLATFORM_EXPORT static BentleyApi::DgnPlatform::PointCloudClipElementHandler::ClipPolygonPtrCollection GetActiveClipPolygons(ElementHandleCR hostElement);

    private:
        static void ComputeClipRange (DRange3d& clipRange, DRange3d const& globalRange000, Transform const& transform, ElementHandleCR eh);
    };
#endif

/*__PUBLISH_SECTION_START__*/

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
