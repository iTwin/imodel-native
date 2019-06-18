/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include "DgnPlatform.h"
#include "Render.h"
#include "ColorUtil.h"
#include "TxnManager.h"
#include "DgnViewport.h"
#include <BeSQLite/L10N.h>
#include <BRepCore/SolidKernel.h>
#include <Logging/bentleylogging.h>

typedef struct _EXCEPTION_POINTERS*  LPEXCEPTION_POINTERS;
struct FT_LibraryRec_;

#define T_HOST DgnPlatformLib::GetHost()

DGNPLATFORM_TYPEDEFS(DgnHost)

BEGIN_BENTLEY_DGN_NAMESPACE

typedef struct ::FT_LibraryRec_* FreeType_LibraryP;

/*=================================================================================**//**
@addtogroup DgnPlatformHost

DgnPlatformLib defines interfaces that a "host" application must implement to use the DgnPlatform dlls.
To supply these services, a host must define a class that derives from DgnPlatformLib::Host.
The host may also define classes that derive from each of the admin base classes returned by the host.
The application must call DgnPlatformLib::Initialize once and only once before using any DgnPlatform services.

The application controls the lifetime of the DgnPlatformLib::Host object. The application must call DgnPlatformLib::Host::Terminate
before deleting the host object.

Beyond supplying services, the application-supplied DgnPlatformLib::Host object also holds context data such as fonts that are used by DgnPlatform functions.

The thread on which the application calls DgnPlatformLib::Initialize becomes the "client" thread. Many operations are only valid from the client thread.

* @bsiclass
+===============+===============+===============+===============+===============+======*/

//=======================================================================================
//! Class for initializing the DgnPlatform library.
//=======================================================================================
class DgnPlatformLib
{
public:
    //! The object that a host application must create and supply in order to use the DgnPlatform dlls
    struct EXPORT_VTABLE_ATTRIBUTE Host : DgnHost
    {
    private:
        void InitializeCrtHeap();
        void Initialize();
        void TerminateDgnCore(bool onProgramExit);

        friend class DgnPlatformLib;

    public:
        //! Provides Exception handling capabilities
        struct ExceptionHandler : IHostObject
            {
            //! Possible outcomes of handling an exception
            enum class WasHandled
                {
                ContinueSearch = 0,
                ExecuteHandler = 1,
                NonContinuable = 2,
                ContinueExecution = -1,
                };

            enum class TerminationCondition
                {
                None          = 0,
                AfterLogging  = 1,
                Immediate     = 2
                };

            //! Handle the specified exception
            virtual WasHandled _ProcessException(_EXCEPTION_POINTERS*) {return WasHandled::ContinueSearch;};
            virtual WasHandled _FilterException(_EXCEPTION_POINTERS*, bool onlyWantFloatingPoint) {return WasHandled::ContinueSearch;}
            //! Handle an assertion failure.
            virtual void _HandleAssert(WCharCP message, WCharCP file, unsigned line) {}
            virtual TerminationCondition _SetTerminationCondition(TerminationCondition when) {return when;}
            DGNPLATFORM_EXPORT virtual uint32_t _ResetFloatingPointExceptions(uint32_t newFpuMask);
            virtual uint32_t _QueryUser(bool attemptRecovery, Utf8CP productName) {return 0;}
            virtual uint32_t _EnterCoreCriticalSection(CharCP) {return 0;} // WIP_CHAR_OK - Just for diagnostic purposes
            virtual uint32_t _ReleaseCoreCriticalSection(CharCP) {return 0;} // WIP_CHAR_OK - Just for diagnostic purposes
            virtual void _RestoreCoreCriticalSection(CharCP, int) {} // WIP_CHAR_OK - Just for diagnostic purposes
            virtual bool _ConIOEnabled() {return false;}
            virtual WString _ConIOGetLine(wchar_t const* prompt) {return L"";}
            };

        //! Provides paths to known locations
        struct IKnownLocationsAdmin : IHostObject
        {
        protected:
            virtual ~IKnownLocationsAdmin() {}
            virtual BeFileNameCR _GetLocalTempDirectoryBaseName() = 0; //!< @see GetLocalTempDirectoryBaseName
            virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() = 0; //!< @see GetDgnPlatformAssetsDirectory
            virtual BeFileNameCR _GetGeoCoordinateDataDirectory() { return _GetDgnPlatformAssetsDirectory(); }

