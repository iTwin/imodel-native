/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/RasterHandlers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <set>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef enum
{
DisplayPriority_BackPlane      = 1, /**< Raster is on back plane  */
DisplayPriority_DesignPlane    = 2, /**< Raster is on design plane and use raster attachment element display priority  */
DisplayPriority_FrontPlane     = 3  /**< Raster is on front plane  */
} RasterDisplayPriorityPlane;


//__PUBLISH_SECTION_END__
//NEVER CHANGE THIS ENUM NUMBERS SINCE THEY ARE USED TO DEFINE XATTRIBUTE MINOR ID STORED IN FILE
enum RasterFrameXAttributesMinorId : uint16_t
    {
    CLIP_XATTR_ID                   =0,
    MASKREFERENCE_XATTR_ID          =1,
    GEOCODING_XATTR_ID              =2,
    FILENAME_XATTR_ID               =3,
    FULLREFERENCEPATH_XATTR_ID      =4,
    LOGICALNAME_XATTR_ID            =5,
    DESCRIPTION_XATTR_ID            =6,
    LAYERDISPLAYOVERRIDE_XATTR_ID   =7,
    MEMFILEBUFFER_XATTR_ID          =8,
    DEM_XATTR_ID                    =9,
    PACKAGEINIMODEL_XATTR_ID        =10,
    SOURCEMONIKER_XATTR_ID          =11,
    };
//__PUBLISH_SECTION_START__

/*=================================================================================**//**
* This class is a facility class to help compute and interpret a raster matrix (Transform).
+===============+===============+===============+===============+===============+======*/
class RasterTransformFacility
{
public:
    // Transform Facility
    DGNPLATFORM_EXPORT static Transform FromMatrixParameters(DPoint3d const& translation, double scalingX, double scalingY, double rotation, double anorthogonality);
    DGNPLATFORM_EXPORT static DVec3d    GetU(TransformCR trn);
    DGNPLATFORM_EXPORT static DVec3d    GetV(TransformCR trn);
    DGNPLATFORM_EXPORT static DVec3d    GetTranslation(TransformCR trn);
    DGNPLATFORM_EXPORT static DVec3d    GetNormal(TransformCR trn);
    DGNPLATFORM_EXPORT static double    GetScalingX(TransformCR trn);
    DGNPLATFORM_EXPORT static double    GetScalingY(TransformCR trn);
    DGNPLATFORM_EXPORT static double    GetRotationXY(TransformCR trn);
    DGNPLATFORM_EXPORT static double    GetAffinity(TransformCR trn);
    DGNPLATFORM_EXPORT static bool      Has3dRotation(TransformCR trn);
    DGNPLATFORM_EXPORT static bool      IsValidRasterTransform(TransformCR trn);

    DGNPLATFORM_EXPORT static void      RemoveRotationAndAffinity(TransformR trn);
    DGNPLATFORM_EXPORT static void      SetUV(TransformR trn, DVec3dCR u, DVec3dCR v);
    DGNPLATFORM_EXPORT static void      SetAffinity(TransformR trn, double affinity);
    DGNPLATFORM_EXPORT static void      SetScalingX(TransformR trn, double scale);
    DGNPLATFORM_EXPORT static void      SetScalingY(TransformR trn, double scale);
    DGNPLATFORM_EXPORT static void      SetTranslation(TransformR trn, DPoint3dCR translation);

    DGNPLATFORM_EXPORT static double    ComputeAngle (DVec3dCR u, DVec3dCR v);
    DGNPLATFORM_EXPORT static DPoint2d  ComputeExtent (TransformCR pixelToUORs, uint64_t bitmapWidth, uint64_t bitmapHeight);
    DGNPLATFORM_EXPORT static void      ComputeBitmapSize (uint64_t& bitmapWidth, uint64_t& bitmapHeight, TransformCR pixelToUORs, DPoint2d extentInUORs);
};

