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
#include "DgnPlatformErrors.r.h"
#include "DgnHost.h"
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>
#include <ECDb/ECDbApi.h>

#define USING_NAMESPACE_BENTLEY_DGNPLATFORM using namespace BentleyApi::Dgn; // for backwards compatibility, do not use
#define USING_NAMESPACE_BENTLEY_DGN         using namespace BentleyApi::Dgn;
#define USING_NAMESPACE_BENTLEY_RENDER      using namespace BentleyApi::Dgn::Render;
#define GLOBAL_TYPEDEF1(_sName_,_name_,structunion) \
    structunion _sName_; \
    namespace BENTLEY_NAMESPACE_NAME {\
    typedef structunion _sName_*          _name_##P, &_name_##R;  \
    typedef structunion _sName_ const*    _name_##CP; \
    typedef structunion _sName_ const&    _name_##CR;}

#define GLOBAL_TYPEDEF(_sName_,_name_) GLOBAL_TYPEDEF1(_sName_,_name_,struct)

#define DGNPLATFORM_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_DGN_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_DGN_NAMESPACE

#define DGNPLATFORM_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_DGN_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_DGN_NAMESPACE

#define GEOCOORD_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_NAMESPACE namespace GeoCoordinates { DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) } END_BENTLEY_NAMESPACE

#define TO_BOOL(x) (0 != (x))

BENTLEY_NAMESPACE_TYPEDEFS(BitMask)
BENTLEY_NAMESPACE_TYPEDEFS(GPArray)
BENTLEY_NAMESPACE_TYPEDEFS(GraphicsPointArray)
BENTLEY_NAMESPACE_TYPEDEFS(BeJsContext)
BENTLEY_NAMESPACE_TYPEDEFS(BeJsEnvironment)

DGNPLATFORM_TYPEDEFS(AnnotationElement)
DGNPLATFORM_TYPEDEFS(AxisAlignedBox2d)
DGNPLATFORM_TYPEDEFS(AxisAlignedBox3d)
DGNPLATFORM_TYPEDEFS(BoundingBox2d)
DGNPLATFORM_TYPEDEFS(BoundingBox3d)
DGNPLATFORM_TYPEDEFS(Caret)
DGNPLATFORM_TYPEDEFS(ChangeAnnotationScale)
DGNPLATFORM_TYPEDEFS(ClipPrimitive)
DGNPLATFORM_TYPEDEFS(ClipVector)
DGNPLATFORM_TYPEDEFS(ClipVolumeOverrides)
DGNPLATFORM_TYPEDEFS(ColorDef)
DGNPLATFORM_TYPEDEFS(ComponentDef)
DGNPLATFORM_TYPEDEFS(ComponentModel)
DGNPLATFORM_TYPEDEFS(DecorateContext)
DGNPLATFORM_TYPEDEFS(DefinitionElement)
DGNPLATFORM_TYPEDEFS(Dgn3DInputEvent)
DGNPLATFORM_TYPEDEFS(DgnAuthority)
DGNPLATFORM_TYPEDEFS(DgnButtonEvent)
DGNPLATFORM_TYPEDEFS(DgnColorMap)
DGNPLATFORM_TYPEDEFS(DgnDb)
DGNPLATFORM_TYPEDEFS(DgnDbExpressionContext);
DGNPLATFORM_TYPEDEFS(DgnDimStyle)
DGNPLATFORM_TYPEDEFS(DgnDomain)
DGNPLATFORM_TYPEDEFS(DgnElement)
DGNPLATFORM_TYPEDEFS(DgnElementExpressionContext);
DGNPLATFORM_TYPEDEFS(DgnFont)
DGNPLATFORM_TYPEDEFS(DgnGCS)
DGNPLATFORM_TYPEDEFS(DgnGeomPart)
DGNPLATFORM_TYPEDEFS(DgnGestureEvent)
DGNPLATFORM_TYPEDEFS(DgnGlyph)
DGNPLATFORM_TYPEDEFS(DgnGlyph)
DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutContext)
DGNPLATFORM_TYPEDEFS(DgnGlyphLayoutResult)
DGNPLATFORM_TYPEDEFS(DgnHost)
DGNPLATFORM_TYPEDEFS(DgnImportContext)
DGNPLATFORM_TYPEDEFS(DgnMarkupProject)
DGNPLATFORM_TYPEDEFS(DgnModel)
DGNPLATFORM_TYPEDEFS(DgnMouseWheelEvent)
DGNPLATFORM_TYPEDEFS(DgnProgressMeter)
DGNPLATFORM_TYPEDEFS(DgnResourceURI)
DGNPLATFORM_TYPEDEFS(DgnRevision)
DGNPLATFORM_TYPEDEFS(DgnRscFont)
DGNPLATFORM_TYPEDEFS(DgnScript)
DGNPLATFORM_TYPEDEFS(DgnShxFont)
DGNPLATFORM_TYPEDEFS(DgnTrueTypeFont)
DGNPLATFORM_TYPEDEFS(DgnViewport)
DGNPLATFORM_TYPEDEFS(DictionaryElement)
DGNPLATFORM_TYPEDEFS(DisplayStyle)
DGNPLATFORM_TYPEDEFS(DisplayStyleFlags)
DGNPLATFORM_TYPEDEFS(DrawingElement)
DGNPLATFORM_TYPEDEFS(DrawingModel)
DGNPLATFORM_TYPEDEFS(DrawingViewDefinition)
DGNPLATFORM_TYPEDEFS(DropGeometry)
DGNPLATFORM_TYPEDEFS(DropGraphics)
DGNPLATFORM_TYPEDEFS(DwgHatchDef)
DGNPLATFORM_TYPEDEFS(DwgHatchDefLine)
DGNPLATFORM_TYPEDEFS(DynamicsContext)
DGNPLATFORM_TYPEDEFS(ECSqlClassParams)
DGNPLATFORM_TYPEDEFS(ElementAlignedBox2d)
DGNPLATFORM_TYPEDEFS(ElementAlignedBox3d)
DGNPLATFORM_TYPEDEFS(ElementLocateManager)
DGNPLATFORM_TYPEDEFS(FenceManager)
DGNPLATFORM_TYPEDEFS(FenceParams)
DGNPLATFORM_TYPEDEFS(Frustum)
DGNPLATFORM_TYPEDEFS(GeomDetail)
DGNPLATFORM_TYPEDEFS(GeometricPrimitive)
DGNPLATFORM_TYPEDEFS(GeometryBuilder)
DGNPLATFORM_TYPEDEFS(GeometrySource)
DGNPLATFORM_TYPEDEFS(GeometrySource2d)
DGNPLATFORM_TYPEDEFS(GeometrySource3d)
DGNPLATFORM_TYPEDEFS(GeometryStream)
DGNPLATFORM_TYPEDEFS(GeometryStreamEntryId)
DGNPLATFORM_TYPEDEFS(HatchLinkage)
DGNPLATFORM_TYPEDEFS(HitDetail)
DGNPLATFORM_TYPEDEFS(HitList)
DGNPLATFORM_TYPEDEFS(IACSManager)
DGNPLATFORM_TYPEDEFS(IAuxCoordSys)
DGNPLATFORM_TYPEDEFS(IDgnFontData)
DGNPLATFORM_TYPEDEFS(IEditAction)
DGNPLATFORM_TYPEDEFS(IEditActionArray)
DGNPLATFORM_TYPEDEFS(IEditActionSource)
DGNPLATFORM_TYPEDEFS(IEditManipulator)
DGNPLATFORM_TYPEDEFS(IElemTopology)
DGNPLATFORM_TYPEDEFS(IElementGroup)
DGNPLATFORM_TYPEDEFS(IElementState)
DGNPLATFORM_TYPEDEFS(IFaceMaterialAttachments)
DGNPLATFORM_TYPEDEFS(IGeoCoordinateServices)
DGNPLATFORM_TYPEDEFS(IGeometryProcessor)
DGNPLATFORM_TYPEDEFS(ILineStyle)
DGNPLATFORM_TYPEDEFS(ILineStyleComponent)
DGNPLATFORM_TYPEDEFS(ILocksManager)
DGNPLATFORM_TYPEDEFS(ILocksServer)
DGNPLATFORM_TYPEDEFS(IPickGeom)
DGNPLATFORM_TYPEDEFS(IRedrawAbort)
DGNPLATFORM_TYPEDEFS(IRedrawOperation)
DGNPLATFORM_TYPEDEFS(ISolidKernelEntity)
DGNPLATFORM_TYPEDEFS(ISubEntity)
DGNPLATFORM_TYPEDEFS(ITransactionHandler)
DGNPLATFORM_TYPEDEFS(ITransientGeometryHandler)
DGNPLATFORM_TYPEDEFS(IVariableMonitor)
DGNPLATFORM_TYPEDEFS(ImageBuffer)
DGNPLATFORM_TYPEDEFS(NotificationManager)
DGNPLATFORM_TYPEDEFS(ParagraphProperties)
DGNPLATFORM_TYPEDEFS(PatternParams)
DGNPLATFORM_TYPEDEFS(PermanentTopologicalId)
DGNPLATFORM_TYPEDEFS(PhysicalElement)
DGNPLATFORM_TYPEDEFS(PhysicalViewDefinition)
DGNPLATFORM_TYPEDEFS(Placement2d)
DGNPLATFORM_TYPEDEFS(Placement3d)
DGNPLATFORM_TYPEDEFS(PropertyContext)
DGNPLATFORM_TYPEDEFS(QueryModel)
DGNPLATFORM_TYPEDEFS(QueryViewController)
DGNPLATFORM_TYPEDEFS(RedlineModel)
DGNPLATFORM_TYPEDEFS(RedlineViewController)
DGNPLATFORM_TYPEDEFS(RegionGraphicsContext)
DGNPLATFORM_TYPEDEFS(RevisionManager)
DGNPLATFORM_TYPEDEFS(ScanCriteria)
DGNPLATFORM_TYPEDEFS(SelectionSetManager)
DGNPLATFORM_TYPEDEFS(SheetElement)
DGNPLATFORM_TYPEDEFS(SheetViewController)
DGNPLATFORM_TYPEDEFS(SheetViewDefinition)
DGNPLATFORM_TYPEDEFS(SnapContext)
DGNPLATFORM_TYPEDEFS(SnapDetail)
DGNPLATFORM_TYPEDEFS(SpatialElement)
DGNPLATFORM_TYPEDEFS(SpatialGroupElement)
DGNPLATFORM_TYPEDEFS(SpatialModel)
DGNPLATFORM_TYPEDEFS(SpatialRedlineModel)
DGNPLATFORM_TYPEDEFS(SpatialRedlineViewController)
DGNPLATFORM_TYPEDEFS(SpatialViewController)
DGNPLATFORM_TYPEDEFS(SpatialViewDefinition)
DGNPLATFORM_TYPEDEFS(SystemElement)
DGNPLATFORM_TYPEDEFS(TextString)
DGNPLATFORM_TYPEDEFS(TextStringStyle)
DGNPLATFORM_TYPEDEFS(TransformClipStack)
DGNPLATFORM_TYPEDEFS(TransformInfo)
DGNPLATFORM_TYPEDEFS(TxnManager)
DGNPLATFORM_TYPEDEFS(ViewContext)
DGNPLATFORM_TYPEDEFS(ViewController)
DGNPLATFORM_TYPEDEFS(ViewDefinition)
DGNPLATFORM_TYPEDEFS(ViewManager)

