/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @namespace BentleyApi::Dgn Types defined by the %DgnPlatform.
    @ref PAGE_DgnPlatform
*/

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeFileName.h>
#include <Bentley/ByteStream.h>
#include "ExportMacros.h"
#include <Geom/GeomApi.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include "DgnPlatform.r.h"
#include "DgnPlatformErrors.h"
#include "DgnHost.h"
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <BeSQLite/RTreeMatch.h>
#include <ECDb/ECDbApi.h>

#define USING_NAMESPACE_BENTLEY_DGNPLATFORM using namespace BentleyApi::Dgn; // for backwards compatibility, do not use
#define USING_NAMESPACE_BENTLEY_DGN         using namespace BentleyApi::Dgn;
#define USING_NAMESPACE_BENTLEY_RENDER      using namespace BentleyApi::Dgn::Render;

#define DGNPLATFORM_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DGN_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_DGN_NAMESPACE

#define DGNPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_DGN_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_DGN_NAMESPACE

#define GEOCOORD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_NAMESPACE namespace GeoCoordinates {DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) } END_BENTLEY_NAMESPACE

#define BENTLEY_RENDER_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_RENDER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_RENDER_NAMESPACE

#define BENTLEY_RENDER_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_RENDER_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_RENDER_NAMESPACE

#define TO_BOOL(x) (0 != (x))

BENTLEY_NAMESPACE_TYPEDEFS(BitMask)
BENTLEY_NAMESPACE_TYPEDEFS(GPArray)
BENTLEY_NAMESPACE_TYPEDEFS(GraphicsPointArray)
BENTLEY_NAMESPACE_TYPEDEFS(BeJsContext)
BENTLEY_NAMESPACE_TYPEDEFS(BeJsNativePointer)
BENTLEY_NAMESPACE_TYPEDEFS(BeJsEnvironment)

