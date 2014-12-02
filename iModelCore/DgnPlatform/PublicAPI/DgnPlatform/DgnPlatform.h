/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

/** @namespace BentleyApi::DgnPlatform Types defined by the %DgnPlatform. */

/** @cond BENTLEY_SDK_Internal */

#include <BentleyApi/BentleyApi.h>
#include <Bentley/RefCounted.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeFileName.h>
#include "ExportMacros.h"
#include <Geom/GeomApi.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include <Geom/IntegerTypes/BSIRect.h>
#include <Geom/IntegerTypes/Point.h>
#include "DgnPlatform.r.h"
#include "DgnPlatformErrors.r.h"
#include "DgnHost.h"
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ECDb/ECDbApi.h>

#define LEVEL_NULL_ID                       (BentleyApi::DgnPlatform::LevelId(0xffffffff))
#define LEVEL_DEFAULT_LEVEL_ID              (BentleyApi::DgnPlatform::LevelId(64))

#define USING_NAMESPACE_BENTLEY_DGNPLATFORM using namespace BentleyApi::DgnPlatform;

#define USING_NAMESPACE_EC                  using namespace BentleyApi::ECN;
#define USING_NAMESPACE_BENTLEY_EC          using namespace BentleyApi::ECN;

#define BEGIN_RASTER_NAMESPACE              BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace Raster {
#define END_RASTER_NAMESPACE                } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_NAMESPACE_RASTER              using namespace BentleyApi::DgnPlatform::Raster;

#define BEGIN_BENTLEY_POINTCLOUD_NAMESPACE  BEGIN_BENTLEY_API_NAMESPACE namespace PointCloud {
#define END_BENTLEY_POINTCLOUD_NAMESPACE    } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_POINTCLOUD  using namespace BentleyApi::PointCloud;

#define BEGIN_BENTLEY_DGNPLATFORM_TEXTEDITOR_NAMESPACE  BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace TextEditor {
#define END_BENTLEY_DGNPLATFORM_TEXTEDITOR_NAMESPACE    } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGNPLATFORM_TEXTEDITOR  using namespace BentleyApi::DgnPlatform::TextEditor;

#define BEGIN_FOREIGNFORMAT_NAMESPACE       BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace ForeignFormat {
#define END_FOREIGNFORMAT_NAMESPACE         } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_NAMESPACE_FOREIGNFORMAT       using namespace BentleyApi::DgnPlatform::ForeignFormat;

#define FOREIGNFORMAT_TYPEDEFS(t) \
    BEGIN_FOREIGNFORMAT_NAMESPACE struct t; END_FOREIGNFORMAT_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS (DgnPlatform::ForeignFormat,t);

#define BEGIN_DGNV8_NAMESPACE               BEGIN_FOREIGNFORMAT_NAMESPACE namespace DgnV8 {
#define END_DGNV8_NAMESPACE                 } END_FOREIGNFORMAT_NAMESPACE
#define USING_NAMESPACE_DGNV8               using namespace BentleyApi::DgnPlatform::ForeignFormat::DgnV8;

#define FOREIGNFORMAT_DGNV8_TYPEDEFS(t) \
    BEGIN_DGNV8_NAMESPACE struct t; END_DGNV8_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS (DgnPlatform::ForeignFormat::DgnV8,t);

