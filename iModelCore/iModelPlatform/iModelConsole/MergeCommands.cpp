/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECJsonUtilities.h>
#include <ECObjects/SchemaMerger.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/Nullable.h>
#include "Command.h"
#include "iModelConsole.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String MergeCommand::_GetUsage() const
    {
    return " .merge schema <ecschema xml folder>\r\n"
        COMMAND_USAGE_IDENT "Merges the specified ECSchema XML files from a folder into the current db.\r\n"
        COMMAND_USAGE_IDENT "Note: Outstanding changes are committed before starting the import.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void MergeCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None;
    if (args.size() != 1)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    Utf8StringCR inputFolder = args[0];

    BeFileName inputPath(inputFolder);
    inputPath.Trim(L"\"");
    if (!inputPath.DoesPathExist() || !inputPath.IsDirectory())
        {
        IModelConsole::WriteErrorLine("Specified directory '%s' does not exist.", inputPath.GetNameUtf8().c_str());
        return;
        }

    BeFileName schemaAssetsFolder = Dgn::PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    BeFileName schemaSearchPath(schemaAssetsFolder);
    schemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"Domain");

    // Initialize first context
    ECN::ECSchemaReadContextPtr context1 = ECN::ECSchemaReadContext::CreateContext(false, true);
    context1->AddSchemaLocater(session.GetFile().GetECDbHandle()->GetSchemaLocater());
    context1->AddSchemaPath(schemaSearchPath);
    context1->AddSchemaPath(inputPath);
    bvector<BeFileName> schemaFilePaths1;
    BeDirectoryIterator::WalkDirsAndMatch(schemaFilePaths1, inputPath, L"*.ecschema.xml", false);
    if (schemaFilePaths1.empty())
        {
        IModelConsole::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", inputPath.GetNameUtf8().c_str());
        return;
        }

    IModelConsole::WriteLine("Reading schemas from directory...");
    for (BeFileName const& ecschemaFilePath : schemaFilePaths1)
        {
        IModelConsole::WriteLine("Reading ECSchema %s...", ecschemaFilePath.GetNameUtf8().c_str());
        if (SUCCESS != DeserializeECSchema(*context1, ecschemaFilePath))
            {
            IModelConsole::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8().c_str());
            return;
            }
        }

    bvector<ECN::ECSchemaCP> existingSchemas = session.GetFile().GetECDbHandle()->Schemas().GetSchemas(true);

    ECN::SchemaMergeResult result;
    auto mergeStatus = ECN::SchemaMerger::MergeSchemas(result,existingSchemas, context1->GetCache().GetSchemas());
    if(mergeStatus != BentleyStatus::SUCCESS)
        {
        //TODO Log from issue reporter on result
        IModelConsole::WriteLine("Failed to merge incoming schemas with existing ones (SchemaMerger).");
        return;
        }

    if (BE_SQLITE_OK != session.GetFile().GetHandleR().SaveChanges())
        {
        IModelConsole::WriteLine("Saving outstanding changes in the file failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    bool schemaImportSuccessful = false;
    if (session.GetFile().GetType() == SessionFile::Type::IModel)
        {
        if (options == SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues)
            schemaImportSuccessful = Dgn::SchemaStatus::Success == session.GetFile().GetAs<IModelFile>().GetDgnDbHandleR().ImportV8LegacySchemas(result.GetResults());
        else
            schemaImportSuccessful = Dgn::SchemaStatus::Success == session.GetFile().GetAs<IModelFile>().GetDgnDbHandleR().ImportSchemas(result.GetResults(), true);
        }
    else
        schemaImportSuccessful = SUCCESS == session.GetFile().GetECDbHandle()->Schemas().ImportSchemas(result.GetResults(), options);

    if (schemaImportSuccessful)
        {
        session.GetFile().GetHandleR().SaveChanges();
        IModelConsole::WriteLine("Successfully merged ECSchemas in folder '%s'.", inputPath.GetNameUtf8().c_str());
        return;
        }

    session.GetFile().GetHandleR().AbandonChanges();
    if (session.GetIssues().HasIssue())
        IModelConsole::WriteErrorLine("Failed to merge ECSchemas in folder '%s': %s", inputPath.GetNameUtf8().c_str(), session.GetIssues().GetIssue());
    else
        IModelConsole::WriteErrorLine("Failed to merge ECSchemas in folder '%s'.", inputPath.GetNameUtf8().c_str());
    }



//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus MergeCommand::DeserializeECSchema(ECSchemaReadContextR readContext, BeFileNameCR ecschemaXmlFile)
    {
    ECN::ECSchemaPtr ecSchema = nullptr;
    const auto stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, ecschemaXmlFile.GetName(), readContext);
    //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
    return stat == ECN::SchemaReadStatus::Success || stat == ECN::SchemaReadStatus::DuplicateSchema ? SUCCESS : ERROR;
    }