DGNPLATFORM_TYPEDEFS(AnnotationElement)
DGNPLATFORM_TYPEDEFS(AnnotationElement2d)
DGNPLATFORM_TYPEDEFS(AxisAlignedBox2d)
DGNPLATFORM_TYPEDEFS(AxisAlignedBox3d)
DGNPLATFORM_TYPEDEFS(BoundingBox2d)
DGNPLATFORM_TYPEDEFS(BoundingBox3d)
DGNPLATFORM_TYPEDEFS(CameraViewDefinition)
DGNPLATFORM_TYPEDEFS(CameraViewController)
DGNPLATFORM_TYPEDEFS(Caret)
DGNPLATFORM_TYPEDEFS(CategorySelector)
DGNPLATFORM_TYPEDEFS(ChangeAnnotationScale)
DGNPLATFORM_TYPEDEFS(ClipPrimitive)
DGNPLATFORM_TYPEDEFS(ClipVector)
DGNPLATFORM_TYPEDEFS(ColorDef)
DGNPLATFORM_TYPEDEFS(ComponentDef)
DGNPLATFORM_TYPEDEFS(ComponentModel)
DGNPLATFORM_TYPEDEFS(DecorateContext)
DGNPLATFORM_TYPEDEFS(DefinitionElement)
DGNPLATFORM_TYPEDEFS(Dgn3DInputEvent)
DGNPLATFORM_TYPEDEFS(DgnAuthority)
DGNPLATFORM_TYPEDEFS(DgnButtonEvent)
DGNPLATFORM_TYPEDEFS(DgnCode)
DGNPLATFORM_TYPEDEFS(DgnColorMap)
DGNPLATFORM_TYPEDEFS(DgnDb)
DGNPLATFORM_TYPEDEFS(DgnDbExpressionContext);
DGNPLATFORM_TYPEDEFS(DgnDimStyle)
DGNPLATFORM_TYPEDEFS(DgnDomain)
DGNPLATFORM_TYPEDEFS(DgnElement)
DGNPLATFORM_TYPEDEFS(DgnElementExpressionContext);
DGNPLATFORM_TYPEDEFS(DgnFont)
DGNPLATFORM_TYPEDEFS(DgnGCS)
DGNPLATFORM_TYPEDEFS(DgnGeometryPart)
DGNPLATFORM_TYPEDEFS(DgnGestureEvent)
DGNPLATFORM_TYPEDEFS(DgnGlyph)
DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutContext)
DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutResult)
DGNPLATFORM_TYPEDEFS(DgnHost)
DGNPLATFORM_TYPEDEFS(DgnImportContext)
DGNPLATFORM_TYPEDEFS(DgnMarkupProject)
DGNPLATFORM_TYPEDEFS(DgnModel)
DGNPLATFORM_TYPEDEFS(DgnMouseWheelEvent)
DGNPLATFORM_TYPEDEFS(DgnProgressMeter)
DGNPLATFORM_TYPEDEFS(DgnRevision)
DGNPLATFORM_TYPEDEFS(DgnRscFont)
DGNPLATFORM_TYPEDEFS(DgnScript)
DGNPLATFORM_TYPEDEFS(DgnShxFont)
DGNPLATFORM_TYPEDEFS(DgnTrueTypeFont)
DGNPLATFORM_TYPEDEFS(DgnViewport)
DGNPLATFORM_TYPEDEFS(DisplayStyle)
DGNPLATFORM_TYPEDEFS(DrawingGraphic)
DGNPLATFORM_TYPEDEFS(DrawingModel)
DGNPLATFORM_TYPEDEFS(DrawingViewController)
DGNPLATFORM_TYPEDEFS(DrawingViewDefinition)
DGNPLATFORM_TYPEDEFS(DropGeometry)
DGNPLATFORM_TYPEDEFS(DropGraphics)
DGNPLATFORM_TYPEDEFS(DynamicsContext)
DGNPLATFORM_TYPEDEFS(ECSqlClassParams)
DGNPLATFORM_TYPEDEFS(ElementAlignedBox2d)
DGNPLATFORM_TYPEDEFS(ElementAlignedBox3d)
DGNPLATFORM_TYPEDEFS(ElementLocateManager)
DGNPLATFORM_TYPEDEFS(EmbeddedFileLink)
DGNPLATFORM_TYPEDEFS(FenceManager)
DGNPLATFORM_TYPEDEFS(FenceParams)
DGNPLATFORM_TYPEDEFS(FitContext)
DGNPLATFORM_TYPEDEFS(Frustum)
DGNPLATFORM_TYPEDEFS(FunctionalElement)
DGNPLATFORM_TYPEDEFS(GeomDetail)
DGNPLATFORM_TYPEDEFS(GeometricPrimitive)
DGNPLATFORM_TYPEDEFS(GeometryBuilder)
DGNPLATFORM_TYPEDEFS(GeometrySource)
DGNPLATFORM_TYPEDEFS(GeometrySource2d)
DGNPLATFORM_TYPEDEFS(GeometrySource3d)
DGNPLATFORM_TYPEDEFS(GeometryStream)
DGNPLATFORM_TYPEDEFS(GeometryStreamEntryId)
DGNPLATFORM_TYPEDEFS(GroupInformationElement)
DGNPLATFORM_TYPEDEFS(HatchLinkage)
DGNPLATFORM_TYPEDEFS(HitDetail)
DGNPLATFORM_TYPEDEFS(HitList)
DGNPLATFORM_TYPEDEFS(IACSManager)
DGNPLATFORM_TYPEDEFS(IAuxCoordSys)
DGNPLATFORM_TYPEDEFS(IBriefcaseManager)
DGNPLATFORM_TYPEDEFS(IDgnFontData)
DGNPLATFORM_TYPEDEFS(IEditManipulator)
DGNPLATFORM_TYPEDEFS(IElemTopology)
DGNPLATFORM_TYPEDEFS(IElementGroup)
DGNPLATFORM_TYPEDEFS(IFaceMaterialAttachments)
DGNPLATFORM_TYPEDEFS(IGeoCoordinateServices)
DGNPLATFORM_TYPEDEFS(IGeometryProcessor)
DGNPLATFORM_TYPEDEFS(ILineStyle)
DGNPLATFORM_TYPEDEFS(ILineStyleComponent)
DGNPLATFORM_TYPEDEFS(InformationContentElement)
DGNPLATFORM_TYPEDEFS(IPickGeom)
DGNPLATFORM_TYPEDEFS(IRedrawAbort)
DGNPLATFORM_TYPEDEFS(IRedrawOperation)
DGNPLATFORM_TYPEDEFS(IRepositoryManager)
DGNPLATFORM_TYPEDEFS(ISolidKernelEntity)
DGNPLATFORM_TYPEDEFS(ISubEntity)
DGNPLATFORM_TYPEDEFS(IVariableMonitor)
DGNPLATFORM_TYPEDEFS(LineStyleContext)
DGNPLATFORM_TYPEDEFS(LinkElement)
DGNPLATFORM_TYPEDEFS(LinkModel)
DGNPLATFORM_TYPEDEFS(ModelSelector)
DGNPLATFORM_TYPEDEFS(NotificationManager)
DGNPLATFORM_TYPEDEFS(OrthographicViewController)
DGNPLATFORM_TYPEDEFS(OrthographicViewDefinition)
DGNPLATFORM_TYPEDEFS(PatternParams)
DGNPLATFORM_TYPEDEFS(PhysicalElement)
DGNPLATFORM_TYPEDEFS(PhysicalModel)
DGNPLATFORM_TYPEDEFS(PhysicalType)
DGNPLATFORM_TYPEDEFS(Placement2d)
DGNPLATFORM_TYPEDEFS(Placement3d)
DGNPLATFORM_TYPEDEFS(PropertyContext)
DGNPLATFORM_TYPEDEFS(RedlineModel)
DGNPLATFORM_TYPEDEFS(RedlineViewController)
DGNPLATFORM_TYPEDEFS(RegionGraphicsContext)
DGNPLATFORM_TYPEDEFS(RenderContext)
DGNPLATFORM_TYPEDEFS(RevisionManager)
DGNPLATFORM_TYPEDEFS(QueryViewController)
DGNPLATFORM_TYPEDEFS(ScanCriteria)
DGNPLATFORM_TYPEDEFS(SceneContext)
DGNPLATFORM_TYPEDEFS(SelectionSetManager)
DGNPLATFORM_TYPEDEFS(SheetViewController)
DGNPLATFORM_TYPEDEFS(SheetViewDefinition)
DGNPLATFORM_TYPEDEFS(SnapContext)
DGNPLATFORM_TYPEDEFS(SnapDetail)
DGNPLATFORM_TYPEDEFS(SpatialElement)
DGNPLATFORM_TYPEDEFS(SpatialModel)
DGNPLATFORM_TYPEDEFS(SpatialRedlineModel)
DGNPLATFORM_TYPEDEFS(SpatialViewController)
DGNPLATFORM_TYPEDEFS(SpatialRedlineViewController)
DGNPLATFORM_TYPEDEFS(SpatialViewController)
DGNPLATFORM_TYPEDEFS(SpatialViewDefinition)
DGNPLATFORM_TYPEDEFS(Subject)
DGNPLATFORM_TYPEDEFS(TerrainContext)
DGNPLATFORM_TYPEDEFS(TextString)
DGNPLATFORM_TYPEDEFS(TextStringStyle)
DGNPLATFORM_TYPEDEFS(TransformInfo)
DGNPLATFORM_TYPEDEFS(TxnManager)
DGNPLATFORM_TYPEDEFS(UrlLink)
DGNPLATFORM_TYPEDEFS(ViewContext)
DGNPLATFORM_TYPEDEFS(ViewController)
DGNPLATFORM_TYPEDEFS(ViewDefinition)
DGNPLATFORM_TYPEDEFS(ViewDefinition2d)
DGNPLATFORM_TYPEDEFS(ViewManager)

