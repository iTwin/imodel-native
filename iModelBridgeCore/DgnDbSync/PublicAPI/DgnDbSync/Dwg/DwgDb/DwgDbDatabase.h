/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/DwgDbDatabase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>
#include    <DgnDbSync/Dwg/DwgDb/BasicTypes.h>
#include    <Bentley/RefCounted.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/DbDatabase.h>
#include    <Teigha/Core/Include/summinfo.h>

#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/dbmain.h>
#include    <RealDwg/Base/summinfo.h>

#else
    #error  "Must define DWGTOOLKIT!"
#endif

BEGIN_DWGDB_NAMESPACE

class DwgDbObjectId;


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDatabase : public DWGDB_EXTENDCLASS(Database)
#ifdef DWGTOOLKIT_OpenDwg
    {
    DEFINE_T_SUPER (OdDbDatabase)
public:
    DWGDB_EXPORT DwgDbDatabase(bool buildDefaultDrawing = true, bool noDocument = false) : OdDbDatabase () {}

#elif DWGTOOLKIT_RealDwg
                    , public DwgDbRefCounted<IDwgDbRefCounted>
    {
    DEFINE_T_SUPER (AcDbDatabase)
public:
    DWGDB_EXPORT DwgDbDatabase(bool buildDefaultDrawing = true, bool noDocument = false) : AcDbDatabase (buildDefaultDrawing, noDocument) {}
#endif

    DWGDB_EXPORT DwgDbStatus        GetFileIdPolicy (uint32_t& id, uint32_t& policy);
    DWGDB_EXPORT DwgDbStatus        SetFileIdPolicy (uint32_t id, uint32_t policy);
    DWGDB_EXPORT DwgDbObjectId      GetModelspaceId ();
    DWGDB_EXPORT DwgDbObjectId      GetPaperspaceId ();
    DWGDB_EXPORT DwgDbObjectId      GetBlockTableId () const;
    DWGDB_EXPORT DwgDbObjectId      GetLayerTableId () const;
    DWGDB_EXPORT DwgDbObjectId      GetLayer0Id () const;
    DWGDB_EXPORT DwgDbObjectId      GetLayerDefpointsId ();
    DWGDB_EXPORT DwgDbObjectId      GetLinetypeTableId () const;
    DWGDB_EXPORT DwgDbObjectId      GetTextStyleTableId () const;
    DWGDB_EXPORT DwgDbObjectId      GetViewportTableId () const;
    DWGDB_EXPORT DwgString          GetFileName () const;
    DWGDB_EXPORT double             GetANGBASE () const;
    DWGDB_EXPORT bool               GetANGDIR () const;
    DWGDB_EXPORT DwgDbAngularUnits  GetAUNITS () const;
    DWGDB_EXPORT DwgCmColor         GetCECOLOR () const;
    DWGDB_EXPORT DwgDbObjectId      GetCELTYPE () const;
    DWGDB_EXPORT DwgDbLineWeight    GetCELWEIGHT () const;
    DWGDB_EXPORT DwgDbObjectId      GetCLAYER () const;
    DWGDB_EXPORT double             GetCANNOSCALE () const;
    DWGDB_EXPORT int16_t            GetAUPREC () const;
    DWGDB_EXPORT double             GetLTSCALE () const;
    DWGDB_EXPORT DwgDbLUnitFormat   GetLUNITS () const;
    DWGDB_EXPORT int16_t            GetLUPREC () const;
    DWGDB_EXPORT DwgDbUnits         GetINSUNITS () const;
    DWGDB_EXPORT double             GetPLINEWID () const;
    DWGDB_EXPORT bool               GetMSLTSCALE () const;      // Linetypes in modelspace scaled by annotation scale?
    DWGDB_EXPORT bool               GetPSLTSCALE () const;      // Linetypes in paperspace scaled by viewports?
    DWGDB_EXPORT DwgDbDate          GetTDUPDATE () const;
    DWGDB_EXPORT DwgDbDate          GetTDUUPDATE () const;
    DWGDB_EXPORT bool               GetFILLMODE () const;
    DWGDB_EXPORT DPoint3d           GetEXTMIN () const;
    DWGDB_EXPORT DPoint3d           GetEXTMAX () const;
    DWGDB_EXPORT DPoint3d           GetPEXTMIN () const;
    DWGDB_EXPORT DPoint3d           GetPEXTMAX () const;
    DWGDB_EXPORT DPoint2d           GetLIMMIN () const;
    DWGDB_EXPORT DPoint2d           GetLIMMAX () const;
    DWGDB_EXPORT DPoint2d           GetPLIMMIN () const;
    DWGDB_EXPORT DPoint2d           GetPLIMMAX () const;
    DWGDB_EXPORT int16_t            GetPDMODE () const;
    DWGDB_EXPORT DwgDbStatus        SetPDMODE (int16_t);
    DWGDB_EXPORT double             GetPDSIZE () const;
    DWGDB_EXPORT DwgDbStatus        SetPDSIZE (double);
    DWGDB_EXPORT size_t             GetISOLINES () const;
    DWGDB_EXPORT double             GetTEXTSIZE () const;
    DWGDB_EXPORT DwgDbObjectId      GetTEXTSTYLE () const;
    DWGDB_EXPORT bool               GetLineweightDisplay () const;
    DWGDB_EXPORT DwgDbObjectId      GetActiveUserViewportId ();         // active viewport entity in active layout
    DWGDB_EXPORT DwgDbObjectId      GetActiveModelspaceViewportId ();   // active viewport table record in modelspace
    DWGDB_EXPORT DwgString          GetFingerprintGuid () const;
    DWGDB_EXPORT DwgString          GetVersionGuid () const;
    DWGDB_EXPORT DwgDbObjectId      GetLinetypeByBlockId () const;
    DWGDB_EXPORT DwgDbObjectId      GetLinetypeByLayerId () const;
    DWGDB_EXPORT DwgDbObjectId      GetLinetypeContinuousId () const;
    DWGDB_EXPORT DwgDbObjectId      GetMaterialDictionaryId () const;
    DWGDB_EXPORT DwgDbObjectId      GetMaterialByLayerId () const;
    DWGDB_EXPORT DwgDbObjectId      GetMaterialByBlockId () const;
    DWGDB_EXPORT DwgDbObjectId      GetMaterialGlobalId () const;
    DWGDB_EXPORT DwgDbObjectId      GetObjectId (DwgDbHandleCR handle);
    };  // DwgDbDatabase

/*=================================================================================**//**
* DwgDbDatabasePtr is a true refcounted pointer(unlike other DwgDb ptr's due to RealDWG limitation)!
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDatabasePtr
#ifdef DWGTOOLKIT_OpenDwg
                    : public OdSmartPtr<DwgDbDatabase>
    {
    DEFINE_T_SUPER (OdSmartPtr)
public:
    DWGDB_ADD_SMARTPTR_CONSTRUCTORS(DbDatabase)

    DWGDB_EXPORT bool               IsNull () const { return T_Super::isNull(); }
    DWGDB_EXPORT bool               IsValid() const { return !T_Super::isNull(); }

#elif DWGTOOLKIT_RealDwg
                    : public RefCountedPtr<DwgDbDatabase>
    {
    DEFINE_T_SUPER (RefCountedPtr)
public:
    DwgDbDatabasePtr () : RefCountedPtr() {}
    DwgDbDatabasePtr (DwgDbDatabaseP dwg) : RefCountedPtr(dwg) {}
    DwgDbDatabasePtr (AcDbDatabase* dwg) : RefCountedPtr(dynamic_cast<DwgDbDatabaseP>(dwg)) {}

#endif  // DWGTOOLKIT_

    };  // DwgDbDatabasePtr


END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
