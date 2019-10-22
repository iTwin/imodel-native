/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

DWGDB_OBJECT_DEFINE_BASEMEMBERS (Object)
DWGDB_OBJECT_DEFINE_BASEMEMBERS (Dictionary)
DWGDB_OBJECT_DEFINE_MEMBERS2 (Material)
DWGDB_OBJECT_DEFINE_MEMBERS2 (GradientBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (GroundPlaneBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (IBLBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (ImageBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (SkyBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (SolidBackground)
DWGDB_OBJECT_DEFINE_MEMBERS2 (Sun)
DWGDB_OBJECT_DEFINE_MEMBERS2 (VisualStyle)
DWGDB_OBJECT_DEFINE_MEMBERS2 (Layout)
DWGDB_OBJECT_DEFINE_MEMBERS2 (Group)
DWGDB_OBJECT_DEFINE_MEMBERS2 (SpatialFilter)
DWGDB_OBJECT_DEFINE_MEMBERS2 (SpatialIndex)
DWGDB_OBJECT_DEFINE_MEMBERS2 (SortentsTable)
DWGDB_OBJECT_DEFINE_MEMBERS2 (Xrecord)

#ifdef DWGTOOLKIT_OpenDwg
#define ReturnVoidOrStatus(_name_)  { ##_name_##; return DwgDbStatus::Success; }
#define ReturnBoolOrStatus(_name_)  { bool b = ##_name_##; return b ? DwgDbStatus::Success : DwgDbStatus::UnknownError; }

#elif DWGTOOLKIT_RealDwg
#define ReturnVoidOrStatus(_name_)  return ToDwgDbStatus(##_name_##)
#define ReturnBoolOrStatus(_name_)  return ToDwgDbStatus(##_name_##)


// An unpublished API in acdbxx.dll
typedef Acad::ErrorStatus (*acdbGetPaperImageOriginFunc)(AcDbPlotSettings* p, double& x, double& y);
static char s_acdbGetPaperImageOrigin[] = "?acdbGetPaperImageOrigin@@YA?AW4ErrorStatus@Acad@@PEAVAcDbPlotSettings@@AEAN1@Z";
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString           DwgDbHandle::AsAscii () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    if (!T_Super::isNull())
        return  T_Super::ascii ();
#elif DWGTOOLKIT_RealDwg
    ACHAR   ascii[128] = { 0 };
    if (T_Super::getIntoAsciiBuffer(ascii, sizeof(ascii)))
        return  DwgString(ascii);
#endif
    return  DwgString();
    }
uint64_t            DwgDbHandle::AsUInt64 () const { return  static_cast <uint64_t> (*this); }
bool                DwgDbHandle::IsNull () const { return T_Super::isNull(); }
void                DwgDbHandle::SetNull () { DWGDB_CALLSDKMETHOD(T_Super,T_Super::setNull)(); }
DwgDbHandleR        DwgDbHandle::operator = (DWGDB_TypeCR(Handle)h) { T_Super::operator=(h); return *this; }

bool                DwgDbObjectId::IsNull () const { return  T_Super::isNull(); }
bool                DwgDbObjectId::IsValid () const { return  T_Super::isValid(); }
DwgDbDatabaseP      DwgDbObjectId::GetDatabase () const { return  static_cast<DwgDbDatabaseP>(database()); }
DwgDbHandle         DwgDbObjectId::GetHandle () const { return DWGDB_CALLSDKMETHOD(getHandle, handle)(); }
uint64_t            DwgDbObjectId::ToUInt64 () const { return this->GetHandle().AsUInt64(); }
DwgString           DwgDbObjectId::ToAscii () const { return this->GetHandle().AsAscii(); }
void*               DwgDbObjectId::PeekStubPtr() const { return DWGDB_CALLSDKMETHOD((OdDbStub*), (AcDbStub*)) this; }
void                DwgDbObjectId::SetNull () { T_Super::setNull(); }
bool                DwgDbObjectId::operator == (DwgDbObjectIdCR id) const { return DWGDB_CALLSDKMETHOD(T_Super::m_Id, T_Super::mId) == id; }
bool                DwgDbObjectId::operator != (DwgDbObjectIdCR id) const { return DWGDB_CALLSDKMETHOD(T_Super::m_Id, T_Super::mId) != id; }
bool                DwgDbObjectId::operator < (DwgDbObjectIdCR id) const { return DWGDB_CALLSDKMETHOD(T_Super::m_Id, T_Super::mId) < id; }
bool                DwgDbObjectId::operator > (DwgDbObjectIdCR id) const { return DWGDB_CALLSDKMETHOD(T_Super::m_Id, T_Super::mId) > id; }
DwgDbObjectIdR      DwgDbObjectId::operator = (DWGDB_TypeP(Stub) stub) { DWGDB_CALLSDKMETHOD(T_Super::m_Id, T_Super::mId) = stub; return *this; }
DwgDbObjectIdR      DwgDbObjectId::operator = (DWGDB_TypeCR(ObjectId) id) { DWGDB_CALLSDKMETHOD(T_Super::m_Id=id, T_Super::mId=id); return *this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbObjectId::GetDwgClassName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectPtr   obj = T_Super::safeOpenObject ();
    if (obj.isNull())
        {
        BeAssert (false && "Null object opened from OdDbObjectId");
        return  L"NullObject";
        }
    return obj->isA()->name();
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    className = T_Super::objectClass()->name ();
    if (nullptr != className && 0 != className[0])
        return  className;
    return  DwgString(L"NullName");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass) DwgDbObjectId::GetDwgClass () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectPtr   obj = T_Super::safeOpenObject ();
    if (obj.isNull())
        {
        BeAssert (false && "Null object opened from OdDbObjectId");
        return  nullptr;
        }
    return obj->isA ();
#elif DWGTOOLKIT_RealDwg
    return T_Super::objectClass ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbObjectId::IsObjectDerivedFrom (DWG_TypeCP(RxClass) rxClass) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectPtr   obj = T_Super::safeOpenObject ();
    if (obj.isNull())
        {
        BeAssert (false && "Null object opened from OdDbObjectId");
        return  false;
        }
    return obj->isA()->isDerivedFrom (rxClass);
#elif DWGTOOLKIT_RealDwg
    return T_Super::objectClass()->isDerivedFrom(rxClass);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectP    DwgDbObjectId::OpenObject (DwgDbOpenMode mode, bool openErased) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  dynamic_cast<DwgDbObjectP> (T_Super::openObject(static_cast<OdDb::OpenMode>(mode), openErased).get());
#elif DWGTOOLKIT_RealDwg
    AcDbObject*     acObject = nullptr;
    if (Acad::eOk != acdbOpenObject(acObject, *this, static_cast<AcDb::OpenMode>(mode), openErased))
        acObject = nullptr;
    return  dynamic_cast<DwgDbObjectP> (acObject);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbObjectId::OpenObject (DwgDbObjectPtr& obj, DwgDbOpenMode mode, bool openErased) const
    {
    obj._OpenObject (*this, mode, openErased);
    return  obj.OpenStatus();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbObject::IsAProxy () const { return T_Super::isAProxy(); }
bool            DwgDbObject::IsPersistent () const { return T_Super::isPersistent(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectIterator::~DwgDbObjectIterator ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_objectIterator)
        {
        delete m_objectIterator;
        m_objectIterator = nullptr;
        }
#endif
    }
bool            DwgDbObjectIterator::IsValid () const { return nullptr != m_objectIterator; }
void            DwgDbObjectIterator::Start () { if (nullptr!=m_objectIterator) m_objectIterator->start(); }
void            DwgDbObjectIterator::Next () { if (nullptr!=m_objectIterator) m_objectIterator->step(); }
bool            DwgDbObjectIterator::Done () const { return nullptr!=m_objectIterator && m_objectIterator->done(); }
DwgDbEntityP    DwgDbObjectIterator::GetEntity () { return nullptr!=m_objectIterator ? DwgDbEntity::Cast(m_objectIterator->entity()) : nullptr; }
DwgDbObjectId   DwgDbObjectIterator::GetObjectId () { return nullptr!=m_objectIterator ? m_objectIterator->objectId() : DwgDbObjectId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbDictionaryIterator::~DwgDbDictionaryIterator ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_dictionaryIterator)
        {
        delete m_dictionaryIterator;
        m_dictionaryIterator = nullptr;
        }
#endif
    }
bool            DwgDbDictionaryIterator::IsValid () const { return nullptr != m_dictionaryIterator; }
void            DwgDbDictionaryIterator::Next () { nullptr!=m_dictionaryIterator && m_dictionaryIterator->next(); }
bool            DwgDbDictionaryIterator::Done () const { return nullptr!=m_dictionaryIterator && m_dictionaryIterator->done(); }
bool            DwgDbDictionaryIterator::SetPosition (DwgDbObjectIdCR id) { return nullptr!=m_dictionaryIterator && m_dictionaryIterator->setPosition(id); }
DwgDbObjectId   DwgDbDictionaryIterator::GetObjectId () const { return nullptr!=m_dictionaryIterator ? m_dictionaryIterator->objectId() : DwgDbObjectId(); }
DwgString       DwgDbDictionaryIterator::GetName () const { return nullptr!=m_dictionaryIterator ? DwgString(m_dictionaryIterator->name()) : DwgString(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbDictionary::GetIdAt (DwgDbObjectIdR outId, DwgStringCR inName) const
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
#ifdef DWGTOOLKIT_OpenDwg
    OdResult    result;
    outId = T_Super::getAt (inName, &result);
    status = ToDwgDbStatus(result);

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::getAt (inName.c_str(), outId);
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbDictionary::GetNameAt (DwgStringR outName, DwgDbObjectIdCR inId) const
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
#ifdef DWGTOOLKIT_OpenDwg
    outName = T_Super::nameAt (inId);
    status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::nameAt (inId, outName);
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbDictionary::SetAt (DwgStringCR name, DwgDbObjectP object, DwgDbObjectIdP entryId)
    {
    DwgDbStatus     status = DwgDbStatus::NotSupported;
    DwgDbObjectId   id;
#ifdef DWGTOOLKIT_OpenDwg
    id = T_Super::setAt (name, object);
    status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::setAt (name.c_str(), object, id);
    status = ToDwgDbStatus (es);
#endif
    if (entryId != nullptr)
        *entryId = id;
    return  status;
    }
DwgDbStatus     DwgDbDictionary::SetName (DwgStringCR old, DwgStringCR newn) { ReturnBoolOrStatus(T_Super::setName(old, newn)); }
DwgDbStatus     DwgDbDictionary::Remove (DwgStringCR name) { ReturnVoidOrStatus(T_Super::remove(name)); }
DwgDbStatus     DwgDbDictionary::Remove (DwgDbObjectIdCR id) { ReturnVoidOrStatus(T_Super::remove(id)); }
bool            DwgDbDictionary::Has (DwgStringCR name) const { return T_Super::has(name); }
bool            DwgDbDictionary::Has (DwgDbObjectIdCR id) const { return T_Super::has(id); }
DwgDbDictionaryIteratorPtr DwgDbDictionary::GetIterator () const { return new DwgDbDictionaryIterator(T_Super::newIterator()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbMaterial::GetNormalMap (DwgGiMaterialMapR map, NormalMapMethod& method, double& strength) const
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
    DWGGI_Type(MaterialTraits::NormalMapMethod) tkmethod = DWGGI_Type(MaterialTraits::kTangentSpace);
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::normalMap (dynamic_cast<OdGiMaterialMap&>(map), tkmethod, strength);
    status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::normalMap (dynamic_cast<AcGiMaterialMap&>(map), tkmethod, strength);
    status = ToDwgDbStatus (es);
#endif
    method = static_cast <NormalMapMethod> (tkmethod);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgDbMaterial::GetOpacity (DwgGiMaterialMapR map) const
    {
    double  percent = 0.0;
    T_Super::opacity (percent, dynamic_cast<DWGGI_TypeR(MaterialMap)>(map));
    return  percent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgDbMaterial::GetRefraction (DwgGiMaterialMapR map) const
    {
    double  index = 0.0;
    T_Super::opacity (index, dynamic_cast<DWGGI_TypeR(MaterialMap)>(map));
    return  index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgDbMaterial::GetSpecular (DwgGiMaterialColorR color, DwgGiMaterialMapR map) const
    {
    double  gloss = 0.0;
    T_Super::specular (dynamic_cast<DWGGI_TypeR(MaterialColor)>(color), dynamic_cast<DWGGI_TypeR(MaterialMap)>(map), gloss);
    return  gloss;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbUnits      DwgDbMaterial::GetScaleUnits () const
    {
    DwgDbUnits          scaleUnits = DwgDbUnits::Undefined;
    DwgResBufIterator   iter = this->GetXData (L"ACAD");
    if (iter.IsValid())
        {
        /*-------------------------------------------------------------------------------
        1001    ACAD
        1070    int16   => index of material list
        1070    int16   => number of materials in the list
        1070    int16   => material scale units value
        1000    string
        1071    int32
        1070    int16
        -------------------------------------------------------------------------------*/
        int     count = 0;
        for (DwgResBufP curr = iter->Start(); curr != iter->End(); curr = curr->Next())
            {
            if (DwgResBuf::DataType::Integer16 == iter->GetDataType())
                {
                if (count++ > 1)
                    {
                    scaleUnits = static_cast<DwgDbUnits> (iter->GetInteger16());
                    break;
                    }
                }
            }
        }
    return  scaleUnits;
    }
DwgString       DwgDbMaterial::GetName () const { return T_Super::name(); }
DwgString       DwgDbMaterial::GetDescription () const { return T_Super::description(); }
bool            DwgDbMaterial::IsAnonymous () const { return T_Super::isAnonymous(); }
bool            DwgDbMaterial::IsTwoSided () const { return T_Super::twoSided(); }
void            DwgDbMaterial::GetAmbient (DwgGiMaterialColorR c) const { T_Super::ambient(dynamic_cast<DWGGI_TypeR(MaterialColor)>(c)); }
void            DwgDbMaterial::GetBump (DwgGiMaterialMapR m) const { T_Super::bump(dynamic_cast<DWGGI_TypeR(MaterialMap)>(m)); }
double          DwgDbMaterial::GetColorBleedScale () const { return T_Super::colorBleedScale(); }
void            DwgDbMaterial::GetDiffuse (DwgGiMaterialColorR c, DwgGiMaterialMapR m) const { T_Super::diffuse(dynamic_cast<DWGGI_TypeR(MaterialColor)>(c), dynamic_cast<DWGGI_TypeR(MaterialMap)>(m)); }
double          DwgDbMaterial::GetIndirectBumpScale () const { return T_Super::indirectBumpScale(); }
double          DwgDbMaterial::GetReflectanceScale () const { return T_Super::reflectanceScale(); }
void            DwgDbMaterial::GetReflection (DwgGiMaterialMapR m) const { T_Super::reflection(dynamic_cast<DWGGI_TypeR(MaterialMap)>(m)); }
double          DwgDbMaterial::GetReflectivity () const { return T_Super::reflectivity(); }
double          DwgDbMaterial::GetSelfIllumination () const { return T_Super::selfIllumination(); }
double          DwgDbMaterial::GetShininess () const { return T_Super::shininess(); }
double          DwgDbMaterial::GetTranslucence () const { return T_Super::translucence(); }
double          DwgDbMaterial::GetTransmittanceScale () const { return T_Super::transmittanceScale(); }
DwgDbMaterial::ChannelFlags           DwgDbMaterial::GetChannelFlags () const { return static_cast<ChannelFlags>(T_Super::channelFlags()); }
DwgDbMaterial::Mode                   DwgDbMaterial::GetMode () const { return static_cast<Mode>(T_Super::mode()); }
DwgDbMaterial::FinalGatherMode        DwgDbMaterial::GetFinalGather () const { return static_cast<FinalGatherMode>(T_Super::finalGather()); }
DwgDbMaterial::IlluminationModel      DwgDbMaterial::GetIlluminationModel () const { return static_cast<IlluminationModel>(T_Super::illuminationModel()); }
DwgDbMaterial::GlobalIlluminationMode DwgDbMaterial::GetGlobalIllumination () const { return static_cast<GlobalIlluminationMode>(T_Super::globalIllumination()); }

DwgDbObjectId   DwgDbSkyBackground::GetSunId() const { return T_Super::sunId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSun::GetSkyParameters (DwgGiSkyParametersR paramsOut) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    OdGiSkyParameters   odParams;
    T_Super::skyParameters (odParams);
    paramsOut = odParams;
#elif DWGTOOLKIT_RealDwg
    AcGiSkyParameters   acParams;
    status = ToDwgDbStatus (T_Super::skyParameters(acParams));
    if (DwgDbStatus::Success == status)
        paramsOut = acParams;
#endif
    return  status;
    }
DwgDbStatus     DwgDbSun::GetShadowParameters (DwgGiShadowParametersR paramsOut) const { paramsOut = T_Super::shadowParameters(); return DwgDbStatus::Success; }
bool            DwgDbSun::IsOn () const { return T_Super::isOn(); }
bool            DwgDbSun::IsDayLightSavingOn () const { return T_Super::isDayLightSavingsOn(); }
double          DwgDbSun::GetAltitude () const { return T_Super::altitude(); }
double          DwgDbSun::GetAzimuth () const { return T_Super::azimuth(); }
DwgDbDateCR     DwgDbSun::GetDateTime () const { return static_cast<DwgDbDateCR>(T_Super::dateTime()); }
double          DwgDbSun::GetIntensity () const { return T_Super::intensity(); }
DwgCmColorCR    DwgDbSun::GetSunColor () const { return static_cast<DwgCmColorCR>(T_Super::sunColor()); }
DVec3d          DwgDbSun::GetSunDirection () const { return Util::DVec3dFrom(T_Super::sunDirection()); }
DwgGiDrawablePtr    DwgDbSun::GetDrawable() { return new DwgGiDrawable(T_Super::drawable()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor       DwgDbSun::GetSunColorPhotometric (double scale)
    {
#ifdef DWGTOOLKIT_OpenDwg
    // Teigha does not have sunColorPhotometric!
    OdCmColor color = T_Super::sunColor ();
    return RgbFactor::From(color.red() * scale, color.green() * scale, color.blue() * scale);
#elif DWGTOOLKIT_RealDwg
    AcGiColorRGB  giRgb = T_Super::sunColorPhotometric (scale);
    return RgbFactor::From(giRgb.red, giRgb.green, giRgb.blue);
#endif
    }

DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorSkyZenith () const { return T_Super::colorSkyZenith(); }
DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorSkyHorizon () const  { return T_Super::colorSkyHorizon(); }
DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorUndergroundHorizon () const { return T_Super::colorUndergroundHorizon(); }
DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorUndergroundAzimuth () const { return T_Super::colorUndergroundAzimuth(); }
DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorGroundPlaneNear () const { return T_Super::colorGroundPlaneNear(); }
DwgCmEntityColor DwgDbGroundPlaneBackground::GetColorGroundPlaneFar () const { return T_Super::colorGroundPlaneFar(); }

DwgString  DwgDbImageBackground::GetImageFileName () const { return T_Super::imageFilename(); }
bool       DwgDbImageBackground::IsToFitScreen () const { return T_Super::fitToScreen(); }
bool       DwgDbImageBackground::IsToMaintainAspectRatio () const { return T_Super::maintainAspectRatio(); }
bool       DwgDbImageBackground::IsToUseTiling () const { return T_Super::useTiling(); }
DPoint2d   DwgDbImageBackground::GetOffset () const { return DPoint2d::From(T_Super::xOffset(),T_Super::yOffset()); }
DPoint2d   DwgDbImageBackground::GetScale () const { return DPoint2d::From(T_Super::xScale(),T_Super::yScale()); }

bool       DwgDbIBLBackground::IsEnabled () const { return T_Super::enable(); }
bool       DwgDbIBLBackground::IsImageDisplayed () const { return T_Super::displayImage(); }
DwgString  DwgDbIBLBackground::GetIBLImageName () const { return T_Super::IBLImageName(); }
double     DwgDbIBLBackground::GetRotation () const { return T_Super::rotation(); }
DwgDbObjectId  DwgDbIBLBackground::GetSecondaryBackground () const { return T_Super::secondaryBackground(); }

DwgCmEntityColor DwgDbGradientBackground::GetColorTop () const { return T_Super::colorTop(); }
DwgCmEntityColor DwgDbGradientBackground::GetColorMiddle () const { return T_Super::colorMiddle(); }
DwgCmEntityColor DwgDbGradientBackground::GetColorBottom () const { return T_Super::colorBottom(); }
double      DwgDbGradientBackground::GetHorizon () const { return T_Super::horizon(); }
double      DwgDbGradientBackground::GetHeight () const { return T_Super::height(); }
double      DwgDbGradientBackground::GetRotation () const { return T_Super::rotation(); }

DwgCmEntityColor DwgDbSolidBackground::GetColorSolid () const { return T_Super::colorSolid(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbVisualStyle::GetName () const
    {
    DwgString   name;
    T_Super::name (name);
    return  name;
    }
DwgGiVisualStyle::RenderType    DwgDbVisualStyle::GetType () const { return static_cast<DwgGiVisualStyle::RenderType>(T_Super::type()); }
DwgString       DwgDbVisualStyle::GetDescription () const { return T_Super::description(); }
bool            DwgDbVisualStyle::GetTraitFlag (DwgGiVisualStyleProperties::Property prop, DwgDbUInt32 flags) const { return T_Super::traitFlag(CASTFROMDwgGiVisProp(prop), flags); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgGiVariantCR  DwgDbVisualStyle::GetTrait (DwgGiVisualStyleProperties::Property prop, DwgGiVisualStyleOperations::Operation* op) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    static DwgGiVariant s_variantCopy(false);
    OdGiVariantPtr  odVar = T_Super::trait (CASTFROMDwgGiVisProp(prop), CASTFROMDwgGiVisOpP(op));
    if (odVar.isNull())
        BeAssert (false && "OdGiVariantPtr nullptr!");
    else
        s_variantCopy = *static_cast<DwgGiVariantP>(odVar.get());
    return  s_variantCopy;
#elif DWGTOOLKIT_RealDwg
    return static_cast<DwgGiVariantCR>(T_Super::trait(CASTFROMDwgGiVisProp(prop), CASTFROMDwgGiVisOpP(op)));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbLayout::GetName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return  T_Super::getLayoutName ();
#elif DWGTOOLKIT_RealDwg
    DwgString   layoutName;
    ACHAR*      name = nullptr;
    if (Acad::eOk == T_Super::getLayoutName(name))
        layoutName.assign (name);
    acutDelString (name);
    return  layoutName;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d        DwgDbLayout::GetExtents () const
    {
    DWGGE_Type(Point3d) min, max;
#ifdef DWGTOOLKIT_OpenDwg
    min = T_Super::getEXTMIN ();
    max = T_Super::getEXTMAX ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getExtents (min, max);
#endif
    return DRange3d::From (Util::DPoint3dFrom(min), Util::DPoint3dFrom(max));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2d        DwgDbLayout::GetLimits () const
    {
    DWGGE_Type(Point2d) min, max;
#ifdef DWGTOOLKIT_OpenDwg
    min = T_Super::getLIMMIN ();
    max = T_Super::getLIMMAX ();
#elif DWGTOOLKIT_RealDwg
    T_Super::getLimits (min, max);
#endif
    return DRange2d::From (Util::DPoint2dFrom(min), Util::DPoint2dFrom(max));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        DwgDbLayout::GetViewports (DwgDbObjectIdArrayR viewportsOut) const
    {
    DWGDB_Type(ObjectIdArray)   vports = T_Super::getViewportArray ();
    uint32_t                    nViewports = vports.length ();
    for (uint32_t i = 0; i < nViewports; i++)
        viewportsOut.push_back (vports.at(i));
    return  nViewports;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbUnits      DwgDbLayout::GetPlotPaperUnits () const
    {
    PlotPaperUnits  plotUnits = T_Super::plotPaperUnits ();
    switch (plotUnits)
        {
        case PlotPaperUnits::kInches:       return  DwgDbUnits::Inches;
        case PlotPaperUnits::kMillimeters:  return  DwgDbUnits::Millimeters;
        case PlotPaperUnits::kPixels:       return  DwgDbUnits::Undefined;
        }
    return  DwgDbUnits::Undefined;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayout::GetPlotPaperSize (DPoint2dR size) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
    double      height = 0.0, width = 0.0;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getPlotPaperSize (width, height);
    if (height == 0.0 || width == 0.0)
        status = ToDwgDbStatus (OdResult::eInvalidPlotArea);

#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getPlotPaperSize(width, height));
#endif
    size.Init (width, height);
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayout::GetPlotPaperMargins (double& left, double& bottom, double& right, double& top) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es =
#endif

    T_Super::getPlotPaperMargins (left, bottom, right, top);

#ifdef DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayout::GetPlotOrigin (DPoint2dR origin) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
    double      x = 0.0, y = 0.0;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getPlotOrigin (x, y);

#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getPlotOrigin(x, y));
#endif
    origin.Init (x, y);
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayout::GetCustomScale (double& numerator, double& denominator) const
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es =
#endif

    T_Super::getCustomPrintScale (numerator, denominator);

#ifdef DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayout::GetPaperImageOrigin (DPoint2dR origin) const
    {
    // get DXF groud codes 148 and 149 in negative so they are read the same as in ACAD:
    origin.Zero ();
#ifdef DWGTOOLKIT_OpenDwg
    origin = Util::DPoint2dFrom (T_Super::getPaperImageOrigin());
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    // need to call unpublished API acdbGetPaperImageOrigin, in acdbxx.dll.
    auto dllHandle = Util::GetOrLoadToolkitDll (L"acdb");
    if (nullptr != dllHandle)
        {
        static acdbGetPaperImageOriginFunc acdbGetPaperImageOrigin = (Acad::ErrorStatus(*)(AcDbPlotSettings*,double&,double&)) ::GetProcAddress (dllHandle, s_acdbGetPaperImageOrigin);
        if (nullptr != acdbGetPaperImageOrigin)
            {
            AcDbPlotSettings*   ps = dynamic_cast<AcDbPlotSettings*>(const_cast<DwgDbLayout*>(this));
            Acad::ErrorStatus   es = (*acdbGetPaperImageOrigin)(ps, origin.x, origin.y);
            if (Acad::eOk == es)
                {
                // negate the readout values
                origin.x = -origin.x;
                origin.y = -origin.y;
                return  ToDwgDbStatus(es);
                }
            }
        else
            {
            BeAssert (false && "Unpublished function acdbGetPaperImageOrigin not found in acdbxx.dll!");
            }
        }
    return  DwgDbStatus::UnknownError;
#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgDbLayout::GetStandardScale () const
    {
    double scale = 1.0;
    T_Super::getStdScale (scale);
    return scale;
    }
int32_t         DwgDbLayout::GetTabOrder () const { return T_Super::getTabOrder(); }
bool            DwgDbLayout::IsAnnoAllVisible() const { return T_Super::annoAllVisible(); }
DwgDbObjectId   DwgDbLayout::GetBlockTableRecordId () const { return T_Super::getBlockTableRecordId(); }
bool            DwgDbLayout::IsStandardScale () const { return T_Super::useStandardScale(); }
double          DwgDbLayout::GetCustomScale () const { double n=1.0,d=1.0; T_Super::getCustomPrintScale(n,d); return d!=0.0 ? n/d : 1.0; }
DwgDbLayout::PaperOrientation DwgDbLayout::GetPaperOrientation () const { return static_cast<PaperOrientation>(T_Super::plotRotation()); }
DwgDbLayout::PlotBy DwgDbLayout::GetPlotBy () const { return DWGDB_UPWARDCAST(Layout::PlotBy)(T_Super::plotType()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId  DwgDbLayoutManager::FindLayoutByName (DwgStringCR name, DwgDbDatabaseCP dwg) const
    {
    DwgDbObjectId   layoutId;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        layoutId = m_layoutManager->findLayoutNamed (dwg, name);
#elif DWGTOOLKIT_RealDwg
    #if VendorVersion > 2017
        layoutId = m_layoutManager->findLayoutNamed (name.c_str(), dwg);
    #else
        AcDbLayout* layout = m_layoutManager->findLayoutNamed (name.c_str(), false, dwg);
        if (nullptr != layout)
            layoutId = layout->objectId ();
    #endif
#endif
        }
    return  layoutId;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId  DwgDbLayoutManager::FindActiveLayout (DwgDbDatabaseCP dwg) const
    {
    DwgDbObjectId   layoutId;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        layoutId = m_layoutManager->findLayoutNamed (dwg, m_layoutManager->findActiveLayout(dwg, true));
#elif DWGTOOLKIT_RealDwg
    #if VendorVersion > 2017
        layoutId = m_layoutManager->findLayoutNamed (m_layoutManager->findActiveLayout(true, dwg), dwg);
    #else
        AcDbLayout* layout = m_layoutManager->findLayoutNamed (m_layoutManager->findActiveLayout(true, dwg), false, dwg);
        if (nullptr != layout)
            layoutId = layout->objectId ();
    #endif
#endif
        }
    return  layoutId;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbLayoutManager::ActivateLayout (DwgDbObjectId layoutId)
    {
    DwgDbStatus status = DwgDbStatus::InvalidData;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        status = DwgDbStatus::InvalidInput;
        if (layoutId.isValid())
            {
            m_layoutManager->setCurrentLayout (layoutId.database(), layoutId);
            status = DwgDbStatus::Success;
            }
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (m_layoutManager->setCurrentLayoutId(layoutId));
#endif
        }
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbLayoutManager::CreateLayout (DwgDbObjectIdR layoutId, DwgDbObjectIdR blockId, DwgStringCR name, DwgDbDatabaseP dwg)
    {
    DwgDbStatus status = DwgDbStatus::InvalidData;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        layoutId = m_layoutManager->createLayout(dwg, name, &blockId);
        status = layoutId.isValid() ? DwgDbStatus::Success : DwgDbStatus::NotPersistentObject;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (m_layoutManager->createLayout(name.c_str(), layoutId, blockId, dwg));
#endif
        }
    return  status;
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbLayoutManager::DeleteLayout (DwgStringCR name, DwgDbDatabaseP dwg)
    {
    DwgDbStatus status = DwgDbStatus::InvalidData;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        status = DwgDbStatus::InvalidInput;
        if (!name.isEmpty() && nullptr != dwg)
            {
            m_layoutManager->deleteLayout (dwg, name);
            status = DwgDbStatus::Success;
            }
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (m_layoutManager->deleteLayout(name.c_str(), dwg));
#endif
        }
    return  status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus    DwgDbLayoutManager::RenameLayout (DwgStringCR oldName, DwgStringCR newName, DwgDbDatabaseP dwg)
    {
    DwgDbStatus status = DwgDbStatus::InvalidData;
    if (this->IsValid())
        {
#ifdef DWGTOOLKIT_OpenDwg
        status = DwgDbStatus::InvalidInput;
        if (!oldName.isEmpty() && !newName.isEmpty() && nullptr != dwg)
            {
            m_layoutManager->renameLayout (dwg, oldName, newName);
            status = DwgDbStatus::Success;
            }
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (m_layoutManager->renameLayout(oldName.c_str(), newName.c_str(), dwg));
#endif
        }
    return  status;
    }
DwgDbLayoutManager::DwgDbLayoutManager (DWGDB_TypeP(LayoutManager) manager) : m_layoutManager(manager) {}
bool DwgDbLayoutManager::IsValid () const { return nullptr != m_layoutManager; }
int  DwgDbLayoutManager::CountLayouts (DwgDbDatabaseP dwg) const { if (IsValid()) return m_layoutManager->countLayouts(dwg); else return -1; }
DwgDbObjectId  DwgDbLayoutManager::GetActiveLayoutBlock (DwgDbDatabaseCP dwg) const { if (IsValid()) return m_layoutManager->getActiveLayoutBTRId(dwg); else return DwgDbObjectId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbFilteredBlockIterator::~DwgDbFilteredBlockIterator ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_filteredBlockIterator)
        {
        delete m_filteredBlockIterator;
        m_filteredBlockIterator = nullptr;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbFilteredBlockIterator::Start ()
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;

    if (nullptr != m_filteredBlockIterator)
        {
#ifdef DWGTOOLKIT_OpenDwg
        m_filteredBlockIterator->start ();
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (m_filteredBlockIterator->start());
#endif
        }
    return status;
    }

DwgDbObjectId   DwgDbFilteredBlockIterator::Next () { return nullptr != m_filteredBlockIterator ? m_filteredBlockIterator->next() : DwgDbObjectId(); }
bool            DwgDbFilteredBlockIterator::Done () const { return nullptr != m_filteredBlockIterator ? m_filteredBlockIterator->id().isNull() : true; }
DwgDbStatus     DwgDbFilteredBlockIterator::Seek (DwgDbObjectId id) { return nullptr != m_filteredBlockIterator ? ToDwgDbStatus(m_filteredBlockIterator->seek(id)) : DwgDbStatus::InvalidData; }
DwgDbObjectId   DwgDbFilteredBlockIterator::GetEntityId () const { return nullptr != m_filteredBlockIterator ? m_filteredBlockIterator->id() : DwgDbObjectId(); }
bool            DwgDbFilteredBlockIterator::IsValid () const { return nullptr != m_filteredBlockIterator ? m_filteredBlockIterator->id().isValid() : false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbFilteredBlockIteratorPtr DwgDbSpatialIndex::NewIterator (DwgDbSpatialFilterCP filter) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return new DwgDbFilteredBlockIterator(T_Super::newIterator(filter).get());
#elif DWGTOOLKIT_RealDwg
    return new DwgDbFilteredBlockIterator(T_Super::newIterator(filter));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSpatialFilter::GetDefinition (DPoint2dArrayR points, DVec3dR normal, double& elevation, double& frontClip, double& backClip, bool& enabled) const
    {
    DWGGE_Type(Point2dArray)    gePoints;
    DWGGE_Type(Vector3d)        zAxis;
    DwgDbStatus                 status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getDefinition (gePoints, zAxis, elevation, frontClip, backClip, enabled);
    if (!T_Super::frontClipEnabled())
        frontClip = 0.0;
    if (!T_Super::backClipDistEnabled())
        backClip = 0.0;

#elif DWGTOOLKIT_RealDwg

    Acad::ErrorStatus   es = T_Super::getDefinition (gePoints, zAxis, elevation, frontClip, backClip, enabled);
    status = ToDwgDbStatus (es);
#endif

    if (DwgDbStatus::Success == status)
        {
        Util::GetPointArray (points, gePoints);
        normal = Util::DVec3dFrom (zAxis);
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSpatialFilter::SetDefinition (DPoint2dArrayCR points, DVec3dCR normal, double elevation, double frontClip, double backClip, bool enabled)
    {
    DWGGE_Type(Vector3d)        zAxis = Util::GeVector3dFrom (normal);
    DWGGE_Type(Point2dArray)    gePoints;
    DwgDbStatus                 status = DwgDbStatus::Success;

    Util::GetGePointArray (gePoints, points);

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setDefinition (gePoints, zAxis, elevation, frontClip, backClip, (DwgDbBool)enabled);
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::setDefinition (gePoints, zAxis, elevation, frontClip, backClip, (DwgDbBool)enabled);
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSpatialFilter::GetVolume (DPoint3dR from, DPoint3dR to, DVec3dR up, DVec2dR viewField)  const
    {
    DWGGE_Type(Point3d)     point1, point2;
    DWGGE_Type(Vector3d)    upDir;
    DWGGE_Type(Vector2d)    vDir;
    DwgDbStatus             status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getVolume (point1, point2, upDir, vDir);
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::getVolume (point1, point2, upDir, vDir);
    status = ToDwgDbStatus (es);
#endif

    if (DwgDbStatus::Success == status)
        {
        from = Util::DPoint3dFrom (point1);
        to = Util::DPoint3dFrom (point2);
        up = Util::DVec3dFrom (upDir);
        viewField = Util::DVec2dFrom (vDir);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TransformR      DwgDbSpatialFilter::GetClipTransform (TransformR transform) const
    {
    DWGGE_Type(Matrix3d)    matrix;
    T_Super::getClipSpaceToWCSMatrix (matrix);
    Util::GetTransform (transform, matrix);
    return  transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TransformR      DwgDbSpatialFilter::GetBlockTransform (TransformR transform) const
    {
    DWGGE_Type(Matrix3d)    matrix;
    T_Super::getOriginalInverseBlockXform (matrix);
    Util::GetTransform (transform, matrix);
    return  transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbSpatialFilter::IsEntityFilteredOut (DwgDbEntityCR entity) const
    {
    DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents)    extents;
    if (DwgDbStatus::Success == ToDwgDbStatus(entity.getGeomExtents(extents)))
        return  !T_Super::clipVolumeIntersectsExtents (extents);
    return  false;
    }

bool            DwgDbSpatialFilter::IsInverted () const { return DWGDB_CALLSDKMETHOD(false, T_Super::isInverted()); }
DwgDbStatus     DwgDbSpatialFilter::SetInverted (bool inverted) { return DWGDB_CALLSDKMETHOD(DwgDbStatus::NotSupported, ToDwgDbStatus(T_Super::setInverted(inverted))); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbSortentsTable::GetFullDrawOrder (DwgDbObjectIdArrayR sortedOut, bool checkSORTENTS) const
    {
    DWGDB_Type(ObjectIdArray)   ids;
    DwgDbStatus                 status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getFullDrawOrder (ids, checkSORTENTS);

#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = T_Super::getFullDrawOrder (ids, checkSORTENTS);
    status = ToDwgDbStatus (es);
#endif

    if (DwgDbStatus::Success == status)
        {
        for (uint32_t i = 0; i < (uint32_t)ids.length(); i++)
            sortedOut.push_back (ids[i]);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgResBufIterator DwgDbXrecord::GetRbChain (DwgDbDatabaseP dwg, DwgDbStatus* status) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdResult    es = OdResult::eNotApplicable;
    OdResBufPtr resbuf = T_Super::rbChain (dwg, &es);
    if (status != nullptr)
        *status = ToDwgDbStatus (es);
    return  DwgResBufIterator::CreateFrom(resbuf.get());
    
#elif DWGTOOLKIT_RealDwg
    struct resbuf*  resbuf = nullptr;
    Acad::ErrorStatus   es = T_Super::rbChain(&resbuf, dwg);
    if (status != nullptr)
        *status = ToDwgDbStatus (es);
    return  DwgResBufIterator::CreateFromAndFree(resbuf);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbXrefGraphNode::DwgDbXrefGraphNode (WCharCP name, DwgDbObjectIdCR id, DwgDbDatabaseP dwg, DwgDbXrefStatus status) { DWGDB_CALLSDKMETHOD(, T_Super(name, id, dwg, DWGDB_CASTTOENUM_DB(XrefStatus)(status))); }
DwgString      DwgDbXrefGraphNode::GetName () const { return T_Super::name(); }
DwgDbObjectId  DwgDbXrefGraphNode::GetBlockId () const { return DWGDB_CALLSDKMETHOD(T_Super::blockId(), T_Super::btrId()); }
DwgDbDatabaseP DwgDbXrefGraphNode::GetDatabase () const { return static_cast<DwgDbDatabaseP>(T_Super::database()); }
bool    DwgDbXrefGraphNode::IsNested () const { return T_Super::isNested(); }
void    DwgDbXrefGraphNode::SetName (DwgStringCR name) { T_Super::setName(name.c_str()); }
void    DwgDbXrefGraphNode::SetBlockId (DwgDbObjectIdCR id) { DWGDB_CALLSDKMETHOD(T_Super::setBlockId(id),T_Super::setBtrId(id)); }
void    DwgDbXrefGraphNode::SetDatabase (DwgDbDatabaseP dwg) { T_Super::setDatabase(dwg); }
int     DwgDbXrefGraphNode::GetNumIncoming () const { return T_Super::numIn(); }
int     DwgDbXrefGraphNode::GetNumOutgoing () const { return T_Super::numOut(); }
int     DwgDbXrefGraphNode::GetNumCycleIn () const { return T_Super::numCycleIn(); }
int     DwgDbXrefGraphNode::GetNumCycleOut () const { return T_Super::numCycleOut(); }
bool    DwgDbXrefGraphNode::IsCycleNode () const { return T_Super::isCycleNode(); }
DwgDbXrefStatus     DwgDbXrefGraphNode::GetXrefStatus () const { return DWGDB_UPWARDCAST(XrefStatus)(T_Super::xrefStatus()); }
DwgDbXrefGraphNodeP DwgDbXrefGraphNode::GetIncomingNode (int index) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::in(index)); }
DwgDbXrefGraphNodeP DwgDbXrefGraphNode::GetOutgoingNode (int index) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::out(index)); }
DwgDbXrefGraphNodeP DwgDbXrefGraphNode::GetCycleIn (int index) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::cycleIn(index)); }
DwgDbXrefGraphNodeP DwgDbXrefGraphNode::GetCycleOut (int index) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::cycleOut(index)); }
DwgDbXrefGraphNodeP DwgDbXrefGraphNode::GetNextCycleNode () const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::nextCycleNode()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgDbXrefGraph::Build (DwgDbXrefGraphR graphOut, DwgDbDatabaseP hostDwg, bool includeGhosts)
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    OdDbXrefGraph::getFrom (hostDwg, graphOut, includeGhosts);
#elif DWGTOOLKIT_RealDwg
    Acad::ErrorStatus   es = acdbGetHostDwgXrefGraph (hostDwg, graphOut, includeGhosts);
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }
DwgDbXrefGraphNodeP DwgDbXrefGraph::GetXrefNode (DwgStringCR name) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::xrefNode(name.c_str())); }
DwgDbXrefGraphNodeP DwgDbXrefGraph::GetXrefNode (DwgDbObjectIdCR blockId) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::xrefNode(blockId)); }
DwgDbXrefGraphNodeP DwgDbXrefGraph::GetXrefNode (DwgDbDatabaseP dwg) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::xrefNode(dwg)); }
DwgDbXrefGraphNodeP DwgDbXrefGraph::GetXrefNode (int index) const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::xrefNode(index)); }
DwgDbXrefGraphNodeP DwgDbXrefGraph::GetHostDwg () const { return static_cast<DwgDbXrefGraphNodeP>(T_Super::hostDwg()); }
bool    DwgDbXrefGraph::IsEmpty () const { return T_Super::isEmpty(); }
size_t  DwgDbXrefGraph::GetNodeCount () const { return T_Super::numNodes(); }
bool    DwgDbXrefGraph::MarkUnresolvedTrees () { return T_Super::markUnresolvedTrees(); }
bool    DwgDbXrefGraph::FindCycles (DwgDbXrefGraphNodeP start) { return T_Super::findCycles(start); }
void    DwgDbXrefGraph::Reset () { T_Super::reset(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbGroupIterator::~DwgDbGroupIterator ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_groupIterator)
        {
        delete m_groupIterator;
        m_groupIterator = nullptr;
        }
#endif
    }
bool            DwgDbGroupIterator::IsValid () const { return nullptr != m_groupIterator; }
void            DwgDbGroupIterator::Next () { nullptr!=m_groupIterator && m_groupIterator->next(); }
bool            DwgDbGroupIterator::Done () const { return nullptr!=m_groupIterator && m_groupIterator->done(); }
DwgDbObjectId   DwgDbGroupIterator::GetObjectId () const { return nullptr!=m_groupIterator ? m_groupIterator->objectId() : DwgDbObjectId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::Clear ()
    {
    DwgDbStatus status = DwgDbStatus::Success;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::clear ();
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::clear());
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DwgDbGroup::GetAllEntityIds (DwgDbObjectIdArrayR idsOut) const
    {
    DWGDB_Type(ObjectIdArray)   ids;
    auto count = T_Super::allEntityIds (ids);
    for (uint32_t i = 0; i < count; i++)
        idsOut.push_back (ids.at(i));
    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::Append (DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_Type(ObjectIdArray)   ids;
    if (Util::GetObjectIdArray(ids, idsIn) > 0)
        {
#ifdef DWGTOOLKIT_OpenDwg
        T_Super::append (ids);
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (T_Super::append(ids));
#endif
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::Prepend (DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_Type(ObjectIdArray) ids;
    if (Util::GetObjectIdArray(ids, idsIn) > 0)
        {
#ifdef DWGTOOLKIT_OpenDwg
        T_Super::prepend (ids);
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (T_Super::prepend(ids));
#endif
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::InsertAt (size_t at, DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_Type(ObjectIdArray) ids;
    if (Util::GetObjectIdArray(ids, idsIn) > 0)
        {
#ifdef DWGTOOLKIT_OpenDwg
        T_Super::insertAt (static_cast<OdInt32>(at), ids);
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (T_Super::insertAt(static_cast<Adesk::UInt32>(at), ids));
#endif
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::Remove (DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_Type(ObjectIdArray) ids;
    if (Util::GetObjectIdArray(ids, idsIn) > 0)
        {
#ifdef DWGTOOLKIT_OpenDwg
        T_Super::remove (ids);
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (T_Super::remove(ids));
#endif
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::RemoveAt (size_t at, DwgDbObjectIdArrayCR idsIn)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
    DWGDB_Type(ObjectIdArray) ids;
    if (Util::GetObjectIdArray(ids, idsIn) > 0)
        {
#ifdef DWGTOOLKIT_OpenDwg
        T_Super::removeAt (static_cast<OdUInt32>(at), ids);
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
        status = ToDwgDbStatus (T_Super::removeAt(static_cast<Adesk::UInt32>(at), ids));
#endif
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus   DwgDbGroup::SetVisibility (DwgDbVisibility visibility)
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setVisibility (static_cast<OdDb::Visibility>(visibility));
    status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::setVisibility(static_cast<AcDb::Visibility>(visibility)));
#endif
    return  status;
    }

DwgDbGroupIteratorPtr DwgDbGroup::GetIterator () { return new DwgDbGroupIterator(T_Super::newIterator()); }
DwgString     DwgDbGroup::GetName () const { return T_Super::name(); }
DwgString     DwgDbGroup::GetDescription () const { return T_Super::description(); }
bool          DwgDbGroup::IsAnonymous () const { return T_Super::isAnonymous(); }
bool          DwgDbGroup::IsSelectable () const { return T_Super::isSelectable(); }
bool          DwgDbGroup::IsNotAccessible () const { return T_Super::isNotAccessible(); }
bool          DwgDbGroup::Has (DwgDbEntityCP entity) const { return T_Super::has(entity); }
size_t        DwgDbGroup::GetNumEntities () const { return T_Super::numEntities(); }
DwgDbStatus   DwgDbGroup::GetIndex (DwgDbObjectIdCR id, size_t& out) const { ReturnVoidOrStatus(T_Super::getIndex(id, (DwgDbUInt32R)out)); }
DwgDbStatus   DwgDbGroup::Append (DwgDbObjectIdCR id) { ReturnVoidOrStatus(T_Super::append(id)); }
DwgDbStatus   DwgDbGroup::Prepend (DwgDbObjectIdCR id) { ReturnVoidOrStatus(T_Super::prepend(id)); }
DwgDbStatus   DwgDbGroup::InsertAt (size_t at, DwgDbObjectIdCR id) { ReturnVoidOrStatus(T_Super::insertAt((DwgDbUInt32)at, id)); }
DwgDbStatus   DwgDbGroup::Remove (DwgDbObjectIdCR id) { ReturnVoidOrStatus(T_Super::remove(id)); }
DwgDbStatus   DwgDbGroup::RemoveAt (size_t at) { ReturnVoidOrStatus(T_Super::removeAt((DwgDbUInt32)at)); }
DwgDbStatus   DwgDbGroup::Replace (DwgDbObjectIdCR oldId, DwgDbObjectIdCR newId) { ReturnVoidOrStatus(T_Super::replace(oldId, newId)); }
DwgDbStatus   DwgDbGroup::Reverse () { ReturnVoidOrStatus(T_Super::reverse()); }
DwgDbStatus   DwgDbGroup::SetName (DwgStringCR name) { ReturnVoidOrStatus(T_Super::setName(name.c_str())); }
DwgDbStatus   DwgDbGroup::SetDescription (DwgStringCR descr) { ReturnVoidOrStatus(T_Super::setDescription(descr.c_str())); }
DwgDbStatus   DwgDbGroup::SetColor (DwgCmColorCR color) { ReturnVoidOrStatus(T_Super::setColor(color)); }
DwgDbStatus   DwgDbGroup::SetColorIndex (uint16_t color) { ReturnVoidOrStatus(T_Super::setColorIndex(color)); }
DwgDbStatus   DwgDbGroup::SetLayer (DwgDbObjectIdCR layerId) { ReturnVoidOrStatus(T_Super::setLayer(layerId)); }
DwgDbStatus   DwgDbGroup::SetLinetype (DwgDbObjectIdCR linetypeId) { ReturnVoidOrStatus(T_Super::setLinetype(linetypeId)); }
DwgDbStatus   DwgDbGroup::SetLinetypeScale (double scale) { ReturnVoidOrStatus(T_Super::setLinetypeScale(scale)); }
DwgDbStatus   DwgDbGroup::SetMaterial (DwgDbObjectIdCR materialId) { ReturnVoidOrStatus(T_Super::setMaterial(materialId)); }
DwgDbStatus   DwgDbGroup::SetSelectable (bool selectable) { ReturnVoidOrStatus(T_Super::setSelectable(selectable)); }
DwgDbStatus   DwgDbGroup::Transfer (size_t from, size_t to, size_t num) { ReturnVoidOrStatus(T_Super::transfer((DwgDbUInt32)from, (DwgDbUInt32)to, (DwgDbUInt32)num)); }
