/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "BimCreater.h"


#define BUILDING_MODEL_NAME "SamplePlantModel"
#define USERLABEL_NAME  "UserLabel"

#define PUMP_CODE_PATTERN       "PMP####"
#define VESSEL_CODE_PATTERN     "V####"
#define PLANT_CODE_PATTERN      "P##"
#define UNIT_CODE_PATTERN       "U##"
#define SUBUNIT_CODE_PATTERN    "SU##"
#define BUILDING_CODE_PATTERN   "BLD##"
#define NOZZLE_CODE_PATTERN     "N##"
#define PID_CODE_PATTERN        "PID-####"
#define PIPELINE_CODE_PATTERN   "####"
#define VALVE_CODE_PATTERN      "CV####"
#define SYSTEM_CODE_PATTERN     "S##"
#define PIPERUN_CODE_PATTERN    "PS#"
#define AREA_CODE_PATTERN       "A##"
#define ROOM_CODE_PATTERN       "RM###"
#define FLOOR_CODE_PATTERN      "FL##"


#define EQUIPMENT_CODESPEC_NAME "PlantFunctional-Equipment"
#define NOZZLE_CODESPEC_NAME    "PlantFunctional-Nozzle"
#define UNIT_CODESPEC_NAME      "PlantFunctional-Unit"
#define PLANT_CODESPEC_NAME     "PlantFunctional-Plant"
#define BUILDING_CODESPEC_NAME  "PlantFunctional-Building"
#define SUBUNIT_CODESPEC_NAME   "PlantFunctional-SubUnit"
#define PID_CODESPEC_NAME       "PlantFunctional-PID"
#define PIPELINE_CODESPEC_NAME  "PlantFunctional-Pipeline"
#define VALVE_CODESPEC_NAME     "PlantFunctional-Valve"
#define SYSTEM_CODESPEC_NAME    "PlantFunctional-System"
#define PIPERUN_CODESPEC_NAME   "PlantFunctional-PipeRun"
#define AREA_CODESPEC_NAME      "PlantFunctional-Area"
#define ROOM_CODESPEC_NAME      "PlantFunctional-Room"
#define FLOOR_CODESPEC_NAME     "PlantFunctional-Floor"


#ifdef USE_PROTOTYPE
    #define PIPING_DOMAIN      DOMAIN_PIPING_FUNCTIONAL
    #define EQUIPMENT_DOMAIN   DOMAIN_EQUIPMENT_FUNCTIONAL
    #define BREAKDOWN_DOMAIN   DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL


    #define CONCENTRIC_REDUCER_CLASS PPF_Class_ConcentricPipeReducer
    #define GATE_VALVE_CLASS         PPF_Class_GateValve
    #define THREE_WAY_VALVE_CLASS    PPF_Class_ThreeWayValve
    #define PIPERUN_CLASS            PPF_Class_PipeRun
    #define PIPELINE_CLASS           PPF_Class_Pipeline
    #define NOZZLE_CLASS             PPF_Class_Nozzle
    #define UNIT_CLASS               PBF_Class_Unit
    #define PLANT_CLASS              PBF_Class_Plant
    #define PUMP_CLASS               PEF_CLASS_CentrifugalPump
    #define DRUM_CLASS               PEF_CLASS_Drum
    #define TANK_CLASS               "Tank"
    #define VESSEL_CLASS             PEF_CLASS_Vessel

    #define UNIT_HAS_PIPELINE_REL_CLASS  PBF_Rel_FunctionalBreakdownGroupsFunctionalElements

#else
    #define PIPING_DOMAIN       DOMAIN_PLANT_FUNCTIONAL
    #define EQUIPMENT_DOMAIN    DOMAIN_PLANT_FUNCTIONAL
    #define BREAKDOWN_DOMAIN    DOMAIN_PLANT_FUNCTIONAL

    #define CONCENTRIC_REDUCER_CLASS "CONCENTRIC_PIPE_REDUCER"
    #define GATE_VALVE_CLASS         "GATE_VALVE"
    #define THREE_WAY_VALVE_CLASS    "THREE_WAY_VALVE"
    #define PIPERUN_CLASS            "PIPING_NETWORK_SEGMENT"
    #define PIPELINE_CLASS           "PIPING_NETWORK_SYSTEM"
    #define NOZZLE_CLASS             "NOZZLE"
    #define UNIT_CLASS               "UNIT"
    #define PLANT_CLASS              "Plant"
    #define PUMP_CLASS               "CENTRIFUGAL_PUMP"
    #define DRUM_CLASS               "DRUM"
    #define TANK_CLASS               "TANK"
    #define VESSEL_CLASS             "VESSEL"

    #define UNIT_HAS_PIPELINE_REL_CLASS  "UNIT_HAS_NAMED_ITEM"

#endif