#define GLOBAL_TYPEDEF1(_sName_,_name_,structunion) \
    structunion _sName_; \
    namespace BENTLEY_API_NAMESPACE_NAME {\
    typedef structunion _sName_*          _name_##P, &_name_##R;  \
    typedef structunion _sName_ const*    _name_##CP; \
    typedef structunion _sName_ const&    _name_##CR;}

#define GLOBAL_TYPEDEF(_sName_,_name_) GLOBAL_TYPEDEF1 (_sName_,_name_,struct)

#define DGNPLATFORM_TYPEDEFS_EX(_name_,_structunion_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE _structunion_ _name_; END_BENTLEY_DGNPLATFORM_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS1(DgnPlatform,_name_,_name_,_structunion_)

#define DGNPLATFORM_TYPEDEFS(_name_) DGNPLATFORM_TYPEDEFS_EX(_name_,struct)

#define DGNPLATFORM_TYPEDEF(_sname_,_tname_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE struct _sname_; END_BENTLEY_DGNPLATFORM_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE typedef struct DgnPlatform::_sname_* _tname_; END_BENTLEY_API_NAMESPACE

#define DGNPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE struct _sname_; END_BENTLEY_DGNPLATFORM_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE typedef RefCountedPtr<DgnPlatform::_sname_> _sname_##Ptr; END_BENTLEY_API_NAMESPACE

#define DGNPLATFORM_CLASS_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE class _name_; END_BENTLEY_DGNPLATFORM_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS1(DgnPlatform,_name_,_name_,class)

#define RASTER_TYPEDEFS(t) \
    BEGIN_RASTER_NAMESPACE struct t; END_RASTER_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS (DgnPlatform::Raster,t);

#define RASTER_REF_COUNTED_PTR(_sname_) \
    BEGIN_RASTER_NAMESPACE struct _sname_; END_RASTER_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE typedef RefCountedPtr<DgnPlatform::Raster::_sname_> _sname_##Ptr; END_BENTLEY_API_NAMESPACE

#define RASTER_TYPEDEF1(_sourceName_,_name_,_structclass_) \
    BEGIN_RASTER_NAMESPACE _structclass_ _sourceName_; END_RASTER_NAMESPACE  \
    ADD_BENTLEY_API_TYPEDEFS1(DgnPlatform::Raster,_sourceName_,_name_,_structclass_)

#define GEOCOORD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_API_NAMESPACE namespace GeoCoordinates { struct _name_; } END_BENTLEY_API_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS(GeoCoordinates, _name_)

/** @endcond */

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// Global types
// ///////////////////////////////////////////////////////////////////////////////////////////////////

/** @cond BENTLEY_SDK_Internal */
GLOBAL_TYPEDEF (QvElem,QvElem)
GLOBAL_TYPEDEF (QvCache,QvCache)
GLOBAL_TYPEDEF (QvView,QvView)
GLOBAL_TYPEDEF (QvMRImage,QvMRImage)
/** @endcond */

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// Bentley types
// ///////////////////////////////////////////////////////////////////////////////////////////////////

/** @cond BENTLEY_SDK_Internal */
BENTLEY_NAMESPACE_TYPEDEFS (IRefCounted)

BENTLEY_API_TYPEDEFS (BitMask)
BENTLEY_API_TYPEDEFS (DataExternalizer)
BENTLEY_API_TYPEDEFS (GPArray)
BENTLEY_API_TYPEDEFS (GraphicsPointArray)
BENTLEY_API_TYPEDEFS (MultiStateMask)
/** @endcond */

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// DgnPlatform types
//__PUBLISH_SECTION_END__
//
//  NOTE: add new types to the appropriate section (organized by SDK type).
//
//__PUBLISH_SECTION_START__
// ///////////////////////////////////////////////////////////////////////////////////////////////////

DGNPLATFORM_TYPEDEFS (CachedDrawHandle)
DGNPLATFORM_TYPEDEFS (CachedGraphics)
DGNPLATFORM_TYPEDEFS (CachedGraphicsCreator)
DGNPLATFORM_TYPEDEFS (DgnModel)
DGNPLATFORM_TYPEDEFS (DgnProject)
DGNPLATFORM_TYPEDEFS (DgnMarkupProject)
DGNPLATFORM_TYPEDEFS (DgnModelSelection)
DGNPLATFORM_TYPEDEFS (DgnResourceURI)
DGNPLATFORM_TYPEDEFS (DisplayStyle)
DGNPLATFORM_TYPEDEFS (DisplayStyleFlags)
DGNPLATFORM_TYPEDEFS (ElementRef)
DGNPLATFORM_TYPEDEFS (GradientSymb)
DGNPLATFORM_TYPEDEFS (ICurvePathQuery)
DGNPLATFORM_TYPEDEFS (IDrawGeom)
DGNPLATFORM_TYPEDEFS (IElemTopology)
DGNPLATFORM_TYPEDEFS (IViewDraw)
DGNPLATFORM_TYPEDEFS (IViewOutput)
DGNPLATFORM_TYPEDEFS (LineStyleSymb)
DGNPLATFORM_TYPEDEFS (PersistentElementRef)
DGNPLATFORM_TYPEDEFS (PhysicalRedlineModel)
DGNPLATFORM_TYPEDEFS (RedlineModel)
DGNPLATFORM_TYPEDEFS (ViewContext)
DGNPLATFORM_TYPEDEFS (ViewDisplayOverrides)
DGNPLATFORM_TYPEDEFS (ViewFlags)
DGNPLATFORM_TYPEDEFS (ViewController)
DGNPLATFORM_TYPEDEFS (ViewPortInfo)
DGNPLATFORM_TYPEDEFS (Viewport)

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_TYPEDEFS_EX (AssocGeom,union)
DGNPLATFORM_TYPEDEFS (AssocPoint)
DGNPLATFORM_TYPEDEFS (Caret)
DGNPLATFORM_TYPEDEFS (ChangeAnnotationScale)
DGNPLATFORM_TYPEDEFS (ClipPrimitive);
DGNPLATFORM_TYPEDEFS (ClipVector);
DGNPLATFORM_TYPEDEFS (ClipVolumeOverrides)
DGNPLATFORM_TYPEDEFS (CookedDisplayStyle)
DGNPLATFORM_TYPEDEFS (CutGraphicsCachedKey)
DGNPLATFORM_TYPEDEFS (DependencyLinkage)
DGNPLATFORM_TYPEDEFS (DgnButtonEvent)
DGNPLATFORM_TYPEDEFS (DgnColorMap)
DGNPLATFORM_TYPEDEFS (DgnDimStyle)
DGNPLATFORM_TYPEDEFS (DgnDomain)
DGNPLATFORM_TYPEDEFS (DgnElementHeader)
DGNPLATFORM_TYPEDEFS (DgnFlickEvent)
DGNPLATFORM_TYPEDEFS (DgnFont)
DGNPLATFORM_TYPEDEFS (DgnFontCatalog)
DGNPLATFORM_TYPEDEFS (DgnFontManager)
DGNPLATFORM_TYPEDEFS (DgnGestureEvent)
DGNPLATFORM_TYPEDEFS (DgnGlyph)
DGNPLATFORM_TYPEDEFS (DgnElement)
DGNPLATFORM_TYPEDEFS (DgnHost)
DGNPLATFORM_TYPEDEFS (DgnModelList)
DGNPLATFORM_TYPEDEFS (DgnMouseWheelEvent)
DGNPLATFORM_TYPEDEFS (Dgn3DInputEvent)
DGNPLATFORM_TYPEDEFS (DgnPlatformIntegration)
DGNPLATFORM_TYPEDEFS (DgnPlatformIntegrationList)
DGNPLATFORM_TYPEDEFS (DgnProgressMeter)
DGNPLATFORM_TYPEDEFS (DgnShxFontManager)
DGNPLATFORM_TYPEDEFS (DerivedElementRange)
DGNPLATFORM_TYPEDEFS (DisplayHandler)
DGNPLATFORM_TYPEDEFS (DisplayPath)
DGNPLATFORM_TYPEDEFS (DisplayPrioritySettings)
DGNPLATFORM_TYPEDEFS (DisplayFilterKey)
DGNPLATFORM_TYPEDEFS (DisplayFilterHandler)
DGNPLATFORM_TYPEDEFS (DisplayFilterHandlerManager)
DGNPLATFORM_TYPEDEFS (DrawContext)
DGNPLATFORM_TYPEDEFS (DrawingModel)
DGNPLATFORM_TYPEDEFS (DropGeometry)
DGNPLATFORM_TYPEDEFS (DropGraphics)
DGNPLATFORM_TYPEDEFS (DwgHatchDef)
DGNPLATFORM_TYPEDEFS (DwgHatchDefLine)
DGNPLATFORM_TYPEDEFS (DynamicViewSettings)
DGNPLATFORM_TYPEDEFS (EditElementHandle)
DGNPLATFORM_TYPEDEFS (DgnGraphics)
DGNPLATFORM_TYPEDEFS (PhysicalGraphics)
DGNPLATFORM_TYPEDEFS (DrawingGraphics)
DGNPLATFORM_TYPEDEFS (ElemDisplayParams)
DGNPLATFORM_TYPEDEFS (ElemHeaderOverrides)
DGNPLATFORM_TYPEDEFS (ElemMatSymb)
DGNPLATFORM_TYPEDEFS (ElementAgenda)
DGNPLATFORM_TYPEDEFS (ElementHandle)
DGNPLATFORM_TYPEDEFS (ElementLocateManager)
DGNPLATFORM_TYPEDEFS (ElementPropertiesGetter)
DGNPLATFORM_TYPEDEFS (ElementPropertiesSetter)
DGNPLATFORM_TYPEDEFS (FenceManager)
DGNPLATFORM_TYPEDEFS (FenceParams)
DGNPLATFORM_TYPEDEFS (Frustum)
DGNPLATFORM_TYPEDEFS (GeomDetail)
DGNPLATFORM_TYPEDEFS (GradientSettings)
DGNPLATFORM_TYPEDEFS (HatchLinkage)
DGNPLATFORM_TYPEDEFS (HitList)
DGNPLATFORM_TYPEDEFS (HitPath)
DGNPLATFORM_TYPEDEFS (IACSManager)
DGNPLATFORM_TYPEDEFS (IAnnotationHandler)
DGNPLATFORM_TYPEDEFS (IAuxCoordSys)
DGNPLATFORM_TYPEDEFS (ICachedDraw)
DGNPLATFORM_TYPEDEFS (IDeleteManipulator)
DGNPLATFORM_TYPEDEFS (IDisplaySymbol)
DGNPLATFORM_TYPEDEFS (IDragManipulator)
DGNPLATFORM_TYPEDEFS (IEditAction)
DGNPLATFORM_TYPEDEFS (IEditActionArray)
DGNPLATFORM_TYPEDEFS (IEditActionSource)
DGNPLATFORM_TYPEDEFS (IElementGraphicsProcessor)
DGNPLATFORM_TYPEDEFS (IElementSet)
DGNPLATFORM_TYPEDEFS (IElementState)
DGNPLATFORM_TYPEDEFS (ILineStyle)
DGNPLATFORM_TYPEDEFS (ILineStyleComponent)
DGNPLATFORM_TYPEDEFS (IMaterialAnnouncer)
DGNPLATFORM_TYPEDEFS (IMaterialProvider)
DGNPLATFORM_TYPEDEFS (IMaterialStore)
DGNPLATFORM_TYPEDEFS (IMRImageTileEventHandler)
DGNPLATFORM_TYPEDEFS (IPickGeom)
DGNPLATFORM_TYPEDEFS (IPropertyManipulator)
DGNPLATFORM_TYPEDEFS (IRasterSourceFileQuery)
DGNPLATFORM_TYPEDEFS (ISolidKernelEntity)
DGNPLATFORM_TYPEDEFS (ISubEntity)
DGNPLATFORM_TYPEDEFS (IFaceMaterialAttachments)
DGNPLATFORM_TYPEDEFS (ISprite)
DGNPLATFORM_TYPEDEFS (ITextEdit)
DGNPLATFORM_TYPEDEFS (ITextPartId)
DGNPLATFORM_TYPEDEFS (ITextQuery)
DGNPLATFORM_TYPEDEFS (ITextQueryOptions)
DGNPLATFORM_TYPEDEFS (ITiledRaster)
DGNPLATFORM_TYPEDEFS (ITransactionHandler)
DGNPLATFORM_TYPEDEFS (ITransformManipulator)
DGNPLATFORM_TYPEDEFS (ITxnManager)
DGNPLATFORM_TYPEDEFS (IVertexManipulator)
DGNPLATFORM_TYPEDEFS (IViewHandlerHitInfo)
DGNPLATFORM_TYPEDEFS (IViewManager)
DGNPLATFORM_TYPEDEFS (IViewTransients)
DGNPLATFORM_TYPEDEFS (IndexedViewSet)
DGNPLATFORM_TYPEDEFS (IndexedViewport)
DGNPLATFORM_TYPEDEFS (LineStyleInfo)
DGNPLATFORM_TYPEDEFS (LineStyleManager)
DGNPLATFORM_TYPEDEFS (LineStyleParams)
DGNPLATFORM_TYPEDEFS (LinkageHeader)
DGNPLATFORM_TYPEDEFS (LsStroke)
DGNPLATFORM_TYPEDEFS (LsSymbolReference)
DGNPLATFORM_TYPEDEFS (LsComponent)
DGNPLATFORM_TYPEDEFS (LsPointComponent)
DGNPLATFORM_TYPEDEFS (LsStrokePatternComponent)
DGNPLATFORM_TYPEDEFS (LsInternalComponent)
DGNPLATFORM_TYPEDEFS (LsSymbolComponent)
DGNPLATFORM_TYPEDEFS (LsCompoundComponent)
DGNPLATFORM_TYPEDEFS (LsDefinition)
DGNPLATFORM_TYPEDEFS (LsMap)
DGNPLATFORM_TYPEDEFS (LsSystemMap)
DGNPLATFORM_TYPEDEFS (LsDgnProjectMap)
DGNPLATFORM_TYPEDEFS (LineStyleManager)
DGNPLATFORM_TYPEDEFS (LsMapIterator)
DGNPLATFORM_TYPEDEFS (LsMapEntry)
DGNPLATFORM_TYPEDEFS (MSElementDescr)
DGNPLATFORM_TYPEDEFS (MSElementDescrVec)
DGNPLATFORM_TYPEDEFS (Material)
DGNPLATFORM_TYPEDEFS (MaterialAssignment)
DGNPLATFORM_TYPEDEFS (MaterialColorMask)
DGNPLATFORM_TYPEDEFS (MaterialManager)
DGNPLATFORM_TYPEDEFS (MaterialPreviewCollection)
DGNPLATFORM_TYPEDEFS (ModelInfo)
DGNPLATFORM_TYPEDEFS (DgnModelIterator)
DGNPLATFORM_TYPEDEFS (NotificationManager)
DGNPLATFORM_TYPEDEFS (OvrMatSymb)
DGNPLATFORM_TYPEDEFS (ParagraphProperties)
DGNPLATFORM_TYPEDEFS (PatternParams)
DGNPLATFORM_TYPEDEFS (PermanentTopologicalId)
DGNPLATFORM_TYPEDEFS (PersistentElementPath)
DGNPLATFORM_TYPEDEFS (PersistentSnapPath)
DGNPLATFORM_TYPEDEFS (PhysicalModel)
DGNPLATFORM_TYPEDEFS (PhysicalViewController)
DGNPLATFORM_TYPEDEFS (PhysicalRedlineViewController)
DGNPLATFORM_TYPEDEFS (PropertyContext)
DGNPLATFORM_TYPEDEFS (ProxyCurveId)
DGNPLATFORM_TYPEDEFS (ProxyDisplayCacheBase)
DGNPLATFORM_TYPEDEFS (ProxyDisplayPath)
DGNPLATFORM_TYPEDEFS (ProxyEdgeId)
DGNPLATFORM_TYPEDEFS (ProxyEdgeIdData)
DGNPLATFORM_TYPEDEFS (ProxyHLEdgeId)
DGNPLATFORM_TYPEDEFS (ProxyHLEdgeSegmentId)
DGNPLATFORM_TYPEDEFS (QueryModel)
DGNPLATFORM_TYPEDEFS (QVAliasMaterialId)
DGNPLATFORM_TYPEDEFS (QueryViewController)
DGNPLATFORM_TYPEDEFS (QvUnsizedKey)
DGNPLATFORM_TYPEDEFS (QvViewport)
DGNPLATFORM_TYPEDEFS (RedlineViewController)
DGNPLATFORM_TYPEDEFS (RegionGraphicsContext)
DGNPLATFORM_TYPEDEFS (RunProperties)
DGNPLATFORM_TYPEDEFS (RunPropertiesBase)
DGNPLATFORM_TYPEDEFS (ScanCriteria)
DGNPLATFORM_TYPEDEFS (SelectionPath)
DGNPLATFORM_TYPEDEFS (SelectionSetManager)
DGNPLATFORM_TYPEDEFS (SheetDef)
DGNPLATFORM_TYPEDEFS (SheetViewController)
DGNPLATFORM_TYPEDEFS (SnapContext)
DGNPLATFORM_TYPEDEFS (SnapPath)
DGNPLATFORM_TYPEDEFS (Symbology)
DGNPLATFORM_TYPEDEFS (SymbologyOverrides)
DGNPLATFORM_TYPEDEFS (StampQvElemMap)
DGNPLATFORM_TYPEDEFS (TextBlock)
DGNPLATFORM_TYPEDEFS (TextBlockProperties)
DGNPLATFORM_TYPEDEFS (TextParamWide)
DGNPLATFORM_TYPEDEFS (TextString)
DGNPLATFORM_TYPEDEFS (TextStringProperties)
DGNPLATFORM_TYPEDEFS (LegacyTextStyle)
DGNPLATFORM_TYPEDEFS (LegacyTextStyleOverrideFlags)
DGNPLATFORM_TYPEDEFS (TransformClipStack)
DGNPLATFORM_TYPEDEFS (TransformInfo)
DGNPLATFORM_TYPEDEFS (TxnSummary)
DGNPLATFORM_TYPEDEFS (UnitInfo)
DGNPLATFORM_TYPEDEFS (UpdateContext)
DGNPLATFORM_TYPEDEFS (ViewportApplyOptions)
DGNPLATFORM_TYPEDEFS (VisibleEdgeCache)
DGNPLATFORM_TYPEDEFS (XAttributeHandle)
DGNPLATFORM_TYPEDEFS (XAttributeHandler)
DGNPLATFORM_TYPEDEFS (XAttributeHandlerId)
DGNPLATFORM_TYPEDEFS (XAttributesHolder)
DGNPLATFORM_TYPEDEFS (XGraphicsContainer)
DGNPLATFORM_TYPEDEFS (XGraphicsSymbol)
DGNPLATFORM_TYPEDEFS (XGraphicsSymbolId)
DGNPLATFORM_TYPEDEFS (XGraphicsSymbolCache)
DGNPLATFORM_TYPEDEFS (XGraphicsSymbolStamp)
DGNPLATFORM_TYPEDEFS (XGraphicsOperationContext)
DGNPLATFORM_CLASS_TYPEDEFS (Handler);
/** @endcond */

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// GeoCoord types
// ///////////////////////////////////////////////////////////////////////////////////////////////////

/** @cond BENTLEY_SDK_Internal */
GEOCOORD_TYPEDEFS (IGeoCoordinateServices)
GEOCOORD_TYPEDEFS (DgnGCS)
/** @endcond */

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// RefCountedPtr types
// ///////////////////////////////////////////////////////////////////////////////////////////////////

DGNPLATFORM_REF_COUNTED_PTR (CachedGraphics)
DGNPLATFORM_REF_COUNTED_PTR (DgnMarkupProject)
DGNPLATFORM_REF_COUNTED_PTR (DgnProject)
DGNPLATFORM_REF_COUNTED_PTR (ElementRef)
DGNPLATFORM_REF_COUNTED_PTR (MSElementDescr)
DGNPLATFORM_REF_COUNTED_PTR (PersistentElementRef)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR (RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_REF_COUNTED_PTR (ClipPrimitive);
DGNPLATFORM_REF_COUNTED_PTR (ClipVector);
DGNPLATFORM_REF_COUNTED_PTR (DgnBaseMoniker)
DGNPLATFORM_REF_COUNTED_PTR (DgnBaseMonikerList)
DGNPLATFORM_REF_COUNTED_PTR (DgnPlatformIntegration)
DGNPLATFORM_REF_COUNTED_PTR (DgnPlatformIntegrationList)
DGNPLATFORM_REF_COUNTED_PTR (DisplayFilterKey)
DGNPLATFORM_REF_COUNTED_PTR (DisplayPath)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)
DGNPLATFORM_REF_COUNTED_PTR (DisplayFilterKey)
DGNPLATFORM_REF_COUNTED_PTR (DisplayStyleHandlerSettings)
DGNPLATFORM_REF_COUNTED_PTR (DrawingGraphics)
DGNPLATFORM_REF_COUNTED_PTR (IProgressiveDisplay)
DGNPLATFORM_REF_COUNTED_PTR (IRasterSourceFileQuery)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalGraphics)
DGNPLATFORM_REF_COUNTED_PTR (ViewController)
DGNPLATFORM_REF_COUNTED_PTR (ViewPortInfo)
DGNPLATFORM_REF_COUNTED_PTR (XGraphicsSymbolStamp)
/** @endcond */

