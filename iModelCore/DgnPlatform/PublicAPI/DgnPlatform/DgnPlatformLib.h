/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatformLib.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include "DgnPlatform.h"
#include "DgnCore/IViewOutput.h"
#include "DgnCore/ColorUtil.h"
#include "DgnCore/NotificationManager.h"
#include "DgnCore/TxnManager.h"
#include "DgnCore/SolidKernel.h"
#include "DgnCore/RealityDataCache.h"
#include "DgnCore/DgnViewport.h"
#include <BeSQLite/L10N.h>
#include "DgnCore/PointCloudBaseModel.h"

typedef struct _EXCEPTION_POINTERS*  LPEXCEPTION_POINTERS;
typedef struct FT_LibraryRec_* FT_Library; // Shield users from freetype.h because they have a bad include scheme.

#define T_HOST DgnPlatformLib::GetHost()

DGNPLATFORM_TYPEDEFS(DgnHost)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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

            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!

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
            virtual void   _RestoreCoreCriticalSection(CharCP, int) {} // WIP_CHAR_OK - Just for diagnostic purposes
            virtual void    _OnHostTermination(bool isProcessShutdown) {delete this;}
            virtual bool    _ConIOEnabled() {return false;}
            virtual WString _ConIOGetLine(wchar_t const* prompt) {return L"";}
            };

        //! Provides paths to known locations
        struct IKnownLocationsAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

        protected:
            virtual ~IKnownLocationsAdmin() {}
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}
            virtual BeFileNameCR _GetLocalTempDirectoryBaseName() = 0; //!< @see GetLocalTempDirectoryBaseName
            virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() = 0; //!< @see GetDgnPlatformAssetsDirectory

        public:
            //! Get the name of a local directory that can be used for creating temporary files. The directory must have
            //! write access. Subsystems generally create subdirectories in this directory for storing temporary data files.
            //! @return A string that holds the name of the base directory for temporary files. This is expected to always return the same object.
            DGNPLATFORM_EXPORT BeFileNameCR GetLocalTempDirectoryBaseName();

            //! Return a local directory that can be used to store temporary files. This directory can optionally be a subdirectory of #GetLocalTempDirectoryBaseName.
            //! @param[out] tempDir The name of temporary directory. This must be MAX_PATH chars in size.
            //! @param[in]  subDirName Optional subdirectory relative to default temp directory. If non-NULL, this subdirectory will be created.
            //! @return NULL if no temporary directory available.
            DGNPLATFORM_EXPORT BentleyStatus GetLocalTempDirectory(BeFileNameR tempDir, WCharCP subDirName);

            //! Return the directory containing the required DgnPlatform assets that must be deployed with any DgnPlatform-based app.
            //! Examples of required assets include fonts, ECSchemas, and localization resources.
            DGNPLATFORM_EXPORT BeFileNameCR GetDgnPlatformAssetsDirectory();
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
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}
            virtual bool _OnPromptReverseAll() {return true;}
            virtual void _RestartTool()  {}
            virtual void _OnNothingToUndo() {}
            virtual void _OnPrepareForUndoRedo(){}
            virtual void _OnNothingToRedo(){}

            DGNPLATFORM_EXPORT virtual void _OnTxnCommit(TxnSummaryCR);
            DGNPLATFORM_EXPORT virtual void _OnTxnReverse(TxnSummaryCR);
            DGNPLATFORM_EXPORT virtual void _OnTxnReversed(TxnSummaryCR);
            DGNPLATFORM_EXPORT virtual void _OnUndoRedoFinished(DgnDbR, TxnDirection);

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
            BeSQLiteDbP m_lastResortFontDb;
            DgnFonts* m_dbFonts;
            DgnFontPtr m_lastResortTTFont;
            DgnFontPtr m_lastResortRscFont;
            DgnFontPtr m_lastResortShxFont;
            bool m_triedToLoadFTLibrary;
            FT_Library m_ftLibrary;
        
            DGNPLATFORM_EXPORT virtual BeFileName _GetLastResortFontDbPath();
            DGNPLATFORM_EXPORT virtual BentleyStatus _EnsureLastResortFontDb();
            DGNPLATFORM_EXPORT virtual DgnFontPtr _CreateLastResortFont(DgnFontType);
        
        public:
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS;
            FontAdmin() : m_isInitialized(false), m_lastResortFontDb(nullptr), m_dbFonts(nullptr), m_triedToLoadFTLibrary(false), m_ftLibrary(nullptr) {}
            DGNPLATFORM_EXPORT virtual ~FontAdmin();
            virtual int _GetVersion() const { return 1; } // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override { delete this; }

            virtual DgnFontCR _GetLastResortTrueTypeFont() { return m_lastResortTTFont.IsValid() ? *m_lastResortTTFont : *(m_lastResortTTFont = _CreateLastResortFont(DgnFontType::TrueType)); }
            DgnFontCR GetLastResortTrueTypeFont() { return _GetLastResortTrueTypeFont(); }
            virtual DgnFontCR _GetLastResortRscFont() { return m_lastResortRscFont.IsValid() ? *m_lastResortRscFont : *(m_lastResortRscFont = _CreateLastResortFont(DgnFontType::Rsc)); }
            DgnFontCR GetLastResortRscFont() { return _GetLastResortRscFont(); }
            virtual DgnFontCR _GetLastResortShxFont() { return m_lastResortShxFont.IsValid() ? *m_lastResortShxFont : *(m_lastResortShxFont = _CreateLastResortFont(DgnFontType::Shx)); }
            DgnFontCR GetLastResortShxFont() { return _GetLastResortShxFont(); }
            virtual DgnFontCR _GetAnyLastResortFont() { return _GetLastResortTrueTypeFont(); }
            DgnFontCR GetAnyLastResortFont() { return _GetAnyLastResortFont(); }
            virtual DgnFontCR _GetDecoratorFont() { return _GetLastResortTrueTypeFont(); }
            DgnFontCR GetDecoratorFont() { return _GetDecoratorFont(); }
            DGNPLATFORM_EXPORT virtual DgnFontCR _ResolveFont(DgnFontCP);
            DgnFontCR ResolveFont(DgnFontCP font) { return _ResolveFont(font); }
            virtual bool _IsUsingAnRtlLocale() { return false; }
            bool IsUsingAnRtlLocale() { return _IsUsingAnRtlLocale(); }
            DGNPLATFORM_EXPORT virtual FT_Library _GetFreeTypeLibrary();
            FT_Library GetFreeTypeLibrary() { return _GetFreeTypeLibrary(); }
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

            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            // Get the version of the LineStyleAdmin api. Do not override this method.
            virtual int _GetVersion() const {return 1;}
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

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

        //! Supervise the processing of materials.
        struct MaterialAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //! Determine whether DgnPlatform should attempt to determine materials for geometry during display.
            virtual bool _WantDisplayMaterials() {return true;}
            //! Convert a stored material preview from a jpeg data block to an rgb image
            virtual BentleyStatus _GetMaterialPreviewAsRGB(Byte *rgbImage, Point2dCR outSize, Byte const* jpegImage, size_t jpegDataSize) {return ERROR;}
            //! Convert an RGB image to a jpeg compressed
            virtual BentleyStatus _GetMaterialPreviewJPEGFromRGB(Byte** jpegDataP, size_t& jpegDataSize, Byte const* rgb, Point2dCR size) {return ERROR;}
            //! Create an RGB image from a buffer
            virtual BentleyStatus _CreateImageFileFromRGB(WStringCR fileName, Byte *image, Point2dCR size, bool isRGBA)  {return ERROR;};
            };

        //! Supervise the processing of Raster Attachments
        struct RasterAttachmentAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //Control if raster are displayed or not
            virtual bool _IsDisplayEnable() const {return true;}

            //Control if raster locate logic can locate raster by its interior or by its border only.
            virtual bool _IsIgnoreInterior() const {return false;}
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

            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //! Get the density used for dynamic display.
            //! @return the dynamic density (a value between 0 and 1).
            virtual float               _GetDisplayDynamicDensity() const {return 0.10f;}

            //! Get the density used for point cloud display.
            //! @return the global density (a value between 0 and 1).
            virtual float               _GetDisplayGlobalDensity() const {return 1.0f;}

            //! Get the action to be taken with a point cloud exported to an i-model.
            //! @return the publishing action.
            virtual PublishingAction    _GetPublishingAction() const {return Publish_KeepOriginal;}

            //! Get the maximum size of a point cloud to be embedded in an i-model.
            //! If the point cloud is bigger than this value (in Mb), it is reduced to this value. This value is used
            //! only when the publishing action is Publish_ReduceSize.
            //! @return the publishing size (in Mb).
            virtual uint32_t            _GetPublishingEmbedSize() const {return 10;}

            //! Get the display type to use for point clouds
            //! @return the display query type.
            virtual DisplayQueryType    _GetDisplayQueryType() const {return DisplayQueryType_Progressive;}

            //! Copy the spatial reference from a point cloud file to a point cloud element
            //! @param[in]      eRef         The point cloud element.
            virtual BentleyStatus _SyncSpatialReferenceFromFile(DgnElementP eRef) { return ERROR; }

            //! returns whether we should automatically synchronize the spatial reference from the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceFromFile() const   { return false; }

            //! returns whether we should automatically synchronize the spatial reference to the POD file
            virtual bool _GetAutomaticallySyncSpatialReferenceToFile() const     { return false; }

            //! returns whether we should automatically synchronize the spatial reference from the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceFromFile() const           { return false; }

            //! returns whether we should automatically synchronize the spatial reference to the POD file even if it is empty
            virtual bool _GetSyncEmptySpatialReferenceToFile() const             { return false; }
            };

        //! Supply IRealityDatahandlers
        struct RealityDataAdmin : IHostObject
        {
        private:
            RealityDataCachePtr m_cache;

        public:
            virtual void _OnHostTermination(bool isProgramExit) {delete this;}

            DGNPLATFORM_EXPORT RealityDataCache& GetCache();
        };

        //! Supervise various graphics operations.
        struct GraphicsAdmin : IHostObject
            {
            //! Display control for edges marked as invisible in Mesh Elements and
            //! for B-spline Curve/Surface control polygons ("splframe" global).
            enum ControlPolyDisplay
                {
                CONTROLPOLY_DISPLAY_ByElement = 0, //! display according to element property.
                CONTROLPOLY_DISPLAY_Always    = 1, //! display on for all elements
                CONTROLPOLY_DISPLAY_Never     = 2, //! display off for all elements
                };

            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //! Return a pointer to a temporary QVCache used to create a temporary QVElem (short-lived).
            virtual QvCache* _GetTempElementCache() {return NULL;}

            //! Create and maintain a cache to hold cached representations of drawn geometry for persistent elements (QVElem).
            virtual QvCache* _CreateQvCache() {return NULL;}

            //! Delete specified qvCache.
            virtual void _DeleteQvCache(QvCacheP qvCache) {}

            //! Reset specified qvCache.
            virtual void _ResetQvCache(QvCacheP qvCache) {}

            //! Delete a specific cached representation from the persistent QVCache.
            virtual void _DeleteQvElem(QvElem*) {}

            //! Return whether a QVElem should be created, host must balance expense against memory use.
            virtual bool _WantSaveQvElem(/*DrawExpenseEnum*/int expense) {return true;}

            //! Return cache to use for symbols (if host has chosen to have a symbol cache).
            //! @note Symbol cache will be required for an interactive host.
            virtual QvCache* _GetSymbolCache() {return NULL;}

            //! Remove all entries in the symbol cache (if host has chosen to have a symbol cache).
            virtual void _EmptySymbolCache() {}

            //! Save cache entry for symbol (if host has chosen to have a symbol cache).
            virtual void _SaveQvElemForSymbol(IDisplaySymbol* symbol, QvElem* qvElem) {}

            //! Return cache entry for symbol (if host has chosen to have a symbol cache).
            virtual QvElem* _LookupQvElemForSymbol(IDisplaySymbol* symbol) {return NULL;}

            //! Delete a specific entry from the symbol cache.
            virtual void _DeleteSymbol(IDisplaySymbol* symbol) {}

            //! Define a texture
            virtual void _DefineTextureId(uintptr_t textureId, Point2dCR imageSize, bool enableAlpha, uint32_t imageFormat, Byte const* imageData) {}

            //! Check if a texture is defined
            virtual bool _IsTextureIdDefined(uintptr_t textureId) {return false;}

            //! Delete a specific texture, tile, or icon.
            virtual void _DeleteTexture(uintptr_t textureId) {}

            //! Define a tile texture
            virtual void _DefineTile(uintptr_t textureId, char const* tileName, Point2dCR imageSize, bool enableAlpha, uint32_t imageFormat, uint32_t pitch, Byte const* imageData) {}

            //! Draw a tile texture
            virtual void _DrawTile(IViewDrawR, uintptr_t textureId, DPoint3d const* verts) {}

            //! Create a 3D multi-resolution image.
            virtual QvMRImage* _CreateQvMRImage(DPoint3dCP fourCorners, Point2dCR imageSize, Point2dCR tileSize, bool enableAlpha, int format, int tileFlags, int numLayers) {return NULL;}

            //! Delete specified qvMRImage.
            virtual void _DeleteQvMRImage(QvMRImage* qvMRI) {}

            //! Add an image tile to a qvMRImage.
            virtual QvElem* _CreateQvTile(bool is3d, QvCacheP hCache, QvMRImage* mri, uintptr_t textureId, int layer, int row, int column, int numLines, int bytesPerLine, Point2dCR bufferSize, Byte const* pBuffer) {return NULL;}

            //! Define a custom raster format(QV_*_FORMAT) for color index data. Return 0 if error.
            virtual int _DefineCIFormat(int dataType, int numColors, unsigned long const* pTBGRColors){return 0;}

            //! An InteractiveHost may choose to allow applications to display non-persistent geometry during an update.
            virtual void _CallViewTransients(ViewContextR, bool isPreupdate) {}

            //! @return true to display a background fill behind text fields.
            virtual bool _WantShowDefaultFieldBackground() {return true;}

            //! @return Value to use for display control setting of mesh edges marked as invisible and for bspline curve/surface control polygons.
            virtual ControlPolyDisplay _GetControlPolyDisplay() {return CONTROLPOLY_DISPLAY_ByElement;}

            //! @return The max number of components before a cell will be drawn "fast" when ViewFlags.fast_cell is enabled.
            virtual uint32_t _GetFastCellThreshold() {return 1;}

            virtual bool _WantInvertBlackBackground() {return false;}

            virtual bool _GetModelBackgroundOverride(ColorDef& rgbColor, DgnModelType modelType) {return false;}

            //! If true, the UI icons that this library loads will be processed to darken their edges to attempt to increase visibility.
            virtual bool _ShouldEnhanceIconEdges() {return false;}

            virtual bool _WantDebugElementRangeDisplay() {return false;}

            virtual void _CacheQvGeometryMap(ViewContextR viewContext, uintptr_t rendMatID) {}

            // Send material to QuickVision.
            virtual BentleyStatus _SendMaterialToQV(MaterialCR material, ColorDef elementColor, DgnViewportP viewport) {return ERROR;}

            //! Supported color depths for this library's UI icons.
            enum IconColorDepth
                {
                ICON_COLOR_DEPTH_32,    //!< 32 BPP icons will be used (transparency)
                ICON_COLOR_DEPTH_24     //!< 24 BPP icons will be used (no transparency)
                };

            //! Gets the desired color depth of the UI icons that this library loads. At this time, 32 is preferred, but 24 can be used if required.
            virtual IconColorDepth _GetIconColorDepth() {return ICON_COLOR_DEPTH_32;}

            //! Get the longest amount of time allowed between clicks to be interpreted as a double click. Units are milliseconds.
            virtual uint32_t _GetDoubleClickTimeout() {return 500;} // default to 1/2 second

            //! Return minimum ratio between near and far clip planes for cameras - for z-Buffered output this is dictated by the depth of the z-Buffer
            //! for pre DX11 this was .0003 - For DX11 it is approximately 1.0E-6.
            virtual double _GetCameraFrustumNearScaleLimit() { return 1.0E-6; }

            virtual void _DrawInVp (HitDetailCP, DgnViewportP vp, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const {}

            DGNPLATFORM_EXPORT virtual void _GetInfoString (HitDetailCP, Utf8StringR pathDescr, Utf8CP delimiter) const;

            //! Gets the directory that holds the sprite definition files.
            virtual StatusInt _GetSpriteContainer(BeFileNameR spritePath, Utf8CP spriteNamespace) { return BSIERROR; }

            //! Return false to inhibit creating rule lines for surface/solid geometry for wireframe display.
            //! Can be used to improve display performance in applications that only work in shaded views (or those that will clear all QvElems before switching to wireframe)
            virtual bool _WantWireframeRuleDisplay() {return true;}

            }; // GraphicsAdmin

        //! Support for elements that store their data as Parasolid or Acis breps. Also required
        //! to output element graphics as solid kernel entities and facet sets.
        struct SolidsKernelAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //! Get the number of radial isoparametric lines to produce for analytic faces.
            //! @return Desired number of radial isoparametrics lines.
            //! @note: Used by _OutputBodyAsWireframe to produce hatch lines for non-planar faces.
            virtual int _GetAnalyticIsoparametricsNumber() const {return 4;}

            //! Get the number of u isoparametric lines to produce for parametric faces.
            //! @return Desired number of u direction isoparametrics lines.
            //! @note: Used by _OutputBodyAsWireframe to produce hatch lines for non-planar faces.
            virtual int _GetUIsoparametricsNumber() const {return 10;}

            //! Get the number of v isoparametric lines to produce for parametric faces.
            //! @return Desired number of v direction isoparametrics lines.
            //! @note: Used by _OutputBodyAsWireframe to produce hatch lines for non-planar faces.
            virtual int _GetVIsoparametricsNumber() const {return 10;}

            //! Get the solid kernel to storage unit scale. Storage units are meters, and a solid kernel unit also meters.
            //! @return storage unit to solid kernel scale to be used when creating a new ISolidKernelEntity.
            //! @note: Current scale will support single solids up to 1km, should be more than adequate 
            //!        to handle any sensible scenario with a high degree of linear precision.
            virtual double _GetSolidScale() const {return 1.0;}

            //! Produce a facet topology table for the supplied ISolidKernelEntity.
            //! @param[out] out Facet topology information for solid kernel entity.
            //! @param[in] in The solid kernel entity to draw.
            //! @param[in] pixelSize Used to support view dependent representations (ex. coarse facets when zoomed out)
            //! @return SUCCESS if operation was handled.
            //! @note: Required to display brep geometry for an interactive host.
            virtual BentleyStatus _FacetBody(IFacetTopologyTablePtr& out, ISolidKernelEntityCR in, double pixelSize = 0.0) const {return ERROR;}

            //! Produce a facet topology table for the supplied ISolidKernelEntity.
            //! @param[out] out Facet topology information for solid kernel entity.
            //! @param[in] in The solid kernel entity to draw.
            //! @param[in] options Facet options.
            //! @return SUCCESS if operation was handled.
            //! @note: Required to drop brep geometry to polyface(s).
            virtual BentleyStatus _FacetBody(IFacetTopologyTablePtr& out, ISolidKernelEntityCR in, IFacetOptionsR options) const {return _FacetBody(out, in, 0.0);}

            //! Output a ISolidKernelEntity as one or more closed planar shapes (may have holes) and surfaces to the supplied view context.
            //! @param[in] in The solid kernel entity to draw.
            //! @param[in] context The context to output the body to.
            //! @param[in] simplify if true faces are output as simple types (CurveVector, SolidPrimitive, and MSBSplineSurface) instead of sheet bodies.
            //! @return SUCCESS if operation was handled.
            virtual BentleyStatus _OutputBodyAsSurfaces(ISolidKernelEntityCR in, ViewContextR context, bool simplify = true) const {return ERROR;}

            //! Output a ISolidKernelEntity as a wireframe representation.
            //! @param[in] in The solid kernel entity.
            //! @param[in] context The context to output the body to.
            //! @param[in] includeEdges Include wire geometry for body edges.
            //! @param[in] includeFaceIso Include wire geometry for face isoparametrics.
            //! @return SUCCESS if operation was handled.
            virtual BentleyStatus _OutputBodyAsWireframe(ISolidKernelEntityCR in, ViewContextR context, bool includeEdges = true, bool includeFaceIso = true) const {return ERROR;}

            //! Return a CurveVector representation for a sheet body with a single planar face.
            //! @param[in] in The solid kernel entity.
            //! @return The CurveVectorPtr for the face geometry, or invalid if input entity was not a sheet body with a single planar face.
            virtual CurveVectorPtr _PlanarSheetBodyToCurveVector (ISolidKernelEntityCR in) const {return nullptr;}

            //! Return a CurveVector representation for a wire body.
            //! @param[in] in The solid kernel entity.
            //! @return The CurveVectorPtr for the edge geometry, or invalid if input entity was not a wire body.
            virtual CurveVectorPtr _WireBodyToCurveVector (ISolidKernelEntityCR in) const {return nullptr;}

            //! Output a cut section through an ISolidKernelEntity to the supplied view context.
            //! @param[in] in The solid kernel entity to display a section cut through.
            //! @param[in] transform The local (UOR) transform (not entity transform).
            //! @param[in] context The context to output the cut section to.
            //! @param[in] plane Clip plane origin and normal.
            //! @param[in] clipRange Clip extents based on clip mask.
            //! @param[in] clipMatrix clip orientation from clip x and y direction and plane normal.
            //! @param[in] clipMask mask detailing which directions are being clipped.
            //! @param[in] compoundDrawState - used for generating CurvePrimitiveId - cannot be extracted from context as this is only for output (not context for this cut).
            //! @return SUCCESS if operation was handled.
            virtual BentleyStatus _OutputBodyCut(ISolidKernelEntityCR in, TransformCP transform, ViewContextR context, DPlane3dCR plane, DRange2dCR clipRange, RotMatrixCR clipMatrix, ClipMask clipMask, CompoundDrawState* compoundDrawState) const {return ERROR;}

            //! Stretch faces/edges of a solid/surface kernel entity.
            //! @param[in] in The solid kernel entity to strecth.
            //! @param[in] transform The stretch transform.
            //! @param[in] fp identifies which faces/edges should be stretched.
            //! @return SUCCESS if the body was stretched.
            virtual BentleyStatus _FenceStretchBody(ISolidKernelEntityR in, TransformInfoCR transform, FenceParamsR fp) const {return ERROR;}

            //! Clip a solid/surface kernel entity with a fence.
            //! @param[out] inside clipped portion inside fence.
            //! @param[out] outside clipped portion outside fence.
            //! @param[in] in The solid kernel entity to clip.
            //! @param[in] fp region to clip against.
            //! @param[in] clipAsSheet treat solids as shells and return clip result as un-capped surfaces.
            //! @return SUCCESS if the body was clipped.
            virtual BentleyStatus _FenceClipBody(bvector<ISolidKernelEntityPtr>* inside, bvector<ISolidKernelEntityPtr>* outside, ISolidKernelEntityCR in, FenceParamsR fp, bool clipAsSheet = false) const {return ERROR;}

            //! Clip a curve vector for an open, closed path, or parity region. with a fence.
            //! @param[out] inside clipped portion inside fence.
            //! @param[out] outside clipped portion outside fence.
            //! @param[in] curves The curve vector to clip.
            //! @param[in] fp region to clip againts.
            //! @return SUCCESS if the geometry was clipped.
            virtual BentleyStatus _FenceClipCurveVector(bvector<CurveVectorPtr>* inside, bvector<CurveVectorPtr>* outside, CurveVectorCR curves, FenceParamsR fp) const {return ERROR;}

            //! Clip a curve vector for an open, closed path, or parity region. with a clip descriptor.
            //! @param[out] output clipped portion
            //! @param[in] input The curve vector to clip.
            //! @param[in] clip clip descriptor to clip against.
            //! @param[in] transformToDgn The transform from the input curve and the clip to uor coordinates.
            virtual BentleyStatus _ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clip, TransformCP transformToDgn) const {return ERROR;}

            //! Clip a body.
            //! @param[out] output clipped portion.
            //! @param[out] clipped true if body was clipped
            //! @param[in] input body to clip
            //! @param[in] clip clip descriptor to clip against.
            virtual BentleyStatus _ClipBody(bvector<ISolidKernelEntityPtr>& output, bool& clipped, ISolidKernelEntityCR input, ClipVectorCR clip) const {return ERROR;}

            //! Query the supplied ISolidKernelEntity for it's range.
            //! @param[out] range The entity range.
            //! @param[in] in The solid kernel entity to query.
            //! @return SUCCESS if range was set.
            virtual BentleyStatus _GetEntityRange(DRange3dR range, ISolidKernelEntityCR in) const {return ERROR;}

            //! Query the supplied ISolidKernelEntity for a basis transform (non-axis aligned orientation).
            //! @param[out] transform The entity basis transform.
            //! @param[in] in The solid kernel entity to query.
            //! @return SUCCESS if transform was set.
            virtual BentleyStatus _GetEntityBasisTransform(TransformR transform, ISolidKernelEntityCR in) const {return ERROR;}

            //! Simple yes/no type queries on ISolidKernelEntity
            enum KernelEntityQuery
                {
                EntityQuery_HasCurvedFaceOrEdge = 0, //!< Check if body has only planar faces and linear edges.
                EntityQuery_HasHiddenEdge       = 1, //!< Check if body has at least one edge with hidden attribute.
                EntityQuery_HasHiddenFace       = 2, //!< Check if body has at least one face with hidden attribute.
                EntityQuery_HasOnlyPlanarFaces  = 3, //!< Check if body has only planar faces.
                };

            //! Query the supplied ISolidKernelEntity for information
            //! @param[in] in The solid kernel entity to query.
            //! @param[in] query The solid kernel entity property to query.
            //! @return true if the supplied entity has the specified query.
            virtual bool _QueryEntityData(ISolidKernelEntityCR in, KernelEntityQuery query) const {return false;}

            //! Query if input ISolidKernelEntity are geometrically equal (differ only by a transform).
            //! @param[in] entity1 The first of the solid kernel entities to compare.
            //! @param[in] entity2 The second of the solid kernel entities to compare.
            //! @param[in] distanceTolerance The tolerance for distances.
            //! @return true if the supplied entities are the same geometry.
            virtual bool _AreEntitiesEqual(ISolidKernelEntityCR entity1, ISolidKernelEntityCR entity2, double distanceTolerance) const {return false;}

            //! Calculates the volume, center of gravity and moments of inertia for a ISolidKernelEntity.
            //! @param[in] in The solid kernel entity.
            //! @param[out] amount Volume of solid body, surface area of sheet body, or length of a wire body.
            //! @param[out] periphery surface area of solid body, circumference of sheet body, and not used for wire body.
            //! @param[out] centroid center of gravity.
            //! @param[out] inertia moment of inertia.
            //! @param[in] tolerance Required tolerance for results.
            //! @return SUCCESS if query results are valid.
            virtual BentleyStatus _MassProperties(ISolidKernelEntityCR in, double* amount, double* periphery, DPoint3dP centroid, double inertia[3][3], double tolerance) const {return ERROR;}

            //! Create an ISolidKernelEntity from the supplied curve vector.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] profile The profile to create a wire or planar sheet body from.
            //! @param[in] curveToDgn Optional transform from curve to modelRef UOR coordinates.
            //! @param[in] idMap Optional map of edge tags to curve topology ids.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _CreateBodyFromCurveVector(ISolidKernelEntityPtr& out, CurveVectorCR profile, TransformCP curveToDgn = NULL, struct EdgeToCurveIdMap* idMap = NULL) const {return ERROR;}

            //! Create an ISolidKernelEntity from the supplied solid primitve data.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] primitive The solid primitive definition.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _CreateBodyFromSolidPrimitive(ISolidKernelEntityPtr& out, ISolidPrimitiveCR primitive) const {return ERROR;}

            //! Create an ISolidKernelEntity from the supplied bspline surface.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] surface The bspline surface to represent as a surface entity.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _CreateBodyFromBSurface(ISolidKernelEntityPtr& out, MSBsplineSurfaceCR surface) const {return ERROR;}

            //! Create an ISolidKernelEntity from the supplied polyface data.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] meshData The polyface data to represent as a surface or solid entity.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _CreateBodyFromPolyface(ISolidKernelEntityPtr& out, PolyfaceQueryCR meshData) const {return ERROR;}

            //! Transmit a part to a logical block of data (in binary format) using the solid kernel.
            //! Implementing this method is required for creating smart surfaces and solids.
            //! @param[out] ppBuffer Archived data for solid kernel entity.
            //! @param[out] bufferSize Size of data referenced by ppBuffer.
            //! @param[in] in The solid kernel entity to transmit.
            //! @return SUCCESS if transmit of kernel entity was successful.
            //! @note The output ppBuffer is assumed to be owned by the input ISolidKernelEntityP who is responsible for freeing it
            //!       if necessary when its destructor is called.
            virtual BentleyStatus _SaveEntityToMemory(uint8_t** ppBuffer, size_t& bufferSize, ISolidKernelEntityCR in) const {return ERROR;}

            //! Receive a part archived in binary format and instantiate an entity in the solid kernel for the current session.
            //! Implementing this method is required to query/output smart surfaces and solids.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] pBuffer The input data (always stored using binary format)
            //! @param[in] bufferSize Size of pBuffer.
            //! @param[in] transform The body to uor transform to store on the output ISolidKernelEntity.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _RestoreEntityFromMemory(ISolidKernelEntityPtr& out, uint8_t const* pBuffer, size_t bufferSize, TransformCR transform) const {return ERROR;}

            //! Create a non-owning ISolidKernelEntity from the input entity.
            //! @param[out] out Ref counted pointer to new solid kernel entity.
            //! @param[in] in The solid kernel entity to instance.
            //! @return SUCCESS if out holds a valid solid kernel entity.
            virtual BentleyStatus _InstanceEntity(ISolidKernelEntityPtr& out, ISolidKernelEntityCR in) const {return ERROR;}

            //! Check if the supplied ISolidKernelEntity has at least the specified number of faces, edges, and vertices.
            //! @param[in] in The solid kernel entity to query.
            //! @param[in] minRequiredFace Minimum required face count.
            //! @param[in] minRequiredEdge Minimum required edge count.
            //! @param[in] minRequiredVertex Minimum required vertex count.
            //! @note Useful for tools that support sub-entity selection in a post-locate filter.
            //! @return true if solid kernel entity passed supplied criteria.
            virtual bool _IsValidForSubEntitySelection(ISolidKernelEntityCR in, size_t minRequiredFace, size_t minRequiredEdge, size_t minRequiredVertex) const {return false;}

            //! Pick face, edge, and vertex sub-entities that intersect a given ray.
            //! @param[in] entity The solid kernel entity to test.
            //! @param[in] boresite Picking ray.
            //! @param[out] intersectEntities selected sub-entities.
            //! @param[out] intersectPts selected points on sub-entities.
            //! @param[out] intersectParams selected parameter on sub-entities (xy is uv for face hits, x is u for edges hits, unused for vertex hits.
            //! @param[in] maxFace Maximum number of face hits to return (0 to not pick faces).
            //! @param[in] maxEdge Maximum number of edge hits to return (0 to not pick edges).
            //! @param[in] maxVertex Maximum number of vertex hits to return (0 to not pick vertices).
            //! @param[in] maxDistance Pick radius for edge and vertex locates.
            //! @return true if at least 1 sub-entity is selected.
            virtual bool _Locate(ISolidKernelEntityCR entity, DRay3dCR boresite, bvector<ISubEntityPtr>& intersectEntities, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams, size_t maxFace, size_t maxEdge, size_t maxVertex, double maxDistance) const {return false;}

            //! Test if a given ray intersects a face sub-entity.
            //! @param[in] subEntity The solid kernel face sub-entity to test.
            //! @param[in] boresite Picking ray.
            //! @param[out] intersectPts selected points on sub-entity.
            //! @param[out] intersectParams selected uv params on sub-entity.
            //! @return true if ray intersects face.
            virtual bool _RayTestFace(ISubEntityCR subEntity, DRay3dCR boresite, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams) const {return false;}

            //! Return closest point and closest sub-entity (face, edge, or vertex) to a given point.
            //! @param[in] entity The solid kernel entity to test.
            //! @param[in] testPt The point to find the closest point on the body to.
            //! @param[out] subEntity closest sub-entity.
            //! @param[out] point closest point on body.
            //! @param[out] param closest parameter on sub-entity (xy is uv for face, x is u for edge, unused for vertex hits.
            //! @param[in] preferFace If closest entity for a sheet or solid body is an edge or vertex, return a surrounding face instead.
            //! @return true if closest point could be evaluated.
            virtual bool _ClosestPoint(ISolidKernelEntityCR entity, DPoint3dCR testPt, ISubEntityPtr& subEntity, DPoint3dR point, DPoint2dR param, bool preferFace = false) const {return false;}

            //! Output the geometry for the supplied sub-entity to the specified context.
            //! @param[in] subEntity The solid kernel sub-entity to draw.
            //! @param[in] context The context to output the sub-entity to.
            //! @return SUCCESS if a valid solid kernel sub-entity was specified.
            //! @note Can be used for selection dynamics as well as to collect sub-entity geometry as a CurveVector or SolidPrimitive using an IElementGraphicsProcessor.
            virtual BentleyStatus _Draw(ISubEntityCR subEntity, ViewContextR context) const {return ERROR;}

            //! Evaluate a uv parameter on a face sub-entity.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[out] point point at supplied uv parameter.
            //! @param[out] normal normal at supplied uv parameter.
            //! @param[out] uDir u direction at supplied uv parameter.
            //! @param[out] vDir v direction at supplied uv parameter.
            //! @param[in] uvParam uv parameter to evaluate.
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _EvaluateFace(ISubEntityCR subEntity, DPoint3dR point, DVec3dR normal, DVec3dR uDir, DVec3dR vDir, DPoint2dCR uvParam) const {return ERROR;}

            //! Evaluate a u parameter on an edge sub-entity.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[out] point point at supplied u parameter.
            //! @param[out] uDir tangent direction at supplied u parameter.
            //! @param[in] uParam u parameter to evaluate.
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _EvaluateEdge(ISubEntityCR subEntity, DPoint3dR point, DVec3dR uDir, double uParam) const {return ERROR;}

            //! Evaluate the position of a vertex sub-entity.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[out] point point at vertex.
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _EvaluateVertex(ISubEntityCR subEntity, DPoint3dR point) const {return ERROR;}

            //! Get the length of a curve coincident with a constant u or v parameter line defined by the face sub-entity parameter range.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[in] direction 0 or u, 1 for v.
            //! @param[in] parameter The constant parameter in u or v.
            //! @param[out] length
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _GetFaceCurveLength(ISubEntityCR subEntity, int direction, double parameter, double& length) const {return ERROR;}

            //! Get the u and v parameter ranges for a supplied face sub-entity.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[out] uRange u parameter range of face.
            //! @param[out] vRange v parameter range of face.
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _GetFaceParameterRange(ISubEntityCR subEntity, DRange1dR uRange, DRange1dR vRange) const {return ERROR;}

            //! Get the u parameter ranges for a supplied edge sub-entity.
            //! @param[in] subEntity The solid kernel sub-entity to query.
            //! @param[out] uRange u parameter range of edge.
            //! @return SUCCESS if sub-entity could be evaluated.
            virtual BentleyStatus _GetEdgeParameterRange(ISubEntityCR subEntity, DRange1dR uRange) const {return ERROR;}

            //! Create an ISubEntityPtr from a string returned by ISubEntity::ToString.
            //! @param[in] in The solid kernel entity that sub-entity string originated from.
            //! @param[in] subEntityStr string returned by ISubEntity::ToString.
            //! @return A new ISubEntityPtr.
            virtual ISubEntityPtr _CreateSubEntityPtr(ISolidKernelEntityCR in, WCharCP subEntityStr) const {return NULL;}

            //! Test is the supplied ISolidKernelEntity is the parent of the supplied ISubEntity.
            //! @param[in] subEntity subEntity The solid kernel sub-entity to query.
            //! @param[in] entity The solid kernel entity to check as the parent.
            //! @return true if entity is the parent.
            virtual bool _IsParentEntity(ISubEntityCR subEntity, ISolidKernelEntityCR entity) const {return false;}

            //! Simple yes/no type queries on ISubEntity
            enum SubEntityQuery
                {
                SubEntityQuery_IsPlanarFace = 1, //!< Check if face sub-entity has as planar surface.
                SubEntityQuery_IsSmoothEdge = 2, //!< Check if edge sub-entity is smooth by comparing the face normals along the edge.
                };

            //! Query the supplied ISubEntity for information
            //! @param[in] subEntity The sub-entity to query.
            //! @param[in] query The sub-entity property to query.
            //! @return true if the supplied entity has the specified query.
            virtual bool _QuerySubEntityData(ISubEntityCR subEntity, SubEntityQuery query) const {return false;}

            //! Unify target body with tools. This will produce an unified output body with any shared faces and edges hidden.
            //! @param[in,out] targetEntity The target body to unify with.
            //! @param[in] toolEntities Neighboring tool bodies.
            //! @param[in] nTools tool body count.
            //! @return SUCCESS if tool body was unified with target.
            virtual BentleyStatus _UnifyBody(ISolidKernelEntityPtr& targetEntity, ISolidKernelEntityPtr* toolEntities, size_t nTools) const {return ERROR;}
            };

        //! Receives messages sent to NotificationManager. Hosts can implement this interface to communicate issues to the user.
        struct NotificationAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

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
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int _GetVersion() const {return 1;} // Do not override!
            virtual void _OnHostTermination(bool isProcessShutdown) override {delete this;}

            //! Allows the host to provide a path to coordinate system definition files.
            virtual WString _GetDataDirectory() { return L""; }

            virtual IGeoCoordinateServicesP _GetServices() const {return NULL;}
            };

        //! Formatter preferences for units, fields, etc
        struct FormatterAdmin : IHostObject
            {
            DEFINE_BENTLEY_NEW_DELETE_OPERATORS

            virtual int     _GetVersion() const {return 1;}
            virtual void    _OnHostTermination(bool isProcessShutdown) override {delete this;}
            //! If true, display coordinates in DGN format(eg. 1:0 1/4); if false, display DWG format(eg. 1'-0 1/4").
            virtual bool    _AllowDgnCoordinateReadout() const {return true;}
            };

        struct CompareTableNames {bool operator()(Utf8CP a, Utf8CP b) const {return strcmp(a, b) < 0;}};
        typedef bmap<Utf8CP,DgnDomain::TableHandler*,CompareTableNames> T_TableHandlers;
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
        GraphicsAdmin*          m_graphicsAdmin;
        MaterialAdmin*          m_materialAdmin;
        SolidsKernelAdmin*      m_solidsKernelAdmin;
        GeoCoordinationAdmin*   m_geoCoordAdmin;
        TxnAdmin*               m_txnAdmin;
        IACSManagerP            m_acsManager;
        LineStyleManagerP       m_lineStyleManager;
        FormatterAdmin*         m_formatterAdmin;
        RealityDataAdmin*       m_realityDataAdmin;
        Utf8String              m_productName;
        T_TableHandlers         m_tableHandlers;
        T_RegisteredDomains     m_registeredDomains;
        
        // Get the version of the DgnPlatform api. Do not override this method.
        virtual int _GetVersion() const {return 1;}

    public:
        T_TableHandlers& TableHandlers() {return m_tableHandlers;}
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

        //! Supply the GraphicsAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual GraphicsAdmin& _SupplyGraphicsAdmin();

        //! Supply the MaterialAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual MaterialAdmin& _SupplyMaterialAdmin();

        //! Supply the SolidsKernelAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again.
        DGNPLATFORM_EXPORT virtual SolidsKernelAdmin& _SupplySolidsKernelAdmin();

        //! Supply the GeoCoordinationStateAdmin for this session. This method is guaranteed to be called once per thread from DgnPlatformLib::Host::Initialize and never again..
        DGNPLATFORM_EXPORT virtual GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        //! Supply the formatter admin
        DGNPLATFORM_EXPORT virtual FormatterAdmin& _SupplyFormatterAdmin();

        //! Supply the RealityDataAdmin
        DGNPLATFORM_EXPORT virtual RealityDataAdmin& _SupplyRealityDataAdmin();

        //! Supply the product name to be used to describe the host.
        virtual void _SupplyProductName(Utf8StringR) = 0;

        virtual BeSQLite::L10N::SqlangFiles  _SupplySqlangFiles() = 0;

        Host()
            {
            m_knownLocationsAdmin = 0;
            m_exceptionHandler = 0;
            m_progressMeter = 0;
            m_fontAdmin = 0;
            m_lineStyleAdmin = 0;
            m_rasterAttachmentAdmin = 0;
            m_pointCloudAdmin = 0;
            m_notificationAdmin = 0;
            m_graphicsAdmin = 0;
            m_materialAdmin = 0;
            m_solidsKernelAdmin = 0;
            m_geoCoordAdmin = 0;
            m_txnAdmin = 0;
            m_acsManager = 0;
            m_lineStyleManager = 0;
            m_formatterAdmin = 0;
            m_realityDataAdmin = 0;

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
        GraphicsAdmin&          GetGraphicsAdmin()         {return *m_graphicsAdmin;}
        MaterialAdmin&          GetMaterialAdmin()         {return *m_materialAdmin;}
        SolidsKernelAdmin&      GetSolidsKernelAdmin()     {return *m_solidsKernelAdmin;}
        GeoCoordinationAdmin&   GetGeoCoordinationAdmin()  {return *m_geoCoordAdmin;}
        TxnAdmin&               GetTxnAdmin()              {return *m_txnAdmin;}
        IACSManagerR            GetAcsManager()            {return *m_acsManager;}
        LineStyleManagerR       GetLineStyleManager()      {return *m_lineStyleManager;}
        FormatterAdmin&         GetFormatterAdmin()        {return *m_formatterAdmin;}
        RealityDataAdmin&       GetRealityDataAdmin()      {return *m_realityDataAdmin;}
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
    //! @return NULL if not Host is associated with the current thread. Otherwise, a pointer to the Host object.
    DGNPLATFORM_EXPORT static Host* QueryHost();

    //! Get the Host that associated with the current thread
    //! @return a reference to the Host object. WARNING: Do not call this function unless you know that there is a Host.
    DGNPLATFORM_EXPORT static Host& GetHost();

    //! Used by DgnDbFileIO to initialize logging for Graphite code.
    //! @param configFileName Optional. The name of the logging configuration file to parse. Pass NULL for logging to the console with default severities.
    //! If configFileName is specified, then the log4cxx provider will be used. Note that this provider comes from log4cxx.dll, and both the Graphite and Vancouver
    //! code will use the same log4cxx.dll. 
    DGNPLATFORM_EXPORT static void InitializeBentleyLogging(WCharCP configFileName);

    //! Forward assertion failures to the specified handler.
    DGNPLATFORM_EXPORT static void ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler*);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