/** @addtogroup RasterElements
*
* Raster Element are used to store a reference to an external raster file or to store directly raster data into a design file.
*
* There are two kind of raster element: "Raster Attachment" and "Raster".
*
* "Raster Attachment" are used to store a reference to a raster file.
*
* "Raster" are used to store small raster bitmap or byte map directly in the design file but are very limited compare to "Raster Attachment".
*
* <h3>Raster Element Creation and Access</h3>
* \li RasterFrameHandler    - The "Raster Attachment" query interface.
* \li RasterFrameHandler     - The "Raster Attachment" creation and edit interface.
* \li RasterFrameHandler        - The "Raster Attachment" handler.
* \li RasterHdrHandler          - The "Raster" creation/access interface and handler.
*
* <h3>Collections and Iterators</h3>
* \li RasterFrameElementIterator
* \li RasterFrameElementCollection
*
* @beginGroup
*/
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard  08/2009
+===============+===============+===============+===============+===============+======*/
struct RasterFrameElement
    {
    //! Query if the element is a raster frame element (aka raster attachment).
    //! @param el   The element to query.
    //! @return true if element is a raster frame element.
    DGNPLATFORM_EXPORT static      bool        IsKindOf(DgnElementCP el);
    };


struct RasterFrameElementCollection;

/*=================================================================================**//**
* An input iterator to go through all raster frame element (aka Raster attachment) in a model.
* @bsiclass                                                     Marc.Bedard      08/2009
+===============+===============+===============+===============+===============+======*/
struct RasterFrameElementIterator : std::iterator<std::input_iterator_tag, const ElementHandle>
{
private:
    PersistentElementRefListIterator    m_it;
    ElementHandle                       m_eh;
    DgnModelP                        m_modelRefP;

    DGNPLATFORM_EXPORT void ToNext ();
    void SetElmList (PersistentElementRefList*);
    DGNPLATFORM_EXPORT RasterFrameElementIterator (PersistentElementRefList* l,DgnModelP modelRefP);
    DGNPLATFORM_EXPORT RasterFrameElementIterator ();

    bool    IsValid () const;

    friend struct RasterFrameElementCollection;
public:
    DGNPLATFORM_EXPORT RasterFrameElementIterator& operator++();
    DGNPLATFORM_EXPORT bool                        operator==(RasterFrameElementIterator const& rhs) const;
    DGNPLATFORM_EXPORT bool                        operator!=(RasterFrameElementIterator const& rhs) const;

    DGNPLATFORM_EXPORT ElementHandleCR             operator* () const;

}; // RasterFrameElementIterator


/*=================================================================================**//**
* The collection of all raster frame element  (aka Raster attachment) in a model that are scoped to that model.
* Example:
*\code
    DgnModelR model = ...

    for each (ElementHandleCR rasterEh in RasterFrameElementCollection (model))
        {
        ...
        }
\endcode
* @bsiclass                                                     Marc.Bedard      08/2009
+===============+===============+===============+===============+===============+======*/
struct  RasterFrameElementCollection
{
private:
    DgnModelR       m_model;
    DgnModelP    m_modelRefP;

public:
    typedef RasterFrameElementIterator    const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    RasterFrameElementCollection (DgnModelR model) :m_model(model),m_modelRefP(&model) {;}

    bool                        empty() const   { return begin() == end(); }

    RasterFrameElementIterator begin () const {return RasterFrameElementIterator (m_model.GetGraphicElementsP(),m_modelRefP);}
    RasterFrameElementIterator end   () const {return RasterFrameElementIterator ();}

}; // RasterFrameElementCollection

