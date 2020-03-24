/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          03/20
+===============+===============+===============+===============+===============+======*/
struct  DwgGcsFactory
{
private:
    DwgImporterR    m_importer;
    DgnGCSPtr       m_targetGcs;
    DgnGCSPtr       m_userGcs;
    DwgDbGeoDataPtr m_dwgGeoData;
    
public:
DwgGcsFactory (DwgImporterR importer) : m_importer(importer)
    {
    m_targetGcs = importer.GetDgnDb().GeoLocation().GetDgnGCS ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateGcsFromUserSourceGcs ()
    {
    if (m_userGcs.IsValid())
        {
        m_targetGcs = m_userGcs;
        m_targetGcs->Store (m_importer.GetDgnDb());
        }
    return  BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateGcsFromDwgGeoData ()
    {
#ifdef WIP_SUPPORT_GCS
    if (m_dwgGeoData.OpenStatus() != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;

    auto coordType = m_dwgGeoData->GetCoordinateSystemType ();
    auto coordDef = m_dwgGeoData->GetCoordinateSystem ();

    m_targetGcs = DgnGCS::CreateGCS (m_importer.GetDgnDb());
    if (m_targetGcs.IsValid())
        return  BentleyStatus::BSIERROR;

    if (acGcs != nullptr)
        delete acGcs;
#endif  // WIP_SUPPORT_GCS

    return  BentleyStatus::BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetDwgOrUserGcs ()
    {
    bool hasSource = false;

    // if DWG has a GCS, use it:
    DwgDbBlockTableRecordPtr    modelspace(m_importer.GetModelSpaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.OpenStatus() == DwgDbStatus::Success)
        {
        DwgDbDictionaryPtr  extDictionary(modelspace->GetExtensionDictionary(), DwgDbOpenMode::ForRead);
        if (extDictionary.OpenStatus() == DwgDbStatus::Success)
            {
            DwgDbObjectId   objectId;
            DwgDbStatus     status = extDictionary->GetIdAt (objectId, L"ACAD_GEOGRAPHICDATA");
            if (status == DwgDbStatus::Success && (status = m_dwgGeoData.OpenObject(objectId, DwgDbOpenMode::ForRead)) == DwgDbStatus::Success)
                hasSource = true;
            }
        }

    if (!hasSource)
        {
        // DWG has no GCS, check bridge params for both output and input GCS's
        iModelBridge::GCSDefinition gcsDef = m_importer.GetOptions().GetOutputGcs ();
        if (gcsDef.m_isValid)
            {
            // Scenario #2-b-i, or #2-b-iii: user has passed in an output GCS
            m_targetGcs = gcsDef.CreateGcs (m_importer.GetDgnDb());
            if (m_targetGcs.IsValid())
                m_targetGcs->Store (m_importer.GetDgnDb());
            }

        // get user input GCS as the source GCS
        if ((gcsDef = m_importer.GetOptions().GetInputGcs()).m_isValid)
            m_userGcs = gcsDef.CreateGcs (m_importer.GetDgnDb());
        hasSource = m_userGcs.IsValid ();
        }

    return  hasSource;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ApplyGcs ()
    {
    /*-----------------------------------------------------------------------------------
    Scenarios of applying GCS:

        1. The target iModel has existing GCS, and there is no source GCS - honor it and do nothing.
        2. iModel has no GCS:
            a) DWG has GCS - convert DWG GCS to target GCS
            b) DWG has no GCS
                i. User has passed in an output GCS but no input GCS - apply user output GCS to the target
                ii. User has passed in a input GCS but not output GCS - apply user input GCS to the target
                iii. User has passed in both input and output GCS's - fall through to case #3 to reproject GCS
        3. Both the target iModel and the source have GCS's - reproject source GCS
    -----------------------------------------------------------------------------------*/
    BentleyStatus   status = BentleyStatus::BSISUCCESS;

#ifdef WIP_SUPPORT_GCS
    // get exiting GCS
    DgnGeoLocation& targetGeoLocation = m_importer.GetDgnDb().GeoLocation ();
    m_targetGcs = targetGeoLocation.GetDgnGCS ();

    // get DWG or user GCS
    bool hasSource = this->GetDwgOrUserGcs ();

    // Scenario #1(iModel has GCS, and there is no source GCS), or Scenario #2-b-i(neither iModel nor DWG has GCS, but user passed in target GCS)
    if (m_targetGcs.IsValid() && !hasSource)
        return  BentleyStatus::BSISUCCESS;

    if (!m_targetGcs.IsValid() && hasSource)
        {
        // Scenario #2-a(iModel has no GCS, but DWG has), or Scenario #2-b-ii(neither iModel nor DWG has GCS, but user has an input GCS)
        if (m_dwgGeoData.OpenStatus() == DwgDbStatus::Success)
            status = this->CreateGcsFromDwgGeoData ();
        else
            status = this->CreateGcsFromUserSourceGcs ();
        
        // apply user ECEF param, only if the iModel does not already have it
        EcefLocation    existingEcef = targetGeoLocation.GetEcefLocation ();
        if (!existingEcef.m_isValid )
            {
            auto userEcef = m_importer.GetOptions().GetEcefLocation ();
            if (userEcef.m_isValid)
                targetGeoLocation.SetEcefLocation (userEcef);
            }

        return  status;
        }

    // Scenario #3 - both target and source have GCS's (including user input & output from Scenario #2-b-iii)
    // WIP reprojection - for now, just use target GCS
#endif  // WIP_SUPPORT_GCS

    return  status;
    }
};  // DwgGcsFactory

END_DWG_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgImporter::ApplyGeoCoordinateSystem ()
    {
    DwgGcsFactory   factory(*this);
    factory.ApplyGcs ();

    DPoint3d    globalOrigin;
    if (this->IsCreatingNewDgnDb())
        {
        // DWG global origin is at 0,0,0
        globalOrigin.FromZero ();
        m_dgndb->GeoLocation().SetGlobalOrigin (globalOrigin);
        m_dgndb->GeoLocation().Save ();
        }
    else
        {
        // adjust models by existing global origin
        globalOrigin = m_dgndb->GeoLocation().GetGlobalOrigin ();
        if (!globalOrigin.IsEqual(DPoint3d::FromZero(), m_sizeTolerance))
            {
            DPoint3d    rootTranslation;
            TransformR  rootTransform = m_rootTransformInfo.GetRootTransformR ();
            rootTransform.GetTranslation (rootTranslation);
            rootTranslation.Add (globalOrigin);
            rootTransform.SetTranslation (rootTranslation);
            }
        }

    }