// ---------------------------------------------------------------------------------------------------------

/** @cond BENTLEY_SDK_Internal */

struct mdlDesc;

// Define other pointer types in the Bentley namespace.
BEGIN_BENTLEY_API_NAMESPACE

typedef mdlDesc MdlDesc;
typedef mdlDesc* MdlDescP;
typedef struct DgnPlatform::MSElementDescr**  MSElementDescrH;
typedef struct DgnPlatform::LegacyTextStyle         MdlTextStyle;

enum
{
    DGNPLATFORM_RESOURCE_MAXFILELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXDIRLENGTH                     = 256,
    DGNPLATFORM_RESOURCE_MAXNAMELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH               = 256,

    MAXFILELENGTH                   = DGNPLATFORM_RESOURCE_MAXFILELENGTH,
    MAXDIRLENGTH                    = DGNPLATFORM_RESOURCE_MAXDIRLENGTH,
    MAXDEVICELENGTH                 = 256,
    MAXNAMELENGTH                   = DGNPLATFORM_RESOURCE_MAXNAMELENGTH,
    MAXEXTENSIONLENGTH              = DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH,
};

/*__PUBLISH_SECTION_END__*/
namespace DgnPlatform {struct _dVector2d; struct _dVector3d;}
typedef struct DgnPlatform::_dVector2d  DVector2d;
typedef struct DgnPlatform::_dVector2d  Dvector2d;
typedef struct DgnPlatform::_dVector3d  DVector3d;
typedef struct DgnPlatform::_dVector3d  Dvector3d;
typedef struct DgnPlatform::_dVector3d* DVector3dP;
typedef struct DgnPlatform::_dVector3d const *DVector3dCP;
typedef struct DgnPlatform::_dVector3d& DVector3dR;
typedef struct DgnPlatform::_dVector3d const& DVector3dCR;
typedef struct DgnPlatform::_dVector2d* DVector2dP;
typedef struct DgnPlatform::_dVector2d const *DVector2dCP;