DGNPLATFORM_REF_COUNTED_PTR(AnnotationElement)
DGNPLATFORM_REF_COUNTED_PTR(ClipPrimitive)
DGNPLATFORM_REF_COUNTED_PTR(ClipVector)
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
DGNPLATFORM_REF_COUNTED_PTR(DgnGeomPart)
DGNPLATFORM_REF_COUNTED_PTR(DgnMarkupProject)
DGNPLATFORM_REF_COUNTED_PTR(DgnModel)
DGNPLATFORM_REF_COUNTED_PTR(DgnRevision)
DGNPLATFORM_REF_COUNTED_PTR(DgnViewport)
DGNPLATFORM_REF_COUNTED_PTR(DictionaryElement)
DGNPLATFORM_REF_COUNTED_PTR(DisplayStyleHandlerSettings)
DGNPLATFORM_REF_COUNTED_PTR(DrawingElement)
DGNPLATFORM_REF_COUNTED_PTR(DrawingViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(IElemTopology)
DGNPLATFORM_REF_COUNTED_PTR(ILocksManager)
DGNPLATFORM_REF_COUNTED_PTR(ImageBuffer)
DGNPLATFORM_REF_COUNTED_PTR(PatternParams)
DGNPLATFORM_REF_COUNTED_PTR(PatternParams)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalElement)
DGNPLATFORM_REF_COUNTED_PTR(ProgressiveDisplay)
DGNPLATFORM_REF_COUNTED_PTR(QueryViewController)
DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetElement)
DGNPLATFORM_REF_COUNTED_PTR(SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(SpatialElement)
DGNPLATFORM_REF_COUNTED_PTR(SpatialGroupElement)
DGNPLATFORM_REF_COUNTED_PTR(SpatialModel)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(SpatialViewDefinition)
DGNPLATFORM_REF_COUNTED_PTR(TextString)
DGNPLATFORM_REF_COUNTED_PTR(TextStringStyle)
DGNPLATFORM_REF_COUNTED_PTR(TxnManager)
DGNPLATFORM_REF_COUNTED_PTR(ViewController)
DGNPLATFORM_REF_COUNTED_PTR(ViewDefinition)

BEGIN_BENTLEY_DISPLAY_NAMESPACE
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Device)
    DEFINE_REF_COUNTED_PTR(Device)
