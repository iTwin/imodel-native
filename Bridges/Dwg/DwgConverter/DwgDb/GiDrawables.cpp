/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB


#ifdef DWGTOOLKIT_OpenDwg

#define RETURNTRUE                      return;
#define RETURNFALSE                     return;
#define DWGGI_TransparencyMode          OdGiRasterImage::TransparencyMode
#define StubToObjectId(_stub_)          reinterpret_cast<OdDbObjectId*>(nullptr==_stub_ ? nullptr : *_stub_)

ODRX_DEFINE_RTTI_MEMBERS (DwgGiMaterialTexture, OdGiMaterialTexture)
ODRX_DEFINE_RTTI_MEMBERS (DwgGiImageFileTexture, OdGiImageFileTexture)
ODRX_DEFINE_RTTI_MEMBERS (DwgGiGenericTexture, OdGiGenericTexture)
ODRX_DEFINE_RTTI_MEMBERS (DwgGiMarbleTexture, OdGiMarbleTexture)
ODRX_DEFINE_RTTI_MEMBERS (DwgGiWoodTexture, OdGiWoodTexture)

#elif DWGTOOLKIT_RealDwg

ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiVariant, AcGiVariant)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiEdgeData, AcGiEdgeData)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiFaceData, AcGiFaceData)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiVertexData, AcGiVertexData)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiMapper, AcGiMapper)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiMaterialColor, AcGiMaterialColor)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiMaterialTexture, AcGiMaterialTexture)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiMaterialMap, AcGiMaterialMap)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiImageFileTexture, AcGiImageFileTexture)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiGenericTexture, AcGiGenericTexture)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiMarbleTexture, AcGiMarbleTexture)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiWoodTexture, AcGiWoodTexture)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiTextStyle, AcGiTextStyle)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiShadowParameters, AcGiShadowParameters)
ACRX_NO_CONS_DEFINE_MEMBERS (DwgGiLightAttenuation, AcGiLightAttenuation)

#define RETURNTRUE                      return Adesk::kTrue;
#define RETURNFALSE                     return Adesk::kFalse;
#define DWGGI_TransparencyMode          AcGiGeometry::TransparencyMode

#endif  // DWGTOOLKIT_

#define ToDwgTransparencyMode(mode)     static_cast<DwgGiTransparencyMode>(mode)

// some different declarations in OdGi vs AcGi
#define VoidOrBool          DWGDB_SDKNAME(void, Adesk::Boolean)
#define BoolOrCBool         DWGDB_SDKNAME(bool, const Adesk::Boolean)
#define Int32OrInt          DWGDB_SDKNAME(OdInt32, int)
#define Int32OrCInt32       DWGDB_SDKNAME(OdInt32, const Adesk::Int32)
#define Int32OrCUInt32      DWGDB_SDKNAME(OdInt32, const Adesk::UInt32)
#define UInt32OrULongPtr    DWGDB_SDKNAME(OdUInt32, Adesk::ULongPtr)
#define ConstDouble         DWGDB_SDKNAME(double, const double)
#define ConstOrNot          DWGDB_SDKNAME(, const)
#define ConstOverride       DWGDB_SDKNAME(override, const override)
#define OverrideConst       DWGDB_SDKNAME(const override, override)
#define StubPOrObjectId     DWGDB_SDKNAME(OdDbStub*, AcDbObjectId)
#define StubPOrCObjectId    DWGDB_SDKNAME(OdDbStub*, const AcDbObjectId)
#define StubPOrCObjectIdR   DWGDB_SDKNAME(OdDbStub*, const AcDbObjectId&)