        public:
            //! Get the name of a local directory that can be used for creating temporary files. The directory must have
            //! write access. Subsystems generally create subdirectories in this directory for storing temporary data files.
            //! @return A string that holds the name of the base directory for temporary files. This is expected to always return the same object.
            DGNPLATFORM_EXPORT BeFileNameCR GetLocalTempDirectoryBaseName();

            //! Return a local directory that can be used to store temporary files. This directory can optionally be a subdirectory of #GetLocalTempDirectoryBaseName.
            //! @param[out] tempDir The name of temporary directory. This must be MAX_PATH chars in size.
            //! @param[in]  subDirName Optional subdirectory relative to default temp directory. If non-nullptr, this subdirectory will be created.
            //! @return nullptr if no temporary directory available.
            DGNPLATFORM_EXPORT BentleyStatus GetLocalTempDirectory(BeFileNameR tempDir, WCharCP subDirName);

            //! Return the directory containing the required DgnPlatform assets that must be deployed with any DgnPlatform-based app.
            //! Examples of required assets include fonts, ECSchemas, and localization resources.
            DGNPLATFORM_EXPORT BeFileNameCR GetDgnPlatformAssetsDirectory();

            //! Return the directory containing the required GeoCoordinateData that must be deployed with any DgnPlatform-based app.
            //! The only reason to use this method is when a PowerPlatform-based app is also using these libraries but wants to use the
            //! PowerPlatform based GeoCoordinateData and not the one from DgnClientSdk
            BeFileNameCR GetGeoCoordinateDataDirectory() { return _GetGeoCoordinateDataDirectory(); }

            //! Gets the directory that holds the sprite definition files.
            virtual StatusInt _GetSpriteContainer(BeFileNameR spritePath, Utf8CP spriteNamespace, Utf8CP spriteName) { return BSIERROR; }
        };


        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   07/13
        //=======================================================================================
        struct TxnAdmin : DgnHost::IHostObject
        {
            typedef bvector<TxnMonitorP> TxnMonitors;

        protected:
            TxnMonitors m_monitors;

        public:
            void CallMonitors(std::function<void (TxnMonitor&)>);
            virtual bool _OnPromptReverseAll() {return true;}
            virtual void _RestartTool() {}
            virtual void _OnNothingToUndo() {}
            virtual void _OnNothingToRedo() {}
            virtual void _OnGraphicElementAdded(DgnDbR, DgnElementId) {}
            virtual void _OnGraphicElementModified(DgnDbR, DgnElementId) {}
            virtual void _OnGraphicElementDeleted(DgnDbR, DgnElementId) {}
            virtual void _OnAppliedModelDelete(DgnModelR) {}
            DGNPLATFORM_EXPORT virtual void _OnPrepareForUndoRedo(TxnManager&);
            DGNPLATFORM_EXPORT virtual void _OnCommit(TxnManager&);
            DGNPLATFORM_EXPORT virtual void _OnCommitted(TxnManager&);
            DGNPLATFORM_EXPORT virtual void _OnAppliedChanges(TxnManager&);
            DGNPLATFORM_EXPORT virtual void _OnUndoRedo(TxnManager&, TxnAction);

            //! @name Transaction Monitors
            //@{
            //! Add a TxnMonitor. The monitor will be notified of all transaction events until it is dropped.
            //! @param monitor a monitor to add
            DGNPLATFORM_EXPORT void AddTxnMonitor(TxnMonitor& monitor);

            //! Drop a TxnMonitor.
            //! @param monitor a monitor to drop
            DGNPLATFORM_EXPORT void DropTxnMonitor(TxnMonitor& monitor);
            //@}
        };