template<class T, class U> RefCountedCPtr<T> const_pointer_cast(RefCountedCPtr<U> const & p) { return dynamic_cast<T const *>(p.get()); }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus BimCreater::PrintUsage(WCharCP exeName)
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
WString BimCreater::GetArgValueW(WCharCP arg)
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
Utf8String BimCreater::GetArgValueUtf8(WCharCP arg)
    {
    Utf8String argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim("\"");
    argValue.Trim();
    return argValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus BimCreater::ParseCommandLine(int argc, WCharP argv[])
    {
   // if (argc < 2)
   //     return PrintUsage(argv[0]);

    WString outputFileNameArg, currentDirectory, workdirArg;
    WString imodelname = L"SamplePlant";
    Utf8String pidArg;

    // Setup defaults for arguments so you can run without any arguments. 

    m_iModelName = "SamplePlant"; 
    m_overwriteExistingOutputFile = false;
    workdirArg = L"d:\\workdir\\";
    m_briefcaseDir = "d:\\Briefcase\\";


    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--workdir="))
            {
            workdirArg = GetArgValueW(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--briefcasedir="))
            {
            m_briefcaseDir = GetArgValueUtf8(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodelname="))
            {
            m_iModelName = GetArgValueUtf8(argv[iArg]).c_str();
            imodelname = GetArgValueW(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--overwrite"))
            {
            m_overwriteExistingOutputFile = true;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--addpid="))
            {
            m_overwriteExistingOutputFile = false;
            pidArg = GetArgValueUtf8(argv[iArg]).c_str();
            m_addPid = true;

            char delimeter = ',';

            size_t pos = pidArg.GetNextToken(m_subUnitCode, &delimeter, 0);
                   pos = pidArg.GetNextToken(m_systemCode,  &delimeter, pos);
                   pos = pidArg.GetNextToken(m_areaCode,    &delimeter, pos);
                   pos = pidArg.GetNextToken(m_roomCode,    &delimeter, pos);
            continue;
            }

        fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
        return PrintUsage(argv[0]);
        }

    WString completeName = workdirArg + imodelname + L".bim";

    m_outputFileName = BeFileName(completeName.c_str());

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
BeSQLite::L10N::SqlangFiles BimCreater::_SupplySqlangFiles()
    {
    BeFileName defaultSqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    defaultSqlang.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");
    return BeSQLite::L10N::SqlangFiles(defaultSqlang);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::CategorySelectorPtr BimCreater::CreateCategorySelector(Dgn::DefinitionModelR model)
    {
    // A CategorySelector is a definition element that is normally shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one category that we use.
    // We have to give the selector a unique name of its own. Since we are set up up a new bim, 
    // we know that we can safely choose any name.

    Dgn::CategorySelectorPtr categorySelector = m_host->FindCategorySelector();

    if (categorySelector.IsValid())
        return categorySelector;

    m_host->CreateCategorySelector();

    return m_host->FindCategorySelector();
    
    //Dgn::DgnCode code = Dgn::CategorySelector::CreateCode(model, PLANT_HOST_PID_CATEGORY_SELECTOR_CODE_VALUE);

    //Dgn::DgnElementId id = m_dgnDb->Elements().QueryElementIdByCode(code);

    //if (id.IsValid())
    //    {
    //    Dgn::CategorySelectorPtr categorySelector = m_dgnDb->Elements().GetForEdit<Dgn::CategorySelector>(id);
    //    return categorySelector;
    //    }
    //
    //auto categorySelector = new Dgn::CategorySelector(model, "Default");
    //categorySelector->AddCategory(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingDrawingCategoryId(model.GetDgnDb(), PID_CATEGORY));
    //return categorySelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::ModelSelectorPtr BimCreater::CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::DgnModelR modelToSelect, Utf8CP name)
    {
    // A ModelSelector is a definition element that is normally shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one model that we use.
    // We have to give the selector a unique name of its own. Since we are set up up a new 
    // bim, we know that we can safely choose any name.
    auto modelSelector = new Dgn::ModelSelector(definitionModel, name);
    modelSelector->AddModel(modelToSelect.GetModelId());
	return modelSelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::DisplayStyle3dPtr BimCreater::CreateDisplayStyle3d(Dgn::DefinitionModelR model)
    {
    // DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a style that can be used as a good default for 3D views.
    // We have to give the style a unique name of its own. Since we are setup up a new bim, we know that we can safely choose any name.
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
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
Dgn::DisplayStyle2dPtr BimCreater::CreateDisplayStyle2d(Dgn::DefinitionModelR model)
	{
	// DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
	// To start off, we'll create a style that can be used as a good default for 2D views.
	// We have to give the style a unique name of its own. Since we are setup up a new bim, we know that we can safely choose any name.
    Dgn::DgnCode code =  Dgn::DisplayStyle2d::CreateCode(model, "Default2D");

    Dgn::DgnElementId id =  m_dgnDb->Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        {
        Dgn::DisplayStyle2dPtr displayStyle = m_dgnDb->Elements().GetForEdit<Dgn::DisplayStyle2d>(id);
        return displayStyle;
        }

	auto displayStyle = new Dgn::DisplayStyle2d(model, "Default2D");
	displayStyle->SetBackgroundColor(Dgn::ColorDef::Black());
	Dgn::Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
	viewFlags.SetRenderMode(Dgn::Render::RenderMode::Wireframe);
	displayStyle->SetViewFlags(viewFlags);
	return displayStyle;
	}



//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr BimCreater::CreateDgnDb(BeFileNameCR outputFileName, BeSQLite::DbResult* createStatus)
    {
    // Initialize parameters needed to create a DgnDb
    Dgn::CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(m_overwriteExistingOutputFile);
    createProjectParams.SetRootSubjectName("Sample Plant BIM");
    createProjectParams.SetRootSubjectDescription("Sample Plant created by BimCreater app");

    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    //BeSQLite::DbResult createStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::CreateDgnDb(createStatus, outputFileName, createProjectParams);
    if (!db.IsValid())
        return nullptr;
    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr BimCreater::OpenDgnDb(BeFileNameCR outputFileName)
	{
	// Initialize parameters needed to create a DgnDb
	Dgn::DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes );

	// Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
	BeSQLite::DbResult openStatus;

	Dgn::DgnDbPtr db =  Dgn::DgnDb::OpenDgnDb(&openStatus, outputFileName, openParams);

	if (!db.IsValid())
		return nullptr;

	return db;
	}






//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::DoUpdateSchema(Dgn::DgnDbPtr db)
	{

	BuildingPhysical::BuildingPhysicalModelCPtr model = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(BUILDING_MODEL_NAME, *db);


	ECN::ECSchemaPtr dynSchema = BuildingDomain::BuildingDomainUtilities::GetUpdateableSchema (model);

	if ( !dynSchema.IsValid() )
		return BentleyStatus::ERROR;

	ECN::ECEntityClassP myClass = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElementEntityClass(db, dynSchema, "MyRctClass");

	ECN::PrimitiveECPropertyP myProp;

	myClass->CreatePrimitiveProperty(myProp, "MyStringProp");

	Dgn::SchemaStatus schemaStatus =  BuildingDomain::BuildingDomainUtilities::UpdateSchemaInDb(*db, *dynSchema);
    
	if (schemaStatus != Dgn::SchemaStatus::Success)
		return BentleyStatus::ERROR;

	return BentleyStatus::SUCCESS;

	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

PIPERUN_TYPEPTR BimCreater::CreatePipeRun(Dgn::DgnElementCPtr pipeline, Dgn::DgnElementId toId, Dgn::DgnElementId fromId, Dgn::FunctionalModelR functionalModel)
    {

    // Create the functional Instance
#ifdef USE_PROTOTYPE
    PIPERUN_TYPEPTR functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_PipeRun, functionalModel);
#else
    PIPERUN_TYPEPTR functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, PIPERUN_CLASS, functionalModel);
#endif

    Utf8String shortCode;
    //int        number;

    BeSQLite::BeGuidCR  pipelineGuid = pipeline->GetFederationGuid();

    m_host->GetNextCodeValue(shortCode, PIPERUN_CODESPEC_NAME, pipelineGuid.ToString(), PIPERUN_CODE_PATTERN);


   // SetCodeFromParent1(number, shortCode, *functionalElement, pipeline, "PS");
    PopulateElementProperties(functionalElement);

    ECN::ECClassCP relClass;
    ECN::ECRelationshipClassCP  relationShipClass;
    BeSQLite::EC::ECInstanceKey rkey;

    // Relate this PipeRun to the input Pipeline 
#ifdef USE_PROTOTYPE
    if (pipeline.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipelineOwnsPipeRuns);
        functionalElement->SetParentId(pipeline->GetElementId(), relClass->GetId());
        }
#endif

    StartBulkOperation();
    SetCodeForElement(shortCode, *functionalElement, *pipeline, PIPERUN_CODESPEC_NAME);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the Too/ From relationships

    if (toId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunConnectsToPipingComponents);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, fe->GetElementId(), toId);
        }

    if (fromId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunConnectsToPipingComponents);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, fe->GetElementId(), fromId);
        }


#ifndef USE_PROTOTYPE
    if (pipeline.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(PIPING_DOMAIN, "PIPELINE_HAS_SEGMENT");
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, pipeline->GetElementId(), fe->GetElementId());
        }
#endif


    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<PIPERUN_TYPE>(functionalModel, fe->GetElementId());

    EndBulkOperation();
    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DrawingGraphicPtr BimCreater::CreateAnnotation(Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, Utf8StringCR text, Dgn::Placement2dCR placement)
    {
    Dgn::DrawingGraphicPtr annotationGraphics = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!annotationGraphics.IsValid())
        return nullptr;

    annotationGraphics->SetPlacement(placement);

    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_ANNOTATION_SUBCATEGORY);
    GeometricTools::CreateAnnotationTextGeometry(*annotationGraphics, categoryId, text, subCategoryId);

    Dgn::DgnElementCPtr ge = annotationGraphics->Insert();


    annotationGraphics = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::DrawingGraphic>(drawingModel, ge->GetElementId());

    return annotationGraphics;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DrawingGraphicPtr BimCreater::CreatePipeRunGraphics(PIPERUN_TYPECPTR pipeRun, Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, DPoint2dCP points, bvector<int> count)
    {

    // Now create each of the graphical line instances.

    ECN::ECClassCP              relClass          = drawingModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP  relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    Dgn::DgnCode code = pipeRun->GetCode();
    ECN::ECValue value;
    value.SetUtf8CP(code.GetValueUtf8CP());

    Dgn::DgnElementCPtr ge;

    Dgn::DrawingGraphicPtr pipeRunGraphic;

    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_PIPING_SUBCATEGORY);

    for (int i = 0; i < count.size(); i++)
        {
        pipeRunGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

        if (!pipeRunGraphic.IsValid())
            return nullptr;

        int startIndex = 0;
        if (i > 0) startIndex = count[i-1];
        GeometricTools::CreatePidLineGeometry(*pipeRunGraphic, drawingModel, &points[startIndex], count[i], categoryId, subCategoryId);
        pipeRunGraphic->SetPropertyValue(USERLABEL_NAME, value);
        ge = pipeRunGraphic->Insert();
        drawingModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), pipeRun->GetElementId());
        }

    pipeRunGraphic = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::DrawingGraphic>(drawingModel, ge->GetElementId());

    return pipeRunGraphic;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateNozzle(Dgn::DgnElementId pipeRunId, Dgn::DgnElementId equipmentId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement, bool isVirtual)
    {

    // Create Nozzle Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, NOZZLE_CLASS, functionalModel);

    Utf8String shortCode;
    int        number;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, parentElement, NOZZLE_CODESPEC_NAME, NOZZLE_CODE_PATTERN);
    PopulateElementProperties(functionalElement);

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, NOZZLE_CODESPEC_NAME);

    //Create the graphics element

    Dgn::DrawingGraphicPtr nozzleGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!nozzleGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    nozzleGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_EQUIP_SUBCATEGORY);


    if(!isVirtual)
        GeometricTools::CreatePidNozzleGeometry(*nozzleGraphic, categoryId, subCategoryId);
    else
        GeometricTools::CreatePidVirtualNozzleGeometry(*nozzleGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    nozzleGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = nozzleGraphic->Insert();

    ECN::ECClassCP relClass;
#ifdef USE_PROTOTYPE

    if (equipmentId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_EquipmentOwnsNozzles);
        functionalElement->SetParentId(equipmentId, relClass->GetId());
        }
#endif

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    relClass                      = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

#ifdef USE_PROTOTYPE
    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunConnectsToPipingComponents);
        ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
        BeSQLite::EC::ECInstanceKey rkey;

        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, pipeRunId, fe->GetElementId());
        }
#endif


#ifndef USE_PROTOTYPE
    if (equipmentId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(PIPING_DOMAIN, "EQUIPMENT_HAS_NOZZLE");
        ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
        BeSQLite::EC::ECInstanceKey rkey;

        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, equipmentId, fe->GetElementId());
        }
