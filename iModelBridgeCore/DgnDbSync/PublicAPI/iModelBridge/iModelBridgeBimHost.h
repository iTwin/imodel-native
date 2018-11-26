/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeBimHost.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! @private
// @bsiclass                                    BentleySystems
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE iModelBridgeKnownLocationsAdmin : Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName        m_fwkAssetsDir;
    static BeFileName s_tempDirectory;

    iModelBridgeKnownLocationsAdmin(BeFileNameCR fa) : m_fwkAssetsDir(fa) {}

    IMODEL_BRIDGE_EXPORT BeFileNameCR _GetLocalTempDirectoryBaseName() override;
    IMODEL_BRIDGE_EXPORT BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_fwkAssetsDir;}
};

//=======================================================================================
//! @private
// @bsiclass                                                    Jeff.Marker     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE iModelBridgeFontAdmin : DgnPlatformLib::Host::FontAdmin
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
//! An implementation of Dgn::DgnPlatformLib::Host that iModelBridgeFwk sets up before running a bridge. 
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE iModelBridgeBimHost : Dgn::DgnPlatformLib::Host
{
    RepositoryAdmin*    m_repoAdmin;
    BeFileName          m_fwkSqlangPath;
    BeFileName          m_fwkAssetsDir;
    Utf8String          m_productName;

    IMODEL_BRIDGE_EXPORT void _SupplyProductName(Utf8StringR name) override {name = m_productName;}
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override {return *new iModelBridgeKnownLocationsAdmin(m_fwkAssetsDir);}
    IMODEL_BRIDGE_EXPORT BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
    RepositoryAdmin& _SupplyRepositoryAdmin() override {return *m_repoAdmin;}
    FontAdmin& _SupplyFontAdmin() override {return *new iModelBridgeFontAdmin;}
    IMODEL_BRIDGE_EXPORT bool _IsFeatureEnabled(Utf8CP) override;

    //! Construct the host to use when running bridges.
    //! @param ra               The framework's RepositoryAdmin
    //! @param fwkAssetsDir     The full path to the framework's Assets directory
    //! @param fwkSqlangPath    The full path to the framework's .db3 file
    //! @param productName      The name of the product that is using this host
    iModelBridgeBimHost(RepositoryAdmin* ra, BeFileNameCR fwkAssetsDir, BeFileNameCR fwkSqlangPath, Utf8StringCR productName) 
        : m_repoAdmin(ra), m_fwkAssetsDir(fwkAssetsDir), m_fwkSqlangPath(fwkSqlangPath), m_productName(productName) {}
};

END_BENTLEY_DGN_NAMESPACE
