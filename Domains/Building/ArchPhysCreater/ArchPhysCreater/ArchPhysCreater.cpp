/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/ArchPhysCreater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::CategorySelectorPtr ArchPhysCreator::CreateCategorySelector(Dgn::DefinitionModelR model)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one category that we use.
    // We have to give the selector a unique name of its own. Since we are set up up a new bim, 
    // we know that we can safely choose any name.
    auto categorySelector = new Dgn::CategorySelector(model, "Default");
    categorySelector->AddCategory(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(model.GetDgnDb()));
    return categorySelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::ModelSelectorPtr ArchPhysCreator::CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::PhysicalModelR modelToSelect)
    {
    // A ModelSelector is a definition element that is normally shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one model that we use.
    // We have to give the selector a unique name of its own. Since we are set up up a new 
    // bim, we know that we can safely choose any name.
    auto modelSelector = new Dgn::ModelSelector(definitionModel, "Default");
    modelSelector->AddModel(modelToSelect.GetModelId());
    return modelSelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::DisplayStyle3dPtr ArchPhysCreator::CreateDisplayStyle3d(Dgn::DefinitionModelR model)
    {
    // DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a style that can be used as a good default for 3D views.
    // We have to give the style a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
    auto displayStyle = new Dgn::DisplayStyle3d(model, "Default");
    displayStyle->SetBackgroundColor(Dgn::ColorDef::White());
    displayStyle->SetSkyBoxEnabled(false);
    displayStyle->SetGroundPlaneEnabled(false);
    Dgn::Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
    viewFlags.SetRenderMode(Dgn::Render::RenderMode::SmoothShade);
    displayStyle->SetViewFlags(viewFlags);
    return displayStyle;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr ArchPhysCreator::CreateDgnDb(BeFileNameCR outputFileName)
    {
    // Initialize parameters needed to create a DgnDb
    Dgn::CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(m_overwriteExistingOutputFile);
    createProjectParams.SetRootSubjectName("Sample Building");
    createProjectParams.SetRootSubjectDescription("Sample Building create by ArchCreater app");

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

    Dgn::DgnDomains::RegisterDomain( BentleyApi::ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain(),   Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    Dgn::DgnDomains::RegisterDomain( BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(),                 Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
    Dgn::DgnDomains::RegisterDomain( BentleyApi::BuildingPhysical::BuildingPhysicalDomain::GetDomain(),             Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);

    Dgn::DgnDbPtr db = CreateDgnDb(GetOutputFileName());
    if (!db.IsValid())
        return BentleyStatus::ERROR;

    // Create an DefinitionPartitionElement for the building model

    Dgn::SubjectCPtr rootSubject = db->Elements().GetRootSubject();
    Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*rootSubject, "BuildingPhysicalModel");
    if (!partition.IsValid())
        return BentleyStatus::ERROR;


    
    BuildingPhysical::BuildingPhysicalModelPtr physicalModel = BuildingPhysical::BuildingPhysicalModel::Create(*partition);
    if (!physicalModel.IsValid())
        return BentleyStatus::ERROR;

    Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*rootSubject, "BuildingTypeDefinitionModel");
    if (!defPartition.IsValid())
        return BentleyStatus::ERROR;

    // Create a ToyTilePhysicalModel in memory
    BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingPhysical::BuildingTypeDefinitionModel::Create(*defPartition);
    if (!typeDefinitionModel.IsValid())
        return BentleyStatus::ERROR;

    CreateBuilding( *physicalModel, *typeDefinitionModel);


    // Set the project extents to include the elements in the physicalModel, plus a margin
    Dgn::AxisAlignedBox3d projectExtents = physicalModel->QueryModelRange();
    projectExtents.Extend(0.5);
    db->GeoLocation().SetProjectExtents(projectExtents);

    // Create the initial view
    Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
    Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
    Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, *physicalModel);
    Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);

    Dgn::DgnViewId viewId = CreateView(dictionary, "Building View", *categorySelector, *modelSelector, *displayStyle);

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnViewId ArchPhysCreator::CreateView(Dgn::DefinitionModelR model, Utf8CP name, Dgn::CategorySelectorR categorySelector, Dgn::ModelSelectorR modelSelector, Dgn::DisplayStyle3dR displayStyle)
    {
    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    Dgn::OrthographicViewDefinition view(model, name, categorySelector, displayStyle, modelSelector);

    // Define the view direction and volume.
    Dgn::DgnDbR db = model.GetDgnDb();
    view.SetStandardViewRotation(Dgn::StandardView::Iso); // Default to a rotated view
    view.LookAtVolume(db.GeoLocation().GetProjectExtents()); // A good default for a new view is to "fit" it to the contents of the bim.

                                                             // Write the ViewDefinition to the bim
    if (!view.Insert().IsValid())
        return Dgn::DgnViewId();

    // Set the DefaultView property of the bim
    Dgn::DgnViewId viewId = view.GetViewId();
    db.SaveProperty(Dgn::DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    return viewId;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------


BentleyStatus ArchPhysCreator::CreateBuilding(BuildingPhysical::BuildingPhysicalModelR physicalModel, BuildingPhysical::BuildingTypeDefinitionModelR typeModel)
    {

    for (int i = 0; i < 100; i++)
        {

        ArchitecturalPhysical::DoorPtr door = DoorTools::CreateDoor ( physicalModel, i + 1 );
        ECN::ECValue doubleValue;

        Dgn::DgnDbStatus status = door->SetPropertyValue("OverallWidth", 250.0);
        status = door->SetPropertyValue("OverallHeight", 250.0);

        Dgn::Placement3d placement;

        placement.GetOriginR() = DPoint3d::From(5.0 * i, 0.0, 0.0);

        status = door->SetPlacement(placement);

        GeometricTools::CreateDoorGeometry(door, physicalModel);

        ECN::ECValue value;

        ECN::IECInstancePtr instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddManufacturerAspect(physicalModel, door);

        value.SetWCharCP(L"ACME");

        instance->SetValue("Manufacturer", value);

        value.SetWCharCP(L"SN-12345");

        instance->SetValue("ModelNumber", value);

        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect (physicalModel, door);

        value.SetWCharCP(L"10-10-10");

        instance->SetValue("OmniClass", value);

        Dgn::DgnElementCPtr element = door->Insert( &status );
        
        if ( Dgn::DgnDbStatus::Success != status )
            return BentleyStatus::ERROR;

//        element->GetCode();

//        ArchitecturalPhysical::DoorPtr door1 = ArchitecturalPhysical::ArchitecturalBaseElement::QueryByCodeValue<ArchitecturalPhysical::Door> (physicalModel, "D001");

//        Dgn::CodeSpec::CreateCode ( ( physicalModel.GetDgnDb(), BIS_CODESPEC_SpatialCategory, categoryName, nameSpace);

        ArchitecturalPhysical::WindowPtr window = DoorTools::CreateWindow1(physicalModel, i + 1);

        Dgn::Placement3d windowPlacement;

        windowPlacement.GetOriginR() = DPoint3d::From(5.0 * i, 4.0, 4.0);

        status = window->SetPlacement(windowPlacement);

        GeometricTools::CreateWindowGeometry(window, physicalModel);

        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect(physicalModel, window);

        value.SetWCharCP(L"20-10-10");

        instance->SetValue("OmniClass", value);

        window->Insert(&status);

        if (Dgn::DgnDbStatus::Success != status)
            return BentleyStatus::ERROR;
        }

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

