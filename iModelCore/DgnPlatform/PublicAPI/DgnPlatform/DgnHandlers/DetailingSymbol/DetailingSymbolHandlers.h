/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DetailingSymbol/DetailingSymbolHandlers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#ifdef WIP_DETAILINGSYMBOLS
#include <ECObjects/ECObjectsAPI.h>
#include "DetailingSymbol.h"
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#ifdef WIP_DETAILINGSYMBOLS

enum DeleteOptions
    {
    ACTION_Yes      = 0,
    ACTION_YesToAll = 1,
    ACTION_No       = 2,
    ACTION_NoToAll  = 3,
    ACTION_Cancel   = 4,
    };

enum ModelTypeBitMask
    {
    MODELTYPEMASK_Normal        = (0x0001 << 0),
    MODELTYPEMASK_Sheet         = (0x0001 << 1),
    MODELTYPEMASK_Extraction    = (0x0001 << 2),
    MODELTYPEMASK_Drawing       = (0x0001 << 3),
    };

/*=================================================================================**//**
* Interface for Named View Creator Helper Class.
* Callout element handlers can provide special NamedViewCreateHelpers to create Named Views
* of specific type and behavior.
* @bsiclass                                                     Sunand.Sandurkar 12/2006
+===============+===============+===============+===============+===============+======*/
struct     INamedViewCreateHelper : public RefCounted <IRefCounted>
{
protected:
    virtual int                                 GetViewNamePrefixID () = 0;
    virtual int                                 GetViewTypeID () = 0;

public:
    DGNPLATFORM_EXPORT WString                  GetViewNamePrefix ();
    DGNPLATFORM_EXPORT WString                  GetViewType ();
};

typedef RefCountedPtr<INamedViewCreateHelper>        INamedViewCreateHelperPtr;
//DGNPLATFORM_TYPEDEFS (INamedViewCreateHelper)

#endif

#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* Detailing Symbol Element Handler Base Class
* @bsiclass                                                     Sunand.Sandurkar 03/2004
+===============+===============+===============+===============+===============+======*/
struct          DetailingSymbolBaseHandler :    Type2Handler,
                                                IAnnotationHandler
#ifdef WIP_DETAILINGSYMBOLS
                                                ITextEdit,
                                                IDependencyHandler,
                                                IDisplayHandlerXGraphicsPublish
#endif
{
    DEFINE_T_SUPER(Type2Handler)
private:

    enum SortPriority
        {
        SORTPRIORITY_Color           = 1000,
        SORTPRIORITY_LineStyle       = 2000,
        SORTPRIORITY_Weight          = 3000,
        SORTPRIORITY_IsAnnotation    = 4000,
        SORTPRIORITY_BubbleSize      = 5000,
        };

    StatusInt                                   GetLevel (LevelId*, ElementHandleCR);
#ifdef WIP_DETAILINGSYMBOLS
    StatusInt                                   FindTextHeight (double * textHeight, ElementHandleCP symbolElemHandle);
    void                                        EditDetailingSymbolChildProperties (EditElementHandleR eeh, PropertyContextR context, EditElementHandleR detElemHdl) const;
#endif
protected:
    virtual void                                _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                                _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override {_GetTypeName (descr, desiredLength);}
    virtual void                                _GetPathDescription (ElementHandleCR, WStringR, DisplayPathCP, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiter) override;
#ifdef WIP_DETAILINGSYMBOLS
    virtual StatusInt                           _OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo) override;
    virtual bool                                _IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR trans) const override;
#endif
    virtual void                                _Draw (ElementHandleCR, ViewContextR) override;
