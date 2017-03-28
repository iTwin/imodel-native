/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/DataCaptureDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DataCaptureSchemaInternal.h>

DOMAIN_DEFINE_MEMBERS(DataCaptureDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataCaptureDomain::DataCaptureDomain() : DgnDomain(BDCP_SCHEMA_NAME, "Bentley DataCapture Domain", 1)
    {
    RegisterHandler(RadialDistortionHandler::GetHandler());
    RegisterHandler(TangentialDistortionHandler::GetHandler());
    RegisterHandler(CameraDeviceHandler::GetHandler());    
    RegisterHandler(CameraDeviceModelHandler::GetHandler());
    RegisterHandler(ShotHandler::GetHandler());
    RegisterHandler(PoseHandler::GetHandler());
    RegisterHandler(GimbalAngleRangeHandler::GetHandler());
    RegisterHandler(GimbalHandler::GetHandler());
    RegisterHandler(DroneHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    InsertCategory(dgndb, ColorDef::White(), 1, BDCP_CATEGORY_AcquisitionDevice);

    //Insert Shot category with it's own color and weight
    InsertCategory(dgndb, Shot::GetDefaultColor(), Shot::GetDefaultWeight(), BDCP_CATEGORY_Shot);

    auto authority = NamespaceAuthority::CreateNamespaceAuthority(BDCP_AUTHORITY_DataCapture, dgndb);
    BeAssert(authority.IsValid());
    if (authority.IsValid())
        {
        authority->Insert();
        BeAssert(authority->GetAuthorityId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DataCaptureDomain::InsertCategory(DgnDbR db, ColorDef const& color, int const weight, Utf8CP code)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    appearance.SetInvisible(false);
    appearance.SetWeight(weight);

    DgnCategory category(DgnCategory::CreateParams(db, code, DgnCategory::Scope::Physical, DgnCategory::Rank::Domain));
    category.Insert(appearance);

    BeAssert(category.GetCategoryId().IsValid());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DataCaptureDomain::QueryDataCaptureAuthorityId(DgnDbCR dgndb)
    {
    DgnAuthorityId authorityId = dgndb.Authorities().QueryAuthorityId(BDCP_AUTHORITY_DataCapture);
    BeAssert(authorityId.IsValid());
    return authorityId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DataCaptureDomain::CreateCode(DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
    {
    BeAssert((nameSpace == BDCP_CLASS_CameraDeviceModel) ||
             (nameSpace == BDCP_CLASS_CameraDevice) ||
             (nameSpace == BDCP_CLASS_Pose)         ||
             (nameSpace == BDCP_CLASS_Shot)         ||
             (nameSpace == BDCP_CLASS_GimbalAngleRange) ||
             (nameSpace == BDCP_CLASS_Gimbal) ||
             (nameSpace == BDCP_CLASS_Drone));

    return NamespaceAuthority::CreateCode(BDCP_AUTHORITY_DataCapture, value, dgndb, nameSpace);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2015
//---------------------------------------------------------------------------------------
Utf8String DataCaptureDomain::FormatId(BeBriefcaseBasedId id)
    {
    Utf8PrintfString formattedId("%" PRIu32 "-%" PRIu64, id.GetBriefcaseId().GetValue(), (uint64_t) (0xffffffffffLL & id.GetValue()));
    return formattedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2015
//---------------------------------------------------------------------------------------
Utf8String DataCaptureDomain::BuildDefaultName(Utf8CP prefix, BeBriefcaseBasedId id)
    {
    Utf8String defaultName(prefix);
    defaultName += "-";
    defaultName += FormatId(id);
    return defaultName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus DataCaptureDomain::UpdateSchema(SchemaUpdateDataCaptureDgnDbParams& params) const
    {
    auto dcDomainCP = params.m_dgnDb->Domains().FindDomain(BDCP_SCHEMA_NAME);

    if (dcDomainCP)
        {         
        // Ignoring VersionDigit2 as it is not completely hooked-up in DgnDb06 by lower layers
        Statement stmt;
        if (DbResult::BE_SQLITE_OK != stmt.Prepare(*params.m_dgnDb, "SELECT VersionDigit1, VersionDigit3 FROM ec_Schema WHERE Name = ?;"))
            return DgnDbStatus::ReadError;

        if (DbResult::BE_SQLITE_OK != stmt.BindText(1, BDCP_SCHEMA_NAME, Statement::MakeCopy::No) ||
            DbResult::BE_SQLITE_ROW != stmt.Step())
            return DgnDbStatus::ReadError;

        int digit1 = stmt.GetValueInt(0);
        int digit3 = stmt.GetValueInt(1);

        if (digit1 != GetExpectedSchemaVersionDigit1() || (uint32_t)digit3 > GetExpectedSchemaVersionDigit3())
            return DgnDbStatus::InvalidSchemaVersion;

        if (digit3 == GetExpectedSchemaVersionDigit3())
            return DgnDbStatus::Success;

        // Finalizing statement early in order to avoid db-locking while updating schema
        stmt.Finalize();
        }

    BeFileName schemaFileName = params.m_assetsRootDir;
    schemaFileName.AppendToPath(BDCP_SCHEMA_LOCATION);
    schemaFileName.AppendToPath(BDCP_SCHEMA_FILE);

    DgnDbStatus retVal;
    if (DgnDbStatus::Success != (retVal = DataCaptureDomain::GetDomain().ImportSchema(*params.m_dgnDb, schemaFileName)))
        return retVal;

    Utf8String schemaUpdateDescr("SAVECHANGES_SchemaUpdate");
    if (DbResult::BE_SQLITE_OK != params.m_dgnDb->SaveChanges(schemaUpdateDescr.c_str()))
        retVal = DgnDbStatus::WriteError;

    return retVal;
    }