END_BENTLEY_DISPLAY_NAMESPACE

BEGIN_BENTLEY_RENDER_NAMESPACE
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GradientSymb)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Graphic)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicList)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(GraphicParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ISprite)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ITiledRaster)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Image)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleInfo)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleSymb)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(LineTexture)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Material)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(MultiResImage)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(OvrGraphicParams)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Plan)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Target)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Task)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture)

    DEFINE_REF_COUNTED_PTR(GradientSymb)
    DEFINE_REF_COUNTED_PTR(Graphic)
    DEFINE_REF_COUNTED_PTR(GraphicList)
    DEFINE_REF_COUNTED_PTR(Image)
    DEFINE_REF_COUNTED_PTR(LineStyleInfo)
    DEFINE_REF_COUNTED_PTR(LineTexture)
    DEFINE_REF_COUNTED_PTR(Material)
    DEFINE_REF_COUNTED_PTR(MultiResImage)
    DEFINE_REF_COUNTED_PTR(Target)
    DEFINE_REF_COUNTED_PTR(Task)
    DEFINE_REF_COUNTED_PTR(Texture)
END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_NAMESPACE

BEBRIEFCASEBASED_ID_CLASS(DgnElementId)       //!< An Id that is assigned to an Element. @ingroup DgnElementGroup
BEBRIEFCASEBASED_ID_CLASS(DgnGeomPartId)      //!< An Id that is assigned to a DgnGeomPart.
BEBRIEFCASEBASED_ID_CLASS(DgnModelId)         //!< An Id that is assigned to a DgnModel.  A DgnModel is a container for DgnElements. @ingroup DgnModelGroup
BEBRIEFCASEBASED_ID_CLASS(DgnLinkId)          //!< An Id that is assigned to a DGN link. See DgnLinkTable.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnMaterialId, DgnElementId) //!< An element Id that refers to a material.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnTextureId, DgnElementId) //!< An element Id that refers to a named texture.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnLightId, DgnElementId) //!< An element Id that refers to a light definition.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnStyleId, DgnElementId) //!< An Id that is assigned to a style. See DgnDb#Styles.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnCategoryId, DgnElementId) //!< An element Id that refers to a DgnCategory. @ingroup DgnCategoryGroup
BEBRIEFCASEBASED_ID_SUBCLASS(DgnSubCategoryId, DgnElementId) //!< An element Id that refers to a DgnSubCategory. @ingroup DgnCategoryGroup
BEBRIEFCASEBASED_ID_SUBCLASS(DgnTrueColorId, DgnElementId) //!< An element Id that refers a a DgnTrueColor.
BEBRIEFCASEBASED_ID_SUBCLASS(DgnViewId, DgnElementId) //!< An element Id that refers to a ViewDefinition.