#ifdef WIP_DETAILINGSYMBOLS

    // ITextEdit
    virtual ITextPartIdPtr                      _GetTextPartId      (ElementHandleCR, HitPathCR) const override;
    virtual void                                _GetTextPartIds     (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const override;
    virtual TextBlockPtr                        _GetTextPart        (ElementHandleCR, ITextPartIdCR) const override;
    virtual ReplaceStatus                       _ReplaceTextPart    (EditElementHandleR, ITextPartIdCR, TextBlockCR) override;

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                             _GetEcPropertiesClassName (ElementHandleCR) override {BeAssert (false); return L"";}
    virtual StatusInt                           _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
    virtual bool                                _IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName) override;
            StatusInt                           AddProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR);

            WString                             GetPropName (int propId);

    static StatusInt                            GetMyBooleanDelegate (bool&, ElementHandleCR, UInt32 propId, size_t arrayIndex);
    static StatusInt                            SetMyBooleanDelegate (EditElementHandleR, UInt32 propId, size_t arrayIndex, bool);
    */

    virtual IDependencyHandler*                 _GetIDependencyHandler ()  override = 0;
    virtual void                                _UndoRedoRootsChanged (ElementHandleR, bvector<RootChange> const&, bvector<XAttributeHandle> const&) const {}
    virtual void                                _OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) override = 0;
    virtual void                                _OnUndoRedoRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) const {}
    bool                                        IsRootChanged (IDependencyHandler::ChangeStatus&, PersistentElementPath* rootPepOut, ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected, uint32_t pepID);
    bool                                        IsSelfChanged (IDependencyHandler::ChangeStatus&, ElementHandleR dependent, bvector<RootChange> const& rootsChanged);

    virtual StatusInt                           _OnPreprocessCopy (EditElementHandleR symbolEH, CopyContextP ccP) override;
#endif
    virtual IAnnotationHandlerP                 _GetIAnnotationHandler (ElementHandleCR)  override {return this;}
    virtual bool                                _GetAnnotationScale (double* annotationScale, ElementHandleCR element) const override;
#ifdef WIP_DETAILINGSYMBOLS
    virtual StatusInt                           _ApplyAnnotationScaleDifferential (EditElementHandleR, double scale) override;
    size_t                                      UpdateFields (EditElementHandleR eeh);
    bool                                        AreIdentical (EditElementHandleR eh, ElementHandleCR    oldEh);
    virtual void                                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
    virtual bool                                _SupressXGraphicsPublish (ElementHandleCR eh) { return true; }
#endif

public:

    void                                        DrawWithAnnotationScale (ElementHandleCR thisElm, ViewContextP context, AnnotationDisplayParameters const&);
#ifdef WIP_DETAILINGSYMBOLS

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static bool                                 IsAnnotation (ElementHandleCR elm);
    static void                                 SetIsAnnotation (EditElementHandleR, bool);
    */
#endif
    virtual void                                GetImageName (WStringR name) = 0;

    static void RegisterHandlers();

}; // DetailingSymbolBaseHandler

/*=================================================================================**//**
* Callout Element Handler base class
* @bsiclass                                                     Sunand.Sandurkar  08/2007
+===============+===============+===============+===============+===============+======*/
struct          CalloutBaseHandler : DetailingSymbolBaseHandler
{
    DEFINE_T_SUPER(DetailingSymbolBaseHandler)
#ifdef WIP_DETAILINGSYMBOLS
private:
    StatusInt                                   FlattenMarker (EditElementHandleR symbolEH);
    bool                                        IsCopyInSameModel (ElementHandleCR symbolEH);
    double                                      GetZOffsetForClip (ElementHandleCR clipEH, DgnModelP refDgnModel, bool forUpdatingView);
    bool                                        IsViewDeleted (PersistentElementPath& viewPep, IDependencyHandler::ChangeStatus viewChangeStatus, DgnModelP homeCache);

protected:

    StatusInt                                   UpdateViewFromMarker (ElementHandleCR markerEH);
    StatusInt                                   UpdateMarkerFromView (ElementHandleCR newElem);
    DgnModelP                                GetRefDgnModel (ElementHandleCR markerEH);
    DgnModelP                                GetViewRootDgnModel (ElementHandleCR viewEH);
    void                                        GetClipRotMatrix (RotMatrix& rMatrix, ElementHandleCR clipEH);
    StatusInt                                   GetClipOrigin (DPoint3d& origin, ElementHandleCR clipEH);
    StatusInt                                   _OnPreprocessCopy (EditElementHandleR symbolEH, CopyContextP ccP);
    virtual void                                _OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) override;

