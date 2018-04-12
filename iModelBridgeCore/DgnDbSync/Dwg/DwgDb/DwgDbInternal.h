/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DwgDbInternal.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>
#include    <DgnDbSync/Dwg/DwgDb/BasicTypes.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgResBuf.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbObjects.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbEntities.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbSymbolTables.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbDatabase.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgRxObjects.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDrawables.h>
#include    <DgnDbSync/Dwg/DwgDb/DwgDbHost.h>

#ifdef DWGTOOLKIT_OpenDwg

#include    <Teigha/Drawing/Include/DbAnnotationScale.h>
#include    <Teigha/Drawing/Include/DbObjectContextCollection.h>
#include    <Teigha/Drawing/Include/DbObjectContextManager.h>
#include    <Teigha/Drawing/Include/DbObjectContextInterface.h>
#include    <Teigha/Drawing/Include/OdFileDepMgr.h>
#include    <Teigha/Drawing/Include/DbSymUtl.h>
#include    <Teigha/Drawing/Include/XRefMan.h>

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
#include    <Teigha/Kernel/Include/Ge/GeLine2d.h>
#include    <Teigha/Kernel/Include/Ge/GeLine3d.h>

#include    <Teigha/Drawing/Include/GiContextForDbDatabase.h>
#include    <Teigha/Kernel/Include/Gi/GiBaseVectorizer.h>
#include    <Teigha/Kernel/Include/Gi/GiPolyline.h>

#ifdef _MSC_VER
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
#include    <RealDwg/base/gemat3d.h>
#include    <RealDwg/base/geell2d.h>
#include    <RealDwg/base/geell3d.h>
#include    <RealDwg/base/gecomp2d.h>
#include    <RealDwg/base/gecomp3d.h>
#include    <RealDwg/base/genurb2d.h>
#include    <RealDwg/base/genurb3d.h>
#include    <RealDwg/base/geextc2d.h>
#include    <RealDwg/base/geextc3d.h>
#include    <RealDwg/base/geoffc2d.h>
#include    <RealDwg/base/geoffc3d.h>
#include    <RealDwg/base/geextsf.h>
#include    <RealDwg/base/gekvec.h>
#include    <RealDwg/base/gecone.h>
#include    <RealDwg/base/gecylndr.h>
#include    <RealDwg/base/genurbsf.h>
#include    <RealDwg/base/gesphere.h>
#include    <RealDwg/base/getorus.h>
#include    <RealDwg/base/gexbndsf.h>

