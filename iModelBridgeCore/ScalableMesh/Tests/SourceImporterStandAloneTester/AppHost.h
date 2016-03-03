// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define winNT

#include "Bentley\Bentley.h"
#include "Bentley\Bentley.r.h"
#include "Bentley\BeCriticalSection.h"

#include "DgnPlatform\DgnPlatform.h"
#include "DgnPlatform\DgnPlatformLib.h"
#include "DgnPlatform\DgnFileIO\DgnFileIOLib.h"
#include "DgnPlatform\NotificationManager.h"
#include "DgnPlatform\SolidKernel.h"
#include "DgnView\DgnViewLib.h"
#include "DgnView\HUDManager.h"

#define _AMD64_
#include <windef.h>

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
/*
class AppViewSet : public CDocument, public IndexedViewSet
{
    DECLARE_DYNCREATE(AppViewSet)
    DECLARE_MESSAGE_MAP()

    DgnFilePtr   m_dgnFile;
    int          m_currentView;

    //virtual int _GetCurrentViewNumber() override {return m_currentView;}
    //virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
    //virtual BOOL OnNewDocument() override {return false;}

public:
    AppViewSet();
    virtual ~AppViewSet ();

    DgnFileP        GetDgnFile()                    {return m_dgnFile.get();}
    DemoViewportP   GetDemoViewport(int index);
    DemoViewportP   GetCurrentViewport()            {return GetDemoViewport(m_currentView);}
    void            OpenViewport (int index, ViewInfoR viewInfo, class DemoViewWindow&);
};
*/
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct AppViewManager : BENTLEY_NAMESPACE_NAME::DgnPlatform::IViewManager
{
private:
    BENTLEY_NAMESPACE_NAME::DgnPlatform::IndexedViewSet* m_activeViewSet;
    HWND                                  m_topWindow;
    
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override {return (BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnDisplayCoreTypes::WindowP)m_topWindow;}
    virtual bool                                               _DoesHostHaveFocus() {return false;}
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::IndexedViewSet&              _GetActiveViewSet() override {assert(!"Not expect to be call in offline mode"); return *m_activeViewSet;}
    virtual int             _GetCurrentViewNumber() override {return 0;}
    virtual HUDManagerP     _GetHUDManager () {return NULL;}

public:
    
    //void SetActiveDgn (AppViewSet* newDgn) {m_activeViewSet = newDgn;}
    //void SetActiveDgn (DemoViewSet* newDgn) {m_activeViewSet = newDgn;}
    AppViewManager() {m_activeViewSet = NULL; m_topWindow = NULL;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
class AppHost : public BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnViewLib::Host
{
    AppViewManager   m_viewManager;

protected:

    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;        
    virtual void                                                            _SupplyProductName(WStringR name) override;     
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager& _SupplyDigitalRightsManager() override;     
    //virtual GraphicsAdmin&                                                 _SupplyGraphicsAdmin() override;            
    //virtual ViewStateAdmin&                                                _SupplyViewStateAdmin() override;           
    //virtual ToolAdmin&                                                     _SupplyToolAdmin() override;                
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::IViewManager&                             _SupplyViewManager() override;              
    //virtual SolidsKernelAdmin&                                             _SupplySolidsKernelAdmin() override;        
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::RasterAttachmentAdmin&      _SupplyRasterAttachmentAdmin() override;    
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::PointCloudAdmin&            _SupplyPointCloudAdmin() override;          
    //virtual FontAdmin&                                                     _SupplyFontAdmin() override;                
    //virtual MaterialAdmin&                                                 _SupplyMaterialAdmin();                     
    //virtual ProgressiveDisplayManager&                                     _SupplyProgressiveDisplayManager() override;
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;

public:
    void Startup (/*HWND*/);
        
    void Terminate ();

    //DemoViewManager& GetDemoViewManager(){return m_viewManager;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct AppNotificationAdmin : BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin
{
    virtual StatusInt _OutputMessage (BENTLEY_NAMESPACE_NAME::DgnPlatform::NotifyMessageDetails const& msg) override;
    virtual void      _OutputPrompt (WCharCP) override;
    virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxValue _OpenMessageBox (BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxType, WCharCP, BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxIconType) override;
};

/*=================================================================================**//**
* DgnViewDemo only displays the contents of a file on the screen. It does not EXPORT
* engineering data. Therefore, it is safe for DgnViewDemo to open rights-restricted DGN files.
* @bsiclass                                     Sam.Wilson                      06/2010
+===============+===============+===============+===============+===============+======*/
struct ReadOnlyDigitalRightsManager : BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager
{
//    virtual StatusInt _OnEnterRestrictedMode (bool assertKeys, BENTLEY_NAMESPACE_NAME::DgnFileProtection::KeyMaterialWithDescription* keylist, uint32_t nkeys, BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileP file, uint32_t rights) override {return SUCCESS;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*
struct AppSolidKernelAdmin : BENTLEY_NAMESPACE_NAME::DgnPlatform::PSolidKernelAdmin
    {
    virtual BentleyStatus _RestoreEntityFromMemory (DgnPlatform::ISolidKernelEntityPtr&, void const* pBuffer, unsigned int bufferSize, DgnPlatform::ISolidKernelEntity::SolidKernelType kernelType, TransformCR) const override;
    };
    */
