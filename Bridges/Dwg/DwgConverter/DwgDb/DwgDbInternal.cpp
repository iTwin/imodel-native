/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// quiet static analysis warnings through toolkit's header files
#pragma warning(disable:6001)
#pragma warning(disable:6011)
#pragma warning(disable:6244)
#pragma warning(disable:6387)

#include "DwgDbInternal.h"

#pragma warning(default:6001)
#pragma warning(default:6011)
#pragma warning(default:6244)
#pragma warning(default:6387)

/*---------------------------------------------------------------------------------------
    Enable Windows Cryptographic API for OpenDWG
---------------------------------------------------------------------------------------*/
#if defined (DWGTOOLKIT_OpenDwg)
    #if (defined (WINNT) || defined(WIN32)) && (DWGDB_ToolkitMajorRelease < 19)
        #include <Teigha/Kernel/Extensions/win/Crypt/WinNTCrypt.cpp>
    #endif
    
    static OdRxModule*          s_gripPointsModule;
#endif

USING_NAMESPACE_DWGDB

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegisterDwgDbObjectExtensions (bool beforeValidation)
    {
    if (beforeValidation)
        {
        /*--------------------------------------------------------------------------------------------
        Apparently RealDWG does not allow some classes to be sub-classed. But our sub-classing is only
        a wrapper that calls super class and has no data memebers, there should be no reason why it is
        not allowed.  Further investigation may needed but for now we workaround that by init these
        classes prior to acdbValidateSetup().  All other classes still must be initialized after that
        call.
        --------------------------------------------------------------------------------------------*/
        DwgDb2dPolyline::rxInit ();
        DwgDb3dPolyline::rxInit ();
        DwgDbPolyFaceMesh::rxInit ();
        DwgDbPolyFaceMeshVertex::rxInit ();
        DwgDbPolygonMesh::rxInit ();
        DwgDbPolygonMeshVertex::rxInit ();
        DwgDbFaceRecord::rxInit ();
        DwgDbViewport::rxInit ();
        return;
        }

    DwgDbObject::rxInit ();
    DwgDbEntity::rxInit ();
    DwgDbLine::rxInit ();
    DwgDbPolyline::rxInit ();
    DwgDbArc::rxInit ();
    DwgDbCircle::rxInit ();
    DwgDbEllipse::rxInit ();
    DwgDbFace::rxInit ();
    DwgDbAttribute::rxInit ();
    DwgDbAttributeDefinition::rxInit ();
    DwgDbBlockReference::rxInit ();
    DwgDbHatch::rxInit ();
    DwgDbRegion::rxInit ();
    DwgDbPoint::rxInit ();
    DwgDbShape::rxInit ();
    DwgDbSolid::rxInit ();
    DwgDbSpline::rxInit ();
    DwgDbMText::rxInit ();
    DwgDbText::rxInit ();
    DwgDbTrace::rxInit ();
    DwgDbExtrudedSurface::rxInit ();
    DwgDbLoftedSurface::rxInit ();
    DwgDbNurbSurface::rxInit ();
    DwgDbPlaneSurface::rxInit ();
    DwgDbRevolvedSurface::rxInit ();
    DwgDbSurface::rxInit ();
    DwgDbSweptSurface::rxInit ();

    DwgDbSymbolTable::rxInit ();
    DwgDbSymbolTableRecord::rxInit ();
    DwgDbBlockTable::rxInit ();
    DwgDbBlockTableRecord::rxInit ();
    DwgDbLayerTable::rxInit ();
    DwgDbLayerTableRecord::rxInit ();
    DwgDbLinetypeTable::rxInit ();
    DwgDbLinetypeTableRecord::rxInit ();
    DwgDbViewportTable::rxInit ();
    DwgDbViewportTableRecord::rxInit ();
    DwgDbTextStyleTable::rxInit ();
    DwgDbTextStyleTableRecord::rxInit ();

    DwgDbDictionary::rxInit ();
    DwgDbMaterial::rxInit ();
    DwgDbLayout::rxInit ();
    DwgDbGroup::rxInit ();
    DwgDbVisualStyle::rxInit ();
    DwgDbSpatialFilter::rxInit ();
    DwgDbSpatialIndex::rxInit ();
    DwgDbSortentsTable::rxInit ();
    DwgDbXrecord::rxInit ();

    DwgRxObject::rxInit ();

#ifdef DWGTOOLKIT_OpenDwg

    DwgResBuf::rxInit ();
    DwgDbSun::rxInit ();
    DwgDbSkyBackground::rxInit ();
    DwgDbGradientBackground::rxInit ();
    DwgDbGroundPlaneBackground::rxInit ();
    DwgDbIBLBackground::rxInit ();
    DwgDbImageBackground::rxInit ();
    DwgDbSolidBackground::rxInit ();

    // register private DwgGi class extensions
    RegisterDwgGiExtensions ();

    // get and append ApiNumber for our OdGripPoints module:
    OdString    moduleName("OdGripPoints");
    moduleName += OdString(DLM_API_NUMBER) + OdString(L".dll");

    s_gripPointsModule = OdRxSystemServices::loadModuleLib (moduleName, true);
    if (nullptr != s_gripPointsModule)
        s_gripPointsModule->initApp ();
    else
        std::wcout << "Alert: " << "failed loading OdGripPointsModule!" << std::endl;

    /*-----------------------------------------------------------------------------------
    Based on ODA's suggestion we opt to explicitly load AcDbPointCloudObj.  As a result
    no need to call OdDbPointCloudEx::rxInit()
    -----------------------------------------------------------------------------------*/
    OdRxModulePtr pointCloudModule = ::odrxDynamicLinker()->loadModule ("AcDbPointCloudObj");

#elif DWGTOOLKIT_RealDwg

    acrxRegisterService (NAME_DwgProtocolExtension);
    // load point cloud DBX for DwgDbPointCloudEx
    acrxLoadModule (L"AcDbPointCloudObj.dbx", 0);
#if VendorVersion == 2017
    // demand loading image OE resulted in a RealDWG2017 crash - TFS 615432
    acrxLoadModule (L"acISMobj21.dbx", 0);
#endif
    // load light DBX for DwgDbLight
    acrxLoadModule (L"AcSceneOE.dbx", 0);
    // load model doc DBX for DwgDbViewBorder
    acrxLoadModule (L"AcModelDocObj.dbx", 0);

    AcDbRasterImage::rxInit ();
    AcDbRasterImageDef::rxInit ();
    DwgDbViewRepBlockReference::rxInit ();
    DwgDbViewBorder::rxInit ();

#endif  // DWGTOOLKIT_

    // post initialize classes after their object enablers have been loaded:
    DwgDbRasterImage::rxInit ();
    DwgDbPointCloudEx::rxInit ();
    DwgDbLight::rxInit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnRegisterDwgDbObjectExtensions ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    if (nullptr != s_gripPointsModule)
        {
        s_gripPointsModule->uninitApp ();
        s_gripPointsModule = nullptr;
        }

    DwgDbObject::rxUninit ();
    DwgDbEntity::rxUninit ();
    DwgDbLine::rxUninit ();
    DwgDbPolyline::rxUninit ();
    DwgDb2dPolyline::rxUninit ();
    DwgDb3dPolyline::rxUninit ();
    DwgDbPolyFaceMesh::rxUninit ();
    DwgDbPolyFaceMeshVertex::rxUninit ();
    DwgDbPolygonMesh::rxUninit ();
    DwgDbPolygonMeshVertex::rxUninit ();
    DwgDbArc::rxUninit ();
    DwgDbCircle::rxUninit ();
    DwgDbEllipse::rxUninit ();
    DwgDbFace::rxUninit ();
    DwgDbFaceRecord::rxUninit ();
    DwgDbAttributeDefinition::rxUninit ();
    DwgDbAttribute::rxUninit ();
    DwgDbViewport::rxUninit ();
    DwgDbBlockReference::rxUninit ();
    DwgDbHatch::rxUninit ();
    DwgDbRasterImage::rxUninit ();
    DwgDbRegion::rxUninit ();
    DwgDbPoint::rxUninit ();
    DwgDbShape::rxUninit ();
    DwgDbSolid::rxUninit ();
    DwgDbSpline::rxUninit ();
    DwgDbMText::rxUninit ();
    DwgDbText::rxUninit ();
    DwgDbTrace::rxUninit ();
    DwgDbExtrudedSurface::rxUninit ();
    DwgDbLoftedSurface::rxUninit ();
    DwgDbNurbSurface::rxUninit ();
    DwgDbPlaneSurface::rxUninit ();
    DwgDbRevolvedSurface::rxUninit ();
    DwgDbSurface::rxUninit ();
    DwgDbSweptSurface::rxUninit ();

    DwgDbSymbolTable::rxUninit ();
    DwgDbSymbolTableRecord::rxUninit ();
    DwgDbBlockTable::rxUninit ();
    DwgDbBlockTableRecord::rxUninit ();
    DwgDbLayerTable::rxUninit ();
    DwgDbLayerTableRecord::rxUninit ();
    DwgDbLinetypeTable::rxUninit ();
    DwgDbLinetypeTableRecord::rxUninit ();
    DwgDbViewportTable::rxUninit ();
    DwgDbViewportTableRecord::rxUninit ();
    DwgDbTextStyleTable::rxUninit ();
    DwgDbTextStyleTableRecord::rxUninit ();

    DwgDbDictionary::rxUninit ();
    DwgDbMaterial::rxUninit ();
    DwgDbLayout::rxUninit ();
    DwgDbGroup::rxUninit ();
    DwgDbVisualStyle::rxUninit ();
    DwgDbSpatialFilter::rxUninit ();
    DwgDbSpatialIndex::rxUninit ();
    DwgDbSortentsTable::rxUninit ();
    DwgDbXrecord::rxUninit ();

    DwgResBuf::rxUninit ();

    DwgDbPointCloudEx::rxUninit ();
    DwgDbLight::rxUninit ();
    DwgDbSun::rxUninit ();
    DwgDbSkyBackground::rxUninit ();
    DwgDbGradientBackground::rxUninit ();
    DwgDbGroundPlaneBackground::rxUninit ();
    DwgDbIBLBackground::rxUninit ();
    DwgDbImageBackground::rxUninit ();
    DwgDbSolidBackground::rxUninit ();

    // Un-register private DwgGi class extensions
    UnRegisterDwgGiExtensions ();

#elif DWGTOOLKIT_RealDwg

    auto protocalExts = acrxServiceDictionary->remove (NAME_DwgProtocolExtension);
    if (protocalExts != nullptr)
        delete protocalExts;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* DwgDbDxfFiler implimentation
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef DWGTOOLKIT_OpenDwg
OdDbFiler::FilerType    DwgDbDxfFiler::filerType () const { return static_cast<FilerType>(m_clientFiler._GetFilerType()); }
OdDbDatabase*   DwgDbDxfFiler::database () const { return reinterpret_cast<OdDbDatabase*>(m_clientFiler._GetDatabase()); }
// WIP - implement read interfaces when needed
void            DwgDbDxfFiler::rdString (OdString& val) {;}
bool            DwgDbDxfFiler::rdBool () { return true; }
OdInt8          DwgDbDxfFiler::rdInt8 () { return 0; }
OdInt16         DwgDbDxfFiler::rdInt16 () { return 0; }
OdInt32         DwgDbDxfFiler::rdInt32 () { return 0; }
OdInt64         DwgDbDxfFiler::rdInt64 () { return 0; }
OdUInt8         DwgDbDxfFiler::rdUInt8 () { return 0; }
OdUInt16        DwgDbDxfFiler::rdUInt16 () { return 0; }
OdUInt32        DwgDbDxfFiler::rdUInt32 () { return 0; }
OdUInt64        DwgDbDxfFiler::rdUInt64 () { return 0; }
OdDbHandle      DwgDbDxfFiler::rdHandle () { return OdDbHandle(); }
OdDbObjectId    DwgDbDxfFiler::rdObjectId () { return OdDbObjectId(); }
double          DwgDbDxfFiler::rdAngle () { return 0.0; }
double          DwgDbDxfFiler::rdDouble () { return 0.0; }
void            DwgDbDxfFiler::rdPoint2d (OdGePoint2d& val) {;}
void            DwgDbDxfFiler::rdPoint3d (OdGePoint3d& val) {;}
void            DwgDbDxfFiler::rdVector2d (OdGeVector2d& val) {;}
void            DwgDbDxfFiler::rdVector3d (OdGeVector3d& val) {;}
void            DwgDbDxfFiler::rdScale3d (OdGeScale3d& val) {;}
void            DwgDbDxfFiler::rdBinaryChunk (OdBinaryData& val) {;}
// implement write methods
void            DwgDbDxfFiler::wrName (int code, const OdString& val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val); }
void            DwgDbDxfFiler::wrString (int code, const OdString& val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val); }
void            DwgDbDxfFiler::wrBool (int code, bool val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val); }
void            DwgDbDxfFiler::wrInt8 (int code, OdInt8 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int8_t>(val)); }
void            DwgDbDxfFiler::wrUInt8 (int code, OdUInt8 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint8_t>(val)); }
void            DwgDbDxfFiler::wrInt16 (int code, OdInt16 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int16_t>(val)); }
void            DwgDbDxfFiler::wrUInt16 (int code, OdUInt16 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint16_t>(val)); }
void            DwgDbDxfFiler::wrInt32 (int code, OdInt32 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int32_t>(val)); }
void            DwgDbDxfFiler::wrUInt32 (int code, OdUInt32 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint32_t>(val)); }
void            DwgDbDxfFiler::wrInt64 (int code, OdInt64 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int64_t>(val)); }
void            DwgDbDxfFiler::wrUInt64 (int code, OdUInt64 val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint64_t>(val)); }
void            DwgDbDxfFiler::wrHandle (int code, OdDbHandle val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<DwgDbHandle>(val)); }
void            DwgDbDxfFiler::wrObjectId (int code, OdDbObjectId val) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<DwgDbObjectId>(val)); }
void            DwgDbDxfFiler::wrAngle (int code, double val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val, static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrDouble (int code, double val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val, static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrPoint2d (int code, const OdGePoint2d& val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DPoint2dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrPoint3d (int code, const OdGePoint3d& val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DPoint3dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrVector2d (int code, const OdGeVector2d& val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DVec2dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrVector3d (int code, const OdGeVector3d& val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DVec3dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrScale3d (int code, const OdGeScale3d& val, int prec) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), val.sx, val.sy, val.sz, static_cast<IDxfFiler::DoublePrecision>(prec)); }
void            DwgDbDxfFiler::wrBinaryChunk (int code, const OdUInt8* buf, OdUInt32 nBytes) { m_clientFiler._Write(static_cast<DxfGroupCode>(code), DwgBinaryData(buf, nBytes)); }

#elif DWGTOOLKIT_RealDwg

AcDb::FilerType     DwgDbDxfFiler::filerType() const { return static_cast<AcDb::FilerType>(m_clientFiler._GetFilerType()); }
int                 DwgDbDxfFiler::rewindFiler  () { return 1; }
Acad::ErrorStatus   DwgDbDxfFiler::filerStatus  () const { return static_cast<Acad::ErrorStatus>(m_status); }
void                DwgDbDxfFiler::resetFilerStatus () { m_status = ToDwgDbStatus(Acad::eOk); }
AcDbDatabase*       DwgDbDxfFiler::database() const { return reinterpret_cast<AcDbDatabase*>(m_clientFiler._GetDatabase()); }
// WIP - implement the only read interface
Acad::ErrorStatus   DwgDbDxfFiler::readResBuf   (resbuf* pRb) { return Acad::eOk; }
// implemenet write interfaces
Acad::ErrorStatus   DwgDbDxfFiler::writeObjectId(AcDb::DxfCode code, const AcDbObjectId& id) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), reinterpret_cast<DwgDbObjectIdCR>(id))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeInt8    (AcDb::DxfCode code, Adesk::Int8 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int8_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeString  (AcDb::DxfCode code, const ACHAR* val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), DwgString(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeString  (AcDb::DxfCode code, const AcString& val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<DwgStringCR>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeBChunk  (AcDb::DxfCode code, const ads_binary& val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), DwgBinaryData(val.buf, val.clen))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeAcDbHandle(AcDb::DxfCode code, const AcDbHandle& val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), reinterpret_cast<DwgDbHandleCR>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeInt64   (AcDb::DxfCode code, Adesk::Int64 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int64_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeInt32   (AcDb::DxfCode code, Adesk::Int32 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int32_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeInt16   (AcDb::DxfCode code, Adesk::Int16 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<int16_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeUInt64  (AcDb::DxfCode code, Adesk::UInt64 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint64_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeUInt32  (AcDb::DxfCode code, Adesk::UInt32 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint32_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeUInt16  (AcDb::DxfCode code, Adesk::UInt16 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint16_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeUInt8   (AcDb::DxfCode code, Adesk::UInt8 val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), static_cast<uint8_t>(val))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeBoolean (AcDb::DxfCode code, Adesk::Boolean val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), Adesk::kTrue == val)); }
Acad::ErrorStatus   DwgDbDxfFiler::writeBool    (AcDb::DxfCode code, bool val) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), val)); }
Acad::ErrorStatus   DwgDbDxfFiler::writeDouble  (AcDb::DxfCode code, double val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), val, static_cast<IDxfFiler::DoublePrecision>(prec))); }
Acad::ErrorStatus   DwgDbDxfFiler::writePoint2d (AcDb::DxfCode code, const AcGePoint2d& val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DPoint2dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec))); }
Acad::ErrorStatus   DwgDbDxfFiler::writePoint3d (AcDb::DxfCode code, const AcGePoint3d& val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DPoint3dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeVector2d(AcDb::DxfCode code, const AcGeVector2d& val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DVec2dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeVector3d(AcDb::DxfCode code, const AcGeVector3d& val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), Util::DVec3dFrom(val), static_cast<IDxfFiler::DoublePrecision>(prec))); }
Acad::ErrorStatus   DwgDbDxfFiler::writeScale3d (AcDb::DxfCode code, const AcGeScale3d& val, int prec) { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(static_cast<DxfGroupCode>(code), val.sx, val.sy, val.sz, static_cast<IDxfFiler::DoublePrecision>(prec))); }
bool                DwgDbDxfFiler::includesDefaultValues() const { return  false; }
// need to implement group code 101 from attrdef that otherwise results in a fatal error from RealDWG!
Acad::ErrorStatus   DwgDbDxfFiler::writeEmbeddedObjectStart () { return static_cast<Acad::ErrorStatus>(m_clientFiler._Write(DxfGroupCode::EmbeddedObjectStart, DwgString(L"Embedded Object"))); }
#endif  // DWGTOOLKIT_

DWGRX_NO_CONS_DEFINE_MEMBERS(DwgDbDxfFiler, DWGDB_Type(DxfFiler))