public:
    virtual void                                _OnAdded (ElementHandleP element) override;

    DGNPLATFORM_EXPORT static StatusInt         FindGeneratedView (EditElementHandleP viewEEH, bool* controlExtents, ElementHandleCR rootEH, bool makeSavedViewModelReadWrite);
    DGNPLATFORM_EXPORT static StatusInt         FindViewName (WStringR viewName, ElementHandleCR viewEH);
    DGNPLATFORM_EXPORT static StatusInt         CopyGeneratedView (EditElementHandleR symbolEH);
#endif
    DGNPLATFORM_EXPORT static StatusInt         FindGeneratedViewName (WStringR viewName, ElementHandleCR symbolEH);
    DGNPLATFORM_EXPORT static DgnViewId         FindGeneratedView (ElementHandleCR rootEH);
}; // CalloutBaseHandler

#ifdef WIP_DETAILINGSYMBOLS
/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
struct          DetailingSymbolBaseXAttributeHandler  : XAttributeHandler, IXAttributePointerContainerHandler
    {
private:
    virtual uint16_t                            GetMinorID () = 0;

protected:
    virtual void                                _DisclosePointers (T_StdElementRefSet*, XAttributeHandleCR, DgnModelP) override;
    virtual StatusInt                           _OnPreprocessCopy (IReplaceXAttribute* replacer, XAttributeHandleCR xa, ElementHandleCR h, CopyContextP cc) {return SUCCESS;}
    virtual StatusInt                           _OnPreprocessCopyRemapIds (IReplaceXAttribute* toBeReplaced, XAttributeHandleCR xa, ElementHandleCR eh) {return SUCCESS;}
    virtual void                                _OnElementIDsChanged (XAttributeHandleR xa, ElementAndModelIdRemappingCR remapTable, ElementHandleCR eh) {}

    }; // DetailingSymbolBaseXAttributeHandler
#endif

