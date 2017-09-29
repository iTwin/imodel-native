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

BEGIN_ORDBRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void iModelBridge_getAffinity(iModelBridge::BridgeAffinity& bridgeAffinity, BeFileName const& affinityLibraryPath, BeFileName const& sourceFileName)
    {
    bridgeAffinity.m_affinity = iModelBridge::Affinity::None;

    try
        {
        static AffinityHost host;
        DgnV8Api::DgnPlatformLib::Host* threadHost = DgnV8Api::DgnPlatformLib::QueryHost();
        if (NULL == threadHost)
            DgnV8Api::DgnPlatformLib::Initialize(host, false);

        DgnFileStatus status;
        DgnDocumentPtr doc = DgnDocument::CreateForLocalFile(sourceFileName.c_str());
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

        auto cifConnPtr = ConsensusConnection::Create(*rootModelP);
        auto cifModelPtr = ConsensusModel::Create(*cifConnPtr);
        if (cifModelPtr.IsValid())
            {
            bridgeAffinity.m_bridgeRegSubKey = L"OpenRoads Designer Bridge";
            bridgeAffinity.m_affinity = iModelBridgeAffinityLevel::ExactMatch;
            return;
            }

        bridgeAffinity.m_bridgeRegSubKey = L"MicroStation";
        bridgeAffinity.m_affinity = iModelBridge::Affinity::Low;
        }
    catch (...)
        {
        return; 
        }
    }

END_ORDBRIDGE_NAMESPACE