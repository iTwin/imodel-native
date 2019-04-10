/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/DTMSymbologyOverrideManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsiclass                                            Daryl.Holmwood     10/2010
//=======================================================================================
class DTMSymbologyOverrideManager : public XAttributeHandler, public IXAttributePointerContainerHandler, public IDependencyHandler
    {
    private:
        static const short CURRENT_REFERENCE_VERSION = 1;
    public:
        DTMSymbologyOverrideManager()
            {
            }
    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 04/10
    //=======================================================================================
    public: static XAttributeHandlerId GetDisplayRefXAttributeHandlerId() { return XAttributeHandlerId(TMElementMajorId, XATTRIBUTES_SUBID_DTM_OVERRIDEDISPLAYREF); }
    public: static XAttributeHandlerId GetDisplayInfoXAttributeHandlerId() { return XAttributeHandlerId(TMElementMajorId, XATTRIBUTES_SUBID_DTM_OVERRIDEDISPLAYINFO); }

            //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: static void SetReferencedElement (EditElementHandleR newElem, ElementHandleCR elem);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: DTMELEMENT_EXPORT static bool GetReferencedElement (ElementHandleCR elem, ElementHandleR referencedElem);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: static bool GetOverridingElement (ElementHandleCR storageElem, ElementHandleCR elem, ElementHandleR relatedElem, bool allowRemap = false);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: static void StoreOverridingElement (EditElementHandleR storageElem, ElementHandleCR elem, ElementHandleR relatedElem);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: static void DeleteOverridingElement (EditElementHandleR storageElem, ElementHandleCR elem);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: DTMELEMENT_EXPORT static bool GetElementForSymbology (ElementHandleCR elem, ElementHandleR symbologyElem, DgnModelRefP destinationModel, bool allowRemap = false);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: DTMELEMENT_EXPORT static bool CanHaveSymbologyOverride (ElementHandleCR elem);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: DTMELEMENT_EXPORT static StatusInt CreateSymbologyOverride (ElementHandleCR elem, DgnModelRefP destinationModelRef);

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 10/10
    //========================================================================"===============
    public: DTMELEMENT_EXPORT static StatusInt DeleteSymbologyOverride (ElementHandleCR elem, DgnModelRefP destinationModelRef);

    public: StatusInt _OnPreprocessCopy (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element, ElementCopyContextP cc);
    public: virtual StatusInt _OnPreprocessCopyRemapIds (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xAttrHandle, ElementHandleCR element) override;
    public: virtual void _OnElementIDsChanged (XAttributeHandleR xa, ElementAndModelIdRemappingCR remapTable, ElementHandleCR element);
    public: virtual void _DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xAttrHandle, DgnModelRefP homeModel);
    public: virtual IDependencyHandler* _GetIDependencyHandler () {return this;}
    public: virtual void _OnRootsChanged(ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected);
    public: virtual void _OnUndoRedoRootsChanged(ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected) const;
    };

struct DTMOverrideSymbologyManager : public Handler,public ITransactionHandler
    {
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR (DTMOverrideSymbologyManager, DTMELEMENT_EXPORT);
public:
    virtual void _QueryProperties (ElementHandleCR element, PropertyContextR context) override;
    virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
    virtual void _OnElementLoaded (ElementHandleCR element) override;
    virtual void _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override;
    virtual void _OnHistoryRestore (ElementHandleP after, ElementHandleP before, ChangeTrackAction actionStep, BentleyDgnHistoryElementChangeType effectiveChange) override;

    virtual void _OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag) override;
    virtual void _OnAdded (ElementHandleP element) override;
    virtual void _OnDeleted (ElementHandleP element) override;
    };

DTMELEMENT_EXPORT void CopySymbology (EditElementHandleR element, ElementHandleCR source, ElementCopyContextP copyContext, bool allowRemap = false);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