        //! Allows hosts to provide required information for DgnFontManager, and gives them callback entry points for its events.
        struct FontAdmin : IHostObject
        {
        protected:
            BeSQLite::DbP m_lastResortFontDb;
            DgnFonts* m_dbFonts;
            DgnFontPtr m_lastResortTTFont;
            DgnFontPtr m_lastResortRscFont;
            DgnFontPtr m_lastResortShxFont;
            DgnFontPtr m_decoratorFont;
            bool m_triedToLoadFTLibrary;
            bool m_suspended = false;
            FreeType_LibraryP m_ftLibrary;

            DGNPLATFORM_EXPORT virtual BeFileName _GetLastResortFontDbPath();
            DGNPLATFORM_EXPORT virtual BentleyStatus _EnsureLastResortFontDb();
            DGNPLATFORM_EXPORT virtual DgnFontPtr _CreateLastResortFont(DgnFontType);

            virtual DgnFontCR _GetLastResortTrueTypeFont() {return m_lastResortTTFont.IsValid() ? *m_lastResortTTFont : *(m_lastResortTTFont = _CreateLastResortFont(DgnFontType::TrueType));}
            virtual DgnFontCR _GetLastResortRscFont() {return m_lastResortRscFont.IsValid() ? *m_lastResortRscFont : *(m_lastResortRscFont = _CreateLastResortFont(DgnFontType::Rsc));}
            virtual DgnFontCR _GetLastResortShxFont() {return m_lastResortShxFont.IsValid() ? *m_lastResortShxFont : *(m_lastResortShxFont = _CreateLastResortFont(DgnFontType::Shx));}
            virtual DgnFontCR _GetAnyLastResortFont() {return _GetLastResortTrueTypeFont();}
            DGNPLATFORM_EXPORT virtual DgnFontCR _GetDecoratorFont();
            DGNPLATFORM_EXPORT virtual FreeType_LibraryP _GetFreeTypeLibrary();
        public:
            FontAdmin() : m_lastResortFontDb(nullptr), m_dbFonts(nullptr), m_triedToLoadFTLibrary(false), m_ftLibrary(nullptr) {}
            DGNPLATFORM_EXPORT virtual ~FontAdmin();

            DgnFontCR GetLastResortTrueTypeFont() {return _GetLastResortTrueTypeFont();}
            DgnFontCR GetLastResortRscFont() {return _GetLastResortRscFont();}
            DgnFontCR GetLastResortShxFont() {return _GetLastResortShxFont();}
            DgnFontCR GetAnyLastResortFont() {return _GetAnyLastResortFont();}
            DgnFontCR GetDecoratorFont() {return _GetDecoratorFont();}
            DGNPLATFORM_EXPORT virtual DgnFontCR _ResolveFont(DgnFontCP);
            DgnFontCR ResolveFont(DgnFontCP font) {return _ResolveFont(font);}
            virtual bool _IsUsingAnRtlLocale() {return false;}
            bool IsUsingAnRtlLocale() {return _IsUsingAnRtlLocale();}
            FreeType_LibraryP GetFreeTypeLibrary() {return _GetFreeTypeLibrary();}
            DGNPLATFORM_EXPORT void Suspend();
            DGNPLATFORM_EXPORT void Resume();

            void Initialize();
        };

        //! Allows interaction between the host and the LineStyleManager.
        struct LineStyleAdmin : IHostObject
            {
            //! Possible values for the first parameter of LineStyleAdmin::_GetSettingsValue()
            enum  Settings
                {
                //! Returning true from LineStyleManager::_GetSettingsValue (SETTINGS_ScaleViewportLineStyles) tells the LineStyleManager
                //! to scale viewport reference attachments linestyles.
                SETTINGS_ScaleViewportLineStyles = 1,
                };

