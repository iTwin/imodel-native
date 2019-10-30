/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    <Dwg/DwgDb/DwgDbCommon.h>
#include    <Dwg/DwgDb/BasicTypes.h>
#include    <Dwg/DwgDb/DwgResBuf.h>
#include    <Dwg/DwgDb/DwgDbObjects.h>
#include    <Dwg/DwgDb/DwgDbEntities.h>
#include    <Dwg/DwgDb/DwgDbSymbolTables.h>
#include    <Dwg/DwgDb/DwgDbDatabase.h>
#include    <Dwg/DwgDb/DwgRxObjects.h>
#include    <Dwg/DwgDb/DwgDrawables.h>
#include    <Dwg/DwgDb/DwgDbHost.h>

#ifdef DWGTOOLKIT_OpenDwg

#include    <Teigha/Drawing/Include/DbAnnotationScale.h>
#include    <Teigha/Drawing/Include/DbObjectContextCollection.h>
#include    <Teigha/Drawing/Include/DbObjectContextManager.h>
#include    <Teigha/Drawing/Include/DbObjectContextInterface.h>
#include    <Teigha/Drawing/Include/OdFileDepMgr.h>
#include    <Teigha/Drawing/Include/DbSymUtl.h>
#include    <Teigha/Drawing/Include/XRefMan.h>

// Geometry modules
#include    <Teigha/Kernel/Include/Ge/GeCircArc2d.h>
#include    <Teigha/Kernel/Include/Ge/GeCircArc3d.h>
#include    <Teigha/Kernel/Include/Ge/GeEllipArc2d.h>
#include    <Teigha/Kernel/Include/Ge/GeEllipArc3d.h>
#include    <Teigha/Kernel/Include/Ge/GeNurbCurve2d.h>
#include    <Teigha/Kernel/Include/Ge/GeNurbCurve3d.h>
#include    <Teigha/Kernel/Include/Ge/GeCompositeCurve2d.h>
#include    <Teigha/Kernel/Include/Ge/GeCompositeCurve3d.h>
#include    <Teigha/Kernel/Include/Ge/GeOffsetCurve2d.h>
#include    <Teigha/Kernel/Include/Ge/GeOffsetCurve3d.h>
#include    <Teigha/Kernel/Include/Ge/GeExternalCurve2d.h>
#include    <Teigha/Kernel/Include/Ge/GeExternalCurve3d.h>
#include    <Teigha/Kernel/Include/Ge/GeLineSeg2d.h>
#include    <Teigha/Kernel/Include/Ge/GeLineSeg3d.h>
#include    <Teigha/Kernel/Include/Ge/GeCone.h>
#include    <Teigha/Kernel/Include/Ge/GeCylinder.h>
#include    <Teigha/Kernel/Include/Ge/GeSphere.h>
#include    <Teigha/Kernel/Include/Ge/GeTorus.h>
#include    <Teigha/Kernel/Include/Ge/GeExternalSurface.h>
#include    <Teigha/Kernel/Include/Ge/GeExternalBoundedSurface.h>
#include    <Teigha/Kernel/Include/Ge/GeVoidPointerArray.h>

// BRep modules
#include    <Teigha/Kernel/Include/Br/BrEnums.h>
#include    <Teigha/Kernel/Include/Br/BrBrep.h>
#include    <Teigha/Kernel/Include/Br/BrLoop.h>
#include    <Teigha/Kernel/Include/Br/BrFace.h>
#include    <Teigha/Kernel/Include/Br/BrEdge.h>
#include    <Teigha/Kernel/Include/Br/BrVertex.h>
#include    <Teigha/Kernel/Include/Br/BrBrepComplexTraverser.h>
#include    <Teigha/Kernel/Include/Br/BrComplexShellTraverser.h>
#include    <Teigha/Kernel/Include/Br/BrBrepShellTraverser.h>
#include    <Teigha/Kernel/Include/Br/BrShellFaceTraverser.h>
#include    <Teigha/Kernel/Include/Br/BrFaceLoopTraverser.h>
#include    <Teigha/Kernel/Include/Br/BrLoopEdgeTraverser.h>

#include    <Teigha/Drawing/Include/GiContextForDbDatabase.h>
#include    <Teigha/Kernel/Include/Gi/GiBaseVectorizer.h>
#include    <Teigha/Kernel/Include/Gi/GiPolyline.h>

#if defined(_MSC_VER) && (DWGDB_ToolkitMajorRelease < 19)
#include    <Teigha/Kernel/Extensions/win/Crypt/WinNTCrypt.h>     // needed for DWG password
#endif

#elif DWGTOOLKIT_RealDwg

#include    <RealDwg/Base/rxregsvc.h>
#include    <RealDwg/Base/acestext.h>
#include    <RealDwg/Base/dbAnnotationScale.h>
#include    <RealDwg/Base/dbObjectContextCollection.h>
#include    <RealDwg/Base/dbObjectContextManager.h>
#include    <RealDwg/Base/dbObjectContextInterface.h>

// AcGe modules
#include    <RealDwg/Base/gemat3d.h>
#include    <RealDwg/Base/geell2d.h>
#include    <RealDwg/Base/geell3d.h>
#include    <RealDwg/Base/gecomp2d.h>
#include    <RealDwg/Base/gecomp3d.h>
#include    <RealDwg/Base/genurb2d.h>
#include    <RealDwg/Base/genurb3d.h>
#include    <RealDwg/Base/geextc2d.h>
#include    <RealDwg/Base/geextc3d.h>
#include    <RealDwg/Base/geoffc2d.h>
#include    <RealDwg/Base/geoffc3d.h>
#include    <RealDwg/Base/geextsf.h>
#include    <RealDwg/Base/gekvec.h>
#include    <RealDwg/Base/gecone.h>
#include    <RealDwg/Base/gecylndr.h>
#include    <RealDwg/Base/genurbsf.h>
#include    <RealDwg/Base/gesphere.h>
#include    <RealDwg/Base/getorus.h>
#include    <RealDwg/Base/gexbndsf.h>

// Util modules
#include    <RealDwg/Base/dbxutil.h>
#include    <RealDwg/Base/textengine.h>
#include    <RealDwg/Base/acgiutil.h>

// BRep modules
#include    <RealDwg/Brep/brgbl.h>
#include    <RealDwg/Brep/brbrep.h>
#include    <RealDwg/Brep/brcplx.h>
#include    <RealDwg/Brep/bredge.h>
#include    <RealDwg/Brep/brloop.h>
#include    <RealDwg/Brep/brface.h>
#include    <RealDwg/Brep/brshell.h>
#include    <RealDwg/Brep/brbctrav.h>
#include    <RealDwg/Brep/brcstrav.h>
#include    <RealDwg/Brep/brbetrav.h>
#include    <RealDwg/Brep/brbftrav.h>
#include    <RealDwg/Brep/brfltrav.h>
#include    <RealDwg/Brep/brletrav.h>
#include    <RealDwg/Brep/brlvtrav.h>
#include    <RealDwg/Brep/brsftrav.h>
#include    <RealDwg/Brep/brvtx.h>

