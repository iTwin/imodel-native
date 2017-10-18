/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgBridge.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined(_MSC_VER) && defined(DWGTOOLKIT_RealDwg)
// RealDWG uses MFC which requires afx headers to be included first!
#ifndef UNITCODE
    #define     UNICODE
#endif
#ifndef _AFXDLL
    #define     _AFXDLL
    #include    <afxwin.h>
#endif
#endif

#include <DgnDbSync/Dwg/DwgImporter.h>
#include <iModelBridge/iModelBridgeBimHost.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DGNDBSYNC_DWG


BEGIN_DGNDBSYNC_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgBridge : iModelBridge
{
DEFINE_T_SUPER (iModelBridge)

protected:
    DwgImporter::Options            m_options;
    std::unique_ptr<DwgImporter>    m_importer;

    // iModelBridge overrides
    DGNDBSYNC_EXPORT WString        _SupplySqlangRelPath () override;
    DGNDBSYNC_EXPORT BentleyStatus   _Initialize (int argc, WCharCP argv[]) override;
    DGNDBSYNC_EXPORT Dgn::SubjectCPtr _InitializeJob () override;
    DGNDBSYNC_EXPORT Dgn::SubjectCPtr _FindJob () override;
    DGNDBSYNC_EXPORT BentleyStatus  _ConvertToBim (Dgn::SubjectCR jobSubject) override;
    DGNDBSYNC_EXPORT BentleyStatus  _OnConvertToBim (DgnDbR db) override;
    DGNDBSYNC_EXPORT void           _OnConvertedToBim (BentleyStatus) override;
    DGNDBSYNC_EXPORT BentleyStatus  _OpenSource () override;
    DGNDBSYNC_EXPORT void           _CloseSource (BentleyStatus) override;
    DGNDBSYNC_EXPORT void           _DeleteSyncInfo () override;
    DGNDBSYNC_EXPORT BentleyStatus  _OnRootFilesConverted() override;
    DGNDBSYNC_EXPORT void           _PrintUsage () override;
    iModelBridge::Params&           _GetParams () override { return m_options; }
    DGNDBSYNC_EXPORT CmdLineArgStatus _ParseCommandLineArg (int iArg, int argc, WCharCP argv[]) override;

public:
    DGNDBSYNC_EXPORT BentleyStatus  RunAsStandaloneExe (int argc, WCharCP argv[]);

private:
    // local class methods
    BentleyStatus   GetLogConfigurationFilename (BeFileNameR configFile, WCharCP argv0);
    BentleyStatus   GetEnv (BeFileName& fn, WCharCP envname);
    void    GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0);
    void    CreateSyncInfoIfAbsent ();
    DwgImporter::Options&   GetImportOptions () { return m_options; }
};  // DwgBridge

END_DGNDBSYNC_DGNV8_NAMESPACE