            //! Allows the host to provide a semi-colon-delimited list of search patterns to search for RSC files (which can each have zero or more RSC line styles).
            //! The order of files in the list is important
            //! because line styles encountered in files earlier in the list hide line styles of the same name encountered in files later in the list.
            //! A search pattern in the list may be a single filename
            //! (e.g. c:/.../resource/lstyle.rsc), a directory with a wildcard (e.g. c:/.../resource/*.rsc), or simply a directory (*.* implied) (e.g. c:/.../resource/).
            //! By default, this will query the ConfigurationAdmin for MS_SYMBRSRC and MS_LINESTYLEPATH.
            DGNPLATFORM_EXPORT virtual bool _GetLocalLineStylePaths(WStringR);
            //! Allows the host to provide a semi-colon-delimited list of search patterns to search for LIN files.
            //! By default, this will query the ConfigurationAdmin for MS_LINFILELIST.
            DGNPLATFORM_EXPORT virtual bool _GetLocalLinFilePaths(WStringR);
            //! Notifies the host that the LineStyleManager has loaded the LsDgnFileMap for the specified design file.  No action is necessary.
            virtual void _LoadedNameMap(DgnDbR) {}
            //! Notifies the host that the LineStyleManager has loaded an entry to the name map for a design file.  No action is necessary.
            virtual void _AddedNameMapEntry(DgnDbP, int lineStyleId) {}
            //! Notifies the host that the LineStyleManager has removed an entry from the name map for a design file.  No action is necessary.
            virtual void _RemovedNameMapEntry(DgnDbP, int lineStyleId) {}
            //! Notifies the host that the LineStyleManager has changed an entry in the name map for a design file.  No action is necessary.
            virtual void _ChangedNameMapEntry(DgnDbP, int lineStyleId) {}
            //! Notifies the host that the LineStyleManager encountered bad line style data in the specified resource file.   No action is necessary.
            virtual void _ReportCorruptedNameMap(WCharCP filename) {}
            //! Queries for a setting.
            virtual bool _GetSettingsValue(LineStyleAdmin::Settings name, bool defaultValue) {return defaultValue;}
            };

        //! Admin for PointCloud services
        //! Supervise the processing of Point Cloud Attachments
        struct PointCloudAdmin : IHostObject
            {
            //! Publishing actions for point clouds in i-model.
            //! Publish_Exclude: exclude the point cloud from i-model.
            //! Publish_KeepOriginal: Keep the point cloud in the i-model as it is.
            //! Publish_ReduceSize: Reduce the point cloud size according to the value returned by _GetPublishingEmbedSize.
            enum PublishingAction
                {
                Publish_Exclude         = 0,
                Publish_KeepOriginal    = 1,
                Publish_ReduceSize      = 2,
                };
            //! Display type to use for point clouds
            //! DisplayQueryType_Progressive: Draw using progressive queries
            //! DisplayQueryType_Complete: Draw using complete queries
            enum DisplayQueryType
                {
                DisplayQueryType_Progressive = 0,
                DisplayQueryType_Complete    = 1,
                };

            //! Get the density used for dynamic display.
            //! @return the dynamic density (a value between 0 and 1).
            virtual float _GetDisplayDynamicDensity() const {return 0.10f;}

            //! Get the density used for point cloud display.
            //! @return the global density (a value between 0 and 1).
            virtual float _GetDisplayGlobalDensity() const {return 1.0f;}

            //! Get the action to be taken with a point cloud exported to an i-model.
            //! @return the publishing action.
            virtual PublishingAction _GetPublishingAction() const {return Publish_KeepOriginal;}

            //! Get the maximum size of a point cloud to be embedded in an i-model.
            //! If the point cloud is bigger than this value (in Mb), it is reduced to this value. This value is used
            //! only when the publishing action is Publish_ReduceSize.
            //! @return the publishing size (in Mb).
            virtual uint32_t _GetPublishingEmbedSize() const {return 10;}

            //! Get the display type to use for point clouds
            //! @return the display query type.
            virtual DisplayQueryType _GetDisplayQueryType() const {return DisplayQueryType_Progressive;}

            //! Copy the spatial reference from a point cloud file to a point cloud element
            //! @param[in]      eRef         The point cloud element.
            virtual BentleyStatus _SyncSpatialReferenceFromFile(DgnElementP eRef) {return BSIERROR; }

            //! returns whether we should automatically synchronize the spatial reference from the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceFromFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference to the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceToFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference from the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceFromFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference to the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceToFile() const {return false;}