/*=================================================================================**//**
* Section Marker Element Handler
* @bsiclass                                                     Ray.Bentley      09/2006
+===============+===============+===============+===============+===============+======*/
struct          SectionCalloutHandler : CalloutBaseHandler
{
    DEFINE_T_SUPER(CalloutBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(SectionCalloutHandler, DGNPLATFORM_EXPORT)

private:

    enum SortPriority
        {
        SORTPRIORITY_FrontDepth      = 6000,
        SORTPRIORITY_BackDepth       = 7000,
        SORTPRIORITY_MinWidth        = 8000,
        SORTPRIORITY_MaxWidth        = 9000,
        SORTPRIORITY_TopHeight       = 10000,
        SORTPRIORITY_BottomHeight    = 11000,
        SORTPRIORITY_FrontCrop       = 12000,
        SORTPRIORITY_BackCrop        = 13000,
        SORTPRIORITY_LeftCrop        = 14000,
        SORTPRIORITY_RightCrop       = 15000,
        SORTPRIORITY_TopCrop         = 16000,
        SORTPRIORITY_BottomCrop      = 17000,
        SORTPRIORITY_FlipDirection   = 18000,
        SORTPRIORITY_PreserveUp      = 19000,

        };

protected:

    virtual void                                    _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                                    _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override;

#ifdef WIP_DETAILINGSYMBOLS

    virtual IDependencyHandler*                     _GetIDependencyHandler ()  override {return this;}

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                                 _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                               _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
    static StatusInt                                GetMyDoubleDelegate (double&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                                SetMyDoubleDelegate (EditElementHandleR, UInt32 propId, size_t, double valueIn);
    static StatusInt                                GetMyBooleanDelegate (bool&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                                SetMyBooleanDelegate (EditElementHandleR, UInt32 propId, size_t, bool valueIn);
    */
#endif

    virtual void                                    GetImageName (WStringR name) {name.append (L"SectionMarker");}

#ifdef WIP_DETAILINGSYMBOLS
    // INamedViewCreateHelper methods
    virtual int                                     GetViewNamePrefixID ();
    virtual int                                     GetViewTypeID ();

public:
    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static double                                   GetDepth        (ElementHandleCR, bool);
    static void                                     SetDepth        (EditElementHandleR, bool, double);
    static bool                                     GetCropMask     (ElementHandleCR, ClipVolumeCropProp);
    static void                                     SetCropMask     (EditElementHandleR, ClipVolumeCropProp, bool);
    static bool                                     GetFlipDirection (ElementHandleCR);
    static void                                     SetFlipDirection (EditElementHandleR, bool);
    static double                                   GetMinWidth     (ElementHandleCR);
    static void                                     SetMinWidth     (EditElementHandleR, double);
    static double                                   GetMaxWidth     (ElementHandleCR);
    static void                                     SetMaxWidth     (EditElementHandleR, double);
    static double                                   GetTopHeight    (ElementHandleCR);
    static void                                     SetTopHeight    (EditElementHandleR, double);
    static double                                   GetBottomHeight (ElementHandleCR);
    static void                                     SetBottomHeight (EditElementHandleR, double);
    static VolumeDef*                               GetVolumeDef    (IDetailingSymbolPtr def);
    static bool                                     GetPreserveUp (ElementHandleCR);
    static void                                     SetPreserveUp (EditElementHandleR, bool);
    */
#endif
};

#ifdef WIP_DETAILINGSYMBOLS
/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
struct          SectionCalloutXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_SectionCallout_IDENTIFIER;}

    }; // SectionCalloutXAttributeHandler
#endif

/*=================================================================================**//**
* Elevation Marker Element Handler
* @bsiclass                                                     SunandSandurkar 01/2007
+===============+===============+===============+===============+===============+======*/
struct          ElevationCalloutHandler : CalloutBaseHandler
{
    DEFINE_T_SUPER(CalloutBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(ElevationCalloutHandler, DGNPLATFORM_EXPORT)

private:

    enum SortPriority
        {
        SORTPRIORITY_FrontDepth      = 6000,
        SORTPRIORITY_BackDepth       = 7000,
        SORTPRIORITY_MinWidth        = 8000,
        SORTPRIORITY_MaxWidth        = 9000,
        SORTPRIORITY_TopHeight       = 10000,
        SORTPRIORITY_BottomHeight    = 11000,
        SORTPRIORITY_FrontCrop       = 12000,
        SORTPRIORITY_BackCrop        = 13000,
        SORTPRIORITY_LeftCrop        = 14000,
        SORTPRIORITY_RightCrop       = 15000,
        SORTPRIORITY_TopCrop         = 16000,
        SORTPRIORITY_BottomCrop      = 17000,
        SORTPRIORITY_PreserveUp      = 18000,
        };

protected:

    virtual void                            _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                            _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override;

#ifdef WIP_DETAILINGSYMBOLS
    virtual IDependencyHandler*             _GetIDependencyHandler ()  override {return this;}

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                         _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                       _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;

    static StatusInt                        GetMyDoubleDelegate (double&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                        SetMyDoubleDelegate (EditElementHandleR, UInt32 propId, size_t, double valueIn);
    static StatusInt                        GetMyBooleanDelegate (bool&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                        SetMyBooleanDelegate (EditElementHandleR, UInt32 propId, size_t, bool valueIn);
    */
#endif

    virtual void                            GetImageName (WStringR name) {name.append (L"ElevationMarker");}

#ifdef WIP_DETAILINGSYMBOLS
    // INamedViewCreateHelper methods
    virtual int                                     GetViewNamePrefixID ();
    virtual int                                     GetViewTypeID ();
#endif

public:
    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static double                           GetDepth (ElementHandleCR, bool);
    static void                             SetDepth (EditElementHandleR, bool, double);
    static bool                             GetCropMask (ElementHandleCR, ClipVolumeCropProp);
    static void                             SetCropMask (EditElementHandleR, ClipVolumeCropProp, bool);
    static double                           GetMinWidth (ElementHandleCR);
    static void                             SetMinWidth (EditElementHandleR, double);
    static double                           GetMaxWidth (ElementHandleCR);
    static void                             SetMaxWidth (EditElementHandleR, double);
    static double                           GetTopHeight (ElementHandleCR);
    static void                             SetTopHeight (EditElementHandleR, double);
    static double                           GetBottomHeight (ElementHandleCR);
    static void                             SetBottomHeight (EditElementHandleR, double);
    static VolumeDef*                       GetVolumeDef (IDetailingSymbolPtr def);
    static bool                             GetPreserveUp (ElementHandleCR);
    static void                             SetPreserveUp (EditElementHandleR, bool);
    */
};

/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
#ifdef WIP_DETAILINGSYMBOLS
struct          ElevationCalloutXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_ElevationCallout_IDENTIFIER;}

    }; // ElevationCalloutXAttributeHandler
#endif

/*=================================================================================**//**
* Plan Marker Element Handler
* @bsiclass                                                     Rohit Dighe 10/2009
+===============+===============+===============+===============+===============+======*/
struct    PlanCalloutHandler : CalloutBaseHandler
{
    DEFINE_T_SUPER(CalloutBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(PlanCalloutHandler, DGNPLATFORM_EXPORT)

private:

    enum SortPriority
        {
        SORTPRIORITY_FrontDepth      = 6000,
        SORTPRIORITY_BackDepth       = 7000,
        SORTPRIORITY_MinWidth        = 8000,
        SORTPRIORITY_MaxWidth        = 9000,
        SORTPRIORITY_TopHeight       = 10000,
        SORTPRIORITY_BottomHeight    = 11000,
        SORTPRIORITY_FrontCrop       = 12000,
        SORTPRIORITY_BackCrop        = 13000,
        SORTPRIORITY_LeftCrop        = 14000,
        SORTPRIORITY_RightCrop       = 15000,
        SORTPRIORITY_TopCrop         = 16000,
        SORTPRIORITY_BottomCrop      = 17000,
        SORTPRIORITY_PreserveUp      = 18000,
        };

protected:

    virtual void                            _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                            _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override;

#ifdef WIP_DETAILINGSYMBOLS
    virtual IDependencyHandler*             _GetIDependencyHandler ()  override {return this;}

    /*
    virtual WString                         _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                       _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;

    static StatusInt                        GetMyDoubleDelegate (double&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                        SetMyDoubleDelegate (EditElementHandleR, UInt32 propId, size_t, double valueIn);
    static StatusInt                        GetMyBooleanDelegate (bool&, ElementHandleCR, UInt32 propId, size_t);
    static StatusInt                        SetMyBooleanDelegate (EditElementHandleR, UInt32 propId, size_t, bool valueIn);
    */
#endif

    virtual void                            GetImageName (WStringR name) {name.append (L"PlanMarker");}

#ifdef WIP_DETAILINGSYMBOLS
    // INamedViewCreateHelper methods
    virtual int                                     GetViewNamePrefixID ();
    virtual int                                     GetViewTypeID ();
#endif

public:
    /*
    static double                           GetDepth (ElementHandleCR, bool);
    static void                             SetDepth (EditElementHandleR, bool, double);
    static bool                             GetCropMask (ElementHandleCR, ClipVolumeCropProp);
    static void                             SetCropMask (EditElementHandleR, ClipVolumeCropProp, bool);
    static double                           GetMinWidth (ElementHandleCR);
    static void                             SetMinWidth (EditElementHandleR, double);
    static double                           GetMaxWidth (ElementHandleCR);
    static void                             SetMaxWidth (EditElementHandleR, double);
    static double                           GetTopHeight (ElementHandleCR);
    static void                             SetTopHeight (EditElementHandleR, double);
    static double                           GetBottomHeight (ElementHandleCR);
    static void                             SetBottomHeight (EditElementHandleR, double);
    static VolumeDef*                       GetVolumeDef (IDetailingSymbolPtr def);
    */
};

/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
#ifdef WIP_DETAILINGSYMBOLS
struct    PlanCalloutXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_PlanCallout_IDENTIFIER;}

    }; // PlanCalloutXAttributeHandler
#endif

/*=================================================================================**//**
* Detail Marker Element Handler
* @bsiclass                                                     SunandSandurkar 12/2006
+===============+===============+===============+===============+===============+======*/
struct          DetailCalloutHandler : CalloutBaseHandler
{
    DEFINE_T_SUPER(CalloutBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(DetailCalloutHandler, DGNPLATFORM_EXPORT)

private:

    enum SortPriority
        {
        SORTPRIORITY_TopHeight       = 9000,
        SORTPRIORITY_BottomHeight    = 10000,
        SORTPRIORITY_TopCrop         = 21000,
        SORTPRIORITY_BottomCrop      = 22000,
        };

protected:

    virtual void                            _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                            _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override;

#ifdef WIP_DETAILINGSYMBOLS
    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                         _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                       _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
    static StatusInt                        GetMyBooleanDelegate (bool&, ElementHandleCR, UInt32 propId, size_t arrayIndex);
    static StatusInt                        SetMyBooleanDelegate (EditElementHandleR, UInt32 propId, size_t arrayIndex, bool valueIn);
    static StatusInt                        GetMyDefDoubleDelegate (double&, ElementHandleCR, UInt32 propId, size_t arrayIndex);
    static StatusInt                        SetMyDefDoubleDelegate (EditElementHandleR, UInt32 propId, size_t arrayIndex, double valueIn);
    */

    virtual IDependencyHandler*             _GetIDependencyHandler ()  override {return this;}
#endif

    virtual void                            GetImageName (WStringR name) {name.append (L"DetailMarker");}

#ifdef WIP_DETAILINGSYMBOLS
    // INamedViewCreateHelper methods
    virtual int                                     GetViewNamePrefixID ();
    virtual int                                     GetViewTypeID ();
#endif

public:
    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static bool                             GetCropMask (ElementHandleCR, ClipVolumeCropProp);
    static void                             SetCropMask (EditElementHandleR, ClipVolumeCropProp, bool);
    */
};

/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
#ifdef WIP_DETAILINGSYMBOLS
struct          DetailCalloutXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_DetailCallout_IDENTIFIER;}

    }; // DetailCalloutXAttributeHandler
