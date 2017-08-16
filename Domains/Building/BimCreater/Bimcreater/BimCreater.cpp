/*--------------------------------------------------------------------------------------+
|
|     $Source: BimCreater/Bimcreater/BimCreater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "BimCreater.h"


#define BUILDING_MODEL_NAME "SamplePlantModel"
#define USERLABEL_NAME  "UserLabel"
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
BentleyStatus BimCreater::ParseCommandLine(int argc, WCharP argv[])
    {
   // if (argc < 2)
   //     return PrintUsage(argv[0]);

    WString outputFileNameArg, currentDirectory;

    // Setup defaults for arguments so you can run without any arguments. 

    outputFileNameArg = L"SamplePlantBim.Bim"; 
    m_overwriteExistingOutputFile = true;


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

    m_outputFileName = BeFileName(outputFileNameArg.c_str());

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
    auto categorySelector = new Dgn::CategorySelector(model, "Default");
    categorySelector->AddCategory(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingDrawingCategoryId(model.GetDgnDb(), "PidLine"));
    return categorySelector;
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
Dgn::DgnDbPtr BimCreater::CreateDgnDb(BeFileNameCR outputFileName)
    {
    // Initialize parameters needed to create a DgnDb
    Dgn::CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting(m_overwriteExistingOutputFile);
    createProjectParams.SetRootSubjectName("Sample Plant BIM");
    createProjectParams.SetRootSubjectDescription("Sample Plant created by BimCreater app");

    // Create the DgnDb file. The BisCore domain schema is also imported. Note that a seed file is not required.
    BeSQLite::DbResult createStatus;
    Dgn::DgnDbPtr db = Dgn::DgnDb::CreateDgnDb(&createStatus, outputFileName, createProjectParams);
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
	Dgn::DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, Dgn::SchemaUpgradeOptions::AllowedDomainUpgrades::CompatibleOnly );

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

Dgn::FunctionalBreakdownElementPtr BimCreater::CreatePipeRun(Dgn::DgnElementCPtr pipeline, Dgn::DgnElementId toId, Dgn::DgnElementId fromId, Dgn::FunctionalModelR functionalModel)
    {

    // Create the functional Instance
    Dgn::FunctionalBreakdownElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_PipeRun, functionalModel);

    Utf8String shortCode;

    SetCodeFromParent1(shortCode, *functionalElement, pipeline, "PS");
    PopulateElementProperties(functionalElement);

    ECN::ECClassCP relClass;
    ECN::ECRelationshipClassCP  relationShipClass;
    BeSQLite::EC::ECInstanceKey rkey;

    // Relate this PipeRun to the input Pipeline 
    if (pipeline.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipelineOwnsPipeRuns);
        functionalElement->SetParentId(pipeline->GetElementId(), relClass->GetId());
        }

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

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalBreakdownElement>(functionalModel, fe->GetElementId());

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

    GeometricTools::CreateAnnotationTextGeometry(*annotationGraphics, categoryId, text);

    Dgn::DgnElementCPtr ge = annotationGraphics->Insert();


    annotationGraphics = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::DrawingGraphic>(drawingModel, ge->GetElementId());

    return annotationGraphics;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DrawingGraphicPtr BimCreater::CreatePipeRunGraphics(Dgn::FunctionalBreakdownElementCPtr pipeRun, Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, DPoint2dCP points, bvector<int> count)
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

    for (int i = 0; i < count.size(); i++)
        {
        pipeRunGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

        if (!pipeRunGraphic.IsValid())
            return nullptr;

        int startIndex = 0;
        if (i > 0) startIndex = count[i-1];
        GeometricTools::CreatePidLineGeometry(*pipeRunGraphic, drawingModel, &points[startIndex], count[i]);
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

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_Nozzle, functionalModel);

    Utf8String shortCode;

    SetCodeFromParent1(shortCode, *functionalElement, parentElement, "N");

    PopulateElementProperties(functionalElement);

    //Create the graphics element

    Dgn::DrawingGraphicPtr nozzleGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!nozzleGraphic.IsValid())
        return nullptr;


    nozzleGraphic->SetPlacement(placement);

    if(!isVirtual)
        GeometricTools::CreatePidNozzleGeometry(*nozzleGraphic, categoryId);
    else
        GeometricTools::CreatePidVirtualNozzleGeometry(*nozzleGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    nozzleGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = nozzleGraphic->Insert();


    ECN::ECClassCP relClass;
    if (equipmentId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_EquipmentOwnsNozzles);
        functionalElement->SetParentId(equipmentId, relClass->GetId());
        }

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    relClass                      = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunConnectsToPipingComponents);
        ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
        BeSQLite::EC::ECInstanceKey rkey;

        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, pipeRunId, fe->GetElementId());
        }

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel,fe->GetElementId());

    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateTank( Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_EQUIPMENT_FUNCTIONAL, "Tank", functionalModel);
    Utf8String shortCode;
    SetCodeFromParent1( shortCode, *functionalElement, parentElement, "T");
    PopulateElementProperties(functionalElement);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    Dgn::DrawingGraphicPtr tankGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!tankGraphic.IsValid())
        return nullptr;

    tankGraphic->SetPlacement(placement);
    GeometricTools::CreatePidTankGeometry(*tankGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    tankGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = tankGraphic->Insert();

    ECN::ECClassCP relClass;
    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }


    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateRoundTank(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_EQUIPMENT_FUNCTIONAL, PEF_CLASS_Drum, functionalModel);

    Utf8String shortCode;
    SetCodeFromParent1(shortCode, *functionalElement, parentElement, "T");
    PopulateElementProperties(functionalElement);
    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the Graphic Element

    Dgn::DrawingGraphicPtr tankGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!tankGraphic.IsValid())
        return nullptr;

    tankGraphic->SetPlacement(placement);

    GeometricTools::CreatePidRoundTankGeometry(*tankGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    tankGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = tankGraphic->Insert();


    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateVessel(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Tank Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_EQUIPMENT_FUNCTIONAL, PEF_CLASS_Vessel, functionalModel);

    Utf8String shortCode;

    SetCodeFromParent1(shortCode, *functionalElement, parentElement, "V");
    PopulateElementProperties(functionalElement);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the Vessel

    Dgn::DrawingGraphicPtr vesselGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!vesselGraphic.IsValid())
        return nullptr;

    vesselGraphic->SetPlacement(placement);

    GeometricTools::CreatePidVesselGeometry(*vesselGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    vesselGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = vesselGraphic->Insert();


    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreatePump(Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement)
    {

    // Create Pump Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_EQUIPMENT_FUNCTIONAL, PEF_CLASS_CentrifugalPump, functionalModel);

    Utf8String shortCode;
    SetCodeFromParent1(shortCode, *functionalElement, parentElement, "PMP");
    PopulateElementProperties(functionalElement);

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the Pump

    Dgn::DrawingGraphicPtr pumpGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!pumpGraphic.IsValid())
        return nullptr;

    pumpGraphic->SetPlacement(placement);
    GeometricTools::CreatePidPumpGeometry(*pumpGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    pumpGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = pumpGraphic->Insert();


    ECN::ECClassCP relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    if (subUnitId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
        relationShipClass = relClass->GetRelationshipClassCP();
        functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnitId, functionalElement->GetElementId());
        }


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

    GeometricTools::CreatePidReducerGeometry(*reducerGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(reducerLabel.c_str());
    reducerGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = reducerGraphic->Insert();

    // Create Reducer Functional Component

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_ConcentricPipeReducer, functionalModel);

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

Dgn::FunctionalComponentElementPtr BimCreater::CreateGateValve(Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement)
    {

    // Create the functional component for the Gate Valve

    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_GateValve, functionalModel);

    Utf8String shortCode;
    SetCodeFromParent1(shortCode, *functionalElement, nullptr, "HV");

    PopulateElementProperties(functionalElement);

    ECN::ECClassCP relClass;
    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunOwnsFunctionalPipingComponents);
        functionalElement->SetParentId(pipeRunId, relClass->GetId());
        }

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    // Create the graphics for the valve

    Dgn::DrawingGraphicPtr valveGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!valveGraphic.IsValid())
        return nullptr;

    valveGraphic->SetPlacement(placement);

    GeometricTools::CreatePidValveGeometry(*valveGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    valveGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = valveGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    return functionalElement;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::FunctionalComponentElementPtr BimCreater::CreateThreeWayValve(Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement)
    {

    // Create the functional component for the three way valve
    Dgn::FunctionalComponentElementPtr functionalElement = BuildingDomain::BuildingDomainUtilities::CreateFunctionalComponentElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_ThreeWayValve, functionalModel);

    Utf8String shortCode;
    SetCodeFromParent1(shortCode, *functionalElement, nullptr, "HV");

    ECN::ECClassCP relClass;
    if (pipeRunId.IsValid())
        {
        relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PIPING_FUNCTIONAL, PPF_REL_PipeRunOwnsFunctionalPipingComponents);
        functionalElement->SetParentId(pipeRunId, relClass->GetId());
        }

    Dgn::DgnElementCPtr fe = functionalElement->Insert();

    Dgn::DrawingGraphicPtr valveGraphic = BuildingDomain::BuildingDomainUtilities::CreateDrawingGraphic(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic, drawingModel, categoryId);

    if (!valveGraphic.IsValid())
        return nullptr;

    valveGraphic->SetPlacement(placement);

    GeometricTools::CreatePid3WayValveGeometry(*valveGraphic, categoryId);

    ECN::ECValue value;
    value.SetUtf8CP(shortCode.c_str());
    valveGraphic->SetPropertyValue(USERLABEL_NAME, value);

    Dgn::DgnElementCPtr ge = valveGraphic->Insert();


    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass("Functional", "DrawingGraphicRepresentsFunctionalElement");
    ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
    BeSQLite::EC::ECInstanceKey rkey;

    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, ge->GetElementId(), fe->GetElementId());

    functionalElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::FunctionalComponentElement>(functionalModel, fe->GetElementId());

    return functionalElement;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

Dgn::DrawingModelPtr BimCreater::CreatePidDrawings(Dgn::DocumentListModelR docListModel, Dgn::FunctionalModelR functionalModel, Utf8StringCR drawingCode, Dgn::DgnElementCPtr subUnit )
	{

	// Create a drawing model

	Dgn::DrawingModelPtr drawingModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingDrawingModel(drawingCode, docListModel.GetDgnDb(), docListModel);

	Dgn::DgnDbR db = drawingModel->GetDgnDb();
	Dgn::DgnModelId modelId = drawingModel->GetModelId();

    Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingDrawingCategoryId(db, "PidLine");

    ECN::ECClassCP              relClass;
    ECN::ECRelationshipClassCP  relationShipClass;
    BeSQLite::EC::ECInstanceKey rkey;

    // Add all the equipment to the PID

    Dgn::Placement2d       placement;
    Dgn::DrawingGraphicPtr annotation;

    // **** Add the Tank ****

    placement.GetOriginR() = DPoint2d::From(-91.256, 6.25);
    Dgn::FunctionalComponentElementCPtr tank = CreateTank(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);
    placement.GetOriginR() = DPoint2d::From(-75, 19.375);
    annotation = CreateAnnotation(categoryId, *drawingModel, tank->GetPropertyValueString(USERLABEL_NAME), placement);

    // ******* Add the Tank Nozzle *****

    placement.GetOriginR() = DPoint2d::From(-60.006, 19.375);
    Dgn::DgnElementId id;
    Dgn::FunctionalComponentElementPtr tankNozzle = CreateNozzle(id, tank->GetElementId(), categoryId, functionalModel, *drawingModel, placement, tank);

    // ******* Add the Pump ********

    placement.GetOriginR() = DPoint2d::From(0, 0);
    Dgn::FunctionalComponentElementCPtr pump = CreatePump(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);
    placement.GetOriginR() = DPoint2d::From(0.0, -5);
    annotation = CreateAnnotation(categoryId, *drawingModel, pump->GetPropertyValueString(USERLABEL_NAME), placement);

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
    placement.GetOriginR() = DPoint2d::From(57.494, 26.25);
    annotation = CreateAnnotation(categoryId, *drawingModel, roundTank->GetPropertyValueString(USERLABEL_NAME), placement);

    // ****** Add Nozzle to Round Tank ******

    placement.GetOriginR() = DPoint2d::From(47.494, 28.75);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(180.0);
    Dgn::FunctionalComponentElementPtr roundTankNozzle = CreateNozzle(id, roundTank->GetElementId(), categoryId, functionalModel, *drawingModel, placement, roundTank);

    // **** Add the Vessel ****

    placement.GetOriginR() = DPoint2d::From(51.244, -20.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0);
    Dgn::FunctionalComponentElementCPtr vessel = CreateVessel(subUnit->GetElementId(), categoryId, functionalModel, *drawingModel, placement, subUnit);
    placement.GetOriginR() = DPoint2d::From(58.744, -20.0);
    annotation = CreateAnnotation(categoryId, *drawingModel, vessel->GetPropertyValueString(USERLABEL_NAME), placement);

    // ****** Add Nozzle to Vessle ******

    placement.GetOriginR() = DPoint2d::From(56.244, -15.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(90.0);
    Dgn::FunctionalComponentElementPtr vesselNozzle = CreateNozzle(id, vessel->GetElementId(), categoryId, functionalModel, *drawingModel, placement, vessel);

    // Create the pipeline breakdown element. 

    Dgn::FunctionalBreakdownElementPtr pipeline = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PIPING_FUNCTIONAL, PPF_Class_Pipeline, functionalModel);
    Utf8String shortCode;

    SetCodeFromParent1(shortCode, *pipeline, subUnit, "L");

    Dgn::DgnElementCPtr pl = pipeline->Insert();

    relClass = functionalModel.GetDgnDb().GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
    relationShipClass = relClass->GetRelationshipClassCP();
    functionalModel.GetDgnDb().InsertLinkTableRelationship(rkey, *relationShipClass, subUnit->GetElementId(), pl->GetElementId());


    // **** Add the Reducer ****

    placement.GetOriginR() = DPoint2d::From(-30.756, 0.0);
    placement.GetAngleR() = AngleInDegrees::FromDegrees(0.0);
    Dgn::FunctionalComponentElementPtr reducer = CreateReducer(categoryId, functionalModel, *drawingModel, placement, "4x2");
    placement.GetOriginR() = DPoint2d::From(-30.156, -2.0);
    annotation = CreateAnnotation(categoryId, *drawingModel, "4x2", placement);


    // PipeRun from Tank to the reducer

    Dgn::FunctionalBreakdownElementPtr pipeRun1 = CreatePipeRun((Dgn::FunctionalBreakdownElementCPtr)pipeline, reducer->GetElementId(), tankNozzle->GetElementId(), functionalModel);

    // PipeRun from Reducer to the Pump

    Dgn::FunctionalBreakdownElementPtr pipeRun2 = CreatePipeRun((Dgn::FunctionalBreakdownElementCPtr)pipeline, pumpNozzle1->GetElementId(), reducer->GetElementId(), functionalModel);

    // PipeRun from Round Tank to the Vessel

    Dgn::FunctionalBreakdownElementPtr pipeRun4 = CreatePipeRun((Dgn::FunctionalBreakdownElementCPtr)pipeline, vesselNozzle->GetElementId(), roundTankNozzle->GetElementId(), functionalModel);

    // **** Add 3 way Valve ****

    placement.GetOriginR() = DPoint2d::From(28.494, 2.5);
    Dgn::FunctionalComponentElementPtr threeWayValve = CreateThreeWayValve(pipeRun4->GetElementId(), categoryId, functionalModel, *drawingModel, placement);
    placement.GetOriginR() = DPoint2d::From(33.5, 2.5);
    annotation = CreateAnnotation(categoryId, *drawingModel, threeWayValve->GetCode().GetValueUtf8CP(), placement);


    // PipeRun from Pump to the 3 way valves

    Dgn::FunctionalBreakdownElementPtr pipeRun3 = CreatePipeRun((Dgn::FunctionalBreakdownElementCPtr)pipeline, threeWayValve->GetElementId(), pumpNozzle2->GetElementId(), functionalModel);

    // ******** Add the Gate Valve ******

    placement.GetOriginR() = DPoint2d::From(-48.361, 19.375);
    Dgn::FunctionalComponentElementPtr gateValve = CreateGateValve(pipeRun1->GetElementId(), categoryId, functionalModel, *drawingModel, placement);
    placement.GetOriginR() = DPoint2d::From(-46.861, 17.375);
    annotation = CreateAnnotation(categoryId, *drawingModel, gateValve->GetCode().GetValueUtf8CP(), placement);

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

	return drawingModel;

	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------

BentleyStatus BimCreater::DoCreate()
    {

    if (BentleyStatus::SUCCESS != BuildingDomain::BuildingDomainUtilities::RegisterDomainHandlers())
        return BentleyStatus::ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantBIM::ProcessEquipmentFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BentleyStatus::ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantBIM::PlantBreakdownFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BentleyStatus::ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantBIM::ProcessPipingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BentleyStatus::ERROR;

    if (BentleyStatus::SUCCESS != Dgn::DgnDomains::RegisterDomain(PlantBIM::ProcessPipingFunctionalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No))
        return BentleyStatus::ERROR;


    Dgn::DgnDbPtr db = CreateDgnDb(GetOutputFileName());
    if (!db.IsValid())
        return BentleyStatus::ERROR;

	// Create the models in the BIM file

	BentleyStatus status = BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(BUILDING_MODEL_NAME, *db);

	if (BentleyStatus::SUCCESS != status)
		return BentleyStatus::ERROR;

	BuildingPhysical::BuildingPhysicalModelPtr       physicalModel       = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel         (BUILDING_MODEL_NAME, *db);
	BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel  (BUILDING_MODEL_NAME, *db);
	Dgn::DocumentListModelPtr                        docListModel        = BuildingDomain::BuildingDomainUtilities::CreateBuildingDocumentListModel (BUILDING_MODEL_NAME, *db);
	Dgn::FunctionalModelPtr                          functionalModel     = BuildingDomain::BuildingDomainUtilities::CreateBuildingFunctionalModel   (BUILDING_MODEL_NAME, *db);
	Dgn::DefinitionModelR                            dictionary          = db->GetDictionaryModel();

	Dgn::CategorySelectorPtr categorySelector = CreateCategorySelector(dictionary);
	Dgn::DisplayStyle2dPtr displayStyle1 = CreateDisplayStyle2d(dictionary);

    Utf8String shortCode;

    // This is where we create add the information to the BIM file. You can control the number of Units, SubUnits and P&ID. 

    int numberOfUnits = 5;
    int numberOfSubunits = 5;
    int numberOfPids = 1000;

    for (int unitNum = 0; unitNum < numberOfUnits; unitNum++)
        {

        Dgn::FunctionalBreakdownElementPtr unit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Class_Unit, *functionalModel);

        SetCodeFromParent1(shortCode, *unit, nullptr, "U");
        Dgn::DgnElementCPtr un = unit->Insert();

        for (int subUnitNum = 0; subUnitNum < numberOfSubunits; subUnitNum++)
            {

            Dgn::FunctionalBreakdownElementPtr subUnit = BuildingDomain::BuildingDomainUtilities::CreateFunctionalBreakdownElement(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Class_SubUnit, *functionalModel);
            SetCodeFromParent1(shortCode, *subUnit, un, "SU");

            Dgn::DgnElementCPtr subUn = subUnit->Insert();

            ECN::ECClassCP relClass = db->GetClassLocater().LocateClass(DOMAIN_PLANT_BREAKDOWN_FUNCTIONAL, PBF_Rel_FunctionalBreakdownGroupsFunctionalElements);
            ECN::ECRelationshipClassCP relationShipClass = relClass->GetRelationshipClassCP();
            BeSQLite::EC::ECInstanceKey rkey;
            db->InsertLinkTableRelationship(rkey, *relationShipClass, un->GetElementId(), subUn->GetElementId());

            static int pidNum = 1;
            for (int j = 1; j < numberOfPids; j++)
                {
                Utf8String pidName;
                pidName.Sprintf("PID-%0.5d", pidNum);
                pidNum++;
                Dgn::DrawingModelPtr drawingModel = CreatePidDrawings(*docListModel, *functionalModel, pidName, subUn);

                Dgn::DgnViewId viewId = CreateView2d(dictionary, pidName.c_str(), *categorySelector, drawingModel->GetModelId(), *displayStyle1);

                if (!viewId.IsValid())
                    return BentleyStatus::ERROR;
                }
            }
        }


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

BentleyStatus BimCreater::PopulateElementProperties(Dgn::DgnElementPtr element)
    {
    ECN::ECClassCP ecClass = element->GetElementClass();
    int i = 0;

    for each (ECN::ECPropertyP prop in ecClass->GetProperties())
        {
        if (nullptr == prop || prop->GetName() == "Roll" || prop->GetName() == "Pitch" || prop->GetName() == "Yaw" || prop->GetName() == "UserLabel" || prop->GetName() == "CodeValue" || prop->GetName() == "JsonProperties")
            continue;

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

			GeometricTools::CreateGeometry(buildingElement, physicalModel);

			ECN::IECInstancePtr instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Classification");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Manufacturer");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Phases");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "IdentityData");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "FireResistance");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AnalyticalProperties");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AcousticalProperties");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIFCOerrides");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIdentification");
			PopulateInstanceProperties(instance);

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

			GeometricTools::CreateGeometry(buildingElement, physicalModel);

			ECN::IECInstancePtr instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Classification");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Manufacturer");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "Phases");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "IdentityData");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "FireResistance");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AnalyticalProperties");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "AcousticalProperties");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIFCOerrides");
			PopulateInstanceProperties(instance);
			instance = BuildingCommon::BuildingCommonDomain::AddAspect(physicalModel, buildingElement, "ABDIdentification");
			PopulateInstanceProperties(instance);

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

Dgn::DgnCode BimCreater::SetCodeFromParent1(Utf8StringR shortCode, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR deviceCode)
    {

    int number;

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

    shortCode.Sprintf("%s%0.3d", deviceCode.c_str(), number);

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

    BimCreater app;
    Dgn::DgnPlatformLib::Initialize(app, false);

    //if (BentleyStatus::SUCCESS != app.ParseCommandLine(argc, argv))
    //    return BentleyStatus::ERROR;

    app.ParseCommandLine(argc, argv);

    return app.DoCreate();

    }

