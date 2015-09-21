/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
#include "ExportMacros.h"
#include <Geom/GeomApi.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/bvector.h>
#include "DgnPlatform.r.h"
#include "DgnPlatformErrors.r.h"
#include "DgnHost.h"
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <ECDb/ECDbApi.h>

#define USING_NAMESPACE_BENTLEY_DGNPLATFORM using namespace BentleyApi::Dgn;

#define GLOBAL_TYPEDEF1(_sName_,_name_,structunion) \
    structunion _sName_; \
    namespace BENTLEY_NAMESPACE_NAME {\
    typedef structunion _sName_*          _name_##P, &_name_##R;  \
    typedef structunion _sName_ const*    _name_##CP; \
    typedef structunion _sName_ const&    _name_##CR;}

#define GLOBAL_TYPEDEF(_sName_,_name_) GLOBAL_TYPEDEF1(_sName_,_name_,struct)

#define DGNPLATFORM_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_DGNPLATFORM_NAMESPACE

#define DGNPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_DGNPLATFORM_NAMESPACE

#define GEOCOORD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_NAMESPACE namespace GeoCoordinates { DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) } END_BENTLEY_NAMESPACE

GLOBAL_TYPEDEF (QvElem,QvElem)
GLOBAL_TYPEDEF (QvCache,QvCache)
GLOBAL_TYPEDEF (QvView,QvView)
GLOBAL_TYPEDEF (QvMRImage,QvMRImage)

BENTLEY_NAMESPACE_TYPEDEFS (BitMask)
BENTLEY_NAMESPACE_TYPEDEFS (DataExternalizer)
BENTLEY_NAMESPACE_TYPEDEFS (GPArray)
BENTLEY_NAMESPACE_TYPEDEFS (GraphicsPointArray)
BENTLEY_NAMESPACE_TYPEDEFS (IRefCounted)
BENTLEY_NAMESPACE_TYPEDEFS (BeJsContext)
BENTLEY_NAMESPACE_TYPEDEFS (BeJsEnvironment)

DGNPLATFORM_TYPEDEFS (ColorDef)
DGNPLATFORM_TYPEDEFS (BoundingBox2d)
DGNPLATFORM_TYPEDEFS (BoundingBox3d)
DGNPLATFORM_TYPEDEFS (DgnDb)
DGNPLATFORM_TYPEDEFS (DgnElement)
DGNPLATFORM_TYPEDEFS (DgnElement2d)
DGNPLATFORM_TYPEDEFS (DgnElement3d)
DGNPLATFORM_TYPEDEFS (DgnFont)
DGNPLATFORM_TYPEDEFS (DgnGeomPart)
DGNPLATFORM_TYPEDEFS (DgnGlyph)
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutContext)
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutResult)
DGNPLATFORM_TYPEDEFS (DgnMarkupProject)
DGNPLATFORM_TYPEDEFS (DgnModel)
DGNPLATFORM_TYPEDEFS (DgnImportContext)
DGNPLATFORM_TYPEDEFS (DgnAuthority)
DGNPLATFORM_TYPEDEFS (DgnResourceURI)
DGNPLATFORM_TYPEDEFS (DgnGlyph)
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutContext)
DGNPLATFORM_TYPEDEFS (DgnGlyphLayoutResult)
DGNPLATFORM_TYPEDEFS (DgnRscFont)
DGNPLATFORM_TYPEDEFS (DgnShxFont)
DGNPLATFORM_TYPEDEFS (DgnTrueTypeFont)
DGNPLATFORM_TYPEDEFS (DgnViewport)
DGNPLATFORM_TYPEDEFS (DisplayStyle)
DGNPLATFORM_TYPEDEFS (DisplayStyleFlags)
DGNPLATFORM_TYPEDEFS (DrawingElement)
DGNPLATFORM_TYPEDEFS (ElementGroup)
DGNPLATFORM_TYPEDEFS (GeomStream)
DGNPLATFORM_TYPEDEFS (GeometricElement)
DGNPLATFORM_TYPEDEFS (GradientSymb)
DGNPLATFORM_TYPEDEFS (IDgnFontData)
DGNPLATFORM_TYPEDEFS (IDrawGeom)
DGNPLATFORM_TYPEDEFS (IElemTopology)
DGNPLATFORM_TYPEDEFS (IRedrawOperation)
DGNPLATFORM_TYPEDEFS (IRedrawAbort)
DGNPLATFORM_TYPEDEFS (ITransientGeometryHandler)
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
DGNPLATFORM_TYPEDEFS (DgnElementExpressionContext);

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_REF_COUNTED_PTR (TextString)
DGNPLATFORM_REF_COUNTED_PTR (TextStringStyle)