#endif

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel,fe->GetElementId());

    EndBulkOperation();
    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateTank( Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(EQUIPMENT_DOMAIN, TANK_CLASS, functionalModel);
    Utf8String shortCode;
    int        number = 0;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, nullptr, EQUIPMENT_CODESPEC_NAME, VESSEL_CODE_PATTERN);

    PopulateElementProperties(functionalElement);
    PopulateEquipmentProperties(functionalElement, "T", "Holding Tank", number);

    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("ProcessEquipmentFunctional", "SubUnitOwnsEquipment");

    functionalElement->SetParentId(subUnitId, relClass->GetId());

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, EQUIPMENT_CODESPEC_NAME);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_EQUIP_SUBCATEGORY);

    Dgn::DrawingGraphicPtr tankGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!tankGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    tankGraphic->SetPlacement(placement);
    GeometricTools::CreatePidTankGeometry(*tankGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    tankGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = tankGraphic->Insert();

    //ECN::ECClassCP relClass;
    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
        //relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, UNIT_HAS_PIPELINE_REL_CLASS);
        //relationShipClass = relClass->GetRelationshipClassCP();
        //functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }

    EndBulkOperation();

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateRoundTank(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(EQUIPMENT_DOMAIN, DRUM_CLASS, functionalModel);

    Utf8String shortCode;
    int        number = 0;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, nullptr, EQUIPMENT_CODESPEC_NAME, VESSEL_CODE_PATTERN);

    PopulateElementProperties(functionalElement);
    PopulateEquipmentProperties(functionalElement, "T", "Retaining Tank", number);

    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("ProcessEquipmentFunctional", "SubUnitOwnsEquipment");

    functionalElement->SetParentId(subUnitId, relClass->GetId());

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, EQUIPMENT_CODESPEC_NAME);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the Graphic Element

    Dgn::DrawingGraphicPtr tankGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!tankGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    tankGraphic->SetPlacement(placement);

    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_EQUIP_SUBCATEGORY);

    GeometricTools::CreatePidRoundTankGeometry(*tankGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    tankGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = tankGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
       // relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, UNIT_HAS_PIPELINE_REL_CLASS);
       // relationShipClass = relClass->GetRelationshipClassCP();
       // functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }

    if (m_room.IsValid())
        {
        Utf8String locationCode = m_roomShortCode + "-" + shortCode;
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, "FunctionalBreakdownGroupsFunctionalElements");
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, m_room->GetElementId(), fe->GetElementId());

        Dgn::DgnElementPtr ele = m_dgnDb->Elements().GetForEdit<Dgn::DgnElement>(fe->GetElementId());

        ECN::IECInstancePtr instance = AddAspect(functionalModel, ele, BREAKDOWN_DOMAIN, "LocationCode");

        PopulateInstanceCodes(instance, ROOM_CODESPEC_NAME, "Plant:Functional", locationCode);

        ele->Update();

        }


    EndBulkOperation();

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateVessel(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(EQUIPMENT_DOMAIN, VESSEL_CLASS, functionalModel);

    Utf8String shortCode;
    int        number = 0;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, nullptr, EQUIPMENT_CODESPEC_NAME, VESSEL_CODE_PATTERN);

    PopulateElementProperties(functionalElement);
    PopulateEquipmentProperties(functionalElement, "V", "Horizontal Vessel", number);

    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("ProcessEquipmentFunctional", "SubUnitOwnsEquipment");
    functionalElement->SetParentId(subUnitId, relClass->GetId());

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, EQUIPMENT_CODESPEC_NAME);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the Vessel

    Dgn::DrawingGraphicPtr vesselGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!vesselGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    vesselGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_EQUIP_SUBCATEGORY);

    GeometricTools::CreatePidVesselGeometry(*vesselGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    vesselGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = vesselGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (m_room.IsValid())
        {
        Utf8String locationCode = m_roomShortCode + "-" + shortCode;
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, "FunctionalBreakdownGroupsFunctionalElements");
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, m_room->GetElementId(), fe->GetElementId());

        Dgn::DgnElementPtr ele = m_dgnDb->Elements().GetForEdit<Dgn::DgnElement>(fe->GetElementId());

        ECN::IECInstancePtr instance = AddAspect(functionalModel, ele, BREAKDOWN_DOMAIN, "LocationCode");

        PopulateInstanceCodes(instance, ROOM_CODESPEC_NAME, "Plant:Functional", locationCode);

        ele->Update();

        }

    EndBulkOperation();

    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ECN::IECInstancePtr BimCreater::AddAspect(Dgn::DgnModelR model, Dgn::DgnElementPtr element, Utf8StringCR schemaName, Utf8StringCR className)
    {

    // Find the class

    ECN::ECClassCP aspectClassP = model.GetDgnDb().GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

    if (nullptr == aspectClassP)
        return nullptr;

    // If the element is already persisted and has the Aspect class, you can't add another

    if (element->GetElementId().IsValid())
        {
        ECN::IECInstanceCP instance = Dgn::DgnElement::GenericUniqueAspect::GetAspect(*element, *aspectClassP);

        if (nullptr != instance)
            return nullptr;
        }

    ECN::StandaloneECEnablerPtr enabler = aspectClassP->GetDefaultStandaloneEnabler();

    if (!enabler.IsValid())
        return nullptr;

    ECN::IECInstancePtr instance = enabler->CreateInstance().get();
    if (!instance.IsValid())
        return nullptr;

    Dgn::DgnDbStatus status = Dgn::DgnElement::GenericUniqueAspect::SetAspect(*element, *instance);

    if (Dgn::DgnDbStatus::Success != status)
        return nullptr;

    return instance;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreatePump(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Pump Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(EQUIPMENT_DOMAIN, PUMP_CLASS, functionalModel);

    Utf8String shortCode;
    int        number = 0;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, nullptr, EQUIPMENT_CODESPEC_NAME, PUMP_CODE_PATTERN);

    PopulateElementProperties(functionalElement);
    PopulateEquipmentProperties(functionalElement, "PMP", "Primary Pump", number);

    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("ProcessEquipmentFunctional", "SubUnitOwnsEquipment");

    functionalElement->SetParentId(subUnitId, relClass->GetId());

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, EQUIPMENT_CODESPEC_NAME);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the Pump

    Dgn::DrawingGraphicPtr pumpGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!pumpGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    pumpGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_EQUIP_SUBCATEGORY);

    GeometricTools::CreatePidPumpGeometry(*pumpGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    pumpGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = pumpGraphic->Insert();

    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (m_room.IsValid())
        {
        Utf8String locationCode = m_roomShortCode + "-" + shortCode;
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, "FunctionalBreakdownGroupsFunctionalElements");
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, m_room->GetElementId(), fe->GetElementId());

        Dgn::DgnElementPtr ele = m_dgnDb->Elements().GetForEdit<Dgn::DgnElement>(fe->GetElementId());

        ECN::IECInstancePtr instance = AddAspect(functionalModel, ele, BREAKDOWN_DOMAIN, "LocationCode");

        PopulateInstanceCodes(instance, ROOM_CODESPEC_NAME, "Plant:Functional", locationCode);

        ele->Update();

        }

    EndBulkOperation();
    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateReducer(Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Utf8StringCR reducerLabel)
    {

    Dgn::DrawingGraphicPtr reducerGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!reducerGraphic.IsValid())
        return nullptr;

    reducerGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_PIPING_SUBCATEGORY);

    GeometricTools::CreatePidReducerGeometry(*reducerGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(reducerLabel.c_str());
    reducerGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = reducerGraphic->Insert();

    // Create Reducer Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, CONCENTRIC_REDUCER_CLASS, functionalModel);

    functionalElement->SetPropertyValue(USERLABEL_NAME, value);

    PopulateElementProperties(functionalElement);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateGateValve(Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create the functional component for the Gate Valve

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, GATE_VALVE_CLASS, functionalModel);

    Utf8String shortCode;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, parentElement, VALVE_CODESPEC_NAME, VALVE_CODE_PATTERN);

    PopulateElementProperties(functionalElement);

    ECN::ECClassCP relClass;

#ifdef USE_PROTOTYPE
    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunOwnsFunctionalPipingComponents);
        functionalElement->SetParentId(pipeRunId, relClass->GetId());
        }
#endif

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, VALVE_CODESPEC_NAME);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the valve

    Dgn::DrawingGraphicPtr valveGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!valveGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    valveGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_PIPING_SUBCATEGORY);

    GeometricTools::CreatePidValveGeometry(*valveGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    valveGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = valveGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

#ifndef USE_PROTOTYPE
    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(PIPING_DOMAIN, "SEGMENT_HAS_PIPING_COMPONENT");
    relationShipClass = relClass->GetRelationshipClassCP();
    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, pipeRunId, fe->GetElementId());
#endif

    EndBulkOperation();

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateThreeWayValve(Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create the functional component for the three way valve
    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, THREE_WAY_VALVE_CLASS, functionalModel);

    Utf8String shortCode;
    int        number = 0;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *functionalElement, parentElement, VALVE_CODESPEC_NAME, VALVE_CODE_PATTERN);

    ECN::ECClassCP relClass;
#ifdef USE_PROTOTYPE
    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunOwnsFunctionalPipingComponents);
        functionalElement->SetParentId(pipeRunId, relClass->GetId());
        }
#endif

    StartBulkOperation();
    SetCodeForElement(codeString, *functionalElement, VALVE_CODESPEC_NAME);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    Dgn::DrawingGraphicPtr valveGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!valveGraphic.IsValid())
        {
        EndBulkOperation();
        return nullptr;
        }

    valveGraphic->SetPlacement(placement);
    Dgn::DgnSubCategoryId subCategoryId;
    m_host->FindOrCreateDrawingSubCategory(categoryId, subCategoryId, PID_PIPING_SUBCATEGORY);

    GeometricTools::CreatePid3WayValveGeometry(*valveGraphic, categoryId, subCategoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    valveGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = valveGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

#ifndef USE_PROTOTYPE
    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(PIPING_DOMAIN, "SEGMENT_HAS_PIPING_COMPONENT");
    relationShipClass = relClass->GetRelationshipClassCP();
    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, pipeRunId, fe->GetElementId());
