/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/OleCellHeaderHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnHandlers/CellHeaderHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#if defined (NEEDS_WORK_DGNITEM)

enum DgnOleStorageType
    {
    DGNOLE_STORAGE_Linked       = 1,
    DGNOLE_STORAGE_Embedded     = 2,
    DGNOLE_STORAGE_Static       = 3,
    };

enum DgnOlePasteMethod
    {
    DGNOLE_PASTEMETHOD_ByCornerPoints       = 1,
    DGNOLE_PASTEMETHOD_ByObjectSize         = 2,
    DGNOLE_PASTEMETHOD_ByObjectMinTextSize  = 3
    };

enum DgnOleViewRotationMode
    {
    DGNOLE_ViewRotation_ViewIndependent     = 0,
    DGNOLE_ViewRotation_ViewDependent       = 1,
    DGNOLE_ViewRotation_AutoCAD             = 2
    };

struct HiMetricSizeShort
    {
    UInt16      x;
    UInt16      y;
    };

struct HiMetricSizeLong
    {
    UInt32      x;
    UInt32      y;
    };

struct DgnOleFlags
    {
    UInt32      transparent:1;
    UInt32      selectable:1;
    UInt32      activateable:1;
    UInt32      insertedObj:1;
    UInt32      doNotDisplayOleObj:1;           // the OLE object is not displayed
    UInt32      viewRotationMode:2;
    UInt32      lockedAspectRatio:1;
    };

struct DgnOleInfo
    {
    UInt32              m_version;
    UInt32              m_reserved;
    UInt32              m_oleAspect;
    HiMetricSizeLong    m_defaultSize;      // size of OLE obj in .001 MM (for restoring to "default" size)
    DgnOleStorageType   m_storageType;
    DgnOleFlags         m_flags;
    double              m_reserved1[4];
    };

enum DgnOleStatus
    {
    DGNOLE_SUCCESS                  = 0,
    DGNOLE_ERROR_BadElement         = 1,
    DGNOLE_ERROR_AlreadyActive      = 2,
    DGNOLE_ERROR_NotOleElement      = 3,
    DGNOLE_ERROR_LockBytesError     = 4,
    DGNOLE_ERROR_BadOleObject       = 5,
    DGNOLE_ERROR_CantActivate       = 6,
    DGNOLE_ERROR_StaticOleElement   = 7,
    DGNOLE_ERROR_ElementInReference = 8,
    DGNOLE_ERROR_CantResolveLink    = 9,
    DGNOLE_ERROR_CantConnectToLink  = 10,
    DGNOLE_ERROR_ClassDifferent     = 11,
    DGNOLE_ERROR_NoObjectInLink     = 12,
    };