// Util modules
#include    <RealDwg/base/dbxutil.h>
#include    <RealDwg/base/textengine.h>
#include    <RealDwg/base/acgiutil.h>

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
#define IMPLEMENT_SMARTPTR_OPENOBJECT(_id_,_mode_,_openErased_,_openLocked_)                                            \
    try {                                                                                                               \
    OdDbObjectPtr odObject;                                                                                             \
    OdResult rs = ##_id_##.openObject(odObject, static_cast<OdDb::OpenMode>(##_mode_##),##_openErased_##);              \
    if (OdResult::eOk == rs)                                                                                            \
        {                                                                                                               \
        this->attach (odObject.detach());                                                                               \
        if (nullptr == this->get())                                                                                     \
            rs = OdResult::eNullObjectPointer;                                                                          \
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

// DwgDbXxxTable::NewIterator
#define DWGDB_DEFINE_SYMBOLTABLE_NEWITERATOR(_tableName_)                                                               \
    DwgDbSymbolTableIterator DwgDb##_tableName_##::NewIterator(bool atBeginning,bool skipErased) const                  \
        {                                                                                                               \
        OdDbSymbolTableIteratorPtr odIter = this->newIterator(atBeginning, skipErased);                                 \
        return  odIter.get();                                                                                           \
        };

// DwgDb psuedo-database object sub-classed from a toolkit's database object
#define DWGDB_PSEUDO_DEFINE_MEMBERS(_classSuffix_)                                                                      \
    DwgDb##_classSuffix_##::DwgDb##_classSuffix_## () : T_Super() {;}                                                   \
    ODDB_PSEUDO_DEFINE_MEMBERS(DwgDb##_classSuffix_##,OdDb##_classSuffix_##,OdDb##_classSuffix_##,DBOBJECT_CONSTR)

#define DWGDB_DEFINE_SYMBOLTABLERECORD_GETNAME(_classSuffix_)                           \
        DwgString DwgDb##_classSuffix_##::GetName() const { return this->getName(); }

#define DWGDB_DEFINE_GETXDATA_MEMBER(_classSuffix_)                                     \
        DwgResBufIterator DwgDb##_classSuffix_##::GetXData (DwgStringCR regapp) const { return DwgResBufIterator::CreateFrom(T_Super::xData(regapp).get()); }

#define DWGRX_DEFINE_RX_MEMBER(_classSuffix_)                                                                       \
    OdRxClass*              Dwg##_classSuffix_##::Desc() { return g_pDesc; }                                        \
    OdRxClass*              Dwg##_classSuffix_##::SuperDesc() { return Od##_classSuffix_##::desc(); }               \
    OdRxObjectPtr           Dwg##_classSuffix_##::CreateObject() { return Dwg##_classSuffix_##::createObject(); }   \
    Dwg##_classSuffix_##*   Dwg##_classSuffix_##::Cast(OdRxObject const* rxObj)                                     \
        {                                                                                                           \
        return (Dwg##_classSuffix_##*)rxObj->queryX(Dwg##_classSuffix_##::desc());                                  \
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

#define DWGDB_ENTITY_DEFINE_GETGRIPPOINTS(_classSuffix_)                                \
        DwgDbStatus DwgDb##_classSuffix_##::GetGripPoints(DPoint3dArrayR points, DwgDbIntArrayP snapModes, DwgDbIntArrayP geomIds) const    \
            {                                                                           \
            OdGePoint3dArray    odPoints;                                               \
            OdResult status = T_Super::getGripPoints (odPoints);                        \
            if (OdResult::eOk == status)                                                \
                Util::GetPointArray(points, odPoints);                                  \
            return  ToDwgDbStatus(status);                                              \
            }

#elif DWGTOOLKIT_RealDwg


// implement DwgDbXxxxPtr:OpenObject method
#define IMPLEMENT_SMARTPTR_OPENOBJECT(_id_,_mode_,_openErased_,_openLocked_)                                            \
        m_openStatus = ToDwgDbStatus(this->open(static_cast<AcDbObjectId>(##_id_##),                                    \
                        static_cast<AcDb::OpenMode>(##_mode_##),##_openErased_##,##_openLocked_##));

// implement DwgDbXxxxPtr:CreateObject method
#define IMPLEMENT_SMARTPTR_CREATEOBJECT(_classSuffix_)                                                                  \
        m_openStatus = ToDwgDbStatus(this->create());

// implement DwgDbXxxxPtr:AcquireObject method
#define IMPLEMENT_SMARTPTR_ACQUIREOBJECT(_toBeAcquired_)                                                                \
        m_openStatus = ToDwgDbStatus(this->acquire(_toBeAcquired_));

// DwgDbXxxTable::NewIterator
#define DWGDB_DEFINE_SYMBOLTABLE_NEWITERATOR(_tableName_)                                                                   \
    DwgDbSymbolTableIterator DwgDb##_tableName_##::NewIterator(bool atBeginning,bool skipErased) const                      \
        {                                                                                                                   \
        AcDb##_tableName_##Iterator* acIter = nullptr;                                                                      \
        if (Acad::eOk == this->newIterator(acIter, atBeginning, skipErased))                                                \
            return DwgDbSymbolTableIterator(acIter);                                                                        \
        else                                                                                                                \
            return DwgDbSymbolTableIterator();                                                                              \
        };

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

#define DWGDB_DEFINE_SYMBOLTABLERECORD_GETNAME(_classSuffix_)                           \
        DwgString DwgDb##_classSuffix_##::GetName() const {                             \
            const ACHAR* acName = nullptr;                                              \
            if (Acad::eOk == this->getName(acName))                                     \
                return  DwgString(acName);                                              \
            return  DwgString();                                                        \
            }

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

#endif  // DWGTOOLKIT_


#define DWGDB_DEFINE_SMARTPTR_OPENOBJECT_CONSTRUCTOR(_classSuffix_)                                                         \
    DwgDb##_classSuffix_##Ptr::DwgDb##_classSuffix_##Ptr(DwgDbObjectId id, DwgDbOpenMode mode, bool openErased, bool openLocked)    \
        {                                                                                                                           \
        IMPLEMENT_SMARTPTR_OPENOBJECT(id, mode, openErased, openLocked);                                                            \
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
        IMPLEMENT_SMARTPTR_OPENOBJECT(id, mode, openErased, openLocked);                                                        \
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

// create a new super DbObject which can then be later saved as a DB resident
#define DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)                                       \
    DwgDb##_classSuffix_##P DwgDb##_classSuffix_##::Create()                            \
        { return DwgDb##_classSuffix_##::Cast(new DWGDB_Type(_classSuffix_)()); }

// define common methods for DbObject derivitives
#define DWGDB_OBJECT_DEFINE_MEMBERS(_classSuffix_)                                                                                                                              \
    DwgDbObjectId      DwgDb##_classSuffix_##::GetObjectId () const { return T_Super::objectId(); }                                                                             \
    DwgDbObjectId      DwgDb##_classSuffix_##::GetOwnerId () const { return T_Super::ownerId(); }                                                                               \
    DwgDbDatabasePtr   DwgDb##_classSuffix_##::GetDatabase () const { return T_Super::database(); }                                                                             \
    DwgDbStatus        DwgDb##_classSuffix_##::UpgradeOpen () { DWGDB_CALLSDKMETHOD(upgradeOpen(); return DwgDbStatus::Success;, return ToDwgDbStatus(upgradeOpen());) }        \
    DwgDbStatus        DwgDb##_classSuffix_##::DowngradeOpen () { DWGDB_CALLSDKMETHOD(downgradeOpen(); return DwgDbStatus::Success;, return ToDwgDbStatus(downgradeOpen());) }  \
    DwgDbStatus        DwgDb##_classSuffix_##::Close () { DWGDB_CALLSDKMETHOD(T_Super::release(); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::close());) }      \
    DwgDbStatus        DwgDb##_classSuffix_##::Erase () { DWGDB_CALLSDKMETHOD(T_Super::release(); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::erase());) }      \
    DwgString          DwgDb##_classSuffix_##::GetDxfName () const { return isA()->dxfName(); }                                                                                 \
    DwgString          DwgDb##_classSuffix_##::GetDwgClassName () const { return isA()->name(); }                                                                               \
    DWGDB_DEFINE_DXFOUTFIELDS_MEMBER(##_classSuffix_##)                                                                                                                         \
    DWGDB_DEFINE_DXFOUT_MEMBER(##_classSuffix_##)                                                                                                                               \
    DWGDB_DEFINE_GETXDATA_MEMBER(##_classSuffix_##)                                                                                                                             \
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
    bool                DwgDb##_classSuffix_##::IsPlanar () const { return T_Super::isPlanar(); }                                          \
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
    void DwgDb##_classSuffix_##::SetDatabaseDefaults (DwgDbDatabaseP dwg) { T_Super::setDatabaseDefaults(dwg); } \
    void DwgDb##_classSuffix_##::SetCastShadows (bool c) { T_Super::setCastShadows(c); } \
    void DwgDb##_classSuffix_##::SetReceiveShadows (bool r) { T_Super::setReceiveShadows(r); } \
    void DwgDb##_classSuffix_##::SetVisibility (DwgDbVisibility v, bool s) { T_Super::setVisibility(DWGDB_CASTTOENUM_DB(Visibility)(v), s); }

// define common methods for DbEntity as above, plus an object Create method
#define DWGDB_ENTITY_DEFINE_MEMBERS(_classSuffix_)                  \
    DWGDB_ENTITY_DEFINE_BASEMEMBERS(_classSuffix_)                  \
    DWGDB_OBJECT_DEFINE_CREATE(_classSuffix_)


void    RegisterDwgDbObjectExtensions (bool beforeValidation);
void    UnRegisterDwgDbObjectExtensions ();
void    RegisterDwgGiExtensions ();
void    UnRegisterDwgGiExtensions ();