//__PUBLISH_SECTION_START__
END_BENTLEY_API_NAMESPACE

/** @endcond */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! An ID that is assigned to a DgnModel. See DgnProject#Models, DgnModel.
struct DgnModelId : BeInt64Id<DgnModelId>
{
    DgnModelId() {Invalidate();}
    explicit DgnModelId(Int64 u) : BeInt64Id (u) {}
    DgnModelId(BeRepositoryId repositoryId, UInt32 id) : BeInt64Id (repositoryId,id) {}
    DgnModelId(DgnModelId const& rhs) : BeInt64Id<DgnModelId>(rhs) {}
    DgnModelId& operator=(DgnModelId const& rhs) {m_id = rhs.m_id; return *this;}
    bool Validate() const {return m_id!=-1;}
    void Invalidate() {m_id = -1;}
};

//! An ID that is assigned to a DgnLevel.
struct LevelId : BeUInt32Id<LevelId,0>
{
    LevelId() {Invalidate();}
    explicit LevelId(UInt32 u) : BeUInt32Id (u) {}
    void CheckValue() const {BeAssert(IsValid());}
};

BESERVER_ISSUED_ID_CLASS(DgnSubLevelId)       //!< An ID that is assigned to a SubLevel of a level.

struct SubLevelId
{
private:
    LevelId         m_level;
    DgnSubLevelId   m_subLevel;

public:
    SubLevelId() {} // create an invalid LevelSubLevel
    explicit SubLevelId(LevelId level, DgnSubLevelId subLevel=DgnSubLevelId(0)) : m_level(level), m_subLevel(subLevel) {}
    explicit SubLevelId(JsonValueCR val) {FromJson(val);}
    LevelId GetLevel() const {return m_level;}
    DgnSubLevelId GetSubLevel() const {return m_subLevel;}
    bool IsValid() const {return m_level.IsValid() && m_subLevel.IsValid();}
    void ToJson(JsonValueR) const;
    void FromJson(JsonValueCR);
    bool operator==(SubLevelId const& rhs) const {return 0==memcmp(this, &rhs, sizeof(*this));}
    bool operator!=(SubLevelId const& rhs) const {return !(*this==rhs);}
    bool operator<(SubLevelId const& rhs) const  {return 0>memcmp(this, &rhs, sizeof(*this));}
    };

//! An ID that is assigned to a style. See DgnProject#Styles.
struct DgnStyleId
{
private:
    bool m_isValid;
    UInt32 m_id;

public:
    DgnStyleId() {Invalidate();}
    explicit DgnStyleId(UInt32 u) : m_id (u), m_isValid (true){}
    bool operator==(DgnStyleId const& rhs) const {return rhs.m_id==m_id;}
    bool operator!=(DgnStyleId const& rhs) const {return !(*this==rhs);}
    bool operator<(DgnStyleId const& rhs) const {return m_id<rhs.m_id;}
    void Invalidate() {m_id = -1; m_isValid = false; }
    bool IsValid() const {return m_isValid;}
    UInt32 GetValue() const {BeAssert(IsValid()); return m_id;}
};

BEREPOSITORYBASED_ID_CLASS(ElementId)          //!< An Id that is assigned to a DgnElement
BEREPOSITORYBASED_ID_CLASS(DgnKeyStringId)     //!< An Id that is assigned to a key string. See DgnProject#KeyStrings.
BEREPOSITORYBASED_ID_CLASS(DgnLinkId)          //!< An Id that is assigned to a DGN link (@see DgnLinkTable).
BEREPOSITORYBASED_ID_CLASS(DgnMaterialId)      //!< An Id that is assigned to a material. See DgnProject#Materials.
BEREPOSITORYBASED_ID_CLASS(DgnModelSelectorId) //!< An Id that is assigned to a model selector. See DgnProject#ModelSelectors.
BEREPOSITORYBASED_ID_CLASS(DgnRasterFileId)    //!< An Id that is assigned to a raster file.
BEREPOSITORYBASED_ID_CLASS(DgnSessionId)       //!< An Id that is assigned to a session. See DgnProject#Sessions.
BEREPOSITORYBASED_ID_CLASS(DgnStampId)         //!< An Id that is assigned to a DgnStamp
BEREPOSITORYBASED_ID_CLASS(DgnViewId)          //!< An Id that is assigned to a view. See DgnProject#Views, ViewController.
BEREPOSITORYBASED_ID_SUBCLASS(DgnItemId,BeSQLite::EC::ECInstanceId) //!< An Id that is assigned to a DgnItem

BESERVER_ISSUED_ID_CLASS(DgnTrueColorId)       //!< An ID that is assigned to a true color. See DgnProject#Colors.

typedef bset<ElementId> ElementIdSet;

/** @cond BENTLEY_SDK_Internal */

#define INVALID_ELEMENTID  ((UInt64) -1)

//! Types used to interface with native DgnDisplayKernel
struct DgnDisplayCoreTypes
    {
    //! Platform-specific view window
    struct Window {};
    typedef Window* WindowP;
    //! Platform-specific system context required by QV
    struct QvSystemContext {};
    typedef QvSystemContext* QvSystemContextP;
    //! Platform-specific device context
    struct DeviceContext {};
    typedef DeviceContext* DeviceContextP;
    //! Platform-specific handle to bitmap
    struct Bitmap {};
    typedef Bitmap* BitmapP;
    };

enum class EvaluationReason
    {
    None                = 0,
    Editor              = (1 << 0),
    Update              = (1 << 1),
    Plot                = (1 << 2),
    ModelLoad           = (1 << 3),
    ModelSave           = (1 << 4),
    DesignHistory       = (1 << 5),
    Unconditional       = -1
    };

/** @endcond */

//=======================================================================================
//! @ingroup ConfigManagement
//! @private
//=======================================================================================
enum class ConfigurationVariableLevel
    {
    Predefined    = -2,        //!< predefined by the host
    SysEnv        = -1,        //!< defined in the Windows system environment variable table
    System        = 0,         //!< system defined
    Appl          = 1,         //!< application defined
    Site          = 2,         //!< site defined
    Project       = 3,         //!< project defined
    User          = 4,         //!< user defined
    };

