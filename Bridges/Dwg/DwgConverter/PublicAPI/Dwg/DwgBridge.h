/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#if defined(_MSC_VER) && defined(DWGTOOLKIT_RealDwg)
// RealDWG uses MFC which requires afx headers to be included first!
#ifndef UNICODE
    #define     UNICODE
#endif
#ifndef _AFXDLL
    #define     _AFXDLL
    #include    <afxwin.h>
#endif
#endif

#include <Dwg/DwgImporter.h>
#include <iModelBridge/iModelBridgeBimHost.h>
#include <WebServices/Client/ClientInfo.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWG


BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgBridge : iModelBridge
{
DEFINE_T_SUPER (iModelBridge)
private:
    DwgImporter::Options            m_options;
    std::unique_ptr<DwgImporter>    m_importer;

protected:
    // Instantiate a new DwgImporter - consumer to override with a customer importer:
    DWG_EXPORT virtual DwgImporter* _CreateDwgImporter ();
    //! Set CONNECTION Client info to track product usage etc. The default implementation sets base product info:
    //! Product name: iModelBridgeService-Dwg
    //! Product GUID: 977E6087-1F14-420E-B51C-C1BC0F128BC4
    //! Product PRGID: 2706
    DWG_EXPORT virtual void _SetClientInfo ();

    // iModelBridge overrides
    DWG_EXPORT WString        _SupplySqlangRelPath () override;
    DWG_EXPORT BentleyStatus   _Initialize (int argc, WCharCP argv[]) override;
    DWG_EXPORT Dgn::SubjectCPtr _InitializeJob () override;
    DWG_EXPORT Dgn::SubjectCPtr _FindJob () override;
    DWG_EXPORT BentleyStatus  _ConvertToBim (Dgn::SubjectCR jobSubject) override;
    //! Default implementation walks through DWG block section and create the DwgAttributeDefinitions schema.
    DWG_EXPORT BentleyStatus  _MakeSchemaChanges (bool &hasMoreChanges) override;
    //! Share common dictionaries (categories, materials, linestyles & textstyles) with other bridges.
    DWG_EXPORT BentleyStatus  _MakeDefinitionChanges(SubjectCR jobSubject) override;
    DWG_EXPORT BentleyStatus  _OnOpenBim (DgnDbR db) override;
    DWG_EXPORT void           _OnCloseBim (BentleyStatus, iModelBridge::ClosePurpose purpose) override;
    DWG_EXPORT BentleyStatus  _OpenSource () override;
    DWG_EXPORT void           _CloseSource (BentleyStatus,iModelBridge::ClosePurpose) override;
    DWG_EXPORT BentleyStatus  _DetectDeletedDocuments() override;
    DWG_EXPORT void           _PrintUsage () override;
    iModelBridge::Params&     _GetParams () override { return m_options; }
    DWG_EXPORT DgnFontCP      _TryResolveFont (DgnFontCP) override;
    DWG_EXPORT CmdLineArgStatus _ParseCommandLineArg (int iArg, int argc, WCharCP argv[]) override;
    DWG_EXPORT void           _Terminate (BentleyStatus convertStatus) override;

public:
    DWG_EXPORT DwgBridge ();
    DWG_EXPORT BentleyStatus  RunAsStandaloneExe (int argc, WCharCP argv[]);
    DWG_EXPORT DwgImporter::Options&  GetImportOptions () { return m_options; }

private:
    // local class methods
    BentleyStatus   GetLogConfigurationFilename (BeFileNameR configFile, WCharCP argv0);
    BentleyStatus   GetEnv (BeFileName& fn, WCharCP envname);
    void    GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0);
};  // DwgBridge

END_DWG_NAMESPACE


// Supply DwgBridge to and register it for iModelBridge Framework
extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getInstance iModelBridge_getInstance;
    EXPORT_ATTRIBUTE T_iModelBridge_releaseInstance iModelBridge_releaseInstance;
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    }
