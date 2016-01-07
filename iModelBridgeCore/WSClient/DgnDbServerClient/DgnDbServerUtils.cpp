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

BeFileName DgnDbServerHost::m_temp(L"");
BeFileName DgnDbServerHost::m_assets(L"");

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
    m_temp = temp;
    m_assets = assets;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
bool DgnDbServerHost::IsInitialized()
    {
    return !m_assets.IsEmpty() && !m_temp.IsEmpty();
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
    return *new DgnDbServerLocationsAdmin(m_temp, m_assets);
    }
END_BENTLEY_DGNDBSERVER_NAMESPACE