/** @cond BENTLEY_SDK_Internal */
/*=================================================================================**//**
  DgnPlatformConstants contains some negative values so it must not contain any
  32-bit unsigned values. Mixing 32-bit unsigned values and negative values causes
  clang to make DgnPlatformConstants a 64-bit signed type.  32-bit unsigned values
  such as INVALID_COLOR that are candidates to be in DgnPlatformConstants must
  instead be in an enum that does not contain any negative values.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum DgnPlatformConstants
    {
    DGNPLATFORM_RESOURCE_MAX_UNIT_LABEL_LENGTH            = 32,
    DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH        = 512,
    DGNPLATFORM_RESOURCE_MAX_MODEL_NAME_LENGTH            = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    DGNPLATFORM_RESOURCE_MAX_DIMSTYLE_NAME_LENGTH         = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    DGNPLATFORM_RESOURCE_MAX_DIMSTYLE_DESCRIPTION_LENGTH  = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    DGNPLATFORM_RESOURCE_MAX_CELLNAME_LENGTH              = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    DGNPLATFORM_RESOURCE_MAX_CELLDSCR_LENGTH              = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,

    MAX_V8_ELEMENT_WORDS            = 65535,
    MAX_V8_ELEMENT_SIZE             = (MAX_V8_ELEMENT_WORDS*2),
    MAX_VIEWS                       = 8,
    MAX_EDFIELDS                    = 20,                          /* max enter data fields / line */
    MAX_LINKAGE_STRING_LENGTH       = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    MAX_LINKAGE_STRING_BYTES        = (2*MAX_LINKAGE_STRING_LENGTH),   /* max number of bytes string in a linkage  */
    MAX_UNIT_LABEL_LENGTH           = DGNPLATFORM_RESOURCE_MAX_UNIT_LABEL_LENGTH,
    MAX_ACS_NAME_LENGTH             = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    MAX_ACS_DESCR_LENGTH            = DGNPLATFORM_RESOURCE_MAX_LINKAGE_STRING_LENGTH,
    MAX_CELLNAME_LENGTH             = DGNPLATFORM_RESOURCE_MAX_CELLNAME_LENGTH,
    MAX_CELLDSCR_LENGTH             = DGNPLATFORM_RESOURCE_MAX_CELLDSCR_LENGTH,

    MAX_TEXTSTYLE_NAME_LENGTH       = MAX_LINKAGE_STRING_LENGTH,
    MAX_DIMSTYLE_NAME_LENGTH        = DGNPLATFORM_RESOURCE_MAX_DIMSTYLE_NAME_LENGTH,
    MAX_DIMSTYLE_DESCRIPTION_LENGTH = DGNPLATFORM_RESOURCE_MAX_DIMSTYLE_DESCRIPTION_LENGTH,
    MAX_VERTICES                    = 5000,
    MIN_LINECODE                    = 0,
    MAX_LINECODE                    = 7,
    MINIMUM_WINDOW_DEPTH            = -32767,
    MAXIMUM_WINDOW_DEPTH            = 32767,
    };

//  Do not add any negative values to this enum.  The comment for DgnPlatformConstants explains why.
enum DgnPlatformInvalidSymbology
    {
    INVALID_COLOR                   = 0xffffff00,
    INVALID_STYLE                   = 0x7fffff00,
    INVALID_WEIGHT                  = 0xffffff00,
    INVALID_CLASS                   = 0xfefd,        /* This is larger than the 4 bits allotted in an element; don't try to put it there */
    };

enum LinkageKeyValues
    {
    BITMASK_LINKAGE_KEY_TextWhiteSpace                  = 6  ,
    BITMASK_LINKAGE_KEY_DimShieldsBase                  = 100,
    BITMASK_LINKAGE_KEY_MlineOverrideFlags              = 8  ,

    STAMPID_LINKAGE_KEY_SymbolIdMap                     = 4  ,

    STRING_LINKAGE_KEY_Generic                          = 0  ,
    STRING_LINKAGE_KEY_Name                             = 1  ,
    STRING_LINKAGE_KEY_Description                      = 2  ,
    STRING_LINKAGE_KEY_FileName                         = 3  ,
    STRING_LINKAGE_KEY_LogicalName                      = 4  ,
    STRING_LINKAGE_KEY_PatternCell                      = 5  ,
    STRING_LINKAGE_KEY_DimensionStyle                   = 6  ,
    STRING_LINKAGE_KEY_DimStyleDescr                    = 7  ,
    STRING_LINKAGE_KEY_Library                          = 8  ,
    STRING_LINKAGE_KEY_ProfileName                      = 9  ,
    STRING_LINKAGE_KEY_LevelNameExpr                    = 10 ,
    STRING_LINKAGE_KEY_LevelDescriptionExpr             = 11 ,
    STRING_LINKAGE_KEY_MastUnitLabel                    = 19 ,
    STRING_LINKAGE_KEY_SubUnitLabel                     = 20 ,
    STRING_LINKAGE_KEY_ModelName                        = 21 ,
    STRING_LINKAGE_KEY_SecondaryMastUnitLabel           = 22 ,
    STRING_LINKAGE_KEY_SecondarySubUnitLabel            = 23 ,
    STRING_LINKAGE_KEY_DimArrowCellName                 = 24 ,
    STRING_LINKAGE_KEY_DimStrokeCellName                = 25 ,
    STRING_LINKAGE_KEY_DimDotCellName                   = 26 ,
    STRING_LINKAGE_KEY_DimOriginCellName                = 27 ,
    STRING_LINKAGE_KEY_DimPrefixCellName                = 28 ,
    STRING_LINKAGE_KEY_DimSuffixCellName                = 29 ,
    STRING_LINKAGE_KEY_NameSpace                        = 30 ,
    STRING_LINKAGE_KEY_FullReferencePath                = 31 ,
    STRING_LINKAGE_KEY_XData                            = 33 ,
    STRING_LINKAGE_KEY_ReportName                       = 34 ,
    STRING_LINKAGE_KEY_RefAlternateFile                 = 35 ,
    STRING_LINKAGE_KEY_RefAlternateModel                = 36 ,
    STRING_LINKAGE_KEY_RefAlternateFullPath             = 37 ,
    STRING_LINKAGE_KEY_DWGPatternName                   = 38 ,
    STRING_LINKAGE_KEY_DwgBlockName                     = 42 ,
    STRING_LINKAGE_KEY_ReferenceNamedGroup              = 46 ,
    STRING_LINKAGE_KEY_DefaultRefLogical                = 47 ,
    STRING_LINKAGE_KEY_DimNoteCellName                  = 49 ,
    STRING_LINKAGE_KEY_AnimationParameter               = 58 ,
    STRING_LINKAGE_KEY_AnimationActionType              = 59 ,
    STRING_LINKAGE_KEY_AnimationOriginalActorName       = 60 ,
    STRING_LINKAGE_KEY_SchemaName                       = 61 ,
    STRING_LINKAGE_KEY_EndField                         = 62 ,
    STRING_LINKAGE_KEY_PstFileName                      = 63 ,
    STRING_LINKAGE_KEY_ReferenceProviderID              = 64 ,
    STRING_LINKAGE_KEY_IlluminatedMesh                  = 66 ,
    STRING_LINKAGE_KEY_SheetName                        = 73 ,
    STRING_LINKAGE_KEY_Sheet_UNUSED_74                  = 74 ,
    STRING_LINKAGE_KEY_DwgEntityPropertyList            = 85 ,

    DOUBLEARRAY_LINKAGE_KEY_Generic                     = 0  ,
    DOUBLEARRAY_LINKAGE_KEY_Fence                       = 1  ,
    DOUBLEARRAY_LINKAGE_KEY_ClippingRct                 = 2  ,
    DOUBLEARRAY_LINKAGE_KEY_ClippingMsk                 = 3  ,
    DOUBLEARRAY_LINKAGE_KEY_ClippingRotation            = 4  ,
    DOUBLEARRAY_LINKAGE_KEY_WorldToViewXForm            = 5  ,
    DOUBLEARRAY_LINKAGE_KEY_ViewToWorldXForm            = 6  ,
    DOUBLEARRAY_LINKAGE_KEY_FlattenTransform            = 7  ,
    DOUBLEARRAY_LINKAGE_KEY_DwgTransform                = 8  ,
    DOUBLEARRAY_LINKAGE_KEY_RefColorAdjustment          = 9  ,
    DOUBLEARRAY_LINKAGE_KEY_HLinePathInfo               = 10 ,
    DOUBLEARRAY_LINKAGE_KEY_RefColorAdvancedAdjustment  = 11 ,
    DOUBLEARRAY_LINKAGE_KEY_RegionTextMarginFactor      = 12 ,
    DOUBLEARRAY_LINKAGE_KEY_AuxCoordScale               = 13 ,
    DOUBLEARRAY_LINKAGE_KEY_RefTransparency             = 14 ,
    DOUBLEARRAY_LINKAGE_KEY_Transform                   = 15 ,
    DOUBLEARRAY_LINKAGE_KEY_PlacemarkMonument           = 16 ,
    DOUBLEARRAY_LINKAGE_KEY_AnnotationScale             = 17 ,
    DOUBLEARRAY_LINKAGE_KEY_ClippingDepth               = 18 ,
    DOUBLEARRAY_LINKAGE_KEY_RefAdditionalFlags          = 19,
    DOUBLEARRAY_LINKAGE_KEY_OriginRelativeOffset        = 20,
    BYTEARRAY_LINKAGE_KEY_FontNameCP                    = 0  ,
    BYTEARRAY_LINKAGE_KEY_AlternateFontNameCP           = 1  ,
    BYTEARRAY_LINKAGE_KEY_HLineTiling                   = 2  ,
    BYTEARRAY_LINKAGE_KEY_SectionEdgeId                 = 3  ,
    BYTEARRAY_LINKAGE_KEY_VectorIconModel               = 4  ,
    BYTEARRAY_LINKAGE_KEY_AuxCoordGrid                  = 5  ,
    MULTISTATEMASK_LINKAGE_KEY_NamedLevelMask           = 1  ,
    SYMBOLOGY_LINKAGE_KEY_RefBound                      = 0  ,
    SYMBOLOGY_LINKAGE_KEY_HLineVisible                  = 1  ,
    SYMBOLOGY_LINKAGE_KEY_HLineHidden                   = 2  ,
    SYMBOLOGY_LINKAGE_KEY_HLineSmooth                   = 3  ,
    };
