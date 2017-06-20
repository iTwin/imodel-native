/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeBimHost.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <iModelBridge/iModelBridge.h>
#include <DgnView/DgnViewLib.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct iModelBridgeKnownLocationsAdmin : Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin
{
    static BeFileName s_assetsDirectory;
    static BeFileName s_tempDirectory;

    IMODEL_BRIDGE_EXPORT BeFileNameCR _GetLocalTempDirectoryBaseName() override;
    IMODEL_BRIDGE_EXPORT BeFileNameCR _GetDgnPlatformAssetsDirectory() override;
    IMODEL_BRIDGE_EXPORT static void SetAssetsDir(BeFileNameCR);
};

//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridgeNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
{
    virtual BentleyApi::StatusInt _OutputMessage(NotifyMessageDetails const& msg) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGE: %s %s\n", msg.GetBriefMsg().c_str(), msg.GetDetailedMsg().c_str());
        return BentleyApi::SUCCESS;
        }

    virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType t, BentleyApi::Utf8CP msg, NotificationManager::MessageBoxIconType iconType) override
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->warningv("MESSAGEBOX: %s\n", msg);
        printf("<<NOTIFICATION MessageBox: %s >>\n", msg);
        return NotificationManager::MESSAGEBOX_VALUE_Ok;
        }

    virtual void _OutputPrompt(BentleyApi::Utf8CP msg) override
        { // Log this as an error because we cannot prompt while running a unit test!
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"NOTIFICATION-ADMIN")->errorv("PROMPT (IGNORED): %s\n", msg);
        }

    virtual bool _GetLogSQLiteErrors() override 
        {
        return BentleyApi::NativeLogging::LoggingManager::GetLogger("BeSQLite")->isSeverityEnabled(BentleyApi::NativeLogging::LOG_INFO);
        }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct iModelBridgeViewManager : ViewManager
{
    Display::SystemContext* m_systemContext = nullptr;
    IMODEL_BRIDGE_EXPORT Display::SystemContext* _GetSystemContext() override;
    bool _DoesHostHaveFocus() override {return true;}
};

//=======================================================================================
// Allows the converter app to forward ResolveFont calls to the converter.
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct iModelBridgeFontAdmin : DgnPlatformLib::Host::FontAdmin
{
private:
    DEFINE_T_SUPER(DgnPlatformLib::Host::FontAdmin);
//    Converter* m_converter;

protected:
    IMODEL_BRIDGE_EXPORT virtual DgnFontCR _ResolveFont(DgnFontCP) override;

public:
//    Converter* GetConverter() const { return m_converter; }
//    void SetConverter(Converter* value) { m_converter = value; }
};

//=======================================================================================
//! An implementation of Dgn::DgnViewLib::Host that an iModelBridge can use to register
//! a host in its iModelBridge::_Initialize method. 
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridgeBimHost : Dgn::DgnViewLib::Host
{
    RepositoryAdmin*    m_repoAdmin;
    WString             m_sqlangRelPath;

    IMODEL_BRIDGE_EXPORT void _SupplyProductName(Utf8StringR name) override;
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new iModelBridgeKnownLocationsAdmin();}
    IMODEL_BRIDGE_EXPORT BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
    NotificationAdmin& _SupplyNotificationAdmin() override {return *new iModelBridgeNotificationAdmin();}
    ViewManager& _SupplyViewManager() override {return *new iModelBridgeViewManager ();}
    RepositoryAdmin& _SupplyRepositoryAdmin() override {return *m_repoAdmin;}
    FontAdmin& _SupplyFontAdmin() override {return *new iModelBridgeFontAdmin;}

    iModelBridgeBimHost(RepositoryAdmin* ra, WString const& sqlangRelPath) : m_repoAdmin(ra), m_sqlangRelPath(sqlangRelPath) {}
};

END_BENTLEY_DGN_NAMESPACE