//====================================================================================================================
//============================================MERGE EXTERNAL COMMAND==================================================
//====================================================================================================================


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String MergeExternalCommand::_GetUsage() const
    {
    return " .mergeexternal <input folder 1> <input folder 2> <output folder>\r\n"
        COMMAND_USAGE_IDENT "Merges ECSchema XML files from two directories into a new directory.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void MergeExternalCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.size() != 3)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR inputFolder1 = args[0];
    Utf8StringCR inputFolder2 = args[1];
    Utf8StringCR outputFolder = args[2];

    BeFileName inputPath1(inputFolder1);
    inputPath1.Trim(L"\"");
    if (!inputPath1.DoesPathExist() || !inputPath1.IsDirectory())
        {
        IModelConsole::WriteErrorLine("Specified directory (input 1) '%s' does not exist.", inputPath1.GetNameUtf8().c_str());
        return;
        }

    BeFileName inputPath2(inputFolder2);
    inputPath2.Trim(L"\"");
    if (!inputPath2.DoesPathExist() || !inputPath2.IsDirectory())
        {
        IModelConsole::WriteErrorLine("Specified directory (input 2) '%s' does not exist.", inputPath2.GetNameUtf8().c_str());
        return;
        }

    BeFileName outputPath(outputFolder);
    outputPath.Trim(L"\"");
    if (!outputPath.DoesPathExist() || !outputPath.IsDirectory())
        {
        IModelConsole::WriteErrorLine("Specified directory (output) '%s' does not exist.", outputPath.GetNameUtf8().c_str());
        return;
        }

    BeFileName schemaAssetsFolder = Dgn::PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    BeFileName schemaSearchPath(schemaAssetsFolder);
    schemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"Domain");

    // Initialize first context
    ECN::ECSchemaReadContextPtr context1 = ECN::ECSchemaReadContext::CreateContext(false, true);
    //context1->AddSchemaLocater(session.GetFile().GetECDbHandle()->GetSchemaLocater());
    context1->AddSchemaPath(schemaSearchPath);
    context1->AddSchemaPath(inputPath1);
    bvector<BeFileName> schemaFilePaths1;
    BeDirectoryIterator::WalkDirsAndMatch(schemaFilePaths1, inputPath1, L"*.ecschema.xml", false);
    if (schemaFilePaths1.empty())
        {
        IModelConsole::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", inputPath1.GetNameUtf8().c_str());
        return;
        }

    IModelConsole::WriteLine("Reading schemas from first directory...");
    for (BeFileName const& ecschemaFilePath : schemaFilePaths1)
        {
        IModelConsole::WriteLine("Reading ECSchema %s...", ecschemaFilePath.GetNameUtf8().c_str());
        if (SUCCESS != DeserializeECSchema(*context1, ecschemaFilePath))
            {
            IModelConsole::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8().c_str());
            return;
            }
        }

    // Initialize second context
    ECN::ECSchemaReadContextPtr context2 = ECN::ECSchemaReadContext::CreateContext(false, true);
    //context2->AddSchemaLocater(session.GetFile().GetECDbHandle()->GetSchemaLocater());
    context2->AddSchemaPath(schemaSearchPath);
    context2->AddSchemaPath(inputPath2);
    bvector<BeFileName> schemaFilePaths2;
    BeDirectoryIterator::WalkDirsAndMatch(schemaFilePaths2, inputPath2, L"*.ecschema.xml", false);
    if (schemaFilePaths2.empty())
        {
        IModelConsole::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", inputPath1.GetNameUtf8().c_str());
        return;
        }

    IModelConsole::WriteLine("Reading schemas from second directory...");
    for (BeFileName const& ecschemaFilePath : schemaFilePaths2)
        {
        IModelConsole::WriteLine("Reading ECSchema %s...", ecschemaFilePath.GetNameUtf8().c_str());
        if (SUCCESS != DeserializeECSchema(*context2, ecschemaFilePath))
            {
            IModelConsole::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8().c_str());
            return;
            }
        }

    ECN::SchemaMergeResult result;
    auto mergeStatus = ECN::SchemaMerger::MergeSchemas(result, context1->GetCache().GetSchemas(), context2->GetCache().GetSchemas());
    if(mergeStatus != BentleyStatus::SUCCESS)
        {
        //TODO Log from issue reporter on result
        IModelConsole::WriteLine("Failed to merge incoming schemas with existing ones (SchemaMerger).");
        return;
        }

    for (ECSchemaCP schema : result.GetResults())
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outputPath);
        outPath.AppendToPath(fileName.c_str());
        schema->WriteToXmlFile(outPath.GetName(), ECN::ECVersion::Latest);
        IModelConsole::WriteLine("Saved ECSchema '%s' to disk", outPath.GetNameUtf8().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus MergeExternalCommand::DeserializeECSchema(ECSchemaReadContextR readContext, BeFileNameCR ecschemaXmlFile)
    {
    ECN::ECSchemaPtr ecSchema = nullptr;
    const auto stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, ecschemaXmlFile.GetName(), readContext);
    //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
    return stat == ECN::SchemaReadStatus::Success || stat == ECN::SchemaReadStatus::DuplicateSchema ? SUCCESS : ERROR;
    }