#endif

/*=================================================================================**//**
* Drawing View Marker Handler Class
* @bsiclass                                                     Sunand.Sandurkar 03/2007
+===============+===============+===============+===============+===============+======*/
struct          DrawingTitleHandler : DetailingSymbolBaseHandler
{
    DEFINE_T_SUPER(DetailingSymbolBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(DrawingTitleHandler, DGNPLATFORM_EXPORT)

private:

    enum SortPriority
        {
        SORTPRIORITY_DrawingName        = 6000,
        SORTPRIORITY_DrawingIdentifier  = 7000,
        SORTPRIORITY_DetailScale        = 8000,

        };

#ifdef WIP_DETAILINGSYMBOLS
    StatusInt                                       UpdateReferenceProperties (DgnAttachmentP refFile, DrawingTitleDef* drawingDef);
    void                                            ScaleRefClipPoints (DgnAttachmentP refFile, double scale);
    StatusInt                                       FlattenMarker (EditElementHandleR symbolEH);
    StatusInt                                       GetDrawingTitleData (DPoint3d* boxOrigin, DPoint3d* boxCorner, DMatrix3dP rotMatrix, ElementHandleCR elHandle) const;
#endif

protected:
    virtual void                                    _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual void                                    _GetDescription (ElementHandleCR el, WStringR descr, uint32_t desiredLength) override;

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                                 _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                               _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
    */

#ifdef WIP_DETAILINGSYMBOLS
    virtual Handler::PreActionStatus    _OnReplace (EditElementHandleR elem, ElementHandleCR oldElem) override;
    virtual void                                    _OnAdded (ElementHandleP drawingEEH);

    virtual IDependencyHandler*                     _GetIDependencyHandler ()  override {return this;}
    virtual void                                    _OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) override;
#endif

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static StatusInt                                GetMyStringDelegate (WStringR propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);
    static StatusInt                                SetMyStringDelegate (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, WCharCP valueIn);
    static StatusInt                                GetMyScaleDelegate  (ScaleInfo& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);
    static StatusInt                                SetMyScaleDelegate  (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, ScaleInfo valueIn);
    */

