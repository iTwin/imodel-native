#include "DgnDbServerUtils.h"
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
CallbackQueue::Callback::Callback(CallbackQueue & queue) : m_bytesTransfered(0.0), m_bytesTotal(0.0), m_queue(queue)
    {
    callback = [=] (double bytesTransfered, double bytesTotal)
        {
        m_bytesTransfered = bytesTransfered;
        m_bytesTotal = bytesTotal;
        m_queue.Notify();
        };
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
CallbackQueue::CallbackQueue(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback) : m_callback(callback)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
HttpRequest::ProgressCallbackCR CallbackQueue::NewCallback()
    {
    std::shared_ptr<CallbackQueue::Callback> callback = std::make_shared<CallbackQueue::Callback>(*this);
    m_callbacks.push_back(callback);
    return callback->callback;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
void CallbackQueue::Notify()
    {
    double transfered = 0.0;
    double total = 0.0;
    for (auto& callback : m_callbacks)
        {
        transfered += callback->m_bytesTransfered;
        total += callback->m_bytesTotal;
        }
    m_callback(transfered, total);
    }

//=======================================================================================
//@bsiclass                                      Karolis.Dziedzelis             11/2015
//=======================================================================================
struct DgnDbServerLocationsAdmin : public Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    private:
        BeFileName m_temp;
        BeFileName m_assets;
    protected:
        virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_temp; };
        virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assets; };
    public:
        DgnDbServerLocationsAdmin(BeFileNameCR temp, BeFileNameCR assets) : m_temp(temp), m_assets(assets) {}
        virtual ~DgnDbServerLocationsAdmin() {}
    };