            //! Create a portable file URI, to be used with _ResolveFileUri. The default behavior strips the path from fileName and keeps only the file name.
            //! The host application can override this method to handle particular schemes or create URIs for specific locations.
            //! @param[out]     fileUri         A portable URI. This should be a name relative to the Bim file or a name with a known scheme ("bim://", ...)
            //! @param[in]      fileName        File name. The host application can override this method to handle names with a specific scheme.
            //! @return SUCCESS if the fileUri could be created. ERROR otherwise.
            DGNPLATFORM_EXPORT virtual BentleyStatus _CreateFileUri(Utf8StringR fileUri, Utf8StringCR fileName) const;

            //! Resolve the URI defined by fileUri. The output fileName should define a full path that can be used by RasterFileModelHandler to open the raster file.
            //! The default behavior assumes that fileUri defines a path relative to the Bim file.
            //! The host application can override this method to resolve specific file schemes. The host application should call the default _ResolveFileUri
            //! implementation if its own implementation fails.
            //! @param[out]     fileName        Resolved file name.
            //! @param[in]      fileUri         File URI that needs to be resolved.
            //! @param[in]      db              The current DgnDb file
            //! @return SUCCESS if the URI was resolved. ERROR otherwise.
            DGNPLATFORM_EXPORT virtual BentleyStatus _ResolveFileUri(BeFileNameR fileName, Utf8StringCR fileUri, DgnDbCR db) const;
            };

        //! Supervises the processing of GeoCoordination
        struct GeoCoordinationAdmin : IHostObject
            {
            //! Allows the host to provide a path to coordinate system definition files.
            virtual BeFileName _GetDataDirectory() {return BeFileName();}
            virtual IGeoCoordinateServicesP _GetServices() const {return nullptr;}
            };

        //! Supplies functionality for coordinating multi-user DgnDb repositories
        struct RepositoryAdmin : IHostObject
            {
            //! Create an IBriefcaseManager for the specified DgnDb. The implementation must *also* allow the DgnDb's ConcurrencyControl to configure the new briefcasemanager.
            //! In general it is not necessary to override this. The default implementation creates:
            //!  - For a master or standalone DgnDb, a briefcase manager which unconditionally grants all requests
            //!  - For any other DgnDb, a briefcase manager which forwards all requests to the repository manager (i.e., a server)
            //!  In either case, this implementation calls db.GetConcurrencyControl()->_ConfigureBriefcaseManager(*newBriefcaseManager);
            DGNPLATFORM_EXPORT virtual IBriefcaseManagerPtr _CreateBriefcaseManager(DgnDbR db) const;

            //! Returns the repository manager for the specified DgnDb. Should be overridden by hosts which support server-hosted repositories
            virtual IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const {return nullptr;}

            //! Specifies options to be associated with requests made to briefcase manager originating from tools.
            //! If queryOnly is true, we are only querying availability of locks/codes; otherwise we are attempting to acquire locks/codes.
            virtual IBriefcaseManager::ResponseOptions _GetResponseOptions(bool queryOnly) const { return IBriefcaseManager::ResponseOptions::None; }

            //! Invoked for each response to a request originating from a tool, e.g. to enable user notification of denied requests.
            //! The response will include whatever details were specified by _GetResponseOptions()
            virtual void _OnResponse(IBriefcaseManager::Response const& response, Utf8CP operation) { }
            };

        typedef bvector<DgnDomain*> T_RegisteredDomains;

    protected:
        IKnownLocationsAdmin*   m_knownLocationsAdmin;
        ExceptionHandler*       m_exceptionHandler;
        DgnProgressMeterP       m_progressMeter;
        FontAdmin*              m_fontAdmin;
        LineStyleAdmin*         m_lineStyleAdmin;
        PointCloudAdmin*        m_pointCloudAdmin;
        GeoCoordinationAdmin*   m_geoCoordAdmin;
        TxnAdmin*               m_txnAdmin;
        RepositoryAdmin*        m_repositoryAdmin;
        Utf8String              m_productName;
        T_RegisteredDomains     m_registeredDomains;

    public:
        T_RegisteredDomains& RegisteredDomains() {return m_registeredDomains;}

        DGNPLATFORM_EXPORT virtual void _OnAssert(WCharCP _Message, WCharCP _File, unsigned _Line);

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() = 0;

        DGNPLATFORM_EXPORT virtual ExceptionHandler& _SupplyExceptionHandler();

