/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dBridge.h"
#include <DgnPlatform/DgnPlatform.h>
#include <iModelBridge/iModelBridge.h>
#include <Dwg/DwgDb/DwgDbHost.h>

USING_NAMESPACE_C3D

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporterP C3dBridge::_CreateDwgImporter ()
    {
    return  new C3dImporter (T_Super::GetImportOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus C3dBridge::_Initialize (int argc, WCharCP argv[])
    {
    auto status = T_Super::_Initialize (argc, argv);
    if (status == BentleyStatus::BSISUCCESS)
        C3dImporter::Initialize ();
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr C3dBridge::_InitializeJob ()
    {
    auto c3dImporter = dynamic_cast<C3dImporterP>(T_Super::GetImporter());
    if (c3dImporter == nullptr)
        return  nullptr;

    auto jobSubject = T_Super::_InitializeJob ();
    if (!jobSubject.IsValid())
        return  nullptr;

    c3dImporter->OnBaseBridgeJobFound (jobSubject->GetElementId());
        
    return  jobSubject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr C3dBridge::_FindJob ()
    {
    auto jobSubject = T_Super::_FindJob ();
    if (jobSubject.IsValid())
        m_importer->OnBaseBridgeJobFound (jobSubject->GetElementId());

    return  jobSubject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/19
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dBridge::_SetClientInfo ()
    {
    // Set CONNECTION Client information to track product usage
    static constexpr char s_bridgeName[] = "iModelBridgeService-C3D";
    static constexpr char s_bridgeGuid[] = "F5EF4029-35E5-41CA-8F42-E3FEFEEB752A";
    static constexpr char s_bridgePrgId[] = "9999";

    BeVersion bridgeVersion;
#ifdef REL_V
    // prg.h exists, use its build version macros
    bridgeVersion.FromString(REL_V "." MAJ_V "." MIN_V "." SUBMIN_V);
#else
    // prg.h does not exist, use base DwgBridge version number
    Utf8String  verstr;
    if (DwgHelper::GetImporterModuleVersion(verstr) == BSISUCCESS)
        bridgeVersion.FromString(verstr.c_str());
    else
        bridgeVersion = BeVersion(1, 0, 0, 999);
#endif
        
    auto& params = this->_GetParams();
    auto sampleInfo = WebServices::ClientInfo::Create(s_bridgeName, bridgeVersion, s_bridgeGuid, s_bridgePrgId, params.GetDefaultHeaderProvider());

    params.SetClientInfo (sampleInfo);
    }


BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct AffinityHost : public IDwgDbHost
{
public:
    AffinityHost () {}
    ~AffinityHost ();
    static AffinityHost&    GetHost ();
}; // AffinityHost


static AffinityHost*    s_affinityHostInstance = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
AffinityHost::~AffinityHost ()
    {
    AffinityHost::TerminateToolkit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
AffinityHost&  AffinityHost::GetHost ()
    {
    if (nullptr == s_affinityHostInstance)
        {
        s_affinityHostInstance = new AffinityHost ();
        AffinityHost::InitializeToolkit (*s_affinityHostInstance);
        }
    return  *s_affinityHostInstance;
    }

END_C3D_NAMESPACE


extern "C"
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
EXPORT_ATTRIBUTE void iModelBridge_getAffinity (WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel, WCharCP affinityLibPath, WCharCP dwgdxfName)
    {
    // supply an affinity to the iModelBridgeAssignment for the input file.
    affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::None;

    BeFileName  filename(dwgdxfName);
    if (!DwgHelper::SniffDwgFile(filename) && !DwgHelper::SniffDxfFile(filename))
        return;

    auto dwg = BentleyApi::C3D::AffinityHost::GetHost().ReadFile (dwgdxfName, false, false, FileShareMode::DenyNo);
    if (!dwg.IsValid())
        return;

    DwgDbObjectId   rootId;
    DwgDbDictionaryPtr  mainDictionary(dwg->GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
    if (mainDictionary.OpenStatus() == DwgDbStatus::Success && mainDictionary->GetIdAt(rootId, L"Root") == DwgDbStatus::Success)
        {
        DwgDbObjectPtr  aeccRoot(rootId, DwgDbOpenMode::ForRead);
        if (aeccRoot.OpenStatus() == DwgDbStatus::Success)
            {
            auto className = aeccRoot->GetDwgClassName ();
            if (className.EqualsI(L"AeccDbTreeNode"))
                affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::ExactMatch;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey)
    {
    return  new BentleyApi::C3D::C3dBridge();
    }

}   // extern "C"
