/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @namespace BentleyApi::DgnPlatform Types defined by the %DgnPlatform. */

/** @cond BENTLEY_SDK_Internal */

#include <BentleyApi/BentleyApi.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeFileName.h>
#include "ExportMacros.h"
#include <Geom/GeomApi.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include "DgnPlatform.r.h"
#include "DgnPlatformErrors.r.h"
#include "DgnHost.h"
#include <BeSQLite/BeSQLite.h>
#include <ECDb/ECDbApi.h>

#define USING_NAMESPACE_BENTLEY_DGNPLATFORM using namespace BentleyApi::DgnPlatform;

#define USING_NAMESPACE_EC                  using namespace BentleyApi::ECN;
#define USING_NAMESPACE_BENTLEY_EC          using namespace BentleyApi::ECN;

#define BEGIN_RASTER_NAMESPACE              BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace Raster {
#define END_RASTER_NAMESPACE                } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_NAMESPACE_RASTER              using namespace BentleyApi::DgnPlatform::Raster;

#define BEGIN_BENTLEY_POINTCLOUD_NAMESPACE  BEGIN_BENTLEY_API_NAMESPACE namespace PointCloud {
#define END_BENTLEY_POINTCLOUD_NAMESPACE    } END_BENTLEY_API_NAMESPACE
#define USING_NAMESPACE_BENTLEY_POINTCLOUD  using namespace BentleyApi::PointCloud;

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
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_DGNPLATFORM_NAMESPACE

#define DGNPLATFORM_CLASS_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE class _name_; END_BENTLEY_DGNPLATFORM_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS1(DgnPlatform,_name_,_name_,class)

#define RASTER_TYPEDEFS(t) \
    BEGIN_RASTER_NAMESPACE struct t; END_RASTER_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS (DgnPlatform::Raster,t);

#define RASTER_REF_COUNTED_PTR(_sname_) \
    BEGIN_RASTER_NAMESPACE struct _sname_; END_RASTER_NAMESPACE \
    BEGIN_BENTLEY_API_NAMESPACE DEFINE_REF_COUNTED_PTR(DgnPlatform::Raster::_sname_) END_BENTLEY_API_NAMESPACE

#define RASTER_TYPEDEF1(_sourceName_,_name_,_structclass_) \
    BEGIN_RASTER_NAMESPACE _structclass_ _sourceName_; END_RASTER_NAMESPACE  \
    ADD_BENTLEY_API_TYPEDEFS1(DgnPlatform::Raster,_sourceName_,_name_,_structclass_)

#define GEOCOORD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_API_NAMESPACE namespace GeoCoordinates { struct _name_; } END_BENTLEY_API_NAMESPACE \
    ADD_BENTLEY_API_TYPEDEFS(GeoCoordinates, _name_)

/** @endcond */

/** @cond BENTLEY_SDK_Internal */
GLOBAL_TYPEDEF (QvElem,QvElem)
GLOBAL_TYPEDEF (QvCache,QvCache)
GLOBAL_TYPEDEF (QvView,QvView)
GLOBAL_TYPEDEF (QvMRImage,QvMRImage)

BENTLEY_NAMESPACE_TYPEDEFS (IRefCounted)

BENTLEY_API_TYPEDEFS (BitMask)
BENTLEY_API_TYPEDEFS (DataExternalizer)
BENTLEY_API_TYPEDEFS (GPArray)
BENTLEY_API_TYPEDEFS (GraphicsPointArray)
/** @endcond */