    virtual void                                    GetImageName (WStringR name) {name.append (L"DrawingViewMarker");}

    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    static double                                   GetDetailScale (ElementHandleCR eh);
    static void                                     SetDetailScale (EditElementHandleR eh, double valueIn);
    static void                                     GetDrawingString (WStringR str, ElementHandleCR eh, bool name);
    static void                                     SetDrawingString (EditElementHandleR eh, WCharCP str, bool name);
    */

#ifdef WIP_DETAILINGSYMBOLS
    StatusInt                                       GetClipPlanes (ClipPlaneSetH ppPlaneSet, ElementHandleCR elHandle, ClipVolumePass pass, bool displayCut);
#endif

public:

};
#endif

/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
#ifdef WIP_DETAILINGSYMBOLS
struct          DrawingTitleXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_DrawingTitle_IDENTIFIER;}

    }; // DrawingTitleXAttributeHandler
#endif

#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* Title Text Handler Class
* @bsiclass                                                     Sunand.Sandurkar 03/2007
+===============+===============+===============+===============+===============+======*/
struct          TitleTextHandler : DetailingSymbolBaseHandler
{
    DEFINE_T_SUPER(DetailingSymbolBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(TitleTextHandler, DGNPLATFORM_EXPORT)

private:
    typedef     DetailingSymbolBaseHandler          BaseClass;

#ifdef WIP_DETAILINGSYMBOLS
protected:
    /* NEEDSWORK MOVE_TO_MSTN_CLRAPPS_PROPERTIES_PROPERTYMANAGER_BASEPROPERTYENABLER
    virtual WString                                 _GetEcPropertiesClassName (ElementHandleCR) override;
    virtual StatusInt                               _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
    */
    virtual void                                    _GetTypeName (WStringR string, uint32_t desiredLength) override;
    virtual IDependencyHandler*                     _GetIDependencyHandler ()  override {return this;}
    virtual void                                    _OnRootsChanged (ElementHandleR dependent, bvector<RootChange> const& rootsChanged, bvector<XAttributeHandle> const&  xAttrsAffected) override {}
#endif

    virtual void                                    GetImageName (WStringR name) {name.append (L"TitleText");}

};

/*=================================================================================**//**
* @bsiclass                                                     Sunand.Sandurkar 03/2008
+===============+===============+===============+===============+===============+======*/
#ifdef WIP_DETAILINGSYMBOLS
struct          TitleTextXAttributeHandler  : DetailingSymbolBaseXAttributeHandler
    {
private:
    virtual uint16_t                            GetMinorID () {return SYMBOLSETTINGS_MINORID_TitleText_IDENTIFIER;}

    }; // TitleTextXAttributeHandler
#endif

/* REMOVE
namespace       BDL     = DesignLinks;
namespace       BECI    = ECObjects::Instance;
namespace       BECS    = ECObjects::Schema;
namespace       BUICWF  = UI::Controls::WinForms;
*/

#ifdef WIP_DETAILINGSYMBOLS
/*=================================================================================**//**
* @bsiclass
* NEEDSWORK: Convert to Native
+===============+===============+===============+===============+===============+======*/
struct ViewProxyDataSchemaHolder
    {
private:
    static const uint32_t       s_AppID             = XATTRIBUTEID_SymbolSettings;
    static const uint32_t       s_SubID             = SYMBOLSETTINGS_MINORID_VIEWPEPID;
    static ECN::ECSchemaPtr      s_schema;
    static WString              s_viewProxyTagClass;

public:
    static uint32_t         AppID ()                {return s_AppID;}
    static uint32_t         SubID ()                {return s_SubID;}
    static WStringCR        XML_ECUserProxyTag ()   {return s_viewProxyTagClass;}
    static ECN::ECSchemaP    GetSchema ();

    };

/*=================================================================================**//**
* @bsiclass
* NEEDSWORK: Convert to Native
+===============+===============+===============+===============+===============+======*/
struct ViewProxyData : public DgnLinkUserData
    {
protected:
    virtual DgnLinkUserDataPtr  _Copy () const          { return new ViewProxyData (); }
    virtual uint32_t            _GetAppID () const      { return ViewProxyDataSchemaHolder::AppID (); }
    virtual uint32_t            _GetSubID () const      { return ViewProxyDataSchemaHolder::SubID (); }
    virtual bool                _IsPersistent () const  { return true; }

