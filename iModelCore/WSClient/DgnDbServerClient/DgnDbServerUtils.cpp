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

std::unique_ptr<DgnDbServerHost> DgnDbServerHost::m_host(nullptr);

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerHost::DgnDbServerHost(BeFileNameCR temp, BeFileNameCR assets) : m_temp(temp), m_assets(assets)
    {
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
void DgnDbServerHost::Initialize(BeFileNameCR temp, BeFileNameCR assets)
    {
    BeAssert(!m_host);
    m_host = std::make_unique<DgnDbServerHost>(temp, assets);
    Dgn::DgnPlatformLib::Initialize(*m_host, true, false);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
bool DgnDbServerHost::IsInitialized()
    {
    return (nullptr != m_host);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerHost::~DgnDbServerHost()
    {
    if (m_host)
        {
        Dgn::DgnPlatformLib::AdoptHost(*m_host);
        Terminate(true);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::IKnownLocationsAdmin& DgnDbServerHost::_SupplyIKnownLocationsAdmin()
    {
    return *new DgnDbServerLocationsAdmin(m_temp, m_assets);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2015
//---------------------------------------------------------------------------------------
DgnDbServerHost& DgnDbServerHost::Host()
    {
    return *m_host;
    }
END_BENTLEY_DGNDBSERVER_NAMESPACE
