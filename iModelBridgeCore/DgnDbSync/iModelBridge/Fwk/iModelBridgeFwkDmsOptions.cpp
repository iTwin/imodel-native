/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelDmsSupport/DmsSession.h>
#include <regex>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridgeFwk"))


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
T_iModelDmsSupport_getInstance* iModelBridgeFwk::DmsServerArgs::LoadDmsLibrary(iModelBridgeError& error)
    {
    auto getInstance = (T_iModelDmsSupport_getInstance*) GetBridgeFunction(m_dmsLibraryName, "iModelDmsSupport_getInstance");
    if (!getInstance)
        {
        Utf8PrintfString errorString("%ls: Does not export a function called 'iModelBridge_releaseInstance'", m_dmsLibraryName.c_str());
        error.m_message = errorString;
        error.m_id = iModelBridgeErrorId::MissingFunctionExport;
        return nullptr;
        }

    return getInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::LoadDmsLibrary(iModelBridgeError& errorContext)
    {
    auto getInstance = m_dmsServerArgs.LoadDmsLibrary(errorContext);
    if (nullptr == getInstance)
        return errorContext.GetBentleyStatus();

    m_dmsSupport = getInstance(m_dmsServerArgs.m_dmsType, m_dmsServerArgs.m_dmsCredentials.GetUsername(), m_dmsServerArgs.m_dmsCredentials.GetPassword());//m_dmsCredentials
    
    if (nullptr == m_dmsSupport)
        {
        Utf8PrintfString errorString("%ls: iModelDmsSupport_getInstance function returned a nullptr", m_dmsServerArgs.m_dmsLibraryName.c_str());
        errorContext.m_message = errorString;
        errorContext.m_id = iModelBridgeErrorId::MissingInstance;
        return errorContext.GetBentleyStatus();
        }
    
    if (!m_dmsServerArgs.m_inputFileUrn.empty())
        {
        if (!m_dmsSupport->_InitializeSession(m_dmsServerArgs.m_inputFileUrn))
            {
            errorContext.m_message = "Error communicating with projectwise.";
            errorContext.m_id = iModelBridgeErrorId::ProjectwiseError;
            return errorContext.GetBentleyStatus();
            }
        }
    else if (!m_dmsServerArgs.m_dataSource.empty())
        {
        if (!m_dmsSupport->_InitializeSessionFromDataSource(m_dmsServerArgs.m_dataSource))
            {
            errorContext.m_message = "Error communicating with projectwise.";
            errorContext.m_id = iModelBridgeErrorId::ProjectwiseError;
            return errorContext.GetBentleyStatus();
            }
        }
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::ReleaseDmsLibrary()
    {
    if (m_dmsSupport)
        m_dmsSupport->_UnInitializeSession();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::SetupDmsFiles(FwkContext& context)
    {
    BentleyStatus status = BentleyStatus::SUCCESS;
    if (m_dmsServerArgs.m_dmsLibraryName.empty())
        return status;

    if (SUCCESS != (status = LoadDmsLibrary(context.m_error)))
        return status;

    if (SUCCESS != (status = StageInputFile(context)))
        return ReleaseDmsLibrary();

    if (SUCCESS != (status = StageWorkspace(context)))
        return ReleaseDmsLibrary();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::StageInputFile(FwkContext& context)
    {
    m_dmsSupport->_Initialize();
    m_dmsSupport->SetApplicationResourcePath(m_dmsServerArgs.m_applicationWorkspace);

    BentleyStatus status = BentleyStatus::SUCCESS;
    if (!m_dmsSupport->_StageInputFile(m_jobEnvArgs.m_inputFileName))
        {
        context.m_error.m_message = "Error communicating with projectwise.";
        context.m_error.m_id = iModelBridgeErrorId::ProjectwiseError;
        }

    m_dmsSupport->_UnInitialize();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            iModelBridgeFwk::DmsServerArgs::SetDgnArg(WString argName, WStringCR argValue, bvector<WCharCP>& bargptrs)
    {
    argName.append(argValue.c_str());
    m_bargs.push_back(argName);  // Keep the string alive
    bargptrs.push_back(argName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeFwk::StageWorkspace(FwkContext& context)
    {
    BentleyStatus status = BentleyStatus::SUCCESS;
    m_dmsSupport->_Initialize();
    m_dmsSupport->SetApplicationResourcePath(m_dmsServerArgs.m_applicationWorkspace);
    bvector <WString> filePatterns = m_dmsServerArgs.m_additionalFilePatterns;
    //! Allow the bridges to inject additional file patterns
    if (NULL != m_bridge)
        filePatterns.insert(filePatterns.end(), m_bridge->GetParamsCR().GetAdditionalFilePattern().begin(), m_bridge->GetParamsCR().GetAdditionalFilePattern().end());

    if (!m_dmsServerArgs.m_inputFileUrn.empty())
        {
        BeFileName workspaceCfgFile;
        if (SUCCESS == m_dmsSupport->_FetchWorkspace(workspaceCfgFile, m_dmsServerArgs.m_inputFileUrn, m_dmsServerArgs.m_workspaceDir, m_dmsServerArgs.m_isv8i, filePatterns))
            m_dmsServerArgs.SetDgnArg(L"--DGN_CFGFILE=", workspaceCfgFile, m_bargptrs);
        }
    else
        {
        BeFileName workspaceCfgFile;
        if (SUCCESS == m_dmsSupport->_FetchWorkspace(workspaceCfgFile, m_dmsServerArgs.m_folderId, m_dmsServerArgs.m_documentId, m_dmsServerArgs.m_workspaceDir, m_dmsServerArgs.m_isv8i, filePatterns))
            m_dmsServerArgs.SetDgnArg(L"--DGN_CFGFILE=", workspaceCfgFile, m_bargptrs);
        }
    m_dmsSupport->_UnInitialize();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::DmsServerArgs::PrintUsage()
    {
    fwprintf (stderr,
    L"DMS :\n\
    --dms-library=          (optional)  The full path to the dms library. eg. iModelDmsSupportB02.dll Use this for direct Dms support from the framework.\n\
    --dms-workspaceDir=     (optional)  Directory to cache workspace files.\n\
    --dms-user=             (optional)  The username for the dms source.\n\
    --dms-password=         (optional)  The password for the dms source.\n\
    --dms-inputFileUrn=     (optional)  The urn to fetch the input file. This and associated workspace will be downloaded and stored in the location specified by\
    --fwk-input and --dms-workspaceDir= eg.pw://server:datasource/Documents/D{c6cbe438-e200-4567-a98c-dfa55aba33be}\n\
    --dms-datasource=       (optional)  The datasource to fetch the files from. Explicit usage instead of URN\n\
    --dms-folderId=         (optional)  FolderId if inputFileUrn is not specified. Explicit usage instead of URN\n\
    --dms-documentId=       (optional)  DocumentId if inputFileUrn is not specified. Explicit usage instead of URN\n\
    --dms-appWorkspace=     (optional)  Reference application workspace if the default fallback is not usable.\n\
    --dms-retries=          (optional)  The number of times to retry\n\
    --dms-type=             (optional)  Supports the following values 1 ( PW (default)), 2 (PWShare), 3 (Azure SAS Url)\n\
    --dms-additionalFiles=  (optional)  Applicable only for PW. Supports staging files specified by the pattern. (eg *.xml ). Can be specified more than once. \n\
    --dms-documentGuid=     (optional)  Document identifier in the system. In case of PW this is obtained from inputFileUrn"

    );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DmsServerArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[], bool isEncrypted)
    {
    WString savedWorkspace, savedCfgFile;
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--DGN_V8I"))
            m_isv8i = true;

        if (argv[iArg] == wcsstr(argv[iArg], L"--DGN_WORKSPACE="))//If a workspace was explicitly specified use it.
            {
            savedWorkspace = getArgValueW(argv[iArg]);
            continue;           //Override the dgnworkspace to the staged workspace location
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--DGN_CFGFILE="))
            {
            savedCfgFile = getArgValueW(argv[iArg]);
            continue;           //Override the dgnworkspace to the staged workspace location
            }

        //DGN_WORKSPACE=
        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--dms", 5))
            {
            // Not a fwk argument. We will forward it to the bridge.
            m_bargs.push_back(argv[iArg]);  // Keep the string alive
            bargptrs.push_back(m_bargs.back().c_str());
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-retries="))
            {
            int n = atoi(getArgValue(argv[iArg]).c_str());
            if (n < 0 || 256 <= n)
                {
                BeAssert(false);
                fprintf(stderr, "%s - invalid retries value. Must be a value between 0 and 255\n", getArgValue(argv[iArg]).c_str());
                return BSIERROR;
                }
            m_maxRetryCount = (uint8_t) n;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-datasource="))
            {
            m_dataSource = getArgValueW(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-user="))
            {
            m_dmsCredentials.SetUsername(getArgValue(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-password="))
            {
            m_dmsCredentials.SetPassword(getArgValue(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-folderId="))
            {
            m_folderId = atoi(getArgValue(argv[iArg]).c_str());
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-documentId="))
            {
            m_documentId = atoi(getArgValue(argv[iArg]).c_str());
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-inputFileUrn="))
            {
            if (!m_inputFileUrn.empty())
                {
                BeAssert(false);
                fwprintf(stderr, L"The --dms-inputFileUrn= option may appear only once.\n");
                return BSIERROR;
                }
            m_inputFileUrn = getArgValueW(argv[iArg]);
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-workspaceDir="))
            {
            m_workspaceDir = BeFileName(WString(getArgValueW(argv[iArg])));
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-appWorkspace="))
            {
            m_applicationWorkspace = BeFileName(WString(getArgValueW(argv[iArg])));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-library"))
            {
            if (!m_dmsLibraryName.empty())
                {
                BeAssert(false);
                fwprintf(stderr, L"The --dms-library= option may appear only once.\n");
                return BSIERROR;
                }

            m_dmsLibraryName.SetName(getArgValueW(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-type="))
            {
            m_dmsType = static_cast<iModelDmsSupport::SessionType>(atoi(getArgValue(argv[iArg]).c_str()));
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-documentGuid="))
            {
            m_documentGuid = getArgValue(argv[iArg]);
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--dms-additionalFiles="))
            {
            m_additionalFilePatterns.push_back(WString(getArgValueW(argv[iArg])));
            continue;
            }
        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized server argument\n", argv[iArg]);
        return BSIERROR;
        }

    if (m_dmsLibraryName.empty())
        {
        if (!savedWorkspace.empty())
            SetDgnArg(L"--DGN_WORKSPACE=", savedWorkspace, bargptrs);
        if (!savedCfgFile.empty())
            SetDgnArg(L"--DGN_CFGFILE=", savedCfgFile, bargptrs);
        return BSISUCCESS;
        }
    //--DGN_WORKSPACE=
    if (!m_applicationWorkspace.empty())
        {
        SetDgnArg(L"--DGN_WORKSPACE=", m_applicationWorkspace, bargptrs);
     //   WString workspaceArg(L"--DGN_WORKSPACE=");
     //   workspaceArg.append(m_applicationWorkspace.c_str());
     //   m_bargs.push_back(workspaceArg);  // Keep the string alive
     //   bargptrs.push_back(workspaceArg.c_str());
        }
    
    if (isEncrypted)
        DecryptCredentials(m_dmsCredentials);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::DmsServerArgs::Validate(int argc, WCharCP argv[])
    {
    if (m_dmsLibraryName.empty())
        {
        GetLogger().trace("Ignoring DMS staging since no supporting library is specified.");
        return SUCCESS;//Do nothing if dms support is not needed.
        }

    if (m_dmsType == iModelDmsSupport::SessionType::PWDI)
        {
        if (!m_workspaceDir.DoesPathExist())
            {
            GetLogger().fatal("Workspace directory provided is invalid.");
            return BSIERROR;
            }
        }

    if (!m_inputFileUrn.empty())
        return SUCCESS;

    if (m_dmsType == iModelDmsSupport::SessionType::PWDI)
        {
        if (m_folderId < 1 || m_documentId < 1 || m_dataSource.empty())
            {
            GetLogger().fatal("FolderId, documentId and datasource needs to be provided or PW URN path should be specified.");
            return BSIERROR;
            }
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeFwk::DmsServerArgs::DmsServerArgs()
    :m_folderId(0), m_documentId(0), m_isv8i(false), m_dmsType(iModelDmsSupport::SessionType::PWDI)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String iModelBridgeFwk::DmsServerArgs::GetDocumentGuid()
    {
    if (!m_documentGuid.empty())
        return m_documentGuid;

    if (m_inputFileUrn.empty())
        return Utf8String();
    std::string fileStr(Utf8String(m_inputFileUrn.c_str()).c_str());
    std::regex guidStr("([0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12})", std::regex::ECMAScript | std::regex::icase);
    std::smatch match;
    if (!std::regex_search(fileStr,match, guidStr))
        return Utf8String();

    return match[1].str().c_str();
    }