#endif

    EndBulkOperation();
    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DrawingModelPtr BimCreater::CreatePidDrawings(Dgn::DocumentListModelR docListModel, Dgn::FunctionalModelR functionalModel, Utf8StringCR drawingCode, Dgn::DgnElementCPtr subUnit )
	{

    Dgn::DgnDbR db = docListModel.GetDgnDb();

    db.BriefcaseManager().StartBulkOperation();

	// Create a drawing model

	Dgn::DrawingModelPtr drawingModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingDrawingModel(drawingCode, docListModel.GetDgnDb(), docListModel);

//	Dgn::DgnDbR db = drawingModel->GetDgnDb();
	Dgn::DgnModelId modelId = drawingModel->GetModelId();

    Dgn::DgnCategoryId categoryId;

    m_host->FindOrCreateDrawingCategory(*m_dgnDb, categoryId, PID_CATEGORY);

    db.BriefcaseManager().EndBulkOperation();

    ECN::ECClassCP              relClass;
    ECN::ECRelationshipClassCP  relationShipClass;
    BeSQLite::EC::ECInstanceKey rkey;

    // Add all the equipment to the PID

    Dgn::Placement2d       placement;
    Dgn::DrawingGraphicPtr annotation;

    // **** Add the Tank ****

    placement.GetOriginR() = DPoint2d::From(-91.256, 6.25);
    Dgn::FunctionalComponentElementCPtr tank = CreateTank(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(-75, 19.375);
    annotation = CreateAnnotation(categoryId, *drawingModel, tank->GetPropertyValueString(USERLABEL_NAME), placement);
    EndBulkOperation();

    // ******* Add the Tank Nozzle *****

    placement.GetOriginR() = DPoint2d::From(-60.006, 19.375);
    Dgn::DgnElementId id;
    Dgn::FunctionalComponentElementPtr tankNozzle = CreateNozzle(id, tank->GetElementId(), categoryId, functionalModel, *drawingModel, placement, tank);


    // ******* Add the Pump ********

    placement.GetOriginR() = DPoint2d::From(0, 0);
    Dgn::FunctionalComponentElementCPtr pump = CreatePump(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(0.0, -5);
    annotation = CreateAnnotation(categoryId, *drawingModel, pump->GetPropertyValueString(USERLABEL_NAME), placement);
    EndBulkOperation();

    // ****** Add Nozzle to Pump Center ******

    placement.GetOriginR() = DPoint2d::From(0.0,0.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0.0);
    Dgn::FunctionalComponentElementPtr pumpNozzle1 = CreateNozzle(id, pump->GetElementId(), categoryId, functionalModel, *drawingModel, placement, pump, true);

    placement.GetOriginR() = DPoint2d::From(4.637, 2.5);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0.0);
    Dgn::FunctionalComponentElementPtr pumpNozzle2 = CreateNozzle(id, pump->GetElementId(), categoryId, functionalModel, *drawingModel, placement, pump, true);


    // **** Add the Round Tank ****

    placement.GetOriginR() = DPoint2d::From(57.494, 26.25);
    Dgn::FunctionalComponentElementCPtr roundTank = CreateRoundTank(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(57.494, 26.25);
    annotation = CreateAnnotation(categoryId, *drawingModel, roundTank->GetPropertyValueString(USERLABEL_NAME), placement);
    EndBulkOperation();

    // ****** Add Nozzle to Round Tank ******

    placement.GetOriginR() = DPoint2d::From(47.494, 28.75);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(180.0);
    Dgn::FunctionalComponentElementPtr roundTankNozzle = CreateNozzle(id, roundTank->GetElementId(), categoryId, functionalModel, *drawingModel, placement, roundTank);


    // **** Add the Vessel ****

    placement.GetOriginR() = DPoint2d::From(51.244, -20.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0);
    Dgn::FunctionalComponentElementCPtr vessel = CreateVessel(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);


    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(58.744, -20.0);
    annotation = CreateAnnotation(categoryId, *drawingModel, vessel->GetPropertyValueString(USERLABEL_NAME), placement);
    EndBulkOperation();

    // ****** Add Nozzle to Vessle ******

    placement.GetOriginR() = DPoint2d::From(56.244, -15.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(90.0);
    Dgn::FunctionalComponentElementPtr vesselNozzle = CreateNozzle(id, vessel->GetElementId(), categoryId, functionalModel, *drawingModel, placement, vessel);


    // Create the pipeline breakdown element. 

#ifdef USE_PROTOTYPE
    Dgn::FunctionalBreakdownElementPtr pipeline = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(PIPING_DOMAIN, PIPELINE_CLASS, functionalModel);
#else
    Dgn::FunctionalComponentElementPtr pipeline = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(PIPING_DOMAIN, PIPELINE_CLASS, functionalModel);
#endif

    Utf8String shortCode;
//    int        number;

    Utf8String codeString = GetCodeFromParentCode(shortCode, *pipeline, subUnit, PIPELINE_CODESPEC_NAME, m_systemShortCode + "-" PIPELINE_CODE_PATTERN + "-CS150");

    PopulateElementProperties(pipeline);

    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(PIPING_DOMAIN, "SubUnitOwnsPipelines");
    relationShipClass = relClass->GetRelationshipClassCP();

    StartBulkOperation();
    SetCodeForElement(codeString, *pipeline, PIPELINE_CODESPEC_NAME);
    pipeline->SetParentId(subUnit->GetElementId(), relClass->GetId());
    Dgn::DgnElementCPtr pl = pipeline->Insert();

    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, "FunctionalBreakdownGroupsFunctionalElements");
    relationShipClass = relClass->GetRelationshipClassCP();
    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, m_system->GetElementId(), pl->GetElementId());
    EndBulkOperation();

    // **** Add the Reducer ****

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(-30.756, 0.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0.0);
    Dgn::FunctionalComponentElementPtr reducer = CreateReducer(categoryId, functionalModel, *drawingModel, placement, "4x2");

    placement.GetOriginR() = DPoint2d::From(-30.156, -2.0);
    annotation = CreateAnnotation(categoryId, *drawingModel, "4x2", placement);
    EndBulkOperation();



    // PipeRun from Tank to the reducer

    PIPERUN_TYPEPTR pipeRun1 = CreatePipeRun((PIPERUN_TYPECPTR)pipeline, reducer->GetElementId(), tankNozzle->GetElementId(), functionalModel);

    // PipeRun from Reducer to the Pump

    PIPERUN_TYPEPTR pipeRun2 = CreatePipeRun((PIPERUN_TYPECPTR)pipeline, pumpNozzle1->GetElementId(), reducer->GetElementId(), functionalModel);

    // PipeRun from Round Tank to the Vessel

    PIPERUN_TYPEPTR pipeRun4 = CreatePipeRun((PIPERUN_TYPECPTR)pipeline, vesselNozzle->GetElementId(), roundTankNozzle->GetElementId(), functionalModel);


    // **** Add 3 way Valve ****

    placement.GetOriginR() = DPoint2d::From(28.494, 2.5);
    Dgn::FunctionalComponentElementPtr threeWayValve = CreateThreeWayValve(pipeRun4->GetElementId(), categoryId, functionalModel, *drawingModel, placement, m_area);

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(33.5, 2.5);
    annotation = CreateAnnotation(categoryId, *drawingModel, threeWayValve->GetUserLabel(), placement);
    
    // PipeRun from Pump to the 3 way valves

    PIPERUN_TYPEPTR pipeRun3 = CreatePipeRun((PIPERUN_TYPECPTR)pipeline, threeWayValve->GetElementId(), pumpNozzle2->GetElementId(), functionalModel);

    EndBulkOperation();

    // ******** Add the Gate Valve ******

    placement.GetOriginR() = DPoint2d::From(-48.361, 19.375);
    Dgn::FunctionalComponentElementPtr gateValve = CreateGateValve(pipeRun1->GetElementId(), categoryId, functionalModel, *drawingModel, placement, m_area);

    StartBulkOperation();
    placement.GetOriginR() = DPoint2d::From(-46.861, 17.375);
    annotation = CreateAnnotation(categoryId, *drawingModel, gateValve->GetUserLabel(), placement);

    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(BREAKDOWN_DOMAIN, "FunctionalBreakdownGroupsFunctionalElements");
    relationShipClass = relClass->GetRelationshipClassCP();
    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, m_area->GetElementId(), gateValve->GetElementId());


    // Add the PipeRun graphics for the fist PipeRun

    DPoint2d points[7];
    points[0] = DPoint2d::From(-59.256, 19.375);
    points[1] = DPoint2d::From(-48.361, 19.375);
    points[2] = DPoint2d::From(-45.361, 19.375);
    points[3] = DPoint2d::From(-35.006, 19.375);
    points[4] = DPoint2d::From(-35.006, 0.0);
    points[5] = DPoint2d::From(-30.756, 0.0);

    bvector<int> counts;
    counts.push_back(2);
    counts.push_back(4);

    CreatePipeRunGraphics(pipeRun1, categoryId, *drawingModel, points, counts);

    // Add the PipeRun from the Reducer to the Pump

    points[0] = DPoint2d::From(-29.256, 0.0);
    points[1] = DPoint2d::From(0.0, 0.0);

    counts.clear();
    counts.push_back(2);

    CreatePipeRunGraphics(pipeRun2, categoryId, *drawingModel, points, counts);

    points[0] = DPoint2d::From(30.0, 4.0);
    points[1] = DPoint2d::From(30.0, 28.75);
    points[2] = DPoint2d::From(46.743, 28.75);

    points[3] = DPoint2d::From(30.0, 1.0);
    points[4] = DPoint2d::From(30.0, -8.75);
    points[5] = DPoint2d::From(56.25, -8.75);
    points[6] = DPoint2d::From(56.25, -14.249);

    counts.clear();
    counts.push_back(3);
    counts.push_back(4);

    CreatePipeRunGraphics(pipeRun4, categoryId, *drawingModel, points, counts);

    points[0] = DPoint2d::From(4.637, 2.5);
    points[1] = DPoint2d::From(28.494, 2.5);
    counts.clear();
    counts.push_back(2);

    CreatePipeRunGraphics(pipeRun3, categoryId, *drawingModel, points, counts);

    EndBulkOperation();

	return drawingModel;

	}