/*=================================================================================**//**
* An ole element is a sub-type of cell. The ole element defines a location,
* orientation, and rectangular area in which ole data is to be displayed.
* @ingroup DisplayHandler
* @bsiclass                                                     Sam.Wilson      10/2004
+===============+===============+===============+===============+===============+======*/
struct          OleCellHeaderHandler : Type2Handler
                                       //__PUBLISH_SECTION_END__
                                       ,ISubTypeHandlerQuery
                                       //__PUBLISH_SECTION_START__
{
    DEFINE_T_SUPER(Type2Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (OleCellHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;
virtual void _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
virtual void _GetPathDescription (ElementHandleCR el, WStringR string, DisplayPathCP path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiter) override;
virtual StatusInt _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;

// DisplayHandler
virtual void _Draw (ElementHandleCR, ViewContextR) override;
virtual bool _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
virtual void _GetOrientation (ElementHandleCR, RotMatrixR) override;

// ISubTypeHandlerQuery
virtual bool _ClaimElement (ElementHandleCR) override;

public:

DGNPLATFORM_EXPORT DgnOleInfo GetInfo (ElementHandleCR eh);
DGNPLATFORM_EXPORT DgnOleFlags GetFlags (ElementHandleCR eh);

DGNPLATFORM_EXPORT void SetTransparent (EditElementHandleR, bool);
DGNPLATFORM_EXPORT void SetActivateable (EditElementHandleR, bool);
DGNPLATFORM_EXPORT void SetSelectable (EditElementHandleR, bool);
DGNPLATFORM_EXPORT void SetLockedAspectRatio (EditElementHandleR, bool);
DGNPLATFORM_EXPORT void SetRotationMode (EditElementHandleR, DgnOleViewRotationMode);
DGNPLATFORM_EXPORT StatusInt SetScale (EditElementHandleR el, DPoint2dCR scale);

DGNPLATFORM_EXPORT bool ExtractShapePts (ElementHandleCR el, ViewContextR context, DPoint3dP shapePts);
DGNPLATFORM_EXPORT bool          ExtractShapePts (ElementHandleCR el, DPoint3dP shapePts);
DGNPLATFORM_EXPORT void PlotRasterized (ElementHandleCR, ViewContextR, double plotRasterScale);
DGNPLATFORM_EXPORT void PlotNonRasterized (ElementHandleCR, ViewContextR, void* hdc, const BSIRect& olePlotRange);

DGNPLATFORM_EXPORT static bool IsObjActive (ElementRefP elemRef);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Test if the supplied element is an ole cell element.
//! @param eh IN The element to test.
//! @param info OUT ole information (optional).
//! @return true if eh is a valid ole element.
DGNPLATFORM_EXPORT static bool IsOleElement (ElementHandleCR eh, DgnOleInfo* info);

//! Query extents, center, and orientation of the ole element.
//! @param eh IN The element to query.
//! @param size OUT Extents of ole element (optional).
//! @param center OUT Center of ole element (optional).
//! @param rMatrix OUT Orientation of ole element (optional).
//! @return SUCCESS if extracted data is valid.
DGNPLATFORM_EXPORT StatusInt GetSize (ElementHandleCR eh, DPoint2dP size, DPoint3dP center, RotMatrixP rMatrix);

//! Query the source data size for the ole data.
//! @param eh IN The element to query.
//! @param sourceSize OUT Source data size.
//! @param label OUT Unit label for source data size.
//! @return SUCCESS if extracted data is valid.
DGNPLATFORM_EXPORT StatusInt GetSourceSize (ElementHandleCR eh, DPoint2dR sourceSize, WStringP label);

//! Query the display scale for the ole data.
//! @param eh IN The element to query.
//! @param scale OUT display scale.
//! @return SUCCESS if extracted data is valid.
DGNPLATFORM_EXPORT StatusInt GetScale (ElementHandleCR eh, DPoint2dR scale);

//! Extract the ole data to supply as the ILockBytes read buffer.
//! @param eh IN The element to test.
//! @param dataPP OUT Returned pointer to buffer of allocated ole data.
//! @param dataSizeP OUT The size of the extracted data in bytes.
DGNPLATFORM_EXPORT BentleyStatus ExtractOleData (ElementHandleCR eh, void** dataPP, UInt32* dataSizeP);

//! Call to free the extracted ole data.
//! @param dataP IN The extracted data to free.
DGNPLATFORM_EXPORT void FreeExtractedOleData (void* dataP);

//! Create a new ole element from the supplied parameters.
//! @param eeh OUT The new element.
//! @param info IN ole information.
//! @param name IN optional name added to ole element.
//! @param description IN optional description added to ole element.
//! @param shape IN The 5 points for the bounding shape.
//! @param oleData IN byte buffer pointer to ole data.
//! @param oleSize IN Size in bytes of oleData.
//! @param is3d IN Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
//! @param[in] modelRef Model to associate this element with. Required to compute range.
//! @return SUCCESS if a valid element is created and range was sucessfully calculated.
DGNPLATFORM_EXPORT static BentleyStatus CreateOleElement (EditElementHandleR eeh, DgnOleInfo const* info, WCharCP name, WCharCP description, DPoint3dCP shape, void const* oleData, size_t oleSize, bool is3d, DgnModelR modelRef);
};
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