//AEC modules
#include    <RealDwg/Base/AecVerChk.h>

#endif  // DWGTOOLKIT_

#ifdef _MSC_VER
#include    <io.h>                  // _wsopen_s, _read, _close
#include    <fcntl.h>               // modes defs for _wsopen_s
#include    <wininet.h>             // URL cache
#include    <Urlmon.h>              // URL download
#endif

#include    <Bentley/BeFileName.h>
#include    <Geom/CurveVector.h>

#include    "ToolkitHost.h"
#include    "DwgDbUtil.h"


USING_NAMESPACE_DWGDB

BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/16
+===============+===============+===============+===============+===============+======*/
struct DoNothingDxfFiler : IDxfFiler
    {
    DoNothingDxfFiler () {;}
    virtual DwgFilerType    _GetFilerType () const override { return DwgFilerType::BagFiler; }
    virtual DwgDbDatabaseP  _GetDatabase () const override { return nullptr; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, int8_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, int16_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, int32_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, int64_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint8_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint16_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint32_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint64_t v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, bool v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, double v, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgStringCR v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgBinaryDataCR v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbHandleCR v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbObjectIdCR v) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint2dCR v, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint3dCR v, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DVec2dCR v, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, DVec3dCR v, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    virtual DwgDbStatus     _Write (DxfGroupCode code, double x, double y, double z, DoublePrecision prec = DoublePrecision::Default) override { return DwgDbStatus::FilerError; }
    };  // DoNothingDxfFiler
static DoNothingDxfFiler      s_defaultDxfFiler;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDxfFiler : public DWGDB_EXTENDCLASS(DxfFiler)
    {
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(DxfFiler))
public:
    DWGRX_DECLARE_MEMBERS(DwgDbDxfFiler)
    DWGRX_DEFINE_SMARTPTR_BASE ()

    // constructors for the toolkit
    DwgDbDxfFiler () : m_clientFiler(s_defaultDxfFiler), m_status(DwgDbStatus::Success) {;}
    DwgDbDxfFiler (DWGDB_TypeCR(DxfFiler) sdkFiler) : T_Super(), m_clientFiler(s_defaultDxfFiler), m_status(DwgDbStatus::Success) {;}
    // construtors for the client
    explicit DwgDbDxfFiler (IDxfFilerR filer) : m_clientFiler(filer), m_status(DwgDbStatus::Success) {}

#ifdef DWGTOOLKIT_OpenDwg
virtual FilerType       filerType() const override;
virtual OdDbDatabase*   database () const override;
virtual void            rdString (OdString& val) override;
virtual bool            rdBool () override;
virtual OdInt8          rdInt8 () override;
virtual OdInt16         rdInt16 () override;
virtual OdInt32         rdInt32 () override;
virtual OdInt64         rdInt64 () override;
virtual OdUInt8         rdUInt8 () override;
virtual OdUInt16        rdUInt16 () override;
virtual OdUInt32        rdUInt32 () override;
virtual OdUInt64        rdUInt64 () override;
virtual OdDbHandle      rdHandle () override;
virtual OdDbObjectId    rdObjectId () override;
virtual double          rdAngle () override;
virtual double          rdDouble () override;
virtual void            rdPoint2d (OdGePoint2d& val) override;
virtual void            rdPoint3d (OdGePoint3d& val) override;
virtual void            rdVector2d (OdGeVector2d& val) override;
virtual void            rdVector3d (OdGeVector3d& val) override;
virtual void            rdScale3d (OdGeScale3d& val) override;
virtual void            rdBinaryChunk (OdBinaryData& val) override;
virtual void            wrName (int code, const OdString& val) override;
virtual void            wrString (int code, const OdString& val) override;
virtual void            wrBool (int code, bool val) override;
virtual void            wrInt8 (int code, OdInt8 val) override;
virtual void            wrUInt8 (int code, OdUInt8 val) override;
virtual void            wrInt16 (int code, OdInt16 val) override;
virtual void            wrUInt16 (int code, OdUInt16 val) override;
virtual void            wrInt32 (int code, OdInt32 val) override;
virtual void            wrUInt32 (int code, OdUInt32 val) override;
virtual void            wrInt64 (int code, OdInt64 val) override;
virtual void            wrUInt64 (int code, OdUInt64 val) override;
virtual void            wrHandle (int code, OdDbHandle val) override;
virtual void            wrObjectId (int code, OdDbObjectId val) override;
virtual void            wrAngle (int code, double val, int precision = kDfltPrec);
virtual void            wrDouble (int code, double val, int precision = kDfltPrec);
virtual void            wrPoint2d (int code, const OdGePoint2d& val, int precision = kDfltPrec) override;
virtual void            wrPoint3d (int code, const OdGePoint3d& val, int precision = kDfltPrec) override;
virtual void            wrVector2d (int code, const OdGeVector2d& val, int precision = kDfltPrec) override;
virtual void            wrVector3d (int code, const OdGeVector3d& val, int precision = kDfltPrec) override;
virtual void            wrScale3d (int code, const OdGeScale3d& val, int precision = kDfltPrec) override;
virtual void            wrBinaryChunk (int code, const OdUInt8* buf, OdUInt32 nBytes) override;

#elif DWGTOOLKIT_RealDwg