DGNPLATFORM_REF_COUNTED_PTR (DgnModel);
DGNPLATFORM_TYPEDEFS (ColorDef)
DGNPLATFORM_TYPEDEFS (BoundingBox2d)
DGNPLATFORM_TYPEDEFS (BoundingBox3d)
DGNPLATFORM_TYPEDEFS (DgnDb)
DGNPLATFORM_TYPEDEFS (DgnElement)
DGNPLATFORM_TYPEDEFS (DgnElement2d)
DGNPLATFORM_TYPEDEFS (DgnElement3d)
DGNPLATFORM_TYPEDEFS (DgnFont);
DGNPLATFORM_TYPEDEFS (DgnGeomPart)
DGNPLATFORM_TYPEDEFS (DgnGlyph);
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutContext);
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutResult);
DGNPLATFORM_TYPEDEFS (DgnMarkupProject)
DGNPLATFORM_TYPEDEFS (DgnModel)
DGNPLATFORM_TYPEDEFS (DgnResourceURI)
DGNPLATFORM_TYPEDEFS (DgnGlyph);
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutContext);
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutResult);
DGNPLATFORM_TYPEDEFS (DgnRscFont);
DGNPLATFORM_TYPEDEFS (DgnShxFont);
DGNPLATFORM_TYPEDEFS (DgnTrueTypeFont);
DGNPLATFORM_TYPEDEFS (DgnViewport)
DGNPLATFORM_TYPEDEFS (DisplayStyle)
DGNPLATFORM_TYPEDEFS (DisplayStyleFlags)
DGNPLATFORM_TYPEDEFS (DrawingElement)
DGNPLATFORM_TYPEDEFS (ElementGroup)
DGNPLATFORM_TYPEDEFS (GeomStream)
DGNPLATFORM_TYPEDEFS (GeometricElement)
DGNPLATFORM_TYPEDEFS (GradientSymb)
DGNPLATFORM_TYPEDEFS (IDgnFontData);
DGNPLATFORM_TYPEDEFS (IDrawGeom)
DGNPLATFORM_TYPEDEFS (IElemTopology)
DGNPLATFORM_TYPEDEFS (IRedrawOperation)
DGNPLATFORM_TYPEDEFS (IRedrawAbort)
DGNPLATFORM_TYPEDEFS (IViewDraw)
DGNPLATFORM_TYPEDEFS (IViewOutput)
DGNPLATFORM_TYPEDEFS (LineStyleInfo)
DGNPLATFORM_TYPEDEFS (LineStyleSymb)
DGNPLATFORM_TYPEDEFS (PhysicalElement)
DGNPLATFORM_TYPEDEFS (PhysicalRedlineModel)
DGNPLATFORM_TYPEDEFS (PlotInfo)
DGNPLATFORM_TYPEDEFS (RedlineModel)
DGNPLATFORM_TYPEDEFS (ViewContext)
DGNPLATFORM_TYPEDEFS (ViewController)
DGNPLATFORM_TYPEDEFS (ViewFlags)
DGNPLATFORM_TYPEDEFS (DgnDbExpressionContext);

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_REF_COUNTED_PTR (TextString);
DGNPLATFORM_REF_COUNTED_PTR (TextStringStyle);