DGNPLATFORM_REF_COUNTED_PTR(AnnotationElement)
DGNPLATFORM_REF_COUNTED_PTR(AnnotationElement2d)
DGNPLATFORM_REF_COUNTED_PTR(CameraViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(ClipPrimitive)
DGNPLATFORM_REF_COUNTED_PTR(ClipVector)
DGNPLATFORM_REF_COUNTED_PTR(CameraViewController)
DGNPLATFORM_REF_COUNTED_PTR(ComponentDef)
DGNPLATFORM_REF_COUNTED_PTR(ComponentModel)
DGNPLATFORM_REF_COUNTED_PTR(DefinitionElement)
DGNPLATFORM_REF_COUNTED_PTR(DgnAuthority)
DGNPLATFORM_REF_COUNTED_PTR(DgnDb)
DGNPLATFORM_REF_COUNTED_PTR(DgnDbExpressionContext)
DGNPLATFORM_REF_COUNTED_PTR(DgnElement)
DGNPLATFORM_REF_COUNTED_PTR(DgnElementExpressionContext)
DGNPLATFORM_REF_COUNTED_PTR(DgnFont)
DGNPLATFORM_REF_COUNTED_PTR(DgnGCS)
DGNPLATFORM_REF_COUNTED_PTR(DgnGeometryPart)
DGNPLATFORM_REF_COUNTED_PTR(DgnMarkupProject)
DGNPLATFORM_REF_COUNTED_PTR(DgnModel)
DGNPLATFORM_REF_COUNTED_PTR(DgnRevision)
DGNPLATFORM_REF_COUNTED_PTR(DgnViewport)
DGNPLATFORM_REF_COUNTED_PTR(DrawingViewController)
DGNPLATFORM_REF_COUNTED_PTR(CategorySelector)
DGNPLATFORM_REF_COUNTED_PTR(DisplayStyle)
DGNPLATFORM_REF_COUNTED_PTR(DrawingGraphic)
DGNPLATFORM_REF_COUNTED_PTR(DrawingViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(GeometricPrimitive)
DGNPLATFORM_REF_COUNTED_PTR(EmbeddedFileLink)
DGNPLATFORM_REF_COUNTED_PTR(IBriefcaseManager)
DGNPLATFORM_REF_COUNTED_PTR(IElemTopology)
DGNPLATFORM_REF_COUNTED_PTR(ISolidKernelEntity)
DGNPLATFORM_REF_COUNTED_PTR(ISubEntity)
DGNPLATFORM_REF_COUNTED_PTR(LinkElement)
DGNPLATFORM_REF_COUNTED_PTR(LinkModel)
DGNPLATFORM_REF_COUNTED_PTR(ModelSelector)
DGNPLATFORM_REF_COUNTED_PTR(OrthographicViewController)
DGNPLATFORM_REF_COUNTED_PTR(OrthographicViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(PatternParams)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalElement)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalModel)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalType)
DGNPLATFORM_REF_COUNTED_PTR(ProgressiveTask)
DGNPLATFORM_REF_COUNTED_PTR(QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(SpatialElement)
DGNPLATFORM_REF_COUNTED_PTR(SpatialModel)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(SpatialViewController)
DGNPLATFORM_REF_COUNTED_PTR(SpatialViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(Subject)
DGNPLATFORM_REF_COUNTED_PTR(TextString)
DGNPLATFORM_REF_COUNTED_PTR(TextStringStyle)
DGNPLATFORM_REF_COUNTED_PTR(TxnManager)
DGNPLATFORM_REF_COUNTED_PTR(UrlLink)
DGNPLATFORM_REF_COUNTED_PTR(ViewController)
DGNPLATFORM_REF_COUNTED_PTR(ViewDefinition)

BEGIN_BENTLEY_RENDER_NAMESPACE
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Device)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GradientSymb)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Graphic)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicBuilder)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicList)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(IGraphicBuilder)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ISprite)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Image)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ImageSource)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleSymb)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Material)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(OvrGraphicParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Plan)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(System)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Target)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Task)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Window)

    DEFINE_REF_COUNTED_PTR(Device)
    DEFINE_REF_COUNTED_PTR(GradientSymb)
    DEFINE_REF_COUNTED_PTR(Graphic)
    DEFINE_REF_COUNTED_PTR(GraphicList)
    DEFINE_REF_COUNTED_PTR(LineStyleInfo)
    DEFINE_REF_COUNTED_PTR(Material)
    DEFINE_REF_COUNTED_PTR(Target)
    DEFINE_REF_COUNTED_PTR(Task)
    DEFINE_REF_COUNTED_PTR(Texture)
    DEFINE_REF_COUNTED_PTR(Window)
