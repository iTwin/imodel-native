/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/StructPhysCreator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"

#define STRUCTURAL_MODEL_NAME "SampleStructuralModel"



#pragma region FREE_FUNCTIONS

template<class T, class U> RefCountedCPtr<T> const_pointer_cast(RefCountedCPtr<U> const & p) { return dynamic_cast<T const *>(p.get()); }

#pragma endregion


#pragma region PRIVATE_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BeSQLite::L10N::SqlangFiles StructPhysCreator::_SupplySqlangFiles()
    {
    BeFileName defaultSqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    defaultSqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
    return BeSQLite::L10N::SqlangFiles(defaultSqlang);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::CategorySelectorPtr StructPhysCreator::CreateCategorySelector(Dgn::DefinitionModelR model)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one category that we use.
    // We have to give the selector a unique name of its own. Since we are set up up a new bim,
    // we know that we can safely choose any name.
    auto categorySelector = new Dgn::CategorySelector(model, "Default");
    return categorySelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::ModelSelectorPtr StructPhysCreator::CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::PhysicalModelR modelToSelect)
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
Dgn::DisplayStyle3dPtr StructPhysCreator::CreateDisplayStyle3d(Dgn::DefinitionModelR model)
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

#pragma endregion


#pragma region PUBLIC_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::PrintUsage(WCharCP exeName)
    {
    fwprintf(stderr,
        L"\n"
        L"Create a Structural Physical BIM file.\n"
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
WString StructPhysCreator::GetArgValueW(WCharCP arg)
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
BentleyStatus StructPhysCreator::ParseCommandLine(int argc, WCharP argv[])
    {
    if (argc < 2)
        {
        return PrintUsage(argv[0]);
        }

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
     //_wgetcwd(currentDirectory);

    // validate output arg
    //   BeFileName outputFileNameDefaults;
    //   outputFileNameDefaults.AppendExtension(L"bim");

    m_outputFileName = BeFileName(outputFileNameArg.c_str());
    //   m_outputFileName.SupplyDefaultNameParts(outputFileNameDefaults);

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
Dgn::DgnDbPtr StructPhysCreator::CreateDgnDb(BeFileNameCR outputFileName)
    {
    // Initialize parameters needed to create a DgnDb
    Dgn::CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(m_overwriteExistingOutputFile);
    createProjectParams.SetRootSubjectName("Sample Structure");
    createProjectParams.SetRootSubjectDescription("Sample Structure create by StructPhysCreater app");

    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    BeSQLite::DbResult createStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::CreateDgnDb(&createStatus, outputFileName, createProjectParams);
    if (!db.IsValid())
        {
        return nullptr;
        }

    // After all domain schemas have been imported, it is valid to create ECClassViews (for debugging and review workflows)
    db->Schemas().CreateClassViewsInDb();

    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::DoUpdateSchema(Dgn::DgnDbPtr db)
    {
    BentleyApi::Structural::StructuralPhysicalModelCPtr model = BentleyApi::Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(STRUCTURAL_MODEL_NAME, *db);

    ECN::ECSchemaPtr dynSchema = BentleyApi::Structural::StructuralDomainUtilities::GetUpdateableSchema(model);

    if (!dynSchema.IsValid())
        return BentleyStatus::ERROR;

    //ECN::ECEntityClassP myClass = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElementEntityClass(db, dynSchema, "MyRctClass");

    //ECN::PrimitiveECPropertyP myProp;

    //myClass->CreatePrimitiveProperty(myProp, "MyStringProp");

    Dgn::SchemaStatus schemaStatus = BentleyApi::Structural::StructuralDomainUtilities::UpdateSchemaInDb(*db, *dynSchema);

    if (schemaStatus != Dgn::SchemaStatus::Success)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::PopulateInstanceProperties(ECN::IECInstancePtr instance)
    {
    ECN::ECClassCR ecClass = instance->GetClass();

    int i = 0;

    for each (ECN::ECPropertyP prop in ecClass.GetProperties())
        {
        if (nullptr == prop || prop->GetName() == "Roll" || prop->GetName() == "Pitch" || prop->GetName() == "Yaw")
            {
            continue;
            }

        Utf8String typeName = prop->GetTypeName();

        if (typeName == "double")
            {
            ECN::ECValue doubleValue;

            doubleValue.SetDouble((i + 1) * 10.0);
            instance->SetValue(prop->GetName().c_str(), doubleValue);
            }
        else if (typeName == "string")
            {
            ECN::ECValue stringValue;

            stringValue.SetWCharCP(L"String Value");
            instance->SetValue(prop->GetName().c_str(), stringValue);
            }
        else if (typeName == "boolean")
            {
            ECN::ECValue boolValue;
            boolValue.SetBoolean(true);
            instance->SetValue(prop->GetName().c_str(), boolValue);
            }

        i++;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::PopulateElementProperties(Dgn::PhysicalElementPtr element)
    {
    ECN::ECClassCP ecClass = element->GetElementClass();
    int i = 0;

    for each (ECN::ECPropertyP prop in ecClass->GetProperties())
        {
        if (nullptr == prop || prop->GetName() == "Roll" || prop->GetName() == "Pitch" || prop->GetName() == "Yaw")
            {
            continue;
            }

        Utf8String typeName = prop->GetTypeName();

        if (typeName == "double")
            {
            ECN::ECValue doubleValue;

            doubleValue.SetDouble((i + 1) * 10.0);
            element->SetPropertyValue(prop->GetName().c_str(), doubleValue);
            }
        else if (typeName == "string")
            {
            ECN::ECValue stringValue;

            stringValue.SetWCharCP(L"String Value");
            element->SetPropertyValue(prop->GetName().c_str(), stringValue);
            }
        else if (typeName == "boolean")
            {
            ECN::ECValue boolValue;
            boolValue.SetBoolean(true);
            element->SetPropertyValue(prop->GetName().c_str(), boolValue);
            }

        i++;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::CreateConcreteStructure(BentleyApi::Structural::StructuralPhysicalModelR physicalModel/*, BentleyApi::Structural::StructuralTypeDefinitionModelR typeModel*/)
    {
    ECN::ECSchemaCP schema = physicalModel.GetDgnDb().Schemas().GetSchema(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME);

    ECN::ECClassContainerCR classes = schema->GetClasses();

    ConcreteStructureCreator* creator = new ConcreteStructureCreator();

    BentleyStatus status;

    for each (ECN::ECClassP derivedClass in classes)
        {
        if (ECN::ECClassModifier::Abstract == derivedClass->GetClassModifier())
            {
            continue;
            }

        Utf8String className = derivedClass->GetFullName();
        if (className == "StructuralPhysical:Beam")
            {
            status = creator->CreateBeams(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Column")
            {
            status = creator->CreateColumns(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Slab")
            {
            status = creator->CreateSlabs(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Wall")
            {
            status = creator->CreateWalls(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::CreateSteelStructure(BentleyApi::Structural::StructuralPhysicalModelR physicalModel/* , BentleyApi::Structural::StructuralTypeDefinitionModelR typeModel*/)
    {
    ECN::ECSchemaCP schema = physicalModel.GetDgnDb().Schemas().GetSchema(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME);

    ECN::ECClassContainerCR classes = schema->GetClasses();

    SteelStructureCreator* creator = new SteelStructureCreator();

    BentleyStatus status;

    for each (ECN::ECClassP derivedClass in classes)
        {
        if (ECN::ECClassModifier::Abstract == derivedClass->GetClassModifier())
            {
            continue;
            }

        Utf8String className = derivedClass->GetFullName();
        if (className == "StructuralPhysical:Beam")
            {
            status = creator->CreateBeams(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Column")
            {
            status = creator->CreateColumns(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Plate")
            {
            status = creator->CreateGussetPlates(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        else if (className == "StructuralPhysical:Brace")
            {
            status = creator->CreateBraces(physicalModel, schema, derivedClass);
            if (status != BentleyStatus::SUCCESS)
                {
                return BentleyStatus::ERROR;
                }
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnViewId StructPhysCreator::CreateView(Dgn::DefinitionModelR model, Utf8CP name, Dgn::CategorySelectorR categorySelector, Dgn::ModelSelectorR modelSelector, Dgn::DisplayStyle3dR displayStyle)
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
        {
        return Dgn::DgnViewId();
        }

    // Set the DefaultView property of the bim
    Dgn::DgnViewId viewId = view.GetViewId();
    db.SaveProperty(Dgn::DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    return viewId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus StructPhysCreator::DoCreate()
    {
     BentleyApi::Structural::StructuralDomainUtilities::RegisterDomainHandlers();

    Dgn::DgnDbPtr db = CreateDgnDb(GetOutputFileName());
    if (!db.IsValid())
        {
        return BentleyStatus::ERROR;
        }

    const BentleyStatus status = BentleyApi::Structural::StructuralDomainUtilities::CreateStructuralModels(STRUCTURAL_MODEL_NAME, *db);

    if (BentleyStatus::SUCCESS != status)
        return BentleyStatus::ERROR;

    BentleyApi::Structural::StructuralPhysicalModelPtr physicalModel = BentleyApi::Structural::StructuralDomainUtilities::GetStructuralPhysicalModel(STRUCTURAL_MODEL_NAME, *db);

    if (physicalModel == nullptr)
        return BentleyStatus::ERROR;

    DoUpdateSchema(db);

    CreateConcreteStructure(*physicalModel);
    CreateSteelStructure(*physicalModel);

    // Set the project extents to include the elements in the physicalModel, plus a margin
    AxisAlignedBox3d projectExtents = physicalModel->QueryModelRange();
    projectExtents.Extend(0.5);
    db->GeoLocation().SetProjectExtents(projectExtents);

    // Create the initial view
    Dgn::DefinitionModelR dictionary = db->GetDictionaryModel();
    Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
    Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, *physicalModel);
    Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);

    Dgn::DgnViewId viewId = CreateView(dictionary, "Structure View", *categorySelector, *modelSelector, *displayStyle);

    if (!viewId.IsValid())
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

#pragma endregion
