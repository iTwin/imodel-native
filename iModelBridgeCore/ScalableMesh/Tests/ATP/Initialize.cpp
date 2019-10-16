#include "Initialize.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <wtypes.h>

using namespace std;
#include <DgnPlatform/IAuxCoordSys.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshURL.h>
#include <DgnPlatform/DgnPlatform.h>

#ifndef VANCOUVER_API   
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#define VIEWMANAGER ViewManager
#else 
#include <DgnView/DgnViewLib.h>
#define VIEWMANAGER IViewManager
#include <DgnGeoCoord/DgnGeoCoord.h>
#endif



#include <ScalableMesh/ScalableMeshAdmin.h>
#include <ScalableMesh/ScalableMeshLib.h>



USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif

using namespace BENTLEY_NAMESPACE_NAME::DgnPlatform;
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

namespace ScalableMeshATPexe
{
#ifdef VANCOUVER_API 
struct AppViewManager : VIEWMANAGER
    {
    protected:
#ifndef VANCOUVER_API   
        virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
#else
        virtual Bentley::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override { return nullptr; }
        virtual int                                                _GetCurrentViewNumber() override { return 0; }
        virtual HUDManager*                                        _GetHUDManager() { return nullptr; }
#endif
        virtual bool                _DoesHostHaveFocus()        override { return true; }
        virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
        virtual int                 _GetDynamicsStopInterval()  override { return 200; }


    public:
        AppViewManager() {}
        ~AppViewManager() {}
    };
#endif
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
#ifdef VANCOUVER_API 
class AppHost : public ScalableMesh::ScalableMeshLib::Host, public DgnViewLib::Host
#else
class AppHost : public ScalableMesh::ScalableMeshLib::Host, public DgnPlatformLib::Host
#endif
    {

    protected:

#ifndef VANCOUVER_API               
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::KnownDesktopLocationsAdmin; }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
        virtual void                                      _SupplyProductName(Utf8StringR name) override;
#else
        virtual void                                      _SupplyProductName(WStringR name) override;
#endif

        virtual DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;        
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;     
		
#ifdef VANCOUVER_API    	
		virtual VIEWMANAGER& _SupplyViewManager() override { return *new AppViewManager(); }
#endif		

    public:
        void Startup();

        void Terminate();
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct AppNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
    {
#ifndef VANCOUVER_API   
    virtual StatusInt _OutputMessage(BentleyApi::Dgn::NotifyMessageDetails const& msg) override;
    virtual void      _OutputPrompt(Utf8CP) override;
    virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP, NotificationManager::MessageBoxIconType) override;
#endif
    };

void AppHost::Startup()
    {
    //Ensure basegeocoord is initialized.
    _SupplyGeoCoordinationAdmin()._GetServices();

    InitializeLogging();
    }

void AppHost::Terminate()
    {
    }

#ifndef VANCOUVER_API   
void                                                                   AppHost::_SupplyProductName(Utf8StringR name) { name.assign("DgnView Demo"); }
#else
void                                                                   AppHost::_SupplyProductName(WStringR name) { name.assign(L"DgnView Demo"); }
#endif

DgnPlatformLib::Host::NotificationAdmin&         AppHost::_SupplyNotificationAdmin() { return *new AppNotificationAdmin(); }
DgnPlatformLib::Host::GeoCoordinationAdmin&      AppHost::_SupplyGeoCoordinationAdmin()
    {
    
#ifndef VANCOUVER_API  
    BeFileName geocoordinateDataPath(L".\\Assets\\DgnGeoCoord\\");
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
#else    
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath, IACSManager::GetManager());
#endif
    }

#ifndef VANCOUVER_API  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+-****/
StatusInt AppNotificationAdmin::_OutputMessage(NotifyMessageDetails const& msg)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      AppNotificationAdmin::_OutputPrompt(Utf8CP prompt)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue AppNotificationAdmin::_OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP message, NotificationManager::MessageBoxIconType iconType)
    {
    return NotificationManager::MESSAGEBOX_VALUE_Ok;
    }
#endif



struct ExeAdmin : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin
    {
    DgnModel* s_activeDgnModelRefP;
    ExeAdmin() :s_activeDgnModelRefP(0) {};

    ~ExeAdmin() {};

    bool _CanImportPODfile() const
        {
        return true;
        }

    void SetActiveModelRef(DgnModel* activeDgnModelRefP)
        {
        s_activeDgnModelRefP = activeDgnModelRefP;
        }

    DgnModel* _GetActiveModelRef() const
        {
        return s_activeDgnModelRefP;
        }

    virtual uint64_t _GetProductId() const override
        {
        return 1; //Product ID for internal application.        
        }    

    };

struct ExeHost : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Host
    {

    ExeHost()
        {
        BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Initialize(*this);        
        }

    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ExeAdmin();
        };
    };

void InitializeATP(DgnPlatformLib::Host& host)
    {
    static AppHost appHost;
    appHost.Startup();

    BeFileName name;

#ifdef VANCOUVER_API       
    BeFileNameStatus beStatus = BeFileName::BeGetTempPath(name);
    assert(BeFileNameStatus::Success == beStatus);
#else        
    DgnPlatformLib::Host::IKnownLocationsAdmin& locationAdmin(DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin());
    name = locationAdmin.GetLocalTempDirectoryBaseName();
    assert(!name.IsEmpty());
#endif
    
    name.AppendToPath(L"temp.dgn");

    static ExeHost smHost;
    }

void CloseATP()
    {
    }
}