END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_NAMESPACE

BEBRIEFCASEBASED_ID_CLASS(DgnElementId)       //!< An Id that is assigned to an Element. @ingroup GROUP_DgnElement
BEBRIEFCASEBASED_ID_CLASS(DgnModelId)         //!< An Id that is assigned to a DgnModel.  A DgnModel is a container for DgnElements. @ingroup GROUP_DgnModel
BEBRIEFCASEBASED_ID_CLASS(DgnLinkId)          //!< An Id that is assigned to a DGN link. See DgnLinkTable.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnGeometryPartId, DgnElementId) //!< A DgnElementId that identifies a DgnGeometryPart.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnMaterialId, DgnElementId) //!< An element Id that refers to a material.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnTextureId, DgnElementId) //!< An element Id that refers to a named texture.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnLightId, DgnElementId) //!< An element Id that refers to a light definition.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnStyleId, DgnElementId) //!< An Id that is assigned to a style. See DgnDb#Styles.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnCategoryId, DgnElementId) //!< An element Id that refers to a DgnCategory. @ingroup GROUP_DgnCategory
BEBRIEFCASEBASED_ID_SUBCLASS(DgnSubCategoryId, DgnElementId) //!< An element Id that refers to a DgnSubCategory. @ingroup GROUP_DgnCategory
BEBRIEFCASEBASED_ID_SUBCLASS(DgnTrueColorId, DgnElementId) //!< An element Id that refers a a DgnTrueColor.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnViewId, DgnElementId) //!< An element Id that refers to a ViewDefinition.

BESERVER_ISSUED_ID_CLASS(DgnAuthorityId)
BESERVER_ISSUED_ID_CLASS(DgnFontId)