    virtual ECN::IECInstancePtr  _ToECInstance ();
    virtual StatusInt           _FromECInstance (ECN::IECInstanceCR, DgnLinkTreeSpecCR );

public:
    ViewProxyData ()            {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ViewProxyDataHandler : public DgnLinkUserDataHandler
    {
private:
    static ViewProxyDataHandler*            s_dataHandler;

public:
    static ViewProxyDataHandler*            Get ();

    // DgnLinkUserData methods
    virtual bool                            _CanHandle (uint32_t appID, uint32_t subID);
    virtual DgnLinkUserDataPtr              _CreateUserData (uint32_t appID, uint32_t subID);
    virtual ECN::ECSchemaP                   _GetSchema ();
    virtual ECN::StandaloneECInstancePtr     _CreateECInstance (WCharCP ecClassName);
    };

typedef RefCountedPtr<ViewProxyDataHandler> ViewProxyDataHandlerPtr;

/*=================================================================================**//**
* @bsiclass                                                     SunandSandurkar 11/06
* Creates a DrawingTitle object
+===============+===============+===============+===============+===============+======*/
struct DrawingTitleBuilder
    {
private:

    DPoint3d                                    m_origin;
    DPoint3d                                    m_size;
    RotMatrix                                   m_viewMatrix;
    DPoint3d                                    m_markerOrigin;
    bool                                        m_markerOriginSpecified;
    WString                                     m_name;
    IDetailingSymbolPtr                         m_def;
    DgnAttachmentP                              m_attachment;
    DgnModelP                                   m_rootModel;
    bool                                        m_bubbleOnLeft;

    void                                        ComputeExtentsFromReference ();
    void                                        ComputeMarkerOrigin ();
    void                                        ComputeViewMatrix ();
    StatusInt                                   AddLink (EditElementHandleR symbolEEH);
    StatusInt                                   CreateMarker (EditElementHandleR symbolEEH);

public:
    DGNPLATFORM_EXPORT                          DrawingTitleBuilder (WCharCP name, DgnAttachmentP attachment, DgnModelP rootModel);
    DGNPLATFORM_EXPORT                          DrawingTitleBuilder (WCharCP name, DPoint3dCP markerOrigin, DPoint3dCR origin, DPoint3dCR size, DgnModelP rootModel);
    DGNPLATFORM_EXPORT void                     SetBubbleOnLeft (bool flag) {m_bubbleOnLeft = flag;}
    DGNPLATFORM_EXPORT StatusInt                CreateDrawing ();

    }; // DrawingTitleBuilder

/*=================================================================================**//**
* @bsiclass                                                     SunandSandurkar 11/06
+===============+===============+===============+===============+===============+======*/
struct DetailingSymbolFileAccess
    {
    private: static void                        ShowFileOpenStatusBalloon (WString const& shortMsg, WString const& longMsg, bool isError);
    private: static WString                     InsertString (WChar const*  fmt, WChar const*  str);
    public:  static StatusInt                   CheckAllowWriteToFile (WString* errMsg, DgnDbP file);
    public:  static StatusInt                   CheckAllowWriteToModel (WString* errMsg, DgnModelP model);
    public:  static StatusInt                   CheckAllowWriteToModelForLocate (WStringP cantAcceptReason, DgnModelP model);
    public:  static StatusInt                   GetReadWriteFile (DgnDbP& file, bool displayError);
    public:  DGNPLATFORM_EXPORT static StatusInt GetReadWriteFileByDgnModel (DgnModelP modelRef, bool displayError);

    }; // DetailingSymbolFileAccess

#endif
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