DGNPLATFORM_TYPEDEFS (AxisAlignedBox2d)
DGNPLATFORM_TYPEDEFS (AxisAlignedBox3d)
DGNPLATFORM_TYPEDEFS (Caret)
DGNPLATFORM_TYPEDEFS (ChangeAnnotationScale)
DGNPLATFORM_TYPEDEFS (ClipPrimitive);
DGNPLATFORM_TYPEDEFS (ClipVector);
DGNPLATFORM_TYPEDEFS (ClipVolumeOverrides)
DGNPLATFORM_TYPEDEFS (CutGraphicsCachedKey)
DGNPLATFORM_TYPEDEFS (Dgn3DInputEvent)
DGNPLATFORM_TYPEDEFS (DgnButtonEvent)
DGNPLATFORM_TYPEDEFS (DgnColorMap)
DGNPLATFORM_TYPEDEFS (DgnDimStyle)
DGNPLATFORM_TYPEDEFS (DgnDomain)
DGNPLATFORM_TYPEDEFS (DgnGestureEvent)
DGNPLATFORM_TYPEDEFS (DgnHost)
DGNPLATFORM_TYPEDEFS (DgnModelInfo)
DGNPLATFORM_TYPEDEFS (DgnModelIterator)
DGNPLATFORM_TYPEDEFS (DgnMouseWheelEvent)
DGNPLATFORM_TYPEDEFS (DgnProgressMeter)
DGNPLATFORM_TYPEDEFS (DisplayPath)
DGNPLATFORM_TYPEDEFS (DrawContext)
DGNPLATFORM_TYPEDEFS (DrawingModel)
DGNPLATFORM_TYPEDEFS (DropGeometry)
DGNPLATFORM_TYPEDEFS (DropGraphics)
DGNPLATFORM_TYPEDEFS (DwgHatchDef)
DGNPLATFORM_TYPEDEFS (DwgHatchDefLine)
DGNPLATFORM_TYPEDEFS (ElemDisplayParams)
DGNPLATFORM_TYPEDEFS (ElemMatSymb)
DGNPLATFORM_TYPEDEFS (ElementAlignedBox2d)
DGNPLATFORM_TYPEDEFS (ElementAlignedBox3d)
DGNPLATFORM_TYPEDEFS (ElementGeometry)
DGNPLATFORM_TYPEDEFS (ElementGeometryBuilder)
DGNPLATFORM_TYPEDEFS (ElementHandler);
DGNPLATFORM_TYPEDEFS (ElementLocateManager)
DGNPLATFORM_TYPEDEFS (FenceManager)
DGNPLATFORM_TYPEDEFS (FenceParams)
DGNPLATFORM_TYPEDEFS (Frustum)
DGNPLATFORM_TYPEDEFS (GeomDetail)
DGNPLATFORM_TYPEDEFS (GradientSettings)
DGNPLATFORM_TYPEDEFS (HatchLinkage)
DGNPLATFORM_TYPEDEFS (HitList)
DGNPLATFORM_TYPEDEFS (HitPath)
DGNPLATFORM_TYPEDEFS (IACSManager)
DGNPLATFORM_TYPEDEFS (IAuxCoordSys)
DGNPLATFORM_TYPEDEFS (ICachedDraw)
DGNPLATFORM_TYPEDEFS (IDisplaySymbol)
DGNPLATFORM_TYPEDEFS (IEditAction)
DGNPLATFORM_TYPEDEFS (IEditActionArray)
DGNPLATFORM_TYPEDEFS (IEditActionSource)
DGNPLATFORM_TYPEDEFS (IEditManipulator)
DGNPLATFORM_TYPEDEFS (IElementGraphicsProcessor)
DGNPLATFORM_TYPEDEFS (IElementState)
DGNPLATFORM_TYPEDEFS (IFaceMaterialAttachments)
DGNPLATFORM_TYPEDEFS (ILineStyle)
DGNPLATFORM_TYPEDEFS (ILineStyleComponent)
DGNPLATFORM_TYPEDEFS (IMRImageTileEventHandler)
DGNPLATFORM_TYPEDEFS (IPickGeom)
DGNPLATFORM_TYPEDEFS (ISolidKernelEntity)
DGNPLATFORM_TYPEDEFS (ISprite)
DGNPLATFORM_TYPEDEFS (ISubEntity)
DGNPLATFORM_TYPEDEFS (ITiledRaster)
DGNPLATFORM_TYPEDEFS (ITransactionHandler)
DGNPLATFORM_TYPEDEFS (ITxnManager)
DGNPLATFORM_TYPEDEFS (IViewHandlerHitInfo)
DGNPLATFORM_TYPEDEFS (IViewTransients)
DGNPLATFORM_TYPEDEFS (IndexedViewSet)
DGNPLATFORM_TYPEDEFS (IndexedViewport)
DGNPLATFORM_TYPEDEFS (LineStyleInfo)
DGNPLATFORM_TYPEDEFS (LineStyleManager)
DGNPLATFORM_TYPEDEFS (LineStyleParams)
DGNPLATFORM_TYPEDEFS (LsComponent)
DGNPLATFORM_TYPEDEFS (LsCompoundComponent)
DGNPLATFORM_TYPEDEFS (LsDefinition)
DGNPLATFORM_TYPEDEFS (LsDgnProjectMap)
DGNPLATFORM_TYPEDEFS (LsInternalComponent)
DGNPLATFORM_TYPEDEFS (LsMap)
DGNPLATFORM_TYPEDEFS (LsMapEntry)
DGNPLATFORM_TYPEDEFS (LsMapIterator)
DGNPLATFORM_TYPEDEFS (LsPointComponent)
DGNPLATFORM_TYPEDEFS (LsStroke)
DGNPLATFORM_TYPEDEFS (LsStrokePatternComponent)
DGNPLATFORM_TYPEDEFS (LsSymbolComponent)
DGNPLATFORM_TYPEDEFS (LsSymbolReference)
DGNPLATFORM_TYPEDEFS (LsSystemMap)
DGNPLATFORM_TYPEDEFS (Material)
DGNPLATFORM_TYPEDEFS (MaterialAssignment)
DGNPLATFORM_TYPEDEFS (ModelHandler);
DGNPLATFORM_TYPEDEFS (NotificationManager)
DGNPLATFORM_TYPEDEFS (OvrMatSymb)
DGNPLATFORM_TYPEDEFS (ParagraphProperties)
DGNPLATFORM_TYPEDEFS (PatternParams)
DGNPLATFORM_TYPEDEFS (PermanentTopologicalId)
DGNPLATFORM_TYPEDEFS (PersistentElementPath)
DGNPLATFORM_TYPEDEFS (PersistentSnapPath)
DGNPLATFORM_TYPEDEFS (PhysicalModel)
DGNPLATFORM_TYPEDEFS (PhysicalRedlineViewController)
DGNPLATFORM_TYPEDEFS (PhysicalViewController)
DGNPLATFORM_TYPEDEFS (Placement2d)
DGNPLATFORM_TYPEDEFS (Placement3d)
DGNPLATFORM_TYPEDEFS (PropertyContext)
DGNPLATFORM_TYPEDEFS (QVAliasMaterialId)
DGNPLATFORM_TYPEDEFS (QueryModel)
DGNPLATFORM_TYPEDEFS (QueryViewController)
DGNPLATFORM_TYPEDEFS (QvUnsizedKey)
DGNPLATFORM_TYPEDEFS (QvViewport)
DGNPLATFORM_TYPEDEFS (RedlineViewController)
DGNPLATFORM_TYPEDEFS (RegionGraphicsContext)
DGNPLATFORM_TYPEDEFS (ScanCriteria)
DGNPLATFORM_TYPEDEFS (SelectionPath)
DGNPLATFORM_TYPEDEFS (SelectionSetManager)
DGNPLATFORM_TYPEDEFS (SheetViewController)
DGNPLATFORM_TYPEDEFS (SnapContext)
DGNPLATFORM_TYPEDEFS (SnapPath)
DGNPLATFORM_TYPEDEFS (StampQvElemMap)
DGNPLATFORM_TYPEDEFS (TextString)
DGNPLATFORM_TYPEDEFS (TextStringStyle);
DGNPLATFORM_TYPEDEFS (TransformClipStack)
DGNPLATFORM_TYPEDEFS (TransformInfo)
DGNPLATFORM_TYPEDEFS (TxnSummary)
DGNPLATFORM_TYPEDEFS (UpdateContext)
DGNPLATFORM_TYPEDEFS (ViewHandler);
DGNPLATFORM_TYPEDEFS (ViewManager)
DGNPLATFORM_TYPEDEFS (VisibleEdgeCache)

