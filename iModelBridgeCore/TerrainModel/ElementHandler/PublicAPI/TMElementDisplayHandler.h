/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/TMElementDisplayHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform\TerrainModel\TMElementHandler.h>
//__PUBLISH_SECTION_END__

#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include <TerrainModel\TerrainModel.h>
#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE
    struct DTMFenceParams;
END_BENTLEY_TERRAINMODEL_NAMESPACE

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
//__PUBLISH_SECTION_END__

USING_NAMESPACE_BENTLEY_DGNPLATFORM

struct DTMDrawingInfo;
struct DTMDataRef;


//=======================================================================================
// @bsiclass                                            Daryl.Holmwood     04/2010
//=======================================================================================
struct DTMElement107Handler:
    public Handler,
    virtual public ITransactionHandler
    {
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (DTMElement107Handler, DTMELEMENT_EXPORT);

    static ElementHandlerId GetElemHandlerId () {return ElementHandlerId (TMElementMajorId, ELEMENTHANDLER_DTMDATA);}
    virtual void DTMElement107Handler::_OnDeleted(ElementHandleP element) override;
    //=======================================================================================
    // @bsimethod                                                   Mathieu.St-Pierre 03/11
    //=======================================================================================
    virtual ITransactionHandlerP _GetITransactionHandler() override
        {
        return this;
        }
    };

/*__PUBLISH_SECTION_START__*/

struct DTMElementDisplayHandler : public DTMElementHandler,
                          virtual public ITransactionHandler
    {
    DEFINE_T_SUPER(DTMElementHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (DTMElementDisplayHandler, DTMELEMENT_EXPORT);
    //__PUBLISH_SECTION_END__

    DTMELEMENT_EXPORT virtual ITransactionHandlerP _GetITransactionHandler() override
        {
        return this;
        }

    DTMELEMENT_EXPORT virtual bool _InstancePresentation (ElementHandleCR eh, bool& unsharedGraphicsPresent) override { return false; }
    DTMELEMENT_EXPORT virtual void _Draw (ElementHandleCR element, ViewContextR context) override;
    DTMELEMENT_EXPORT virtual StatusInt _DrawCut (ElementHandleCR element, ICutPlaneR cutPlane, ViewContextR context) override;
    DTMELEMENT_EXPORT virtual void _GetPathDescription (ElementHandleCR element, WStringR string, const DisplayPath* path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) override;
    DTMELEMENT_EXPORT virtual SnapStatus _OnSnap (SnapContextP context, int snapPathIndex) override;
    DTMELEMENT_EXPORT virtual BentleyStatus _GeneratePresentation (EditElementHandleR element, ViewContextR viewContext) override;

    DTMELEMENT_EXPORT virtual void _OnConvertTo2d (EditElementHandleR element, TransformCR flattenTrans, DVec3dCR flattenDir) override;
    DTMELEMENT_EXPORT virtual ReprojectStatus _OnGeoCoordinateReprojection (EditElementHandleR element, IGeoCoordinateReprojectionHelper& geo, bool inChain) override;
    DTMELEMENT_EXPORT virtual StatusInt _OnFenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options) override;
    DTMELEMENT_EXPORT virtual StatusInt _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options) override;
    DTMELEMENT_EXPORT virtual StatusInt _FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options) override;

    DTMELEMENT_EXPORT virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;

    DTMELEMENT_EXPORT virtual StatusInt _OnTransform (EditElementHandleR element, TransformInfo const& transform) override;
    DTMELEMENT_EXPORT virtual bool _IsTransformGraphics (ElementHandleCR element, TransformInfoCR tInfo) override;
    DTMELEMENT_EXPORT virtual StatusInt _OnPreprocessCopy (EditElementHandleR element, ElementCopyContextP copyContext) override;
    DTMELEMENT_EXPORT virtual StatusInt _OnPostProcessCopy (EditElementHandleR element, ElementCopyContextP copyContext) override;

    DTMELEMENT_EXPORT virtual ITransactionHandler::PreActionStatus _OnAdd (EditElementHandleR element) override;
    DTMELEMENT_EXPORT virtual ITransactionHandler::PreActionStatus _OnReplace (EditElementHandleR element, ElementHandleCR oldElement) override;
    DTMELEMENT_EXPORT virtual void _OnDeleted (ElementHandleP element) override;
    DTMELEMENT_EXPORT virtual void _OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag) override;
    DTMELEMENT_EXPORT virtual void _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override;
    DTMELEMENT_EXPORT virtual void _OnUndoRedoFinished (ElementRefP element, bool isUndo) override;
    DTMELEMENT_EXPORT virtual void _OnHistoryRestore (ElementHandleP after, ElementHandleP before, ChangeTrackAction actionStep, BentleyDgnHistoryElementChangeType effectiveChange) override;

    DTMELEMENT_EXPORT virtual BentleyStatus _ValidateElementRange (EditElementHandleR element, bool setZ) override;
    DTMELEMENT_EXPORT virtual void _GetDescription (ElementHandleCR element, WString& string, uint32_t desiredLength) override;

    DTMELEMENT_EXPORT virtual StatusInt _OnDecorate (ElementHandleCR, ViewContextP) override { return 1; }
    DTMELEMENT_EXPORT virtual IAnnotationHandlerP _GetIAnnotationHandler (ElementHandleCR el) override;

    static bool GetProjectedPointOnDTM (DPoint3dR pointOnDTM, ElementHandleCR element, ViewportP viewport, DPoint3dCR testPoint);
    static void GetDTMDrawingInfo (DTMDrawingInfo& info, ElementHandleCR element, DTMDataRef* DTMDataRef, ViewContextP context, HitPathCP hitPath = nullptr);
    static bool DrawPart (ViewContextR context, DTMDrawingInfo& info, bool hasHilite, DTMSubElementIter& iter);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
    // ToDo DGNPLAT    virtual void _ModifyingSubElement (EditElementHandleR element, const DTMSubElementId & xAttrId, const DTMElementSubHandler::SymbologyParams& params) const override;
    DTMELEMENT_EXPORT static bool HasHighlight(ElementRefP element);
    DTMELEMENT_EXPORT static void SetHighlight(ElementRefP element, uint32_t id, ElementHiliteState hiliteState);
    DTMELEMENT_EXPORT static ElementHiliteState GetHighlight(ElementRefP element, uint32_t id);

    DTMELEMENT_EXPORT static void DrawSubElement (ElementHandleCR element, const ElementHandle::XAttributeIter& xAttr, ViewContextR context, const BENTLEY_NAMESPACE_NAME::TerrainModel::DTMFenceParams& fence);

    public: DTMELEMENT_EXPORT bool IsDrawForAnimation();

    //! Test if the ElementHandle is a DTMElement
    //! @param[in] element the Element Handle to test
    //! @return if the DTM is an Element
    public: DTMELEMENT_EXPORT static bool IsDTMElement (ElementHandleCR element);
    //! Gets the DTMDataRef from an ElementHandle
    //! @param[out] the DTMDataRef
    //! @param[in] element the Element Handle to test
    //! @return Errorcode
    public: DTMELEMENT_EXPORT static StatusInt GetDTMDataRef (RefCountedPtr<DTMDataRef>& outRef, ElementHandleCR element);

    //! Creates an editHandle from the dtm, using current transform is one exists.
    //! @param editHandle IN OUT this is the editHandle which will be set to the DTMElement, it will use the Model set in the EditHandle.
    //! @param storageTransformation IN the transformation from UORS to storage units.
    //! @param dtm IN the DTM to add.
    //! @param disposeDTM IN release the DTM memory and assocate with the DTMElement (see remarks)
    //! @returns SUCCESS if all went ok, otherwise ERROR.
    //! @remarks If you aren't going to use the DTM after adding then set disposeDTM.
    //! @note the editHandle needs to be added to the Model.
    public: DTMELEMENT_EXPORT static BentleyStatus CreateDTMElement (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, DgnModelRefR modelRef, bool disposeDTM = false);
    };