namespace dgn_ElementHandler{struct Element;};
namespace dgn_ModelHandler  {struct Model;};
namespace dgn_AuthorityHandler {struct Authority;};
typedef struct dgn_ElementHandler::Element* ElementHandlerP;
typedef struct dgn_ElementHandler::Element& ElementHandlerR;
typedef struct dgn_ModelHandler::Model* ModelHandlerP;
typedef struct dgn_ModelHandler::Model& ModelHandlerR;
typedef struct dgn_AuthorityHandler::Authority* AuthorityHandlerP;
typedef struct dgn_AuthorityHandler::Authority& AuthorityHandlerR;
typedef Byte const* ByteCP;

typedef BeSQLite::IdSet<DgnElementId> DgnElementIdSet;            //!< IdSet with DgnElementId members. @ingroup GROUP_DgnElement
typedef BeSQLite::IdSet<DgnModelId> DgnModelIdSet;                //!< IdSet with DgnModelId members. @ingroup GROUP_DgnModel
typedef BeSQLite::IdSet<DgnCategoryId> DgnCategoryIdSet;          //!< IdSet with DgnCategoryId members. @ingroup GROUP_DgnCategory
typedef BeSQLite::IdSet<DgnSubCategoryId> DgnSubCategoryIdSet;    //!< IdSet with DgnSubCategoryId members. @ingroup GROUP_DgnCategory
typedef BeSQLite::IdSet<DgnMaterialId> DgnMaterialIdSet;          //!< IdSet with DgnMaterialId members.

typedef ECN::ECClassId DgnClassId;

//=======================================================================================
//! The GeometryStreamEntryId class identifies a geometric primitive in a GeometryStream.
//=======================================================================================
struct GeometryStreamEntryId
{
private:
    DgnGeometryPartId   m_partId;       // Valid when m_index refers to a part
    uint16_t            m_index;        // Index into top-level GeometryStream
    uint16_t            m_partIndex;    // Index into part GeometryStream

public:
    GeometryStreamEntryId() {Init();}
    GeometryStreamEntryId(GeometryStreamEntryIdCR rhs) {m_partId = rhs.m_partId; m_index = rhs.m_index; m_partIndex = rhs.m_partIndex;}

    bool operator==(GeometryStreamEntryIdCR rhs) const {if (this == &rhs) return true; return (m_partId == rhs.m_partId && m_index == rhs.m_index && m_partIndex == rhs.m_partIndex);}
    bool operator!=(GeometryStreamEntryIdCR rhs) const {return !(*this == rhs);}
    GeometryStreamEntryIdR operator=(GeometryStreamEntryIdCR rhs) {m_partId = rhs.m_partId; m_index = rhs.m_index; m_partIndex = rhs.m_partIndex; return *this;}

    void Init() {m_index = 0; m_partIndex = 0; m_partId = DgnGeometryPartId();}
    void SetGeometryPartId(DgnGeometryPartId partId) {m_partId = partId; m_partIndex = 0;}
    void SetIndex(uint16_t index) {m_index = index;}
    void SetPartIndex(uint16_t partIndex) {m_partIndex = partIndex;}

    DgnGeometryPartId GetGeometryPartId() const {return m_partId;}
    uint16_t GetIndex() const {return m_index;}
    uint16_t GetPartIndex() const {return m_partIndex;}
    bool IsValid() const {return 0 != m_index;}

    void SetActive(bool enable) {if (m_partId.IsValid()) {if (!enable) SetGeometryPartId(DgnGeometryPartId()); return;} Init();}
    void SetActiveGeometryPart(DgnGeometryPartId partId) {SetGeometryPartId(partId);}
    void Increment() {if (m_partId.IsValid()) SetPartIndex(GetPartIndex()+1); else SetIndex(GetIndex()+1);}
};

//=======================================================================================
//! A DRange3d that holds min/max values for an object in each of x,y,z in some coordinate system.
//! @note A BoundingBox3d makes no guarantee that the box is the minimum (smallest) box possible, just that no portion of the object
//! described by it will extend beyond its values.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct BoundingBox3d : DRange3d
{
    BoundingBox3d() {DRange3d::Init();}
    explicit BoundingBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    bool IsValid() const {return !IsEmpty();}
};

//=======================================================================================
//! A BoundingBox3d that is aligned with the axes of a CoordinateSpace.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct AxisAlignedBox3d : BoundingBox3d
{
    AxisAlignedBox3d() {}
    explicit AxisAlignedBox3d(DRange3dCR range) {DRange3d::InitFrom(range.low, range.high);}
    explicit AxisAlignedBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    AxisAlignedBox3d(DPoint3dCR lowPt, DPoint3dCR highPt) {DRange3d::InitFrom(lowPt, highPt);}
    DPoint3d GetCenter() const {return DPoint3d::FromInterpolate(low, .5, high);}
};