BeSQLite::DbResult BimCreater::RegisterMyDomains()
    {

    if (BentleyStatus::SUCCESS != BuildingDomain::BuildingDomainUtilities::RegisterDomainHandlers())
        return BeSQLite::DbResult::BE_SQLITE_ERROR;

#ifdef USE_PROTOTYPE
    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantPrototypeBIM::PlantBreakdownFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BeSQLite::DbResult::BE_SQLITE_ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantPrototypeBIM::ProcessEquipmentFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BeSQLite::DbResult::BE_SQLITE_ERROR;


    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantPrototypeBIM::ProcessPipingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BeSQLite::DbResult::BE_SQLITE_ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantPrototypeBIM::ProcessPipingFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BeSQLite::DbResult::BE_SQLITE_ERROR;
#else
    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantBIM::ProcessPlantFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BeSQLite::DbResult::BE_SQLITE_ERROR;
#endif

    return BeSQLite::DbResult::BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BeSQLite::DbResult BimCreater::CreateLocal(Dgn::DgnDbPtr db)
    {


    BeSQLite::DbResult createStatus;
//    Dgn::DgnDbPtr db = CreateDgnDb(GetOutputFileName(), &createStatus);
//    if (!db.IsValid())
//        return createStatus;

    // Create the models in the BIM file

    BentleyStatus status = BuildingDomain::BuildingDomainUtilities::CreateBuildingModels("Plant", *db);

    if (BentleyStatus::SUCCESS != status)
        return BeSQLite::DbResult::BE_SQLITE_ERROR;

    BuildingPhysical::BuildingPhysicalModelPtr       physicalModel       = BuildingDomain::BuildingDomainUtilities::CreateBuildingPhyicalModel("Plant", *db);
    BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingTypeDefinitionModel("Plant", *db);
    Dgn::DocumentListModelPtr                        docListModel        = BuildingDomain::BuildingDomainUtilities::CreateBuildingDocumentListModel("PID", *db);
    Dgn::FunctionalModelPtr                          functionalModel     = BuildingDomain::BuildingDomainUtilities::CreateBuildingFunctionalModel("Plant", *db);
    Dgn::DefinitionModelR                            dictionary          = db->GetDictionaryModel();

    

    Utf8String prefix     = "PlantFunctional";
    Utf8String classScope = "Equipment";

    Dgn::CodeSpecPtr equipCodeSpec = Dgn::CodeSpec::Create(*db, EQUIPMENT_CODESPEC_NAME/*CreateCodeSpecName (prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (equipCodeSpec.IsValid())
        equipCodeSpec->Insert();

    classScope = "Nozzle";
    Dgn::CodeSpecPtr nozzleCodeSpec = Dgn::CodeSpec::Create(*db, NOZZLE_CODESPEC_NAME/*CreateCodeSpecName (prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (nozzleCodeSpec.IsValid())
        nozzleCodeSpec->Insert();

    classScope = "Plant";
    Dgn::CodeSpecPtr plantCodeSpec = Dgn::CodeSpec::Create(*db, PLANT_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (plantCodeSpec.IsValid())
        plantCodeSpec->Insert();

    classScope = "Unit";
    Dgn::CodeSpecPtr unitCodeSpec = Dgn::CodeSpec::Create(*db, UNIT_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (unitCodeSpec.IsValid())
        unitCodeSpec->Insert();

    classScope = "Building";
    Dgn::CodeSpecPtr buildignCodeSpec = Dgn::CodeSpec::Create(*db, BUILDING_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (buildignCodeSpec.IsValid())
        buildignCodeSpec->Insert();

    classScope = "Floor";
    Dgn::CodeSpecPtr floorCodeSpec = Dgn::CodeSpec::Create(*db, FLOOR_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (floorCodeSpec.IsValid())
        floorCodeSpec->Insert();

    classScope = "Room";
    Dgn::CodeSpecPtr roomCodeSpec = Dgn::CodeSpec::Create(*db, ROOM_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (roomCodeSpec.IsValid())
        roomCodeSpec->Insert();

    classScope = "SubUnit";
    Dgn::CodeSpecPtr subUnitCodeSpec = Dgn::CodeSpec::Create(*db, SUBUNIT_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (subUnitCodeSpec.IsValid())
        subUnitCodeSpec->Insert();

    classScope = "PID";
    Dgn::CodeSpecPtr pidCodeSpec = Dgn::CodeSpec::Create(*db, PID_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (pidCodeSpec.IsValid())
        pidCodeSpec->Insert();

    classScope = "Pipeline";
    Dgn::CodeSpecPtr pipelineCodeSpec = Dgn::CodeSpec::Create(*db, PIPELINE_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (pipelineCodeSpec.IsValid())
        pipelineCodeSpec->Insert();

    classScope = "Valve";
    Dgn::CodeSpecPtr valveCodeSpec = Dgn::CodeSpec::Create(*db, VALVE_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (valveCodeSpec.IsValid())
        valveCodeSpec->Insert();

    classScope = "System";
    Dgn::CodeSpecPtr systemCodeSpec = Dgn::CodeSpec::Create(*db, SYSTEM_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (systemCodeSpec.IsValid())
        systemCodeSpec->Insert();

    classScope = "Area";
    Dgn::CodeSpecPtr areaCodeSpec = Dgn::CodeSpec::Create(*db, AREA_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateModelScope());
    if (areaCodeSpec.IsValid())
        areaCodeSpec->Insert();

    classScope = "PipeRun";
    Dgn::CodeSpecPtr pipeRunCodeSpec = Dgn::CodeSpec::Create(*db, PIPERUN_CODESPEC_NAME /*CreateCodeSpecName(prefix, classScope).c_str()*/, Dgn::CodeScopeSpec::CreateRelatedElementScope());
    if (pipeRunCodeSpec.IsValid())
        pipeRunCodeSpec->Insert();


    return db->SaveChanges("Initial BIM Setup");

    }

    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::AddPids()
    {
    BuildingPhysical::BuildingPhysicalModelPtr       physicalModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel("Plant", *m_dgnDb);
    BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel("Plant", *m_dgnDb);
    Dgn::DocumentListModelPtr                        docListModel = BuildingDomain::BuildingDomainUtilities::GetBuildingDocumentListModel("PID", *m_dgnDb);
    Dgn::FunctionalModelPtr                          functionalModel = BuildingDomain::BuildingDomainUtilities::GetBuildingFunctionalModel("Plant", *m_dgnDb);
    Dgn::DefinitionModelR                            dictionary = m_dgnDb->GetDictionaryModel();

    StartBulkOperation();
    Dgn::CategorySelectorPtr  categorySelector = CreateCategorySelector(dictionary);
    Dgn::DisplayStyle2dPtr    displayStyle1 = CreateDisplayStyle2d(dictionary);
    EndBulkOperation();

    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode ( ROOM_CODESPEC_NAME, *functionalModel, m_roomCode);

    Dgn::DgnElementId id = m_dgnDb->Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        {
        m_room = m_dgnDb->Elements().Get<Dgn::DgnElement>(id);
        m_roomShortCode = m_roomCode;// m_room->GetUserLabel();
        }

    code = Dgn::CodeSpec::CreateCode(SYSTEM_CODESPEC_NAME, *functionalModel, m_systemCode);

    id = m_dgnDb->Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        {
        m_system = m_dgnDb->Elements().Get<Dgn::DgnElement>(id);
        m_systemShortCode = m_system->GetUserLabel();
        }

    code = Dgn::CodeSpec::CreateCode(AREA_CODESPEC_NAME, *functionalModel,  m_areaCode);

    id = m_dgnDb->Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        {
        m_area = m_dgnDb->Elements().Get<Dgn::DgnElement>(id);
        m_areaShortCode = m_area->GetUserLabel();
        }

    code = Dgn::CodeSpec::CreateCode(SUBUNIT_CODESPEC_NAME, *functionalModel, m_subUnitCode);

    id = m_dgnDb->Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        {
        m_subUnit = m_dgnDb->Elements().Get<Dgn::DgnElement>(id);
        m_subUnitShortCode = m_subUnit->GetUserLabel();
        }


    int numberOfPids = 3;

    for (int j = 0; j < numberOfPids; j++)
        {
        Utf8String pidCode;

        m_host->GetNextCodeValue(pidCode, PID_CODESPEC_NAME, "Plant:Functional", PID_CODE_PATTERN);

        Dgn::DrawingModelPtr drawingModel = CreatePidDrawings(*docListModel, *functionalModel, pidCode, m_subUnit);

        StartBulkOperation();
        Dgn::DgnViewId viewId = CreateView2d(dictionary, pidCode.c_str(), *categorySelector, drawingModel->GetModelId(), *displayStyle1);
        EndBulkOperation();

        if (!viewId.IsValid())
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::DoCreate(Dgn::DgnDbPtr db, BentleyApi::PlantBIM::PlantHostP host)
    {
	BuildingPhysical::BuildingPhysicalModelPtr       physicalModel       = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel         ("Plant", *db);
	BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel  ("Plant", *db);
	Dgn::DocumentListModelPtr                        docListModel        = BuildingDomain::BuildingDomainUtilities::GetBuildingDocumentListModel    ("PID",   *db);
	Dgn::FunctionalModelPtr                          functionalModel     = BuildingDomain::BuildingDomainUtilities::GetBuildingFunctionalModel      ("Plant", *db);
	Dgn::DefinitionModelR                            dictionary          = db->GetDictionaryModel();

    StartBulkOperation();
	Dgn::CategorySelectorPtr  categorySelector = CreateCategorySelector(dictionary);
	Dgn::DisplayStyle2dPtr    displayStyle1    = CreateDisplayStyle2d(dictionary);
    EndBulkOperation();

    Utf8String shortCode;
    int        number;

    // This is where we create add the information to the BIM file. You can control the number of Units, SubUnits and P&ID. 

    int numberOfUnits = 2;
    int numberOfSubunits = 2;
    int numberOfPids = 2;

    int numberOfFloors = 3;
    int numberOfRooms = 4;

    Dgn::DgnDbStatus dbStatus;

#ifdef USE_PROTOTYPE
    PIPERUN_TYPEPTR plant = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, PLANT_CLASS, *functionalModel);
#else
    PIPERUN_TYPEPTR plant = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(BREAKDOWN_DOMAIN, PLANT_CLASS, *functionalModel);
#endif

    plant->SetPropertyValue("IsTreeRoot", true);

    Utf8String plantCode =  GetCodeFromParentCode(shortCode, *plant, nullptr, PLANT_CODESPEC_NAME, PLANT_CODE_PATTERN);

    StartBulkOperation();
    SetCodeForElement(plantCode, *plant, PLANT_CODESPEC_NAME);
    Dgn::DgnElementCPtr plt = plant->Insert(&dbStatus);
    EndBulkOperation();

    PIPERUN_TYPEPTR building = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, "Building", *functionalModel);

    building->SetPropertyValue("IsTreeRoot", true);

    Utf8String buildingCode = GetCodeFromParentCode(shortCode, *building, nullptr, BUILDING_CODESPEC_NAME, BUILDING_CODE_PATTERN);

    StartBulkOperation();
    SetCodeForElement(buildingCode, *building, BUILDING_CODESPEC_NAME);
    Dgn::DgnElementCPtr bld = building->Insert(&dbStatus);
    EndBulkOperation();

    PIPERUN_TYPEPTR system = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, "System", *functionalModel);

    system->SetPropertyValue("IsTreeRoot", true);

    Utf8String systemCode = GetCodeFromParentCode(m_systemShortCode, *system, nullptr, SYSTEM_CODESPEC_NAME, SYSTEM_CODE_PATTERN);

    StartBulkOperation();
    SetCodeForElement(systemCode, *system, SYSTEM_CODESPEC_NAME);
    m_system = system->Insert(&dbStatus);
    EndBulkOperation();

    PIPERUN_TYPEPTR area = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, "PlantArea", *functionalModel);

    area->SetPropertyValue("IsTreeRoot", true);

    Utf8String areaCode = GetCodeFromParentCode(m_areaShortCode, *area, nullptr, AREA_CODESPEC_NAME, AREA_CODE_PATTERN);

    StartBulkOperation();
    SetCodeForElement(areaCode, *area, AREA_CODESPEC_NAME);
    m_area = area->Insert(&dbStatus);
    EndBulkOperation();


    for (int floorNum = 0; floorNum < numberOfFloors; floorNum++)
        {

#ifdef USE_PROTOTYPE
        PIPERUN_TYPEPTR floor = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, "Floor", *functionalModel);
#else
        PIPERUN_TYPEPTR unit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(BREAKDOWN_DOMAIN, UNIT_CLASS, *functionalModel);