BESERVER_ISSUED_ID_CLASS(DgnAuthorityId)
BESERVER_ISSUED_ID_CLASS(DgnFontId)
BESERVER_ISSUED_ID_CLASS(DgnSessionId)       //!< An Id that is assigned to a session. See DgnDb#Sessions.

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
struct BeBriefcaseBasedIdSet : bset<BeSQLite::BeBriefcaseBasedId>
{
    DGNPLATFORM_EXPORT void FromJson(Json::Value const& in);
    DGNPLATFORM_EXPORT void ToJson(Json::Value& out) const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
template<typename IdType> struct IdSet : BeBriefcaseBasedIdSet, BeSQLite::VirtualSet
{
private:
    BeBriefcaseBasedIdSet m_set;

    virtual bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const
        {
        BeAssert(nVals == 1);
        return Contains(IdType(vals[0].GetValueUInt64()));
        }
public:
    IdSet(){static_assert(sizeof(IdType)==sizeof(BeSQLite::BeBriefcaseBasedId),"IdSets may only contain BeBriefcaseBasedId");}

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

    BeBriefcaseBasedIdSet const& GetBriefcaseBasedIdSet() const { return m_set; }
};

typedef IdSet<DgnElementId> DgnElementIdSet;            //!< IdSet with DgnElementId members. @ingroup DgnElementGroup
typedef IdSet<DgnModelId> DgnModelIdSet;                //!< IdSet with DgnModelId members. @ingroup DgnModelGroup
typedef IdSet<DgnCategoryId> DgnCategoryIdSet;          //!< IdSet with DgnCategoryId members. @ingroup DgnCategoryGroup
typedef IdSet<DgnSubCategoryId> DgnSubCategoryIdSet;    //!< IdSet with DgnSubCategoryId members. @ingroup DgnCategoryGroup
typedef IdSet<DgnMaterialId> DgnMaterialIdSet;          //!< IdSet with DgnMaterialId members.

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
//! The GeometryStreamEntryId class identifies a geometric primitive in a GeometryStream.
//=======================================================================================
struct GeometryStreamEntryId
{
    enum class Type
        {
        Invalid = 0,
        Indexed = 1,
        };

private:
    Type            m_type;
    DgnGeomPartId   m_partId;
    uint32_t        m_index;
    uint32_t        m_partIndex;

public:
    GeometryStreamEntryId() {Init();}
    GeometryStreamEntryId(GeometryStreamEntryIdCR rhs) {m_type = rhs.m_type; m_partId = rhs.m_partId; m_index = rhs.m_index; m_partIndex = rhs.m_partIndex;}

