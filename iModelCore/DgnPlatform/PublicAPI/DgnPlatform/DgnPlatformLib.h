/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformLib.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include "DgnPlatform.h"
#include "Render.h"
#include "ColorUtil.h"
#include "NotificationManager.h"
#include "TxnManager.h"
#include "SolidKernel.h"
#include "DgnViewport.h"
#include <BeSQLite/L10N.h>
#include <Logging/bentleylogging.h>

typedef struct _EXCEPTION_POINTERS*  LPEXCEPTION_POINTERS;
struct FT_LibraryRec_;

#define T_HOST DgnPlatformLib::GetHost()

DGNPLATFORM_TYPEDEFS(DgnHost)

BEGIN_BENTLEY_DGN_NAMESPACE

typedef struct ::FT_LibraryRec_* FreeType_LibraryP; 

/*=================================================================================**//**
@addtogroup DgnPlatformHost
DgnPlatformLib defines interfaces that a "host" application can or must implement to use the DgnPlatform dlls.
To supply these services, a host must define a class that derives from DgnPlatformLib::Host.
The host may also define classes that derive from each of the admin base classes returned by the host.
The application must call DgnPlatformLib::Initialize before using any DgnPlatform services.

The application controls the lifetime of the DgnPlatformLib::Host object. The application must call DgnPlatformLib::Host::Terminate
before deleting a host object.

Beyond supplying services, the application-supplied DgnPlatformLib::Host object also holds context data such as fonts that are used by DgnPlatform functions.

\section HostsAndThreads Hosts and Threads

The application must associate a Host with a thread before using DgnPlatform in that thread. This rule applies to all threads, including the main thread.

Most functions in DgnPlatform are <em>not</em> thread-safe. DgnPlatform can safely be used by multiple threads concurrently,
provided that each thread is associated with its own DgnPlatformLib::Host. The Host object holds the context data and
owns the DgnDb that the thread can use. (Host sharing is discussed below.)

There are several options for associating a Host with a thread:
\li The simplest option is to create a Host and then call DgnPlatformLib::Initialize in a thread before calling any other DgnPlatform functions in that thread. The thread should call DgnPlatformLib::Host::Terminate before it exits.
\li You can move a Host from one thread to another. Initialize the Host in thread A. Then call call DgnPlatformLib::ForgetHost in thread A and DgnPlatformLib::AdoptHost in thread B. Thread B should call DgnPlatformLib::Host::Terminate before it exits.
\li A related option is to initialize a Host in one thread for use by another thread. In this case, thread A would call DgnPlatformLib::Intialize on a Host and pass false for the argument to adopt the host. Thread B would then call DgnPlatformLib::AdoptHost to take ownership of the host and should call DgnPlatformLib::Host::Terminate before it exits.

Note that a DgnDb may be used only in a thread that is associated with the Host that was used to open it. So, you cannot open a file in
one thread and then try to use it in another thread, unless both threads are associated with the same host.

Host Sharing. You can share a single host among multiple threads, but only if you are prepared to synchronize access to it.
For example, you could initialize a Host in thread A and then call AdoptHost in several other threads, passing them the same host.
Each thread would have to enter a critical section before calling DgnPlatform methods, to ensure
that none of the other threads could use the host concurrently. When all threads were finished, thread A (only) would call
DgnPlatformLib::Host::Terminate before it exits.

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
        void InitializeDgnCore();
        void TerminateDgnCore(bool onProgramExit);
        void InitializeDgnHandlers();
        void LoadResources();

        friend class DgnPlatformLib;

    public:
        //! Provides access to scripting services.
        //! This is a complete implementation of the Admin needed to establish a scripting environment and to set up and use the DgnScript.
        //! You may subclass ScriptAdmin if you want to add more thread-specific contexts to it.
        struct ScriptAdmin : IHostObject
            {
            enum class LoggingSeverity : uint32_t
                {
                // *** NB: These values must be the same as NativeLogging::SEVERITY, except that they are positive instead of negative
                Fatal   = 0, 
                Error   = 1, 
                Warning = 2, 
                Info    = 3, 
                Debug   = 4, 
                Trace   = 5
                };

            static NativeLogging::SEVERITY ToNativeLoggingSeverity(LoggingSeverity severity)
                {
                // *** NB: ScriptAdmin::LoggingSeverity must be the same as NativeLogging::SEVERITY, except that they are positive instead of negative
                return (NativeLogging::SEVERITY)(-(int32_t)severity); 
                }

            //! Interface to be implemented by helpers that can import optional script libraries into the DgnScriptContext
            struct ScriptLibraryImporter : IHostObject
                {
                //! Import the specified script
                virtual void _ImportScriptLibrary(BeJsContextR, Utf8CP libName) = 0;
                };

            //! Interface for handling errors reported by scripts or that prevent scripts from running
            struct ScriptNotificationHandler : IHostObject
                {
                //! Handle a script error
                enum class Category {ReportedByScript, ParseError, Exception, Other};
                DGNPLATFORM_EXPORT virtual void _HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details);
                DGNPLATFORM_EXPORT virtual void _HandleLogMessage(Utf8CP category, LoggingSeverity sev, Utf8CP msg);
                };

            BeJsEnvironmentP m_jsenv;
            BeJsContextP m_jsContext;
            bmap<Utf8String, bpair<ScriptLibraryImporter*,bool>> m_importers;
            ScriptNotificationHandler* m_notificationHandler;

            DGNPLATFORM_EXPORT ScriptAdmin();
            DGNPLATFORM_EXPORT ~ScriptAdmin();

            //! Provide the script environment needed to evaluate script expressions on the host's thread. 
            //! There can only be one BeJsEnvironment per thread ... and this is it!
            //! All BeJsContexts that run on this thread must use this BeJsEnvironment.
            DGNPLATFORM_EXPORT BeJsEnvironmentR GetBeJsEnvironment();

            //! Provide the BeJsContext to use when executing script that needs to use the Dgn script object model. 
            //! There can only be one DgnScript per thread ... and this is it!
            DGNPLATFORM_EXPORT BeJsContextR GetDgnScriptContext();

            //! Obtain the text of the specified script program.
            //! This base class implementation looks for the program by name in the specified DgnDb.
            //! @param[out] sText           The text of the script that was found in the library
            //! @param[out] stypeFound      The type of script actually found in the library
            //! @param[out] lastModifiedTime The last-modified-time of the script that was found.
            //! @param[in] db               The current DgnDb file
            //! @param[in] sName            Identifies the script in the library
            //! @param[in] stypePreferred   The type of script that the caller prefers, if there are multiple kinds stored for the specified name.
            //! @return non-zero if the JS program is not available from the library.
            DGNPLATFORM_EXPORT virtual DgnDbStatus _FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lastModifiedTime, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred);

            //! Generate an exception in JavaScript
            //! @param exname   The name of the exception to throw
            //! @param details  Information about the exception
            DGNPLATFORM_EXPORT virtual void _ThrowException(Utf8CP exname, Utf8CP details);

            //! Register the script error handler
            ScriptNotificationHandler* RegisterScriptNotificationHandler(ScriptNotificationHandler& h) {auto was  = m_notificationHandler; m_notificationHandler = &h; return was;}

            //! Handle a reported script error. Invokes the registered error handler.
            DGNPLATFORM_EXPORT void HandleScriptError (ScriptNotificationHandler::Category category, Utf8CP description, Utf8CP details);

            //! Handle a notification sent from the script. Invokes the registered handler.
            DGNPLATFORM_EXPORT void HandleLogMessage (Utf8CP category, LoggingSeverity sev, Utf8CP msg);

            //! Register to import a set of projections or other script classes into the DgnScript JsContext
            //! @param libName  the library's unique ID that will be requested by script client programs
            //! @param importer the importer that can install the specified library
            DGNPLATFORM_EXPORT void RegisterScriptLibraryImporter(Utf8CP libName, ScriptLibraryImporter& importer);

            //! Make sure that the specified script library is loaded
            //! @param libName  the library's unique ID that will be requested by script client programs
            DGNPLATFORM_EXPORT BentleyStatus ImportScriptLibrary(Utf8CP libName); 

            DGNPLATFORM_EXPORT void/*Json::Value*/ EvaluateScript(Utf8CP script);

            //! Clean up
            DGNPLATFORM_EXPORT void _OnHostTermination(bool px) override;
            };

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
            template <typename CALLER> void CallMonitors(CALLER const& caller);

        public:
            virtual bool _OnPromptReverseAll() {return true;}
            virtual void _RestartTool() {}
            virtual void _OnNothingToUndo() {}
            virtual void _OnPrepareForUndoRedo() {}
            virtual void _OnNothingToRedo() {}
            virtual void _OnGraphicsRemoved(Render::GraphicSet&) {}
            virtual void _OnGraphicElementAdded(DgnElementId) {}
            DGNPLATFORM_EXPORT virtual void _OnCommit(TxnManager&);
            DGNPLATFORM_EXPORT virtual void _OnReversedChanges(TxnManager&);
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
            bool m_isInitialized;
            BeSQLite::DbP m_lastResortFontDb;
            DgnFonts* m_dbFonts;
            DgnFontPtr m_lastResortTTFont;
            DgnFontPtr m_lastResortRscFont;
            DgnFontPtr m_lastResortShxFont;
            DgnFontPtr m_decoratorFont;
            bool m_triedToLoadFTLibrary;
            FreeType_LibraryP m_ftLibrary;
        
            DGNPLATFORM_EXPORT virtual BeFileName _GetLastResortFontDbPath();
            DGNPLATFORM_EXPORT virtual BentleyStatus _EnsureLastResortFontDb();
            DGNPLATFORM_EXPORT virtual DgnFontPtr _CreateLastResortFont(DgnFontType);
        
        public:
            FontAdmin() : m_isInitialized(false), m_lastResortFontDb(nullptr), m_dbFonts(nullptr), m_triedToLoadFTLibrary(false), m_ftLibrary(nullptr) {}
            DGNPLATFORM_EXPORT virtual ~FontAdmin();

            virtual DgnFontCR _GetLastResortTrueTypeFont() {return m_lastResortTTFont.IsValid() ? *m_lastResortTTFont : *(m_lastResortTTFont = _CreateLastResortFont(DgnFontType::TrueType));}
            DgnFontCR GetLastResortTrueTypeFont() {return _GetLastResortTrueTypeFont();}
            virtual DgnFontCR _GetLastResortRscFont() {return m_lastResortRscFont.IsValid() ? *m_lastResortRscFont : *(m_lastResortRscFont = _CreateLastResortFont(DgnFontType::Rsc));}
            DgnFontCR GetLastResortRscFont() {return _GetLastResortRscFont();}
            virtual DgnFontCR _GetLastResortShxFont() {return m_lastResortShxFont.IsValid() ? *m_lastResortShxFont : *(m_lastResortShxFont = _CreateLastResortFont(DgnFontType::Shx));}
            DgnFontCR GetLastResortShxFont() {return _GetLastResortShxFont();}
            virtual DgnFontCR _GetAnyLastResortFont() {return _GetLastResortTrueTypeFont();}
            DgnFontCR GetAnyLastResortFont() {return _GetAnyLastResortFont();}
            DGNPLATFORM_EXPORT virtual DgnFontCR _GetDecoratorFont();
            DgnFontCR GetDecoratorFont() {return _GetDecoratorFont();}
            DGNPLATFORM_EXPORT virtual DgnFontCR _ResolveFont(DgnFontCP);
            DgnFontCR ResolveFont(DgnFontCP font) {return _ResolveFont(font);}
            virtual bool _IsUsingAnRtlLocale() {return false;}
            bool IsUsingAnRtlLocale() {return _IsUsingAnRtlLocale();}
            DGNPLATFORM_EXPORT virtual FreeType_LibraryP _GetFreeTypeLibrary();
            FreeType_LibraryP GetFreeTypeLibrary() {return _GetFreeTypeLibrary();}
            DGNPLATFORM_EXPORT void Suspend();
            DGNPLATFORM_EXPORT void Resume();
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

        //! Supervise the processing of Raster Attachments
        struct RasterAttachmentAdmin : IHostObject
            {
            //Control if raster are displayed or not
            virtual bool _IsDisplayEnable() const {return true;}

            //Control if raster locate logic can locate raster by its interior or by its border only.
            virtual bool _IsIgnoreInterior() const {return false;}

            //! Resolve the file name that corresponds to the fileId. The fileId is provided by the application, which is responsible to map this fileId to
            //! a file that the application knows. Generally, the application will override this admin method.
            //! @param[out]     fileName        Resolved file name. 
            //! @param[in]      fileId          An id that can be mapped to a file
            //! @param[in]      db              The current DgnDb file
            //! @return SUCCESS if the name could be resolved. ERROR otherwise. 
            virtual BentleyStatus _ResolveFileName(BeFileNameR fileName, Utf8StringCR fileId, DgnDbCR db) const {return ERROR;}

            //! Create a standard fileId, to be used with _ResolveFileName.
            //! @param[out]     fileId          An id created from fullPath and basePath
            //! @param[in]      fullPath        Full path of the file (including name). 
            //! @param[in]      basePath        Base path, usually the path of the current DgnDb file. 
            //! @return SUCCESS if the fileId could be created. ERROR otherwise. 
            virtual BentleyStatus _CreateLocalFileId(Utf8StringR fileId, BeFileNameCR fullPath, BeFileNameCR basePath) const {return ERROR;}
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
            virtual BentleyStatus _SyncSpatialReferenceFromFile(DgnElementP eRef) {return ERROR; }

            //! returns whether we should automatically synchronize the spatial reference from the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceFromFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference to the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceToFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference from the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceFromFile() const {return false;}

            //! returns whether we should automatically synchronize the spatial reference to the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceToFile() const {return false;}

            //! Resolve the file name that corresponds to the fileId. The fileId is provided by the application, which is responsible to map this fileId to
            //! a file that the application knows. Generally, the application will override this admin method.
            //! @param[out]     fileName        Resolved file name. 
            //! @param[in]      fileId          An id that can be mapped to a file
            //! @param[in]      db              The current DgnDb file
            //! @return SUCCESS if the name could be resolved. ERROR otherwise. 
            virtual BentleyStatus _ResolveFileName(BeFileNameR fileName, Utf8StringCR fileId, DgnDbCR db) const {return ERROR;}

            //! Create a standard fileId, to be used with _ResolveFileName.
            //! @param[out]     fileId          An id created from fullPath and basePath
            //! @param[in]      fullPath        Full path of the file (including name). 
            //! @param[in]      basePath        Base path, usually the path of the current DgnDb file. 
            //! @return SUCCESS if the fileId could be created. ERROR otherwise. 
            virtual BentleyStatus _CreateLocalFileId(Utf8StringR fileId, BeFileNameCR fullPath, BeFileNameCR basePath) const {return ERROR;}
            };

        //! Receives messages sent to NotificationManager. Hosts can implement this interface to communicate issues to the user.
        struct NotificationAdmin : IHostObject
            {
            //! Implement this method to display messages from NotificationManager::OutputMessage.
            virtual StatusInt _OutputMessage(NotifyMessageDetails const&) {return SUCCESS;}

            //! Implement this method to open/display MessageBoxes from NotificationManager::OpenMessageBox.
            virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP, NotificationManager::MessageBoxIconType) {return NotificationManager::MESSAGEBOX_VALUE_Ok;}

            //! Implement this method to display a prompt from NotificationManager::OutputPrompt.
            virtual void _OutputPrompt(Utf8CP) {}

            //! Return true if you want SQLite to log errors. Should be used only for limited debugging purposes.
            virtual bool _GetLogSQLiteErrors() {return false;}

            //! MicroStation internal only.
            DGNPLATFORM_EXPORT static void ChangeAdmin(NotificationAdmin&);
            };

        //! Supervises the processing of GeoCoordination
        struct GeoCoordinationAdmin : IHostObject
            {
            //! Allows the host to provide a path to coordinate system definition files.
            virtual BeFileName _GetDataDirectory() {return BeFileName();}
            virtual IGeoCoordinateServicesP _GetServices() const {return nullptr;}
            };

        //! Formatter preferences for units, fields, etc
        struct FormatterAdmin : IHostObject
            {

            //! If true, display coordinates in DGN format(eg. 1:0 1/4); if false, display DWG format(eg. 1'-0 1/4").
            virtual bool    _AllowDgnCoordinateReadout() const {return true;}
            };

        //! Supplies functionality for coordinating multi-user DgnDb repositories
        struct RepositoryAdmin : IHostObject
            {
            //! Create an IBriefcaseManager for the specified DgnDb
            //! In general it is not necessary to override this. The default implementation creates:
            //!  - For a master or standalone DgnDb, a briefcase manager which unconditionally grants all requests
            //!  - For any other DgnDb, a briefcase manager which forwards all requests to the repository manager (i.e., a server)
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
        RasterAttachmentAdmin*  m_rasterAttachmentAdmin;
        PointCloudAdmin*        m_pointCloudAdmin;
        NotificationAdmin*      m_notificationAdmin;
        GeoCoordinationAdmin*   m_geoCoordAdmin;
        TxnAdmin*               m_txnAdmin;
        IACSManagerP            m_acsManager;
        FormatterAdmin*         m_formatterAdmin;
        ScriptAdmin*            m_scriptingAdmin;
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

        //! Supply the RasterAttachmentAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual RasterAttachmentAdmin& _SupplyRasterAttachmentAdmin();

        //! Supply the PointCloudAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual PointCloudAdmin& _SupplyPointCloudAdmin();

        //! Supply the NotificationAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual NotificationAdmin& _SupplyNotificationAdmin();

        //! Supply the GeoCoordinationStateAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again..
        DGNPLATFORM_EXPORT virtual GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        //! Supply the formatter admin
        DGNPLATFORM_EXPORT virtual FormatterAdmin& _SupplyFormatterAdmin();

        //! Supply the ScriptAdmin
        DGNPLATFORM_EXPORT virtual ScriptAdmin& _SupplyScriptingAdmin();

        //! Supply the RepositoryAdmin
        DGNPLATFORM_EXPORT virtual RepositoryAdmin& _SupplyRepositoryAdmin();

        //! Supply the product name to be used to describe the host.
        virtual void _SupplyProductName(Utf8StringR) = 0;

        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() = 0;

        Host()
            {
            m_knownLocationsAdmin = nullptr;
            m_exceptionHandler = nullptr;
            m_progressMeter = nullptr;
            m_fontAdmin = nullptr;
            m_lineStyleAdmin = nullptr;
            m_rasterAttachmentAdmin = nullptr;
            m_pointCloudAdmin = nullptr;
            m_notificationAdmin = nullptr;
            m_geoCoordAdmin = nullptr;
            m_txnAdmin = nullptr;
            m_acsManager = nullptr;
            m_formatterAdmin = nullptr;
            m_scriptingAdmin = nullptr;
            m_repositoryAdmin = nullptr;
            };

        virtual ~Host() {}

        IKnownLocationsAdmin&   GetIKnownLocationsAdmin() {return *m_knownLocationsAdmin;}
        ExceptionHandler&       GetExceptionHandler()     {return *m_exceptionHandler;}
        void                    SetProgressMeter(DgnProgressMeterP meter) {m_progressMeter=meter;}
        DgnProgressMeterP       GetProgressMeter()         {return m_progressMeter;}
        FontAdmin&              GetFontAdmin()             {return *m_fontAdmin;}
        LineStyleAdmin&         GetLineStyleAdmin()        {return *m_lineStyleAdmin;}
        RasterAttachmentAdmin&  GetRasterAttachmentAdmin() {return *m_rasterAttachmentAdmin;}
        PointCloudAdmin&        GetPointCloudAdmin()       {return *m_pointCloudAdmin;}
        NotificationAdmin&      GetNotificationAdmin()     {return *m_notificationAdmin;}
        GeoCoordinationAdmin&   GetGeoCoordinationAdmin()  {return *m_geoCoordAdmin;}
        TxnAdmin&               GetTxnAdmin()              {return *m_txnAdmin;}
        IACSManagerR            GetAcsManager()            {return *m_acsManager;}
        FormatterAdmin&         GetFormatterAdmin()        {return *m_formatterAdmin;}
        ScriptAdmin&            GetScriptAdmin()           {return *m_scriptingAdmin;}
        RepositoryAdmin&        GetRepositoryAdmin()       {return *m_repositoryAdmin;}
        Utf8CP                  GetProductName()           {return m_productName.c_str();}

        void ChangeNotificationAdmin(NotificationAdmin& newAdmin) {m_notificationAdmin = &newAdmin;}

        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized() {return 0 != m_fontAdmin;}

        //! Terminate this Host.
        //! This method should be called when the application is done with this host. After this method is called, no further DgnPlatform methods can ever be called on this host again (including Initialize).
        //! If other threads are sharing this host, be sure to terminate those threads first and/or call ForgetHost in them before calling Terminate on this host.
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        DGNPLATFORM_EXPORT void Terminate(bool onProgramExit);
        };

    static void StaticInitialize();