/** @endcond */

//! Enumeration of possible coordinate system types
enum class DgnCoordSystem
    {
    Screen    = 0,     //!< Coordinates are relative to the origin of the screen
    View      = 1,     //!< Coordinates are relative to the origin of the view
    Npc       = 2,     //!< Coordinates are relative to normalized plane coordinates.
    World     = 3,     //!< Coordinates are relative to the <i>world</i> coordinate system for the physical elements in the DgnProject
    Root      = 3,     //!< @private Deprecated: coordinates are relative to the root model (replaced by DgnCoordSystem::World)
    };

/** @cond BENTLEY_SDK_Internal */
enum class GradientFlags
    {
    None         = 0,
    Invert       = (1 << 0),
    Outline      = (1 << 1),
    AlwaysFilled = (1 << 2),
    };

ENUM_IS_FLAGS(GradientFlags)

enum class ClipMask
    {
    None       = 0,
    XLow       = (0x0001 << 0),
    XHigh      = (0x0001 << 1),
    YLow       = (0x0001 << 2),
    YHigh      = (0x0001 << 3),
    ZLow       = (0x0001 << 4),
    ZHigh      = (0x0001 << 5),
    XAndY      = (0x000f),         // (XLow | XHigh | YLow | YHigh) - set back to that when we compile everything with VS2012.
    All        = (0x003f),         // (XAndY | ZLow | ZHigh),       - "
    };

ENUM_IS_FLAGS(ClipMask)

enum
    {
    MAX_GRADIENT_KEYS            =  8,
    };

enum ByLevelVals
    {
    COLOR_BYLEVEL       = 0xffffffff,
    COLOR_BYCELL        = 0xfffffffe,
    STYLE_BYLEVEL       = 0x7fffffff,
    STYLE_BYCELL        = 0x7ffffffe,
    WEIGHT_BYLEVEL      = 0xffffffff,
    WEIGHT_BYCELL       = 0xfffffffe,
    LEVEL_BYCELL        = 64,
    };

enum ElementLinkageIds
    {
    LINKAGEID_DDE_LINK                       = 20285,        //  0x4f3d
    LINKAGEID_Node                           = 20357,        /* 0x4f85 */
    LINKAGEID_CellDef                        = 20372,        /* 0x4f94 */
    LINKAGEID_ACS                            = 20389,        /* 0x4fa5 */
    LINKAGEID_AssociatedElements             = 20394,        /* 0x4faa */
    LINKAGEID_UvVertex                       = 20799,        // 0x513f
    LINKAGEID_RenderVertex                   = 20899,        // 0x51a3
    LINKAGEID_AnimatorCompressionCell        = 20904,        /* 0x51a8 */
    LINKAGEID_Feature                        = 21033,        /* 0x5229 */
    LINKAGEID_EmbeddedBRep                   = 21038,        /* 0x522e */
    LINKAGEID_Profile                        = 21041,        /* 0x5231 */
    LINKAGEID_Compression                    = 21047,        /* 0x5237 */
    TEXTNODE_Linkage                         = 22220,        /* 0x56CC */
    TEXT_Linkage                             = 22221,        /* 0x56CD */
    LINKAGEID_Dependency                     = 22224,        // 0x56d0 */
    LINKAGEID_String                         = 22226,        /* 0x56d2 XATTRIBUTEID_String */
    LINKAGEID_BitMask                        = 22227,        /* 0x56d3 */
    LINKAGEID_Thickness                      = 22228,        /* 0x56d4 */
    LINKAGEID_DoubleArray                    = 22229,        /* 0x56d5 */
    LINKAGEID_ToolTemplate                   = 22230,        /* 0x56d6 */
    LINKAGEID_AssocRegion                    = 22232,        /* 0x56d8 */
    LINKAGEID_SeedPoints                     = 22234,        /* 0x56da */
    LINKAGEID_MultipleLevels                 = 22235,        /* 0x56db */
    LINKAGEID_ClipBoundary                   = 22236,        /* 0x56dc */
    LINKAGEID_FilterMember                   = 22237,        /* 0x56dd */
    LINKAGEID_DimExtensionLinkage            = 22238,        /* 0x56de */
    LINKAGEID_Symbology                      = 22241,        /* 0x56e1 */
    LINKAGEID_XML                            = 22243,        /* 0x56e3 XATTRIBUTEID_XML */
    LINKAGEID_XData                          = 22244,        /* 0x56e4 */
    LINKAGEID_BoundaryAssociations           = 22245,        /* 0x56e5 */
    LINKAGEID_LoopOEDCode                    = 22247,        /* 0x56e7 */
    LINKAGEID_InfiniteLine                   = 22249,        /* 0x56e9 */
//__PUBLISH_SECTION_END__

    /* BEGIN 22250 to 22350 - reserved for Bentley Systems Usage  */
    LINKAGEID_SharedCellFlags                = 22250,        /* 0x56ea */
    LINKAGEID_RasterMetadata                 = 22251,        /* 0x56eb */
    LINKAGEID_SheetProperties                = 22253,        /* 0x56ed */
    LINKAGEID_SheetScales                    = 22254,        /* 0x56ee */
    LINKAGEID_StandardsChecker               = 22255,        /* 0x56ef */
    LINKAGEID_StandardsCheckerSettings       = 22256,        /* 0x56f0 */
    LINKAGEID_ElementIDArray                 = 22257,        /* 0x56f1 */
    LINKAGEID_StdsCheckIgnoredError          = 22258,        /* 0x56f2 XATTRIBUTEID_StdsCheckIgnoredError */
    LINKAGEID_TextAnnotation                 = 22259,        /* 0x56f3 */
    LINKAGEID_AnimationModel                 = 22262,        /* 0x56f6 */
    LINKAGEID_AnimationScriptParameter       = 22263,        /* 0x56f7 */
    LINKAGEID_AnimationData                  = 22264,        /* 0x56f8 */
    LINKAGEID_AnimationPlugins               = 22265,        /* 0x56f9 */
    LINKAGEID_AnimationEntryDescriptions     = 22266,        /* 0x56fa */
    LINKAGEID_CustomKeypoint                 = 22267,        /* 0x56fb */
    LINKAGEID_AnimationTimeElement           = 22268,        /* 0x56fc */
    LINKAGEID_TestLinkage                    = 22269,        /* 0x56fd */
    LINKAGEID_AnimationKeyFrameElement       = 22270,        /* 0x56fe */
    LINKAGEID_ECXAttributes                  = 22271,        /* 0x56ff XATTRIBUTEID_ECXAttributes */
    LINKAGEID_SheetPropertiesEx              = 22273,        /* 0x5701 */
    LINKAGEID_AnimationElemOrch              = 22284,        /* 0x570c */
    LINKAGEID_PersistentTopology             = 22285,        /* 0x570d */
    LINKAGEID_ConflictRevisions              = 22287,        /* 0x570f XATTRIBUTEID_ConflictRevisions */
    LINKAGEID_MultiStateMask                 = 22288,        /* 0x5710 */
    LINKAGEID_MstnApplicationSetting         = 22289,        /* 0x5711 */
    LINKAGEID_MaxwellMaterialMapping         = 22291,        /* 0x5713 */
    LINKAGEID_DGNECPlugin                    = 22294,        /* 0x5716 XATTRIBUTEID_DGNECPlugin */
    LINKAGEID_ModelID                        = 22295,        /* 0x5717 */
    LINKAGEID_ModelHandler                   = 22296,        /* 0x5718 */
    LINKAGEID_ComponentSet                   = 22297,        /* 0x5719 */
    LINKAGEID_PrintStyle                     = 22298,        /* 0x571a XATTRIBUTEID_PrintStyle  */
    LINKAGEID_ECOMConnection                 = 22299,        /* 0x571b */
    LINKAGEID_LuxologyPresetMapping          = 22300,        /* 0x571c */

    LINKAGEID_ByteArray                      = 22353,        /* 0x5751 */
    LINKAGEID_DwgToleranceData               = 22525,        /* 0x57fd */
    LINKAGEID_DgnLinks                       = 22527,        /* 0x57ff */
    LINKAGEID_TEXTSTYLE                      = 22529,        /* 0x5801 */
    TEXT_IndentationLinkage                  = 22544,        /* 0x5810 */
    LINKAGEID_TextRendering                  = 22551,        /* 0x5817 */
    LINKAGEID_TemplateData                   = 22587,        /* 0x583b XATTRIBUTEID_TemplateData */
    LINKAGEID_REFERENCE_PROVIDERID           = 22624,        // reserved identifier, needed for Raster References
    TEXTATTR_ID                              = 32980,        /* 0x80d4 */     /* Attribute ID (rad50 'TXT')  */
    LINKAGEID_OLE                            = (45086),       /* 0xB01e */
    LINKAGEID_ReferenceRenderingPlot         = 22765         /* 0x58ed */

    // ***  STOP!!! DO NOT ADD NEW ENTRIES HERE!!!!!
    // This is NOT a comprehensive list of LINKAGEIDs and should not be used to obtain new ids!!!
    // Ideally, all of these values should be moved to private header files.
    // You must get new  IDs from http://toolsnet.bentley.com/Signature/Default.aspx
//__PUBLISH_SECTION_START__
    };

