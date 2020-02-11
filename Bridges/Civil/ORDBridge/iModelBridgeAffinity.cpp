/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ORDBridgeInternal.h>

/*=================================================================================**//**
* @bsiclass                                                     Abeesh.Basheer     03/09
+===============+===============+===============+===============+===============+======*/
struct NotificationAdmin : DgnV8Api::DgnPlatformLib::Host::NotificationAdmin
    {
    protected:  virtual StatusInt   _OutputMessage  (DgnV8Api::NotifyMessageDetails const&) override {return SUCCESS;}
    }; // DummyNotificationManager

/*=================================================================================**//**
* @bsiclass                                                     Abeesh.Basheer     03/09
+===============+===============+===============+===============+===============+======*/
struct AffinityHost : DgnV8Api::DgnPlatformLib::Host
    {
    private:
    template <typename AdminType> AdminType& GetAdmin ();

    virtual NotificationAdmin&      _SupplyNotificationAdmin() override
        {
        return GetAdmin <NotificationAdmin>();
        }

    public:

    void    Initialize ();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename AdminType>
AdminType& AffinityHost::GetAdmin ()
    {
    static bool s_initialized = false;
    AdminType* s_adminhost = NULL;
    if (s_initialized)
        return *s_adminhost;

    s_adminhost = new AdminType();
    s_initialized = true;
    return *s_adminhost;
    }

USING_NAMESPACE_BENTLEY_ORDBRIDGE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void iModelBridge_getAffinity(WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
    WCharCP affinityLibraryPath, WCharCP sourceFileName)
    {
    affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::None;
    BentleyApi::BeFileName sourceFile(sourceFileName);
    BentleyApi::BeFileName lowercaseSourceFile(sourceFile.ToLower().c_str());
    if (!lowercaseSourceFile.EndsWith(L".dgn"))
        return;

    if (lowercaseSourceFile.EndsWith(L".i.dgn"))
        return;

    BentleyApi::BeFileName originalPath(::_wgetenv(L"PATH"));

    BentleyApi::BeFileName dirPath(BentleyApi::BeFileName::DevAndDir, affinityLibraryPath);

    BentleyApi::WString path(L"PATH=");
    path.append(dirPath);
    path.append(L";");
    path.append(::_wgetenv(L"PATH"));
    _wputenv(path.c_str());

    ORDBridge::AppendCifSdkToDllSearchPath(dirPath);
    ORDBridge::AppendObmSdkToDllSearchPath(dirPath);

    BentleyApi::BeFileName v8Path = dirPath;
    v8Path.AppendToPath(L"DgnV8");

    path = L"PATH=";
    path.append(v8Path);
    path.append(L";");
    path.append(::_wgetenv(L"PATH"));
    _wputenv(path.c_str());    

    DgnV8Api::DgnPlatformLib::Host* threadHost = nullptr;

    try
        {        
        static AffinityHost host;
        threadHost = DgnV8Api::DgnPlatformLib::QueryHost();
        bool wasThreadHostCreated = (threadHost != nullptr);
        if (nullptr == threadHost)
            DgnV8Api::DgnPlatformLib::Initialize(host, false);

        DgnDocumentPtr doc = DgnDocument::CreateForLocalFile(sourceFileName);
        if (doc.IsNull())
            return;

        DgnFilePtr dgnFilePtr = DgnFile::Create(*doc, DgnFileOpenMode::ReadOnly);
        if (dgnFilePtr.IsNull())
            return;

        StatusInt nStatus;
        dgnFilePtr->LoadFile(&nStatus, nullptr, true);

        DgnV8Api::ModelId modelId = dgnFilePtr->GetDefaultModelId();

        DependencyManager::SetProcessingDisabled(false);

        auto rootModelP = dgnFilePtr->LoadRootModelById(&nStatus, modelId, true, true, true);

        DependencyManager::SetProcessingDisabled(true);

        if (!rootModelP)
            return;

        // Initialize Cif SDK
        if (!wasThreadHostCreated)
            {
            DgnPlatformCivilLib::InitializeWithDefaultHost();
            GeometryModelDgnECDataBinder::GetInstance().Initialize();
            }

        auto cifConnPtr = ConsensusConnection::Create(*rootModelP);
        auto cifModelPtr = ConsensusModel::Create(*cifConnPtr);
        if (cifModelPtr.IsValid())
            {
            auto geomModelsPtr = cifModelPtr->GetActiveGeometricModels();
            while (geomModelsPtr.IsValid() && geomModelsPtr->MoveNext())
                {
                // At least one CIF geometric model found...
                BeStringUtilities::Wcsncpy(buffer, bufferSize, ORDBridge::GetRegistrySubKey());
                affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::ExactMatch;
                }
            }

        cifConnPtr = nullptr;
        dgnFilePtr = nullptr;

        if (BentleyApi::Dgn::iModelBridgeAffinityLevel::None == affinityLevel)
            {
            BeStringUtilities::Wcsncpy(buffer, bufferSize, L"MicroStation");
            affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::Low;
            }
        }
    catch (...)
        {
        }    

    //DgnPlatformCivilLib::Unload();
//    if (nullptr == threadHost)
//        DgnV8Api::DgnPlatformLib::ForgetHost();

    _wputenv(originalPath.c_str());
    }