public:
    //! Must be called before DgnPlatform services can be used on a thread.
    //! Optionally associates the host with the curent thread. See \ref HostsAndThreads.
    //! @param host         The host to associate with this thread.
    //! @param loadResources You may pass false for this only if you know your application will never require any fonts or linestyles.
    //! @param adoptHost    Should \a host be associated with the current thread? If true, this function calls AdoptHost(host) for you. If false, this
    //!                     function initializes \a host but does not adopt it. This option is useful if you want to initialize a host for later use for use by another thread.
    //! See StaticInitialize, AdoptHost
    DGNPLATFORM_EXPORT static void Initialize(Host& host, bool loadResources, bool adoptHost=true);

    //! Associate a host with the current thread, allowing it to call DgnPlatform functions. The host must have been initialized already.
    //! This function can be used to move a Host between threads or to sharea Host among multiple threads. See \ref HostsAndThreads
    //! Note that, before you call DgnPlatformLib::Host::Terminate on a host object, you must call ForgetHost in any thread that is sharing it.
    DGNPLATFORM_EXPORT static void AdoptHost(Host&);

    //! Break the association between the curent thread and its host. See \ref HostsAndThreads
    DGNPLATFORM_EXPORT static void ForgetHost();

    //! Query if a Host is associated with the current thread
    //! @return nullptr if not Host is associated with the current thread. Otherwise, a pointer to the Host object.
    DGNPLATFORM_EXPORT static Host* QueryHost();

    //! Get the Host that associated with the current thread
    //! @return a reference to the Host object. WARNING: Do not call this function unless you know that there is a Host.
    DGNPLATFORM_EXPORT static Host& GetHost();

    //! Used by DgnDbFileIO to initialize logging for Graphite code.
    //! @param configFileName Optional. The name of the logging configuration file to parse. Pass nullptr for logging to the console with default severities.
    //! If configFileName is specified, then the log4cxx provider will be used. Note that this provider comes from log4cxx.dll, and both the Graphite and Vancouver
    //! code will use the same log4cxx.dll. 
    DGNPLATFORM_EXPORT static void InitializeBentleyLogging(WCharCP configFileName);

    //! Forward assertion failures to the specified handler.
    DGNPLATFORM_EXPORT static void ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler*);
};

END_BENTLEY_DGN_NAMESPACE

