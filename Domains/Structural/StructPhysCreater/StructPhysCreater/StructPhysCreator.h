/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/StructPhysCreator.h $
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
struct StructPhysCreator : Dgn::DgnPlatformLib::Host
    {
    private:
        BeFileName m_outputFileName;
        bool m_overwriteExistingOutputFile = true;

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::WindowsKnownLocationsAdmin(); }
        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("StructPhysCreator"); }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

        Dgn::CategorySelectorPtr CreateCategorySelector(Dgn::DefinitionModelR);
        Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, Dgn::PhysicalModelR modelToSelect);
        Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR model);

    public:
        StructPhysCreator() {}

        BeFileNameCR GetOutputFileName() { return m_outputFileName; }

        BentleyStatus PrintUsage(WCharCP exeName);
        WString GetArgValueW(WCharCP arg);
        BentleyStatus ParseCommandLine(int argc, WCharP argv[]);


        Dgn::DgnDbPtr CreateDgnDb(BeFileNameCR outputFileName);

        BentleyStatus PopulateInstanceProperties(ECN::IECInstancePtr instance);
        BentleyStatus PopulateElementProperties(Dgn::PhysicalElementPtr element);
        BentleyStatus CreateStructure(BuildingPhysical::BuildingPhysicalModelR, BuildingPhysical::BuildingTypeDefinitionModelR);

        Dgn::DgnViewId CreateView(Dgn::DefinitionModelR, Utf8CP, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR displayStyle);

        BentleyStatus DoCreate();
    };