//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Eric.Paquet  06/2013
//=======================================================================================
struct MrDTMDgnModelAppData : public DgnModelAppData, NonCopyableClass
    {
    public:
        static DgnModelAppData::Key const&          GetKey ();

        DTMELEMENT_EXPORT static MrDTMDgnModelAppData& GetAppData(DgnModelR model);
        DTMELEMENT_EXPORT size_t GetNbMrDTM ();
        DTMELEMENT_EXPORT T_StdElementRefSet const& GetMrDTMElementRefList() const;

        bool AddMrDTMElementRef(ElementRefP elemRefP);
        size_t RemoveMrDTMElementRef(ElementRefP elemRefP);

    protected:
        virtual void _OnCleanup (DgnModelR host) override;
        virtual void _OnSaveModelProperties (DgnModelR host, ModelInfoCR original) override;

    private:
        MrDTMDgnModelAppData();


    private:
        T_StdElementRefSet  m_elementRefList;       // MrDTM elementRef that exist in this DgnModel.
    };

//=======================================================================================
//! The STM display handler that override the 'simple' DgnPlatform::MrDTMDefaultElementHandler.
//! Ideally we would have liked to inherit from MrDTMDefaultElementHandler as well as DTMElementDisplayHandler
//! but we removed MrDTMDefaultElementHandler to avoid multiple inheritance of DTMElementHandler. It's Ok to remove
//! it since the only useful functionnality is _GetTypeName() and we override it here.
// @bsiclass
//=======================================================================================
class MrDTMElementDisplayHandler : public DTMElementDisplayHandler
    {
    public:
    DEFINE_T_SUPER(DTMElementDisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (MrDTMElementDisplayHandler, DTMELEMENT_EXPORT);

    protected:

        DTMELEMENT_EXPORT virtual bool _IsSupportedOperation (ElementHandleCP element, Element::SupportOperation stype) override;

        DTMELEMENT_EXPORT virtual void _OnDeleted (ElementHandleP elHandle) override;
        DTMELEMENT_EXPORT virtual void _OnAdded (ElementHandleP elHandle) override;

    public :

        DTMELEMENT_EXPORT virtual void _GetTypeName (WStringR string, uint32_t desiredLength) override;

        DTMELEMENT_EXPORT static ElementHandlerId GetElemHandlerId ();
        
        DTMELEMENT_EXPORT virtual void _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

        DTMELEMENT_EXPORT virtual void _OnElementLoaded (ElementHandleCR element) override;

        //Helper function                
        DTMELEMENT_EXPORT static void OnModelRefActivate (DgnModelRefR newModelRef, DgnModelRefP oldModelRef);
        DTMELEMENT_EXPORT static void OnModelRefActivated (DgnModelRefR newModelRef, DgnModelRefP oldModelRef);

        DTMELEMENT_EXPORT static bool IsHighQualityDisplayForMrDTM();
    };

//__PUBLISH_SECTION_START__

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