/*=================================================================================**//**
* This class is the handler for raster attachment element
* @bsiclass                                                     Mathieu.Marchand  01/2005
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE RasterFrameHandler : DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (RasterFrameHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__

    DGNPLATFORM_EXPORT         double               CalcDisplayDepth(long rasterLayer);
private:

    ViewContext::RasterPlane DisplayOrderToRasterPlane(int displayOrder);

    bool NeedDrawFill(ViewContextR ViewContextR) const;
    bool ShouldDrawInContext (ElementHandleCR rasterEh, ViewContextR ViewContextR);
    bool ShouldDrawBitmap(ViewContextR ViewContextR);
    bool ShouldDrawBorder(ElementHandleCR rasterEh, ViewContextR ViewContextR);

    void DrawBorder(ElementHandleCR rasterEh, ViewContextR ViewContextR);
    void DrawBitmap(ElementHandleCR rasterEh, ViewContextR ViewContextR);
    BentleyStatus DrawRange(ElementHandleCR rasterEh, ViewContextR ViewContextR);

protected:
DGNPLATFORM_EXPORT RasterFrameElm const* GetFrameElmCP(ElementHandleCR eh) const;
DGNPLATFORM_EXPORT RasterFrameElm*  GetFrameElmP(EditElementHandleR eeh);

// Handler
DGNPLATFORM_EXPORT virtual StatusInt            _OnTransform (EditElementHandleR element, TransformInfoCR transform) override;
DGNPLATFORM_EXPORT virtual StatusInt            _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options) override;
DGNPLATFORM_EXPORT virtual void                 _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                 _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual void                 _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                 _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
#ifdef DGN_IMPORTER_REORG_WIP
DGNPLATFORM_EXPORT virtual StatusInt            _OnPreprocessCopy (EditElementHandleR element, CopyContextP copyContext) override;
#endif
DGNPLATFORM_EXPORT virtual bool                 _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;
DGNPLATFORM_EXPORT virtual bool                 _IsTransformGraphics (ElementHandleCR element, TransformInfoCR transform) override;
DGNPLATFORM_EXPORT virtual void                 _GetDescription (ElementHandleCR el, WStringR string, uint32_t desiredLength) override;
DGNPLATFORM_EXPORT virtual void                 _GetTypeName (WStringR string, uint32_t desiredLength) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus      _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;

// //ITransactionHandler interface                                                                                                                                                 removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnAdded (ElementHandleP element) override;                                                                                     removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnDeleted (ElementHandleP element) override;                                                                                   removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, CHANGE_TRACK_Action action, bool isUndo) override;    removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnModified (ElementHandleP newElement, ElementHandleP oldElement, CHANGE_TRACK_Action action) override;                        removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnXAttributeChanged (XAttributeHandleCR xAttr, CHANGE_TRACK_Action action) override;                                           removed in Graphite
// DGNPLATFORM_EXPORT virtual void                 _OnUndoRedoXAttributeChange (XAttributeHandleCR, DgnPlatform::CHANGE_TRACK_Action  action, bool isUndo) override;               removed in Graphite

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                 _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void                 _GetTransformOrigin (ElementHandleCR, DPoint3dR);
DGNPLATFORM_EXPORT virtual void                 _GetSnapOrigin (ElementHandleCR, DPoint3dR);
DGNPLATFORM_EXPORT virtual void                 _GetOrientation (ElementHandleCR, RotMatrixR);
DGNPLATFORM_EXPORT virtual bool                 _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal);

//__PUBLISH_SECTION_START__

//__PUBLISH_SECTION_END__
public:
    DGNPLATFORM_EXPORT static WString ComposeDgnDbUrl(WCharCP filename, DgnRasterFileId rasterId);
    DGNPLATFORM_EXPORT static DgnRasterFileId RasterIdFromDgnDbUrl(WCharCP url);

    //! Change view independent display state for this attachment.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  state           Set to true if this raster display is view independent.
    //! @param[in]  reValidateRange Set to true to recompute the element range.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetViewIndependentState(EditElementHandleR eeh, bool state, bool reValidateRange);

    //! Change raster size (in UOR)
    //! Note that this field is always recomputed from transform and raster file number of pixel.
    //! @param[out] eeh         The element to modify.
    //! @param[in]  extentInUOR The new raster size in UOR.
    //! @param[in]  reValidateRange Set to true to recompute the element range.
    //! @return true if element was updated.
    bool SetExtent(EditElementHandleR eeh,DPoint2dCR extentInUOR, bool reValidateRange);

    //! Query if raster border should be display on normal DRAW.
    //! @param[in] eh        The element to query.
    //! @return true if raster border should be display on normal DRAW.
    DGNPLATFORM_EXPORT bool GetDisplayBorderState(ElementHandleCR eh) const;

    //! Change the raster border display status.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true if raster border should be display on normal DRAW.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetDisplayBorderState(EditElementHandleR eeh, bool state);

    //! Returns the version of the raster attachment element
    //! @param[in] eh        The element to query.
    DGNPLATFORM_EXPORT uint32_t GetVersion(ElementHandleCR eh) const;


    //__PUBLISH_CLASS_VIRTUAL__
    //__PUBLISH_SECTION_START__
public:

    //! Create a new Raster attachment with the supplied parameters.
    //! @param[out] eeh           The new element.
    //! @param[in]  templateEh    Initialize default properties from this template element or NULL.
    //! @param[in]  sourceUrl     URL string that identifies the raster file.
    //! @param[in]  origin        The origin of the raster attachment, also origin of u and v vector below.
    //! @param[in]  uVect         The new u vector which define direction of the raster x-axis with a length corresponding to x-axis extent (in UOR).
    //! @param[in]  vVect         The new v vector which define direction of the raster y-axis with a length corresponding to y-axis extent (in UOR).
    //! @param[in]  modelRef      Model to associate this element with. Required to compute range.
    //! @return SUCCESS if a valid element is created and range was successfully calculated.
    DGNPLATFORM_EXPORT static BentleyStatus CreateRasterAttachment (EditElementHandleR eeh, ElementHandleCP templateEh, WCharCP sourceUrl,
        DPoint3dCR origin, DVec3dCR uVect, DVec3dCR vVect, DgnModelR modelRef);

    //! Check if the matrix is a valid raster transform matrix.
    //! @param[in] matrix       The matrix to check
    //! @return true if matrix is valid.
    DGNPLATFORM_EXPORT static bool IsValidTransform(TransformCR matrix);

    //! Check if the matrix contains 3D transformation.
    //! @param[in] matrix       The matrix to check.
    //! @return true if matrix contain 3D transformation.
    DGNPLATFORM_EXPORT static bool IsTransform3D (TransformCR matrix);

    //! Query the raw color index of an RGB triplet from a DgnModel color table, add a color book entry if not found.
    //! @param[out] index         The raw color index of an RGB triplet from a DgnModel color table or color book entries.
    //! @param[in] modelRef       The DgnModel to look to for color table.
    //! @param[in] rgbColor       The RGB triplet to find index in color table.
    //! @return SUCCESS if Successful, ERROR otherwise.
    DGNPLATFORM_EXPORT static StatusInt ColorIndexFromRgbInModel (uint32_t& index, DgnModelR modelRef, RgbColorDef const& rgbColor);

    //! Query the RGB triplet from a DgnModel rawIndex.
    //! @param[out] color         the RGB triplet found.
    //! @param[in] project        The DgnDb to look to for RGB color value.
    //! @param[in] rawIndex       The rawIndex of the RGB triplet to extract.
    //! @return SUCCESS if Successful, ERROR otherwise.
    DGNPLATFORM_EXPORT static StatusInt RgbFromColorIndexInModel (RgbColorDef& color, DgnDbR project, uint32_t rawIndex);

    //! Return default search path used with the attach moniker to find attached raster file.
    //! @param[in] modelRef            Will search in this model design file folder.
    //! @return the search string.
    DGNPLATFORM_EXPORT static WString GetSearchPath (DgnModelP modelRef);

    //! Return the url string that identifies the raster file
    DGNPLATFORM_EXPORT WString GetSourceUrl (ElementHandleCR eh) const;

    //! Set the url string that identifies the raster file
    //! @param[out] eeh         The element to modify.
    //! @param[in]  sourceUrl   The url string that identify that raster file.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetSourceUrl (EditElementHandleR eeh, WCharCP sourceUrl);

    //! Query raster attachment description.
    //! @param[in] eh        The element to query.
    //! @return description.
    DGNPLATFORM_EXPORT WString GetAttachDescription(ElementHandleCR eh) const;

    //! Change raster attachment description.
    //! @param[out] eeh         The element to modify.
    //! @param[in]  description The new description.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetAttachDescription(EditElementHandleR eeh, WCharCP description);

    //! Query logical name.
    //! @param[in] eh        The element to query.
    //! @return logical name.
    DGNPLATFORM_EXPORT WString GetLogicalName(ElementHandleCR eh) const;

    //! Change logical name.
    //! @param[out] eeh         The element to modify.
    //! @param[in]  logicalName The new logical name.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetLogicalName(EditElementHandleR eeh, WCharCP logicalName);

    //! Query if snap is enable for this raster.
    //! @param[in] eh        The element to query.
    //! @return true if snap is enable for this raster.
    DGNPLATFORM_EXPORT bool GetSnappableState(ElementHandleCR eh) const;

    //! Change if snap is enable for this raster.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true if snap is enable for this raster.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetSnappableState(EditElementHandleR eeh, bool state);

    //! Query if lock is set for this raster.
    //! @param[in] eh        The element to query.
    //! @return true if lock is enable for this raster.
    DGNPLATFORM_EXPORT bool GetLockedState(ElementHandleCR eh) const;

    //! Change the lock state for this raster.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   The new lock state to set.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetLockedState(EditElementHandleR eeh, bool state);

    //! Query if this raster display is view independent.
    //! @param[in] eh        The element to query.
    //! @return true if this raster display is view independent.
    DGNPLATFORM_EXPORT bool GetViewIndependentState(ElementHandleCR eh) const;

    //! Query if the raster file should be open in read-write mode, otherwise open in read-only.
    //! @param[in] eh        The element to query.
    //! @return true if the raster file should be open in read-write mode, otherwise open in read-only.
    DGNPLATFORM_EXPORT bool GetOpenReadWrite(ElementHandleCR eh) const;

    //! Change if the raster file should be open in read-write mode or in read-only.
    //! @param[out] eeh         The element to modify.
    //! @param[in]  isWritable  Set to true if the raster file should be open in read-write mode, for read-only set it to false.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetOpenReadWrite(EditElementHandleR eeh, bool isWritable);

    //! Query raster attachment u vector which defines x axis direction and extent.
    //! @param[in]  eh          The element to query.
    //! @return The u vector of that raster attachment. Vector magnitude define x raster extent.
    DGNPLATFORM_EXPORT DVec3d       GetU(ElementHandleCR eh) const;


    //! Change raster attachment u vector which defines x axis direction and extent.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  uVector The new u vector to set. Vector magnitude define x raster extent.
    //! @param[in]  reValidateRange Set to true to recompute the element range.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetU(EditElementHandleR eeh, DVec3dCR uVector, bool reValidateRange);

    //! Query raster attachment v vector which defines y axis direction and extent.
    //! @param[in]  eh          The element to query.
    //! @return The v vector of that raster attachment. Vector magnitude define y raster extent.
    DGNPLATFORM_EXPORT DVec3d       GetV(ElementHandleCR eh) const;

    //! Change raster attachment v vector which defines y axis direction and extent.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  vVector The new v vector to set. Vector magnitude define y raster extent.
    //! @param[in]  reValidateRange Set to true to recompute the element range.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetV(EditElementHandleR eeh, DVec3dCR vVector, bool reValidateRange);

    //! Query raster attachment origin.
    //! @param[in]  eh          The element to query.
    //! @return The origin of that raster attachment.
    DGNPLATFORM_EXPORT DPoint3d     GetOrigin(ElementHandleCR eh) const;

    //! Change raster attachment origin.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  origin  The new origin to set.
    //! @param[in]  reValidateRange Set to true to recompute the element range.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetOrigin(EditElementHandleR eeh, DPoint3dCR origin, bool reValidateRange);

    //! Query raster size in UORs
    //! @param[in] eh        The element to query.
    //! @return raster size in UORs
    DGNPLATFORM_EXPORT DPoint2d GetExtent(ElementHandleCR eh) const;

    //! Query scanning resolution (in dot per inches (DPI), in x and y directions).
    //! @param[in] eh        The element to query.
    //! @return scanning resolution (in x and y directions).
    DGNPLATFORM_EXPORT DPoint2d GetScanningResolution(ElementHandleCR eh) const;

    //! Change scanning resolution (in dot per inches (DPI), in x and y directions).
    //! @param[out] eeh                     The element to modify.
    //! @param[in]  scanningResolutionDPI   The new scanning resolution (in dot per inches (DPI), in x and y directions).
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetScanningResolution(EditElementHandleR eeh,DPoint2dCR scanningResolutionDPI);

    //! Query if the raster is displayed in a specific view.
    //! @param[in] eh           The element to query.
    //! @param[in] viewNumber   The view number to query [0..7].
    //! @return true if the raster is displayed in the view.
    DGNPLATFORM_EXPORT bool GetViewState(ElementHandleCR eh,int viewNumber) const;

    //! Change if the raster is displayed in a specific view.
    //! @param[out] eeh         The element to modify.
    //! @param[in]  viewNumber  The view number to query [0..7].
    //! @param[in]  state       Set to true if the raster is displayed in the view.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetViewState(EditElementHandleR eeh,int viewNumber,bool state);


    //! Query if raster color must be inverted on display.
    //! @param[in] eh        The element to query.
    //! @return true if raster color must be inverted on display.
    DGNPLATFORM_EXPORT bool GetInvertState(ElementHandleCR eh) const;

    //! Change if raster color must be inverted on display.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true if raster color must be inverted on display.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetInvertState(EditElementHandleR eeh, bool state);

    //! Query if raster must be print.
    //! @param[in] eh        The element to query.
    //! @return true if raster must be print.
    DGNPLATFORM_EXPORT bool GetPrintState(ElementHandleCR eh) const;

    //! Change if raster must be print.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true if raster must be print.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetPrintState(EditElementHandleR eeh, bool state);


    //! Query if raster clip properties are applied at display time.
    //! @param[in] eh        The element to query.
    //! @return true if raster clip properties are applied at display time.
    DGNPLATFORM_EXPORT bool GetClipState(ElementHandleCR eh) const;

    //! Change if raster clip properties are applied at display time.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true to applied clip properties at display time.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetClipState(EditElementHandleR eeh, bool state);

    //! Query if transparency must be displayed.
    //! @param[in] eh        The element to query.
    //! @return true if transparency must be displayed.
    DGNPLATFORM_EXPORT bool GetTransparencyState(ElementHandleCR eh) const;

    //! Change if transparency must be displayed.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true  if transparency must be displayed.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetTransparencyState(EditElementHandleR eeh, bool state);

    //! Query if background and foreground color should be inverted for binary raster file during print.
    //! @remark This setting has not effect if the raster file is not binary.
    //! @param[in] eh        The element to query.
    //! @return true if background and foreground color should be inverted for binary raster file during print.
    DGNPLATFORM_EXPORT bool GetBinaryPrintInvertState(ElementHandleCR eh) const;

    //! Change if background and foreground color should be inverted for binary raster file during print.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  state   Set to true if background and foreground color should be inverted for binary raster file during print.
    //! @return true if element was updated.
    //! @remark This setting has not effect if the raster file is not binary.
    DGNPLATFORM_EXPORT bool SetBinaryPrintInvertState(EditElementHandleR eeh, bool state);

    //! Query the display order value of this raster attachment.
    //! Negative means view back plane, positive means view front plane, 0 means design plane.
    //! Raster in back and front plane are ordered by there layer number.
    //! Display order, from back to front of the view: [-1, -2, -3,...], [0] [1, 2, 3...]
    //! Two raster might have the same display order in which case the order in file will determine the final order.
    //! You should try to avoid this situation.
    //! @param[in] eh        The element to query.
    //! @return the display order of this raster attachment.
    DGNPLATFORM_EXPORT long GetDisplayOrder(ElementHandleCR eh) const;

    //! Change the layer number of this raster attachment.
    //! Raster in back and front plane are ordered by there layer number.
    //! Smaller Layer number being at the back of the view and stack to front in order.
    //! Two raster might have the same layer number in which case the order in file will determine the final order.
    //! You should try to avoid this situation.
    //! @param[out] eeh     The element to modify.
    //! @param[in]  layer   The new layer number.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetDisplayOrder(EditElementHandleR eeh, long layer);

    //! Query raw color index used as foreground color.
    //! @See RgbFromColorIndexInModel
    //! @param[in] eh        The element to query.
    //! @return raw color index used as foreground color.
    DGNPLATFORM_EXPORT uint32_t GetForegroundColor(ElementHandleCR eh) const;

    //! Change raw color index used as foreground color.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  rawColorIndex   The raw color index used as foreground color.
    //! @return true if element was updated.
    //! @See RgbFromColorIndexInModel
    //! @See ColorIndexFromRgbInModel
    DGNPLATFORM_EXPORT bool SetForegroundColor(EditElementHandleR eeh, uint32_t rawColorIndex);

    //! Query raw color index used as background color.
    //! @See RgbFromColorIndexInModel
    //! @param[in] eh        The element to query.
    //! @return raw color index used as background color.
    DGNPLATFORM_EXPORT uint32_t GetBackgroundColor(ElementHandleCR eh) const;

    //! Change raw color index used as background color.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  rawColorIndex   The raw color index used as background color.
    //! @return true if element was updated.
    //! @See RgbFromColorIndexInModel
    //! @See ColorIndexFromRgbInModel
    DGNPLATFORM_EXPORT bool SetBackgroundColor(EditElementHandleR eeh, uint32_t rawColorIndex);

    //! Query transparency value applied to raster image foreground at display [0, 255], 0 -> Opaque.
    //! @param[in] eh        The element to query.
    //! @return transparency value applied to all raster image foreground at display [0, 255], 0 -> Opaque.
    DGNPLATFORM_EXPORT uint8_t GetForegroundTransparencyLevel(ElementHandleCR eh) const;

    //! Change transparency value applied to raster image foreground at display [0,255], 0 -> Opaque.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  transparency    The transparency value applied to all raster image at display [0,255], 0 -> Opaque.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetForegroundTransparencyLevel(EditElementHandleR eeh,uint8_t transparency);

    //! Query transparency value applied to all raster image background at display [0, 255], 0 -> Opaque.
    //! @param[in] eh        The element to query.
    //! @return transparency value applied to all raster image background at display [0, 255], 0 -> Opaque.
    DGNPLATFORM_EXPORT uint8_t GetBackgroundTransparencyLevel(ElementHandleCR eh) const;

    //! Change transparency value applied to raster image background at display [0,255], 0 -> Opaque.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  transparency    The transparency value applied to all raster image at display [0,255], 0 -> Opaque.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetBackgroundTransparencyLevel(EditElementHandleR eeh,uint8_t transparency);

    //! Query transparency value applied to all raster image at display [0, 255], 0 -> Opaque.
    //! @param[in] eh        The element to query.
    //! @return transparency value applied to all raster image at display [0, 255], 0 -> Opaque.
    DGNPLATFORM_EXPORT uint8_t GetImageTransparencyLevel(ElementHandleCR eh) const;

    //! Change transparency value applied to all raster image at display [0,255], 0 -> Opaque.
    //! @param[out] eeh             The element to modify.
    //! @param[in]  transparency    The transparency value applied to all raster image at display [0,255], 0 -> Opaque.
    //! @return true if element was updated.
    DGNPLATFORM_EXPORT bool SetImageTransparencyLevel(EditElementHandleR eeh,uint8_t transparency);


}; // RasterFrameHandler
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