        //! Supply the FontAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual FontAdmin& _SupplyFontAdmin();

        //! Supply the LineStyleAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual LineStyleAdmin& _SupplyLineStyleAdmin();

        //! This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual TxnAdmin& _SupplyTxnAdmin();

        //! Supply the PointCloudAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual PointCloudAdmin& _SupplyPointCloudAdmin();

        //! Supply the GeoCoordinationStateAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again..
        DGNPLATFORM_EXPORT virtual GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        //! Supply the RepositoryAdmin
        DGNPLATFORM_EXPORT virtual RepositoryAdmin& _SupplyRepositoryAdmin();

        //! Supply the product name to be used to describe the host.
        virtual void _SupplyProductName(Utf8StringR) = 0;

        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() = 0;

        virtual void _OnUndisplayedSetChanged(DgnDbR) {}

        //! Given the name of a 'gated' feature, return whether or not it is enabled. By default, returns false.
        DGNPLATFORM_EXPORT virtual bool _IsFeatureEnabled(Utf8CP featureName);

        Host()
            {
            m_knownLocationsAdmin = nullptr;
            m_exceptionHandler = nullptr;
            m_progressMeter = nullptr;
            m_fontAdmin = nullptr;
            m_lineStyleAdmin = nullptr;
            m_pointCloudAdmin = nullptr;
            m_geoCoordAdmin = nullptr;
            m_txnAdmin = nullptr;
            m_repositoryAdmin = nullptr;
            };

        virtual ~Host() {}

        IKnownLocationsAdmin&   GetIKnownLocationsAdmin()  {return *m_knownLocationsAdmin;}
        ExceptionHandler&       GetExceptionHandler()      {return *m_exceptionHandler;}
        FontAdmin&              GetFontAdmin()             {return *m_fontAdmin;}
        LineStyleAdmin&         GetLineStyleAdmin()        {return *m_lineStyleAdmin;}
        PointCloudAdmin&        GetPointCloudAdmin()       {return *m_pointCloudAdmin;}
        GeoCoordinationAdmin&   GetGeoCoordinationAdmin()  {return *m_geoCoordAdmin;}
        TxnAdmin&               GetTxnAdmin()              {return *m_txnAdmin;}
        RepositoryAdmin&        GetRepositoryAdmin()       {return *m_repositoryAdmin;}
        Utf8CP                  GetProductName()           {return m_productName.c_str();}
        DgnProgressMeterP GetProgressMeter() {return m_progressMeter;}
        void SetProgressMeter(DgnProgressMeterP meter) {m_progressMeter=meter;}

        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized() {return 0 != m_fontAdmin;}

        //! Terminate this Host.
        //! This method should be called when the application is done with this host. After this method is called, no further DgnPlatform methods can ever be called on this host again (including Initialize).
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        DGNPLATFORM_EXPORT void Terminate(bool onProgramExit);
        };

    static void StaticInitialize();

public:
    //! Must be called before DgnPlatform services can be used.
    //! @param host The host to associate with this process.
    DGNPLATFORM_EXPORT static void Initialize(Host& host);

    //! Query whether Initialize has been called.
    //! @return nullptr if not Host is associated with the current process. Otherwise, a pointer to the Host object.
    DGNPLATFORM_EXPORT static Host* QueryHost();

    //! Get the Host
    //! @return a reference to the Host object. WARNING: Do not call this function unless you know that there is a Host.
    DGNPLATFORM_EXPORT static Host& GetHost();

    //! Used by DgnDbFileIO to initialize logging for Graphite code.
    //! @param configFileName Optional. The name of the logging configuration file to parse. Pass nullptr for logging to the console with default severities.
    //! If configFileName is specified, then the log4cxx provider will be used. Note that this provider comes from log4cxx.dll, and both the Graphite and Vancouver
    //! code will use the same log4cxx.dll.
    //! @private
    DGNPLATFORM_EXPORT static void InitializeBentleyLogging(WCharCP configFileName);

    //! Forward assertion failures to the specified handler.
    DGNPLATFORM_EXPORT static void ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler*);
};

END_BENTLEY_DGN_NAMESPACE

