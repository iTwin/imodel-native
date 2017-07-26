/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/PublicAPI/ORDBridge/ORDBridge.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ORDBridgeApi.h"

BEGIN_ORDBRIDGE_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct ORDBridge : Dgn::iModelBridgeWithSyncInfoBase
{
    DEFINE_T_SUPER(iModelBridgeBase)

private:
    WString m_rootModelName;

    static void AppendCifSdkToDllSearchPath(BeFileNameCR libraryDir);

protected:
    Dgn::CategorySelectorPtr CreateSpatialCategorySelector(Dgn::DefinitionModelR);
    Dgn::CategorySelectorPtr CreateDrawingCategorySelector(Dgn::DefinitionModelR);
    Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR, Utf8StringCR name);
    Dgn::DisplayStyle2dPtr CreateDisplayStyle2d(Dgn::DefinitionModelR);
    Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR);
    BentleyStatus Create2dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::DgnModelId, Dgn::DisplayStyle2dR);
    BentleyStatus Create3dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR);
    void UpdateProjectExtents(Dgn::SpatialModelR);
    Utf8String ComputeJobSubjectName();

    Dgn::SubjectCPtr CreateAndInsertJobSubject(Dgn::DgnDbR db, Utf8CP jobName);
    Dgn::SubjectCPtr QueryJobSubject(Dgn::DgnDbR db, Utf8CP jobName);

public:
    virtual Dgn::iModelBridge::CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]) override;
    virtual WString _SupplySqlangRelPath() override {return L"sqlang/DgnV8Converter_en-US.sqlang.db3";}
    virtual BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    virtual BentleyStatus _OpenSource() override;
    virtual void _CloseSource(BentleyStatus) override { }
    virtual BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
    virtual Dgn::SubjectCPtr _InitializeJob() override;
    virtual Dgn::SubjectCPtr _FindJob() override;
    virtual void _OnSourceFileDeleted() override;

    ORDBridge() {}
};

extern "C"
{
ORDBRIDGE_EXPORT Dgn::iModelBridge* iModelBridge_getInstance();
};

END_ORDBRIDGE_NAMESPACE