BeFileName DgnDbServerHost::s_temp(L"");
BeFileName DgnDbServerHost::s_assets(L"");

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerHost::DgnDbServerHost() : m_initialized(false), m_terminated(false)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbServerHost::Initialize(BeFileNameCR temp, BeFileNameCR assets)
    {
    s_temp = temp;
    s_assets = assets;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
bool DgnDbServerHost::IsInitialized()
    {
    return !s_assets.IsEmpty() && !s_temp.IsEmpty();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             01/2016
//---------------------------------------------------------------------------------------
void DgnDbServerHost::Adopt(std::shared_ptr<DgnDbServerHost> const& host)
    {
    if (DgnPlatformLib::QueryHost())
        return;

    if (!host->m_initialized)
        {
        DgnPlatformLib::Initialize(*host, false);
        host->m_initialized = true;
        }
    else
        {
        DgnPlatformLib::AdoptHost(*host);
        BeStringUtilities::Initialize(host->GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             01/2016
//---------------------------------------------------------------------------------------
void DgnDbServerHost::Forget(std::shared_ptr<DgnDbServerHost> const& host, bool terminate)
    {
    if (DgnPlatformLib::QueryHost() == host.get())
        {
        if (terminate && host->m_initialized && !host->m_terminated)
            {
            host->m_terminated = true;
            host->Terminate(false);
            }
        else
            {
            DgnPlatformLib::ForgetHost();
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerHost::~DgnDbServerHost()
    {
    if (m_initialized && !m_terminated)
        {
        auto wasHost = DgnPlatformLib::QueryHost();
        if (wasHost && wasHost != this)
            {
            DgnPlatformLib::ForgetHost();
            DgnPlatformLib::AdoptHost(*this);
            }
        else if (!wasHost)
            {
            DgnPlatformLib::AdoptHost(*this);
            }
        Terminate(false);
        if (wasHost && wasHost != this)
            {
            DgnPlatformLib::AdoptHost(*wasHost);
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::IKnownLocationsAdmin& DgnDbServerHost::_SupplyIKnownLocationsAdmin()
    {
    return *new DgnDbServerLocationsAdmin(s_temp, s_assets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool BeInt64IdFromJson (BeInt64Id& id, JsonValueCR value)
    {
    if (value.isNull ())
        return false;

    if (value.isString ())
        {
        uint64_t idParsed;
        if (SUCCESS != BeStringUtilities::ParseUInt64 (idParsed, value.asCString ()))
            return false;
        id = BeInt64Id (idParsed);
        }
    else
        id = BeInt64Id((uint64_t) value.asInt64 ());

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool StringFromJson (Utf8StringR result, JsonValueCR value)
    {
    if (value.isNull ())
        return false;

    result = value.asCString ();
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
bool GetLockFromServerJson (JsonValueCR serverJson, DgnLockR lock, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR repositoryId)
    {
    BeInt64Id id;
    LockLevel           level;
    LockableType        type;

    if (!BeInt64IdFromJson (id, serverJson[ServerSchema::Property::ObjectId])                             ||
        !RepositoryJson::LockLevelFromJson (level, serverJson[ServerSchema::Property::LockLevel])           ||
        !RepositoryJson::LockableTypeFromJson (type, serverJson[ServerSchema::Property::LockType])          ||
        !RepositoryJson::BriefcaseIdFromJson (briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]) ||
        !StringFromJson (repositoryId, serverJson[ServerSchema::Property::ReleasedWithRevision]))
        return false;

    lock = DgnLock (LockableId (type, id), level);

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
void AddLockInfoToList (DgnLockInfoSet& lockInfos, const DgnLock& dgnLock, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR repositoryId)
    {
    DgnLockInfo&      info = *lockInfos.insert (DgnLockInfo (dgnLock.GetLockableId ())).first;
    DgnLockOwnershipR ownership = info.GetOwnership ();

    if (LockLevel::Exclusive == dgnLock.GetLevel ())
        ownership.SetExclusiveOwner (briefcaseId);
    else if (LockLevel::Shared == dgnLock.GetLevel ())
        ownership.AddSharedOwner (briefcaseId);

    if (!repositoryId.empty ())
        info.SetRevisionId (repositoryId);

    if (info.IsOwned () || !repositoryId.empty ())
        info.SetTracked ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
bool CodeStateFromJson(DgnCodeStateR codeState, JsonValueCR value, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR revisionId)
    {
    if (value.isNull())
        return false;

    uint64_t stateInt;

    if (value.isString())
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(stateInt, value.asCString()))
            return false;
        }
    else
        stateInt = (uint64_t)value.asInt64();

    if (stateInt == 0)
        {
        codeState.SetAvailable();
        return true;
        }
    else if (stateInt == 1)
        {
        codeState.SetReserved(briefcaseId);
        return true;
        }
    else if (stateInt == 2)
        {
        codeState.SetUsed(revisionId);
        return true;
        }
    else if (stateInt == 3)
        {
        codeState.SetDiscarded(revisionId);
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
bool GetCodeFromServerJson(JsonValueCR serverJson, DgnCodeR code, DgnCodeStateR codeState, BeSQLite::BeBriefcaseId& briefcaseId, Utf8StringR revisionId)
    {
    BeInt64Id      authorityId;
    Utf8String     nameSpace = "";
    Utf8String     value = "";

    if (!BeInt64IdFromJson(authorityId, serverJson[ServerSchema::Property::AuthorityId]) ||
        !StringFromJson(nameSpace, serverJson[ServerSchema::Property::Namespace]) ||
        !StringFromJson(value, serverJson[ServerSchema::Property::Value]) ||
        !RepositoryJson::BriefcaseIdFromJson(briefcaseId, serverJson[ServerSchema::Property::BriefcaseId]) ||
        !CodeStateFromJson(codeState, serverJson[ServerSchema::Property::State], briefcaseId, revisionId))
        return false;

    // State revision is optional
    StringFromJson(revisionId, serverJson[ServerSchema::Property::StateRevision]);

    code = DgnCode(DgnAuthorityId(authorityId.GetValue()), value, nameSpace);
    
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas              06/2016
//---------------------------------------------------------------------------------------
void AddCodeInfoToList(DgnCodeInfoSet& codeInfos, const DgnCode& dgnCode, DgnCodeState codeState, const BeSQLite::BeBriefcaseId briefcaseId, Utf8StringCR revisionId)
    {
    DgnCodeInfo&      info = *codeInfos.insert(DgnCodeInfo(dgnCode)).first;
    
    if (codeState.IsAvailable())
        {
        info.SetAvailable();
        }
    else if(codeState.IsReserved())
        {
        info.SetReserved(briefcaseId);
        }
    else if (codeState.IsUsed())
        {
        info.SetUsed(revisionId);
        }
    else if (codeState.IsDiscarded())
        {
        info.SetDiscarded(revisionId);
        }
    }

END_BENTLEY_DGNDBSERVER_NAMESPACE
