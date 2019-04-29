/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      
#include <DgnPlatform/DesktopTools\KnownDesktopLocationsAdmin.h>


#ifdef USE_PROTOTYPE

    #define PIPERUN_TYPEPTR         Dgn::FunctionalBreakdownElementPtr
    #define PIPERUN_TYPECPTR        Dgn::FunctionalBreakdownElementCPtr
    #define PIPERUN_TYPE            Dgn::FunctionalBreakdownElement
    #define PIPERUN_CTYPE           Dgn::FunctionalBreakdownElementCPtr
#else
    #define PIPERUN_TYPEPTR         Dgn::FunctionalComponentElementPtr
    #define PIPERUN_TYPECPTR        Dgn::FunctionalComponentElementCPtr
    #define PIPERUN_TYPE            Dgn::FunctionalComponentElement
    #define PIPERUN_CTYPE           Dgn::FunctionalComponentElementCPtr

#endif


                
                           
//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct BimCreater : Dgn::DgnPlatformLib::Host, Dgn::DgnPlatformLib::Host::RepositoryAdmin
    {
    private:
        BeFileName                          m_outputFileName;
        Utf8String                          m_iModelName;
        Utf8String                          m_briefcaseDir;
        bool                                m_overwriteExistingOutputFile = true;
        bool                                m_addPid                      = false;
        BentleyApi::PlantBIM::PlantHostP    m_host;
        Dgn::DgnDbPtr                       m_dgnDb;

        Dgn::DgnElementCPtr                 m_subUnit;
        Utf8String                          m_subUnitShortCode;

        Dgn::DgnElementCPtr                 m_system;
        Utf8String                          m_systemShortCode;

        Dgn::DgnElementCPtr                 m_area;
        Utf8String                          m_areaShortCode;

        Dgn::DgnElementCPtr                 m_room;
        Utf8String                          m_roomShortCode;

        Utf8String                          m_subUnitCode;
        Utf8String                          m_systemCode;
        Utf8String                          m_areaCode;
        Utf8String                          m_roomCode;


        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("BimCreater"); }
        //__PUBLISH_SECTION_START__
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new   Dgn::KnownDesktopLocationsAdmin(); }

        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

        Dgn::CategorySelectorPtr CreateCategorySelector(Dgn::DefinitionModelR);
        Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::DgnModelR modelToSelect, Utf8CP name);
        Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR model);
		Dgn::DisplayStyle2dPtr CreateDisplayStyle2d(Dgn::DefinitionModelR model);

    public:
        BimCreater() {}

        bool CreateNewFile() { return m_overwriteExistingOutputFile; }
        Utf8String    BriefcaseDir() { return m_briefcaseDir; }
        Utf8String    iModelName() { return m_iModelName; }
        BeFileNameCR  GetOutputFileName() { return m_outputFileName; }
        bool          GetAddPids() { return m_addPid;  }
        BentleyStatus PrintUsage(WCharCP exeName);
        WString GetArgValueW(WCharCP arg);
        Utf8String   GetArgValueUtf8(WCharCP arg);
        BentleyApi::PlantBIM::PlantHostP GetPlantHost() { return m_host; }
        void  SetPlantHost(BentleyApi::PlantBIM::PlantHostP host) { m_host = host; }
        Dgn::DgnDbPtr GetDgndb() { return m_dgnDb; }
        void SetDgnDb(Dgn::DgnDbPtr dgnDb) { m_dgnDb = dgnDb; }
        BentleyStatus ParseCommandLine(int argc, WCharP argv[]);
        BentleyStatus CreateBuilding( BuildingPhysical::BuildingPhysicalModelR, BuildingPhysical::BuildingTypeDefinitionModelR);
        BentleyStatus PopulateInstanceProperties(ECN::IECInstancePtr instance);
        BentleyStatus PopulateElementProperties(Dgn::DgnElementPtr element);
        BentleyStatus PopulateEquipmentProperties(Dgn::DgnElementPtr element, Utf8StringCR deviceTypeCode, Utf8StringCR description, int number);
		Dgn::DgnDbPtr OpenDgnDb(BeFileNameCR outputFileName);

        void StartBulkOperation()
            {
            m_dgnDb->BriefcaseManager().StartBulkOperation();
            }

        void EndBulkOperation()
            {
            m_dgnDb->BriefcaseManager().EndBulkOperation();
            }

        Dgn::DgnCode SetCodeFromParent1(int& number, Utf8StringR shortCode, Dgn::FunctionalElementR functionElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR deviceCode);
        Utf8String   GetCodeFromParentCode(Utf8StringR shortCode, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR codeSpecName, Utf8StringCR codePattern);
        Dgn::DgnCode SetCodeForElement(Utf8StringR codeString, Dgn::FunctionalElementR functionalElement, Utf8StringCR codeSpecName);
        Dgn::DgnCode SetCodeForElement(Utf8StringR codeString, Dgn::FunctionalElementR functionalElement, Dgn::DgnElementCR relatedElement, Utf8StringCR codeSpecName);

        Dgn::FunctionalComponentElementPtr CreateNozzle          (Dgn::DgnElementId pipeRunId,  Dgn::DgnElementId equipmentId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement, bool isVirtual = false);

        Dgn::DrawingGraphicPtr             CreatePipeRunGraphics (PIPERUN_TYPECPTR, Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, DPoint2dCP points, bvector<int> count);
        Dgn::DrawingModelPtr               CreatePidDrawings     (Dgn::DocumentListModelR docListModel, Dgn::FunctionalModelR functionModel, Utf8StringCR drawingcode, Dgn::DgnElementCPtr subUnit);
        PIPERUN_TYPEPTR                    CreatePipeRun         (Dgn::DgnElementCPtr pipeline, Dgn::DgnElementId toId, Dgn::DgnElementId fromId, Dgn::FunctionalModelR functionalModel);
        Dgn::FunctionalComponentElementPtr CreateGateValve       (Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateTank            (Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateRoundTank       (Dgn::DgnElementId  subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateVessel          (Dgn::DgnElementId  subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::DrawingGraphicPtr             CreateAnnotation      (Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, Utf8StringCR text, Dgn::Placement2dCR placement);
        Dgn::FunctionalComponentElementPtr CreateReducer         (Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Utf8StringCR reducerLabel);
        Dgn::FunctionalComponentElementPtr CreatePump            (Dgn::DgnElementId  subUnitId,  Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateThreeWayValve   (Dgn::DgnElementId  pipeRunId,  Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);

        ECN::IECInstancePtr                 AddAspect            (Dgn::DgnModelR model, Dgn::DgnElementPtr element, Utf8StringCR schemaName, Utf8StringCR className);
        BentleyStatus                       PopulateInstanceCodes(ECN::IECInstancePtr instance, Utf8String codeSpec, Utf8String codeScope, Utf8String codeValue);

        Dgn::DgnDbPtr  CreateDgnDb(BeFileNameCR, BeSQLite::DbResult* createStatus);
        Dgn::DgnViewId CreateView(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR displayStyle);
		Dgn::DgnViewId CreateView2d(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::DgnModelId baseModelId, Dgn::DisplayStyle2dR displayStyle);
		BentleyStatus  DoCreate(Dgn::DgnDbPtr db, BentleyApi::PlantBIM::PlantHostP host);
        BeSQLite::DbResult  RegisterMyDomains();

        BeSQLite::DbResult  CreateLocal(Dgn::DgnDbPtr db );
		BentleyStatus  DoUpdateSchema(Dgn::DgnDbPtr db); 
        BentleyStatus BimCreater::AddPids();

        static Utf8String    CreateCodeSpecName(Utf8StringR codeScopePrefix, Utf8StringR classScopeName) { Utf8String codeSpecName = codeScopePrefix + "-" + classScopeName; return codeSpecName; }
        static Dgn::DgnCode  CreateCode(Utf8StringR codeScopePrefix, Utf8StringR classScopeName, Dgn::DgnModelCR model, Utf8StringCR codeValue) { return Dgn::CodeSpec::CreateCode(CreateCodeSpecName(codeScopePrefix, classScopeName).c_str(), model, codeValue); }

	};
