/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridge.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnView/DgnViewLib.h>

#ifdef __IMODEL_BRIDGE_BUILD__
    #define IMODEL_BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODEL_BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_DGN_NAMESPACE

struct iModelBridgeFwk;
struct iModelBridgeSacAdapter;

/**
@addtogroup GROUP_iModelBridge

An iModel "bridge" converts data from an external data source into a BIM. 

See @ref ANCHOR_TypicalBridgeConversionLogic "iModelBridge conversion logic pattern" for details on how to write bridge conversion logic.

A bridge is normally called as part of a @ref ANCHOR_BridgeJob "job" by the @ref GROUP_iModelBridgeFwk "iModelBridge Framework"
as part of a process of reacting to changes to source documents and then converting those changes and updating an iModel.
A bridge may also be called by a @ref BentleyApi::Dgn::iModelBridgeSacAdapter "standalone converter" in order to write to a standalone dgndb file.
A @ref BentleyApi::Dgn::iModelBridgeSacAdapter "standalone converter" may be written in order to @em test a bridge's conversion logic.

A bridge must be @ref ANCHOR_BridgeLoading "implemented by a shared library".

Here is a summary of the process that is conducted by the framework. The process shown here is for the case of a bridge doing an incremental update.
The process of creating a new BIM is essentially similar. The differences are noted below.

-# The framework first registers the DgnPlatformLib::Host
-# The framework signs into iModelHub and gets access to the specified iModel
-# The framework acquires a briefcse from iModelHub, if necessary, and pulls and merges revisions to make sure it is up to date.
-# The framework @ref ANCHOR_BridgeLoading "loads the bridge dll specified for the job and asks it to create a bridge object".
The framework then makes the following calls on the bridge object:
-# iModelBridge::_ParseCommandLine
-# iModelBridge::_Initialize
-# The framework opens the BIM.
    -# The framework calls BentleyApi::Dgn::DgnDomains::ImportSchemas, if necessary, in order to ensure that the domains 
registered by the bridge in its _Initialize method are imported into the BIM and are up to date. 
-# iModelBridge::_OnConvertToBim
-# iModelBridge::_OpenSource
-# Find or initialize the @ref ANCHOR_BridgeJob "job"
    -# iModelBridge::_GetParams().SetIsUpdating (true);
    -# jobsubject = iModelBridge::_FindJob
    -# If jobsubject.IsInvalid
        -# iModelBridge::_GetParams().SetIsUpdating (false)
        -# jobsubject = iModelBridge::_InitializeJob
-# iModelBridge::_ConvertToBim 
    -# If _ConvertToBim fails, 
        -# then the framework will abandon all changes. 
-# The framework attempts to obtain the @ref ANCHOR_iModelBridgeLocksAndCodes "locks and codes" that are required by the inserts and updates that were done by iModelBridge::_ConvertToBim.
    -# If locks and codes are not acquired, 
        -# then the framework will abandon all changes.
-# iModelBridge::_CloseSource
-# iModelBridge::_OnConvertedToBim
-# DgnDb::SaveChanges
-# The framework attempts to pull, merge, and push. 
    
If pullmergepush fails, then the bridge's local BIM will still contain the results of the conversion, 
but no revision was created on the iModelHub. The framework will retry the pullmergepush step automatically the next time it runs.

If pullmergepush succeeds, then a new revision containing the BIM conversion results is on the iModelHub 
and may be downloaded by other briefcases.

<h2>Creating a New Repository</h2>

The bridge framework may create a new repository. As far as the bridge is concerned, the process is the same as 
that described above, specifically the case where the bridge does an initial full conversion. There will be two
minor differences:
- iModelBridge::_DeleteSyncInfo is called.
- iModelBridge::_FindJob is @em not called. Instead, the framework skips that step and goes right to the call to iModelBridge::_InitializeJob.
- iModelBridge::Params::IsCreatingNewDgnDb will be true, where normally that property is false.

@anchor ANCHOR_iModelUnitsAndGCS
<h2>Units and Geographic Coordinate System</h2>

All coordinates and distances in an iModel must be stored in meters, and so the bridge must transform source data coordinates and distances into meters.

If the target iModel has a Geographic Coordinate System (GCS), the bridge must transform the source data into that GCS.
Similarly, if the iModel has a global origin, the bridge must subtract off that global origin from the source data as part of the conversion.
Call DgnDb::GeoLocation::GetDgnGCS to get the details of the iModel's GCS and global origin.
*/