#endif

        ECN::ECClassCP             relClass = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, "BuildingOwnsFloors");
        ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();

        dbStatus = floor->SetParentId(bld->GetElementId(), relationShipClass->GetId());

        PopulateElementProperties(floor);

        Utf8String floorCode = GetCodeFromParentCode(shortCode, *floor, bld, FLOOR_CODESPEC_NAME, FLOOR_CODE_PATTERN);

        StartBulkOperation();
        SetCodeForElement(floorCode, *floor, FLOOR_CODESPEC_NAME);
        Dgn::DgnElementCPtr fl = floor->Insert(&dbStatus);
        EndBulkOperation();

        for (int roomNum = 0; roomNum < numberOfRooms; roomNum++)
            {
            Dgn::FunctionalBreakdownElementPtr room = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, "Room", *functionalModel);

            ECN::ECClassCP             relClass = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, "FloorOwnsRooms");
            ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();

            dbStatus = room->SetParentId(fl->GetElementId(), relationShipClass->GetId());

            Utf8String roomCode = GetCodeFromParentCode(shortCode, *room, fl, ROOM_CODESPEC_NAME, ROOM_CODE_PATTERN);

            StartBulkOperation();
            SetCodeForElement(roomCode, *room, ROOM_CODESPEC_NAME);

            // SetCodeFromParent1(number, shortCode, *subUnit, un, "SU");

            m_roomShortCode = roomCode;
            m_room = room->Insert(&dbStatus);

            EndBulkOperation();
            }
        }


    for (int unitNum = 0; unitNum < numberOfUnits; unitNum++)
        {

#ifdef USE_PROTOTYPE
        PIPERUN_TYPEPTR unit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(BREAKDOWN_DOMAIN, UNIT_CLASS, *functionalModel);
#else
        PIPERUN_TYPEPTR unit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(BREAKDOWN_DOMAIN, UNIT_CLASS, *functionalModel);
#endif

        ECN::ECClassCP             relClass          = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, "PlantOwnsUnits");
        ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();

        dbStatus = unit->SetParentId(plt->GetElementId(), relationShipClass->GetId());

        PopulateElementProperties(unit);

        Utf8String unitCode = GetCodeFromParentCode(shortCode, *unit, plt, UNIT_CODESPEC_NAME, UNIT_CODE_PATTERN);

        StartBulkOperation();
        SetCodeForElement(unitCode, *unit, UNIT_CODESPEC_NAME);
        Dgn::DgnElementCPtr un = unit->Insert(&dbStatus);
        EndBulkOperation();

        //ICurvePrimitivePtr buildingGeometry = GeometricTools::CreateContainmentBuildingGeometry(50, 250);

        //CurveVectorPtr a = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
        
        //a->Add(buildingGeometry);

        //Building::SpacePlanning::BuildingPtr containmentBuilding = Building::SpacePlanning::Building::Create ( *physicalModel, a, 250.0);

        //Building::SpacePlanning::BuildingRequirementPtr building = Building::SpacePlanning::BuildingRequirement::Create(*db, functionalModel->GetModelId());

        //Dgn::DgnElementCPtr bldg = containmentBuilding->Insert();
        //Dgn::DgnElementCPtr fbldg = building->Insert();


        for (int subUnitNum = 0; subUnitNum < numberOfSubunits; subUnitNum++)
            {

#ifdef USE_PROTOTYPE
            Dgn::FunctionalBreakdownElementPtr subUnit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Class_SubUnit, *functionalModel);

            ECN::ECClassCP             relClass = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, "UnitOwnsSubUnits");
            ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();

            dbStatus = subUnit->SetParentId(un->GetElementId(), relationShipClass->GetId());

            Utf8String subUnitCode = GetCodeFromParentCode(shortCode, *subUnit, un, SUBUNIT_CODESPEC_NAME, SUBUNIT_CODE_PATTERN);

            StartBulkOperation();
            SetCodeForElement(subUnitCode, *subUnit, SUBUNIT_CODESPEC_NAME);

            // SetCodeFromParent1(number, shortCode, *subUnit, un, "SU");

            Dgn::DgnElementCPtr subUn = subUnit->Insert(&dbStatus);
            EndBulkOperation();

            //ECN::ECClassCP relClass = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
            //ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
            //BeSQLite::EC::ECInstanceKey rkey;
            //db->InsertLinkTableRelationship(rkey, *relationShipClass, un->GetElementId(), subUn->GetElementId());
