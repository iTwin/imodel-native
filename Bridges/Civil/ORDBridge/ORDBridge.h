/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams m_params;
    ORDConverter* m_converter;
    bool m_isUnitTesting;

    enum class SchemaImportPhase : int16_t { Base = 1, RoadRail = 2, Dynamic = 3, Extensions = 4, Done = 5 };
    SchemaImportPhase m_schemaImportPhase;

private:
    BentleyStatus InitializeAlignedPartitions(Dgn::SubjectCR jobSubject);

protected:
    Dgn::CategorySelectorPtr CreateSpatialCategorySelector(Dgn::DefinitionModelR);
    Dgn::CategorySelectorPtr CreateDrawingCategorySelector(Dgn::DefinitionModelR);
    Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR, Utf8StringCR name);
    Dgn::DisplayStyle2dPtr CreateDisplayStyle2d(Dgn::DefinitionModelR);
    Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR);
    BentleyStatus Create2dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::DgnModelId, Dgn::DisplayStyle2dR);
    BentleyStatus Create3dView(Dgn::DefinitionModelR, Dgn::CategorySelectorR, Dgn::ModelSelectorR, Dgn::DisplayStyle3dR);

    Dgn::SubjectCPtr QueryJobSubject(Dgn::DgnDbR db, Utf8CP jobName);
    Dgn::iModelBridge::Params& _GetParams() override { return m_params; }

public:
    virtual Dgn::iModelBridge::CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]) override;
    virtual BentleyStatus _ParseCommandLine(int argc, WCharCP argv[]) override;
    virtual WString _SupplySqlangRelPath() override {return L"sqlang/ORDBridge_en-US.sqlang.db3";}
    virtual BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    virtual BentleyStatus _OpenSource() override;
    virtual void _CloseSource(BentleyStatus, ClosePurpose) override { }
    virtual BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
    virtual Dgn::SubjectCPtr _InitializeJob() override;
    virtual Dgn::SubjectCPtr _FindJob() override;
    virtual void _OnDocumentDeleted(Utf8StringCR documentId, Dgn::iModelBridgeSyncInfoFile::ROWID documentSyncId) override;

    virtual BentleyStatus _MakeDefinitionChanges(Dgn::SubjectCR jobSubject) override;
    virtual BentleyStatus _MakeSchemaChanges(bool& hasMoreChanges) override;
    virtual BentleyStatus _OnOpenBim(Dgn::DgnDbR db) override;
    void _OnCloseBim(BentleyStatus, ClosePurpose) override;
    virtual BentleyStatus _DetectDeletedDocuments() override;

    static WCharCP GetRegistrySubKey() { return L"Civil"; }
    static void AppendCifSdkToDllSearchPath(BeFileNameCR libraryDir);
	static void AppendObmSdkToDllSearchPath(BeFileNameCR libraryDir);
    void SetIsUnitTesting(bool isUnitTesting) { m_isUnitTesting = isUnitTesting; }
    bool CheckIfUnitTesting(int argc, WCharCP argv[]);

    ORDBridge() : m_converter(nullptr), m_isUnitTesting(false), m_schemaImportPhase(SchemaImportPhase::Base) {}
    virtual ~ORDBridge()
        {
        if (m_converter != nullptr)
            {
            delete m_converter;
            m_converter = nullptr;
            }
        }
};

END_ORDBRIDGE_NAMESPACE