//=======================================================================================
//! Interface between an @ref GROUP_iModelBridge "iModel bridge" and the @ref GROUP_iModelBridgeFwk "iModelBridge framework"
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridge
{
    static WCharCP str_BriefcaseIExt() {return L"ibim";}
    static WCharCP str_BriefcaseExt() {return L"bim";}
    
    // *** NEEDS WORK: We need some kind of registry of unique bridge types. This can then be
    // ***              used by a given bridge to help make its job name unique in the iModel's dictionary model.
    static Utf8CP str_BridgeType_DgnV8() {return "DgnV8";}

    enum class CmdLineArgStatus{Success, Error, NotRecognized};

    struct GCSDefinition
        {
        bool                 m_isValid;         //!< Is this definition valid?
        Utf8String           m_coordSysKeyName; //!< the coordinate system key name
        DPoint3d             m_originUors;      //!< The XYZ coordinates of the origin
        BentleyApi::GeoPoint m_geoPoint;        //!< The longitude and latitude of the origin
        double               m_azimuthAngle;    //!< The azimuthal angle of the Cartesian coordinate system
        GCSDefinition() : m_isValid(false) {}

        IMODEL_BRIDGE_EXPORT DgnGCSPtr CreateGcs(DgnDbR);
        };

    enum class GCSCalculationMethod 
        {
        UseDefault,                             //!< Base the choice on the GCS type in the root and imported model.
        UseReprojection,                        //!< Convert using full reprojection calculations.
        UseGcsTransform,                        //!< Convert using a best fit, unscaled transform.
        UseGcsTransformWithScaling,             //!< Convert using a best fit transform that includes scaling to GCS grid coordinates.
        };

    //! Parameters that are common to all bridges.
    //! These parameters are set up by the iModelBridgeFwk based on job definition parameters and other sources. 
    //! In a standalone converter, they are set from the command line. 
    struct Params
        {
      protected:
        friend struct iModelBridge;
        friend struct iModelBridgeFwk;
        friend struct iModelBridgeSacAdapter;

        bool m_isCreatingNewDb = false;
        bool m_isUpdating = false;
        bool m_wantThumbnails = true;
        BeFileName m_inputFileName;
        BeFileName m_drawingsDirs;
        // *** TBD: location of mangaged workspace
        GCSDefinition m_inputGcs;
        GCSDefinition m_outputGcs;
        GCSCalculationMethod m_gcsCalculationMethod;
        BeFileName m_briefcaseName;
        BeFileName m_assetsDir;
        BeFileName m_libraryDir;
        BeFileName m_reportFileName;
        Utf8String m_converterJobName;
        DgnPlatformLib::Host::RepositoryAdmin* m_repoAdmin;
        BeDuration m_thumbnailTimeout = BeDuration::Seconds(30);

        void SetIsCreatingNewDgnDb(bool b) {m_isCreatingNewDb=b;}
        IMODEL_BRIDGE_EXPORT void SetReportFileName();
        void SetThumbnailTimeout(BeDuration timeout) {m_thumbnailTimeout = timeout;}

      public:
        //! @name Helper functions
        //! @{

        IMODEL_BRIDGE_EXPORT static BentleyStatus ParseGcsSpec(GCSDefinition& gcs, Utf8StringCR gcsParms);
        IMODEL_BRIDGE_EXPORT static BentleyStatus ParseGCSCalculationMethod(GCSCalculationMethod& cm, Utf8StringCR value);

        //! @}

        IMODEL_BRIDGE_EXPORT BentleyStatus Validate();

        //! @private
        bool IsCreatingNewDgnDb() const {return m_isCreatingNewDb;} //!< True in the rare case when the bridge is asked to populate a new local file, in preparation for pushing a new repository to the iModelHub
        void SetIsUpdating(bool b) {m_isUpdating=b;}
        bool IsUpdating() const {return m_isUpdating;} //!< True if the bridge is updating an existing job subject and its contents from a previous conversion.
        BeFileNameCR GetBriefcaseName() const {return m_briefcaseName;} //!< The name of the BIM that is being updated
        BeFileNameCR GetInputFileName() const {return m_inputFileName;} //!< The name of the input file that is to be read and converted and/or scanned for changes.
        BeFileNameCR GetAssetsDir() const {return m_assetsDir;} //!< The bridge library's assets directory
        BeFileNameCR GetLibraryDir() const {return m_libraryDir;} //!< The directory from which the bridge library itself was loaded 
        BeFileNameCR GetDrawingsDirs() const {return m_drawingsDirs;} //!< The top-level directory to scan for other files that may contain drawings and sheets
        BeFileNameCR GetReportFileName() const {return m_reportFileName;} //!< Where to write a report of results and issues that occurred during the conversion. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues and logging".
        //! Once the BIM name has been set, the framework calls this to compute the report file name. This is also called automatically by Validate.
        DgnPlatformLib::Host::RepositoryAdmin* GetRepositoryAdmin() const {return m_repoAdmin;} //!< The repository admin
        GCSDefinition GetInputGcs() const {return m_inputGcs;} //!< Optional. The GCS of the input data source
        GCSDefinition GetOutputGcs() const {return m_outputGcs;} //!< Optional. The GCS that the (new) iModel should have.
        GCSCalculationMethod GetGCSCalculationMethod() const {return m_gcsCalculationMethod;} //!< Optional. How to transform the input's GCS to the BIM's GCS
        BeDuration GetThumbnailTimeout() const {return m_thumbnailTimeout;}
        void SetWantThumbnails(bool b) {m_wantThumbnails = b;}
        bool WantThumbnails() const {return m_wantThumbnails;}
        void SetBridgeJobName(Utf8StringCR str) {m_converterJobName=str;}
        Utf8String GetBridgeJobName() const {return m_converterJobName;}
        };

    private:
        friend struct iModelBridgeFwk;
        friend struct iModelBridgeSacAdapter;

    //! @name Helper functions
    //! @{

    //! @private
    //! Create a new local repository and allow this bridge to populate it. This called by the framework as part of creating a new repository.
    //! @param[out] jobModels   The set of models that were created by the job.
    //! @return nullptr if the db could not be created or populated. 
    //! @note that it is up to the caller to call SaveChanges 
    IMODEL_BRIDGE_EXPORT DgnDbPtr DoCreateDgnDb(bvector<DgnModelId>& jobModels);

    //! @private
    //! Open an existing BIM for read-write, possibly doing a schema upgrade on it.
    //! @param[out] dbres  If the BIM cannot be opened or upgraded, return the error status here.
    //! @param[out] madeSchemaChanges Set to true if OpenDgnDb imported or upgraded schemas.
    //! @return Opened BIM or an invalid ptr if the BIM could not be opened.
    IMODEL_BRIDGE_EXPORT DgnDbPtr OpenBim(BeSQLite::DbResult& dbres, bool& madeSchemaChanges);

    //! @private
    //! Convert source data to an existing BIM. This is called by the framework as part of a normal conversion.
    //! @param[in] db The BIM to be updated
    //! @return non-zero error status if the bridge cannot convert the BIM. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues" 
    //! @note The caller must check the return status and call SaveChanges on success or AbandonChanges on error.
    //! @see OpenBim
    IMODEL_BRIDGE_EXPORT BentleyStatus DoConvertToExistingBim(DgnDbR db);

    //! @}


public:
    //! Supply the relative path to the .db3 file that contains translatable strings used by the bridge. 
    //! Must be relative to the directory returned by DgnPlatformLib::QueryHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory()
    virtual WString _SupplySqlangRelPath() = 0;

    //! Return the Params struct that holds all of the parameters for the bridge.
    //! @note The @ref GROUP_iModelBridgeFwk "framework" may use this function to update bridge parameters in the course of executing a job.
    //! (For example, the framework may call iModelBridge::_GetParams().SetIsUpdating (false) after discoverting that there is no existing 
    //! job.) Therefore, _GetParams must return a reference to the live copy of the params, and the bridge must get parameters from this live copy as it runs,
    //! in order to see the correct values.
    virtual Params& _GetParams() = 0;

    //! Print a message describing command line arguments
    virtual void _PrintUsage() {}

    //! Parse an individual command line argument. This is the way to handle bridge-specific arguments.
    //! @param iarg the index of the command line argument to be parsed
    //! @param argc the number of command line arguments
    //! @param argv the command line arguments
    //! @return an indication of whether the argument was handled and if so if it was valid
    virtual CmdLineArgStatus _ParseCommandLineArg(int iarg, int argc, WCharCP argv[]) {return CmdLineArgStatus::NotRecognized;}

    //! The bridge should parse any bridge-specific command-line arguments. The framework takes care of the standard
    //! bridge command-line arguments. Note that the BIM is not yet open. 
    //! @note The bridge should @em em attempt to open the BIM.
    //! @param argc the number of command line arguments
    //! @param argv the command line arguments
    //! @return non-zero if you recognize an argument and its value is invalid or if a required argument is not supplied. 
    //! @remarks Do not return error if you encounter an unrecognized argument
    //! @see _ParseCommandLineArg for handling an individual argument
    virtual BentleyStatus _ParseCommandLine(int argc, WCharCP argv[]) {return BSISUCCESS;}

    //! The bridge should register domains and handlers and do any other initilization that is not specific to a particular BIM.
    //! @note The framework will have already registered the DgnPlatformLib::Host. The bridge does not do that.
    //! @param argc the number of command line arguments
    //! @param argv the command line arguments
    //! @return non-zero error status if the bridge cannot run. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    //! @see iModelBridgeBimHost
    //! @note The bridge must wait until _Initialize to register domains.
    virtual BentleyStatus _Initialize(int argc, WCharCP argv[]) = 0;

    //! This is called in the rare case where an attempt to create a new dgndb failed. This function should clean up syncinfo.
    //! The BIM is closed when this is called.
    virtual void _DeleteSyncInfo() {}

    //! The bridge should prepare to write to the BIM.
    //! <p>In its implementation of _OnConvertToBim, the bridge should:
    //! - Should store a pointer to @a db.
    //! - @ref ANCHOR_MutiFileTransaction "Open and attach its syncinfo file".
    //! - Create temp tables.
    //! <p>_OnConvertToBim may call DgnDb::SaveChanges and in fact should do that if it creates temp tables or attaches syncinfo.
    //! <p>This function is called after _ParseCommandLine, _Initialize, and _OpenSyncInfo. It is called right after the 
    //! framework opens the BIM. It is called after domains are imported.
    //! <p>The bridge should wait for the call to _FindJob/_InitializeJob before creating elements or models in the BIM.
    //! @param db   The BIM or local DgnDb that will be updates
    //! @return non-zero error status if the bridge cannot perform the conversion. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    //! @see _OnConvertedToBim, _FindJob, _InitializeJob, _ConvertToBim
    //! @note _OnConvertToBim is called @em before domains and schemas are imported.
    virtual BentleyStatus _OnConvertToBim(DgnDbR db) = 0;

    //! The conversion is finished. It may have been terminated abnormally. 
    //! <p>The bridge must drop its pointer to the DgnDb that was passed to _OnConvertToBim. The DgnDb will be closed after this function is called.
    //! <p>If the bridge has attached syncinfo to the DgnDb, it must close syncinfo.
    //! <p>This function will not be called if _OnConvertToBim returns an error status.
    //! @param updateStatus non-zero error status if any step in the conversion failed. If so, the conversion will be rolled back.
    //! @note Even if updateStatus is SUCCESS, the bridge's changes have not yet been saved to the BIM or pushed to the iModelHub.
    //! @see _OnConvertToBim
    virtual void _OnConvertedToBim(BentleyStatus updateStatus) = 0;

    //! Open the data source and be prepared to do the conversion
    //! @return non-zero error status if the bridge cannot open the source. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    //! @see _CloseSource
    virtual BentleyStatus _OpenSource() {return BSISUCCESS;}

    //! The bridge can close its source data files, because the conversion is finished. It may have been terminated abnormally. 
    //! This function will not be called if _OpenSource returned a non-zero error status.
    //! @param updateStatus non-zero error status if any step in the conversion failed. If so, the conversion will be rolled back.
    virtual void _CloseSource(BentleyStatus updateStatus) {}

    //! Try to find an existing @ref ANCHOR_BridgeJob "job" in the BIM.
    //! This is called prior to _ConvertToBim.
    //! <p>Normally, the bridge should look up a job subject element by its code. 
    //! <p>If _FindJob does detect an existing job, that means that this is an update.
    //! In this case, the bridge should prepare for update-style change detection.
    //! @return null if the BIM does not contain a job subject for this bridge and this data source.
    virtual SubjectCPtr _FindJob() = 0;

    //! Create a @ref ANCHOR_BridgeJob "job" subject element and all infrastructure elements and models below it.
    //! This is called once in the life of a bridge and is called only if _FindJob returned an invalid element ptr.
    //! <p>In this function, the bridge should create a job subject element. The new job subject element should be inserted
    //! in the dictionary model as a child of the iModel's root subject.
    //! <p>_InitializeJob may import domain schemas.
    //! <p>_InitializeJob may also create some or all of the permanent infrastructure that the bridge will need in the initial conversion and in future updates.
    //! That includes partitions, other kinds of subjects, document models, fixed categories, and other fixed definitions that the bridge knows that it 
    //! will need and that apply to any data source.
    //! In the same way, the bridge can also add any domains and import any ECSchemas that it knows that it will need in the future.
    //! (The platform takes care of importing the BIS core domain and schema. The bridge does not have to do that.)
    //! If the bridge may convert any external ECSchemas during this initial conversion, then _InitializeJob is the time to prepare for that.
    //! @note The job subject's code must have a name that is unique among all job subjects that are chlidren of the root subject.
    //! @note All other subjects, partitions, and definitions created by a bridge for a given data source should be scoped to its job subject. See @ref ANCHOR_BridgeJob "bridge job".
    //! @return null if the bridge could not create a job subject for this bridge and this data source.
    virtual SubjectCPtr _InitializeJob() = 0;

    //! Convert the source data to BIS and write to the BIM. 
    //! See @ref ANCHOR_TypicalBridgeConversionLogic "iModelBridge conversion logic pattern" for details on how to write bridge conversion logic.
    //! _ConvertToBim must transform source data into the @ref ANCHOR_iModelUnitsAndGCS "Units and GCS" of the BIM.
    //! <h2>Initial Conversion</h2>
    //! The iModelBridge::Params::IsUpdating property will return @c false if this is the initial conversion.
    //! For the initial conversion, the bridge should generally convert all source content.
    //! <h2>Incremental Updates</h2>
    //! The iModelBridge::Params::IsUpdating property will return @c true if this is an update.
    //! For updates, the bridge should convert only the source data that has changed since the last call to _ConvertToBim.
    //! <p>
    //! The bridge's _ConvertToBim logic can be the same for the initial conversion and for updates. _ConvertToBim 
    //! should always use a @ref GROUP_syncinfo "change detector" and simply adopt a nop change detector in the case of the initial conversion. 
    //! <p>
    //! @note The bridge should create all subjects, views, categories, and other definitions as children (perhaps indirectly) of its @ref ANCHOR_BridgeJob "job subject". 
    //! @note _ConvertToBim must not call SaveChanges on the BIM. If it does, the job will be terminated and all changes rolled back and lost.
    //! @note _ConvertToBim must not attempt to create temp tables or call Db::AttachDb. See #_OnConvertToBim.
    //! @param[in] jobSubject The bridge's job subject, as returned by _FindJob or _IntializeJob.
    //! @return non-zero error status if the bridge cannot conversion the BIM. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues" 
    //! @see _OnConvertToBim
    virtual BentleyStatus _ConvertToBim(SubjectCR jobSubject) = 0;

    //! Returns true if the DgnDb itself is being generated from an empty file (rare).
    bool IsCreatingNewDgnDb() {return _GetParams().IsCreatingNewDgnDb();}

    //! @name Helper functions
    //! @{

    IMODEL_BRIDGE_EXPORT static WString GetArgValueW (WCharCP arg);
    IMODEL_BRIDGE_EXPORT static Utf8String GetArgValue (WCharCP arg);

    //! @}
    };

