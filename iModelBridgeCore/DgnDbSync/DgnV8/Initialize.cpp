/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Initialize.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <windows.h>

#include <VersionedDgnV8Api/PSolid/PSolidCore.h>
#include <VersionedDgnV8Api/PSolidAcisInterop/PSolidAcisInterop.h>
#include <VersionedDgnV8Api/DgnGeoCoord/DgnGeoCoord.h>
#include <VersionedDgnV8Api/DgnPlatform/Tools/MacroConfigurationAdmin.h>
#include <VersionedDgnV8Api/GeoCoord/GCSLibrary.h>
#include <VersionedDgnV8Api/Mstn/RealDWG/DwgPlatformHost.h>

#include <VersionedDgnV8Api/VisEdgesLib/VisEdgeslib.h>
#include <VersionedDgnV8Api/DgnPlatform/CveHandler.h>


#include <ScalableMesh/ScalableMeshLib.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <Bentley/Desktop/FileSystem.h>
USING_NAMESPACE_BENTLEY_REALITYPLATFORM


BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

bvector<XDomain*> XDomainRegistry::s_xdomains;

struct ConverterV8Host;

//=========================================================================================
// @bsiclass                                                    Eric.Paquet         8/2016
//=========================================================================================
struct V8ViewManager : DgnV8Api::IViewManager
{
private:
    DgnV8Api::IndexedViewSet m_activeViewSet;

protected:
    virtual bool _DoesHostHaveFocus() override {return false;}
    virtual DgnV8Api::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override {return nullptr;} 
    virtual DgnV8Api::IndexedViewSet& _GetActiveViewSet() override {return m_activeViewSet;}
    virtual int _GetCurrentViewNumber(void) override {return 0;}
    virtual DgnV8Api::HUDManager* _GetHUDManager () override {return nullptr;}

public:
    V8ViewManager(ConverterV8Host& host);
};


/*=================================================================================**//**
* @bsiclass                                     Ray.Bentley                     02/2018
+===============+===============+===============+===============+===============+======*/
struct ConverterDgnAttachmentAdmin : DgnV8Api::DgnPlatformLib::Host::DgnAttachmentAdmin
{
    virtual StatusInt _ComputeCachedVisibleEdgeHash (CachedVisibleEdgeHashParamsR params, DgnAttachmentR dgnAttachment) override
        {
        // By default this was routed through UstnRefAdmin --- this is needed to  make sure that CVE caches are properly generated only when out of date.
        return VisibleEdgesLib::ComputeCurrentProxyHash (params.m_hash, params.m_newestElement, dgnAttachment, params.m_viewport, params.m_options, params.m_stopIfNewer, params.m_cryptographer);
        }
};