//__PUBLISH_SECTION_END__
enum DgnStoreIds
    {
    DGN_STORE_ID                      = 'dStr',  /* MicroStation dgnStore Id */
    EMBEDDED_BREP_ID                  = 'BRep',  /* MicroStation dgnStore appId */
    XMLFRAGMENT_ID                    = 'XMLf',  /* MicroStation dgnStore appId */
    TAGSET_ID                         = 'tSet',  /* MicroStation dgnStore Id */
    DGNSTOREID_OLE                    = 'Ole ',  /* MicroStation dgnStore Id */
    DGNSTOREAPPID_OLE_STORAGE         = 'OleS',  /* MicroStation dgnStore appId */
    };

enum DisplayPriorityConstants
    {
    DISPLAYPRIORITY_BYCELL       = 0x80000000,
    };
//__PUBLISH_SECTION_START__

enum class SnapStatus
    {
    Success              = SUCCESS,
    Aborted              = 1,
    NoElements           = 2,
    Disabled             = 100,
    NoSnapPossible       = 200,
    NotSnappable         = 300,
    RefNotSnappable      = 301,
    FilteredByLevel      = 400,
    FilteredByUser       = 500,
    FilteredByApp        = 600,
    FilteredByAppQuietly = 700,
    };

enum class OutputMessagePriority
    {
    None           = 0,
    Error          = 10,
    Warning        = 11,
    Info           = 12,
    Debug          = 13,
    OldStyle       = 14,
    TempRight      = 15,
    TempLeft       = 16,
    Fatal          = 17,
    };

/* Values for mdlOutput_messageCenter openAlertBox argument */
enum class OutputMessageAlert
    {
    None     = 0,
    Dialog   = 1,
    Balloon  = 2,
    };

enum TransformOptionValues
    {
    TRANSFORM_OPTIONS_ModelFromElmdscr          = (1 << 0),
    TRANSFORM_OPTIONS_DimValueMatchSource       = (1 << 1),     // Turn off if dimension value should be scaled
    TRANSFORM_OPTIONS_DimSizeMatchSource        = (1 << 2),     // Turn off if non-annotation dimension size should be scaled
    TRANSFORM_OPTIONS_MlineScaleOffsets         = (1 << 3),
    TRANSFORM_OPTIONS_MlineMirrorOffsets        = (1 << 4),
    TRANSFORM_OPTIONS_DisableMirrorCharacters   = (1 << 5),
//__PUBLISH_SECTION_END__
    TRANSFORM_OPTIONS_DisallowSizeChange        = (1 << 6),     // Support for old api.
//__PUBLISH_SECTION_START__
    TRANSFORM_OPTIONS_AnnotationSizeMatchSource = (1 << 7),     // Turn off if annotations should be scaled
    TRANSFORM_OPTIONS_RotateDimView             = (1 << 8),     // Turn on if dim view orientation should be changed (so that dim text orientation remains constant)
    TRANSFORM_OPTIONS_ApplyAnnotationScale      = (1 << 9),     // Turn on if annotation scale (provided by caller) should be applied to annotations
    TRANSFORM_OPTIONS_FromClone                 = (1 << 10),    // transforming for purposes of cloning between models.
    TRANSFORM_OPTIONS_NoteScaleSize             = (1 << 11),    // Apply scale to note's sizes. Used as an override when TRANSFORM_OPTIONS_DimSizeMatchSource == True.
    TRANSFORM_OPTIONS_DisableRotateCharacters   = (1 << 12)     // If a rotation is specified, only the text's origin will be transformed (i.e. the characters will retain their original visual orientation).
    };

enum class GridOrientationType
    {
    View    = 0,
    WorldXY = 1,           // Top
    WorldYZ = 2,           // Right
    WorldXZ = 3,           // Front
    ACS     = 4,
    Maximum = 4,
    };

enum AngleModeVals
    {
    ANGLE_PRECISION_Active   = -1,
    ANGLE_MODE_Active        = -1,
    ANGLE_MODE_Standard      = 0,
    ANGLE_MODE_Azimuth       = 1,
    ANGLE_MODE_Bearing       = 2,
    };

enum class DisplayPathType
    {
    Display      = 0,
    Selection    = 1,
    Hit          = 2,
    Snap         = 3,
    Intersection = 4,
    };

enum DitherModes
    {
    DITHERMODE_Pattern              = 0,
    DITHERMODE_ErrorDiffusion       = 1,
    };

/*=================================================================================**//**
*  The symbology for a multi-line profile or end cap.
* @group          "Multi-Line Functions"
* @See      MlineProfile
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MlineSymbology
    {
    /** Reserved for future use; should be set to 0. */
    UInt32          reserved1:16;
    /** Use the vaule in style member of this struct; otherwise use the value in the element header.*/
    UInt32          useStyle:1;
    /** Use the vaule in weight member of this struct; otherwise use the value in the element header.*/
    UInt32          useWeight:1;
    /** Use the vaule in color member of this struct; otherwise use the value in the element header.*/
    UInt32          useColor:1;
    /** Draw inner arcs on cap; ignored for profiles. */
    UInt32          capInArc:1;
    /** Draw outer arcs on cap; ignored for profiles. */
    UInt32          capOutArc:1;
    /** Draw line on cap; ignored for profiles. */
    UInt32          capLine:1;
    /** Use the vaule in class member of this struct; otherwise use the value in the element header.*/
    UInt32          useClass:1;
    /** The style is a custom line style, not just a line code.  This is a legacy bit; set for backward compatibility.  */
    UInt32          customStyle:1;
    /** Use segment colors to draw end caps; a quarter arc of each color.  Ignored for profiles. */
    UInt32          capColorFromSeg:1;
    /** Reserved for future use; should be set to 0. */
    UInt32          reserved:6;
    /** The class of the profile or cap. */
    UInt32          conClass:1;
    /** The line style of the profile or cap. */
    Int32           style;
    /** The weight of the profile or cap. */
    UInt32          weight;
    /** The color of the profile or cap. */
    UInt32          color;
    /** The level of the profile or cap. If it is 0, then the level from the header is used. */
    UInt32          level;
    };

/*=================================================================================**//**
*  Definition of a multi-line profile.
* @group          "Multi-Line Functions"
* @See      MlineSymbology
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MlineProfile
    {
    /** The distance from the work line for the profile. */
    double          dist;
    /** Reserved for future use; should be set to 0. */
    Int32           reserved;
    /** The symbology for the profile or end cap. */
    MlineSymbology  symb;
    };

/*=================================================================================**//**
* Influences how handler should apply annotation scale.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class AnnotationScaleAction
    {
    Update  = 0,
    Add     = 1,
    Remove  = 2,
    };

/*=================================================================================**//**
* Influences how handler should apply fence stretch.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class FenceStretchFlags
    {
    /*! no special options */
    None      = (0),
    /*! stretch user defined cell components */
    Cells     = (1<<0),
    };

ENUM_IS_FLAGS (FenceStretchFlags)

/*=================================================================================**//**
* Influences how handler should apply fence clip.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class FenceClipFlags
    {
    /*! no special options */
    None         = (0),
    /*! maintain closed elements/solids/surfaces */
    Optimized    = (1<<0),
    };