//=======================================================================================
//! Base class for iModel bridges. This base class has a Params member. 
//! This base class assumes that the bridge has no additional parameters of its own.
//! This base class implements the bridge methods that are used to store and release a reference to the BIM.
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridgeBase : iModelBridge
{
protected:
    Params m_params;
    DgnDbP m_db;
    
public:
    //! Get a reference to the BIM that the bridge is writing.
    DgnDbR GetDgnDbR() {return *m_db;}
    
    //! Store a reference to the BIM that the bridge is to write.
    BentleyStatus _OnConvertToBim(DgnDbR db) override {m_db = &db; return BSISUCCESS;}

    //! Release the reference to the BIM when the conversion is over.
    void _OnConvertedToBim(BentleyStatus) override {m_db = nullptr;}

    //! Get a reference to the bridge's parameters. This base class assumes that the bridge
    //! has no additional parameters of its own.
    Params& _GetParams() override {return m_params;}
    };

END_BENTLEY_DGN_NAMESPACE

/*! \typedef typedef BentleyApi::Dgn::iModelBridge* (*T_iModelBridge_getInstance)();
 *  \brief The signature of the extern "C" function that a shared library must implement in order to create and return its iModelBridge object.
 *  Note that the iModelBridge_getInstance function must have extern "C" linkage.
 *  Note that the iModelBridge_getInstance function must be exported.
 *  \return An iModelBridge object.
 * @ingroup GROUP_iModelBridge
 */
extern "C" 
    {
    typedef BentleyApi::Dgn::iModelBridge* (*T_iModelBridge_getInstance)();
    };