virtual AcDb::FilerType     filerType () const override;
virtual Acad::ErrorStatus   readResBuf   (resbuf* pRb);
virtual int                 rewindFiler  () override;
virtual Acad::ErrorStatus   filerStatus  () const override;
virtual void                resetFilerStatus () override;
virtual AcDbDatabase*       database () const override;
virtual Acad::ErrorStatus   writeObjectId(AcDb::DxfCode code, const AcDbObjectId& id) override;
virtual Acad::ErrorStatus   writeInt8    (AcDb::DxfCode code, Adesk::Int8 val) override;
virtual Acad::ErrorStatus   writeString  (AcDb::DxfCode code, const ACHAR* val) override;
virtual Acad::ErrorStatus   writeString  (AcDb::DxfCode code, const AcString& val) override;
virtual Acad::ErrorStatus   writeBChunk  (AcDb::DxfCode code, const ads_binary& val) override;
virtual Acad::ErrorStatus   writeAcDbHandle(AcDb::DxfCode code, const AcDbHandle& val) override;
virtual Acad::ErrorStatus   writeInt64   (AcDb::DxfCode code, Adesk::Int64 val) override;
virtual Acad::ErrorStatus   writeInt32   (AcDb::DxfCode code, Adesk::Int32 val) override;
virtual Acad::ErrorStatus   writeInt16   (AcDb::DxfCode code, Adesk::Int16 val) override;
virtual Acad::ErrorStatus   writeUInt64  (AcDb::DxfCode code, Adesk::UInt64 val) override;
virtual Acad::ErrorStatus   writeUInt32  (AcDb::DxfCode code, Adesk::UInt32 val) override;
virtual Acad::ErrorStatus   writeUInt16  (AcDb::DxfCode code, Adesk::UInt16 val) override;
virtual Acad::ErrorStatus   writeUInt8   (AcDb::DxfCode code, Adesk::UInt8 val) override;
virtual Acad::ErrorStatus   writeBoolean (AcDb::DxfCode code, Adesk::Boolean val) override;
virtual Acad::ErrorStatus   writeBool    (AcDb::DxfCode code, bool val) override;
virtual Acad::ErrorStatus   writeDouble  (AcDb::DxfCode code, double val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writePoint2d (AcDb::DxfCode code, const AcGePoint2d& val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writePoint3d (AcDb::DxfCode code, const AcGePoint3d& val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writeVector2d(AcDb::DxfCode code, const AcGeVector2d& val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writeVector3d(AcDb::DxfCode code, const AcGeVector3d& val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writeScale3d (AcDb::DxfCode code, const AcGeScale3d& val, int prec = kDfltPrec) override;
virtual Acad::ErrorStatus   writeEmbeddedObjectStart () override;
virtual bool                includesDefaultValues() const override;
#endif  // DWGTOOLKIT_

private:
    IDxfFilerR                  m_clientFiler;
    DwgDbStatus                 m_status;
    };  // DwgDbDxfFiler

END_DWGDB_NAMESPACE


#ifdef DWGTOOLKIT_OpenDwg

// implement DwgDbXxxxPtr:OpenObject method
#define IMPLEMENT_SMARTPTR_OPENOBJECT(_id_,_mode_,_openErased_,_openLocked_,_classSuffix_)                              \
    try {                                                                                                               \
    OdDbObjectPtr odObject;                                                                                             \
    OdResult rs = ##_id_##.openObject(odObject, static_cast<OdDb::OpenMode>(##_mode_##),##_openErased_##);              \
    if (OdResult::eOk == rs)                                                                                            \
        {                                                                                                               \
        if (odObject->isKindOf(DwgDb##_classSuffix_##::desc())) {                                                       \
            this->attach (odObject.detach());                                                                           \
            if (nullptr == this->get())                                                                                 \
                rs = OdResult::eNullObjectPointer;                                                                      \
            } else {                                                                                                    \
                rs = OdResult::eNotThatKindOfClass;                                                                     \
            }                                                                                                           \
        }                                                                                                               \
    m_openStatus = ToDwgDbStatus(rs);                                                                                   \
    } catch (...) { m_openStatus = DwgDbStatus::UnknownError; }

// implement DwgDbXxxxPtr:CreateObject method
#define IMPLEMENT_SMARTPTR_CREATEOBJECT(_classSuffix_)                                                                  \
    OdResult rs = OdResult::eNullObjectPointer;                                                                         \
    OdDbObjectPtr odObject = OdDb##_classSuffix_##::createObject();                                                     \
    if (!odObject.isNull())                                                                                             \
        {                                                                                                               \
        this->attach (odObject.detach());                                                                               \
        if (nullptr != this->get())                                                                                     \
            rs = OdResult::eOk;                                                                                         \
        }                                                                                                               \
    m_openStatus = ToDwgDbStatus(rs);

// implement DwgDbXxxxPtr:AcquireObject method
#define IMPLEMENT_SMARTPTR_ACQUIREOBJECT(_toBeAcquired_)                                                                \
    OdResult rs = OdResult::eNullObjectPointer;                                                                         \
    this->attach (_toBeAcquired_);                                                                                      \
    if (nullptr != this->get())                                                                                         \
        rs = OdResult::eOk;                                                                                             \
    m_openStatus = ToDwgDbStatus(rs);

// implement common DwgDbXxxTable methods
#define DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(_tableName_)                                                                   \
    DwgDbSymbolTableIteratorPtr DwgDb##_tableName_##::NewIterator(bool atBeginning,bool skipErased) const               \
        {                                                                                                               \
        OdDbSymbolTableIteratorPtr odIter = T_Super::newIterator(atBeginning, skipErased);                              \
        return new DwgDbSymbolTableIterator(odIter.get());                                                              \
        };                                                                                                              \
    DwgDbObjectId DwgDb##_tableName_##::GetByName(WStringCR name, bool erased) const                                    \
        {                                                                                                               \
        return T_Super::getAt (OdString(name.c_str()), erased);                                                         \
        }                                                                                                               \
    DwgDbObjectId DwgDb##_tableName_##::GetByName(DwgStringCR name, bool erased) const                                  \
        {                                                                                                               \
        return T_Super::getAt (name, erased);                                                                           \
        }                                                                                                               \
    DwgDbObjectId DwgDb##_tableName_##::Add(DwgDb##_tableName_##Record* rec)                                            \
        {                                                                                                               \
        return T_Super::add (static_cast<OdDb##_tableName_##Record*>(rec));                                             \
        }                                                                                                               \
    bool DwgDb##_tableName_##::Has(WStringCR name) const { return T_Super::has(name.c_str()); }                         \
    bool DwgDb##_tableName_##::Has(DwgStringCR name) const { return T_Super::has(name); }                               \
    bool DwgDb##_tableName_##::Has(DwgDbObjectIdCR id) const { return T_Super::has(id); }                               \


// DwgDb psuedo-database object sub-classed from a toolkit's database object
#define DWGDB_PSEUDO_DEFINE_MEMBERS(_classSuffix_)                                                                      \
    DwgDb##_classSuffix_##::DwgDb##_classSuffix_## () : T_Super() {;}                                                   \
    ODDB_PSEUDO_DEFINE_MEMBERS(DwgDb##_classSuffix_##,OdDb##_classSuffix_##,OdDb##_classSuffix_##,DBOBJECT_CONSTR)

#define DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(_classSuffix_)                               \
        DwgString DwgDb##_classSuffix_##::GetName() const { return T_Super::getName(); }    \
        bool DwgDb##_classSuffix_##::IsDependent() const { return T_Super::isDependent(); } \
        bool DwgDb##_classSuffix_##::IsResolved() const { return T_Super::isResolved(); }   \
        DwgDbStatus DwgDb##_classSuffix_##::SetName(DwgStringCR n) { T_Super::setName(n); return DwgDbStatus::Success; }

#define DWGDB_DEFINE_GETXDATA_MEMBER(_classSuffix_)                                     \
        DwgResBufIterator DwgDb##_classSuffix_##::GetXData (DwgStringCR regapp) const { return DwgResBufIterator::CreateFrom(T_Super::xData(regapp).get()); }

#define DWGRX_DEFINE_RX_MEMBER(_classSuffix_)                                                                       \
    OdRxClass*              Dwg##_classSuffix_##::Desc() { return g_pDesc; }                                        \
    OdRxClass*              Dwg##_classSuffix_##::SuperDesc() { return Od##_classSuffix_##::desc(); }               \
    OdRxObjectPtr           Dwg##_classSuffix_##::CreateObject() { return Dwg##_classSuffix_##::createObject(); }   \
    Dwg##_classSuffix_##*   Dwg##_classSuffix_##::Cast(OdRxObject const* rxObj)                                     \
        {                                                                                                           \
        if (rxObj == nullptr || rxObj->numRefs() < 1) return nullptr;                                               \
        return OdSmartPtr<Dwg##_classSuffix_##>(static_cast<Dwg##_classSuffix_##*>(rxObj->queryX(Dwg##_classSuffix_##::desc())),OdRxObjMod::kOdRxObjAttach).get(); \
        }                                                                                                           \
    OdRxObject* Dwg##_classSuffix_##::QueryX (OdRxClass const* rxc) const { return this->queryX(rxc); }             \
    OdRxObject* Dwg##_classSuffix_##::GetX (OdRxClass const* rxc) const { return this->x(rxc); }                    \
    OdRxClass*  Dwg##_classSuffix_##::IsA () const { return this->isA(); }

// use this wherever OpenDWG returns us an OdString whereas RealDWG does ACHAR*
#define DWGDB_ToWString(_odString_)       WString(##_odString_##.c_str())

#define DWGDB_DEFINE_DXFOUTFIELDS_MEMBER(_classSuffix_)                                 \
        DwgDbStatus DwgDb##_classSuffix_##::DxfOutFields(IDxfFilerR filer) const        \
            {                                                                           \
            DwgDbDxfFiler   opendwgFiler(filer);                                        \
            T_Super::dxfOutFields (&opendwgFiler);                                      \
            return  DwgDbStatus::Success;                                               \
            }
#define DWGDB_DEFINE_DXFOUT_MEMBER(_classSuffix_)                                       \
        DwgDbStatus DwgDb##_classSuffix_##::DxfOut(IDxfFilerR filer) const              \
            {                                                                           \
            DwgDbDxfFiler   opendwgFiler(filer);                                        \
            T_Super::dxfOut (&opendwgFiler);                                            \
            return  DwgDbStatus::Success;                                               \
            }

#define DWGDB_DEFINE_GETBINARYDATA_MEMBER(_classSuffix_)                                \
        DwgDbStatus DwgDb##_classSuffix_##::GetBinaryData(DwgStringCR key, size_t& size, char*& bytes) const \
            {                                                                           \
            size = 0; bytes = nullptr;                                                  \
            try {                                                                       \
                auto o = T_Super::extensionDictionary().openObject(OdDb::kForRead);     \
                if (o.isNull()) return DwgDbStatus::ObjectNotOpenYet;                   \
                auto dict = OdDbDictionary::cast(o.get());                              \
                if (nullptr == dict) return DwgDbStatus::InvalidInput;                  \
                if ((o = dict->getAt(key).openObject(OdDb::kForRead)).isNull()) return DwgDbStatus::ObjectNotOpenYet; \
                auto xrec = OdDbXrecord::cast(o.get());                                 \
                if (nullptr == xrec) return DwgDbStatus::InvalidInput;                  \
                auto rb = xrec->rbChain();                                              \
                if (rb.isNull()) return DwgDbStatus::InvalidInput;                      \
                for (; nullptr != rb; rb = rb->next())                                  \
                    if (OdDxfCode::_getType(rb->restype()) == OdDxfCode::BinaryChunk) { \
                        auto arr = rb->getBinaryChunk();                                \
                        size = arr.length();                                            \
                        if (size == 0) return DwgDbStatus::UnknownError;                \
                        bytes = new char[size];                                         \
                        if (nullptr == bytes) return DwgDbStatus::MemoryError;          \
                        ::memcpy (bytes, arr.asArrayPtr(), size); }                     \
                } /* try */                                                             \
            catch (OdError& e) { return ToDwgDbStatus(e.code()); }                      \
            return  size > 0 ? DwgDbStatus::Success : DwgDbStatus::UnknownError;        \
            }

#define DWGDB_DEFINE_SETBINARYDATA_MEMBER(_classSuffix_)                                \
        DwgDbStatus DwgDb##_classSuffix_##::SetBinaryData(DwgStringCR key, size_t size, const char* bytes) \
            {                                                                           \
            try {                                                                       \
                auto xrec = T_Super::createXrecord (key);                               \
                if (xrec.isNull()) return DwgDbStatus::MemoryError;                     \
                auto rb = OdResBuf::newRb(OdResBuf::kDxfBinaryChunk);                   \
                if (rb.isNull()) return DwgDbStatus::MemoryError;                       \
                OdBinaryData arr;                                                       \
                arr.setPhysicalLength(static_cast<unsigned int>(size));                 \
                for (size_t i = 0; i < size; i++) arr.append(bytes[i]);                 \
                rb->setBinaryChunk (arr);                                               \
                auto rs = xrec->setFromRbChain(rb.get(), T_Super::database());          \
                if (OdResult::eOk != rs) return ToDwgDbStatus(rs);                      \
                } /* try */                                                             \
            catch (OdError& e) { return ToDwgDbStatus(e.code()); }                      \
            return  size > 0 ? DwgDbStatus::Success : DwgDbStatus::UnknownError;        \
            }

#define DWGDB_ENTITY_DEFINE_GETGRIPPOINTS(_classSuffix_)                                \
        DwgDbStatus DwgDb##_classSuffix_##::GetGripPoints(DPoint3dArrayR points, DwgDbIntArrayP snapModes, DwgDbIntArrayP geomIds) const    \
            {                                                                           \
            OdGePoint3dArray    odPoints;                                               \
            OdResult status = T_Super::getGripPoints (odPoints);                        \
            if (OdResult::eOk == status)                                                \
                Util::GetPointArray(points, odPoints);                                  \
            return  ToDwgDbStatus(status);                                              \
            }

// create a new super DbObject which can then be later saved as a DB resident
#define DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)                                       \
        DwgDb##_classSuffix_##* DwgDb##_classSuffix_##::Create()                        \
            {                                                                           \
            auto odObj = new T_Super();                                                 \
            if (odObj == nullptr) return nullptr;                                       \
            odObj->addRef();                                                            \
            return DwgDb##_classSuffix_##::Cast(odObj);                                 \
            }


#elif DWGTOOLKIT_RealDwg


// implement DwgDbXxxxPtr:OpenObject method
#define IMPLEMENT_SMARTPTR_OPENOBJECT(_id_,_mode_,_openErased_,_openLocked_,_classSuffix_)                              \
        m_openStatus = ToDwgDbStatus(this->open(static_cast<AcDbObjectId>(##_id_##),                                    \
                        static_cast<AcDb::OpenMode>(##_mode_##),##_openErased_##,##_openLocked_##));

// implement DwgDbXxxxPtr:CreateObject method
#define IMPLEMENT_SMARTPTR_CREATEOBJECT(_classSuffix_)                                                                  \
        m_openStatus = ToDwgDbStatus(this->create());

// implement DwgDbXxxxPtr:AcquireObject method
#define IMPLEMENT_SMARTPTR_ACQUIREOBJECT(_toBeAcquired_)                                                                \
        m_openStatus = ToDwgDbStatus(this->acquire(_toBeAcquired_));

// implement common DwgDbXxxTable methods
#define DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(_tableName_)                                                                       \
    DwgDbSymbolTableIteratorPtr DwgDb##_tableName_##::NewIterator(bool atBeginning,bool skipErased) const                   \
        {                                                                                                                   \
        AcDb##_tableName_##Iterator* acIter = nullptr;                                                                      \
        if (Acad::eOk == this->newIterator(acIter, atBeginning, skipErased))                                                \
            return new DwgDbSymbolTableIterator(acIter);                                                                    \
        else                                                                                                                \
            return new DwgDbSymbolTableIterator();                                                                          \
        };                                                                                                                  \
    DwgDbObjectId DwgDb##_tableName_##::GetByName(WStringCR name, bool includeErased) const                                 \
        {                                                                                                                   \
        AcDbObjectId    id;                                                                                                 \
        if (T_Super::getAt(name.c_str(), id, includeErased) != Acad::eOk) id.setNull();                                     \
        return  id;                                                                                                         \
        }                                                                                                                   \
    DwgDbObjectId DwgDb##_tableName_##::GetByName(DwgStringCR name, bool includeErased) const                               \
        {                                                                                                                   \
        AcDbObjectId    id;                                                                                                 \
        if (T_Super::getAt(name.c_str(), id, includeErased) != Acad::eOk) id.setNull();                                     \
        return  id;                                                                                                         \
        }                                                                                                                   \
    DwgDbObjectId DwgDb##_tableName_##::Add(DwgDb##_tableName_##Record* rec)                                                \
        {                                                                                                                   \
        AcDbObjectId    id;                                                                                                 \
        if (rec == nullptr || T_Super::add(id, static_cast<AcDb##_tableName_##Record*>(rec)) != Acad::eOk) id.setNull();    \
        return  id;                                                                                                         \
        }                                                                                                                   \
    bool DwgDb##_tableName_##::Has(WStringCR name) const { return T_Super::has(name.c_str()); }                             \
    bool DwgDb##_tableName_##::Has(DwgStringCR name) const { return T_Super::has(name.c_str()); }                           \
    bool DwgDb##_tableName_##::Has(DwgDbObjectIdCR id) const { return T_Super::has(id); }

// DwgDb psuedo-database object sub-classed from a toolkit's database object
#define DWGDB_PSEUDO_DEFINE_MEMBERS(_classSuffix_)                                                                          \
    DwgDb##_classSuffix_##::DwgDb##_classSuffix_## () : T_Super() {;}                                                       \
    AcRxClass* DwgDb##_classSuffix_##::desc() { return AcDb##_classSuffix_##::desc(); }                                     \
    AcRxClass* DwgDb##_classSuffix_##::gpDesc = nullptr;                                                                    \
    void DwgDb##_classSuffix_##::rxInit() {                                                                                 \
        AcString className = AcString(L"DwgDb") + AcString(#_classSuffix_);                                                 \
        if (nullptr != DwgDb##_classSuffix_##::gpDesc) {                                                                    \
            AcRxClass *pClass = (AcRxClass*)((AcRxDictionary*)acrxSysRegistry()->at(ACRX_CLASS_DICTIONARY))->at(className.kwszPtr());  \
            if (nullptr != pClass) {                                                                                        \
                if (DwgDb##_classSuffix_##::gpDesc == pClass) return;                                                       \
                else acrx_abort(ACRX_T(/*MSGO*/"Class mismatch"));                                                          \
                }                                                                                                           \
            }                                                                                                               \
        AcString parentName = AcString(L"AcDb") + AcString(#_classSuffix_);                                                 \
        DwgDb##_classSuffix_##::gpDesc = newAcRxClass (className.kwszPtr(), parentName.kwszPtr());                          \
        }

#define DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(_classSuffix_)                           \
        DwgString DwgDb##_classSuffix_##::GetName() const {                             \
            const ACHAR* acName = nullptr;                                              \
            if (Acad::eOk == this->getName(acName))                                     \
                return  DwgString(acName);                                              \
            return  DwgString(); }                                                      \
        DwgDbStatus DwgDb##_classSuffix_##::SetName(DwgStringCR n) {                    \
            return ToDwgDbStatus(T_Super::setName(n.c_str())); }                        \
        bool DwgDb##_classSuffix_##::IsDependent() const { return T_Super::isDependent(); } \
        bool DwgDb##_classSuffix_##::IsResolved() const { return T_Super::isResolved(); }

#define DWGDB_DEFINE_GETXDATA_MEMBER(_classSuffix_)                                                                         \
        DwgResBufIterator DwgDb##_classSuffix_##::GetXData (DwgStringCR regapp) const                                       \
            { return  DwgResBufIterator::CreateFromAndFree(T_Super::xData(regapp.IsEmpty() ? nullptr : regapp.c_str())); }

#define DWGRX_DEFINE_RX_MEMBER(_classSuffix_)                                                                                       \
        AcRxClass*  Dwg##_classSuffix_##::SuperDesc() { return Ac##_classSuffix_##::desc(); }                                       \
        AcRxObject* Dwg##_classSuffix_##::CreateObject()                                                                            \
            {                                                                                                                       \
            return nullptr!=Dwg##_classSuffix_##::desc() ? Dwg##_classSuffix_##::desc()->create() : nullptr;                        \
            }                                                                                                                       \
        AcRxClass*  Dwg##_classSuffix_##::Desc()                                                                                    \
            {                                                                                                                       \
            if (gpDesc != nullptr) return gpDesc;                                                                                   \
            AcString className = AcString(L"Dwg") + AcString(#_classSuffix_);                                                       \
            return gpDesc = (AcRxClass*)((AcRxDictionary*)acrxSysRegistry()->at(ACRX_CLASS_DICTIONARY))->at(className.kwszPtr());   \
            }                                                                                                                       \
        AcRxClass*  Dwg##_classSuffix_##::isA() const                                                                               \
            {                                                                                                                       \
            if (gpDesc != nullptr) return gpDesc;                                                                                   \
            AcString className = AcString(L"Dwg") + AcString(#_classSuffix_);                                                       \
            return gpDesc = (AcRxClass*)((AcRxDictionary*)acrxSysRegistry()->at(ACRX_CLASS_DICTIONARY))->at(className.kwszPtr());   \
            }                                                                                                                       \
        Dwg##_classSuffix_##*   Dwg##_classSuffix_##::Cast(AcRxObject const* rxObj)                                                 \
            {                                                                                                                       \
            return ((rxObj == nullptr) || !rxObj->isKindOf(Dwg##_classSuffix_##::desc())) ? nullptr : (Dwg##_classSuffix_##*)rxObj; \
            }                                                                                                                       \
        AcRxObject* Dwg##_classSuffix_##::QueryX (AcRxClass const* rxc) const { return T_Super::queryX(rxc); }                      \
        AcRxObject* Dwg##_classSuffix_##::GetX (AcRxClass const* rxc) const { return T_Super::x(rxc); }                             \
        AcRxClass*  Dwg##_classSuffix_##::IsA () const { return this->isA(); }

// use this wherever OpenDWG returns us an OdString whereas RealDWG does ACHAR*
#define DWGDB_ToWString(_chars_)       WString(static_cast<WCharCP>(##_chars_##))

#define DWGDB_DEFINE_DXFOUTFIELDS_MEMBER(_classSuffix_)                                 \
        DwgDbStatus DwgDb##_classSuffix_##::DxfOutFields(IDxfFilerR filer) const        \
            {                                                                           \
            DwgDbDxfFiler   realdwgFiler(filer);                                        \
            return static_cast<DwgDbStatus>(T_Super::dxfOutFields(&realdwgFiler));      \
            }
#define DWGDB_DEFINE_DXFOUT_MEMBER(_classSuffix_)                                           \
        DwgDbStatus DwgDb##_classSuffix_##::DxfOut(IDxfFilerR filer) const                  \
            {                                                                               \
            DwgDbDxfFiler       realdwgFiler(filer);                                        \
            Acad::ErrorStatus   es = T_Super::dxfOut(&realdwgFiler, Adesk::kTrue, nullptr); \
            return static_cast<DwgDbStatus>(es);                                            \
            }

#define DWGDB_DEFINE_GETBINARYDATA_MEMBER(_classSuffix_)                                    \
        DwgDbStatus DwgDb##_classSuffix_##::GetBinaryData(DwgStringCR key, size_t& size, char*& data) const \
            {                                                                               \
            auto es = T_Super::getBinaryData(key.c_str(), (Adesk::Int32&)size, data);       \
            return  ToDwgDbStatus(es);                                                      \
            }

#define DWGDB_DEFINE_SETBINARYDATA_MEMBER(_classSuffix_)                                    \
        DwgDbStatus DwgDb##_classSuffix_##::SetBinaryData(DwgStringCR key, size_t size, const char* data) \
            {                                                                               \
            auto es = T_Super::setBinaryData(key.c_str(), (Adesk::Int32)size, data);        \
            return  ToDwgDbStatus(es);                                                      \
            }

#define DWGDB_ENTITY_DEFINE_GETGRIPPOINTS(_classSuffix_)                                    \
        DwgDbStatus DwgDb##_classSuffix_##::GetGripPoints(DPoint3dArrayR points, DwgDbIntArrayP snapModes, DwgDbIntArrayP geomIds) const    \
            {                                                                               \
            AcGePoint3dArray    acPoints;                                                   \
            AcDbIntArray        acModes, acGeomIds;                                         \
            Acad::ErrorStatus status=T_Super::getGripPoints(acPoints, acModes, acGeomIds);  \
            if (Acad::eOk == status)                                                        \
                {                                                                           \
                if (nullptr != snapModes)                                                   \
                    {                                                                       \
                    for (int i = 0; i < acModes.length(); i++)                              \
                        snapModes->push_back (acModes.at(i));                               \
                    }                                                                       \
                if (nullptr != geomIds)                                                     \
                    {                                                                       \
                    for (int i = 0; i < acGeomIds.length(); i++)                            \
                        geomIds->push_back (acGeomIds.at(i));                               \
                    }                                                                       \
                Util::GetPointArray(points, acPoints);                                      \
                }                                                                           \
            return  ToDwgDbStatus(status);                                                  \
            }

// create a new super DbObject which can then be later saved as a DB resident
#define DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)                                       \
    DwgDb##_classSuffix_##P DwgDb##_classSuffix_##::Create()                            \
        { return DwgDb##_classSuffix_##::Cast(new AcDb##_classSuffix_##()); }

#endif  // DWGTOOLKIT_


#define DWGDB_DEFINE_SMARTPTR_OPENOBJECT_CONSTRUCTOR(_classSuffix_)                                                         \
    DwgDb##_classSuffix_##Ptr::DwgDb##_classSuffix_##Ptr(DwgDbObjectId id, DwgDbOpenMode mode, bool openErased, bool openLocked)    \
        {                                                                                                                           \
        IMPLEMENT_SMARTPTR_OPENOBJECT(id, mode, openErased, openLocked, _classSuffix_);                                             \
        }

// implement IDwgDbSmartPtr methods and define non-virtual methods to call them
#define DWGDB_IMPLEMENT_SMARTPTR_INTERFACE(_classSuffix_)                                                                       \
    bool DwgDb##_classSuffix_##Ptr::_IsNull() const { return DWGDB_CALLSDKMETHOD(isNull(), object()==nullptr); }                \
    DwgDb##_classSuffix_##CP DwgDb##_classSuffix_##Ptr::_Get () const                                                           \
        {                                                                                                                       \
        return static_cast<DwgDb##_classSuffix_##CP>(DWGDB_CALLSDKMETHOD(m_pObject, object()));                                 \
        }                                                                                                                       \
    DwgDb##_classSuffix_##P DwgDb##_classSuffix_##Ptr::_Get ()                                                                  \
        {                                                                                                                       \
        return static_cast<DwgDb##_classSuffix_##P>(DWGDB_CALLSDKMETHOD(m_pObject, object()));                                  \
        }                                                                                                                       \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::_OpenObject(DwgDbObjectId id, DwgDbOpenMode mode, bool openErased, bool openLocked)  \
        {                                                                                                                       \
        IMPLEMENT_SMARTPTR_OPENOBJECT(id, mode, openErased, openLocked, _classSuffix_);                                         \
        return  m_openStatus;                                                                                                   \
        }                                                                                                                       \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::_CreateObject ()                                                                     \
        {                                                                                                                       \
        IMPLEMENT_SMARTPTR_CREATEOBJECT (##_classSuffix_##);                                                                    \
        return  m_openStatus;                                                                                                   \
        }                                                                                                                       \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::_AcquireObject (DwgDb##_classSuffix_## *& obj)                                       \
        {                                                                                                                       \
        IMPLEMENT_SMARTPTR_ACQUIREOBJECT (obj);                                                                                 \
        return  m_openStatus;                                                                                                   \
        }                                                                                                                       \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::_CloseObject ()                                                                      \
        {                                                                                                                       \
        DwgDbStatus closeStatus = DwgDbStatus::Success;                                                                         \
        DWGDB_CALLSDKMETHOD (this->release(), closeStatus = ToDwgDbStatus(this->close()));                                      \
        m_openStatus = DwgDbStatus::ObjectNotOpenYet;                                                                           \
        return  closeStatus;                                                                                                    \
        }                                                                                                                       \
    bool DwgDb##_classSuffix_##Ptr::IsNull() const { return _IsNull(); }                                                        \
    DwgDb##_classSuffix_##CP DwgDb##_classSuffix_##Ptr::get () const { return this->_Get(); }                                   \
    DwgDb##_classSuffix_##P DwgDb##_classSuffix_##Ptr::get () { return this->_Get(); }                                          \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::OpenObject(DwgDbObjectId id, DwgDbOpenMode mode, bool openErased, bool openLocked)   \
        {                                                                                                                       \
        return  this->_OpenObject(id, mode, openErased, openLocked);                                                            \
        }                                                                                                                       \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::AcquireObject(DwgDb##_classSuffix_## *& obj) { return this->_AcquireObject(obj); }   \
    DwgDbStatus DwgDb##_classSuffix_##Ptr::CloseObject() { return this->_CloseObject(); }                                       \
    DwgDb##_classSuffix_##Ptr::DwgDb##_classSuffix_##Ptr (DwgDb##_classSuffix_##* obj)                                          \
        {                                                                                                                       \
        IMPLEMENT_SMARTPTR_ACQUIREOBJECT(obj);                                                                                  \
        }                                                                                                                       \
    DwgDb##_classSuffix_##Ptr& DwgDb##_classSuffix_##Ptr::operator = (DwgDb##_classSuffix_##* obj)                              \
        {                                                                                                                       \
        IMPLEMENT_SMARTPTR_ACQUIREOBJECT(obj);                                                                                  \
        return *this;                                                                                                           \
        }
        
#define DWGDB_ENTITY_DEFINE_GETRANGE(_classSuffix_)                                     \
    DwgDbStatus DwgDb##_classSuffix_##::GetRange (DRange3dR range) const                \
        {                                                                               \
        DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) extents;                               \
        DwgDbStatus status = ToDwgDbStatus (T_Super::getGeomExtents(extents));          \
        if (DwgDbStatus::Success == status)                                             \
            range = Util::DRange3dFrom (extents);                                       \
        else                                                                            \
            range.Init ();                                                              \
        return  status;                                                                 \
        }


#define DWGDB_DEFINE_SMARTPTR_MEMBERS(_classSuffix_)                                    \
            DWGDB_DEFINE_SMARTPTR_OPENOBJECT_CONSTRUCTOR(##_classSuffix_##)             \
            DWGDB_IMPLEMENT_SMARTPTR_INTERFACE(##_classSuffix_##)

// define common methods for DbObject derivitives
#define DWGDB_OBJECT_DEFINE_MEMBERS(_classSuffix_)                                                                                                                              \
    DwgDbObjectId      DwgDb##_classSuffix_##::GetObjectId () const { return T_Super::objectId(); }                                                                             \
    DwgDbObjectId      DwgDb##_classSuffix_##::GetOwnerId () const { return T_Super::ownerId(); }                                                                               \
    DwgDbDatabasePtr   DwgDb##_classSuffix_##::GetDatabase () const { return T_Super::database(); }                                                                             \
    DwgDbStatus        DwgDb##_classSuffix_##::UpgradeOpen () { DWGDB_CALLSDKMETHOD(upgradeOpen(); return DwgDbStatus::Success;, return ToDwgDbStatus(upgradeOpen());) }        \
    DwgDbStatus        DwgDb##_classSuffix_##::DowngradeOpen () { DWGDB_CALLSDKMETHOD(downgradeOpen(); return DwgDbStatus::Success;, return ToDwgDbStatus(downgradeOpen());) }  \
    DwgDbStatus        DwgDb##_classSuffix_##::Close () { DWGDB_CALLSDKMETHOD(T_Super::release(); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::close());) }      \
    DwgDbStatus        DwgDb##_classSuffix_##::Erase () { return ToDwgDbStatus(T_Super::erase()); }                                                                             \
    DwgString          DwgDb##_classSuffix_##::GetDxfName () const { return isA()->dxfName(); }                                                                                 \
    DwgString          DwgDb##_classSuffix_##::GetDwgClassName () const { return isA()->name(); }                                                                               \
    DwgDbObjectId      DwgDb##_classSuffix_##::GetExtensionDictionary () const { return T_Super::extensionDictionary(); }                                                       \
    DwgDbStatus        DwgDb##_classSuffix_##::CreateExtensionDictionary () { DWGDB_CALLSDKMETHOD(T_Super::createExtensionDictionary(); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::createExtensionDictionary());) }    \
    DwgDbStatus        DwgDb##_classSuffix_##::ReleaseExtensionDictionary () { DWGDB_CALLSDKMETHOD(T_Super::releaseExtensionDictionary(); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::releaseExtensionDictionary());) } \
    bool               DwgDb##_classSuffix_##::HasPersistentReactor (DwgDbObjectIdCR id) const { return T_Super::hasPersistentReactor(id); }                                    \
    DwgDbObjectIdArray DwgDb##_classSuffix_##::GetPersistentReactors () const { return Util::GetPersistentReactors(DWGDB_Type(Object)::cast(this)); }                           \
    DWGDB_DEFINE_DXFOUTFIELDS_MEMBER(##_classSuffix_##)                                                                                                                         \
    DWGDB_DEFINE_DXFOUT_MEMBER(##_classSuffix_##)                                                                                                                               \
    DWGDB_DEFINE_GETXDATA_MEMBER(##_classSuffix_##)                                                                                                                             \
    DWGDB_DEFINE_GETBINARYDATA_MEMBER(##_classSuffix_##)                                                                                                                        \
    DWGDB_DEFINE_SETBINARYDATA_MEMBER(##_classSuffix_##)                                                                                                                        \
    DWGRX_DEFINE_RX_MEMBER(Db##_classSuffix_##)

// define common methods for DbObject derivitives requiring psuedo-database (e.g. DwgGi objects)
    // define psuedo-database entities
    // implement SmartPtr interfaces & define their non-virtual counterparts
    // define common DbObject methods (above)
#define DWGDB_OBJECT_DEFINE_BASEMEMBERS(_classSuffix_)              \
    DWGDB_PSEUDO_DEFINE_MEMBERS(_classSuffix_)                      \
    DWGDB_DEFINE_SMARTPTR_MEMBERS(_classSuffix_)                    \
    DWGDB_OBJECT_DEFINE_MEMBERS(_classSuffix_)

// define common methods generic Object, plus an object Create method
#define DWGDB_OBJECT_DEFINE_MEMBERS2(_classSuffix_)                 \
    DWGDB_OBJECT_DEFINE_BASEMEMBERS(_classSuffix_)                  \
    DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)
    
// define common methods for DbEntity derivitives:
#define DWGDB_ENTITY_DEFINE_BASEMEMBERS(_classSuffix_)              \
    DWGDB_OBJECT_DEFINE_BASEMEMBERS(_classSuffix_)                  \
    DWGDB_ENTITY_DEFINE_GETGRIPPOINTS(_classSuffix_)                \
    DWGDB_ENTITY_DEFINE_GETRANGE(_classSuffix_)                     \
    uint16_t            DwgDb##_classSuffix_##::GetColorIndex () const { return static_cast<uint16_t>(T_Super::colorIndex()); }            \
    DwgCmColor          DwgDb##_classSuffix_##::GetColor () const { return static_cast<DwgCmColor>(T_Super::color()); }                    \
    DwgCmEntityColor    DwgDb##_classSuffix_##::GetEntityColor () const { return static_cast<DwgCmEntityColor>(T_Super::entityColor()); }  \
    DwgDbObjectId       DwgDb##_classSuffix_##::GetLayerId () const { return T_Super::layerId(); }                                         \
    DwgDbObjectId       DwgDb##_classSuffix_##::GetLinetypeId () const { return T_Super::linetypeId(); }                                   \
    double              DwgDb##_classSuffix_##::GetLinetypeScale () const { return T_Super::linetypeScale(); }                             \
    DwgDbLineWeight     DwgDb##_classSuffix_##::GetLineweight () const { return DWGDB_UPWARDCAST(LineWeight)(T_Super::lineWeight()); }     \
    DwgDbObjectId       DwgDb##_classSuffix_##::GetMaterialId () const { return T_Super::materialId(); }                                   \
    DwgTransparency     DwgDb##_classSuffix_##::GetTransparency () const { return T_Super::transparency(); }                               \
    DwgDbVisibility     DwgDb##_classSuffix_##::GetVisibility () const { return static_cast<DwgDbVisibility>(T_Super::visibility()); }     \
    void                DwgDb##_classSuffix_##::List () const { T_Super::list(); }                                                         \
    DwgGiDrawablePtr    DwgDb##_classSuffix_##::GetDrawable () { return new DwgGiDrawable(T_Super::drawable()); }                          \
    bool                DwgDb##_classSuffix_##::CanCastShadows () const { return T_Super::castShadows(); }                                 \
    bool                DwgDb##_classSuffix_##::CanReceiveShadows () const { return T_Super::receiveShadows(); }                           \
    bool                DwgDb##_classSuffix_##::IsPlanar () const { return DWGDB_Type(Entity)::isPlanar(); }                               \
    bool                DwgDb##_classSuffix_##::IsAProxy () const { return DWGDB_Type(Entity)::isAProxy(); }                               \
    void DwgDb##_classSuffix_##::GetEcs(TransformR ecs) const { DWGGE_Type(Matrix3d) m; DWGDB_CALLSDKMETHOD(m=T_Super::getEcs(), T_Super::getEcs(m)); return Util::GetTransform(ecs, m); } \
    DwgDbStatus DwgDb##_classSuffix_##::TransformBy (TransformCR t) { DWGGE_Type(Matrix3d) m; Util::GetGeMatrix(m, t); return ToDwgDbStatus(T_Super::transformBy(m)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetColor (DwgCmColorCR c, bool s) { return ToDwgDbStatus(T_Super::setColor(static_cast<DWGCM_TypeCR(Color)>(c), s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetColorIndex (uint16_t c, bool s) { return ToDwgDbStatus(T_Super::setColorIndex(c, s)); }         \
    DwgDbStatus DwgDb##_classSuffix_##::SetLinetype (DwgDbObjectId id, bool s) { return ToDwgDbStatus(T_Super::setLinetype(DWGDB_DOWNWARDCAST(ObjectId)(id), s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetLinetypeScale (double scale, bool s) { return  ToDwgDbStatus(T_Super::setLinetypeScale(scale, s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetLineweight (DwgDbLineWeight w, bool s) { return ToDwgDbStatus(T_Super::setLineWeight(DWGDB_CASTTOENUM_DB(LineWeight)(w), s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetLayer (DwgDbObjectId id, bool s, bool h) { return ToDwgDbStatus(T_Super::setLayer(DWGDB_DOWNWARDCAST(ObjectId)(id), s, h)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetMaterial (DwgDbObjectId id, bool s) { return ToDwgDbStatus(T_Super::setMaterial(DWGDB_DOWNWARDCAST(ObjectId)(id), s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetTransparency (DwgTransparencyCR t, bool s) { return ToDwgDbStatus(T_Super::setTransparency(static_cast<DWGCM_TypeCR(Transparency)>(t), s)); } \
    DwgDbStatus DwgDb##_classSuffix_##::SetPropertiesFrom (DwgDbEntityCR e, bool s) { DwgDbStatus r=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setPropertiesFrom(OdDbEntity::cast(&e), s), r=ToDwgDbStatus(T_Super::setPropertiesFrom(AcDbEntity::cast(&e), s))); return r; } \
    void DwgDb##_classSuffix_##::SetDatabaseDefaults (DwgDbDatabaseP dwg) { T_Super::setDatabaseDefaults(dwg); } \
    void DwgDb##_classSuffix_##::SetCastShadows (bool c) { T_Super::setCastShadows(c); } \
    void DwgDb##_classSuffix_##::SetReceiveShadows (bool r) { T_Super::setReceiveShadows(r); } \
    void DwgDb##_classSuffix_##::SetVisibility (DwgDbVisibility v, bool s) { T_Super::setVisibility(DWGDB_CASTTOENUM_DB(Visibility)(v), s); } \
    DwgDbStatus DwgDb##_classSuffix_##::Explode (DwgDbObjectPArrayR entities) const { \
        TkObjectArray array; \
        DwgDbStatus status = ToDwgDbStatus (T_Super::explode(array)); \
        if (DwgDbStatus::Success == status) { \
            Util::GetObjectArray(entities, array); } \
        return status; }

// define common methods for DbEntity as above, plus an object Create method
#define DWGDB_ENTITY_DEFINE_MEMBERS(_classSuffix_)                  \
    DWGDB_ENTITY_DEFINE_BASEMEMBERS(_classSuffix_)                  \
    DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)

// define common methods for DbSurface
#define DWGDB_SURFACE_DEFINE_MEMBERS(_classSuffix_)                 \
    DWGDB_ENTITY_DEFINE_MEMBERS(_classSuffix_)                      \
    DwgDbStatus DwgDb##_classSuffix_##::GetArea (double& a) const { return ToDwgDbStatus(T_Super::getArea(a)); }  \
    DwgDbStatus DwgDb##_classSuffix_##::GetPerimeter (double& p) const { return ToDwgDbStatus(T_Super::getPerimeter(p)); } \
    DwgDbStatus DwgDb##_classSuffix_##::ConvertToRegion (DwgDbEntityPArrayR regions) { \
        TkEntityArray array;                                        \
        DwgDbStatus status = ToDwgDbStatus (T_Super::convertToRegion(array)); \
        if (DwgDbStatus::Success == status) {                       \
            Util::GetEntityArray(regions, array); }                 \
        return status; }


void    RegisterDwgDbObjectExtensions (bool beforeValidation);
void    UnRegisterDwgDbObjectExtensions ();
void    RegisterDwgGiExtensions ();
void    UnRegisterDwgGiExtensions ();