#endif

            for (int j = 1; j < numberOfPids; j++)
                {
//                Utf8String pidName;
//                pidName.Sprintf("PID-%0.3d", pidNum);
//                pidNum++;

                Utf8String pidCode;

                host->GetNextCodeValue(pidCode, PID_CODESPEC_NAME, "Plant:Functional", PID_CODE_PATTERN);


                Dgn::DgnElementCPtr a = unit;
#ifdef USE_PROTOTYPE
                Dgn::DrawingModelPtr drawingModel = CreatePidDrawings(*docListModel, *functionalModel, pidCode, subUn);
#else
                Dgn::DrawingModelPtr drawingModel = CreatePidDrawings(*docListModel, *functionalModel, pidName, a);
#endif
                StartBulkOperation();
                Dgn::DgnViewId viewId = CreateView2d(dictionary, pidCode.c_str(), *categorySelector, drawingModel->GetModelId(), *displayStyle1);
                EndBulkOperation();

                if (!viewId.IsValid())
                    return BentleyStatus::ERROR;
                }
            host->SaveAllChanges();
            host->SyncBriefcase();
            }
        }

    db->BriefcaseManager().StartBulkOperation();

    // The 3D physical element will start out with some Architectural components. 

    CreateBuilding( *physicalModel, *typeDefinitionModel);

   // Set the project extents to include the elements in the physicalModel, plus a margin
    Dgn::AxisAlignedBox3d projectExtents = physicalModel->QueryModelRange();
    projectExtents.Extend(0.5);
    db->GeoLocation().SetProjectExtents(projectExtents);

    // Create the initial view
    Dgn::ModelSelectorPtr modelSelector = CreateModelSelector(dictionary, *physicalModel, "Default3D");
	Dgn::DisplayStyle3dPtr displayStyle = CreateDisplayStyle3d(dictionary);

	Dgn::DgnViewId viewId = CreateView(dictionary, "Building View", *categorySelector, *modelSelector, *displayStyle);

    db->BriefcaseManager().EndBulkOperation();

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Dgn::DgnViewId BimCreater::CreateView(Dgn::DefinitionModelR model, Utf8CP name, Dgn::CategorySelectorR categorySelector, Dgn::ModelSelectorR modelSelector, Dgn::DisplayStyle3dR displayStyle)
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
Dgn::DgnViewId BimCreater::CreateView2d(Dgn::DefinitionModelR model, Utf8CP name, Dgn::CategorySelectorR categorySelector, Dgn::DgnModelId baseModelId, Dgn::DisplayStyle2dR displayStyle)
	{
	// CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
	// That is why they are inputs to this function. 
	//Dgn::OrthographicViewDefinition view(model, name, categorySelector, displayStyle, modelSelector);

	Dgn::DrawingViewDefinition view(model, name, baseModelId, categorySelector, displayStyle);

	// Define the view direction and volume.
	Dgn::DgnDbR db = model.GetDgnDb();

	DPoint3d points[3];

	points[0] = DPoint3d::From(-100, -40);
	points[1] = DPoint3d::From(100,  60);

	DRange3d range = DRange3d::From(points, 2);
	//view.SetStandardViewRotation(Dgn::StandardView::Iso); // Default to a rotated view
	view.LookAtVolume(range); // A good default for a new view is to "fit" it to the contents of the bim.

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

BentleyStatus BimCreater::PopulateInstanceCodes(ECN::IECInstancePtr instance, Utf8String codeSpec, Utf8String codeScope, Utf8String codeValue)
    {

    ECN::ECValue stringValue;

    stringValue.SetUtf8CP(codeSpec.c_str());
    instance->SetValue("CodeSpec", stringValue);

    stringValue.SetUtf8CP(codeScope.c_str());
    instance->SetValue("CodeScope", stringValue);

    stringValue.SetUtf8CP(codeValue.c_str());
    instance->SetValue("CodeValue", stringValue);

    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::PopulateInstanceProperties(ECN::IECInstancePtr instance)
    {
    ECN::ECClassCR ecClass = instance->GetClass();

    int i = 0;

    for each (ECN::ECPropertyP prop in ecClass.GetProperties())
        {
        if (nullptr == prop || prop->GetName() == "Roll" || prop->GetName() == "Pitch" || prop->GetName() == "Yaw" )
            continue;

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

BentleyStatus BimCreater::PopulateEquipmentProperties(Dgn::DgnElementPtr element, Utf8StringCR deviceTypeCode, Utf8StringCR description, int number)
    {

    ECN::ECValue stringValue;
    ECN::ECValue doubleValue;
    ECN::ECValue intValue;

    stringValue.SetUtf8CP(deviceTypeCode.c_str());
    element->SetPropertyValue("DeviceTypeCode", stringValue);
    element->SetPropertyValue("DEVICE_TYPE_CODE", stringValue);
    stringValue.SetUtf8CP(description.c_str());
    element->SetPropertyValue("Description", stringValue);
    element->SetPropertyValue("DESCRIPTION", stringValue);
    stringValue.SetUtf8CP("");
    element->SetPropertyValue("Comment", stringValue);
    element->SetPropertyValue("Alias", stringValue);
    element->SetPropertyValue("AdditionalCode", stringValue);
    element->SetPropertyValue("UnitClassification", stringValue);
    element->SetPropertyValue("UnitCode", stringValue);

    element->SetPropertyValue("COMMENT", stringValue);
    element->SetPropertyValue("ALIAS", stringValue);
    element->SetPropertyValue("ADDITIONAL_CODE", stringValue);
    element->SetPropertyValue("UNIT_CLASSIFICATION", stringValue);
    element->SetPropertyValue("UNIT_CODE", stringValue);

//    Utf8String num;
//    num.Sprintf("%.5d", number);
 //   stringValue.SetUtf8CP(num.c_str());
 //   element->SetPropertyValue("Number", stringValue);
  //  element->SetPropertyValue("NUMBER", stringValue);

    BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, "Diameter", AEC_UNIT_MM, 150.0);
    BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, "DIAMETER", AEC_UNIT_MM, 150.0);

 
    return BentleyStatus::SUCCESS;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::PopulateElementProperties(Dgn::DgnElementPtr element)
    {
    ECN::ECClassCP ecClass = element->GetElementClass();
    int i = 0;

    for each (ECN::ECPropertyP prop in ecClass->GetProperties())
        {
        if (nullptr == prop || prop->GetName() == "Roll" || prop->GetName() == "Pitch" || prop->GetName() == "Yaw" || prop->GetName() == "UserLabel" || prop->GetName() == "CodeValue" || prop->GetName() == "JsonProperties")
            continue;

        BeSQLite::BeGuid federationGuid;
        
        federationGuid.Create();

        element->SetFederationGuid(federationGuid);

        Utf8String typeName = prop->GetTypeName();

        if (typeName == "double")
            {
            ECN::ECValue doubleValue;

            if (prop->GetName() == "InsulationThickness" || prop->GetName() == "INSULATION_THICKNESS")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_MM, 50.0);
            else if (prop->GetName() == "DryWeight" || prop->GetName() == "DRY_WEIGHT")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_LBM, 900.0);
            else if (prop->GetName() == "TotalWeight" || prop->GetName() == "TOTAL_WEIGHT")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_LBM, 1500.0);
            else if (prop->GetName() == "OverallHeight" || prop->GetName() == "Height" || prop->GetName() == "HEIGHT" )
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_MM, 2500.0);
            else if (prop->GetName() == "OverallWidth" || prop->GetName() == "Width" || prop->GetName() == "WIDTH")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_MM, 1000.0);
            else if (prop->GetName() == "LowerLimitDesignTemperature" )
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_CELSIUS, 50.0);
            else if (prop->GetName() == "UpperLimitDesignTemperature" || prop->GetName() == "UPPER_LIMIT_DESIGN_TEMPERATURE")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_CELSIUS, 90.0);
            else if (prop->GetName() == "OperatingTemperature" || prop->GetName() == "OPERATING_TEMPERATURE")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_CELSIUS, 80.0);
            else if (prop->GetName() == "AmbientOperatingTemperature" || prop->GetName() == "AMBIENT_OPERATING_TEMPERATURE")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_CELSIUS, 80.0);
            else if (prop->GetName() == "LowerLimitDesignPressure" || prop->GetName() == "LOWER_LIMIT_DESIGN_PRESSURE" )
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_PA, 3.0);
            else if (prop->GetName() == "OperatingPressure" || prop->GetName() == "OPERATING_PRESSURE")
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_PA, 5.0);
            else if (prop->GetName() == "UpperLimitDesignPressure" || prop->GetName() == "UPPER_LIMIT_DESIGN_PRESSURE" )
                BentleyB0200::AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, prop->GetName(), AEC_UNIT_PA, 8.0);

            
            
            else
                {
                doubleValue.SetDouble((i + 1) * 10.0);
                element->SetPropertyValue(prop->GetName().c_str(), doubleValue);
                }
            }
        else if (typeName == "string")
            {
            ECN::ECValue stringValue;

            if (prop->GetName() == "Material" || prop->GetName() == "MATERIAL")
                stringValue.SetWCharCP(L"Carbon Steel");
            else if (prop->GetName() == "Designer" || prop->GetName() == "DESIGNER") 
                stringValue.SetWCharCP(L"John Doe");
            else if (prop->GetName() == "Manufacturer" || prop->GetName() == "MANUFACTURER")
                stringValue.SetWCharCP(L"ACME Equipment");
            else if (prop->GetName() == "ModelNumber" || prop->GetName() == "MODEL_NUMBER")
                stringValue.SetWCharCP(L"E-123456");
            else if (prop->GetName() == "PaintCode" || prop->GetName() == "PAINT_CODE")
                stringValue.SetWCharCP(L"1969 X44");
            else if (prop->GetName() == "InsulationMaterial" || prop->GetName() == "INSULATION_MATERIAL")
                stringValue.SetWCharCP(L"Super-X");
            else if (prop->GetName() == "Insulation" || prop->GetName() == "INSULATION")
                stringValue.SetWCharCP(L"Super-X");
            else if (prop->GetName() == "OrderNumber" || prop->GetName() == "ORDER_NUMBER")
                stringValue.SetWCharCP(L"ON-34567");
            else if (prop->GetName() == "Rating" || prop->GetName() == "RATING")
                stringValue.SetWCharCP(L"10");
            else if (prop->GetName() == "FluidCode" || prop->GetName() == "FLUID_CODE")
                stringValue.SetUtf8CP("C10");
            else if (prop->GetName() == "Fluid" || prop->GetName() == "FLUID")
                stringValue.SetUtf8CP("C10");
            else if (prop->GetName() == "FluidDescription" || prop->GetName() == "FLUID_DESCRIPTION")
                stringValue.SetUtf8CP("C10");
            else
                stringValue.SetWCharCP(L"String Value");

            element->SetPropertyValue(prop->GetName().c_str(), stringValue);
            }
        else if (typeName == "boolean")
            {
            ECN::ECValue boolValue;
            boolValue.SetBoolean(false);
            element->SetPropertyValue(prop->GetName().c_str(), boolValue);
            }
        i++;
        }

    return BentleyStatus::SUCCESS;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::CreateBuilding(BuildingPhysical::BuildingPhysicalModelR physicalModel, BuildingPhysical::BuildingTypeDefinitionModelR typeModel)
    {

	ECN::ECSchemaCP archSchema = physicalModel.GetDgnDb().Schemas().GetSchema(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME);

	ECN::ECClassContainerCR classes = archSchema->GetClasses();

	int i = 0;

	for (int j = 1; j < 5; j++)
		{
		if (i > 100)
			i = 0;

		for each (ECN::ECClassP var in classes)
			{

			if (ECN::ECClassModifier::Abstract == var->GetClassModifier())
				continue;

			ECN::ECBaseClassesList baseClasses = var->GetBaseClasses();

			if (baseClasses.size() == 0)
				continue;

			bool foundPhysicalElement = false;

			for each (ECN::ECClassCP baseClass in baseClasses)
				{
				if (baseClass->GetName() == "PhysicalElement")
					{
					foundPhysicalElement = true;
					break;
					}
				}

			if (!foundPhysicalElement)
				continue;

			Dgn::PhysicalElementPtr buildingElement = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, var->GetName(), physicalModel);

			Dgn::Placement3d placement;

			placement.GetOriginR() = DPoint3d::From((2.0 * i) , .5 * j, 0.0);

			Dgn::DgnDbStatus status = buildingElement->SetPlacement(placement);

            Dgn::DgnCategoryId    categoryId;
            m_host->FindOrCreateCategory(*m_dgnDb, categoryId, buildingElement->GetElementClass()->GetName().c_str());
            m_host->AddCategoryToCategorySelector(categoryId);

			GeometricTools::CreateGeometry(buildingElement, physicalModel, categoryId);

			//ECN::IECInstancePtr instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Classification");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Manufacturer");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Phases");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "IdentityData");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "FireResistance");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AnalyticalProperties");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AcousticalProperties");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIFCOerrides");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIdentification");
			//PopulateInstanceProperties(instance);

			PopulateElementProperties(buildingElement);

			buildingElement->Insert(&status);

			if (Dgn::DgnDbStatus::Success != status)
				return BentleyStatus::ERROR;

			i++;
			}

		ECN::ECSchemaCP dynSchema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(&physicalModel);

		if (nullptr == dynSchema)
			continue;

		ECN::ECClassContainerCR dynClasses = dynSchema->GetClasses();

		for each (ECN::ECClassP var in dynClasses)
			{

			if (ECN::ECClassModifier::Abstract == var->GetClassModifier())
				continue;

			ECN::ECBaseClassesList baseClasses = var->GetBaseClasses();

			if (baseClasses.size() == 0)
				continue;

			bool foundPhysicalElement = false;

			for each (ECN::ECClassCP baseClass in baseClasses)
				{
				if (baseClass->GetName() == "PhysicalElement")
					{
					foundPhysicalElement = true;
					break;
					}
				}

			if (!foundPhysicalElement)
				continue;

			Dgn::PhysicalElementPtr buildingElement = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(dynSchema->GetName(), var->GetName(), physicalModel);

			Dgn::Placement3d placement;

			placement.GetOriginR() = DPoint3d::From(2.0 * i, .5 * j, 0.0);

			Dgn::DgnDbStatus status = buildingElement->SetPlacement(placement);
            Dgn::DgnCategoryId    categoryId;
            m_host->FindOrCreateCategory(*m_dgnDb, categoryId, buildingElement->GetElementClass()->GetName().c_str());
            m_host->AddCategoryToCategorySelector(categoryId);

			GeometricTools::CreateGeometry(buildingElement, physicalModel, categoryId);

			//ECN::IECInstancePtr instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Classification");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Manufacturer");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Phases");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "IdentityData");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "FireResistance");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AnalyticalProperties");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AcousticalProperties");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIFCOerrides");
			//PopulateInstanceProperties(instance);
			//instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIdentification");
			//PopulateInstanceProperties(instance);

			PopulateElementProperties(buildingElement);

			buildingElement->Insert(&status);

			if (Dgn::DgnDbStatus::Success != status)
				return BentleyStatus::ERROR;

			i++;
			}
	
		}

#ifdef NOTNOW
        Dgn::SpatialLocationModelCPtr spatialModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingSpatialLocationModel(BUILDING_MODEL_NAME, physicalModel.GetDgnDb());
        if (spatialModel.IsValid())
            {
            Grids::RadialGridPortion::CreateParams params(&(*spatialModel), 4, 2, PI/8, 10, 20, 20, true);

            Grids::GridAxisMap grid;

            BentleyStatus status = Grids::RadialGridPortion::CreateAndInsert(grid, params);

            Grids::OrthogonalGridPortion::CreateParams params1(&(*spatialModel), 2, 2, 10, 15, 20, 20, DVec3d::From(0, 0, 10), DVec3d::From(10, 0, 0));

            Grids::GridAxisMap grid2;

            status = Grids::OrthogonalGridPortion::CreateAndInsert(grid2, params1);

            }
#endif