/** @endcond */

/** @cond BENTLEY_SDK_Internal */
GEOCOORD_TYPEDEFS (IGeoCoordinateServices)
GEOCOORD_TYPEDEFS (DgnGCS)
/** @endcond */

DGNPLATFORM_REF_COUNTED_PTR (DgnDb)
DGNPLATFORM_REF_COUNTED_PTR (DgnElement)
DGNPLATFORM_REF_COUNTED_PTR (DgnFont);
DGNPLATFORM_REF_COUNTED_PTR (DgnGeomPart)
DGNPLATFORM_REF_COUNTED_PTR (DgnMarkupProject)
DGNPLATFORM_REF_COUNTED_PTR (DrawingElement)
DGNPLATFORM_REF_COUNTED_PTR (GeometricElement)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalElement)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR (RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR (DgnDbExpressionContext)

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_REF_COUNTED_PTR (ClipPrimitive);
DGNPLATFORM_REF_COUNTED_PTR (ClipVector);
DGNPLATFORM_REF_COUNTED_PTR (DisplayPath)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)
DGNPLATFORM_REF_COUNTED_PTR (DisplayStyleHandlerSettings)
DGNPLATFORM_REF_COUNTED_PTR (IProgressiveDisplay)
DGNPLATFORM_REF_COUNTED_PTR (IRasterSourceFileQuery)
DGNPLATFORM_REF_COUNTED_PTR (ViewController)
/** @endcond */

//__PUBLISH_SECTION_END__
// ///////////////////////////////////////////////////////////////////////////////////////////////////
// DO NOT USE: these MAX*LENGTH values are not portable or correct!
// ///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_BENTLEY_API_NAMESPACE

enum
{
    DGNPLATFORM_RESOURCE_MAXFILELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXDIRLENGTH                     = 256,
    DGNPLATFORM_RESOURCE_MAXNAMELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH               = 256,

    MAXFILELENGTH         = DGNPLATFORM_RESOURCE_MAXFILELENGTH,
    MAXDIRLENGTH          = DGNPLATFORM_RESOURCE_MAXDIRLENGTH,
    MAXDEVICELENGTH       = 256,
    MAXNAMELENGTH         = DGNPLATFORM_RESOURCE_MAXNAMELENGTH,
    MAXEXTENSIONLENGTH    = DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH,
};

END_BENTLEY_API_NAMESPACE

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