DGNPLATFORM_TYPEDEFS (AxisAlignedBox2d)
DGNPLATFORM_TYPEDEFS (AxisAlignedBox3d)
DGNPLATFORM_TYPEDEFS (Caret)
DGNPLATFORM_TYPEDEFS (ChangeAnnotationScale)
DGNPLATFORM_TYPEDEFS (ClipPrimitive)
DGNPLATFORM_TYPEDEFS (ClipVector)
DGNPLATFORM_TYPEDEFS (ClipVolumeOverrides)
DGNPLATFORM_TYPEDEFS (ComponentModel)
DGNPLATFORM_TYPEDEFS (CutGraphicsCachedKey)
DGNPLATFORM_TYPEDEFS (Dgn3DInputEvent)
DGNPLATFORM_TYPEDEFS (DgnButtonEvent)
DGNPLATFORM_TYPEDEFS (DgnColorMap)
DGNPLATFORM_TYPEDEFS (DgnDimStyle)
DGNPLATFORM_TYPEDEFS (DgnDomain)
DGNPLATFORM_TYPEDEFS (DgnGestureEvent)
DGNPLATFORM_TYPEDEFS (DgnHost)
DGNPLATFORM_TYPEDEFS (DgnMouseWheelEvent)
DGNPLATFORM_TYPEDEFS (DgnProgressMeter)
DGNPLATFORM_TYPEDEFS (DgnScript)
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
DGNPLATFORM_TYPEDEFS (ElementLocateManager)
DGNPLATFORM_TYPEDEFS (FenceManager)
DGNPLATFORM_TYPEDEFS (FenceParams)
DGNPLATFORM_TYPEDEFS (Frustum)
DGNPLATFORM_TYPEDEFS (GeomDetail)
DGNPLATFORM_TYPEDEFS (GeomStreamEntryId)
DGNPLATFORM_TYPEDEFS (HatchLinkage)
DGNPLATFORM_TYPEDEFS (HitList)
DGNPLATFORM_TYPEDEFS (HitDetail)
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
DGNPLATFORM_TYPEDEFS (TxnManager)
DGNPLATFORM_TYPEDEFS (IViewHandlerHitInfo)
DGNPLATFORM_TYPEDEFS (IViewTransients)
DGNPLATFORM_TYPEDEFS (IndexedViewSet)
DGNPLATFORM_TYPEDEFS (IndexedViewport)
DGNPLATFORM_TYPEDEFS (LineStyleInfo)
DGNPLATFORM_TYPEDEFS (LineStyleParams)
DGNPLATFORM_TYPEDEFS (Material)
DGNPLATFORM_TYPEDEFS (MaterialAssignment)
DGNPLATFORM_TYPEDEFS (NotificationManager)
DGNPLATFORM_TYPEDEFS (OvrMatSymb)
DGNPLATFORM_TYPEDEFS (ParagraphProperties)
DGNPLATFORM_TYPEDEFS (PatternParams)
DGNPLATFORM_TYPEDEFS (PermanentTopologicalId)
DGNPLATFORM_TYPEDEFS (PersistentElementPath)
DGNPLATFORM_TYPEDEFS (PersistentSnapDetail)
DGNPLATFORM_TYPEDEFS (PhysicalModel)
DGNPLATFORM_TYPEDEFS (PhysicalRedlineViewController)
DGNPLATFORM_TYPEDEFS (PhysicalViewController)
DGNPLATFORM_TYPEDEFS (Placement2d)
DGNPLATFORM_TYPEDEFS (Placement3d)
DGNPLATFORM_TYPEDEFS (PropertyContext)
DGNPLATFORM_TYPEDEFS (QVAliasMaterialId)
DGNPLATFORM_TYPEDEFS (QvOutput)
DGNPLATFORM_TYPEDEFS (QueryModel)
DGNPLATFORM_TYPEDEFS (QueryViewController)
DGNPLATFORM_TYPEDEFS (QvUnsizedKey)
DGNPLATFORM_TYPEDEFS (QvViewport)
DGNPLATFORM_TYPEDEFS (RedlineViewController)
DGNPLATFORM_TYPEDEFS (RegionGraphicsContext)
DGNPLATFORM_TYPEDEFS (ScanCriteria)
DGNPLATFORM_TYPEDEFS (SelectionSetManager)
DGNPLATFORM_TYPEDEFS (SheetViewController)
DGNPLATFORM_TYPEDEFS (SnapContext)
DGNPLATFORM_TYPEDEFS (SnapDetail)
DGNPLATFORM_TYPEDEFS (StampQvElemMap)
DGNPLATFORM_TYPEDEFS (TextString)
DGNPLATFORM_TYPEDEFS (TextStringStyle)
DGNPLATFORM_TYPEDEFS (TransformClipStack)
DGNPLATFORM_TYPEDEFS (TransformInfo)
DGNPLATFORM_TYPEDEFS (UpdateContext)
DGNPLATFORM_TYPEDEFS (ViewHandler)
DGNPLATFORM_TYPEDEFS (ViewManager)
DGNPLATFORM_TYPEDEFS (VisibleEdgeCache)
DGNPLATFORM_TYPEDEFS (DgnMaterials)
DGNPLATFORM_TYPEDEFS (DgnTextures)
/** @endcond */
DGNPLATFORM_TYPEDEFS (DgnGCS)
DGNPLATFORM_TYPEDEFS (IGeoCoordinateServices)

