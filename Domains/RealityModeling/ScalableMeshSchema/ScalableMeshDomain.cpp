/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ScalableMeshSchemaPCH.h"
USING_NAMESPACE_BENTLEY_DGN
#include <ScalableMeshSchema\ScalableMeshDomain.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include <ScalableMeshSchema\ScalableMeshSchemaApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

DOMAIN_DEFINE_MEMBERS(ScalableMeshDomain)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
ScalableMeshDomain::ScalableMeshDomain() : DgnDomain(BENTLEY_SCALABLEMESH_SCHEMA_NAME, "Scalable Mesh Domain", 1)
    {
    RegisterHandler(ScalableMeshModelHandler::GetHandler());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Elenie.Godzaridis   02/16
//-----------------------------------------------------------------------------------------
void ScalableMeshDomain::_OnSchemaImported(DgnDbR db) const
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    diego.diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshDomain::SchemaOperation ScalableMeshDomain::SchemaOperationNeeded(Dgn::DgnDbCR db) const
    {
    auto smDomainCP = db.Domains().FindDomain(BENTLEY_SCALABLEMESH_SCHEMA_NAME);

    if (!smDomainCP)
        return SchemaOperation::Import;

    // Ignoring VersionDigit2 as it is not completely hooked-up in DgnDb06 by lower layers
    Statement stmt;
    if (DbResult::BE_SQLITE_OK != stmt.Prepare(db, "SELECT VersionDigit1, VersionDigit3 FROM ec_Schema WHERE Name = ?;"))
        return SchemaOperation::Undetermined;

    if (DbResult::BE_SQLITE_OK != stmt.BindText(1, BENTLEY_SCALABLEMESH_SCHEMA_NAME, Statement::MakeCopy::No) ||
        DbResult::BE_SQLITE_ROW != stmt.Step())
        return SchemaOperation::Import;

    uint32_t digit1 = (uint32_t)stmt.GetValueInt(0);
    uint32_t digit3 = (uint32_t)stmt.GetValueInt(1);

    if (digit1 == GetExpectedSchemaVersionDigit1() && digit3 < GetExpectedSchemaVersionDigit3())
        return SchemaOperation::MinorSchemaUpdate;
    if (digit1 < GetExpectedSchemaVersionDigit1())
        return SchemaOperation::MajorSchemaUpgrade;
    else if (digit1 > GetExpectedSchemaVersionDigit1() || digit3 > GetExpectedSchemaVersionDigit3())
        return SchemaOperation::UnsupportedSchema;

    return SchemaOperation::None;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre   12/16
//-----------------------------------------------------------------------------------------
Dgn::DgnDbStatus ScalableMeshDomain::UpdateSchema(SchemaUpdateScalableMeshDgnDbParams& params) const
    {
    auto schemaOp = SchemaOperationNeeded(*params.m_dgnDb);
    if (schemaOp == SchemaOperation::None)
        return DgnDbStatus::Success;

    if (schemaOp != SchemaOperation::MinorSchemaUpdate && schemaOp != SchemaOperation::Import)
        return DgnDbStatus::InvalidProfileVersion;

    BeFileName schemaFileName = params.m_assetsRootDir;
    schemaFileName.AppendToPath(BENTLEY_SCALABLEMESH_SCHEMA_PATH);

    DgnDbStatus retVal = DgnDbStatus::Success;
/*
    if (DgnDbStatus::Success != (retVal = ScalableMeshDomain::GetDomain().ImportSchema(*params.m_dgnDb, schemaFileName, DgnDomain::ImportSchemaOptions::CreateECClassViews)))
        return retVal;
*/

    Utf8String schemaUpdateDescr("SAVECHANGES_SchemaUpdate");
    if (schemaOp == SchemaOperation::Import)
        schemaUpdateDescr = "SAVECHANGES_SchemaImport";

    if (DbResult::BE_SQLITE_OK != params.m_dgnDb->SaveChanges(schemaUpdateDescr.c_str()))
        retVal = DgnDbStatus::WriteError;

    return retVal;
    }