ECINSTANCE_ID_CLASS(DgnCategoryId)      //!< An Id that is assigned to a DgnCategory.  A DgnElement belongs to exactly one DgnCategory.
ECINSTANCE_ID_CLASS(DgnElementId)       //!< An Id that is assigned to an Element.
ECINSTANCE_ID_CLASS(DgnGeomPartId)      //!< An Id that is assigned to a DgnGeomPart. A collection of DgnGeomParts make up the geometry aspect.
ECINSTANCE_ID_CLASS(DgnLinkId)          //!< An Id that is assigned to a DGN link. See DgnLinkTable.
ECINSTANCE_ID_CLASS(DgnModelId)         //!< An Id that is assigned to a DgnModel.  A DgnModel is a container for DgnElements.
ECINSTANCE_ID_CLASS(DgnStyleId)         //!< An Id that is assigned to a style. See DgnDb#Styles.
ECINSTANCE_ID_CLASS(DgnSubCategoryId)   //!< An Id that is assigned to a SubCategory of a DgnCategory.
ECINSTANCE_ID_CLASS(DgnTrueColorId)     //!< An Id that is assigned to a true color. See DgnDb#Colors.
ECINSTANCE_ID_CLASS(DgnViewId)          //!< An Id that is assigned to a view. See DgnDb#Views, ViewController.

BEREPOSITORYBASED_ID_CLASS(DgnMaterialId)      //!< An Id that is assigned to a material. See DgnDb#Materials.
BEREPOSITORYBASED_ID_CLASS(DgnRasterFileId)    //!< An Id that is assigned to a raster file.
BEREPOSITORYBASED_ID_CLASS(DgnSessionId)       //!< An Id that is assigned to a session. See DgnDb#Sessions.