ENUM_IS_FLAGS (FenceClipFlags)

enum class  ClipVolumePass
    {
    None,
    InsideForward,
    InsideBackward,
    Outside,
    Inside,
    Cut,
    Maximum
    };

/*=================================================================================**//**
* Enums for tool agenda+handler cooperation
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class  AgendaEvent
    {
    ModifyEntries           = 1,
    PreModify               = 2,
    PostModify              = 3,
    AddClipboardFormats     = 4,
    };

enum class AgendaModify
    {
    Original                = 0,
    Copy                    = 1,
    ClipOriginal            = 2,
    ClipCopy                = 3,
    };

enum class AgendaOperation
    {
    NotSpecified   = 0,
    Translate      = 1,
    Scale          = 2,
    Rotate         = 3,
    Mirror         = 4,
    Array          = 5,
    Stretch        = 6,
    Delete         = 7,
    Clipboard      = 8,
    DragDrop       = 9,
    ChangeAttrib   = 10,
    FileFence      = 11,
    Drop           = 12,
    };

struct RgbColorShort
    {
    UInt16  red;
    UInt16  green;
    UInt16  blue;
    };

struct HsvColorDef
    {
    Int32    hue;           /* red=0, yellow, green, cyan, blue, magenta */
                            /* 0 -> 360 */
    Int32    saturation;    /* 0=white, 100=no white, tints */
    Int32    value;         /* 0=black, 100=no black, shades */
    };

struct FColor3
    {
    float   r;
    float   g;
    float   b;
    };

struct FColor4
    {
    float   r;
    float   g;
    float   b;
    float   a;
    };

struct FTexture2
    {
    float   u;
    float   v;
    };

struct FTexture3
    {
    float   u;
    float   v;
    float   w;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class DrawPurpose
    {
    NotSpecified               = 0,
    Update                     = 1,
    UpdateDynamic              = 2,
    UpdateHealing              = 3,
    UpdateProgressive          = 4,
    Hilite                     = 5,
    Unhilite                   = 6,
    ChangedPre                 = 9,
    ChangedPost                = 10,
    RestoredPre                = 11,
    RestoredPost               = 12,
    Dynamics                   = 15,
    RangeCalculation           = 20,
    Plot                       = 21,
    Pick                       = 22,
    Flash                      = 23,
    TransientChanged           = 25,
    CaptureGeometry            = 26,
    GenerateThumbnail          = 27,
    ForceRedraw                = 29,
    FenceAccept                = 30,
    RegionFlood                = 31, //! Collect graphics to find closed regions/flood...
    FitView                    = 32,
    XGraphicsCreate            = 34,
    CaptureShadowList          = 35,
    ExportVisibleEdges         = 36,
    InterferenceDetection      = 37,
    CutXGraphicsCreate         = 38,
    ModelFacet                 = 39,
    Measure                    = 40,
    VisibilityCalculation      = 41,
    ProxyHashExtraction        = 42,
    };

/*=================================================================================**//**
* Used to communicate the result of handling an event from a GPS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class GeoLocationEventStatus
    {
    EventHandled                    = 0,    //!< LocationEvent was handled and modified the view
    EventIgnored                    = 1,    //!< LocationEvent was ignored and did not modify the view because of an unknown failure to convert
    NoGeoCoordinateSystem           = 2,    //!< LocationEvent was ignored and did not modify the view because the DgnProject does not have a geographic coordinate system defined
    PointOutsideGeoCoordinateSystem = 3,    //!< LocationEvent was ignored and did not modify the view because the LocationEvent's point is outside the bounds where the geographic coordinate system is accurate
    };


/*=================================================================================**//**
* Used to specify desired accuracy.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class GeoLocationServicesAccuracy
    {
    BestForNavigation       = 1,
    Best                    = 2,
    Coarse                  = 3
    };

/*=================================================================================**//**
* Used to describe status of a location provider.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class GeoLocationProviderStatus
    {
    NotDetermined           = 0,
    Disabled                = 1,
    Enabled                 = 2,
    TemporarilyUnavailable  = 3,
    Available               = 4,
    OutOfService            = 5,
    LocationUnavailable     = 6,
    };

/*=================================================================================**//**
* Holds information about levels and element classes that are displayed.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LevelClassMask
    {
    BitMaskCP   levelBitMaskP;     /* level bit mask */
    UInt16      classMask;         /* active classes for this view */
    };

struct AssocPoint
    {
    UShort      buf[20];
    };

typedef bvector<double> T_DoubleVector;
typedef T_DoubleVector*        T_DoubleVectorP, &T_DoubleVectorR;
typedef T_DoubleVector const*  T_DoubleVectorCP;
typedef T_DoubleVector const&  T_DoubleVectorCR;

enum class MlineModifyPoint
    {
    None                = 0,
    ShiftBreaks         = 0x2,
    RemoveAssociations  = 0x4,
    };

ENUM_IS_FLAGS (MlineModifyPoint)

#define   RMINI4                  (-2147483648.0)
#define   RMAXI4                  2147483647.0
#define   RMAXUI4                 4294967295.0
#define   LMAXI4                  INT32_MAX
#define   LMINI4                  INT32_MIN
#define   IMAXI8                  INT64_MAX
#define   IMINI8                  INT64_MIN
#define   IMAXUI8                 UINT64_MAX

//__PUBLISH_SECTION_END__

struct _dVector2d
    {
    DPoint2d    org;
    DPoint2d    end;
    };

#ifndef NO_Vector3d
struct Vector3d
    {
    Point3d     org;
    Point3d     end;
    };
#endif

typedef struct _dVector3d
    {
    DPoint3d    org;
    DPoint3d    end;
    } _dVector3d;

typedef struct _drectangle
    {
    DPoint2d    origin;
    DPoint2d    corner;
    } _drectangle;

typedef struct _drectangle Drectangle;

typedef struct srectangle
    {
    SPoint2d    origin;
    SPoint2d    corner;
    } Srectangle;

typedef struct upoint2d
    {
    UInt32      x;
    UInt32      y;
    } Upoint2d;

typedef struct upoint3d
    {
    UInt32      x;
    UInt32      y;
    UInt32      z;
    } Upoint3d;

typedef struct uspoint2d
    {
    UInt16      x;
    UInt16      y;
    } Uspoint2d;

typedef struct spoint3d
    {
    Int16       x;
    Int16       y;
    Int16       z;
    } Spoint3d, SPoint3d;

typedef struct uspoint3d
    {
    UInt16      x;
    UInt16      y;
    UInt16      z;
    } Uspoint3d;

typedef struct svector2d
    {
    SPoint2d    org;
    SPoint2d    end;
    } Svector2d, SVector2d;

typedef struct svector3d
    {
    SPoint3d    org;
    SPoint3d    end;
    } Svector3d, SVector3d;

typedef struct urectangle
    {
    Uspoint2d   origin;
    Uspoint2d   corner;
    } Urectangle;

typedef struct _vector2d
    {
    Point2d     org;
    Point2d     end;
    } Vector2d;

//__PUBLISH_SECTION_START__

enum class ScanTestResult
{
    Pass = 0,
    Fail = 1,
};

enum    DisplayFilterHandlerId
{
    DisplayFilterHandlerId_Parameter             = 4,
    DisplayFilterHandlerId_ViewFlag              = 5,
    DisplayFilterHandlerId_ECExpression          = 11,
    DisplayFilterHandlerId_PresentationFormId    = 12,
    DisplayFilterHandlerId_PresentationFormFlag  = 13
};

enum FileOpenConstants              // WIP_DGNPLATFORM_TOOLS
    {
    OPEN_FOR_WRITE         = 2,
    OPEN_FOR_READ          = 0,
    UF_WTR_SUCCESS         = 42,
    UF_OPEN_READ           = 0,
    UF_OPEN_WRITE          = 1,
    UF_OPEN_CREATE         = 2,
    UF_TRY_WRITE_THEN_READ = 4,
    UF_CUR_DIR_SWAP        = 8,
    UF_NO_CUR_DIR          = 0x10,
    UF_JUST_BUILD          = 0x20,
    UF_FIND_FOLDER         = 0x100,
    };

//__PUBLISH_SECTION_END__

#define SemiPrivate

// Used for verifying published tests in DgnPlatformTest are using published headers. DO NOT REMOVE.
#define __DGNPLATFORM_NON_PUBLISHED_HEADER__ 1
/*__PUBLISH_SECTION_START__*/

#define TO_BOOL(x) (0 != (x))

/** @endcond */

END_BENTLEY_DGNPLATFORM_NAMESPACE
