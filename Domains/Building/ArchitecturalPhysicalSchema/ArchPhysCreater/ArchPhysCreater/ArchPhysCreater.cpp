// ArchPhysCreater.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ArchPhysCreater.h"


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ArchPhysCreator::PrintUsage(WCharCP exeName)
    {
    fwprintf(stderr,
        L"\n"
        L"Create a Architectural Physical BIM file.\n"
        L"\n"
        L"Usage: %ls -o|--output= [OPTIONS...]\n"
        L"\n"
        L"  --output=   (required)  Output DgnDb file\n"
        L"\n"
        L"OPTIONS:\n"
        L"  --overwrite (optional)  Overwrite an existing output file"
        L"\n", BeFileName::GetFileNameAndExtension(exeName).c_str());
    return BentleyStatus::ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
WString ArchPhysCreator::GetArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ArchPhysCreator::ParseCommandLine(int argc, WCharP argv[])
    {
    if (argc < 2)
        return PrintUsage(argv[0]);

    WString outputFileNameArg, currentDirectory;

    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
            {
            outputFileNameArg = GetArgValueW(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--overwrite"))
            {
            m_overwriteExistingOutputFile = true;
            continue;
            }

        fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
        return PrintUsage(argv[0]);
        }

    // validate input arg
    BeFileName::GetCwd(currentDirectory);

    // validate output arg
    BeFileName outputFileNameDefaults(currentDirectory);
    outputFileNameDefaults.AppendExtension(L"bim");

    m_outputFileName = BeFileName(outputFileNameArg.c_str());
    m_outputFileName.SupplyDefaultNameParts(outputFileNameDefaults);

    if (m_outputFileName.DoesPathExist() && !m_overwriteExistingOutputFile)
        {
        fwprintf(stderr, L"Output file already exists: %ls\n", m_outputFileName.c_str());
        fwprintf(stderr, L"Specify --overwrite to overwrite an existing output file\n\n");
        return PrintUsage(argv[0]);
        }

#if 1
    fwprintf(stdout, L"OutputFileName: %ls\n", m_outputFileName.GetName());
#endif

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BeSQLite::L10N::SqlangFiles ArchPhysCreator::_SupplySqlangFiles()
    {
    BeFileName defaultSqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    defaultSqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
    return BeSQLite::L10N::SqlangFiles(defaultSqlang);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr ArchPhysCreator::CreateDgnDb(BeFileNameCR outputFileName)
    {
    // Initialize parameters needed to create a DgnDb
    Dgn::CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(m_overwriteExistingOutputFile);
    createProjectParams.SetRootSubjectName("ToyTile Sample BIM");
    createProjectParams.SetRootSubjectDescription("ToyTile sample BIM created by ToyTileCreator");

    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    BeSQLite::DbResult createStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::CreateDgnDb(&createStatus, outputFileName, createProjectParams);
    if (!db.IsValid())
        return nullptr;

    // After all domain schemas have been imported, it is valid to create ECClassViews (for debugging and review workflows)
    db->Schemas().CreateClassViewsInDb();

    return db;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ArchPhysCreator::DoCreate()
    {

    Dgn::DgnDomains::RegisterDomain( ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
  //  Dgn::DgnDomains::RegisterDomain( BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);

    Dgn::DgnDbPtr db = CreateDgnDb(GetOutputFileName());
    if (!db.IsValid())
        return BentleyStatus::ERROR;

    // Create an DefinitionPartitionElement for the building model

    Dgn::SubjectCPtr rootSubject = db->Elements().GetRootSubject();
    Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*rootSubject, "BuildingPhysicalModel");
    if (!partition.IsValid())
        return BentleyStatus::ERROR;

#ifdef NOTYET

    BBPH::BuildingPhysicalModelPtr physicalModel = BBPH::BuildingPhysicalModel::Create(*partition);
    if (!physicalModel.IsValid())
        return BentleyStatus::ERROR;

    DefinitionPartitionCPtr defPartition = DefinitionPartition::CreateAndInsert(*rootSubject, "BuildingTypeDefinitionModel");
    if (!defPartition.IsValid())
        return BentleyStatus::ERROR;

    // Create a ToyTilePhysicalModel in memory
    BBPH::BuildingTypeDefinitionModelPtr typeDefinitionModel = BBPH::BuildingTypeDefinitionModel::Create(*defPartition);
    if (!typeDefinitionModel.IsValid())
        return BentleyStatus::ERROR;


    //  BuildingTypeDefinitionModelPtr typeDefinitionModel = CreateBuildingTypeDefinitionModel(*db);
    //  if (!typeDefinitionModel.IsValid())
    //      return BentleyStatus::ERROR;


    if (BentleyStatus::SUCCESS != CreatePhysicalElements(*physicalModel, *typeDefinitionModel))
        return BentleyStatus::ERROR;

    // Set the project extents to include the elements in the physicalModel, plus a margin
    AxisAlignedBox3d projectExtents = physicalModel->QueryModelRange();
    projectExtents.Extend(0.5);
    db->GeoLocation().SetProjectExtents(projectExtents);

    // Create the initial view
    DefinitionModelR dictionary = db->GetDictionaryModel();
    CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
    ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, *physicalModel);
    DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);

    Dgn::DgnViewId viewId = CreateView(dictionary, TOYTILECREATOR_ViewName, *categorySelector, *modelSelector, *displayStyle);
    if (!viewId.IsValid())
        return BentleyStatus::ERROR;
#endif
    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

int wmain(int argc, WCharP argv[])
    {

    ArchPhysCreator app;
    Dgn::DgnPlatformLib::Initialize(app, false);

    //if (BentleyStatus::SUCCESS != app.ParseCommandLine(argc, argv))
    //    return BentleyStatus::ERROR;

    app.ParseCommandLine(argc, argv);

    return app.DoCreate();

    }

