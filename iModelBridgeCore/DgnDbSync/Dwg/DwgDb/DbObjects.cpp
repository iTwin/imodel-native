/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DbObjects.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

DWGDB_ENTITY_DEFINE_MEMBERS (Object)
DWGDB_ENTITY_DEFINE_MEMBERS (Dictionary)
DWGDB_ENTITY_DEFINE_MEMBERS (Material)
DWGDB_ENTITY_DEFINE_MEMBERS (VisualStyle)
DWGDB_ENTITY_DEFINE_MEMBERS (Layout)
DWGDB_ENTITY_DEFINE_MEMBERS (SpatialFilter)
DWGDB_ENTITY_DEFINE_MEMBERS (SpatialIndex)
DWGDB_ENTITY_DEFINE_MEMBERS (SortentsTable)


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
DwgString       DwgDbObjectId::GetClassName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectPtr   obj = T_Super::safeOpenObject ();
    if (obj.isNull())
        return  L"NullObject";
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
bool            DwgDbObjectId::IsObjectDerivedFrom (DWG_TypeCP(RxClass) rxClass) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectPtr   obj = T_Super::safeOpenObject ();
    if (obj.isNull())
        return  false;
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
        delete m_objectIterator;
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
        delete m_dictionaryIterator;
#endif
    }
bool            DwgDbDictionaryIterator::IsValid () const { return nullptr != m_dictionaryIterator; }
void            DwgDbDictionaryIterator::Next () { nullptr!=m_dictionaryIterator && m_dictionaryIterator->next(); }
bool            DwgDbDictionaryIterator::Done () const { return nullptr!=m_dictionaryIterator && m_dictionaryIterator->done(); }
bool            DwgDbDictionaryIterator::SetPosition (DwgDbObjectIdCR id) { return nullptr!=m_dictionaryIterator && m_dictionaryIterator->setPosition(id); }
DwgDbObjectId   DwgDbDictionaryIterator::GetObjectId () const { return nullptr!=m_dictionaryIterator ? m_dictionaryIterator->objectId() : DwgDbObjectId(); }
DwgString       DwgDbDictionaryIterator::GetName () const { return nullptr!=m_dictionaryIterator ? m_dictionaryIterator->name() : DwgString(); }

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
bool            DwgDbDictionary::Has (DwgStringCR name) const { return T_Super::has(name); }
bool            DwgDbDictionary::Has (DwgDbObjectIdCR id) const { return T_Super::has(id); }
DwgDbDictionaryIterator DwgDbDictionary::GetIterator () const { return DwgDbDictionaryIterator(T_Super::newIterator()); }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbVisualStyle::GetName () const
    {
    DwgString   name;
    T_Super::name (name);
    return  name;
    }

DwgDbVisualStyleType    DwgDbVisualStyle::GetType () const { return static_cast<DwgDbVisualStyleType>(T_Super::type()); }
DwgString               DwgDbVisualStyle::GetDescription () const { return T_Super::description(); }

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
    if (x = 0.0 || y == 0.0)
        status = ToDwgDbStatus (OdResult::eInvalidPlotArea);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbFilteredBlockIterator::~DwgDbFilteredBlockIterator ()
    {
#ifdef DWGTOOLKIT_RealDwg
    if (nullptr != m_filteredBlockIterator)
        delete m_filteredBlockIterator;
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
DwgDbFilteredBlockIterator  DwgDbSpatialIndex::NewIterator (DwgDbSpatialFilterCP filter) const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return T_Super::newIterator(filter).get();
#elif DWGTOOLKIT_RealDwg
    return T_Super::newIterator(filter);
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
