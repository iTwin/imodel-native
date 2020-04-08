/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

using namespace BentleyApi::GeoCoordinates;

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
BentleyStatus   LinearTransformModelToGcs ()
    {
    // get the model origin in GCS's coord sys
    DPoint3d    gcsOrigin;
    auto  dwgStatus = m_dwgGeoData->TransformFromLonLatAlt (gcsOrigin, DPoint3d::FromZero());

    if (dwgStatus == DwgDbStatus::Success)
        {
        auto& rootTransform = m_importer.GetRootTransformR ();
        auto translation = rootTransform.Origin ();
        auto azimuth = m_dwgGeoData->GetNorthDirection ();

        // set rotation matrix
        if (fabs(azimuth) > 0.01)
            {
            auto rotation = RotMatrix::FromAxisAndRotationAngle (2, azimuth);
            auto matrix = rootTransform.Matrix ();

            rotation.Multiply (gcsOrigin);
            matrix.InitProduct (matrix, rotation);
            
            rootTransform.SetMatrix (matrix);
            }

        // units in Meters
        gcsOrigin.Scale (m_importer.GetScaleToMeters());

        // set translation
        translation.Subtract (gcsOrigin);
        rootTransform.SetTranslation (translation);
        }

    return  static_cast<BentleyStatus>(dwgStatus);
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
BentleyStatus   InitGcsFromAzimuthalEqualArea (WStringR message, DwgDbGeoCoordinateSystemCR dwgGcs)
    {
    StatusInt   gcsStatus = BentleyStatus::BSIERROR;
    if (!m_targetGcs.IsValid())
        return  static_cast<BentleyStatus>(gcsStatus);

    DwgDbGeoCoordinateSystem::Datum datum;
    auto dwgStatus = dwgGcs.GetDatum (datum);

    WCharCP datumName = dwgStatus == DwgDbStatus::Success ? datum.m_id.c_str() : nullptr;
    if (datumName == nullptr)
        return  static_cast<BentleyStatus>(gcsStatus);

    DwgDbUnits  units;
    WString     unitName;

    if ((dwgStatus = dwgGcs.GetUnit(units)) == DwgDbStatus::Success)
        gcsStatus = DgnGCS::GetCSUnitName(unitName, UnitDefinition::GetStandardUnit(DwgHelper::GetStandardUnitFromDwgUnit(units)));
    if (gcsStatus != BentleyStatus::BSISUCCESS)
        unitName.assign (L"Meter");

    DPoint3d    geoOrigin;
    dwgStatus = m_dwgGeoData->TransformToLonLatAlt (geoOrigin, DPoint3d::FromZero());
    if (dwgStatus != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;

    // set Azimuth
    double azimuth = m_dwgGeoData->GetNorthDirection ();
    azimuth = -Angle::RadiansToDegrees (azimuth);

    // set scale
    double scale = 0.0;
    dwgStatus = dwgGcs.GetUnitScale (scale);
    if (dwgStatus != DwgDbStatus::Success)
        scale = 1.0;

    int quadrant = 1;

#ifdef DEBUG_PROJECTION_PARAMETERS

    DwgDbGeoCoordinateSystem::ProjectionCode    code;
    dwgStatus = dwgGcs.GetProjectionCode (code);

    DVec2d  offset;
    dwgStatus = dwgGcs.GetOffset (offset);

    auto designPoint = m_dwgGeoData->GetDesignPoint ();
    auto northVector = m_dwgGeoData->GetNorthDirectionVector ();
    auto upDirection = m_dwgGeoData->GetUpDirection ();
    double elevation = m_dwgGeoData->GetSeaLevelElevation ();

    DwgDbGeoCoordinateSystem::Ellipsoid ellipsoid;
    dwgStatus = dwgGcs.GetEllipsoid (ellipsoid);

    double  originLongitude = 0.0, originLatitude = 0.0, naturalLongitude = 0.0, standardParallel = 0.0, falseEasting = 0.0, falseNorthing = 0.0, scaleReduction = 0.0;
    DwgDbGeoCoordinateSystem::ProjectionParameters  projParams;

    if ((dwgStatus = dwgGcs.GetProjectionParameters(projParams, true)) == DwgDbStatus::Success)
        {
        for (auto param : projParams)
            {
            if (param.m_name.EqualsI(L"Origin Longitude"))
                originLongitude = param.m_value;
            else if (param.m_name.EqualsI(L"Origin Latitude"))
                originLatitude = param.m_value;
            if (param.m_name.EqualsI(L"Natural Longitude"))
                naturalLongitude = param.m_value;
            else if (param.m_name.EqualsI(L"Standard Parallel"))
                standardParallel = param.m_value;
            else if (param.m_name.EqualsI(L"False Easting"))
                falseEasting = param.m_value;
            else if (param.m_name.EqualsI(L"False Northing"))
                falseNorthing = param.m_value;
            else if (param.m_name.EqualsI(L"Scale Reduction"))
                scaleReduction = param.m_value;
            }
        }
#endif

    gcsStatus = m_targetGcs->InitAzimuthalEqualArea (&message, datumName, unitName.c_str(), geoOrigin.x, geoOrigin.y, azimuth, scale, 0.0, 0.0, quadrant);

    return  static_cast<BentleyStatus>(gcsStatus);
    }

#ifdef DEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
void DumpWellKnownText (DwgDbGeoCoordinateSystemCR dwgGcs, DwgStringCR defXml)
    {
    DwgString   wellknownText;
    if (dwgGcs.GetWktRepresentation(wellknownText) != DwgDbStatus::Success)
        return;

    auto filePath = m_importer.GetDgnDb().GetFileName().GetDirectoryName ();
    filePath.AppendToPath (L"WellKnownTextGcs.dump");
    filePath.BeDeleteFile ();

    BeFileStatus    status;
    auto dumpFile = BeTextFile::Open (status, filePath, TextFileOpenType::Write, TextFileOptions::None);
    if (status != BeFileStatus::Success || !dumpFile.IsValid())
        return;

    dumpFile->PutLine (wellknownText.c_str(), true);
    dumpFile->Close ();

    filePath = m_importer.GetDgnDb().GetFileName().GetDirectoryName ();
    filePath.AppendToPath (L"GCSDefinition.xml");
    filePath.BeDeleteFile ();

    dumpFile = BeTextFile::Open (status, filePath, TextFileOpenType::Write, TextFileOptions::None);
    if (status != BeFileStatus::Success || !dumpFile.IsValid())
        return;

    dumpFile->PutLine (defXml.c_str(), true);
    dumpFile->Close ();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateGcsFromDwgGeoData ()
    {
    if (m_dwgGeoData.OpenStatus() != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;

    auto coordType = m_dwgGeoData->GetCoordinateSystemType ();
    auto coordDef = m_dwgGeoData->GetCoordinateSystem ();

    DwgDbGeoCoordinateSystemPtr dwgGcs;
    auto dwgStatus = DwgDbGeoCoordinateSystem::Create (dwgGcs, coordDef);
    if (dwgStatus != DwgDbStatus::Success)
        return  static_cast<BentleyStatus>(dwgStatus);

    DwgDbGeoCoordinateSystem::Type gcsType = DwgDbGeoCoordinateSystem::Type::Unknown;
    if ((dwgStatus = dwgGcs->GetType(gcsType)) != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;

    DwgString   gcsName;
    if ((dwgStatus = dwgGcs->GetId(gcsName)) != DwgDbStatus::Success)
        return  static_cast<BentleyStatus>(dwgStatus);

    m_targetGcs = DgnGCS::CreateGCS (m_importer.GetDgnDb());
    if (!m_targetGcs.IsValid())
        return  BentleyStatus::BSIERROR;

#ifdef DEBUG
    this->DumpWellKnownText (*dwgGcs, coordDef.c_str());
#endif

    StatusInt   gcsStatus = BentleyStatus::BSIERROR, warning = BentleyStatus::BSISUCCESS;
    WString     message;

    if (coordType == DwgDbGeoData::Grid || coordType == DwgDbGeoData::GeoGraphic)
        {
        BaseGCS::WktFlavor  wktFlavor = BaseGCS::wktFlavorAutodesk;
        DwgString   wellknownText;

        // try "well known text"
        if ((dwgStatus = dwgGcs->GetWktRepresentation(wellknownText)) == DwgDbStatus::Success)
            gcsStatus = m_targetGcs->InitFromWellKnownText (&warning, &message, wktFlavor, wellknownText.c_str());

        // try EPSG code
        int epsgCode = 0;
        if (dwgStatus != DwgDbStatus::Success && (dwgStatus = dwgGcs->GetEpsgCode(epsgCode)) == DwgDbStatus::Success)
            gcsStatus = m_targetGcs->InitFromEPSGCode (&warning, &message, epsgCode);
        }
    else if (gcsType == DwgDbGeoCoordinateSystem::Type::Projected && coordType == DwgDbGeoData::Local)
        {
#ifdef WIP_REPROJECT_GCS
        dwgStatus = dwgGcs->GetEpsgCode (epsgCode);
        if (dwgStatus == DwgDbStatus::Success)
            {
            gcsStatus = m_targetGcs->InitFromEPSGCode (&warning, &message, epsgCode);
            if (gcsStatus == BentleyStatus::SUCCESS)
                gcsStatus = this->LinearTransformModelToGcs ();
            }
        else
            {
            // try projection by Azimuthal equal area
            gcsStatus = m_targetGcs->SetFromCSName (gcsName.c_str());
            gcsStatus = this->InitGcsFromAzimuthalEqualArea (message, *dwgGcs);
            }
#endif
        }
    else
        {
        BeAssert (false && "Unsupported GCS type!");
        }

    // save GCS to dgndb
    if (gcsStatus == BentleyStatus::BSISUCCESS)
        gcsStatus = m_targetGcs->Store (m_importer.GetDgnDb());

    return  static_cast<BentleyStatus>(gcsStatus);
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
                {
                targetGeoLocation.SetEcefLocation (userEcef);
                targetGeoLocation.Save ();
                }
            }

        return  status;
        }

    // Scenario #3 - both target and source have GCS's (including user input & output from Scenario #2-b-iii)
    // WIP reprojection - for now, just use target GCS

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
        globalOrigin.Zero ();
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

