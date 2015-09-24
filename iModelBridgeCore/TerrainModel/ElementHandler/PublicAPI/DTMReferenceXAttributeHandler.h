/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/DTMReferenceXAttributeHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

class DTMDataRefXAttribute;
class DTMReferenceXAttributeHandler : public XAttributeHandler, public IXAttributeHostTransactionHandler, public IXAttributeTransactionHandler, public IXAttributePointerContainerHandler, public IDependencyHandler
    {
    friend DTMDataRefXAttribute;
    private: static ElementRefP s_ignoreXAttributeHandlerDelete;
    public: static XAttributeHandlerId GetXAttributeHandlerId () {return XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_REFERENCE);}
    public: static const int GetXAttributeId() { return 1; }

    public: static XAttributeHandlerId GetRefCountXAttributeHandlerId () {return XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_REFERENCECOUNT);}
    public: static const int GetRefCountXAttributeId() { return 0; }

    public: DTMELEMENT_EXPORT static void RemoveDTMDataReference(EditElementHandleR el);

    private: DTMELEMENT_EXPORT static void SetIgnoreXAttributeHandlerDelete (ElementRefP elRef);
    public: DTMELEMENT_EXPORT static void SetDTMDataReference(EditElementHandleR el, ElementHandleCR dataEl);
    public: DTMELEMENT_EXPORT static bool GetDTMDataReference(ElementHandleCR el, ElementHandleR dataEl);

    //XAttributeHandler
    public: virtual StatusInt _OnPreprocessCopy (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xa, ElementHandleCR element, ElementCopyContextP cc) override;

    // IXAttributeHostTransactionHandler
    public: virtual IXAttributeHostTransactionHandler* _GetIXAttributeHostTransactionHandler() override {return this;}
//    public: virtual StatusInt OnPreprocessCopy(IReplaceXAttribute* toBeReplaced, XAttributeHandle const& xa, ElementHandleCR element, ElementCopyContextP cc);
    public: virtual void _OnPreHostDelete (XAttributeHandleCR xAttr, TransactionType type);
    public: virtual void _OnPreHostChange (XAttributeHandleCR xAttr, TransactionType type);

    // IXAttributeTransactionHandler
    public: virtual IXAttributeTransactionHandler* _GetIXAttributeTransactionHandler() override {return this;}
    public: virtual void _OnPostAdd (XAttributeHandleCR xAttr, TransactionType type);
    virtual void _OnPostModifyData (XAttributeHandleCR xAttr, TransactionType type) {}
    virtual void _OnPreReplaceData (XAttributeHandleCR xAttr, void const* newData, uint32_t newSize, TransactionType type);

    public: virtual void _OnPreDelete (XAttributeHandleCR xAttr, TransactionType type);

    // IXAttributePointerContainerHandler
    virtual StatusInt _OnPreprocessCopyRemapIds (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xa, ElementHandleCR element) override;
    virtual void _OnElementIDsChanged (XAttributeHandleR xa, ElementAndModelIdRemappingCR remapTable, ElementHandleCR element);
    virtual void _DisclosePointers (T_StdElementRefSet* refs, XAttributeHandleCR xa, DgnModelRefP homeModel);
    virtual IDependencyHandler* _GetIDependencyHandler () {return this;}

    virtual void    _OnRootsChanged(ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected);
    virtual void    _OnUndoRedoRootsChanged(ElementHandleR dependent, bvector<RootChange> const& _RootsChanged, bvector<XAttributeHandle> const& xAttrsAffected) const;

    private:
    DTMELEMENT_EXPORT static ElementHandle ExtractReferencedElement (const void* data, uint32_t size, ElementRefP ref, DgnModelRefP model);
    DTMELEMENT_EXPORT static ElementHandle ExtractReferencedElement (XAttributeHandleCR xAttr, DgnModelRefP model = 0, bool checkForDelete = false);
    DTMELEMENT_EXPORT static int RemoveReferenceCount(ElementHandleR el);
    DTMELEMENT_EXPORT static void AddReferenceCount(ElementHandleR el);
    };


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
