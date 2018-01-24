/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridge.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ORDBridgeInternal.h"

BEGIN_ORDBRIDGE_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct ORDBridge : Dgn::iModelBridgeWithSyncInfoBase
{
    DEFINE_T_SUPER(Dgn::iModelBridgeWithSyncInfoBase)

private:
    static void AppendCifSdkToDllSearchPath(BeFileNameCR libraryDir);
    BentleyStatus CreateSyncInfoIfNecessary();

    Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams m_params;
    std::unique_ptr<ORDConverter> m_converter;

protected:
    Dgn::CategorySelectorPtr CreateSpatialCategorySelector(Dgn::DefinitionModelR);
    Dgn::CategorySelectorPtr CreateDrawingCategorySelector(Dgn::DefinitionModelR);
    Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR, Utf8StringCR name);
    Dgn::DisplayStyle2dPtr CreateDisplayStyle2d(Dgn::DefinitionModelR);
    Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR);
    BentleyStatus Create2dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::DgnModelId, Dgn::DisplayStyle2dR);
    BentleyStatus Create3dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR);
    Utf8String ComputeJobSubjectName(Utf8StringCR docId);
    Utf8String ComputeJobSubjectName();

    Dgn::SubjectCPtr CreateAndInsertJobSubject(Dgn::DgnDbR db, Utf8CP jobName);
    Dgn::SubjectCPtr QueryJobSubject(Dgn::DgnDbR db, Utf8CP jobName);
    Dgn::iModelBridge::Params& _GetParams() override { return m_params; }

public:
    virtual Dgn::iModelBridge::CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]) override;
    virtual WString _SupplySqlangRelPath() override {return L"sqlang/DgnV8Converter_en-US.sqlang.db3";}
    virtual BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    virtual BentleyStatus _OpenSource() override;
    virtual void _CloseSource(BentleyStatus) override { }
    virtual BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
    virtual Dgn::SubjectCPtr _InitializeJob() override;
    virtual Dgn::SubjectCPtr _FindJob() override;
    virtual void _OnDocumentDeleted(Utf8StringCR documentId, Dgn::iModelBridgeSyncInfoFile::ROWID documentSyncId) override;
    virtual void _DeleteSyncInfo() override;

    virtual BentleyStatus _MakeSchemaChanges() override;
    virtual BentleyStatus _OnOpenBim(Dgn::DgnDbR db) override;
    void _OnCloseBim(BentleyStatus) override;
    virtual BentleyStatus _DetectDeletedDocuments() override;

    static WCharCP GetRegistrySubKey() { return L"OpenRoads Designer Bridge"; }

    ORDBridge() {}
};

END_ORDBRIDGE_NAMESPACE