//=======================================================================================
//! A BoundingBox3d that is aligned with the local coordinate system of a DgnElement.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct ElementAlignedBox3d : BoundingBox3d
{
    ElementAlignedBox3d() {}
    explicit ElementAlignedBox3d(DRange2dCR range2d) {DRange3d::InitFrom(&range2d.low, 2, 0.0);}
    ElementAlignedBox3d(double left, double front, double bottom, double right, double back, double top) {DRange3d::InitFrom(left, front, bottom, right, back, top);}
    explicit ElementAlignedBox3d(DRange3dCR range) {DRange3d::InitFrom(range.low, range.high);}

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
//! A DRange2d that holds min/max values for an object in each of x and y in some coordinate system.
//! @note A BoundingBox2d makes no guarantee that the box is the minimum (smallest) box possible, just that no portion of the object
//! described by it will extend beyond its values.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct BoundingBox2d : DRange2d
{
    BoundingBox2d() {DRange2d::Init();}
    bool IsValid() const {return !IsEmpty();}
};

//=======================================================================================
//! A BoundingBox2d that is aligned with the axes of a CoordinateSpace.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct AxisAlignedBox2d : BoundingBox2d
{
    AxisAlignedBox2d() {}
    AxisAlignedBox2d(DRange2dCR range) {DRange2d::InitFrom(range.low, range.high);}
    AxisAlignedBox2d(DPoint2dCR low, DPoint2dCR high) {DRange2d::InitFrom(low, high);}
};

//=======================================================================================
//! A BoundingBox2d that is aligned with the local coordinate system of a DgnElement.
// @bsiclass                                                    Keith.Bentley   03/14
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
//! The 8 corners of the NPC cube.
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
enum NpcCorners
{
    NPC_000 = 0,  //!< Left bottom rear
    NPC_100 = 1,  //!< Right bottom rear
    NPC_010 = 2,  //!< Left top rear
    NPC_110 = 3,  //!< Right top rear
    NPC_001 = 4,  //!< Left bottom front
    NPC_101 = 5,  //!< Right bottom front
    NPC_011 = 6,  //!< Left top front
    NPC_111 = 7,  //!< Right top front

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
//! The region of physical (3d) space that appears in a view. It forms the field-of-view of a camera.
//! It is stored as 8 points, in NpcCorner order, that must define a truncated pyramid.
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
    void ToRangeR(DRange3dR range) const {range.InitFrom(m_pts, 8);}
    DRange3d ToRange() const {DRange3d range; range.InitFrom(m_pts, 8); return range;}
    DGNPLATFORM_EXPORT void ScaleAboutCenter(double scale);
    void Invalidate() {memset(this, 0, sizeof(*this));}
    bool operator==(Frustum const& rhs) const {return 0==memcmp(m_pts, rhs.m_pts, sizeof(*this));}
    bool operator!=(Frustum const& rhs) const {return !(*this == rhs);}
    Frustum() {} // uninitialized!
    DGNPLATFORM_EXPORT explicit Frustum(DRange3dCR);
    explicit Frustum(BeSQLite::RTree3dValCR);
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

//! @private
enum class ConfigurationVariableLevel
{
    Predefined    = -2,        //!< predefined by the host
    SysEnv        = -1,        //!< defined in the Windows system environment variable table
    System        = 0,         //!< system defined
    Application   = 1,         //!< application defined
    Organization  = 2,         //!< Organization defined
    WorkSpace     = 3,         //!< WorkSpace defined
    WorkSet       = 4,         //!< WorkSet defined
    Role          = 5,         //!< Role defined.
    User          = 6,         //!< user defined
};

//! @private
enum DgnPlatformConstants
{
    MIN_LINECODE                    = 0,
    MAX_LINECODE                    = 7,
};

//! A kind of script
enum class DgnScriptType{JavaScript=0, TypeScript=1};

//! @private
enum class DgnFontType {TrueType=1, Rsc=2, Shx=3,};

//! @private
enum class DgnFontStyle {Regular, Bold, Italic, BoldItalic,};

//! Enumeration of possible coordinate system types
enum class DgnCoordSystem
{
    Screen    = 0,     //!< Coordinates are relative to the origin of the screen
    View      = 1,     //!< Coordinates are relative to the origin of the view
    Npc       = 2,     //!< Coordinates are relative to normalized plane coordinates.
    World     = 3,     //!< Coordinates are relative to the world coordinate system for the physical elements in the DgnDb
};

enum class ClipMask
{
    None  = 0,
    XLow  = (0x0001 << 0),
    XHigh = (0x0001 << 1),
    YLow  = (0x0001 << 2),
    YHigh = (0x0001 << 3),
    ZLow  = (0x0001 << 4),
    ZHigh = (0x0001 << 5),
    XAndY = (0x000f),         // (XLow | XHigh | YLow | YHigh)
    All   = (0x003f),         // (XAndY | ZLow | ZHigh),
};

ENUM_IS_FLAGS(ClipMask)

//! Values held in line style definition elements; normally not used by clients of this API
//! @ingroup LineStyleManagerModule
enum class LsComponentType
{
    Unknown         = 0,             //!<  Unknown, should never occur
    PointSymbol     = 1,
    Compound        = 2,
    LineCode        = 3,
    LinePoint       = 4,
    Internal        = 6,
    RasterImage     = 7,
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LsComponentId
{
private:
    uint32_t            m_number;              // Component property ID
    LsComponentType     m_type;
public:
    uint32_t GetValue() const {return m_number;}
    LsComponentType GetType() const {return m_type;}
    LsComponentId() {m_type = LsComponentType::Unknown; m_number = 0xFFFFFFFF;}
    bool IsValid() const {return m_number != 0xFFFFFFFF;}
    explicit LsComponentId(LsComponentType type, uint32_t value) : m_type(type), m_number(value) {}

    bool operator<(LsComponentId const&r) const
        {
        if (m_type < r.m_type)
            return true;

        if (m_type > r.m_type)
            return false;

        return m_number < r.m_number;
        }
};

enum class SnapStatus
{
    Success              = SUCCESS,
    Aborted              = 1,
    NoElements           = 2,
    Disabled             = 100,
    NoSnapPossible       = 200,
    NotSnappable         = 300,
    ModelNotSnappable    = 301,
    FilteredByCategory   = 400,
    FilteredByUser       = 500,
    FilteredByApp        = 600,
    FilteredByAppQuietly = 700,
};

//! @private
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

//! Values for NotificationManager::OutputMessage
//! @private
enum class OutputMessageAlert
{
    None     = 0,
    Dialog   = 1,
    Balloon  = 2,
};

enum class GridOrientationType
{
    View    = 0,
    WorldXY = 1,           //!< Top
    WorldYZ = 2,           //!< Right
    WorldXZ = 3,           //!< Front
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

enum class HitDetailType
{
    Hit          = 1,
    Snap         = 2,
    Intersection = 3,
};

enum DitherModes
{
    DITHERMODE_Pattern              = 0,
    DITHERMODE_ErrorDiffusion       = 1,
};

//! Influences how handler should apply annotation scale.
enum class AnnotationScaleAction
{
    Update  = 0,
    Add     = 1,
    Remove  = 2,
};

//! Influences how handler should apply fence clip.
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

enum class DrawPurpose
{
    NotSpecified = 0,
    CreateTerrain,
    CreateScene,
    Progressive,
    Plot,
    Pick,
    CaptureGeometry,
    Decorate,
    FenceAccept,
    RegionFlood,                 //!< Collect graphics to find closed regions/flood...
    FitView,
    ExportVisibleEdges,
    ClashDetection,
    ModelFacet,
    Measure,
    VisibilityCalculation,
    Dynamics,
    Redraw,
    Heal,
};

//! Used to communicate the result of handling an event from a GPS.
enum class GeoLocationEventStatus
{
    EventHandled                    = 0,    //!< LocationEvent was handled and modified the view
    EventIgnored                    = 1,    //!< LocationEvent was ignored and did not modify the view because of an unknown failure to convert
    NoGeoCoordinateSystem           = 2,    //!< LocationEvent was ignored and did not modify the view because the DgnDb does not have a geographic coordinate system defined
    PointOutsideGeoCoordinateSystem = 3,    //!< LocationEvent was ignored and did not modify the view because the LocationEvent's point is outside the bounds where the geographic coordinate system is accurate
};

//! Used to specify desired accuracy.
enum class GeoLocationServicesAccuracy
{
    BestForNavigation       = 1,
    Best                    = 2,
    Coarse                  = 3
};

//! Used to describe status of a location provider.
enum class GeoLocationProviderStatus
{
    NotDetermined           = 0,
    Disabled                = 1,
    Enabled                 = 2,
    TemporarilyUnavailable  = 3,
    Available               = 4,
    OutOfService            = 5,
    LocationUnavailable     = 6,
    Initializing            = 7,
};

typedef bvector<double> T_DoubleVector;
typedef T_DoubleVector* T_DoubleVectorP, &T_DoubleVectorR;
typedef T_DoubleVector const* T_DoubleVectorCP;
typedef T_DoubleVector const& T_DoubleVectorCR;

//=======================================================================================
//! @ingroup GROUP_DgnView
//=======================================================================================
enum class StandardView
{
    NotStandard = -1,
    Top         = 1,
    Bottom      = 2,
    Left        = 3,
    Right       = 4,
    Front       = 5,
    Back        = 6,
    Iso         = 7,
    RightIso    = 8,
};

//=======================================================================================
//! RGBA values for a color
//! @ingroup GROUP_Appearance
//=======================================================================================
struct ColorDef
{
private:
    union
    {
        uint32_t m_intVal;
        struct
        {
            Byte m_red;
            Byte m_green;
            Byte m_blue;
            Byte m_alpha;
        };
    };

public:
    void SetColors(Byte r, Byte g, Byte b, Byte a) {m_red = r; m_green = g; m_blue = b; m_alpha = a;}
    void SetAllColors(Byte val) {m_red = m_green = m_blue = val;}
    void SetRed(Byte v) {m_red=v;}
    void SetGreen(Byte v) {m_green=v;}
    void SetBlue(Byte v) {m_blue=v;}
    void SetAlpha(Byte v) {m_alpha=v;}
    Byte GetRed() const {return m_red;}
    Byte GetGreen() const {return m_green;}
    Byte GetBlue() const {return m_blue;}
    Byte GetAlpha() const {return m_alpha;}

    uint32_t GetValue() const {return m_intVal;} //<! for use with Render primitives
    uint32_t GetValueRgba() const {return ColorDef(m_alpha, m_blue, m_green, m_red).GetValue();} //<! for use with UI controls
    uint32_t GetValueNoAlpha() const {return 0xffffff & GetValue();}

    bool operator==(ColorDef const& rhs) const {return GetValue() == rhs.GetValue();}
    bool operator!=(ColorDef const& rhs) const {return GetValue() != rhs.GetValue();}

    ColorDef() {m_intVal = 0;}
    explicit ColorDef(uint32_t intval) {m_intVal=intval;}
    ColorDef(Byte red, Byte green, Byte blue, Byte alpha=0) {SetColors(red,green,blue,alpha);}

    static ColorDef Black()       {return ColorDef(0,0,0);}
    static ColorDef White()       {return ColorDef(0xff,0xff,0xff);}
    static ColorDef Red()         {return ColorDef(0xff,0,0);}
    static ColorDef Green()       {return ColorDef(0,0xff,0);}       //<! Lime
    static ColorDef Blue()        {return ColorDef(0,0,0xff);}
    static ColorDef Yellow()      {return ColorDef(0xff,0xff,0);}
    static ColorDef Cyan()        {return ColorDef(0,0xff,0xff);}
    static ColorDef Magenta()     {return ColorDef(0xff,0,0xff);}
    static ColorDef Brown()       {return ColorDef(0xa5,0x2a,0x2a);}
    static ColorDef LightGrey()   {return ColorDef(0xbb,0xbb,0xbb);}
    static ColorDef MediumGrey()  {return ColorDef(0x88,0x88,0x88);}
    static ColorDef DarkGrey()    {return ColorDef(0x55,0x55,0x55);}
    static ColorDef DarkRed()     {return ColorDef(0x80,0,0);}       //<! Maroon
    static ColorDef DarkGreen()   {return ColorDef(0,0x80,0);}       //<! Green
    static ColorDef DarkBlue()    {return ColorDef(0,0,0x80);}       //<! Navy
    static ColorDef DarkYellow()  {return ColorDef(0x80,0x80,0);}    //<! Olive
    static ColorDef DarkCyan()    {return ColorDef(0,0x80,0x80);}    //<! Teal
    static ColorDef DarkMagenta() {return ColorDef(0x80,0,0x80);}    //<! Purple
    static ColorDef DarkBrown()   {return ColorDef(0x8b,0x45,0x13);}

    static ColorDef NotSelected() {return ColorDef(0x49,0x98,0xc8);} //<! Bluish color used to denote unselected state
    static ColorDef Selected()    {return ColorDef(0xf6,0xcc,0x7f);} //<! Orangish color used to denote selected state
};

//__PUBLISH_SECTION_END__
// Used for verifying published tests in DgnPlatformTest are using published headers. DO NOT REMOVE.
#define __DGNPLATFORM_NON_PUBLISHED_HEADER__ 1
/*__PUBLISH_SECTION_START__*/

END_BENTLEY_DGN_NAMESPACE
