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
    BentleyApi::BeStringUtilities::Wcsncpy(buffer, bufferSize, ORDBridge::GetRegistrySubKey());

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

        if (nStatus != SUCCESS || dgnFilePtr->IsIModel())
            return;

        auto format = dgnFilePtr->GetOriginalFormat();
        // Foreign file formats do not have authoring file info - don't load them:
        if (format != DgnV8Api::DgnFileFormatType::V8 && format != DgnV8Api::DgnFileFormatType::V7)
            return;

        Bentley::WString applicationName;
        if (SUCCESS == dgnFilePtr->GetAuthoringProductName(applicationName))
            {
            if (applicationName.Equals(Bentley::WString(ORD_AUTHORING_PRODNAME).c_str()) ||
                applicationName.Equals(Bentley::WString(ORAIL_AUTHORING_PRODNAME).c_str()) ||
                applicationName.Equals(Bentley::WString(OSITE_AUTHORING_PRODNAME).c_str()))
                {
                affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::ExactMatch;
                return;
                }
            }

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

        if (auto planModelRefP = GeometryModelDgnECDataBinder::GetInstance().GetPlanModelFromModel(rootModelP))
            {
            //in case GetPlanModelFromModel fails to discover planModel, might return the argument, so we still 
            //need to investigate if we have any GeometricModel in the planModelRefP
            auto cifConnPtr = ConsensusConnection::Create(*planModelRefP);
            auto cifModelPtr = ConsensusModel::Create(*cifConnPtr);
            if (cifModelPtr.IsValid())
                {
                auto activeGeomModelPtr = cifModelPtr->GetActiveGeometricModel();//this will return the first found, even if found in some reference attachment
                                                                                 //therefore I will compare its dgnModelP with the planModelRefP we found above, and I will consider it failed even if 
                                                                                 //dgnFile might match, because that means we did not locate correctly the root model.
                if (activeGeomModelPtr.IsValid() && activeGeomModelPtr->GetDgnModelP() == planModelRefP)
                    {
                    //  CIF geometric model found in current file (we might have switched the root model if 3d was the default)...
                    if (applicationName.empty())
                        affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::ExactMatch;
                    else
                        affinityLevel = BentleyApi::Dgn::iModelBridgeAffinityLevel::High;
                    }
                }
            cifConnPtr = nullptr;

            }

        if (BentleyApi::Dgn::iModelBridgeAffinityLevel::None == affinityLevel)
            {
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

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" BentleyApi::BentleyStatus iModelBridge_discloseFilesAndAffinities(WCharCP outputFileName, WCharCP affinityLibraryPathStr, WCharCP assetsPathStr, WCharCP sourceFileNameStr, WCharCP bridgeId)
    {
    ORDBridge bridge;
    return iModelBridge::DiscloseFilesAndAffinities(bridge, outputFileName, affinityLibraryPathStr, assetsPathStr, sourceFileNameStr, bridgeId);
    }