/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      07/14
+===============+===============+===============+===============+===============+======*/
struct ConverterV8Host : DgnV8Api::DgnViewLib::Host
    {
protected:
    struct Exceptions : ExceptionHandler
        {
        virtual void _HandleAssert(Bentley::WCharCP message, Bentley::WCharCP file, unsigned line) override
            {
            // forward the assertion failure to the Graphite assertion handler (which we override)
            BeAssertFunctions::PerformBeAssert(message, file, line);
            }
        };

    struct NulGraphics : GraphicsAdmin
        {
            QvCache* _CreateQvCache() override {return NULL;}
        };

    struct Notifications : NotificationAdmin
        {
        virtual StatusInt _OutputMessage(DgnV8Api::NotifyMessageDetails const& v8Details) override 
            {
            NotifyMessageDetails details(
                (OutputMessagePriority)v8Details.GetPriority(), 
                Utf8String(v8Details.GetBriefMsg().c_str()).c_str(), 
                Utf8String(v8Details.GetDetailedMsg().c_str()).c_str());
            return T_HOST.GetNotificationAdmin()._OutputMessage(details);
            }
        }; 

    struct Solids : DgnV8Api::PSolidKernelAdmin
        {
        DEFINE_T_SUPER (DgnV8Api::PSolidKernelAdmin)

        virtual Bentley::BentleyStatus _RestoreEntityFromMemory(DgnV8Api::ISolidKernelEntityPtr& entityOut, void const* pBuffer, uint32_t bufferSize, DgnV8Api::ISolidKernelEntity::SolidKernelType kernelType, Bentley::TransformCR transform) const override
            {
            if (DgnV8Api::ISolidKernelEntity::SolidKernel_PSolid == kernelType)
                return T_Super::_RestoreEntityFromMemory(entityOut, pBuffer, bufferSize, kernelType, transform);

            int         entityTag;
            Bentley::Transform   entityTransform;

            DgnV8Api::PSolidKernelManager::StartSession(); // NOTE: Make sure parasolid is started...

            if (Bentley::SUCCESS != DgnV8Api::PSolidAcisInterop::SATEntityToXMTEntity(entityTag, entityTransform, pBuffer, bufferSize, transform))
                return Bentley::ERROR;

            entityOut = DgnV8Api::PSolidKernelManager::CreateEntityPtr(entityTag, entityTransform);

            return Bentley::SUCCESS;
            }
        };

    // Overrides for DgnV8Api::DgnPlatformLib::Host
    NotificationAdmin&      _SupplyNotificationAdmin() override {return *new Notifications();}
    ExceptionHandler&       _SupplyExceptionHandler() override {return *new Exceptions();}
    SolidsKernelAdmin&      _SupplySolidsKernelAdmin() override {return *new Solids();}
    GeoCoordinationAdmin&   _SupplyGeoCoordinationAdmin() override {return *Bentley::GeoCoordinates::DgnGeoCoordinationAdmin::Create(NULL, *m_acsManager);} 
    RasterAttachmentAdmin&  _SupplyRasterAttachmentAdmin() override { return DgnV8Api::Raster::RasterCoreLib::GetDefaultRasterAttachmentAdmin(); }
    GraphicsAdmin&          _SupplyGraphicsAdmin() override {return *new NulGraphics;}
    DgnAttachmentAdmin&     _SupplyDgnAttachmentAdmin() override {return *new ConverterDgnAttachmentAdmin();}
    DgnDocumentManager&     _SupplyDocumentManager() override;

    // Overrides for DgnV8Api::DgnViewLib::Host
    virtual void                    _SupplyProductName (Bentley::WStringR name) override {name.assign(L"DgnV8Converter");}
    virtual DgnV8Api::IViewManager& _SupplyViewManager() override {return *new V8ViewManager(*this); }

    IDmsSupport*    m_dmsSupport;
    public:
    ConverterV8Host(IDmsSupport* dmsSupport)
        :m_dmsSupport(dmsSupport)
        {}
    }; 


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnDocumentManager& ConverterV8Host::_SupplyDocumentManager()
    {
    DgnV8Api::DgnDocumentManager* mgr = NULL;
    if (NULL != m_dmsSupport)
        mgr = m_dmsSupport->_GetDgnDocumentManager();
    
    if (NULL != mgr)
        return *mgr;
    
    return *new DgnV8Api::DgnDocumentManager();
    }
