/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DbDatabase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgDbInternal.h"

#define     DWGDB_FILEIDPOLICY_KEY          L"FILEIDPOLICY"
#define     DWGDB_FILEIDPOLICY_FORMAT       "%I32u+%I32u"

USING_NAMESPACE_DWGDB


DwgDbObjectId       DwgDbDatabase::GetBlockTableId () const { return DWGDB_CALLSDKMETHOD(T_Super::getBlockTableId, T_Super::blockTableId)(); }
DwgDbObjectId       DwgDbDatabase::GetLayerTableId () const { return DWGDB_CALLSDKMETHOD(T_Super::getLayerTableId, T_Super::layerTableId)(); }
DwgDbObjectId       DwgDbDatabase::GetLayer0Id () const { return DWGDB_CALLSDKMETHOD(T_Super::getLayerZeroId, T_Super::layerZero)(); }
DwgDbObjectId       DwgDbDatabase::GetLayerDefpointsId () { return DWGDB_CALLSDKMETHOD(T_Super::getLayerDefpointsId(false), acdbSymUtil()->layerDefpointsId(this)); }
DwgDbObjectId       DwgDbDatabase::GetLinetypeTableId () const { return DWGDB_CALLSDKMETHOD(T_Super::getLinetypeTableId, T_Super::linetypeTableId)(); }
DwgDbObjectId       DwgDbDatabase::GetTextStyleTableId () const { return DWGDB_CALLSDKMETHOD(T_Super::getTextStyleTableId, T_Super::textStyleTableId)(); }
DwgDbObjectId       DwgDbDatabase::GetViewportTableId () const { return DWGDB_CALLSDKMETHOD(T_Super::getViewportTableId, T_Super::viewportTableId)(); }
DwgDbObjectId       DwgDbDatabase::GetModelspaceId () { return DWGDB_CALLSDKMETHOD(T_Super::getModelSpaceId(), acdbSymUtil()->blockModelSpaceId(this)); }
DwgDbObjectId       DwgDbDatabase::GetPaperspaceId () { return DWGDB_CALLSDKMETHOD(T_Super::getPaperSpaceId(), acdbSymUtil()->blockPaperSpaceId(this)); }
double              DwgDbDatabase::GetANGBASE () const { return DWGDB_CALLSDKMETHOD(T_Super::getANGBASE(), T_Super::angbase()); }
bool                DwgDbDatabase::GetANGDIR () const { return DWGDB_CALLSDKMETHOD(T_Super::getANGDIR(), T_Super::angdir()); }
DwgDbAngularUnits   DwgDbDatabase::GetAUNITS () const { return DWGDB_UPWARDCAST(AngularUnits)(DWGDB_CALLSDKMETHOD(T_Super::getAUNITS, T_Super::aunits)()); }
int16_t             DwgDbDatabase::GetAUPREC () const { return DWGDB_CALLSDKMETHOD(T_Super::getAUPREC(), T_Super::auprec()); }
DwgCmColor          DwgDbDatabase::GetCECOLOR () const { return DWGDB_CALLSDKMETHOD(T_Super::getCECOLOR(), T_Super::cecolor()); }
DwgDbObjectId       DwgDbDatabase::GetCELTYPE () const { return DWGDB_CALLSDKMETHOD(T_Super::getCELTYPE(), T_Super::celtype()); }
DwgDbLineWeight     DwgDbDatabase::GetCELWEIGHT () const { return static_cast<DwgDbLineWeight>(DWGDB_CALLSDKMETHOD(T_Super::getCELWEIGHT(), T_Super::celweight())); }
DwgDbObjectId       DwgDbDatabase::GetCLAYER () const { return DWGDB_CALLSDKMETHOD(T_Super::getCLAYER(), T_Super::clayer()); }
double              DwgDbDatabase::GetLTSCALE () const { return DWGDB_CALLSDKMETHOD(T_Super::getLTSCALE(), T_Super::ltscale()); }
DwgDbLUnitFormat    DwgDbDatabase::GetLUNITS () const { return DWGDB_UPWARDCAST(LUnitFormat)(DWGDB_CALLSDKMETHOD(T_Super::getLUNITS, T_Super::lunits)()); }
int16_t             DwgDbDatabase::GetLUPREC () const { return DWGDB_CALLSDKMETHOD(T_Super::getLUPREC(), T_Super::luprec()); }
double              DwgDbDatabase::GetPLINEWID () const { return DWGDB_CALLSDKMETHOD(T_Super::getPLINEWID, T_Super::plinewid)(); }
bool                DwgDbDatabase::GetMSLTSCALE () const { return DWGDB_CALLSDKMETHOD(T_Super::getMSLTSCALE, T_Super::msltscale)(); }
bool                DwgDbDatabase::GetPSLTSCALE () const { return DWGDB_CALLSDKMETHOD(T_Super::getPSLTSCALE, T_Super::psltscale)(); }
DwgDbDate           DwgDbDatabase::GetTDUPDATE () const { return DWGDB_CALLSDKMETHOD(T_Super::getTDUPDATE, T_Super::tdupdate)(); }
DwgDbDate           DwgDbDatabase::GetTDUUPDATE () const { return DWGDB_CALLSDKMETHOD(T_Super::getTDUUPDATE, T_Super::tduupdate)(); }
bool                DwgDbDatabase::GetFILLMODE () const { return DWGDB_CALLSDKMETHOD(T_Super::getFILLMODE, T_Super::fillmode)(); }
DPoint3d            DwgDbDatabase::GetEXTMIN () const { return Util::DPoint3dFrom(DWGDB_CALLSDKMETHOD(T_Super::getEXTMIN, T_Super::extmin)()); }
DPoint3d            DwgDbDatabase::GetEXTMAX () const { return Util::DPoint3dFrom(DWGDB_CALLSDKMETHOD(T_Super::getEXTMAX, T_Super::extmax)()); }
DPoint3d            DwgDbDatabase::GetPEXTMIN () const { return Util::DPoint3dFrom(DWGDB_CALLSDKMETHOD(T_Super::getPEXTMIN, T_Super::pextmin)()); }
DPoint3d            DwgDbDatabase::GetPEXTMAX () const { return Util::DPoint3dFrom(DWGDB_CALLSDKMETHOD(T_Super::getPEXTMAX, T_Super::pextmax)()); }
DPoint2d            DwgDbDatabase::GetLIMMIN () const { return Util::DPoint2dFrom(DWGDB_CALLSDKMETHOD(T_Super::getLIMMIN, T_Super::limmin)()); }
DPoint2d            DwgDbDatabase::GetLIMMAX () const { return Util::DPoint2dFrom(DWGDB_CALLSDKMETHOD(T_Super::getLIMMAX, T_Super::limmax)()); }
DPoint2d            DwgDbDatabase::GetPLIMMIN () const { return Util::DPoint2dFrom(DWGDB_CALLSDKMETHOD(T_Super::getPLIMMIN, T_Super::plimmin)()); }
DPoint2d            DwgDbDatabase::GetPLIMMAX () const { return Util::DPoint2dFrom(DWGDB_CALLSDKMETHOD(T_Super::getPLIMMAX, T_Super::plimmax)()); }
int16_t             DwgDbDatabase::GetPDMODE () const { return DWGDB_CALLSDKMETHOD(T_Super::getPDMODE, T_Super::pdmode)(); }
DwgDbStatus         DwgDbDatabase::SetPDMODE (int16_t m) { DWGDB_CALLSDKMETHOD(T_Super::setPDMODE(m); return DwgDbStatus::Success;, return ToDwgDbStatus(T_Super::setPdmode(m))); }
double              DwgDbDatabase::GetPDSIZE () const { return DWGDB_CALLSDKMETHOD(T_Super::getPDSIZE, T_Super::pdsize)(); }
DwgDbStatus         DwgDbDatabase::SetPDSIZE (double s) { DWGDB_CALLSDKMETHOD(T_Super::setPDSIZE(s); return DwgDbStatus::Success, return ToDwgDbStatus(T_Super::setPdsize(s))); }
size_t              DwgDbDatabase::GetISOLINES () const { return DWGDB_CALLSDKMETHOD(T_Super::getISOLINES, T_Super::isolines)(); }
double              DwgDbDatabase::GetTEXTSIZE () const { return DWGDB_CALLSDKMETHOD(T_Super::getTEXTSIZE, T_Super::textsize)(); }
DwgDbObjectId       DwgDbDatabase::GetTEXTSTYLE () const { return DWGDB_CALLSDKMETHOD(T_Super::getTEXTSTYLE, T_Super::textstyle)(); }
bool                DwgDbDatabase::GetLineweightDisplay () const { return DWGDB_CALLSDKMETHOD(T_Super::getLWDISPLAY, T_Super::lineWeightDisplay)(); }
DwgDbObjectId       DwgDbDatabase::GetLinetypeByBlockId () const { return DWGDB_CALLSDKMETHOD(T_Super::getLinetypeByBlockId(), T_Super::byBlockLinetype()); }
DwgDbObjectId       DwgDbDatabase::GetLinetypeByLayerId () const { return DWGDB_CALLSDKMETHOD(T_Super::getLinetypeByLayerId(), T_Super::byLayerLinetype()); }
DwgDbObjectId       DwgDbDatabase::GetLinetypeContinuousId () const { return DWGDB_CALLSDKMETHOD(T_Super::getLinetypeContinuousId(), T_Super::continuousLinetype()); }
DwgDbObjectId       DwgDbDatabase::GetMaterialDictionaryId () const { return DWGDB_CALLSDKMETHOD(T_Super::getMaterialDictionaryId(), T_Super::materialDictionaryId()); }
DwgDbObjectId       DwgDbDatabase::GetMaterialByLayerId () const { return DWGDB_CALLSDKMETHOD(T_Super::byLayerMaterialId(), T_Super::byLayerMaterial()); }
DwgDbObjectId       DwgDbDatabase::GetMaterialByBlockId () const { return DWGDB_CALLSDKMETHOD(T_Super::byBlockMaterialId(), T_Super::byBlockMaterial()); }
DwgDbObjectId       DwgDbDatabase::GetMaterialGlobalId () const { return DWGDB_CALLSDKMETHOD(T_Super::globalMaterialId(), T_Super::globalMaterial()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId       DwgDbDatabase::GetObjectId (DwgDbHandleCR handle)
    {
    // find existing objectId from this database:
    DwgDbObjectId   objectId;
#ifdef DWGTOOLKIT_OpenDwg
    objectId = T_Super::getOdDbObjectId (handle);
#elif DWGTOOLKIT_RealDwg
    if (Acad::eOk != T_Super::getAcDbObjectId(objectId, false, handle))
        objectId.SetNull ();
#endif
    return  objectId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId       DwgDbDatabase::GetActiveUserViewportId ()
    {
    DwgDbObjectId   anyvportId;
#ifdef DWGTOOLKIT_OpenDwg
    anyvportId = this->activeViewportId ();
#elif DWGTOOLKIT_RealDwg
    if (Acad::eOk != acdbGetCurUserViewportId(this, anyvportId))
        anyvportId = DwgDbObjectId ();
#endif
    return  anyvportId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId       DwgDbDatabase::GetActiveModelspaceViewportId ()
    {
    DwgDbObjectId   vportTableRecordId;
#ifdef DWGTOOLKIT_OpenDwg
    vportTableRecordId = this->activeViewportId ();
    OdDbObjectPtr   obj = vportTableRecordId.safeOpenObject ();
    if (obj.isNull() || !obj->isKindOf(OdDbViewportTableRecord::desc()))
        {
        // return the
        OdDbViewportTablePtr    vportTable = this->getViewportTableId().safeOpenObject ();
        vportTableRecordId = vportTable.isNull() ? DwgDbObjectId() : vportTable->getActiveViewportId();
        }
#elif DWGTOOLKIT_RealDwg

    vportTableRecordId = acdbGetCurVportTableRecordId (this);
    if (!vportTableRecordId.isValid())
        {
        AcDbViewportTableIterator*  iterator = nullptr;
        AcDbViewportTablePointer    vportTable (this->viewportTableId(), AcDb::kForRead);
        if (Acad::eOk == vportTable.openStatus() && Acad::eOk == vportTable->newIterator(iterator))
            {
            for (iterator->start(); !iterator->done(); iterator->step())
                {
                AcDbObjectId    viewportId;
                if (Acad::eOk != iterator->getRecordId(viewportId))
                    continue;

                AcDbViewportTableRecordPointer  vport (viewportId, AcDb::kForRead);
                if (Acad::eOk != vport.openStatus())
                    continue;

                ACHAR const*    vportName = nullptr;
                if (Acad::eOk != vport->getName(vportName) || 0 != wcsicmp(vportName, L"*active"))
                    continue;

                // the 1st entry that has name *ACTIVE is the effective active viewport table:
                vportTableRecordId = vport->objectId ();
                break;
                }
            }
        }
#endif

    return  vportTableRecordId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgDbDatabase::GetCANNOSCALE () const
    {
    double  scale = 1.0;
#ifdef DWGTOOLKIT_OpenDwg
    OdDbAnnotationScalePtr  annoscale = T_Super::cannoscale ();
    if (!annoscale.isNull())
        annoscale->getScale (scale);
#elif DWGTOOLKIT_RealDwg
    AcDbAnnotationScale*    annoscale = T_Super::cannoscale ();
    if (nullptr != annoscale)
        {
        annoscale->getScale (scale);
        delete annoscale;
        }
#endif
    // ensure to return a non-zero value:
    if (scale < 1.0e-5)
        scale = 1.0;

    return  scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbUnits          DwgDbDatabase::GetINSUNITS () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    OdResBufPtr     unitsResbuf = this->getSysVar (L"INSUNITS");
    if (unitsResbuf.isNull())
        return  DwgDbUnits::Undefined;
    else
        return  static_cast<DwgDbUnits> (unitsResbuf->getInt16());
#elif DWGTOOLKIT_RealDwg
    return DWGDB_UPWARDCAST(Units) (this->insunits());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbDatabase::GetFileName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return this->getFilename ();
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    name = nullptr;
    if (Acad::eOk == this->getFilename(name) && nullptr != name && name[0] != 0)
        return  name;
    return  nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbDatabase::SetFileIdPolicy (uint32_t id, uint32_t policy)
    {
    char    keynvalue[248];
    sprintf (keynvalue, DWGDB_FILEIDPOLICY_FORMAT, id, policy);

#ifdef DWGTOOLKIT_OpenDwg
    OdDbDatabaseSummaryInfoPtr  summaryInfo = oddbGetSummaryInfo (this);
    if (!summaryInfo.isNull())
        {
        summaryInfo->setCustomSummaryInfo (DWGDB_FILEIDPOLICY_KEY, OdString(keynvalue));

        oddbPutSummaryInfo (summaryInfo);

        return  DwgDbStatus::Success;
        }

#elif DWGTOOLKIT_RealDwg
    AcDbDatabaseSummaryInfo*    summaryInfo = nullptr;
    if (Acad::eOk == acdbGetSummaryInfo(this, summaryInfo) && nullptr != summaryInfo)
        {
        AcString                idAndPolicy(keynvalue);
        Acad::ErrorStatus       es = summaryInfo->setCustomSummaryInfo (DWGDB_FILEIDPOLICY_KEY, idAndPolicy.kwszPtr());
        if (Acad::eKeyNotFound == es)
            es = summaryInfo->addCustomSummaryInfo (DWGDB_FILEIDPOLICY_KEY, idAndPolicy.kwszPtr());
        
        if (Acad::eOk == es)
            es = acdbPutSummaryInfo (summaryInfo);

        delete summaryInfo;

        return  ToDwgDbStatus(es);
        }
#endif

    return  DwgDbStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbDatabase::GetFileIdPolicy (uint32_t& id, uint32_t& policy)
    {
    id = policy = 0;

#ifdef DWGTOOLKIT_OpenDwg
    OdDbDatabaseSummaryInfoPtr  summaryInfo = oddbGetSummaryInfo (this);
    if (!summaryInfo.isNull())
        {
        OdString    value;
        if (summaryInfo->getCustomSummaryInfo(DWGDB_FILEIDPOLICY_KEY, value) && !value.isEmpty())
            {
            sscanf (static_cast<const char*>(value), DWGDB_FILEIDPOLICY_FORMAT, &id, &policy);

            return  DwgDbStatus::Success;
            }
        }

#elif DWGTOOLKIT_RealDwg
    AcDbDatabaseSummaryInfo*    summaryInfo = nullptr;
    if (Acad::eOk == acdbGetSummaryInfo(this, summaryInfo) && nullptr != summaryInfo)
        {
        ACHAR*              value = nullptr;
        Acad::ErrorStatus   es = summaryInfo->getCustomSummaryInfo (DWGDB_FILEIDPOLICY_KEY, value);

        if (Acad::eOk == es && nullptr != value && 0 != value[0])
            {
            AcString    str(value);
#if VendorVersion < 2017
            ::sscanf (str.kszPtr(), DWGDB_FILEIDPOLICY_FORMAT, &id, &policy);
#else
            ::sscanf (str.utf8Ptr(), DWGDB_FILEIDPOLICY_FORMAT, &id, &policy);
#endif
            }
        
        delete summaryInfo;

        return  ToDwgDbStatus(es);
        }
#endif

    return  DwgDbStatus::UnknownError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbDatabase::GetFingerprintGuid () const
    {
    DwgString   fingerprintGuid;
#if DWGTOOLKIT_OpenDwg
    OdFileDependencyManagerPtr  filedep = this->fileDependencyManager ();
    if (filedep->countEntries() > 0)
        {
        // extract the first entry
        OdFileDependencyInfoPtr fileinfo;
        if (OdResult::eOk == filedep->getEntry(0, fileinfo))
            fingerprintGuid = fileinfo->m_FingerprintGuid;
        }
#elif DWGTOOLKIT_RealDwg
    ACHAR*      guidValue = nullptr;
    if (Acad::eOk == this->getFingerprintGuid(guidValue))
        {
        fingerprintGuid.assign (guidValue);
        acutDelString (guidValue);
        }
#endif
    return  fingerprintGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbDatabase::GetVersionGuid () const
    {
    DwgString   versionGuid;
#if DWGTOOLKIT_OpenDwg
    OdFileDependencyManagerPtr  filedep = this->fileDependencyManager ();
    if (filedep->countEntries() > 0)
        {
        // extract the first entry
        OdFileDependencyInfoPtr fileinfo;
        if (OdResult::eOk == filedep->getEntry(0, fileinfo))
            versionGuid = fileinfo->m_VersionGuid;
        }
#elif DWGTOOLKIT_RealDwg
    ACHAR*      guidValue = nullptr;
    if (Acad::eOk == this->getVersionGuid(guidValue))
        {
        versionGuid.assign (guidValue);
        acutDelString (guidValue);
        }
#endif
    return  versionGuid;
    }