    DGNPLATFORM_EXPORT bool operator==(GeometryStreamEntryIdCR rhs) const;
    DGNPLATFORM_EXPORT bool operator!=(GeometryStreamEntryIdCR rhs) const;
    DGNPLATFORM_EXPORT GeometryStreamEntryIdR operator=(GeometryStreamEntryIdCR rhs);

    void Init() {m_type = Type::Invalid; m_index = 0; m_partIndex = 0; m_partId = DgnGeomPartId();}
    void SetType(Type type) {m_type = type;}
    void SetGeomPartId(DgnGeomPartId partId) {m_partId = partId; m_partIndex = 0;}
    void SetIndex(uint32_t index) {m_index = index;}
    void SetPartIndex(uint32_t partIndex) {m_partIndex = partIndex;}

    Type GetType() const {return m_type;}
    DgnGeomPartId GetGeomPartId() const {return m_partId;}
    uint32_t GetIndex() const {return m_index;}
    uint32_t GetPartIndex() const {return m_partIndex;}
};

//=======================================================================================
//! DEPRECATED: Use DgnElementId (preferred) or ECInstanceKey (for ECRelationships) instead
//! @private
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

typedef DgnElementKey const& DgnElementKeyCR; //!< @private

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
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
#endif

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

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct CheckStop
{  
private:
    bool m_aborted;

public:
    bool InitAborted(bool val) {return m_aborted = val;}
    bool ClearAborted() {return m_aborted = false;}
    bool WasAborted()  {return m_aborted;}
    bool SetAborted() {return m_aborted = true;}
    bool AddAbortTest(bool val) {return  m_aborted |= val;}

    CheckStop() {m_aborted=false;}

    //! return true to abort the current operation.
    //! @note Overrides MUST call SetAborted or use AddAbortTest since WasAborted may be directly tested!
    virtual bool _CheckStop() {return m_aborted;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct ProgressiveDisplay : RefCounted<NonCopyableClass>
{
    enum class Completion {Finished=0, Aborted=1, Failed=2};
    virtual Completion _Process(ViewContextR, uint32_t batchSize) = 0;  // if this returns Finished, it is removed from the viewport
};

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   02/04
+===============+===============+===============+===============+===============+======*/
struct StopEvents
    {
    bool    m_keystrokes;
    bool    m_wheel;
    bool    m_button;
    bool    m_buttonUp;
    bool    m_paint;
    bool    m_focus;
    bool    m_modifierKeyTransition;
    bool    m_sensor;
    bool    m_abortUpdateRequest;
    bool    m_touchMotion;          //  Ignored unless the motion exceeds range.
    bool    m_anyEvent;
    uint32_t m_touchLimit;
    uint32_t m_numTouches;
    BentleyApi::Point2d m_touches[3];

    enum StopMask
        {
        None        = 0,
        OnKeystrokes  = 1<<0,
        OnWheel       = 1<<2,
        OnButton      = 1<<3,
        OnPaint       = 1<<4,
        OnFocus       = 1<<5,
        OnModifierKey = 1<<6,
        OnTouch       = 1<<7,
        OnAbortUpdate = 1<<8,
        OnSensor      = 1<<9,   //  GPS, Gyro
        OnButtonUp    = 1<<10,
        AnyEvent      = 1<<11,   //  includes all of the other events plus unknown events

        ForFullUpdate  = OnWheel | OnAbortUpdate,             // doesn't stop on keystrokes, buttons, or touch
        ForQuickUpdate = ForFullUpdate | OnKeystrokes | OnButton | OnTouch,
        };

    void Clear()
        {
        m_keystrokes = m_wheel = m_button = m_paint = m_focus = m_modifierKeyTransition = m_abortUpdateRequest = m_touchMotion = m_anyEvent = false;
        m_touchLimit = 0;
        }

    StopEvents(int mask)
        {
        if (mask & AnyEvent)
            mask = -1;

        m_keystrokes = TO_BOOL(mask & OnKeystrokes);
        m_wheel      = TO_BOOL(mask & OnWheel);
        m_button     = TO_BOOL(mask & OnButton);
        m_buttonUp   = TO_BOOL(mask & OnButtonUp);
        m_paint      = TO_BOOL(mask & OnPaint);
        m_focus      = TO_BOOL(mask & OnFocus);
        m_sensor     = TO_BOOL(mask & OnSensor);
        m_modifierKeyTransition = TO_BOOL(mask & OnModifierKey);
        m_touchMotion = TO_BOOL(mask & OnTouch);
        m_abortUpdateRequest = TO_BOOL(mask & OnAbortUpdate);
        m_anyEvent   = TO_BOOL(mask & AnyEvent);
        m_touchLimit = 0;
        }

    void SetTouchLimit(uint32_t limit, uint32_t numTouches, Point2dCP touches);

    // Stop when the ctrl or shift key is pressed or released.
    void SetStopOnModifierKey(bool stop) {m_modifierKeyTransition = stop;}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct UpdatePlan
{
    friend struct ViewSet;

    struct Query
    {
        struct Minimum
        {
            bool      m_enable = false;
            double    m_seconds = 1.;        // check for minimum number of elements after this time during query (seconds)
            uint64_t  m_timeout;
            void Enable(bool val) {m_enable=val;}
            void SetSeconds(double seconds) {m_seconds=seconds;}
        };

        Minimum     m_minimum;
        uint32_t    m_maxTime = 1000;    // maximum time query should run (seconds)
        double      m_minPixelSize = 40;
        bool        m_wait = false;
        uint32_t    m_minElements = 300;
        uint32_t    m_maxElements = 50000;
        mutable uint32_t m_delayAfter = 0;
        mutable uint32_t m_targetNumElements;

        uint32_t GetTimeout() const {return m_maxTime;}
        uint32_t GetMinElements() const {return m_minElements;}
        uint32_t GetMaxElements() const {return m_maxElements;}
        void SetMinElements(uint32_t val) {m_minElements = val;}
        void SetMaxElements(uint32_t val) {m_maxElements = val;}
        double GetMinimumSizePixels() const {return m_minPixelSize;}
        void SetMinimumSizePixels(double val) {m_minPixelSize=val;}
        void SetTargetNumElements(uint32_t val) const {m_targetNumElements=val;}
        uint32_t GetTargetNumElements() const {return m_targetNumElements;}
        void SetWait(bool val) {m_wait=val;}
        bool WantWait() const {return m_wait;}
        uint32_t GetDelayAfter() const {return m_delayAfter;}
        void SetDelayAfter (uint32_t val) const {m_delayAfter=val;}
        Minimum& GetMinimum() {return m_minimum;}
    };

    struct Scene
    {   
        double m_timeout = 0.0; // abort create scene after this time. If 0, no timeout
        double GetTimeout() const {return m_timeout;}
        void SetTimeout(double seconds) {m_timeout=seconds;}
    };

    struct AbortFlags
    {
        struct Motion
        {
            int     m_tolerance = 0;
            int     m_total = 0;
            Point2d m_cursorPos;
            void Clear() {m_total=0; m_cursorPos.x = m_cursorPos.y = 0;}

            void AddMotion(int val) {m_total += val;}
            int GetTotalMotion() {return m_total;}
            void SetCursorPos(Point2d pt) {m_cursorPos=pt;}
            void SetTolerance(int val) {m_tolerance=val;}
            int GetTolerance() {return m_tolerance;}
            Point2d GetCursorPos() {return m_cursorPos;}
        };

        StopEvents  m_stopEvents = StopEvents::ForFullUpdate;
        mutable Motion m_motion;

        void SetTouchCheckStopLimit(bool enabled, uint32_t pixels, uint32_t numberTouches, Point2dCP touches);
        void SetStopEvents(StopEvents stopEvents) {m_stopEvents = stopEvents;}
        StopEvents GetStopEvents() const {return m_stopEvents;}
        Motion& GetMotion() const {return m_motion;}
        bool WantMotionAbort() const {return 0 != m_motion.GetTolerance();}
    };

    double      m_targetFPS = 10.0; // Frames Per second
    Query       m_query;
    Scene       m_scene;
    AbortFlags  m_abortFlags;

public:
    double GetTargetFramesPerSecond() const {return m_targetFPS;}
    void SetTargetFramesPerSecond(double fps) {m_targetFPS = fps;}
    Query& GetQueryR() {return m_query;}
    Query const& GetQuery() const {return m_query;}
    Scene& GetSceneR() {return m_scene;}
    Scene const& GetScene() const {return m_scene;}
    AbortFlags const& GetAbortFlags() const {return m_abortFlags;}
    AbortFlags& GetAbortFlagsR() {return m_abortFlags;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct DynamicUpdatePlan : UpdatePlan
    {
    DynamicUpdatePlan() {m_abortFlags.SetStopEvents(StopEvents::ForQuickUpdate);}
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
enum class DgnFontType {TrueType = 1, Rsc = 2, Shx = 3,};

//! @private
enum class DgnFontStyle {Regular, Bold, Italic, BoldItalic,};

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

//! Values held in line style definition elements; normally not used by clients of this API
//! @ingroup LineStyleManagerModule
enum class LsComponentType
{
    Unknown         = 0,             //!<   Unknown, should never occur
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
    uint32_t GetValue() const { return m_number; }
    LsComponentType GetType() const { return m_type; }
    LsComponentId() { m_type = LsComponentType::Unknown; m_number = 0xFFFFFFFF; }
    bool IsValid() const { return m_number != 0xFFFFFFFF; }
    explicit LsComponentId(LsComponentType type, uint32_t value) : m_type(type), m_number(value) {}

    bool operator<(LsComponentId const&r) const
        {
        if (this->m_type < r.m_type)
            return true;

        if (this->m_type > r.m_type)
            return false;

        return this->m_number < r.m_number;
        }
};

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

/* Values for NotificationManager::OutputMessage */
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

enum class DrawPurpose
{
    NotSpecified = 0,
    CreateScene,
    Hilite,
    Unhilite,
    Dynamics,
    Plot,
    Pick,
    CaptureGeometry,
    GenerateThumbnail,
    Decorate,
    FenceAccept,
    RegionFlood,                 //!< Collect graphics to find closed regions/flood...
    FitView,
    ExportVisibleEdges,
    InterferenceDetection,
    ModelFacet,
    Measure,
    VisibilityCalculation,
    UpdateDynamic,
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
// @bsiclass                                                    Keith.Bentley   12/14
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

//__PUBLISH_SECTION_END__

#define QV_RESERVED_DISPLAYPRIORITY     (32)
#define MAX_HW_DISPLAYPRIORITY          ((1<<23)-QV_RESERVED_DISPLAYPRIORITY)
#define RESERVED_DISPLAYPRIORITY        (1<<19)

// Used for verifying published tests in DgnPlatformTest are using published headers. DO NOT REMOVE.
#define __DGNPLATFORM_NON_PUBLISHED_HEADER__ 1
/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
* Interface to be adopted by a class that wants to supply a copyright message that must
* be display in a viewport.
* @bsiclass                                                     Sam.Wilson      10/15
+===============+===============+===============+===============+===============+======*/
struct CopyrightSupplier
    {
    //! Return the copyright message to display in the specified viewport
    //! @param vp   The viewport that is being displayed
    //! @return a copyright message to display or an empty string to display nothing
    virtual Utf8String _GetCopyrightMessage(DgnViewportR vp) = 0;
    };


/** @endcond */

END_BENTLEY_DGN_NAMESPACE