static DgnV8Api::MacroConfigurationAdmin* s_macros;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
V8ViewManager::V8ViewManager(ConverterV8Host& host)
    : m_activeViewSet(host)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8Api::IConfigurationAdmin& getConvertMacros()
    {
    return *s_macros;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  
+---------------+---------------+---------------+---------------+---------------+------*/
static void InitCustomGcsDir(int argc, WCharCP argv[])
    {    
    BentleyApi::BeFileName customGcsDir;

    Bentley::T_WStringVector assignmentArgs;    
    WChar                    tmpString[8*MAXFILELENGTH];    

    for ( ; argc > 0; argc--, argv++)
        {
        WCharCP  thisArg = *argv;
        if ( (*thisArg != '-') || (*(thisArg+1) != '-') )
            continue;

        thisArg = thisArg+2;

        BeStringUtilities::Wcsncpy (tmpString, _countof (tmpString), thisArg);
        BeStringUtilities::Wcsupr (tmpString);
               
        if (wcsncmp (tmpString, L"CUSTOMGCSDIR=", 13) == 0)
            {            
            customGcsDir.assign (thisArg + 13);
            customGcsDir.DropQuotes();
            customGcsDir.AppendSeparator(customGcsDir);

            break;            
            }                  
        }
  
    if (customGcsDir.size() > 0)
        {                              
        BentleyApi::BeDirectoryIterator directoryIter(customGcsDir);        

        bool       isDir;
        BeFileName gcsFileName; 
               
        while (directoryIter.GetCurrentEntry(gcsFileName, isDir) == SUCCESS)
            {                                                
            if (!isDir)
                {
                Bentley::GeoCoordinates::LibraryManager::Instance()->AddUserLibrary(gcsFileName.c_str(), nullptr);                                
                }            

            directoryIter.ToNext();
            }        
        }    
    }

//=========================================================================================
// @bsiclass                                                    Barry.Bentley       2/2017
//=========================================================================================
struct ConfigDebugOutput : DgnV8Api::IMacroDebugOutput
    {
    void  ShowDebugMessage (int indent, WCharCP format, ...) override
        {
        WString     message;
        va_list     ap;

        va_start (ap, format);
        message.VSprintf (format, ap);
        va_end (ap);

        int numSpaces = indent*2;

        for (int iSpace = 0; iSpace < numSpaces; iSpace++)
            wprintf (L" ");

        wprintf (L"%ls", message.c_str());
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  
+---------------+---------------+---------------+---------------+---------------+------*/
static void initializeV8HostConfigVars(Bentley::BeFileNameCR v8RootDir, int argc, WCharCP argv[])
    {
    // the command line argument tell us whether we're looking for CONNECT or V8i config files, and hopefully where to find them.
    // This function is called from main_needAlways after everything else is initialized (including the ConfigurationManager and Resource files)
    // If the particular product supports auto-conversion of V8i Configuration files, then we convert them to V8 CONNECT WorkSpace/WorkSet configuration files.
    WString workSpaceOrUser;
    WString workSetOrProject;
    WString configRootDir;
    WString installDir;
    WString msConfigFileName;
    Bentley::T_WStringVector assignmentArgs;

    bool    isV8i       = false;
    DgnV8Api::IMacroDebugOutput*  debugOutput = nullptr;
    int     debugLevel  = 1;

    for ( ; argc > 0; argc--, argv++)
        {
        WCharCP  thisArg = *argv;
        if ( (*thisArg != '-') || (*(thisArg+1) != '-') )
            continue;

        thisArg = thisArg+2;

        WString tmpString (thisArg);
        tmpString.ToUpper();
        tmpString.ReplaceAll(L"-", L"_");

        if ( 0 == (wcsncmp(tmpString.c_str(), L"DGN_WORKSET=", 12)) || (0 == (wcsncmp(tmpString.c_str(), L"DGN_PROJECT=", 12))) )
            {
            workSetOrProject.assign(thisArg+12);
            workSetOrProject.DropQuotes();
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_USER=", 8))
            {
            workSpaceOrUser.assign(thisArg+9);
            workSpaceOrUser.DropQuotes();
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_WORKSPACE=", 14))
            {
            workSpaceOrUser.assign(thisArg+14);
            workSpaceOrUser.DropQuotes();
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_CFGROOT=", 12))
            {
            // this command line option isn't documented in the "usage" info, and isn't working yet. 
            // It will require shipping fallback .cfg files and specifying the directory where they are shipped in the "fallback" variables below.
            configRootDir.assign(thisArg+12);
            configRootDir.DropQuotes();
            BeFileName::AppendSeparator(configRootDir);
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_CFGFILE=", 12))
            {
            // this command line option isn't documented in the "usage"info. The equivalent option is available as "WS" in MicroStation, and thus might be needed.
            msConfigFileName.assign(thisArg+12);
            msConfigFileName.DropQuotes();
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_INSTALL=", 12))
            {
            installDir.assign(thisArg + 12);
            installDir.DropQuotes();
            BeFileName::AppendSeparator(installDir);
            }
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_V8I", 7))
            {
            // we could infer this from using "DGN-USER" or "DGN-PROJECT" command line arguments, but at this point we don't.
            isV8i = true;
            }

        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_CFGVAR=", 7))
            {
            // remaining string must have the format MACRO=value 
            WChar    definitionString[1024];
            // recopy because we don't want strupr version.
            BeStringUtilities::Wcsncpy(definitionString, _countof(definitionString), thisArg+7);
            if (nullptr != wcschr(definitionString, '='))
                assignmentArgs.push_back(definitionString);
            }            
        else if (0 == wcsncmp(tmpString.c_str(), L"DGN_DEBUGCFG", 8))
            {
            if ( (tmpString.length()) > 9 && ('=' == tmpString[8]) )
                swscanf (tmpString.c_str() + 9, L"%i", &debugLevel);
            debugOutput = new ConfigDebugOutput();
            }

        }

    // if we didn't find any of our command line arguments, then we don't try to use any configuration files, and don't install the MacroConfigurationAdmin ConfigurationAdmin
    // NOTE: Need to revisit this if we ship fallback configuration files.
    if (!configRootDir.empty() || !installDir.empty() ||!msConfigFileName.empty())
        {
        s_macros = new DgnV8Api::MacroConfigurationAdmin;
        DgnV8Api::ConfigurationManager::SetGetAdminFunc(getConvertMacros);  // *** TRICKY: Our startup code, including LoadMacros below, makes calls on ConfigurationManager, but it isn't set up yet,
                                                                             // so Sam implemented this scheme to get it while it's used. It's not really right, but it works. See comment in MicroStation's msmacro.cpp

        // If we want to operate without  MicroStation installation we have to ship fallback configuration files for both V8i and CONNECT with the DgnV8 converter. We find those relative to this dll.
        BeFileName exeDir = Desktop::FileSystem::GetExecutableDir();
        BeFileName  fallbackV8iConfigDir(exeDir);
        fallbackV8iConfigDir.AppendToPath(L"Dgnv8\\v8iConfig");
        BeFileName  fallbackConnectConfigDir(exeDir);
        fallbackConnectConfigDir.AppendToPath(L"Dgnv8\\CEconfig");

        // if enough information is supplied, read either CONNECT or V8i configuration files to define the macros.
        if (!isV8i)
            {
            s_macros->ReadCONNECTConfigurationFiles(workSpaceOrUser.c_str(), workSetOrProject.c_str(), configRootDir.c_str(), installDir.c_str(), msConfigFileName.c_str(), fallbackConnectConfigDir.c_str(), assignmentArgs, debugOutput, debugLevel);
            }
        else
            {
            s_macros->ReadV8iConfigurationFiles(workSpaceOrUser.c_str(), workSetOrProject.c_str(), configRootDir.c_str(), installDir.c_str(), msConfigFileName.c_str(), fallbackV8iConfigDir.c_str(), assignmentArgs, debugOutput, debugLevel);
            }
        }
    if (nullptr != debugOutput)
        delete debugOutput;

    Bentley::WString cfgVarValue;

    // Setting MS_FONTCONFIGFILE means we don't have to provide our own DgnV8 FontAdmin just to set the paths.
    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_FONTCONFIGFILE"))
        {
        Bentley::BeFileName cfgFileName(v8RootDir);
        cfgFileName.AppendToPath(L"Fonts");
        cfgFileName.AppendToPath(L"MstnFontConfig.xml");
        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_FONTCONFIGFILE", cfgFileName);
        }

    // Setting MS_FONTPATH means we don't have to provide our own DgnV8 FontAdmin just to set the paths.
    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_FONTPATH"))
        {
        Bentley::BeFileName fontPath(v8RootDir);
        fontPath.AppendToPath(L"Fonts");
        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_FONTPATH", fontPath);
        }

    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_SYMB"))
        {
        Bentley::BeFileName lineStylePath(v8RootDir);
        lineStylePath.AppendToPath(L"Symb");
        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_SYMB", lineStylePath);
        }

    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_SEEDFILES"))
        {
        Bentley::BeFileName seedFilesPath(v8RootDir);
        seedFilesPath.AppendToPath(L"Seed");
        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_SEEDFILES", seedFilesPath);        
        }    
    
    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_TRANSSEED"))
        {
        Bentley::BeFileName transeed(v8RootDir);
        transeed.AppendToPath(L"Seed\\seed3d.dgn");
        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_TRANSEED", transeed);
        }    
    
    // If MS_SMARTSOLID is not defined, Vancouver will look for a "schema" sub-directory next to the process's EXE to provide ParaSolid with its schema files.
    // In DgnDb, we place ParaSolid schemas in a "PSolidSchemas" sub-directory for clarity, so we need to inject a better path.
    // Further, might as well ensure V8 uses the schema from the V8 delivery, and not ours.
    if (SUCCESS != DgnV8Api::ConfigurationManager::GetVariable(cfgVarValue, L"MS_SMARTSOLID"))
        {
        WString smartSolidDir = v8RootDir;
        
        // Vancouver uses wcscat, not AppendToPath, so must ensure this ends in a slash.
        if (!smartSolidDir.EndsWith(WCSDIR_SEPARATOR))
            smartSolidDir += WCSDIR_SEPARATOR;

        DgnV8Api::ConfigurationManager::DefineVariable(L"MS_SMARTSOLID", smartSolidDir.c_str());
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Ramanujam.Raman                      11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SetDllSearchPath(BentleyApi::BeFileNameCR v8Path, BentleyApi::BeFileNameCP realdwgPath)
    {
    /*
    * Note: We use two mechanisms to setup the PATH - it otherwise results in hard to find and/or reproduce bugs 
    * with the converter:   
    *
    * 1. ::SetDllDirectoryW() by itself was unreliable on some machines - ::SetDllDirectoryA() was getting called 
    * from other Dlls not under our control clobbering up the DgnV8 path setup here. So we ended up setting 
    * system PATH environment for these cases (as is done in V8 Microstation)
    * 
    * 2. System PATH by itself wasn't sufficient to load some DLLs like SPAXAcis.dll. I couldn't find a reasonable 
    * explanation for why this is the case. 
    * 
    * See description @ https://msdn.microsoft.com/en-us/library/ms686203(VS.85).aspx
    */
    ::SetDllDirectoryW(v8Path.c_str());

    WString newPath;
    newPath = L"PATH=" + v8Path + L";";

    if (nullptr != realdwgPath)
        newPath += *realdwgPath + L";";

    newPath.append(::_wgetenv(L"PATH"));
    _wputenv(newPath.c_str());
    }

DGNV8_ELEMENTHANDLER_DEFINE_MEMBERS(ThreeMxElementHandler)
DGNV8_ELEMENTHANDLER_DEFINE_MEMBERS(ScalableMeshElementHandler)

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct     DomainInitCaller: public DgnV8Api::IEnumerateAvailableHandlers
    {
    virtual StatusInt _ProcessHandler(DgnV8Api::Handler& handler)
        {
        ConvertToDgnDbElementExtension* extension = ConvertToDgnDbElementExtension::Cast(handler);
        if (NULL == extension)
            return SUCCESS;
        extension->_InitDgnDomain();
        return SUCCESS;
        }
    };

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct ConverterV8Txn : DgnV8Api::DgnCacheTxn
    {
    void _CallModelChangeMonitors (Bentley::DgnModelP, Bentley::ModelInfoCP, DgnV8Api::ChangeTrackAction, bool) override {;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct  SMHost : ScalableMesh::ScalableMeshLib::Host
    {
    SMHost()
        {
        }

    ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        struct CsScalableMeshAdmin : public ScalableMesh::ScalableMeshAdmin
            {
            virtual IScalableMeshTextureGeneratorPtr _GetTextureGenerator() override
                {
                IScalableMeshTextureGeneratorPtr generator;
                return generator;
                }

            virtual bool _CanImportPODfile() const override
                {
                return false;
                }

            virtual Utf8String _GetProjectID() const override
                {             
                Utf8String projectGUID("95b8160c-8df9-437b-a9bf-22ad01fecc6b");

                Bentley::WString projectGUIDw;

                if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(projectGUIDw, L"SM_PROJECT_GUID"))
                    {
                    projectGUID.Assign(projectGUIDw.c_str());
                    }
                return projectGUID;
                }

            virtual uint64_t _GetProductId() const override
                {
                return 1; //Default product Id for internal product
                }            
            };
        return *new CsScalableMeshAdmin;
        };

    };

// =====================================================================================
// Stand-in for the CifSheetExaggeratedViewHandler that is defined in Vancouver.
// =====================================================================================
struct CifSheetExaggeratedViewHandlerStandin : DgnV8Api::ViewHandler
    {
    bool _GetAspectRatioSkew (DynamicViewSettingsCR viewSettings, double& aspectRatio) const override
        {
        int dataSize;
        double* data;
        DgnV8Api::XAttributeHandlerId hid = DgnV8Api::XAttributeHandlerId(BENTLEY_CIF_XATTRIBUTE_ID, DgnV8Api::CIF::XATTRIBUTES_SUBID_EXAGGERATEDVIEWPROPERTIES);

        if (NULL == (data = (double*) (viewSettings.GetXAttributesHolderCR().GetXAttribute(&dataSize, hid, 0))) || dataSize != sizeof(double))
            aspectRatio = 1;
        else
            aspectRatio = *data;
        return true;
        }

    static void Register()
        {
        DgnV8Api::XAttributeHandlerId hid = DgnV8Api::XAttributeHandlerId(BENTLEY_CIF_XATTRIBUTE_ID, DgnV8Api::CIF::XATTRIBUTES_SUBID_EXAGGERATEDSHEETVIEWHANDLER);
        DgnV8Api::XAttributeHandlerManager::RegisterHandler(hid, new CifSheetExaggeratedViewHandlerStandin());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::Initialize(BentleyApi::BeFileNameCR bridgeLibraryDir, BentleyApi::BeFileNameCR bridgeAssetsDir, BentleyApi::BeFileNameCR v8DllsRelativeDir, 
                           BentleyApi::BeFileNameCP realdwgAbsoluteDir, bool isPowerPlatformBased, int argc, WCharCP argv[], BentleyApi::Dgn::IDmsSupport* dmsSupport)
    {
    if (!isPowerPlatformBased)
        {
        BentleyApi::BeFileName dllDirectory(bridgeLibraryDir);
        dllDirectory.AppendToPath(v8DllsRelativeDir);

        BentleyApi::BeFileName realdwgDirectory;
        if (nullptr != realdwgAbsoluteDir)
            {
            // the caller product has installed RealDWG, use it:
            realdwgDirectory.SetName (realdwgAbsoluteDir->c_str());
            }
        else
            {
            // use the default RealDWG that is a part of the V8SDK:
            realdwgDirectory.SetName (dllDirectory.c_str());
            realdwgDirectory.AppendToPath (L"RealDwg");
            }
        SetDllSearchPath(dllDirectory, &realdwgDirectory);

        Bentley::BeFileName dllDirectoryV8(dllDirectory.c_str());
        initializeV8HostConfigVars(dllDirectoryV8, argc, argv);

        DgnV8Api::DgnViewLib::Initialize (*new ConverterV8Host(dmsSupport), true);
        DgnV8Api::Raster::RasterCoreLib::Initialize (*new DgnV8Api::Raster::RasterCoreLib::Host());    

        DgnV8Api::ITxnManager::GetManager().SetCurrentTxn(*new ConverterV8Txn);

        DgnV8Api::DependencyManager::SetTrackingDisabled(true);
        DgnV8Api::DependencyManager::SetProcessingDisabled(true);

        // We need a V8 handler for ThreeMx attachment elements. That handler is in an MDL app in MicroStation. We don't need it to do anythning, but 
        // we need it to exist so we can put the "ToDgnDbExtension" on it.
        DgnV8Api::ElementHandlerManager::RegisterHandler(DgnV8Api::ElementHandlerId(ThreeMxElementHandler::XATTRIBUTEID_ThreeMxAttachment, 0), ThreeMxElementHandler::GetInstance());
        DgnV8Api::ElementHandlerManager::RegisterHandler(DgnV8Api::ElementHandlerId(ScalableMeshElementHandler::XATTRIBUTEID_ScalableMeshAttachment, 0), ScalableMeshElementHandler::GetInstance());

        Converter::RegisterForeignFileTypes (dllDirectory, realdwgDirectory);
        }
    // Directly register basic DgnV8 converter extensions here (that platform owns).
    // In the future, may need an extensibility point here to allow apps and/or arbitrary DLLs to participate in this process.
    ConvertV8TextToDgnDbExtension::Register();
    ConvertV8TagToDgnDbExtension::Register();
    ConvertV8Lights::Register();
    ConvertThreeMxAttachment::Register();
    ConvertScalableMeshAttachment::Register();
    ConvertDetailingSymbolExtension::Register();
    CifSheetExaggeratedViewHandlerStandin::Register();

    //Ensure tha V8i::DgnGeocoord is using the GCS library from this application admin.
    DgnV8Api::ConfigurationManager::UndefineVariable (L"MS_GEOCOORDINATE_DATA");
    DgnV8Api::ConfigurationManager::DefineVariable (L"MS_GEOCOORDINATE_DATA", T_HOST.GetGeoCoordinationAdmin()._GetDataDirectory().c_str());

    Bentley::GeoCoordinates::BaseGCS::Initialize(T_HOST.GetGeoCoordinationAdmin()._GetDataDirectory().c_str());
    InitCustomGcsDir(argc, argv);

    // Must register all domains as required, so that OpenDgnDb will import them. We are not allowed to import schemas later in the conversion process.
    //  Note that this bridge delivers the domains that it uses, and their schemas are in the bridge's assets directory, not the platform's assets directory.
    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    DgnDomains::RegisterDomain(Raster::RasterDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());

    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_RegisterDomain(bridgeAssetsDir);

    DomainInitCaller caller;
    DgnV8Api::ElementHandlerManager::EnumerateAvailableHandlers(caller);
    }

/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      07/14
+===============+===============+===============+===============+===============+======*/
struct MinimalV8Host : DgnV8Api::DgnPlatformLib::Host
    {
    };

/*=================================================================================**//**
* @bsiclass                                                     SamWilson       08/13
+===============+===============+===============+===============+===============+======*/
class SupplyBlankPassword : public DgnV8Api::DgnFileSupplyRights
    {
    StatusInt getLicense (DgnV8Api::DgnFileLicenseDef* pLic, bool* userCancel, const Byte*) override
        {
        pLic->keypw.InitFromHash (nullptr, (size_t)0);
        *userCancel = false;
        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void   Converter::InitializeDgnv8Platform(BentleyApi::BeFileName const& thisLibraryPath)
    {
    static std::once_flag s_initOnce;
    std::call_once(s_initOnce, [&]
    {
        BentleyApi::BeFileName dllDirectory(thisLibraryPath.GetDirectoryName());
        dllDirectory.AppendToPath(L"DgnV8");
        BentleyApi::BeFileName realdwgDirectory(dllDirectory);
        realdwgDirectory.AppendToPath(L"RealDwg");
        Converter::SetDllSearchPath(dllDirectory, &realdwgDirectory);

        initializeV8HostConfigVars(Bentley::BeFileName(dllDirectory.c_str()), 0, nullptr);
        DgnV8Api::DgnPlatformLib::Initialize(*new MinimalV8Host, true);
        Converter::RegisterForeignFileTypes (dllDirectory, realdwgDirectory);
    });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GetAuthoringFileInfo(WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel, BentleyApi::BeFileName const& sourceFileName)
    {
    //  The generic V8 bridge has an affinity to any file that V8 can open.

    DgnV8Api::DgnFileStatus openStatus;
    auto doc = DgnV8Api::DgnDocument::CreateFromFileName(openStatus, sourceFileName, nullptr, DEFDGNFILE_ID, DgnV8Api::DgnDocument::FetchMode::Read, DgnV8Api::DgnDocument::FetchOptions::Default);
    if (doc == nullptr)
        return BSIERROR;

    auto fullSpec = doc->GetMonikerPtr()->ResolveFileName();
    DgnFilePtr file = DgnV8Api::DgnFile::Create(*doc, DgnV8Api::DgnFileOpenMode::ReadOnly);

    if (!file.IsValid())
        {
        BeAssert(false);
        return BSIERROR; // ??
        }

    SupplyBlankPassword supplyBlankPw;
    DgnV8Api::DgnFileLoadContext v8LoadContext(&supplyBlankPw);

    openStatus = (DgnV8Api::DgnFileStatus) file->LoadFile(nullptr, &v8LoadContext, true);

    if (DgnV8Api::DGNFILE_STATUS_Success == openStatus && !file->HasDigitalRight(DgnV8Api::DgnFile::DIGITAL_RIGHT_Export))
        openStatus = DgnV8Api::DGNFILE_ERROR_RightNotGranted;

    if (SUCCESS != openStatus)
        return BSIERROR;
    
    Bentley::WString applicationName;
    if (SUCCESS == file->GetAuthoringProductName(applicationName))
        {
        BeStringUtilities::Wcsncpy(buffer, bufferSize, applicationName.c_str());
        affinityLevel = iModelBridge::Affinity::ExactMatch;
        return BSISUCCESS;
        }

#ifdef USEABDFILECHECKER
    if (dgnFile_isABDFile(file))
        {
        BeStringUtilities::Wcsncpy(buffer, bufferSize, L"AECOsimBuildingDesigner");
        affinityLevel = BentleyM0200::Dgn::iModelBridge::Affinity::ExactMatch;
        return BSISUCCESS;
        }
#endif

    affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::Low;
    BeStringUtilities::Wcsncpy(buffer, bufferSize, L"IModelBridgeForMstn");
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void XDomain::Register(XDomain& xd) 
    {
    XDomainRegistry::s_xdomains.push_back(&xd); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void XDomain::UnRegister(XDomain& xd) 
    {
    auto i = std::find(XDomainRegistry::s_xdomains.begin(), XDomainRegistry::s_xdomains.end(), &xd); 
    if (i != XDomainRegistry::s_xdomains.end())
        XDomainRegistry::s_xdomains.erase(i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            Converter::GetAffinity(WCharP buffer, const size_t bufferSize, iModelBridgeAffinityLevel& affinityLevel, WCharCP affinityLibraryPathStr, WCharCP sourceFileNameStr)
    {
    affinityLevel = BentleyM0200::Dgn::iModelBridge::Affinity::None;

    BentleyM0200::BeFileName affinityLibraryPath(affinityLibraryPathStr);
    InitializeDgnv8Platform(affinityLibraryPath);

    // if no file handler recognizes this file, don't bother loading it:
    DgnFileFormatType   format = DgnFileFormatType::Invalid;
    if (!Bentley::dgnFileObj_validateFile(&format, nullptr, nullptr, nullptr, nullptr, nullptr, sourceFileNameStr))
        return;

    // Foreign file formats do not have authoring file info - don't load them:
    if (format != DgnFileFormatType::V8 && format != DgnFileFormatType::V7)
        {
        affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::Low;
#ifdef USEABDFILECHECKER
        BeStringUtilities::Wcsncpy(buffer, bufferSize, L"AECOsimBuildingDesigner");
#else
        BeStringUtilities::Wcsncpy(buffer, bufferSize, L"IModelBridgeForMstn");
#endif
        return;
        }

    BentleyM0200::BeFileName sourceFileName(sourceFileNameStr);
    
    if (BSISUCCESS != Converter::GetAuthoringFileInfo(buffer, bufferSize, affinityLevel, sourceFileName))
        {
        //Log error.
        }
    }


END_DGNDBSYNC_DGNV8_NAMESPACE