BESERVER_ISSUED_ID_CLASS(DgnFontId);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct IdSet : bset<BeRepositoryBasedId>
{
    DGNPLATFORM_EXPORT void FromJson (Json::Value const& in);
    DGNPLATFORM_EXPORT void ToJson (Json::Value& out) const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
template<typename IdType> struct ECIdSet
{
private:
    IdSet m_set;

public:
    ECIdSet(){static_assert(sizeof(IdType)==sizeof(BeRepositoryBasedId),"ECIdSets may only contain BeRepositoryBasedId");}

    typedef bset<IdType> T_SetType;
    typedef typename T_SetType::const_iterator const_iterator;
    typedef typename T_SetType::iterator iterator;

    const_iterator begin() const {return ((T_SetType&)m_set).begin();}
    const_iterator end() const {return ((T_SetType&)m_set).end();}
    const_iterator find(IdType id) const {return ((T_SetType&)m_set).find(id);}
    bool empty() const {return m_set.empty();}
    void clear() {m_set.clear();}
    size_t size() const {return m_set.size();}
    bpair<iterator,bool> insert (IdType const& val) {return ((T_SetType&)m_set).insert(val);}
    void insert (const_iterator first, const_iterator last) {((T_SetType&)m_set).insert(first,last);}
    size_t erase (IdType const& val) {return ((T_SetType&)m_set).erase(val);}
    iterator erase (iterator it) {return ((T_SetType&)m_set).erase(it);}
    bool Contains(IdType id) const {return end() != find(id);}
    void FromJson (Json::Value const& in) {m_set.FromJson(in);}
    void ToJson (Json::Value& out) const {m_set.ToJson(out);}
};

typedef ECIdSet<DgnElementId> DgnElementIdSet;
typedef ECIdSet<DgnModelId> DgnModelIdSet;
typedef ECIdSet<DgnCategoryId> DgnCategoryIdSet;

//=======================================================================================
//! A DgnClassId is the local id for an ECClass in a DgnDb
//=======================================================================================
struct DgnClassId : BeInt64Id<DgnClassId>
{
    DgnClassId() {Invalidate();}
    explicit DgnClassId(int64_t val) : BeInt64Id(val) {}
    DgnClassId(DgnClassId&& rhs) : BeInt64Id<DgnClassId> (std::move(rhs)) {}
    DgnClassId(DgnClassId const& rhs) : BeInt64Id<DgnClassId>(rhs) {}
    DgnClassId& operator=(DgnClassId const& rhs) {m_id = rhs.m_id; return *this;}
    bool Validate() const {return m_id!=0 && m_id!=-1;}
    void Invalidate() {m_id = -1;}
};

//=======================================================================================
//! The key (classId,instanceId) of an element
//=======================================================================================
struct DgnElementKey : BeSQLite::EC::ECInstanceKey
{
    DgnElementKey() : BeSQLite::EC::ECInstanceKey() {}
    DgnElementKey (ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey (classId, instanceId) {}
    DgnElementKey (DgnClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey (classId.GetValue(), instanceId) {}

    //! Converts an ECInstanceKey to a DgnElementKey.
    //! @note Does a simple type conversion without checking if the specified ECInstanceKey is a valid DgnElementKey
    explicit DgnElementKey(BeSQLite::EC::ECInstanceKeyCR key) : BeSQLite::EC::ECInstanceKey(key) {}

    DgnClassId GetClassId() const {return DgnClassId(GetECClassId());}
    DgnElementId GetElementId() const {return DgnElementId(GetECInstanceId().GetValue());}
};

typedef DgnElementKey const& DgnElementKeyCR;

//=======================================================================================
//! The key (classId,instanceId) of a the Item aspect.
//=======================================================================================
struct ElementItemKey : BeSQLite::EC::ECInstanceKey
{
    ElementItemKey() : BeSQLite::EC::ECInstanceKey() {}
    ElementItemKey(ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey(classId, instanceId) {}
    ElementItemKey(DgnClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey(classId.GetValue(), instanceId) {}
    //! Converts an ECInstanceKey to a ElementItemKey.
    //! @note Does a simple type conversion without checking if the specified ECInstanceKey is a valid ElementItemKey
    explicit ElementItemKey(BeSQLite::EC::ECInstanceKeyCR key) : BeSQLite::EC::ECInstanceKey(key) {}
    //! Return the DgnElementId held by this key.
    //! @note The ECInstanceId of an Element and its ElementGeom aspect are the same.
    DgnElementId GetElementId() const {return DgnElementId(GetECInstanceId().GetValue());}
    DgnClassId GetClassId() const {return DgnClassId(GetECClassId());}
};

typedef ElementItemKey const& ElementItemKeyCR;

//=======================================================================================
//! Bounding box is a DRange3d the holds the min/max values for an object in each of x,y,z in some coordinate system.
//! A bounding box makes no guarantee of 
// @bsiclass
//=======================================================================================
struct BoundingBox3d : DRange3d
{
    BoundingBox3d() {DRange3d::Init();}
    explicit BoundingBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    bool IsValid() const {return !IsEmpty();}
};

//=======================================================================================
//! Bounding box that is aligned with the axes of a DgnModels::Model::CoordinateSpace
// @bsiclass
//=======================================================================================
struct AxisAlignedBox3d : BoundingBox3d
{
    AxisAlignedBox3d() {}
    explicit AxisAlignedBox3d(DRange3dCR range) {DRange3d::InitFrom(range.low, range.high);}
    explicit AxisAlignedBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    AxisAlignedBox3d(DPoint3dCR low, DPoint3dCR high) {DRange3d::InitFrom(low, high);}
};

//=======================================================================================
//! Bounding box that is aligned with the local coordinate system of a DgnElement
// @bsiclass
//=======================================================================================
struct ElementAlignedBox3d : BoundingBox3d
{
    ElementAlignedBox3d() {}
    explicit ElementAlignedBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    ElementAlignedBox3d(double left, double front, double bottom, double right, double back, double top) {DRange3d::InitFrom(left, front, bottom, right, back, top);}

    double GetLeft() const {return low.x;}
    double GetFront() const {return low.y;}
    double GetBottom() const {return low.z;}
    double GetRight() const {return high.x;}
    double GetBack() const {return high.y;}
    double GetTop() const {return high.z;}
    double GetWidth() const {return XLength();}
    double GetDepth() const {return YLength();}
    double GetHeight() const {return ZLength();}
    void SetLeft(double left) {low.x = left;}
    void SetFront(double front) {low.y = front;}
    void SetBottom(double bottom) {low.z = bottom;}
    void SetRight(double right) {high.x = right;}
    void SetBack(double back) {high.y = back;}
    void SetTop(double top) {high.z = top;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BoundingBox2d : DRange2d
{
    BoundingBox2d() {DRange2d::Init();}
    bool IsValid() const {return !IsEmpty();}
};

//=======================================================================================
//! Bounding box that is aligned with the axes of a DgnModels::Model::CoordinateSpace
// @bsiclass
//=======================================================================================
struct AxisAlignedBox2d : BoundingBox2d
{
    AxisAlignedBox2d() {}
    AxisAlignedBox2d(DRange2dCR range) {DRange2d::InitFrom(range.low, range.high);}
    AxisAlignedBox2d(DPoint2dCR low, DPoint2dCR high) {DRange2d::InitFrom(low, high);}
};

//=======================================================================================
//! Bounding box that is aligned with the local coordinate system of a DgnElement
// @bsiclass
//=======================================================================================
struct ElementAlignedBox2d : BoundingBox2d
{
    ElementAlignedBox2d() {}
    ElementAlignedBox2d(double left, double bottom, double right, double top) {DRange2d::InitFrom(left, bottom, right, top);}

    double GetLeft() const {return low.x;}
    double GetBottom() const {return low.y;}
    double GetRight() const {return high.x;}
    double GetTop() const {return high.y;}
    double GetWidth() const {return XLength();}
    double GetHeight() const {return YLength();}
    void SetLeft(double left) {low.x = left;}
    void SetBottom(double bottom) {low.y = bottom;}
    void SetRight(double right) {high.x = right;}
    void SetTop(double top) {high.y = top;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
enum NpcCorners     /// The 8 corners of the NPC cube.
{
    NPC_000           = 0,  //!< Left bottom rear
    NPC_100           = 1,  //!< Right bottom rear
    NPC_010           = 2,  //!< Left top rear
    NPC_110           = 3,  //!< Right top rear
    NPC_001           = 4,  //!< Left bottom front
    NPC_101           = 5,  //!< Right bottom front
    NPC_011           = 6,  //!< Left top front
    NPC_111           = 7,  //!< Right top front

    NPC_LeftBottomRear    = 0,
    NPC_RightBottomRear   = 1,
    NPC_LeftTopRear       = 2,
    NPC_RightTopRear      = 3,
    NPC_LeftBottomFront   = 4,
    NPC_RightBottomFront  = 5,
    NPC_LeftTopFront      = 6,
    NPC_RightTopFront     = 7,

    NPC_CORNER_COUNT  = 8
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct Frustum
{
    DPoint3d m_pts[8];
    DPoint3dCP GetPts() const {return m_pts;}
    DPoint3dP GetPtsP() {return m_pts;}
    DPoint3dCR GetCorner(int i) const {return *(m_pts+i);}
    DPoint3dR GetCornerR(int i) {return *(m_pts+i);}
    DPoint3d GetCenter() const {DPoint3d center; center.Interpolate(m_pts[NPC_111], 0.5, m_pts[NPC_000]); return center;}
    void Multiply(TransformCR trans) {trans.Multiply(m_pts, m_pts, 8);}
    void Translate(DVec3dCR offset) {for (auto& pt : m_pts) pt.Add(offset);}
    Frustum TransformBy(TransformCR trans) {Frustum out; trans.Multiply(out.m_pts, m_pts, 8); return out;}
    DRange3d ToRange() const {DRange3d range; range.InitFrom(m_pts, 8); return range;}
    void Invalidate() {memset(this, 0, sizeof(*this));}
    bool operator==(Frustum const& rhs) const {return 0==memcmp(m_pts, rhs.m_pts, sizeof(*this));}
    bool operator!=(Frustum const& rhs) const {return !(*this == rhs);}
};

//=======================================================================================
//! A list of DgnElementPtr's.
// @bsiclass                                                    Keith.Bentley   02/14
//=======================================================================================
struct DgnElementPtrVec : bvector<DgnElementPtr>
{
    const_iterator Find(DgnElementCR val) const
        {
        for (auto it=begin(); it!=end(); ++it)
            {
            if (it->get() == &val)
                return it;
            }
        return end(); // not found
        }
};

//=======================================================================================
//! A list of DgnElementCPtr's.
// @bsiclass
//=======================================================================================
struct DgnElementCPtrVec : bvector<DgnElementCPtr>
{
    const_iterator Find(DgnElementCR val) const
        {
        for (auto it=begin(); it!=end(); ++it)
            {
            if (it->get() == &val)
                return it;
            }
        return end(); // not found
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ElementIdSet : bset<DgnElementId>, BeSQLite::VirtualSet
{
    virtual bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const
        {
        BeAssert(nVals == 1);
        return (find(vals[0].GetValueId<DgnElementId>()) != end());
        }
};

/** @cond BENTLEY_SDK_Internal */

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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum DgnPlatformConstants
{
    MIN_LINECODE                    = 0,
    MAX_LINECODE                    = 7,
    MINIMUM_WINDOW_DEPTH            = -32767,
    MAXIMUM_WINDOW_DEPTH            = 32767,
};

enum DgnPlatformInvalidSymbology
{
    INVALID_STYLE = 0x7fffff00,
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
enum struct DgnFontType { TrueType = 1, Rsc = 2, Shx = 3 };

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
enum struct DgnFontStyle { Regular, Bold, Italic, BoldItalic };

/** @endcond */

//! Enumeration of possible coordinate system types
enum class DgnCoordSystem
{
    Screen    = 0,     //!< Coordinates are relative to the origin of the screen
    View      = 1,     //!< Coordinates are relative to the origin of the view
    Npc       = 2,     //!< Coordinates are relative to normalized plane coordinates.
    World     = 3,     //!< Coordinates are relative to the <i>world</i> coordinate system for the physical elements in the DgnDb
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
    MAX_GRADIENT_KEYS =  8,
};

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

enum class ClipVolumePass
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
enum class AgendaEvent
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
    NoGeoCoordinateSystem           = 2,    //!< LocationEvent was ignored and did not modify the view because the DgnDb does not have a geographic coordinate system defined
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

typedef bvector<double> T_DoubleVector;
typedef T_DoubleVector*        T_DoubleVectorP, &T_DoubleVectorR;
typedef T_DoubleVector const*  T_DoubleVectorCP;
typedef T_DoubleVector const&  T_DoubleVectorCR;

#define   IMAXI8      INT64_MAX
#define   IMINI8      INT64_MIN
#define   IMAXUI8     UINT64_MAX

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct HsvColorDef
{
    int32_t  hue;           /* red=0, yellow, green, cyan, blue, magenta */
    int32_t  saturation;    /* 0=white, 100=no white, tints */
    int32_t  value;         /* 0=black, 100=no black, shades */
};

//=======================================================================================
//! RGBA values for a color
//! @bsiclass
//=======================================================================================
struct ColorDef
{
private:
    Byte    m_red;
    Byte    m_green;
    Byte    m_blue;
    Byte    m_alpha;
    uint32_t* AsUInt32() {return reinterpret_cast<uint32_t*>(this);}

public:
    void SetColors (Byte r, Byte g, Byte b, Byte a) {m_red = r; m_green = g; m_blue = b; m_alpha = a;}
    void SetAllColors (Byte val) {m_red = m_green = m_blue = val;}
    void SetRed(Byte v) {m_red=v;}
    void SetGreen(Byte v) {m_green=v;}
    void SetBlue(Byte v) {m_blue=v;}
    void SetAlpha(Byte v) {m_alpha=v;}
    Byte GetRed() const {return m_red;}
    Byte GetGreen() const {return m_green;}
    Byte GetBlue() const {return m_blue;}
    Byte GetAlpha() const {return m_alpha;}

    uint32_t GetValue() const {return *reinterpret_cast<uint32_t const*>(this);}
    uint32_t GetValueNoAlpha() const {return 0xffffff & GetValue();}

    bool operator==(ColorDef const& rhs) const {return GetValue() == rhs.GetValue();}
    bool operator!=(ColorDef const& rhs) const {return GetValue() != rhs.GetValue();}

    ColorDef () {*AsUInt32() = 0;}
    explicit ColorDef (uint32_t intval) {*AsUInt32()=intval;}
    ColorDef (Byte red, Byte green, Byte blue, Byte alpha=0) {SetColors (red,green,blue,alpha);}

    static ColorDef White()      {return ColorDef(255,255,255);}
    static ColorDef Black()      {return ColorDef(0,0,0);}
    static ColorDef Magenta()    {return ColorDef(255,0,255);}
    static ColorDef Blue()       {return ColorDef(0,0,255);}
    static ColorDef Red()        {return ColorDef(255,0,0);}
    static ColorDef Green()      {return ColorDef(0,255,0);}
    static ColorDef LightGrey()  {return ColorDef(0xbb,0xbb,0xbb);}
    static ColorDef DarkGrey()   {return ColorDef(0x55,0x55,0x55);}
    static ColorDef MediumGrey() {return ColorDef(0x88,0x88,0x88);}
    static ColorDef Yellow()     {return ColorDef(0xff,0xff,0);}
    static ColorDef DarkYellow() {return ColorDef(0x80,0x80,0);}
    static ColorDef Violet()     {return ColorDef(0x80,0,0x80);}
    static ColorDef Maroon()     {return ColorDef(0x80,0,0);}
};

//__PUBLISH_SECTION_END__

#define QV_RESERVED_DISPLAYPRIORITY     (32)
#define MAX_HW_DISPLAYPRIORITY          ((1<<23)-QV_RESERVED_DISPLAYPRIORITY)
#define RESERVED_DISPLAYPRIORITY        (1<<19)

// Used for verifying published tests in DgnPlatformTest are using published headers. DO NOT REMOVE.
#define __DGNPLATFORM_NON_PUBLISHED_HEADER__ 1
/*__PUBLISH_SECTION_START__*/

#define TO_BOOL(x) (0 != (x))

/** @endcond */

END_BENTLEY_DGNPLATFORM_NAMESPACE