DGNPLATFORM_REF_COUNTED_PTR (DgnDb)
DGNPLATFORM_REF_COUNTED_PTR (DgnDbExpressionContext)
DGNPLATFORM_REF_COUNTED_PTR (DgnElement)
DGNPLATFORM_REF_COUNTED_PTR (DgnElementExpressionContext)
DGNPLATFORM_REF_COUNTED_PTR (DgnFont)
DGNPLATFORM_REF_COUNTED_PTR (DgnGCS)
DGNPLATFORM_REF_COUNTED_PTR (DgnGeomPart)
DGNPLATFORM_REF_COUNTED_PTR (DgnMarkupProject)
DGNPLATFORM_REF_COUNTED_PTR (DgnModel)
DGNPLATFORM_REF_COUNTED_PTR (DrawingElement)
DGNPLATFORM_REF_COUNTED_PTR (ElementGroup)
DGNPLATFORM_REF_COUNTED_PTR (GeometricElement)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalElement)
DGNPLATFORM_REF_COUNTED_PTR (ComponentModel)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalModel)
DGNPLATFORM_REF_COUNTED_PTR (PhysicalRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR (RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR (SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR (TxnManager)
DGNPLATFORM_REF_COUNTED_PTR (DgnAuthority)

/** @cond BENTLEY_SDK_Internal */
DGNPLATFORM_REF_COUNTED_PTR (ClipPrimitive)
DGNPLATFORM_REF_COUNTED_PTR (ClipVector)
DGNPLATFORM_REF_COUNTED_PTR (PatternParams)
DGNPLATFORM_REF_COUNTED_PTR (DisplayStyleHandlerSettings)
DGNPLATFORM_REF_COUNTED_PTR (IElemTopology)
DGNPLATFORM_REF_COUNTED_PTR (IProgressiveDisplay)
DGNPLATFORM_REF_COUNTED_PTR (ViewController)
/** @endcond */

//__PUBLISH_SECTION_END__
// ///////////////////////////////////////////////////////////////////////////////////////////////////
// DO NOT USE: these MAX*LENGTH values are not portable or correct!
// ///////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_BENTLEY_NAMESPACE

enum
{
    DGNPLATFORM_RESOURCE_MAXFILELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXNAMELENGTH                    = 256,
    DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH               = 256,

    MAXFILELENGTH         = DGNPLATFORM_RESOURCE_MAXFILELENGTH,
    MAXDIRLENGTH          = 256,
    MAXDEVICELENGTH       = 256,
    MAXNAMELENGTH         = DGNPLATFORM_RESOURCE_MAXNAMELENGTH,
    MAXEXTENSIONLENGTH    = DGNPLATFORM_RESOURCE_MAXEXTENSIONLENGTH,
};

END_BENTLEY_NAMESPACE

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

BEREPOSITORYBASED_ID_CLASS(DgnElementId)       //!< An Id that is assigned to an Element. @ingroup DgnElementGroup
BEREPOSITORYBASED_ID_CLASS(DgnGeomPartId)      //!< An Id that is assigned to a DgnGeomPart.
BEREPOSITORYBASED_ID_CLASS(DgnModelId)         //!< An Id that is assigned to a DgnModel.  A DgnModel is a container for DgnElements. @ingroup DgnModelGroup
BEREPOSITORYBASED_ID_CLASS(DgnViewId)          //!< An Id that is assigned to a view. See DgnDb#Views, ViewController. @ingroup DgnViewGroup
BEREPOSITORYBASED_ID_CLASS(DgnLinkId)          //!< An Id that is assigned to a DGN link. See DgnLinkTable.

BESERVER_ISSUED_ID_CLASS(DgnAuthorityId)
BESERVER_ISSUED_ID_CLASS(DgnCategoryId)      //!< An Id that is assigned to a DgnCategory.  A DgnElement belongs to exactly one DgnCategory. @ingroup DgnCategoryGroup
BESERVER_ISSUED_ID_CLASS(DgnFontId)
BESERVER_ISSUED_ID_CLASS(DgnLightId)         //!< An Id that is assigned to a light. See DgnDb#Lights.
BESERVER_ISSUED_ID_CLASS(DgnMaterialId)      //!< An Id that is assigned to a material. See DgnDb#Materials.
BESERVER_ISSUED_ID_CLASS(DgnSessionId)       //!< An Id that is assigned to a session. See DgnDb#Sessions.
BESERVER_ISSUED_ID_CLASS(DgnStyleId)         //!< An Id that is assigned to a style. See DgnDb#Styles.
BESERVER_ISSUED_ID_CLASS(DgnSubCategoryId)   //!< An Id that is assigned to a SubCategory of a DgnCategory. @ingroup DgnCategoryGroup
BESERVER_ISSUED_ID_CLASS(DgnTrueColorId)     //!< An Id that is assigned to a true color. See DgnDb#Colors.
BESERVER_ISSUED_ID_CLASS(DgnTextureId)       //!< An Id that is assigned to a texture. See DgnDb#Textures.

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

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct BeRepositoryBasedIdSet : bset<BeSQLite::BeRepositoryBasedId>
{
    DGNPLATFORM_EXPORT void FromJson(Json::Value const& in);
    DGNPLATFORM_EXPORT void ToJson(Json::Value& out) const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
template<typename IdType> struct IdSet : BeRepositoryBasedIdSet, BeSQLite::VirtualSet
{
private:
    BeRepositoryBasedIdSet m_set;

    virtual bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const
        {
        BeAssert(nVals == 1);
        return Contains(IdType(vals[0].GetValueInt64()));
        }
public:
    IdSet(){static_assert(sizeof(IdType)==sizeof(BeSQLite::BeRepositoryBasedId),"IdSets may only contain BeRepositoryBasedId");}

    typedef BentleyApi::bset<IdType> T_SetType;
    typedef typename T_SetType::const_iterator const_iterator;
    typedef typename T_SetType::iterator iterator;

    const_iterator begin() const {return ((T_SetType&)m_set).begin();}
    const_iterator end() const {return ((T_SetType&)m_set).end();}
    const_iterator find(IdType id) const {return ((T_SetType&)m_set).find(id);}
    bool empty() const {return m_set.empty();}
    void clear() {m_set.clear();}
    size_t size() const {return m_set.size();}
    bpair<iterator,bool> insert(IdType const& val) {BeAssert(val.IsValid()); return ((T_SetType&)m_set).insert(val);}
    void insert(const_iterator first, const_iterator last) {((T_SetType&)m_set).insert(first,last);}
    size_t erase(IdType const& val) {return ((T_SetType&)m_set).erase(val);}
    iterator erase(iterator it) {return ((T_SetType&)m_set).erase(it);}

    bool Contains(IdType id) const {return end() != find(id);}

    void FromJson(Json::Value const& in) {m_set.FromJson(in);}
    void ToJson(Json::Value& out) const {m_set.ToJson(out);}

    BeRepositoryBasedIdSet const& GetRepositoryBasedIdSet() const { return m_set; }
};

typedef IdSet<DgnElementId> DgnElementIdSet;    //!< IdSet with DgnElementId members. @ingroup DgnElementGroup
typedef IdSet<DgnModelId> DgnModelIdSet;        //!< IdSet with DgnModelId members. @ingroup DgnModelGroup
typedef IdSet<DgnCategoryId> DgnCategoryIdSet;  //!< IdSet with DgnCategoryId members. @ingroup DgnCategoryGroup

//=======================================================================================
//! A DgnClassId is the local id for an ECClass in a DgnDb.
//=======================================================================================
struct DgnClassId : BeSQLite::BeInt64Id
{
    DgnClassId() {Invalidate();}
    explicit DgnClassId(int64_t val) : BeInt64Id(val) {}
    DgnClassId(DgnClassId&& rhs) : BeInt64Id(std::move(rhs)) {}
    DgnClassId(DgnClassId const& rhs) : BeInt64Id(rhs) {}
    DgnClassId& operator=(DgnClassId const& rhs) {m_id = rhs.m_id; return *this;}
};

//=======================================================================================
//! The key (classId,instanceId) of a DgnElement.
//=======================================================================================
struct DgnElementKey : BeSQLite::EC::ECInstanceKey
{
    DgnElementKey() : BeSQLite::EC::ECInstanceKey() {}
    DgnElementKey(ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey(classId, instanceId) {}
    DgnElementKey(DgnClassId classId, BeSQLite::EC::ECInstanceId instanceId) : BeSQLite::EC::ECInstanceKey(classId.GetValue(), instanceId) {}

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
    AxisAlignedBox3d(DPoint3dCR low, DPoint3dCR high) {DRange3d::InitFrom(low, high);}
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

//! Types used to interface with native DgnDisplayKernel
//! @private
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

//! @private
enum class ConfigurationVariableLevel
{
    Predefined    = -2,        //!< predefined by the host
    SysEnv        = -1,        //!< defined in the system environment variable table
    System        = 0,         //!< system defined
    Appl          = 1,         //!< application defined
    Site          = 2,         //!< site defined
    Project       = 3,         //!< project defined
    User          = 4,         //!< user defined
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
enum class DgnFontType { TrueType = 1, Rsc = 2, Shx = 3, };

//! @private
enum class DgnFontStyle { Regular, Bold, Italic, BoldItalic, };

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
    XAndY      = (0x000f),         // (XLow | XHigh | YLow | YHigh)
    All        = (0x003f),         // (XAndY | ZLow | ZHigh),
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
    ModelNotSnappable    = 301,
    FilteredByCategory   = 400,
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

//! Influences how handler should apply fence stretch.
enum class FenceStretchFlags
{
    /*! no special options */
    None      = (0),
    /*! stretch user defined cell components */
    Cells     = (1<<0),
};

ENUM_IS_FLAGS (FenceStretchFlags)

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


//  QvUInt32 is a temporary solution to a difference in D3D QV and OpenGL QV.  Both used unsigned long
//  types in a few places where it is important that the type be a 32-bit integer.  Once we encountered
//  a platform where unsigned long is a 64-bit integer we had to change the OpenGL QV but did not
//  want to change D3D QV. Code that uses QvUInt32 will compile correctly for either case.
#if defined (BENTLEYCONFIG_GRAPHICS_OPENGLES) || defined (BENTLEYCONFIG_GRAPHICS_OPENGL)
    typedef int32_t QvInt32;
    typedef uint32_t QvUInt32;
    typedef short QvInt16;
    typedef unsigned short QvUInt16;
#else
    #if !defined (BENTLEYCONFIG_GRAPHICS_DIRECTX)
        #error Expect BENTLEYCONFIG_GRAPHICS_DIRECTX when BENTLEYCONFIG_GRAPHICS_OPENGLES is not defined
    #endif

    typedef long QvInt32;
    typedef unsigned long QvUInt32;
    typedef short QvInt16;
    typedef unsigned short QvUInt16;
#endif

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
    RegionFlood                = 31, //!< Collect graphics to find closed regions/flood...
    FitView                    = 32,
    ExportVisibleEdges         = 36,
    InterferenceDetection      = 37,
    ModelFacet                 = 39,
    Measure                    = 40,
    VisibilityCalculation      = 41,
    ProxyHashExtraction        = 42,
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
};

typedef bvector<double> T_DoubleVector;
typedef T_DoubleVector*        T_DoubleVectorP, &T_DoubleVectorR;
typedef T_DoubleVector const*  T_DoubleVectorCP;
typedef T_DoubleVector const&  T_DoubleVectorCR;

#define   IMAXI8      INT64_MAX
#define   IMINI8      INT64_MIN
#define   IMAXUI8     UINT64_MAX

//=======================================================================================
//! RGBA values for a color
//! @ingroup DgnColorGroup
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

    uint32_t GetValue() const {return *reinterpret_cast<uint32_t const*>(this);}
    uint32_t GetValueNoAlpha() const {return 0xffffff & GetValue();}

    bool operator==(ColorDef const& rhs) const {return GetValue() == rhs.GetValue();}
    bool operator!=(ColorDef const& rhs) const {return GetValue() != rhs.GetValue();}

    ColorDef() {*AsUInt32() = 0;}
    explicit ColorDef(uint32_t intval) {*AsUInt32()=intval;}
    ColorDef(Byte red, Byte green, Byte blue, Byte alpha=0) {SetColors(red,green,blue,alpha);}

    static ColorDef Black()       {return ColorDef(0,0,0);}
    static ColorDef White()       {return ColorDef(0xff,0xff,0xff);}
    static ColorDef Red()         {return ColorDef(0xff,0,0);}
    static ColorDef Green()       {return ColorDef(0,0xff,0);}       //! Lime
    static ColorDef Blue()        {return ColorDef(0,0,0xff);}
    static ColorDef Yellow()      {return ColorDef(0xff,0xff,0);}
    static ColorDef Cyan()        {return ColorDef(0,0xff,0xff);}
    static ColorDef Magenta()     {return ColorDef(0xff,0,0xff);}
    static ColorDef LightGrey()   {return ColorDef(0xbb,0xbb,0xbb);}
    static ColorDef MediumGrey()  {return ColorDef(0x88,0x88,0x88);}
    static ColorDef DarkGrey()    {return ColorDef(0x55,0x55,0x55);}
    static ColorDef DarkRed()     {return ColorDef(0x80,0,0);}       //! Maroon
    static ColorDef DarkGreen()   {return ColorDef(0,0x80,0);}       //! Green
    static ColorDef DarkBlue()    {return ColorDef(0,0,0x80);}       //! Navy
    static ColorDef DarkYellow()  {return ColorDef(0x80,0x80,0);}    //! Olive
    static ColorDef DarkCyan()    {return ColorDef(0,0x80,0x80);}    //! Teal
    static ColorDef DarkMagenta() {return ColorDef(0x80,0,0x80);}    //! Purple

    static ColorDef NotSelected() {return ColorDef(0x49,0x98,0xc8);} //! Bluish color used to denote unselected state
    static ColorDef Selected()    {return ColorDef(0xf6,0xcc,0x7f);} //! Orangish color used to denote selected state
};

//=======================================================================================
//! Colors in elements should typically be either by-category or a specific RGBA.
//! This wraps a bool and a ColorDef to encourage and enforce this pattern, where a color is either by-category or a ColorDef, not both at the same time.
//! This means that the ColorDef is not available when by-category, but is always available otherwise. This structure will not clear the its color when toggling by-category on, so it can be recovered later.
//! Your specific scenario may or may not want to persist this way, but this structure allows it.
//! @ingroup DgnColorGroup
// @bsiclass                                                    Jeff.Marker     06/2015
//=======================================================================================
struct ElementColor
{
private:
    static const int64_t COLOR_VALUE_MASK = 0x00000000FFFFFFFF;
    static const int64_t BY_CATEGORY_MASK = 0x8000000000000000;
    bool m_isByCategory;
    ColorDef m_color;

public:
    ElementColor() : m_isByCategory(true) {}
    explicit ElementColor(ColorDef color) : m_isByCategory(false), m_color(color) {}
    explicit ElementColor(int64_t value) : m_isByCategory(BY_CATEGORY_MASK == (BY_CATEGORY_MASK & value)), m_color((uint32_t)(COLOR_VALUE_MASK & value)) {}
    bool operator==(ElementColor const& rhs) const { return m_isByCategory == rhs.m_isByCategory && m_color == rhs.m_color; }
    bool IsByCategory() const { return m_isByCategory; }
    void SetIsByCategory(bool value) { m_isByCategory = value; }
    ColorDefCP GetColorCP() const { return m_isByCategory ? nullptr : &m_color; }
    ColorDefP GetColorP() { return m_isByCategory ? nullptr : &m_color; }
    void SetColor(ColorDef value) { m_isByCategory = false; m_color = value; }
    void SetColor(ColorDefCP value) { m_isByCategory = (nullptr == value); if (value) m_color = *value; }
    int64_t ToInt64() const { int64_t value = (int64_t)m_color.GetValue(); if (m_isByCategory) value |= BY_CATEGORY_MASK; return value; }
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
