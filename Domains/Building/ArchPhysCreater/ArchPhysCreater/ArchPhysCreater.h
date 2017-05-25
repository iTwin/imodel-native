/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/ArchPhysCreater.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "stdafx.h"      
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>

                
                
                           
//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct ArchPhysCreator : Dgn::DgnPlatformLib::Host
    {
    private:
        BeFileName m_outputFileName;
        bool m_overwriteExistingOutputFile = true;

        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ArchPhysCreator"); }
        //__PUBLISH_SECTION_START__
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new   Dgn::WindowsKnownLocationsAdmin(); }

        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

        Dgn::CategorySelectorPtr CreateCategorySelector(Dgn::DefinitionModelR);
        Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::PhysicalModelR modelToSelect);
        Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR model);

    public:
        ArchPhysCreator() {}

        BeFileNameCR GetOutputFileName() { return m_outputFileName; }
        BentleyStatus PrintUsage(WCharCP exeName);
        WString GetArgValueW(WCharCP arg);
        BentleyStatus ParseCommandLine(int argc, WCharP argv[]);
        BentleyStatus CreateBuilding( BuildingPhysical::BuildingPhysicalModelR, BuildingPhysical::BuildingTypeDefinitionModelR);
        BentleyStatus PopulateInstanceProperties(ECN::IECInstancePtr instance);
        BentleyStatus PopulateElementProperties(Dgn::PhysicalElementPtr element);




        Dgn::DgnDbPtr CreateDgnDb(BeFileNameCR);
//        ToyTileGroupModelPtr CreateGroupModel(Dgn::DgnDbR);
//        BentleyStatus CreateGroupElements(ToyTileGroupModelR);
//        ToyTilePhysicalModelPtr CreatePhysicalModel(Dgn::DgnDbR);
        //  BuildingTypeDefinitionModelPtr CreateBuildingTypeDefinitionModel(Dgn::DgnDbR db);
        Dgn::DgnViewId CreateView(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR displayStyle);
        BentleyStatus DoCreate();
    };