//    for (int i = 0; i < 100; i++)
//        {
//
//        ArchitecturalPhysical::DoorPtr door = DoorTools::CreateDoor ( physicalModel, i + 1 );
//        ECN::ECValue doubleValue;
//
//        Dgn::DgnDbStatus status = door->SetPropertyValue("OverallWidth", 250.0);
//        status = door->SetPropertyValue("OverallHeight", 250.0);
//
//        Dgn::Placement3d placement;
//
//        placement.GetOriginR() = DPoint3d::From(5.0 * i, 0.0, 0.0);
//
//        status = door->SetPlacement(placement);
//
//        GeometricTools::CreateDoorGeometry(door, physicalModel);
//
//        ECN::ECValue value;
//
//        ECN::IECInstancePtr instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddManufacturerAspect(physicalModel, door);
//
//        value.SetWCharCP(L"ACME");
//
//        instance->SetValue("Manufacturer", value);
//
//        value.SetWCharCP(L"SN-12345");
//
//        instance->SetValue("ModelNumber", value);
//
//        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect (physicalModel, door);
//
//        value.SetWCharCP(L"10-10-10");
//
//        instance->SetValue("OmniClass", value);
//
//        Dgn::DgnElementCPtr element = door->Insert( &status );
//        
//        if ( Dgn::DgnDbStatus::Success != status )
//            return BentleyStatus::ERROR;
//
////        element->GetCode();
//
////        ArchitecturalPhysical::DoorPtr door1 = ArchitecturalPhysical::ArchitecturalBaseElement::QueryByCodeValue<ArchitecturalPhysical::Door> (physicalModel, "D001");
//
////        Dgn::CodeSpec::CreateCode ( ( physicalModel.GetDgnDb(), BIS_CODESPEC_SpatialCategory, categoryName, nameSpace);
//
//        ArchitecturalPhysical::WindowPtr window = DoorTools::CreateWindow1(physicalModel, i + 1);
//
//        Dgn::Placement3d windowPlacement;
//
//        windowPlacement.GetOriginR() = DPoint3d::From(5.0 * i, 4.0, 4.0);
//
//        status = window->SetPlacement(windowPlacement);
//
//        GeometricTools::CreateWindowGeometry(window, physicalModel);
//
//        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect(physicalModel, window);
//
//        value.SetWCharCP(L"20-10-10");
//
//        instance->SetValue("OmniClass", value);
//
//        window->Insert(&status);
//
//        if (Dgn::DgnDbStatus::Success != status)
//            return BentleyStatus::ERROR;
//
//        ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create("Wall", physicalModel);
//
//        windowPlacement.GetOriginR() = DPoint3d::From(5.0 * i, 8.0, 8.0);
//
//        status = buildingElement->SetPlacement(windowPlacement);
//
//        GeometricTools::CreateWindowGeometry(buildingElement, physicalModel);
//
//        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect(physicalModel, buildingElement);
//
//        value.SetWCharCP(L"40-10-10");
//
//        instance->SetValue("OmniClass", value);
//
//        buildingElement->Insert(&status);
//
//        if (Dgn::DgnDbStatus::Success != status)
//            return BentleyStatus::ERROR;
//
//        buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create("Casework", physicalModel);
//
//        windowPlacement.GetOriginR() = DPoint3d::From(5.0 * i, 12.0, 8.0);
//
//        status = buildingElement->SetPlacement(windowPlacement);
//
//        GeometricTools::CreateWindowGeometry(buildingElement, physicalModel);
//
//        instance = ArchitecturalPhysical::ArchitecturalBaseElement::AddClassificationAspect(physicalModel, buildingElement);
//
//        value.SetWCharCP(L"50-10-10");
//
//        instance->SetValue("OmniClass", value);
//
//        buildingElement->Insert(&status);
//
//        if (Dgn::DgnDbStatus::Success != status)
//            return BentleyStatus::ERROR;
//
//        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

//BentleyStatus BimCreater::SetCodeFromParent(Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR shortCode)
//    {
//
//    Utf8String codeValue;
//
//    if (parentElement.IsValid())
//        codeValue.Sprintf("%s-%s", parentElement->GetCode().GetValueCP(), shortCode.c_str());
//    else
//        codeValue = shortCode;
//
//    Dgn::DgnCode code = BuildingDomain::BuildingDomainUtilities::CreateCode (*functionalElement.GetModel(), codeValue);
//
//    functionalElement.SetCode(code);
//
//    ECN::ECValue value;
//    value.SetUtf8CP(shortCode.c_str());
//    functionalElement.SetPropertyValue(USERLABEL_NAME, value);
//
//    return BentleyStatus::SUCCESS;
//
//    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DgnCode BimCreater::SetCodeForElement(Utf8StringR codeString, Dgn::FunctionalElementR functionalElement, Utf8StringCR codeSpecName)
    {
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(codeSpecName.c_str(), *functionalElement.GetModel(), codeString);
    functionalElement.SetCode(code);
    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DgnCode BimCreater::SetCodeForElement(Utf8StringR codeString, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCR relatedElement, Utf8StringCR codeSpecName)
    {
    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(codeSpecName.c_str(), relatedElement, codeString);
    functionalElement.SetCode(code);
    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Utf8String BimCreater::GetCodeFromParentCode(Utf8StringR shortCode, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR codeSpecName, Utf8StringCR codePattern)
    {
    Utf8String parentCode;
    Utf8String codeString;

    if (parentElement.IsValid())
        {
        parentCode = parentElement->GetCode().GetValueUtf8CP();
        m_host->GetNextCodeValue(codeString, codeSpecName.c_str(), "Plant:Functional", parentCode + "-" + codePattern);
        shortCode = codeString.substr(parentCode.length() + 1, codeString.length() - parentCode.length() - 1);
        }
    else
        {
        m_host->GetNextCodeValue(codeString, codeSpecName.c_str(), "Plant:Functional", codePattern);
        shortCode = codeString;
        }

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    functionalElement.SetPropertyValue(USERLABEL_NAME, value);

    return codeString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DgnCode BimCreater::SetCodeFromParent1(int& number, Utf8StringR shortCode, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR deviceCode)
    {

    //int number;

    static bmap<Utf8String, int> s_numberMap;

    Utf8String lookUp;

    if (parentElement.IsValid())
        lookUp.Sprintf("%s%s", parentElement->GetCode().GetValueUtf8CP(), deviceCode.c_str());
    else
        lookUp = deviceCode;

    auto iterator = s_numberMap.find(lookUp);

    if (iterator != s_numberMap.end())
        {
        number = iterator->second;
        }
    else
        {
        number = 1;
        }

    s_numberMap[lookUp] = number + 1;

    Utf8String codeValue;

    shortCode.Sprintf("%s%0.5d", deviceCode.c_str(), number);

    if (parentElement.IsValid())
        codeValue.Sprintf("%s-%s", parentElement->GetCode().GetValueUtf8CP(), shortCode.c_str());
    else
        codeValue = shortCode;

    Dgn::DgnCode code = BuildingDomain::BuildingDomainUtilities::CreateCode(*functionalElement.GetModel(), codeValue);

    functionalElement.SetCode(code);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    functionalElement.SetPropertyValue(USERLABEL_NAME, value);

    return code;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

int wmain(int argc, WCharP argv[])
    {

    // --workdir=d:\workdir\  --imodelname=samplePlant20 --briefcasedir=d:\briefcases\ --addpid="P01-U01-SU02,S01,A01,BLD01-FL02-RM001"
    // --workdir=d:\workdir\  --imodelname=samplePlant20 --briefcasedir=d:\briefcases\ --overwrite
    BimCreater app;
   // Dgn::DgnPlatformLib::Initialize(app, false);

    //if (BentleyStatus::SUCCESS != app.ParseCommandLine(argc, argv))
    //    return BentleyStatus::ERROR;

    app.ParseCommandLine(argc, argv);

    BentleyApi::PlantBIM::PlantHostP host = BentleyApi::PlantBIM::PlantHost::GetPlantHost();
    BentleyApi::BeFileName assetFolder("D:\\Builds\\SourceTrees\\BuildingDomainDev\\source\\DgnDomains\\Building\\BimCreater\\x64\\Debug\\Assets\\");

    int loginStatus = host->Init(assetFolder, "b564396c-bebd-464a-a839-8d5f82db5a37", "abeesh.basheer@bentley.com", "Tt2~PG[u");

    app.RegisterMyDomains();

    if (app.CreateNewFile())
        {
        Utf8String iModelName = app.iModelName();

        host->DeleteiModelRepo(iModelName.c_str());

        //        Dgn::DgnPlatformLib::Initialize(app, false);
        host->CreateDgnDb(app.GetOutputFileName());
        app.CreateLocal(host->GetDgndb());

        Utf8String iModelGuid;
        host->CreateNewRepository(iModelGuid);

        host->CloseDgnDb();
        }

    //Dgn::DgnDbPtr dgnDb;
    //dgnDb->BriefcaseManager().LockSchemas();
    Utf8String briefcaseFile = app.BriefcaseDir() + app.iModelName() + ".bim";

    BentleyApi::BeFileName briefcaseName(briefcaseFile);
    BentleyApi::BeFileName briefcaseDir(app.BriefcaseDir());

    briefcaseName.BeDeleteFile();

    host->AcquireBriefcase(briefcaseName, app.iModelName().c_str() /*"148c9241-aa05-411a-a3a7-fe57dcc67b3c"*/, briefcaseDir);

    BeSQLite::DbResult openStatus =  host->OpenDgnDb(briefcaseName);

    Dgn::DgnDbPtr db = host->GetDgndb();

    //Utf8String results;
    //Utf8String pattern;
    //pattern = "PMP-####";

    //host->GetNextCodeValue(results, "PlantFunctional-Equipment", "Plant:Functional", pattern);
    //host->GetNextCodeValue(results, "PlantFunctional-Equipment", "Plant:Functional", pattern);
    //host->GetNextCodeValue(results, "PlantFunctional-Equipment", "Plant:Functional", pattern);

    app.SetPlantHost(host);
    app.SetDgnDb(db);

    if (app.GetAddPids())
        {
        app.AddPids();
        }
    else
        {
        app.DoCreate(db, host);
        }

    host->SaveAllChanges();
    host->SyncBriefcase();
    host->RelinquishLocks();
    host->CloseDgnDb();


    return 0;

    }

