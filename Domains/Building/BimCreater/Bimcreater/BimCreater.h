/*--------------------------------------------------------------------------------------+
|
|     $Source: BimCreater/Bimcreater/BimCreater.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "stdafx.h"      
#include <DgnPlatform/DesktopTools\KnownDesktopLocationsAdmin.h>

                
                
                           
//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct BimCreater : Dgn::DgnPlatformLib::Host
    {
    private:
        BeFileName m_outputFileName;
        bool m_overwriteExistingOutputFile = true;

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

        BeFileNameCR GetOutputFileName() { return m_outputFileName; }
        BentleyStatus PrintUsage(WCharCP exeName);
        WString GetArgValueW(WCharCP arg);
        BentleyStatus ParseCommandLine(int argc, WCharP argv[]);
        BentleyStatus CreateBuilding( BuildingPhysical::BuildingPhysicalModelR, BuildingPhysical::BuildingTypeDefinitionModelR);
		Dgn::DrawingModelPtr CreatePidDrawings(Dgn::DocumentListModelR docListModel, Dgn::FunctionalModelR functionModel, Utf8StringCR drawingcode, Dgn::DgnElementCPtr subUnit);
        BentleyStatus PopulateInstanceProperties(ECN::IECInstancePtr instance);
        BentleyStatus PopulateElementProperties(Dgn::DgnElementPtr element);
		Dgn::DgnDbPtr OpenDgnDb(BeFileNameCR outputFileName);

       // BentleyStatus SetCodeFromParent(Dgn::FunctionalElementR functionElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR shortCode);
        Dgn::DgnCode SetCodeFromParent1(Utf8StringR shortCode, Dgn::FunctionalElementR functionElement, Dgn::DgnElementCPtr parentElement, Utf8StringCR deviceCode);

        Dgn::FunctionalComponentElementPtr CreateNozzle          (Dgn::DgnElementId pipeRunId,  Dgn::DgnElementId equipmentId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement, bool isVirtual = false);
        Dgn::DrawingGraphicPtr             CreatePipeRunGraphics (Dgn::FunctionalBreakdownElementCPtr, Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, DPoint2dCP points, bvector<int> count);
        Dgn::FunctionalBreakdownElementPtr CreatePipeRun         (Dgn::DgnElementCPtr pipeline, Dgn::DgnElementId toId, Dgn::DgnElementId fromId, Dgn::FunctionalModelR functionalModel);
        Dgn::FunctionalComponentElementPtr CreateGateValve       (Dgn::DgnElementId pipeRunId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement);
        Dgn::FunctionalComponentElementPtr CreateTank            (Dgn::DgnElementId subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateRoundTank       (Dgn::DgnElementId  subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateVessel          (Dgn::DgnElementId  subUnitId, Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::DrawingGraphicPtr             CreateAnnotation      (Dgn::DgnCategoryId categoryId, Dgn::DrawingModelR drawingModel, Utf8StringCR text, Dgn::Placement2dCR placement);
        Dgn::FunctionalComponentElementPtr CreateReducer         (Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Utf8StringCR reducerLabel);
        Dgn::FunctionalComponentElementPtr CreatePump            (Dgn::DgnElementId  subUnitId,  Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionalModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement, Dgn::DgnElementCPtr parentElement);
        Dgn::FunctionalComponentElementPtr CreateThreeWayValve   (Dgn::DgnElementId  pipeRunId,  Dgn::DgnCategoryId categoryId, Dgn::FunctionalModelR functionModel, Dgn::DrawingModelR drawingModel, Dgn::Placement2dCR placement);


        Dgn::DgnDbPtr CreateDgnDb(BeFileNameCR);
//        ToyTileGroupModelPtr CreateGroupModel(Dgn::DgnDbR);
//        BentleyStatus CreateGroupElements(ToyTileGroupModelR);
//        ToyTilePhysicalModelPtr CreatePhysicalModel(Dgn::DgnDbR);
        //  BuildingTypeDefinitionModelPtr CreateBuildingTypeDefinitionModel(Dgn::DgnDbR db);
        Dgn::DgnViewId CreateView(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR displayStyle);
		Dgn::DgnViewId CreateView2d(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::DgnModelId baseModelId, Dgn::DisplayStyle2dR displayStyle);
		BentleyStatus DoCreate();
		BentleyStatus DoUpdateSchema(Dgn::DgnDbPtr db); 
	};