DwgGiVariant::ValueType DwgGiVariant::GetType() const { return static_cast<DwgGiVariant::ValueType>(T_Super::type()); }
void            DwgGiVariant::Set (bool v) { T_Super::set(v); }
void            DwgGiVariant::Set (DwgDbInt32 v) { T_Super::set(v); }
void            DwgGiVariant::Set (double v) { T_Super::set(v); }
void            DwgGiVariant::Set (WCharCP v) { DWGDB_CALLSDKMETHOD(T_Super::set(reinterpret_cast<const OdChar*>(v)), T_Super::set(v)); } 
void            DwgGiVariant::Set (DwgCmColorCR v) { DWGDB_CALLSDKMETHOD(T_Super::set(v.GetEntityColor()), T_Super::set(v)); }
bool            DwgGiVariant::AsBoolean () const { return T_Super::asBoolean(); }
int             DwgGiVariant::AsInteger () const { return T_Super::asInt(); }
double          DwgGiVariant::AsDouble () const { return T_Super::asDouble(); }
DwgString       DwgGiVariant::AsString () const { return static_cast<DwgString>(T_Super::asString()); }
float           DwgGiVariant::AsFloat ()  const { return T_Super::asFloat(); }
char            DwgGiVariant::AsChar () const { return T_Super::asChar(); }
unsigned char   DwgGiVariant::AsUChar () const { return T_Super::asUchar(); }
short           DwgGiVariant::AsShort () const { return T_Super::asShort(); }
unsigned short  DwgGiVariant::AsUShort () const { return T_Super::asUshort(); }
unsigned int    DwgGiVariant::AsUInt () const { return T_Super::asUint(); }
int32_t         DwgGiVariant::AsLong () const { return T_Super::asLong(); }
uint32_t        DwgGiVariant::AsULong () const { return T_Super::asUlong(); }
DwgCmColor      DwgGiVariant::AsColor () const { return static_cast<DwgCmColor>(T_Super::asColor()); }
DwgGiVariant::EnumValueType   DwgGiVariant::AsEnum () const { return T_Super::asEnum(); }
DwgGiVariantCP  DwgGiVariant::GetElem (WCharCP key) const { return static_cast<DwgGiVariantCP>(T_Super::getElem(key)); }
void            DwgGiVariant::SetElem (WCharCP key, DwgGiVariantCR v) { T_Super::setElem(key,v); }
void            DwgGiVariant::DeleteElem (WCharCP key) { T_Super::deleteElem(key); }
uint32_t        DwgGiVariant::GetElemCount () const { return T_Super::getElemCount(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgGiVariant::GetElem (WCharCP key, DwgGiVariantR v) const
    {
#if DWGTOOLKIT_OpenDwg
    return ToDwgDbStatus(T_Super::getElem(key, v));
#elif DWGTOOLKIT_RealDwg
    return T_Super::getElem(key,v) ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgGiVariant::GetElemAt (int i, DwgStringR s, DwgGiVariantR v) const
    { 
#if DWGTOOLKIT_OpenDwg
    return  T_Super::getElemAt(i, s, v) ? DwgDbStatus::Success : DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    ACHAR*  acChars = nullptr;
    Acad::ErrorStatus es = T_Super::getElemAt (i, acChars, v);
    if (nullptr != acChars)
        {
        s.assign (acChars);
        ::acutDelString (acChars);
        }
    return  ToDwgDbStatus(es);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiVariantCP  DwgGiVariant::GetElemAt (int i, DwgStringR s) const
    { 
#if DWGTOOLKIT_OpenDwg
    return  static_cast<DwgGiVariantCP>(T_Super::getElemAt(i, s));
#elif DWGTOOLKIT_RealDwg
    ACHAR*  acChars = nullptr;
    AcGiVariant*    acElem = T_Super::getElemAt(i, acChars);
    if (nullptr != acChars)
        {
        s.assign (acChars);
        ::acutDelString (acChars);
        }
    return  static_cast<DwgGiVariantCP>(acElem);
#endif
    }
DwgGiVariantR   DwgGiVariant::operator = (DWGGI_TypeCR(Variant) v) { T_Super::operator=(v); return *this; }
bool            DwgGiVariant::operator == (DWGGI_TypeCR(Variant) v) const { return v == *this; }
DwgGiVariant::DwgGiVariant (bool v) { T_Super::set(v); }
DwgGiVariant::DwgGiVariant (DwgDbInt32 v) { T_Super::set(v); }
DwgGiVariant::DwgGiVariant (double v) { T_Super::set(v); }
DwgGiVariant::DwgGiVariant (DwgCmColorCR v) { DWGDB_CALLSDKMETHOD(T_Super::set(v.GetEntityColor()),T_Super::set(v)); }
DwgGiVariant::DwgGiVariant (WCharCP v) { DWGDB_CALLSDKMETHOD(T_Super::set(reinterpret_cast<const OdChar*>(v)), T_Super::set(v)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiVariantCR  DwgGiVisualStyle::GetTrait (DwgGiVisualStyleProperties::Property prop, DwgGiVisualStyleOperations::Operation* op) const
    {
#if DWGTOOLKIT_OpenDwg
     static DwgGiVariant s_variantCopy(false);
     OdGiVariantPtr odVar = m_toolkitVisualStyle.trait (CASTFROMDwgGiVisProp(prop), CASTFROMDwgGiVisOpP(op));
     if (odVar.isNull())
        BeAssert (false && "OdGiVisualStyle::trait() returned a nullptr!");
    else
        s_variantCopy = *static_cast<DwgGiVariantP>(odVar.get());
    return  s_variantCopy;
#elif DWGTOOLKIT_RealDwg
     return static_cast<DwgGiVariantCR>(m_toolkitVisualStyle.trait(CASTFROMDwgGiVisProp(prop), CASTFROMDwgGiVisOpP(op)));
#endif    
    }
bool            DwgGiVisualStyle::GetTraitFlag (DwgGiVisualStyleProperties::Property prop, DwgDbUInt32 flags) const { return m_toolkitVisualStyle.traitFlag(CASTFROMDwgGiVisProp(prop), flags); }
DwgGiVisualStyle::RenderType    DwgGiVisualStyle::GetType () const { return static_cast<DwgGiVisualStyle::RenderType>(m_toolkitVisualStyle.type()); }
DwgGiVisualStyleOperations::Operation   DwgGiVisualStyle::GetOperation (DwgGiVisualStyleProperties::Property prop) const { return CASTTODwgGiVisOp(m_toolkitVisualStyle.operation(CASTFROMDwgGiVisProp(prop))); }

uint8_t const*          DwgGiEdgeData::GetVisibility () const { return T_Super::visibility(); }
int16_t const*          DwgGiEdgeData::GetColors () const { return reinterpret_cast<int16_t const*>(T_Super::colors()); }
DwgCmEntityColorCP      DwgGiEdgeData::GetTrueColors () const { return static_cast<DwgCmEntityColorCP>(T_Super::trueColors()); }
DwgDbObjectIdCP         DwgGiEdgeData::GetLayers () const { return static_cast<DwgDbObjectIdCP>(DWGDB_CALLSDKMETHOD(StubToObjectId(T_Super::layerIds()),T_Super::layerIds())); }
DwgDbObjectIdCP         DwgGiEdgeData::GetLinetypes () const { return static_cast<DwgDbObjectIdCP>(DWGDB_CALLSDKMETHOD(StubToObjectId(T_Super::linetypeIds()),T_Super::linetypeIds())); }
DwgDbLongPtr const*     DwgGiEdgeData::GetSelectionMarkers () const { return static_cast<DwgDbLongPtr const*>(T_Super::selectionMarkers()); }


int16_t const*          DwgGiFaceData::GetColors () const { return reinterpret_cast<int16_t const*>(T_Super::colors()); }
DwgCmEntityColorCP      DwgGiFaceData::GetTrueColors () const { return static_cast<DwgCmEntityColorCP>(T_Super::trueColors()); }
DwgTransparencyCP       DwgGiFaceData::GetTransparencies () const { return static_cast<DwgTransparencyCP>(T_Super::transparency()); }
DwgDbObjectIdCP         DwgGiFaceData::GetLayers () const { return static_cast<DwgDbObjectIdCP>(DWGDB_CALLSDKMETHOD(StubToObjectId(T_Super::layerIds()),T_Super::layerIds())); }
DwgDbObjectIdCP         DwgGiFaceData::GetMaterials () const { return static_cast<DwgDbObjectIdCP>(DWGDB_CALLSDKMETHOD(StubToObjectId(T_Super::materials()),T_Super::materials())); }
DwgGiMapper const*      DwgGiFaceData::GetMappers () const { return static_cast<DwgGiMapper const*>(T_Super::mappers()); }
DVec3dCP                DwgGiFaceData::GetNormals () const { return reinterpret_cast<DVec3dCP>(T_Super::normals()); }


DwgCmEntityColorCP      DwgGiVertexData::GetTrueColors () const { return static_cast<DwgCmEntityColorCP>(T_Super::trueColors()); }
DPoint3dCP              DwgGiVertexData::GetMappingCoords (MappingChannel c) const { return reinterpret_cast<DPoint3dCP>(T_Super::mappingCoords(static_cast<MapChannel>(c))); }
DVec3dCP                DwgGiVertexData::GetNormals () const { return reinterpret_cast<DVec3dCP>(T_Super::normals()); }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiContext : public DWGGI_EXTENDCLASS(Context)
{
private:
    DwgDbDatabaseP  m_database;

public:

#ifdef DWGTOOLKIT_OpenDwg
    ODRX_DECLARE_MEMBERS (DwgGiContext);
    DWGRX_DEFINE_SMARTPTR_BASE ()

virtual OdGiDrawablePtr openDrawable (OdDbStub* drawableId) override
    {
    if (nullptr != drawableId)
        {
        OdDbObjectId    id(drawableId);
        return id.openObject ();
        }
    return  OdGiDrawablePtr();
    }

#elif DWGTOOLKIT_RealDwg

virtual Adesk::Boolean  isPsOut () const override { return Adesk::kFalse; }
virtual Adesk::Boolean  isPlotGeneration () const override { return Adesk::kFalse; }
virtual bool            isBoundaryClipping () const override { return false; }

#endif  // DWGTOOLKIT_
virtual DWGDB_TypeP(Database)   database () const override { return DWGDB_DOWNWARDCAST(Database*)(m_database); }

// constructors and our own methods
DwgGiContext () : m_database(nullptr) {}
explicit DwgGiContext (DwgDbDatabaseP dwg) : m_database(dwg) {}

void    SetDatabase (DwgDbDatabaseP dwg) { m_database = dwg; }
};  // DwgGiContext
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiContext)
#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgGiContext,OdGiContext,RXIMPL_CONSTR)
#endif

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiSubEntityTraits : public DWGGI_EXTENDCLASS(SubEntityTraits)
{
private:
    DWGDB_Type(::PlotStyleNameType) m_plotstyleNametype;
    DWGDB_Type(ObjectId)            m_plotstyleNameId;
    DWGDB_Type(ObjectId)            m_visualstyleId;
    DWGGI_TypeCP(Mapper)            m_mapper;
    IDwgDrawParametersP             m_drawParameters;
    DwgGiFillPtr                    m_fillHolder;

public:
DwgGiSubEntityTraits () : m_plotstyleNameId(), m_visualstyleId(), m_mapper(), m_drawParameters(nullptr) {}

DwgGiSubEntityTraits (IDwgDrawParametersP drawParams) : m_plotstyleNameId(), m_visualstyleId(), m_mapper()
    {
    m_drawParameters = drawParams;
    }

#ifdef DWGTOOLKIT_OpenDwg
    DWGRX_DECLARE_MEMBERS (DwgGiSubEntityTraits)
    DWGRX_DEFINE_SMARTPTR_BASE ()

virtual OdDbStub*               layer () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetLayer() : nullptr; }
virtual OdDbStub*               lineType () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetLineType() : nullptr; }
virtual OdDbStub*               material () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetMaterial() : nullptr; }
virtual OdDb::PlotStyleNameType plotStyleNameType () const { return m_plotstyleNametype; }
virtual OdDbStub*               plotStyleNameId() const { return m_plotstyleNameId; }
virtual void                    setVisualStyle (const OdDbStub* id) override { m_visualstyleId = (OdDbObjectId)const_cast<OdDbStub*>(id); }

#elif DWGTOOLKIT_RealDwg

virtual AcDbObjectId            layerId () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetLayer() : AcDbObjectId::kNull; }
virtual AcDbObjectId            lineTypeId () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetLineType() : AcDbObjectId::kNull; }
virtual AcDbObjectId            materialId () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetMaterial() : AcDbObjectId::kNull; }
virtual AcDbObjectId            getPlotStyleNameId() const { return m_plotstyleNameId; }
virtual void                    setVisualStyle (const AcDbObjectId id) override { m_visualstyleId = id; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual const AcGiFill*         fill () const override
    {
    if (nullptr != m_drawParameters)
        {
        DwgGiFillCP fill = m_drawParameters->_GetFill ();
        if (nullptr != fill)
            return  fill->GetAcGiFill ();
        }
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    setFill (const AcGiFill* acFill) override
    {
    if (nullptr != m_drawParameters && m_fillHolder.IsValid())
        {
        m_fillHolder->SetAcGiFill (acFill);
        m_drawParameters->_SetFill (m_fillHolder.get());
        }
    }
#endif  // DWGTOOLKIT_

virtual DwgDbUInt16                 color () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetColor().colorIndex() : 7; }
virtual DWGCM_Type(EntityColor)     trueColor () const override { return nullptr!=m_drawParameters ? static_cast<DWGCM_TypeCR(EntityColor)>(m_drawParameters->_GetColor()) : DWGCM_Type(EntityColor)(); }
virtual DWGGI_Type(FillType)        fillType () const override { return static_cast<DWGGI_Type(FillType)>(nullptr!=m_drawParameters ? m_drawParameters->_GetFillType() : DwgGiFillType::Always); }
virtual DWGDB_Type(::LineWeight)    lineWeight () const override { return DWGDB_CASTTOENUM_DB(LineWeight)(nullptr!=m_drawParameters ? m_drawParameters->_GetLineWeight() : DwgDbLineWeight::WeightByDefault); }
virtual double                      lineTypeScale () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetLineTypeScale() : 1.0; }
virtual double                      thickness () const override { return nullptr!=m_drawParameters ? m_drawParameters->_GetThickness() : 0.0; }
virtual StubPOrObjectId             visualStyle () const override { return m_visualstyleId; }
virtual DWGGI_TypeCP(Mapper)        mapper () const override { return m_mapper; }
virtual DWGCM_Type(Transparency)    transparency () const override { return nullptr!=m_drawParameters ? DWGCM_Type(Transparency)(m_drawParameters->_GetTransparency()) : DWGCM_Type(Transparency)(1.0); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void setColor (ConstOrNot DwgDbUInt16 index) override
    {
    if (nullptr != m_drawParameters)
        {
        DWGCM_Type(EntityColor) color;
        color.setColorIndex (index);
        m_drawParameters->_SetColor (color);
        }
    }

virtual void setTrueColor (DWGCM_TypeCR(EntityColor) color) override {if (nullptr!=m_drawParameters) m_drawParameters->_SetColor(color); }
virtual void setLayer (StubPOrCObjectId layerId) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetLayer(layerId); }
virtual void setLineType (StubPOrCObjectId linetypeId) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetLineType(linetypeId); }
virtual void setSelectionMarker (DWGDB_SDKNAME(OdGsMarker,const Adesk::LongPtr) markerId) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetSelectionMarker(markerId); }
virtual void setFillType (ConstOrNot DWGGI_Type(FillType) filltype) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetFillType(static_cast<DwgGiFillType>(filltype)); }
virtual void setLineWeight (ConstOrNot DWGDB_Type(::LineWeight) w) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetLineWeight(DWGDB_UPWARDCAST(LineWeight)(w)); }
virtual void setLineTypeScale (double scale = 1.0) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetLineTypeScale(scale); }
virtual void setThickness (double thickness) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetThickness(thickness); }
virtual void setPlotStyleName (DWGDB_Type(::PlotStyleNameType) type, StubPOrCObjectIdR id = DWGDB_Type(ObjectId)::kNull) override { m_plotstyleNametype=type; m_plotstyleNameId=id; }
virtual void setTransparency (DWGCM_TypeCR(Transparency) transparency) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetTransparency(transparency); }
virtual void setMaterial (StubPOrCObjectId id) override { if (nullptr!=m_drawParameters) m_drawParameters->_SetMaterial(id); }
virtual void setMapper (DWGGI_TypeCP(Mapper) mapper) override { m_mapper = mapper; }
};  // DwgGiSubEntityTraits
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiSubEntityTraits)
#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgGiSubEntityTraits,OdGiSubEntityTraits,RXIMPL_CONSTR)
#endif


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiWorldGeometry : public DWGGI_EXTENDCLASS(WorldGeometry)
    {
    IDwgDrawGeometryP               m_drawGeometry;
    DwgGiContextP                   m_context;
    DwgGiSubEntityTraitsP           m_subEntityTraits;
    mutable bool                    m_isInBlockRecord;

    void Initialize (IDwgDrawGeometryP geom, DwgGiContextP context, DwgGiSubEntityTraitsP traits)
        {
        m_drawGeometry = geom;
        m_context = context;
        m_subEntityTraits = traits;
        m_isInBlockRecord = false;
        }

public:
    DWGRX_DECLARE_MEMBERS (DwgGiWorldGeometry)
    DWGRX_DEFINE_SMARTPTR_BASE ()

    DwgGiWorldGeometry ()
        {
        this->Initialize (nullptr, nullptr, nullptr);
        }

    explicit DwgGiWorldGeometry (IDwgDrawGeometryR geom, DwgGiContextR context, DwgGiSubEntityTraitsR traits)
        {
        this->Initialize (&geom, &context, &traits);
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool  pushModelTransform (DWGGE_TypeCR(Vector3d) normal) override
    {
    DWGGE_Type(Matrix3d)    matrix;
    matrix.setToPlaneToWorld (normal);

    return this->pushModelTransform (matrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool  popModelTransform () override
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_PopModelTransform ();

    RETURNTRUE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      pushModelTransform (DWGGE_TypeCR(Matrix3d) matrix) override
    {
    if (nullptr != m_drawGeometry)
        {
        Transform   trans;
        Util::GetTransform (trans, matrix);
        m_drawGeometry->_PushModelTransform (trans);
        }
    
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      circle (DWGGE_TypeCR(Point3d) center, ConstDouble radius, DWGGE_TypeCR(Vector3d) normal) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Circle (Util::DPoint3dFrom(center), radius, Util::DVec3dFrom(normal));
    RETURNFALSE
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      circle (DWGGE_TypeCR(Point3d) p1, DWGGE_TypeCR(Point3d) p2, DWGGE_TypeCR(Point3d) p3) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Circle (Util::DPoint3dFrom(p1), Util::DPoint3dFrom(p2), Util::DPoint3dFrom(p3));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      circularArc
(
DWGGE_TypeCR(Point3d)       center,
ConstDouble                 radius,
DWGGE_TypeCR(Vector3d)      normal,
DWGGE_TypeCR(Vector3d)      startVector,
ConstDouble                 sweepAngle,
ConstOrNot DWGGI_Type(ArcType)   arcType
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_CircularArc (Util::DPoint3dFrom(center), radius, Util::DVec3dFrom(normal), Util::DVec3dFrom(startVector), sweepAngle, static_cast<DwgGiArcType>(arcType));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      circularArc
(
DWGGE_TypeCR(Point3d)       start,
DWGGE_TypeCR(Point3d)       point,
DWGGE_TypeCR(Point3d)       end,
ConstOrNot DWGGI_Type(ArcType)   arcType
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_CircularArc (Util::DPoint3dFrom(start), Util::DPoint3dFrom(point), Util::DPoint3dFrom(end), static_cast<DwgGiArcType>(arcType));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      polyline (Int32OrCUInt32 nPoints, DWGGE_TypeCP(Point3d) points, DWGGE_TypeCP(Vector3d) normal, DwgDbLongPtr subEntMarker) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        {
        DPoint3dCP      p = Util::DPoint3dCPFrom (points);
        DVec3dCP        n = reinterpret_cast <DVec3dCP> (normal);
        m_drawGeometry->_Polyline (nPoints, p, n, subEntMarker);
        }
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      polygon (Int32OrCUInt32 nPoints, DWGGE_TypeCP(Point3d) points) ConstOrNot
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Polygon (nPoints, reinterpret_cast<DPoint3dCP>(points));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      mesh
(
Int32OrCUInt32              rows,
Int32OrCUInt32              columns,
DWGGE_TypeCP(Point3d)       points,
DWGGI_TypeCP(EdgeData)      edgeData,
DWGGI_TypeCP(FaceData)      faceData,
DWGGI_TypeCP(VertexData)    vertexData
#ifdef DWGTOOLKIT_RealDwg
, const bool                bAutoGenerateNormals
#endif
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        {
        DPoint3dCP          p = Util::DPoint3dCPFrom (points);
        DwgGiEdgeDataCP     e = static_cast <DwgGiEdgeDataCP> (edgeData);
        DwgGiFaceDataCP     f = static_cast <DwgGiFaceDataCP> (faceData);
        DwgGiVertexDataCP   v = static_cast <DwgGiVertexDataCP> (vertexData);

        m_drawGeometry->_Mesh (rows, columns, p, e, f, v);
        }
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      shell
(
Int32OrCUInt32              nPoints,
DWGGE_TypeCP(Point3d)       points,
Int32OrCUInt32              faceListSize,
DwgDbInt32CP                faceList,
DWGGI_TypeCP(EdgeData)      edgeData,
DWGGI_TypeCP(FaceData)      faceData,
DWGGI_TypeCP(VertexData)    vertexData
#ifdef DWGTOOLKIT_RealDwg
, const struct resbuf*      resBuf,
const bool                  bAutoGenerateNormals = true
#endif
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        {
        uint32_t            nList = static_cast <uint32_t> (faceListSize);
        int32_t const*      list = reinterpret_cast <int32_t const*> (faceList);
        DPoint3dCP          p = Util::DPoint3dCPFrom (points);
        DwgGiEdgeDataCP     e = static_cast <DwgGiEdgeDataCP> (edgeData);
        DwgGiFaceDataCP     f = static_cast <DwgGiFaceDataCP> (faceData);
        DwgGiVertexDataCP   v = static_cast <DwgGiVertexDataCP> (vertexData);
        DwgResBufCP         x = nullptr;

#ifdef DWGTOOLKIT_RealDwg
        x = static_cast<DwgResBufCP> (resBuf);
#endif
    
        m_drawGeometry->_Shell (nPoints, p, nList, list, e, f, v, x, false);
        }

    RETURNFALSE
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      text
(
DWGGE_TypeCR(Point3d)   position,
DWGGE_TypeCR(Vector3d)  normal,
DWGGE_TypeCR(Vector3d)  direction,
ConstDouble             height,
ConstDouble             width,
ConstDouble             oblique,
const DWGDB_SDKNAME(OdString&, ACHAR*) msg
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Text (Util::DPoint3dFrom(position), Util::DVec3dFrom(normal), Util::DVec3dFrom(direction), height, width, oblique, DwgString(msg));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      text
(
DWGGE_TypeCR(Point3d)   position,
DWGGE_TypeCR(Vector3d)  normal,
DWGGE_TypeCR(Vector3d)  direction,
DwgDbCharCP             msg,
Int32OrCInt32           nChars,
BoolOrCBool             raw,
#ifdef DWGTOOLKIT_OpenDwg
const OdGiTextStyle*    textStyle
) ConstOverride
    {
    DwgGiTextStyle      style;
    if (nullptr != textStyle)
        style = DwgGiTextStyle (*textStyle);

    bool    isRaw = raw;

    // input string may not be null terminated
    OdString    string(msg, nChars);

#elif DWGTOOLKIT_RealDwg
const AcGiTextStyle&    textStyle
) const override
    {
    DwgGiTextStyle  style(textStyle);

    bool    isRaw = Adesk::kTrue == raw;

    // input string may not be null terminated
    DwgString   string(msg);
    string = string.substr (nChars);
#endif

    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Text (Util::DPoint3dFrom(position), Util::DVec3dFrom(normal), Util::DVec3dFrom(direction), string, isRaw, style);
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      xline (DWGGE_TypeCR(Point3d) point1, DWGGE_TypeCR(Point3d) point2) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Xline (Util::DPoint3dFrom(point1), Util::DPoint3dFrom(point2));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      ray (DWGGE_TypeCR(Point3d) origin, DWGGE_TypeCR(Point3d) point) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_Ray (Util::DPoint3dFrom(origin), Util::DPoint3dFrom(point));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      draw (DWGDB_SDKNAME(const OdGiDrawable,AcGiDrawable)* drawable) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        {
        DwgGiDrawable   dr(drawable);
        m_drawGeometry->_Draw (dr);
        }
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      image
(
DWGGI_TypeCR(ImageBGRA32)   source,
DWGGE_TypeCR(Point3d)       position,
DWGGE_TypeCR(Vector3d)      u,      //orientation and magnitude of width
DWGGE_TypeCR(Vector3d)      v,      //orientation and magnitude of height
DWGGI_TransparencyMode      mode
) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        {
        DwgGiImageBGRA32    img(source);
        m_drawGeometry->_Image(img, Util::DPoint3dFrom(position), Util::DVec3dFrom(u), Util::DVec3dFrom(v), ToDwgTransparencyMode(mode));
        }
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      rowOfDots (Int32OrInt count, DWGGE_TypeCR(Point3d) start, DWGGE_TypeCR(Vector3d) step) ConstOverride
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_RowOfDots (count, Util::DPoint3dFrom(start), Util::DVec3dFrom(step));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      pushClipBoundary (DWGGI_TypeP(ClipBoundary) boundary) override
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_PushClipBoundary (static_cast<DwgGiClipBoundaryP>(boundary));
    RETURNFALSE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            popClipBoundary () override
    {
    if (nullptr != m_drawGeometry)
        m_drawGeometry->_PopClipBoundary ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
VoidOrBool      worldLine (const DWGGE_Type(Point3d) pointsIn[2]) override
    {
    if (nullptr != m_drawGeometry)
        {
        DPoint3d    points[2];
        Util::GetDPoint3d (points, pointsIn, 2);
        m_drawGeometry->_WorldLine (points);
        }
    RETURNFALSE
    }

// toolkit speccific methods
#ifdef DWGTOOLKIT_OpenDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
OdGeMatrix3d    getModelToWorldTransform () const override
    {
    OdGeMatrix3d    matrix;
    if (nullptr == m_drawGeometry)
        {
        matrix.setToIdentity();
        }
    else
        {
        Transform   trans = Transform::FromIdentity ();
        m_drawGeometry->_GetModelTransform (trans);

        Util::GetGeMatrix (matrix, trans);
        }

    return  matrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
OdGeMatrix3d    getWorldToModelTransform () const override
    {
    OdGeMatrix3d    matrix;

    if (nullptr == m_drawGeometry)
        {
        matrix.setToIdentity();
        }
    else
        {
        Transform   trans = Transform::FromIdentity ();
        m_drawGeometry->_GetModelTransform (trans);

        Util::GetGeMatrix (matrix, trans);
        matrix.invert ();
        }

    return  matrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            nurbs (const OdGeNurbCurve3d& nurbsCurve) override
    {
    if (nullptr != m_drawGeometry)
        {
        MSBsplineCurve  curve;
        if (DwgDbStatus::Success == Util::GetMSBsplineCurve(curve, nurbsCurve))
            m_drawGeometry->_Curve (curve);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ellipArc (const OdGeEllipArc3d& ellipArc, const OdGePoint3d* endPointsOverrides, OdGiArcType arcType) override
    {
    if (nullptr == m_drawGeometry)
        return;

    DEllipse3d  ellipse;

    if (nullptr == endPointsOverrides)
        {
        ellipse = Util::DEllipse3dFrom (ellipArc);
        }
    else
        {
        DPoint3dCP  endPoints = Util::DPoint3dCPFrom (endPointsOverrides);
        ellipse = DEllipse3d::FromArcCenterStartEnd (Util::DPoint3dFrom(ellipArc.center()), endPoints[0], endPoints[1]);
        }

    m_drawGeometry->_Ellipse (ellipse, static_cast<DwgGiArcType>(arcType));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            pline (const OdGiPolyline& lwBuf, OdUInt32 fromIndex, OdUInt32 numSegs) override
    {
    if (nullptr != m_drawGeometry)
        {
        OdDbPolylinePtr     odPline = lwBuf.getDbPolyline ();
        DwgDbPolylinePtr    pline = odPline;
        if (pline.isNull())
            {
            pline = OdDbPolyline::createObject ();
            if (pline.isNull())
                return;
            pline->SetFromGiPolyline (lwBuf);
            }

        m_drawGeometry->_Pline (*pline.get(), fromIndex, numSegs);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            edge (const OdGiEdge2dArray& edges) override
    {
    if (nullptr != m_drawGeometry)
        {
        OdArray<OdGeCurve2d*>   curves;
        for (auto e : edges)
            curves.append (e);

        CurveVectorPtr  curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        if (curveVector.IsValid() && DwgDbStatus::Success == Util::GetCurveVector(*curveVector.get(), curves))
            m_drawGeometry->_Edge (*curveVector.get());
        }
    }

void            setExtents (const OdGePoint3d* extents) override { /* do nothing */ }

#elif DWGTOOLKIT_RealDwg

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            getModelToWorldTransform (AcGeMatrix3d& matrix) const override
    {
    if (nullptr == m_drawGeometry)
        {
        matrix.setToIdentity();
        }
    else
        {
        Transform   trans = Transform::FromIdentity ();
        m_drawGeometry->_GetModelTransform (trans);

        Util::GetGeMatrix (matrix, trans);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            getWorldToModelTransform (AcGeMatrix3d& matrix) const override
    {
    if (nullptr == m_drawGeometry)
        {
        matrix.setToIdentity();
        }
    else
        {
        Transform   trans = Transform::FromIdentity ();
        m_drawGeometry->_GetModelTransform (trans);

        Util::GetGeMatrix (matrix, trans);
        matrix.invert ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
AcGeMatrix3d    pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset) override
    {
    // not needed yet
    return  AcGeMatrix3d();
    }
AcGeMatrix3d    pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset) override
    {
    // not needed yet
    return  AcGeMatrix3d();
    }
AcGeMatrix3d    pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents) override
    {
    // not needed yet
    return  AcGeMatrix3d();
    }
AcGeMatrix3d    pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents) override
    {
    // not needed yet
    return  AcGeMatrix3d();
    }
AcGeMatrix3d    pushOrientationTransform (AcGiOrientationTransformBehavior behavior) override
    {
    // not needed yet
    return  AcGeMatrix3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  pline (const AcDbPolyline& lwBuf, Adesk::UInt32 fromIndex = 0, Adesk::UInt32 numSegs = 0) const override
    {
    if (nullptr != m_drawGeometry)
        {
        DwgDbPolylineCP     pLw = DwgDbPolyline::Cast (&lwBuf);
        if (nullptr != pLw)
            m_drawGeometry->_Pline (*pLw, fromIndex, numSegs);
        else
            BeAssert (false && L"failed casting AcDbPolyline to DwgDbPolyline!");
        }
    return  Adesk::kFalse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  polyline (const AcGiPolyline& polyline) const override
    {
    if (nullptr != m_drawGeometry)
        {
        const Adesk::LongPtr*   subEntMarkerList = polyline.subEntMarkerList ();
        Adesk::LongPtr          subEntityMarker = -1;

        if (nullptr != subEntMarkerList)
            subEntityMarker = subEntMarkerList[0];

        size_t          s = polyline.points ();
        DPoint3dCP      p = Util::DPoint3dCPFrom (polyline.vertexList());
        DVec3dCP        n = reinterpret_cast <DVec3dCP> (polyline.normal());

        m_drawGeometry->_Polyline (s, p, n, subEntityMarker);
        }

    return  Adesk::kTrue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  polyPolyline (Adesk::UInt32 numPolylines, const AcGiPolyline* polylines) const override
    {
    if (nullptr == polylines)
        return Adesk::kFalse;

    for (Adesk::UInt32 i = 0; i < numPolylines; i++)
        this->polyline (polylines[i]);

    return  Adesk::kFalse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  polyPolygon
(
const Adesk::UInt32     numPolygonIndices,
const Adesk::UInt32*    numPolygonPositions,
const AcGePoint3d*      polygonPositions,
const Adesk::UInt32*    numPolygonPoints,
const AcGePoint3d*      polygonPoints,
const AcCmEntityColor*  outlineColors = nullptr,
const AcGiLineType*     outlineTypes = nullptr,
const AcCmEntityColor*  fillColors = nullptr,
const AcCmTransparency* fillOpacities = nullptr
) const override
    {
    if (nullptr == m_drawGeometry || nullptr == numPolygonPositions || nullptr == polygonPositions || nullptr == numPolygonPoints || nullptr == polygonPoints)
        return Adesk::kFalse;

    DwgGiSubEntityTraits    savedSubEntityTraits = *m_subEntityTraits;

    if (nullptr != fillColors)
        m_subEntityTraits->setFillType (kAcGiFillAlways);

    for (Adesk::UInt32 i = 0; i < numPolygonIndices; i++)
        {
        if (nullptr != fillColors)
            m_subEntityTraits->setTrueColor (fillColors[i]);
        else if (nullptr != outlineColors)
            m_subEntityTraits->setTrueColor (outlineColors[i]);

        // NEEDSWORK - map AcGiLineType to existing linetype or DGN line code?

        if (nullptr != fillOpacities)
            m_subEntityTraits->setTransparency (fillOpacities[i]);

        // create numPolygonPositions[i] identical polygons:
        for (Adesk::UInt32 j = 0; j < numPolygonPositions[i]; j++)
            m_drawGeometry->_Polygon (numPolygonPoints[i], reinterpret_cast<DPoint3dCP>(&polygonPoints[i]));
        }

    *m_subEntityTraits = savedSubEntityTraits;
    
    return  Adesk::kFalse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  ellipticalArc
(
const AcGePoint3d&  center,
const AcGeVector3d& normal,
double              majorAxisLength,
double              minorAxisLength,
double              startDegreeInRads,
double              endDegreeInRads,
double              tiltDegreeInRads,
AcGiArcType         arcType = kAcGiArcSimple
) const override
    {
    if (nullptr == m_drawGeometry)
        return  Adesk::kTrue;

    DVec3d          zAxis = Util::DVec3dFrom (normal);

    // apply the arbitrary axis algorithm:
    AcGeMatrix3d    toEllipse;
    toEllipse.setToPlaneToWorld (normal);

    RotMatrix       matrix;
    Util::GetRotMatrix (matrix, toEllipse);

    DVec3d  xAxis, yAxis;
    matrix.GetColumns (xAxis, yAxis, zAxis);

    // rotate the ellipse about the zAxis:
    DVec3d  majorAxis = DVec3d::FromSumOf (xAxis,  cos(tiltDegreeInRads), yAxis, sin (tiltDegreeInRads));
    DVec3d  minorAxis = DVec3d::FromSumOf (xAxis, -sin(tiltDegreeInRads), yAxis, cos (tiltDegreeInRads));

    DPoint3d        origin = Util::DPoint3dFrom (center);

    double          sweptAngle = endDegreeInRads - startDegreeInRads;
    if (!Angle::IsFullCircle(sweptAngle))
        sweptAngle = Angle::AdjustToSweep (sweptAngle, 0, msGeomConst_2pi);
    
    DEllipse3d      ellipse;
    ellipse.InitFromScaledVectors (origin, majorAxis, minorAxis, majorAxisLength, minorAxisLength, startDegreeInRads, sweptAngle);

    m_drawGeometry->_Ellipse (ellipse, static_cast<DwgGiArcType>(arcType));
    
    return  Adesk::kFalse;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Adesk::Boolean  edge (const AcArray<AcGeCurve2d*>& edges) const override
    {
    if (nullptr != m_drawGeometry)
        {
        CurveVectorPtr  curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        if (curveVector.IsValid() && DwgDbStatus::Success == Util::GetCurveVector(*curveVector.get(), edges))
            m_drawGeometry->_Edge (*curveVector.get());
        }
    return  Adesk::kFalse;
    }

void            setExtents (AcGePoint3d* extents) const override { /* do nothing */ }

#endif  // DWGTOOLKIT_
};  // DwgGiWorldGeometry
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiWorldGeometry)
DWGRX_CONS_DEFINE_MEMBERS (DwgGiWorldGeometry, DWGGI_Type(WorldGeometry))
    

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiViewportGeometry : public DWGGI_EXTENDCLASS(ViewportGeometry)
    {
private:
    DwgGiWorldGeometry      m_worldGeometry;

public:
    DWGRX_DECLARE_MEMBERS (DwgGiViewportGeometry)
    DWGRX_DEFINE_SMARTPTR_BASE ()

    DwgGiViewportGeometry () : m_worldGeometry() {}
    explicit DwgGiViewportGeometry (IDwgDrawGeometryR geom, DwgGiContextR context, DwgGiSubEntityTraitsR traits) : m_worldGeometry(geom, context, traits) { }

// methods shared in both OpenDWG and RealDWG:
VoidOrBool  pushModelTransform (DWGGE_TypeCR(Vector3d) normal) override { return m_worldGeometry.pushModelTransform(normal); }
VoidOrBool  popModelTransform () override { return  m_worldGeometry.popModelTransform(); }
VoidOrBool  pushModelTransform (DWGGE_TypeCR(Matrix3d) matrix) override { return m_worldGeometry.pushModelTransform(matrix); }
VoidOrBool  circle (DWGGE_TypeCR(Point3d) center, ConstDouble radius, DWGGE_TypeCR(Vector3d) normal) ConstOverride { return m_worldGeometry.circle(center, radius, normal); }
VoidOrBool  circle (DWGGE_TypeCR(Point3d) p1, DWGGE_TypeCR(Point3d) p2, DWGGE_TypeCR(Point3d) p3) ConstOverride { return m_worldGeometry.circle(p1, p2, p3); }
VoidOrBool  circularArc (DWGGE_TypeCR(Point3d) c, ConstDouble r, DWGGE_TypeCR(Vector3d) n, DWGGE_TypeCR(Vector3d) v1, ConstDouble a, ConstOrNot DWGGI_Type(ArcType) t) ConstOverride { return m_worldGeometry.circularArc(c, r, n, v1, a, t); }
VoidOrBool  circularArc (DWGGE_TypeCR(Point3d) p1, DWGGE_TypeCR(Point3d) p2, DWGGE_TypeCR(Point3d) p3, ConstOrNot DWGGI_Type(ArcType) t) ConstOverride { return m_worldGeometry.circularArc(p1, p2, p3, t); }
VoidOrBool  polyline (Int32OrCUInt32 n, DWGGE_TypeCP(Point3d) p, DWGGE_TypeCP(Vector3d) v, DwgDbLongPtr s) ConstOverride { return m_worldGeometry.polyline (n, p, v, s); }
VoidOrBool  polygon (Int32OrCUInt32 nPoints, DWGGE_TypeCP(Point3d) points) ConstOrNot { return m_worldGeometry.polygon(nPoints, points); }
VoidOrBool  mesh (Int32OrCUInt32 r, Int32OrCUInt32 c, DWGGE_TypeCP(Point3d) p, DWGGI_TypeCP(EdgeData) e, DWGGI_TypeCP(FaceData) f, DWGGI_TypeCP(VertexData) v
                #ifdef DWGTOOLKIT_RealDwg
                    , const bool    bAutoGenerateNormals
                #endif
                    ) ConstOverride { return m_worldGeometry.mesh(r, c, p, e, f, v
                #ifdef DWGTOOLKIT_RealDwg
                    , bAutoGenerateNormals
                #endif
                    ); }
VoidOrBool  shell (Int32OrCUInt32 n, DWGGE_TypeCP(Point3d) p, Int32OrCUInt32 fs, DwgDbInt32CP l, DWGGI_TypeCP(EdgeData) ed, DWGGI_TypeCP(FaceData) fd,  DWGGI_TypeCP(VertexData) vd
                #ifdef DWGTOOLKIT_RealDwg
                    , const struct resbuf* resBuf, const bool bAutoGenerateNormals = true
                #endif
                    ) ConstOverride { return m_worldGeometry.shell(n, p, fs, l, ed, fd, vd
                #ifdef DWGTOOLKIT_RealDwg
                    , resBuf, bAutoGenerateNormals
                #endif
                    ); }
VoidOrBool  text (DWGGE_TypeCR(Point3d) p, DWGGE_TypeCR(Vector3d) n, DWGGE_TypeCR(Vector3d) d, ConstDouble h, ConstDouble w, ConstDouble o, const DWGDB_SDKNAME(OdString&, ACHAR*) s) ConstOverride { return m_worldGeometry.text(p, n, d, h, w, o, s); }
VoidOrBool  text (DWGGE_TypeCR(Point3d) p, DWGGE_TypeCR(Vector3d) n, DWGGE_TypeCR(Vector3d) d, DwgDbCharCP s, Int32OrCInt32 c, BoolOrCBool r,
                #ifdef DWGTOOLKIT_OpenDwg
                    const OdGiTextStyle*    textStyle
                #elif DWGTOOLKIT_RealDwg
                    const AcGiTextStyle&    textStyle
                #endif
                    ) ConstOverride { return m_worldGeometry.text(p, n, d, s, c, r, textStyle); }
VoidOrBool  xline (DWGGE_TypeCR(Point3d) point1, DWGGE_TypeCR(Point3d) point2) ConstOverride { return m_worldGeometry.xline(point1, point2); }
VoidOrBool  ray (DWGGE_TypeCR(Point3d) origin, DWGGE_TypeCR(Point3d) point) ConstOverride { return m_worldGeometry.ray(origin, point); }
VoidOrBool  draw (DWGDB_SDKNAME(const OdGiDrawable,AcGiDrawable)* drawable) ConstOverride { return m_worldGeometry.draw(drawable); }
VoidOrBool  image (DWGGI_TypeCR(ImageBGRA32) s, DWGGE_TypeCR(Point3d) p, DWGGE_TypeCR(Vector3d) u, DWGGE_TypeCR(Vector3d) v, DWGGI_TransparencyMode m) ConstOverride { return m_worldGeometry.image(s, p, u, v, m); }
VoidOrBool  rowOfDots (Int32OrInt c, DWGGE_TypeCR(Point3d) p, DWGGE_TypeCR(Vector3d) t) ConstOverride { return m_worldGeometry.rowOfDots(c, p, t); }
VoidOrBool  pushClipBoundary (DWGGI_TypeP(ClipBoundary) boundary) override { return m_worldGeometry.pushClipBoundary(boundary); }
void        popClipBoundary () override { return m_worldGeometry.popClipBoundary(); }
VoidOrBool  worldLine (const DWGGE_Type(Point3d) pointsIn[2]) override { return m_worldGeometry.worldLine(pointsIn); }

// toolkit speccific methods
#ifdef DWGTOOLKIT_OpenDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
OdGeMatrix3d    getModelToWorldTransform () const override { return m_worldGeometry.getModelToWorldTransform(); }
OdGeMatrix3d    getWorldToModelTransform () const override { return m_worldGeometry.getWorldToModelTransform(); }
void            nurbs (const OdGeNurbCurve3d& nurbsCurve) override { m_worldGeometry.nurbs(nurbsCurve); }
void            ellipArc (const OdGeEllipArc3d& e, const OdGePoint3d* o, OdGiArcType t) override { m_worldGeometry.ellipArc(e, o, t); }
void            pline (const OdGiPolyline& lwBuf, OdUInt32 fromIndex, OdUInt32 numSegs) override { m_worldGeometry.pline(lwBuf, fromIndex, numSegs); }
void            edge (const OdGiEdge2dArray& edges) override { m_worldGeometry.edge(edges); }

#elif DWGTOOLKIT_RealDwg

void            getModelToWorldTransform (AcGeMatrix3d& matrix) const override { m_worldGeometry.getModelToWorldTransform(matrix); }
void            getWorldToModelTransform (AcGeMatrix3d& matrix) const override { m_worldGeometry.getWorldToModelTransform(matrix); }
AcGeMatrix3d    pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset) override { return m_worldGeometry.pushPositionTransform(behavior, offset); }
AcGeMatrix3d    pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset) override { return m_worldGeometry.pushPositionTransform(behavior, offset); }
AcGeMatrix3d    pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents) override { return m_worldGeometry.pushScaleTransform(behavior, extents); }
AcGeMatrix3d    pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents) override { return m_worldGeometry.pushScaleTransform(behavior, extents); }
AcGeMatrix3d    pushOrientationTransform (AcGiOrientationTransformBehavior behavior) override { return m_worldGeometry.pushOrientationTransform(behavior); }
Adesk::Boolean  pline (const AcDbPolyline& lwBuf, Adesk::UInt32 fromIndex = 0, Adesk::UInt32 numSegs = 0) const override { return m_worldGeometry.pline(lwBuf, fromIndex, numSegs); }
Adesk::Boolean  polyline (const AcGiPolyline& polyline) const override { return m_worldGeometry.polyline(polyline); }
Adesk::Boolean  polyPolyline (Adesk::UInt32 numPolylines, const AcGiPolyline* polylines) const override { return m_worldGeometry.polyPolyline(numPolylines, polylines); }
Adesk::Boolean  polyPolygon (const Adesk::UInt32 n, const Adesk::UInt32* np, const AcGePoint3d* pp, const Adesk::UInt32* npp, const AcGePoint3d* pv, const AcCmEntityColor* oc, const AcGiLineType* ot, const AcCmEntityColor* fc, const AcCmTransparency* fo) const override { return m_worldGeometry.polyPolygon(n, np, pp, npp, pv, oc, ot, fc, fo); }
Adesk::Boolean  ellipticalArc(const AcGePoint3d& c, const AcGeVector3d& n, double r1, double r2, double a1, double a2, double l, AcGiArcType t) const override { return m_worldGeometry.ellipticalArc(c, n, r1, r2, a1, a2, l, t); }
Adesk::Boolean  edge (const AcArray<AcGeCurve2d*>& edges) const override { return m_worldGeometry.edge(edges); }

#endif  // DWGTOOLKIT_

// implementing viewport specific methods:
virtual VoidOrBool polylineEye (ConstOrNot DwgDbUInt32 nbPoints, DWGGE_TypeCP(Point3d) pPoints) ConstOverride { RETURNFALSE }
virtual VoidOrBool polygonEye (ConstOrNot DwgDbUInt32 nbPoints, DWGGE_TypeCP(Point3d) pPoints) ConstOverride { RETURNFALSE }
virtual VoidOrBool polylineDc (ConstOrNot DwgDbUInt32 nbPoints, DWGGE_TypeCP(Point3d) pPoints) ConstOverride { RETURNFALSE }
virtual VoidOrBool polygonDc (ConstOrNot DwgDbUInt32 nbPoints, DWGGE_TypeCP(Point3d) pPoints) ConstOverride { RETURNFALSE }

#if DWGTOOLKIT_OpenDwg
virtual void rasterImageDc (const OdGePoint3d& origin, const OdGeVector3d& u, const OdGeVector3d& v, const OdGiRasterImage* pImage, const OdGePoint2d* uvBoundary, OdUInt32 numBoundPts, bool transparency = false, double brightness = 50.0, double contrast = 50.0, double fade = 0.0) override {}
virtual void metafileDc (const OdGePoint3d& origin, const OdGeVector3d& u, const OdGeVector3d& v, const OdGiMetafile* pMetafile, bool dcAligned = true, bool allowClipping = false) override {}
virtual void ownerDrawDc (const OdGePoint3d& origin, const OdGeVector3d& u, const OdGeVector3d& v, const OdGiSelfGdiDrawable* pDrawable, bool dcAligned = true, bool allowClipping = false) override {}

#elif DWGTOOLKIT_RealDwg
virtual VoidOrBool rasterImageDc
(
const AcGePoint3d&  origin,
const AcGeVector3d& u,
const AcGeVector3d& v,
const AcGeMatrix2d& pixelToDc,
AcDbObjectId        entityId, 
AcGiImageOrg        imageOrg,
Adesk::UInt32       imageWidth,
Adesk::UInt32       imageHeight,
Adesk::Int16        imageColorDepth,
Adesk::Boolean      transparency,
ImageSource         source,
const AcGeVector3d& unrotatedU,
const AcGiImageOrg  origionalImageOrg,
const AcGeMatrix2d& unrotatedPixelToDc,
const Adesk::UInt32 unrotatedImageWidth,
const Adesk::UInt32 unrotatedImageHeight
) const override
    {
    return Adesk::kFalse;
    }
virtual Adesk::Boolean  ownerDrawDc (Adesk::Int32 vpnumber, Adesk::Int32 left, Adesk::Int32 top, Adesk::Int32 right, Adesk::Int32 bottom, const OwnerDraw* pOwnerDraw) const override { return Adesk::kFalse; }
#endif  // DWGTOOLKIT_

};  // DwgGiViewportGeometry
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiViewportGeometry)
DWGRX_CONS_DEFINE_MEMBERS (DwgGiViewportGeometry, DWGGI_Type(ViewportGeometry))


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiViewport : public DWGGI_EXTENDCLASS(Viewport)
{
private:
    DWGGE_Type(Vector3d)    m_viewDirection;
    DWGGE_Type(Point3d)     m_cameraLocation;
    DWGGE_Type(Point3d)     m_cameraTarget;
    DWGGE_Type(Vector3d)    m_cameraUpDirection;
    DWGGE_Type(Point2d)     m_lowerleftCorner;
    DWGGE_Type(Point2d)     m_upperrightCorner;
    DWGGE_Type(Matrix3d)    m_worldToEye;

public:
#ifdef DWGTOOLKIT_OpenDwg
    ODRX_DECLARE_MEMBERS (DwgGiViewport);
    DWGRX_DEFINE_SMARTPTR_BASE ()
#endif

    // constructors
    DwgGiViewport () : m_viewDirection(0.0, 0.0, 1.0), m_lowerleftCorner(0.0,0.0), m_upperrightCorner(0.0,0.0) { m_worldToEye.setToIdentity(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
explicit DwgGiViewport (DVec3dCR viewDir, DVec3dCR cameraUp, DPoint3dCR cameraLoc)
    {
    m_viewDirection = Util::GeVector3dFrom (viewDir);
    m_cameraUpDirection = Util::GeVector3dFrom (cameraUp);
    m_cameraLocation = Util::GePoint3dFrom (cameraLoc);
    if (m_viewDirection == DWGGE_Type(Vector3d)::kZAxis)
        m_cameraTarget.set (0.0, 0.0, 0.0);
    else
        m_cameraTarget.setToSum (m_cameraLocation, m_viewDirection);
    m_lowerleftCorner.set (0.0, 0.0);
    m_upperrightCorner.set (0.0, 0.0);
    m_worldToEye = DWGGE_Type(Matrix3d)::worldToPlane (m_viewDirection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SetViewportRange (DRange2dCR range)
    { 
    if (!range.IsNull() && !range.IsPoint())
        {
        m_lowerleftCorner = Util::GePoint2dFrom (range.low);
        m_upperrightCorner = Util::GePoint2dFrom (range.high);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void getViewportDcCorners (DWGGE_TypeR(Point2d) lower_left, DWGGE_TypeR(Point2d) upper_right) const override
    {
    // AcDbPointCloud(as opposed to AcDbPointCloudEx), does not display objects if we return viewport corners!!
#ifdef USE_VIEWPORT_SIZE
    if (m_lowerleftCorner != m_upperrightCorner)
        {
        lower_left = m_lowerleftCorner;
        upper_right = m_upperrightCorner;
        }
#endif
    }

virtual DwgDbBool       isPerspective() const override  { return false; }
virtual DwgDbBool       doPerspective (DWGGE_TypeR(Point3d) point) const override { return false; }
virtual DwgDbBool       doInversePerspective (DWGGE_TypeR(Point3d) point) const override { return false; }
virtual void            getNumPixelsInUnitSquare (DWGGE_TypeCR(Point3d) givenWorldpt, DWGGE_TypeR(Point2d) pixelArea, bool includePerspective = true) const override { }
virtual DwgDbInt16      acadWindowId() const override { return 0; }
virtual DwgDbBool       getFrontAndBackClipValues (DwgDbBoolR clip_front, DwgDbBoolR clip_back, double& front, double& back) const override { clip_front = false, clip_back = false; return false; }
virtual double          linetypeScaleMultiplier() const override { return 1.0; }
virtual double          linetypeGenerationCriteria() const override { return 1.0; }
virtual DwgDbBool       layerVisible (StubPOrCObjectIdR idLayer) const override { return true; }
virtual UInt32OrULongPtr        viewportId() const override { return 0; }   // a GI viewport
virtual DWGGE_Type(Vector3d)    viewDir () const override { return m_viewDirection; }

#if DWGTOOLKIT_OpenDwg
virtual OdGeMatrix3d    getModelToEyeTransform () const override { return OdGeMatrix3d::kIdentity; }
virtual OdGeMatrix3d    getEyeToModelTransform () const override { return OdGeMatrix3d::kIdentity; }
virtual OdGeMatrix3d    getWorldToEyeTransform () const override { return OdGeMatrix3d::kIdentity; }
virtual OdGeMatrix3d    getEyeToWorldTransform () const override { return OdGeMatrix3d::kIdentity; }
virtual OdGePoint3d     getCameraLocation () const override { return m_cameraLocation; }
virtual OdGePoint3d     getCameraTarget () const override { return m_cameraTarget; }
virtual OdGeVector3d    getCameraUpVector() const override { return m_cameraUpDirection; }

#elif DWGTOOLKIT_RealDwg
virtual void            getModelToEyeTransform (DWGGE_TypeR(Matrix3d) matrix) const override { matrix = m_worldToEye; }
virtual void            getEyeToModelTransform (DWGGE_TypeR(Matrix3d) matrix) const override { matrix = m_worldToEye.transpose(); }
virtual void            getWorldToEyeTransform (DWGGE_TypeR(Matrix3d) matrix) const override { matrix = m_worldToEye; }
virtual void            getEyeToWorldTransform (DWGGE_TypeR(Matrix3d) matrix) const override { matrix = m_worldToEye.transpose(); }
virtual void            getCameraLocation (AcGePoint3d& location) const override { location = m_cameraLocation; }
virtual void            getCameraTarget (AcGePoint3d& target) const override { target = m_cameraTarget; }
virtual void            getCameraUpVector(AcGeVector3d& upDir) const override { upDir = m_cameraUpDirection; }
#endif  // DWGTOOLKIT_

};  // DwgGiViewport
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgGiViewport)
#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgGiViewport,OdGiViewport,RXIMPL_CONSTR)
#endif


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiWorldDraw : public DWGGI_EXTENDCLASS(WorldDraw)
{
private:
    IDwgDrawGeometryP               m_drawGeometry;
    IDwgDrawOptionsP                m_options;
    DwgDbEntityCP                   m_entity;
    DwgGiContext                    m_context;
    mutable DwgGiSubEntityTraits    m_subEntityTraits;  // initialize this one before worldGeometry
    mutable DwgGiWorldGeometry      m_worldGeometry;

    void Initialize (IDwgDrawGeometryP drawGeom, IDwgDrawOptionsP options, DwgDbEntityCP ent)
        {
        m_drawGeometry = drawGeom;
        m_options = options;
        m_entity = ent;
        if (nullptr != options)
            m_context.SetDatabase (options->_GetDatabase());
        }
public:

#ifdef DWGTOOLKIT_OpenDwg
    DWGRX_DECLARE_MEMBERS (DwgGiWorldDraw)
    DWGRX_DEFINE_SMARTPTR_BASE ()

virtual OdGiContext*                context () const override { return static_cast<OdGiContext*>(const_cast<DwgGiContext*>(&m_context)); }
virtual OdGiGeometry&               rawGeometry () const override { return m_worldGeometry; }
#elif DWGTOOLKIT_RealDwg
virtual AcGiContext*                context () override { return static_cast<AcGiContext*>(&m_context); }
virtual AcGiGeometry*               rawGeometry () const override { return &m_worldGeometry; }
#endif  // OpenDWG

virtual DWGGI_TypeR(WorldGeometry)  geometry () const override { return m_worldGeometry; }
virtual double                      deviation (const DWGGI_Type(DeviationType) deviationType, DWGGE_TypeCR(Point3d) point) const override { return 0.01; }
virtual DwgDbBool                   regenAbort () const override { return false; };
virtual DWGGI_Type(SubEntityTraits)& subEntityTraits () const override { return m_subEntityTraits; }
virtual DwgDbBool                   isDragging () const override { return false; }
virtual DwgDbUInt32                 numberOfIsolines () const override { return nullptr == m_options ? 0 : static_cast<DwgDbUInt32>(m_options->_GetNumberOfIsolines()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DWGGI_Type(RegenType)   regenType () const override
    {
    DWGGI_Type(RegenType)   regenType;

    if (nullptr == m_options)
        regenType = static_cast<DWGGI_Type(RegenType)>(DwgGiRegenType::StandardDisplay);
    else
        regenType = static_cast<DWGGI_Type(RegenType)>(m_options->_GetRegenType());

    return  regenType;
    }

// Constructors
DwgGiWorldDraw ()
    {
    this->Initialize (nullptr, nullptr, nullptr);
    }

DwgGiWorldDraw (IDwgDrawGeometryR drawGeom, IDwgDrawOptionsR options, IDwgDrawParametersR params, DwgDbEntityCP ent)
    : m_subEntityTraits(&params), m_context(options._GetDatabase()), m_worldGeometry(drawGeom, m_context, m_subEntityTraits)
    {
    this->Initialize (&drawGeom, &options, ent);
    }
};  // DwgGiWorldDraw
#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgGiWorldDraw, OdGiWorldDraw, RXIMPL_CONSTR)
#endif


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgGiViewportDraw : public DWGGI_EXTENDCLASS(ViewportDraw)
{
private:
    IDwgDrawGeometryP               m_drawGeometry;
    IDwgDrawOptionsP                m_options;
    DwgDbEntityCP                   m_entity;
    DwgGiContext                    m_context;
    DwgGiViewport                   m_viewport;
    DwgGiRegenType                  m_regenType;
    mutable DwgGiSubEntityTraits    m_subEntityTraits;  // initialize subEntTraits before viewportGeometry
    mutable DwgGiViewportGeometry   m_viewportGeometry;

public:

// implement CommonDraw methods
#ifdef DWGTOOLKIT_OpenDwg
    DWGRX_DECLARE_MEMBERS (DwgGiViewportDraw)
    DWGRX_DEFINE_SMARTPTR_BASE ()

virtual OdGiContext*                context () const override { return static_cast<OdGiContext*>(const_cast<DwgGiContext*>(&m_context)); }
virtual OdGiGeometry&               rawGeometry () const override { return m_viewportGeometry; }
#elif DWGTOOLKIT_RealDwg
virtual AcGiContext*                context () override { return static_cast<AcGiContext*>(&m_context); }
virtual AcGiGeometry*               rawGeometry () const override { return &m_viewportGeometry; }
#endif  // OpenDWG

virtual double                      deviation (const DWGGI_Type(DeviationType) deviationType, DWGGE_TypeCR(Point3d) point) const override { return 0.01; }
virtual DwgDbBool                   regenAbort () const override { return false; };
virtual DWGGI_Type(SubEntityTraits)& subEntityTraits () const override { return m_subEntityTraits; }
virtual DwgDbBool                   isDragging () const override { return false; }
virtual DwgDbUInt32                 numberOfIsolines () const override { return nullptr == m_options ? 0 : static_cast<DwgDbUInt32>(m_options->_GetNumberOfIsolines()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DWGGI_Type(RegenType)   regenType () const override
    {
    DWGGI_Type(RegenType)   regenType;

    if (nullptr == m_options)
        regenType = static_cast<DWGGI_Type(RegenType)>(DwgGiRegenType::StandardDisplay);
    else
        regenType = static_cast<DWGGI_Type(RegenType)>(m_options->_GetRegenType());

    return  regenType;
    }

// implement ViewportDraw specifics
virtual DWGGI_TypeR(ViewportGeometry)   geometry () const override { return m_viewportGeometry; }
virtual DWGGI_TypeR(Viewport)           viewport() const override { return static_cast<DWGGI_TypeR(Viewport)>(const_cast<DwgGiViewport&>(m_viewport)); }
virtual DwgDbUInt32                     sequenceNumber() const override { return 0; }
virtual DwgDbBool                       isValidId (const UInt32OrULongPtr viewportId) const override { return true; }
virtual StubPOrObjectId                 viewportObjectId () const override { return nullptr!=m_options ? m_options->_GetViewportId() : DwgDbObjectId::kNull; }

// Constructors
DwgGiViewportDraw () : m_drawGeometry(nullptr), m_options(nullptr), m_entity(nullptr) {}

DwgGiViewportDraw (IDwgDrawGeometryR drawGeom, IDwgDrawOptionsR options, IDwgDrawParametersR params, DwgDbEntityCP ent)
    : m_subEntityTraits(&params), m_context(options._GetDatabase()), m_viewportGeometry(drawGeom, m_context, m_subEntityTraits), m_viewport(options._GetViewDirection(), options._GetCameraUpDirection(), options._GetCameraLocation())
    {
    m_drawGeometry = &drawGeom;
    m_options = &options;
    m_entity = ent;
    m_regenType = options._GetRegenType ();

    DRange2d    range;
    if (options._GetViewportRange(range))
        m_viewport.SetViewportRange (range);
    }
};  // DwgGiViewportDraw
#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgGiViewportDraw, OdGiViewportDraw, RXIMPL_CONSTR)
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgDbEntity::Draw (IDwgDrawGeometryR drawGeom, IDwgDrawOptionsR options, IDwgDrawParametersR drawParams)
    {
    // Normally, we should try worldDraw first, then try viewportDraw if worldDraw failed:
    bool        worldDrawFirst = true;

#ifdef DWGTOOLKIT_RealDwg
    // But for a gradient hatch entity, RealDWG world draws it as a planar mesh - force it to viewportDraw:
    if (T_Super::isKindOf(DWGDB_Type(Hatch)::desc()))
        worldDrawFirst = false;
#endif

    if (worldDrawFirst)
        {
        DwgGiWorldDraw  worldDraw (drawGeom, options, drawParams, this);
        if (T_Super::worldDraw(&worldDraw))
            return;
        }

    DwgGiViewportDraw viewportDraw (drawGeom, options, drawParams, this);
    T_Super::viewportDraw (&viewportDraw);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiDrawable::Draw (IDwgDrawGeometryR drawGeom, IDwgDrawOptionsR options, IDwgDrawParametersR drawParams)
    {
    // call toolkit's drawable
    if (nullptr != m_toolkitDrawable)
        {
        DwgGiWorldDraw  worldDraw (drawGeom, options, drawParams, nullptr);
        if (!m_toolkitDrawable->worldDraw(&worldDraw))
            {
            DwgGiViewportDraw viewportDraw (drawGeom, options, drawParams, nullptr);
#ifdef DEBUG
            auto attribFlags = T_Super::viewportDrawLogicalFlags(viewportDraw);
#endif
            m_toolkitDrawable->viewportDraw (&viewportDraw);
            }
        }
    }

bool            DwgGiDrawable::IsValid () const { return nullptr!=m_toolkitDrawable; }
bool            DwgGiDrawable::IsPersistent () const { return nullptr!=m_toolkitDrawable ? DWGDB_IsTrue(m_toolkitDrawable->isPersistent()) : false; }
DwgDbObjectId   DwgGiDrawable::GetId () const { return nullptr!=m_toolkitDrawable ? DwgDbObjectId(m_toolkitDrawable->id()) : DwgDbObjectId(DWGDB_Type(ObjectId)::kNull); }
DwgDbEntityP    DwgGiDrawable::GetEntityP () { return DwgDbEntity::cast(m_toolkitDrawable); }
DwgDbLineP      DwgGiDrawable::GetLineP () { return DwgDbLine::cast(m_toolkitDrawable); }
DwgDbCircleP    DwgGiDrawable::GetCircleP () { return DwgDbCircle::cast(m_toolkitDrawable); }
DwgDbArcP       DwgGiDrawable::GetArcP () { return DwgDbArc::cast(m_toolkitDrawable); }
DwgDbAttributeDefinitionP  DwgGiDrawable::GetAttributeDefinitionP ()  { return DwgDbAttributeDefinition::cast(m_toolkitDrawable); }
DwgDbBlockTableRecordP     DwgGiDrawable::GetBlockP ()  { return DwgDbBlockTableRecord::cast(m_toolkitDrawable); }
DwgGiDrawable::DrawableType DwgGiDrawable::GetDrawableType () const { return nullptr!=m_toolkitDrawable ? static_cast<DrawableType>(m_toolkitDrawable->drawableType()) : DrawableType::Geometry; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgGiTextStyle::GetFontInfo (DwgFontInfo& info) const
    {
    DwgDbBool   bold = false, italic = false;
    int         charset = 0, pitchNFamily = 0;
#ifdef DWGTOOLKIT_OpenDwg
    OdString    typeface;

    this->font (typeface, bold, italic, charset, pitchNFamily);

#elif DWGTOOLKIT_RealDwg
    ACHAR*      typeface = nullptr;
#if VendorVersion <= 2016
    Acad::ErrorStatus   es = this->font (typeface, bold, italic, charset, pitchNFamily);
#else   // >= 2017
    Charset     acCharset = Charset::kAnsiCharset;

    Autodesk::AutoCAD::PAL::FontUtils::FontPitch    pitch;
    Autodesk::AutoCAD::PAL::FontUtils::FontFamily   family;

    Acad::ErrorStatus   es = this->font (typeface, bold, italic, acCharset, pitch, family);
#endif  // 2016
#endif  // DWGTOOLKIT_

    WString     shx = this->GetFileName ();
    WString     bigfont = this->GetBigFontFileName ();

#ifdef DWGTOOLKIT_OpenDwg

    const OdChar*   odChars = typeface.c_str ();
    info.Set (reinterpret_cast<WCharCP>(odChars), shx.c_str(), bigfont.c_str(), bold, italic, charset, pitchNFamily);
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg

    if (Acad::eOk == es)
        {
#if VendorVersion >= 2017
        charset = acCharset;
        pitchNFamily = static_cast<int>(pitch) | static_cast<int>(family);
#endif

        info.Set (typeface, shx.c_str(), bigfont.c_str(), Adesk::kTrue == bold, Adesk::kTrue == italic, charset, pitchNFamily);
        }

    acutDelString (typeface);
    return  ToDwgDbStatus(es);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DwgGiTextStyle::GetFileName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdFont*         font = this->getFont ();
    if (nullptr != font)
        return WString (font->getFileName().c_str());
    return WString();
#elif DWGTOOLKIT_RealDwg
    return this->fileName ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgGiTextStyle::CopyFrom (DwgDbTextStyleTableRecordCR dbStyle)
    {
    DwgDbStatus     status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::set (dbStyle.fileName(), dbStyle.bigFontFileName(), dbStyle.textSize(), dbStyle.xScale(), dbStyle.obliquingAngle(),
                  0.5, dbStyle.isBackwards(), dbStyle.isUpsideDown(), dbStyle.isVertical(), false, false);

    T_Super::loadStyleRec (dbStyle.database());

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = ::fromAcDbTextStyle (*this, dbStyle.objectId());

    // above call does not appear to copy SHX filename!!??
    const ACHAR*        fileName = nullptr;
    if (Acad::eOk == es && Acad::eOk == (es = dbStyle.fileName(fileName)))
        this->setFileName (fileName);
    status = ToDwgDbStatus (es);

    int     loadStatus = T_Super::loadStyleRec (dbStyle.database());

    /*-----------------------------------------------------------------------------------
    RealDWG font load status:
    0x10 =  Another bigfont file opened and loaded in place of the bigfont already in use by this AcGiTextStyle object. The new file name has been copied into the AcGiTextStyle object.  
    0x08 =  Another font file opened and loaded in place of the font already in use by this AcGiTextStyle object. The new file name has been copied into the AcGiTextStyle object.  
    0x04 =  The currently specified bigfont file failed to load.  
    0x02 =  The currently specified font file failed to load.  
    0x01 =  Both the current font and bigfont files were loaded. 
    -----------------------------------------------------------------------------------*/
    if (0 != (loadStatus & 0x02))
        status = DwgDbStatus::FileNotFound;
#endif

    return  status;
    }

WString         DwgGiTextStyle::GetBigFontFileName () const { return DWGDB_CALLSDKMETHOD(reinterpret_cast<WCharCP>(bigFontFileName().c_str()), bigFontFileName()); }
double          DwgGiTextStyle::GetTextSize () const { return this->textSize(); }
double          DwgGiTextStyle::GetWidthFactor () const { return this->xScale(); }
double          DwgGiTextStyle::GetObliqueAngle () const { return this->obliquingAngle(); }
bool            DwgGiTextStyle::IsVertical () const { return DWGDB_IsTrue(this->isVertical()); }
bool            DwgGiTextStyle::IsUnderlined () const { return DWGDB_IsTrue(this->isUnderlined()); }
bool            DwgGiTextStyle::IsOverlined () const { return DWGDB_IsTrue(this->isOverlined()); }
bool            DwgGiTextStyle::IsStrikethrough () const { return DWGDB_CALLSDKMETHOD(this->isStriked(), DWGDB_IsTrue(this->isStrikethrough())); }
bool            DwgGiTextStyle::IsBackward () const { return DWGDB_IsTrue(this->isBackward()); }
bool            DwgGiTextStyle::IsUpsideDown () const { return DWGDB_IsTrue(this->isUpsideDown()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiFill::DwgGiFill ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_deviation = 0.0;
#elif DWGTOOLKIT_RealDwg
    m_acFill = new AcGiFill ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiFill::~DwgGiFill ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_acFill)
        delete m_acFill;
    m_acFill = nullptr;
#endif
    }

#ifdef DWGTOOLKIT_RealDwg
DwgGiFill::DwgGiFill (const AcGiFill& fill) { this->SetAcGiFill(&fill); }
DwgGiGradientFill::DwgGiGradientFill (const AcGiGradientFill& grad) : m_tint(0.0) { T_Super::SetAcGiFill(AcGiFill::cast(&grad)); }
DwgGiHatchPattern::DwgGiHatchPattern (const AcGiHatchPattern& hatch) { T_Super::SetAcGiFill(AcGiFill::cast(&hatch)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiFill::SetAcGiFill (const AcGiFill* fill)
    {
    if (nullptr == fill)
        {
        if (nullptr != m_acFill)
            delete m_acFill;
        m_acFill = nullptr;
        }
    else
        {
        AcGiHatchPattern*   pattern;
        AcGiGradientFill*   grad = AcGiGradientFill::cast (fill);
        if (nullptr != grad)
            m_acFill = new AcGiGradientFill (*grad);
        else if (nullptr != (pattern = AcGiHatchPattern::cast(fill)))
            m_acFill = new AcGiHatchPattern (*pattern);
        else
            this->SetDeviation (fill->deviation());
        }
    }
#endif  // DWGTOOLKIT_RealDwg

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiFill::SetDeviation (double dev)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_deviation = dev;
    
#elif DWGTOOLKIT_RealDwg
    if (nullptr == m_acFill)
        m_acFill = new AcGiFill ();
    if (nullptr != m_acFill)
        m_acFill->setDeviation (dev);
#endif
    }
double          DwgGiFill::GetDeviation () const { DWGDB_CALLSDKMETHOD(return m_deviation, return nullptr==m_acFill ? 0.0 : m_acFill->deviation()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiFillPtr    DwgGiFill::Clone () const
    {
    DwgGiGradientFillCP gradient = static_cast<DwgGiGradientFillCP> (this);
    if (gradient == nullptr)
        {
        DwgGiFillP fill = new DwgGiFill ();
        fill->SetDeviation (this->GetDeviation());
        return  fill;
        }
    else
        {
        DwgColorArray   colors;
        gradient->GetColors (colors);
        return new DwgGiGradientFill(gradient->GetAngle(), gradient->GetShift(), gradient->GetTint(), gradient->IsAdjustAspect(), colors, gradient->GetType());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiGradientFillPtr    DwgGiGradientFill::CreateFrom (DwgDbHatchCP hatchIn)
    {
    // AcDbHatch::getGradientColors is not a const method!!
    DwgDbHatchP         hatch = const_cast<DwgDbHatchP> (hatchIn);
    if (nullptr == hatch || !hatch->IsGradient())
        return  DwgGiGradientFillPtr();

    DwgGiGradientFill::Type type = DwgGiGradientFill::Type::Linear;
    DwgString               name = hatch->GetGradientName ();

    // name to type
    if (name.EqualsI(L"Linear"))
        type = DwgGiGradientFill::Type::Linear;
    else if (name.EqualsI(L"Spherical"))
        type = DwgGiGradientFill::Type::Spherical;
    else if (name.EqualsI(L"InvSpherical"))
        type = DwgGiGradientFill::Type::InvSpherical;
    else if (name.EqualsI(L"Hemispherical"))
        type = DwgGiGradientFill::Type::Hemispherical;
    else if (name.EqualsI(L"InvHemispherical"))
        type = DwgGiGradientFill::Type::InvHemispherical;
    else if (name.EqualsI(L"Cylinder"))
        type = DwgGiGradientFill::Type::Cylinder;
    else if (name.EqualsI(L"InvCylinder"))
        type = DwgGiGradientFill::Type::InvCylinder;
    else if (name.EqualsI(L"Curved"))
        type = DwgGiGradientFill::Type::Curved;
    else if (name.EqualsI(L"InvCurved"))
        type = DwgGiGradientFill::Type::InvCurved;

    DwgColorArray       colorArray;
    DwgDbDoubleArray    valueArray;
    hatch->GetGradientColors (colorArray, valueArray);

    return new DwgGiGradientFill(hatch->GetGradientAngle(), hatch->GetGradientShift(), hatch->GetGradientTint(), false, colorArray, type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiGradientFill::DwgGiGradientFill (double angle, double shift, double tint, bool adjustAspect, DwgColorArrayCR colors, Type type)
    {
    m_tint = tint;

#ifdef DWGTOOLKIT_OpenDwg
    m_angle = angle;
    m_shift = shift;
    m_adjustAspect = adjustAspect;
    m_colorArray = colors;
    m_type = type;

#elif DWGTOOLKIT_RealDwg
    AcArray<AcCmColor>      acColors;
    for (auto const& color : colors)
        acColors.append (static_cast<AcCmColor const&>(color));

    m_acFill = new AcGiGradientFill (angle, shift, adjustAspect, acColors, static_cast<AcGiGradientFill::GradientType>(type));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgGiGradientFill::GetColors (DwgColorArrayR colorsOut) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    colorsOut = m_colorArray;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        {
        AcArray<AcCmColor>  acColors = grad->gradientColors ();
        for (int i = 0; i < acColors.length(); i++)
            colorsOut.push_back (static_cast<DwgCmColorCR>(acColors[i]));
        }
#endif
    return colorsOut.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiGradientFill::SetColors (DwgColorArrayCR colorsIn)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_colorArray = colorsIn;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        {
        AcArray<AcCmColor>  acColors;
        for (auto const& color : colorsIn)
            acColors.append (static_cast<AcCmColor>(color));
        grad->setGradientColors (acColors);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgGiGradientFill::GetAngle () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  m_angle;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    return nullptr == grad ? 0.0 : grad->gradientAngle();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiGradientFill::SetAngle (double angle)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_angle = angle;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        grad->setGradientAngle (angle);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgGiGradientFill::GetShift () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  m_shift;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    return nullptr == grad ? 0.0 : grad->gradientShift();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiGradientFill::SetShift (double shift)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_shift = shift;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        grad->setGradientShift (shift);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgGiGradientFill::IsAdjustAspect () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  m_adjustAspect;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    return nullptr == grad ? false : grad->isAdjustAspect();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiGradientFill::SetAdjustAspect (bool isTrue)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_adjustAspect = isTrue;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        grad->setIsAdjustAspect (isTrue);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiGradientFill::Type DwgGiGradientFill::GetType () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  m_type;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    return nullptr == grad ? DwgGiGradientFill::Type::Linear : static_cast<DwgGiGradientFill::Type>(grad->gradientType());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgGiGradientFill::SetType (DwgGiGradientFill::Type type)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_type = type;
#elif DWGTOOLKIT_RealDwg
    AcGiGradientFill* grad = AcGiGradientFill::cast(m_acFill);
    if (nullptr != grad)
        grad->setGradientType (static_cast<AcGiGradientFill::GradientType>(type));
#endif
    }
double          DwgGiGradientFill::GetTint () const { return m_tint; }
void            DwgGiGradientFill::SetTint (double t) { m_tint = t; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiHatchPatternDefinition::DwgGiHatchPatternDefinition (double angle, double baseX, double baseY, double offX, double offY, DWGGE_TypeR(DoubleArray) defs)
#ifdef DWGTOOLKIT_OpenDwg
    {
    m_angle = angle;
    m_baseX = baseX;
    m_baseY = baseY;
    m_offsetX = offX;
    m_offsetY = offY;

    for (uint32_t i = 0; i < defs.size(); i++)
        m_dashArray.push_back (defs.at(i));

#elif DWGTOOLKIT_RealDwg
        : T_Super(angle, baseX, baseY, offX, offY, defs)
    {
#endif  // DWGTOOLKIT_
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiHatchPatternPtr    DwgGiHatchPattern::CreateFrom (DwgDbHatchCP hatch)
    {
    if (nullptr != hatch)
        {
        DwgPatternArray     patterns;
        if (hatch->GetPatternDefinitions(patterns) > 0)
            return new DwgGiHatchPattern(patterns);
        }
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiHatchPattern::DwgGiHatchPattern (DwgPatternArrayCR patterns)
    {
#ifdef DWGTOOLKIT_OpenDwg
    m_patternArray = patterns;

#elif DWGTOOLKIT_RealDwg
    AcArray<AcGiHatchPatternDefinition> acPatterns;
    for (auto const& pat : patterns)
        acPatterns.append (static_cast<AcGiHatchPatternDefinition const&>(pat));

    m_acFill = new AcGiHatchPattern (acPatterns);
#endif
    }

void                        DwgGiMapper::GetTransform (TransformR t) const { Util::GetTransform(t, T_Super::transform()); }
DwgGiMapper::TransformBy    DwgGiMapper::GetAutoTransform () const { return static_cast<TransformBy>(T_Super::autoTransform()); }
DwgGiMapper::ProjectBy      DwgGiMapper::GetProjection () const { return static_cast<ProjectBy>(T_Super::projection()); }
DwgGiMapper::TileBy         DwgGiMapper::GetUTiling () const { return static_cast<TileBy>(T_Super::uTiling()); }
DwgGiMapper::TileBy         DwgGiMapper::GetVTiling () const { return static_cast<TileBy>(T_Super::vTiling()); }

void                        DwgGiMaterialColor::GetColor (DwgCmEntityColorR c) const { c = T_Super::color(); }
double                      DwgGiMaterialColor::GetFactor () const { return T_Super::factor(); }
DwgGiMaterialColor::ColorBy DwgGiMaterialColor::GetMethod () const { return static_cast<ColorBy>(T_Super::method()); }

double                      DwgGiMaterialMap::GetBlendFactor () const { return T_Super::blendFactor(); }
DwgGiMaterialMap::FilterBy  DwgGiMaterialMap::GetFilter () const { return DWGDB_CALLSDKMETHOD(FilterBy::Default,static_cast<FilterBy>(T_Super::filter())); }
void                        DwgGiMaterialMap::GetMapper (DwgGiMapperR m) const { m = T_Super::mapper(); }
DwgGiMaterialMap::ImageBy   DwgGiMaterialMap::GetSource () const { return static_cast<ImageBy>(T_Super::source()); }
DwgGiMaterialTextureCP      DwgGiMaterialMap::GetTexture () const { return static_cast<DwgGiMaterialTextureCP>(T_Super::DWGDB_CALLSDKMETHOD(texture().get,texture)()); }
DwgString   DwgGiMaterialMap::GetSourceFileName () const { return T_Super::sourceFileName(); }

DwgGiImageFileTextureCP     DwgGiMaterialTexture::ToDwgGiImageFileTextureCP () const { return static_cast<DwgGiImageFileTextureCP>(T_Super::DWGDB_CALLSDKMETHOD(cast(this).get(),cast(this))); }
DwgGiGenericTextureCP       DwgGiMaterialTexture::ToDwgGiGenericTextureCP () const { return static_cast<DwgGiGenericTextureCP>(T_Super::DWGDB_CALLSDKMETHOD(cast(this).get(),cast(this))); }
DwgGiMarbleTextureCP        DwgGiMaterialTexture::ToDwgGiMarbleTextureCP () const { return static_cast<DwgGiMarbleTextureCP>(T_Super::DWGDB_CALLSDKMETHOD(cast(this).get(),cast(this))); }
DwgGiWoodTextureCP          DwgGiMaterialTexture::ToDwgGiWoodTextureCP () const { return static_cast<DwgGiWoodTextureCP>(T_Super::DWGDB_CALLSDKMETHOD(cast(this).get(),cast(this))); }
bool                        DwgGiMaterialTexture::operator==(DwgGiMaterialTextureCR t) const { return t.isEqualTo(this); }
DwgString   DwgGiImageFileTexture::GetSourceFileName () const { return T_Super::sourceFileName(); }

#ifdef DWGTOOLKIT_RealDwg
// RealDWG must have these missed in their implementation!
Acad::ErrorStatus   AcGiImageFileTexture::copyFrom(const AcRxObject* other) { return __super::copyFrom(other); }
Acad::ErrorStatus   AcGiMarbleTexture::copyFrom(const AcRxObject* other) { return __super::copyFrom(other); }
Acad::ErrorStatus   AcGiWoodTexture::copyFrom(const AcRxObject* other) { return __super::copyFrom(other); }
#endif


DwgGiLightAttenuation::Type DwgGiLightAttenuation::GetType () const { return static_cast<Type>(T_Super::attenuationType()); }
bool    DwgGiLightAttenuation::UseLimits () const { return T_Super::useLimits(); }
double  DwgGiLightAttenuation::GetStartLimit () const { return T_Super::startLimit(); }
double  DwgGiLightAttenuation::GetEndLimit () const { return T_Super::endLimit(); }

bool   DwgGiShadowParameters::AreShadowsOn () const { return T_Super::shadowsOn(); }
DwgGiShadowParameters::Type DwgGiShadowParameters::GetShadowType () const { return static_cast<Type>(T_Super::shadowType()); }
uint16_t   DwgGiShadowParameters::GetShadowMapSize () const { return T_Super::shadowMapSize(); }
uint8_t    DwgGiShadowParameters::GetShadowMapSoftness () const { return T_Super::shadowMapSoftness(); }
uint16_t   DwgGiShadowParameters::GetShadowSamples () const { return T_Super::shadowSamples(); }
bool   DwgGiShadowParameters::IsShapeVisible() const { return T_Super::shapeVisibility(); }
DwgGiShadowParameters::Shape  DwgGiShadowParameters::GetExtendedLightShape() const { return static_cast<Shape>(T_Super::extendedLightShape()); }
double DwgGiShadowParameters::GetExtendedLightLength() const { return  T_Super::extendedLightLength(); }
double DwgGiShadowParameters::GetExtendedLightWidth() const { return T_Super::extendedLightWidth(); }
double DwgGiShadowParameters::GetExtendedLightRadius() const { return T_Super::extendedLightRadius(); }

DwgGiSkyParameters::DwgGiSkyParameters (DWGGI_TypeCR(SkyParameters) in) { m_toolkitImpl = in; }
bool       DwgGiSkyParameters::HasAerialPerspective() const { return m_toolkitImpl.aerialPerspective(); }
double     DwgGiSkyParameters::GetDiskIntensity () const { return m_toolkitImpl.diskIntensity(); }
double     DwgGiSkyParameters::GetDiskScale () const { return m_toolkitImpl.diskScale(); }
double     DwgGiSkyParameters::GetGlowIntensity () const { return m_toolkitImpl.glowIntensity(); }
DwgCmEntityColor DwgGiSkyParameters::GetGroundColor () const { return DWGDB_CALLSDKMETHOD(m_toolkitImpl.groundColor(),m_toolkitImpl.groundColor().entityColor()); }
DwgCmEntityColor DwgGiSkyParameters::GetNightColor () const { return DWGDB_CALLSDKMETHOD(m_toolkitImpl.nightColor(),m_toolkitImpl.nightColor().entityColor()); }
double     DwgGiSkyParameters::GetHaze () const { return m_toolkitImpl.haze(); }
double     DwgGiSkyParameters::GetHorizonBlur () const { return m_toolkitImpl.horizonBlur(); }
double     DwgGiSkyParameters::GetHorizonHeight () const { return m_toolkitImpl.horizonHeight(); }
bool       DwgGiSkyParameters::HasIllumination () const { return m_toolkitImpl.illumination(); }
double     DwgGiSkyParameters::GetIntensityFactor () const { return m_toolkitImpl.intensityFactor(); }
double     DwgGiSkyParameters::GetRedBlueShift () const { return m_toolkitImpl.redBlueShift(); }
double     DwgGiSkyParameters::GetSaturation () const { return m_toolkitImpl.saturation(); }
uint16_t   DwgGiSkyParameters::GetSolarDiskSamples () const { return m_toolkitImpl.solarDiskSamples(); }
DVec3d     DwgGiSkyParameters::GetSunDirection () const { return Util::DVec3dFrom(m_toolkitImpl.sunDirection()); }
double     DwgGiSkyParameters::GetVisibilityDistance () const { return m_toolkitImpl.visibilityDistance(); }


#ifdef DWGTOOLKIT_OpenDwg
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          10/16
+===============+===============+===============+===============+===============+======*/
struct TextContourCollector : public OdStaticRxObject<OdGiConveyorNode>, public OdGiEmptyGeometry, public OdGiConveyorInput, public OdGiConveyorOutput
{
private:
    struct TextStroker : public OdGiBaseVectorizer, public OdGiContextForDbDatabase
    {
    public:
    ODRX_USING_HEAP_OPERATORS(OdGiBaseVectorizer);
    // Constructor
    TextStroker ()
        {
        OdGiBaseVectorizer::m_flags |= (kDrawInvisibleEnts|kDrawLayerOff|kDrawLayerFrozen);
        setContext (this);
        m_pModelToEyeProc->setDrawContext (OdGiBaseVectorizer::drawContext());
        }

    OdGiRegenType regenType() const override { return kOdGiForExplode; }
    const OdGiSubEntityTraitsData& effectiveTraits () const override { return m_entityTraitsData; }
    void affectTraits (const OdGiSubEntityTraitsData* traitsIn, OdGiSubEntityTraitsData& traitsOut) const override {}
    };  // TextStroker

    OdStaticRxObject<TextStroker>   m_textstroker;
    DPoint3dArray                   m_newLinestring;
    bvector<DPoint3dArray>*         m_outputLinestrings;
    DRange3dP                       m_outputRange;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddPoint (DPoint3dCR newPoint)
    {
    if (nullptr != m_outputLinestrings)
        m_newLinestring.push_back (newPoint);
    if (nullptr != m_outputRange)
        m_outputRange->Extend (newPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void StartLineString ()
    {
    m_newLinestring.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FinishLineString (DPoint3dCP lastPoint = nullptr)
    {
    if (nullptr != m_outputLinestrings)
        {
        if (nullptr != lastPoint)
            m_newLinestring.push_back (*lastPoint);
        m_outputLinestrings->push_back (m_newLinestring);
        m_newLinestring.clear ();
        }
    if (nullptr != m_outputRange && nullptr != lastPoint)
        m_outputRange->Extend (*lastPoint);
    }

protected:
    OdGiConveyorInput& input() override { return *this; }
    OdGiConveyorOutput& output() override { return *this; }
    void addSourceNode (OdGiConveyorOutput& sourceNode) override { sourceNode.setDestGeometry (*this); }
    void removeSourceNode (OdGiConveyorOutput& /*sourceNode*/) override { }
    void setDestGeometry (OdGiConveyorGeometry& destGeometry) override { }
    OdGiConveyorGeometry& destGeometry () const override { return OdGiEmptyGeometry::kVoid; }

public:
// Constructors
TextContourCollector (bvector<DPoint3dArray>& output) : m_outputLinestrings(&output), m_outputRange(nullptr) {}
TextContourCollector (DRange3dR output) : m_outputLinestrings(nullptr), m_outputRange(&output) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void polylineProc (OdInt32 nPoints, const OdGePoint3d* points, const OdGeVector3d* normal, const OdGeVector3d* extrusion, OdGsMarker subEntMarker) override
    {
    this->StartLineString ();

    for (OdInt32 i = 0; i < nPoints; i++)
        this->AddPoint (Util::DPoint3dFrom(points[i]));

    this->FinishLineString ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void circleProc (const OdGePoint3d& center, double radius, const OdGeVector3d& normal, const OdGeVector3d* extrusion) override
    {
    double      alpha = 0.0;
    DPoint3d    strokePoint;

    this->StartLineString ();

    // tessellate the circle
    for (; alpha < msGeomConst_2pi; alpha += 0.1)
        {
        strokePoint.Init (center.x - cos(alpha) * radius, center.y + sin(alpha) * radius, 0.0);
        this->AddPoint (strokePoint);
        }

    // close the circle
    strokePoint.Init (center.x - radius, center.y, 0.0);
    this->FinishLineString (&strokePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void circularArcProc (const OdGePoint3d& p1, const OdGePoint3d& p2, const OdGePoint3d& p3, OdGiArcType arcType, const OdGeVector3d* extrusion) override
    {
    DEllipse3d  arc = DEllipse3d::FromPointsOnArc (Util::DPoint3dFrom(p1), Util::DPoint3dFrom(p2), Util::DPoint3dFrom(p3));
    double      alpha = arc.start;
    double      end = arc.IsCCWSweepXY() ? alpha + arc.sweep : alpha - arc.sweep;
    double      radius = arc.vector0.Magnitude ();
    DPoint3d    strokePoint;

    this->StartLineString ();

    // tessellate the arc
    for (; alpha < end; alpha += 0.1)
        {
        strokePoint.Init (arc.center.x - cos(alpha) * radius, arc.center.y + sin(alpha) * radius, 0.0);
        this->AddPoint (strokePoint);
        }

    // add the last segment    
    strokePoint.Init (arc.center.x - cos(end) * radius, arc.center.y + sin(end) * radius, 0.0);
    this->FinishLineString (&strokePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void ellipArcProc (const OdGeEllipArc3d& ellipse, const OdGePoint3d* point, OdGiArcType arcType, const OdGeVector3d* extrusion) override
    {
    DPoint3d    center = Util::DPoint3dFrom (ellipse.center());
    double      alpha = ellipse.startAng ();
    double      end = ellipse.endAng ();
    double      r1 = ellipse.majorRadius ();
    double      r2 = ellipse.minorRadius ();
    DPoint3d    strokePoint;

    this->StartLineString ();

    // tessellate the ellipse
    for (; alpha < end; alpha += 0.1)
        {
        strokePoint.Init (center.x - cos(alpha) * r1, center.y + sin(alpha) * r2, 0.0);
        this->AddPoint (strokePoint);
        }

    // add the last segment    
    strokePoint.Init (center.x - cos(end) * r1, center.y + sin(end) * r2, 0.0);
    this->FinishLineString (&strokePoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tessellate (const OdString& text, const OdGiTextStyle& textstyle)
    {
    OdGePoint3d     origin(0, 0, 0);
    OdGeVector3d    xAxis(1, 0, 0);
    OdGeVector3d    yAxis(0, 1, 0);

    if (nullptr != m_outputLinestrings)
        m_outputLinestrings->clear ();
    if (nullptr != m_outputRange)
        m_outputRange->Init ();

    m_textstroker.context()->drawText (this, origin, xAxis, yAxis, text.c_str(), text.getLength(), true, &textstyle, nullptr);
    }
};  // TextContourCollector

#elif DWGTOOLKIT_RealDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void     CollectLinestrings (int nPlines, int const* nPoints, AcGePoint3d const* points, void* args)
    {
    bvector<DPoint3dArray>* linestringArray = static_cast<bvector<DPoint3dArray>*> (args);
    if (nullptr == linestringArray || nPlines < 1 || nullptr == nPoints || nullptr == points)
        return;

    int count = 0;
    for (int i = 0; i < nPlines; i++)
        {
        DPoint3dArray   newLinestring;
        for (int j = 0; j < nPoints[i]; j++)
            newLinestring.push_back (Util::DPoint3dFrom(points[count++]));

        if (!newLinestring.empty())
            linestringArray->push_back (newLinestring);
        }
    }
#endif  // DWGTOOLKIT_

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     ShapeTextProcessor::Drop (bvector<DPoint3dArray>& linestringsOut, double deviation)
    {
#ifdef DWGTOOLKIT_OpenDwg
    TextContourCollector    contourTracer (linestringsOut);
    contourTracer.Tessellate (m_textstring, m_textstyle);

#elif DWGTOOLKIT_RealDwg

    AcGiTextEngine* textEngine = AcGiTextEngine::create ();
    if (nullptr == textEngine)
        return  DwgDbStatus::MemoryError;

    linestringsOut.clear ();

    textEngine->tessellate (m_textstyle, m_textstring.c_str(), m_textstring.length(), m_raw, deviation, (void*)&linestringsOut, (PolylineCallback)CollectLinestrings);
    
    delete textEngine;
#endif

    return  linestringsOut.empty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     ShapeTextProcessor::GetExtents (DPoint2dR extentsOut, bool bearings)
    {
#ifdef DWGTOOLKIT_OpenDwg
    DRange3d    extents3d = DRange3d::NullRange ();

    TextContourCollector    extentsFinder (extents3d);
    extentsFinder.Tessellate (m_textstring, m_textstyle);

    extentsOut.x = extents3d.XLength ();
    extentsOut.y = extents3d.YLength ();

#elif DWGTOOLKIT_RealDwg

    AcGiTextEngine* textEngine = AcGiTextEngine::create ();
    if (nullptr == textEngine)
        return  DwgDbStatus::MemoryError;

    AcGePoint2d     extents;
    textEngine->getExtents (m_textstyle, m_textstring.c_str(), m_textstring.length(), bearings, m_raw, extents);

    extentsOut = Util::DPoint2dFrom (extents);
    
    delete textEngine;
#endif

    return  extentsOut.MaxAbs() == 0.0 ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    RegisterDwgGiExtensions ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    DwgGiContext::rxInit ();
    DwgGiSubEntityTraits::rxInit ();
    DwgGiViewport::rxInit ();
    DwgGiWorldDraw::rxInit ();
    DwgGiViewportDraw::rxInit ();
#endif  // DWGTOOLKIT_OpenDwg
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    UnRegisterDwgGiExtensions ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    DwgGiContext::rxUninit ();
    DwgGiSubEntityTraits::rxUninit ();
    DwgGiViewport::rxUninit ();
    DwgGiWorldDraw::rxUninit ();
    DwgGiViewportDraw::rxUninit ();
#endif  // DWGTOOLKIT_OpenDwg
    }
