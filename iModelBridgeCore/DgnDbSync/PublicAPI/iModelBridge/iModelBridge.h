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
#include <Bentley/SHA1.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnView/DgnViewLib.h>
#include <iModelBridge/iModelBridgeFwkTypes.h>

#ifdef __IMODEL_BRIDGE_BUILD__
    #define IMODEL_BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODEL_BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

//! @brief Begins a table of translatable strings contained in an iModelBridge.
//! This macro defines a struct called STRUCT_NAME that has entries for a group of transatable strings.
//! The struct has a GetString function that calls iModelBridge::GetString using the specified NAMESPACE_NAME.
//! The struct also has a GetNameSpace function that returns NAMESPACE_NAME as a constant string.
#define IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(STRUCT_NAME,NAMESPACE_NAME)\
    struct STRUCT_NAME\
        {\
        typedef BeSQLite::L10N::NameSpace NameSpace;\
        typedef BeSQLite::L10N::StringId StringId;\
        static NameSpace GetNameSpace() {return NameSpace(#NAMESPACE_NAME);}\
        static Utf8String GetString(StringId id) {return iModelBridge::L10N::GetString(GetNameSpace(), id);}\
        static WString    GetStringW(StringId id) {return WString(GetString(id).c_str(), BentleyCharEncoding::Utf8);}\

//! @brief Eneds a table of translatable strings contained in an iModelBridge.
#define IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END };

BEGIN_BENTLEY_DGN_NAMESPACE

struct iModelBridgeFwk;
struct iModelBridgeSacAdapter;

/**
@addtogroup GROUP_iModelBridge

An iModel "bridge" converts data from an external data source into a BIM.

See @ref ANCHOR_TypicalBridgeConversionLogic "iModelBridge conversion logic pattern" for details on how to write bridge conversion logic.

A bridge is normally called as part of a @ref ANCHOR_iModelBridgeJobOverview "job" by the @ref GROUP_iModelBridgeFwk "iModelBridge Framework"
as part of a process of reacting to changes to source documents and then converting those changes and updating an iModel.
A bridge may also be called by a @ref BentleyApi::Dgn::iModelBridgeSacAdapter "standalone converter" in order to write to a standalone dgndb file.
A @ref BentleyApi::Dgn::iModelBridgeSacAdapter "standalone converter" may be written in order to @em test a bridge's conversion logic.

A bridge must be @ref ANCHOR_BridgeLoading "implemented by a shared library", and it must @ref ANCHOR_BridgeRegistration "be registered" and must report its document @ref iModelBridge_getAffinity "affinity".

Also see @ref ANCHOR_BridgeConfig "bridge-specific configuration".

<h2>Data Conversion</h2>

Here is a summary of the process that is conducted by the framework when running bridges to convert data, starting from a spatial root file. 
The process shown here is for the case of a bridge doing an incremental update. The process of creating a new BIM is essentially similar. The differences are noted below.

-# The framework first registers the DgnPlatformLib::Host
-# The framework signs into iModelHub and gets access to the specified iModel
-# The framework acquires a briefcse from iModelHub, if necessary, and pulls and merges revisions to make sure it is up to date.
-# The framework @ref ANCHOR_BridgeLoading "loads the bridge dll specified for the job and asks it to create a bridge object".
The framework then makes the following calls on the bridge object:
-# iModelBridge::_ParseCommandLine (standalone converters only)
-# iModelBridge::_Initialize
-# The framework opens the BIM.
    -# The framework calls BentleyApi::Dgn::DgnDomains::ImportSchemas, if necessary, in order to ensure that the domains
registered by the bridge in its _Initialize method are imported into the BIM and are up to date.
-# iModelBridge::_OnConvertToBim
-# iModelBridge::_OpenSource
-# Find or initialize the @ref ANCHOR_BridgeJobSubject "job subject"
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

<h3>Creating a New Repository</h3>

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

Also see iModelBridge::GetTransformCorrectionFromJobSubject for an additional transform that the bridge may be required to apply to all converted spatial data.

@anchor ANCHOR_TrackingDocuments
<h2>Tracking Documents</h2>

A bridge must track documents. That means that the bridge must record documents in its syncinfo file, and must use each document's syncinfo ID as the scope when adding 
syncinfo records to record elements that were created from that document. The iModelBridgeWithSyncInfoBase base class has convenience methods to make this easy. See 
iModelBridgeWithSyncInfoBase::RecordDocument and the ToyTileBridge::_ConvertToBim example.

A bridge should identify a document by using the GUID assigned to it by its home DMS, if available. The document's GUID is more stable than the document's local filename. 
See iModelBridge::QueryDocumentGuid.

Use the GUID for tracking the document in syncinfo.

Use the document GUID for generating codes that relate to the document, especially for the @ref ANCHOR_BridgeJobSubject "Job Subject" element. A job subject must be specific
to the root document. Here is an example of how to compute a unique job subject name.
@verbatim
Utf8String ToyTileBridge::ComputeJobSubjectName()
    {
    Utf8String docId;
    BeGuid docGuid = QueryDocumentGuid(_GetParams().GetInputFileName());
    if (docGuid.IsValid())
        docId = docGuid.ToString();                                 // Use the document GUID, if available, to ensure a stable and unique Job subject name.
    else
        docId = Utf8String(_GetParams().GetInputFileName());        // fall back on using local file name -- not as stable!

    Utf8String name(GetRegistrySubKey()); // start job name with my registry subkey. That will help ensure uniqueness.
    name.append(":");
    name.append(docId.c_str());
    return name;
    }
@endverbatim

@anchor ANCHOR_Provenance
<h2>Provenance</h2>
An iModelBridge is responsible for storing provenance data that relates elements in the iModel to information in the source documents.
Currently, provenance for model elements is required. Provenance for other kinds of elements is optional.

<h3>PartitionOriginatesFromRepository</h3>
A bridge must relate each physical model that it creates to source document(s) that it used to create that model.
Specifically, each bridge must create a PartitionOriginatesFromRepository ECRelationship from the InformationPartitionElement element that represents the model
to one or more RepositoryLink elements that describe the source document. See iModelBridge::WriteRepositoryLink and iModelBridge::InsertPartitionOriginatesFromRepositoryRelationship.

<h2>Detecting Deleted Documents</h2>

A bridge must be able to detect deleted files and clean up the iModel as appropriate. See _DetectDeletedDocuments. The algorithm must be along these lines
@verbatim
BentleyStatus SampleBridge::_DetectDeletedDocuments()
    {
    auto documentsInSyncInfo = GetSyncInfo().MakeIterator( ... document records ... );
    
    for (auto documentInSyncInfo : documentsInSyncInfo)
        {
        Utf8String docId = documentInSyncInfo.GetSourceIdentity().GetId();

        if (IsDocumentAssignedToJob(docId))   // This is how to check if the document still exists.
            continue;                         //  If it does, do nothing.

        // Infer that that document was deleted. 

        // Delete related elements and models in the briefcase
        ...

        // Delete corresponding items from syncinfo
        GetSyncInfo().DeleteAllItemsInScope(documentInSyncInfo.GetSyncInfoID());
        GetSyncInfo().DeleteItem(documentInSyncInfo.GetSyncInfoID());
        }

    return BSISUCCESS;
    }

@endverbatim

The iModelBridgeWithSyncInfoBase base class implements this algorithm to detect deleted documents and clean up syncinfo, delegating the briefcase cleanup to the 
derived class. So, if you subclass your bridge from iModelBridgeWithSyncInfoBase, you only need to implement a callback that can find and delete elements and models 
that the bridge previously converted from the specified document.

<h2>Bridge Assets</h2>
The bridge's assets are in the directory identified by iModelBridge::Params::GetAssetsDir.
The bridge's assets are separate from the framework's assets. The DgnPlatformLib::Host::IKnownLocationsAdmin::GetDgnPlatformAssetsDirectory points to the assets of the framework, not the bridge.

<h2>Translatable Strings</h2>
A bridge should call BentleyApi::Dgn::iModelBridge::L10N::GetString to look up its own translatable strings.
A bridge must override _SupplySqlangRelPath to specify the location of its .db3 file, relative to its own assets directory.
A bridge must define its own translatable string tables using the IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START macro, and @em not the BENTLEY_TRANSLATABLE_STRINGS_START macro.

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

    //! The bridge's affinity to some source file.
    typedef iModelBridgeAffinityLevel Affinity;

    //! Identifies a bridge that has some affinity to a requested source file
    typedef iModelBridgeWithAffinity BridgeAffinity;

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

    //! Interface implemented by an agent that can get the document properties associated with a local file and can check of a file is assigned to a specified bridge
    struct IDocumentPropertiesAccessor
        {
        //! Check if the specified file is assigned to the specified bridge.
        //! @param fn   The name of the file that is to be converted
        //! @param bridgeRegSubKey The registry subkey that identifies the bridge
        //! @return true if the specified bridge should convert the specified file
        virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;

        //! Get the URN and other properties for a document from the host document control system (e.g., ProjectWise)
        //! @param[in] fn   The name of the file that is to be converted
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @return non-zero error status if doc properties could not be found for this file.
        virtual BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn) = 0;

        //! Look up a document's properties by its document GUID. 
        //! @param[out] props Properties that may be assigned to a document by its home document control system (DCS)
        //! @param[out] localFilePath The local filepath of the staged document
        //! @param[in] docGuid  The document's GUID in its home DCS
        //! @return non-zero error status if the GUID is not in the table of registered documents.
        virtual BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) = 0;
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
        bvector<BeFileName> m_drawingAndSheetFiles;
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
        IDocumentPropertiesAccessor* m_documentPropertiesAccessor = nullptr;
        WString m_thisBridgeRegSubKey;
        Transform m_spatialDataTransform;
        DgnElementId m_jobSubjectId;

        void SetIsCreatingNewDgnDb(bool b) {m_isCreatingNewDb=b;}
        IMODEL_BRIDGE_EXPORT void SetReportFileName();
        void SetThumbnailTimeout(BeDuration timeout) {m_thumbnailTimeout = timeout;}

      public:
        IMODEL_BRIDGE_EXPORT Params();

        //! @name Helper functions
        //! @{

        IMODEL_BRIDGE_EXPORT static BentleyStatus ParseGcsSpec(GCSDefinition& gcs, Utf8StringCR gcsParms);
        IMODEL_BRIDGE_EXPORT static BentleyStatus ParseGCSCalculationMethod(GCSCalculationMethod& cm, Utf8StringCR value);
        IMODEL_BRIDGE_EXPORT static BentleyStatus ParseTransform(Transform& cm, Utf8StringCR value);

        //! @}

        IMODEL_BRIDGE_EXPORT BentleyStatus Validate();

        //! @private
        bool IsCreatingNewDgnDb() const {return m_isCreatingNewDb;} //!< True in the rare case when the bridge is asked to populate a new local file, in preparation for pushing a new repository to the iModelHub
        void SetIsUpdating(bool b) {m_isUpdating=b;}
        bool IsUpdating() const {return m_isUpdating;} //!< True if the bridge is updating an existing job subject and its contents from a previous conversion.
        BeFileNameCR GetBriefcaseName() const {return m_briefcaseName;} //!< The name of the BIM that is being updated
        BeFileNameCR GetInputFileName() const {return m_inputFileName;} //!< The name of the input file that is to be read and converted and/or scanned for changes.
        void SetInputFileName(BeFileNameCR fn) {m_inputFileName=fn;} //!< Set the name of the input file that is to be read and converted and/or scanned for changes.
        BeFileNameCR GetAssetsDir() const {return m_assetsDir;} //!< The bridge library's assets directory
        BeFileNameCR GetLibraryDir() const {return m_libraryDir;} //!< The directory from which the bridge library itself was loaded
        BeFileNameCR GetDrawingsDirs() const {return m_drawingsDirs;} //!< The top-level directory to scan for other files that may contain drawings and sheets
        bvector<BeFileName> const& GetDrawingAndSheetFiles() const {return m_drawingAndSheetFiles;} //!< The list of files to search for drawings and sheets
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
        void SetBridgeRegSubKey(WStringCR str) {m_thisBridgeRegSubKey=str;}
        WString GetBridgeRegSubKey() const {return m_thisBridgeRegSubKey;}
        Utf8String GetBridgeRegSubKeyUtf8() const {return Utf8String(m_thisBridgeRegSubKey);}
        void SetDocumentPropertiesAccessor(IDocumentPropertiesAccessor& c) {m_documentPropertiesAccessor = &c;}
        void ClearDocumentPropertiesAccessor() {m_documentPropertiesAccessor = nullptr;}
        IDocumentPropertiesAccessor* GetDocumentPropertiesAccessor() const {return m_documentPropertiesAccessor;}
        void SetSpatialDataTransform(Transform const& t) {m_spatialDataTransform = t;} //!< Optional. The transform that the bridge job should pre-multiply to the normal transform that is applied to all converted spatial data.
        TransformCR GetSpatialDataTransform() const {return m_spatialDataTransform;} //!< The transform, if any, that the bridge job should pre-multiply to the normal transform that is applied to all converted spatial data. See iModelBridge::GetJobTransform
        void SetJobSubjectId(DgnElementId eid) {m_jobSubjectId = eid;}  //!< @private called by framework
        DgnElementId GetJobSubjectId() const {return m_jobSubjectId;} //!< Identifies the job Subject element

	    //! Check if a document is assigned to this job or not.
        //! @param docId    Identifies the document uniquely in the source document management system. Normally, this will be a GUID (in string form). Some standalone converters may use local filenames instead.
	    IMODEL_BRIDGE_EXPORT bool IsDocumentAssignedToJob(Utf8StringCR docId) const;

	    //! Check if the specified file is assigned to this bridge or not.
	    IMODEL_BRIDGE_EXPORT bool IsFileAssignedToBridge(BeFileNameCR fn) const;

	    //! Get the document GUID for the specified file, if available.
	    //! @param localFileName    The filename of the source file.
	    //! @return the document GUID, if available.
	    IMODEL_BRIDGE_EXPORT BeSQLite::BeGuid QueryDocumentGuid(BeFileNameCR localFileName) const;
        };

    private:
        friend struct iModelBridgeFwk;
        friend struct iModelBridgeSacAdapter;

    //! @name Helper functions
    //! @{

    //! @private
    //! Create a new local repository and allow this bridge to populate it. This called by the framework as part of creating a new repository.
    //! @param[out] jobModels   The set of models that were created by the job.
    //! @param[in]  rootSubjectDescription. Optional. If supplied, this becomes the description of the new root subject.
    //! @return nullptr if the db could not be created or populated.
    //! @note that it is up to the caller to call SaveChanges
    IMODEL_BRIDGE_EXPORT DgnDbPtr DoCreateDgnDb(bvector<DgnModelId>& jobModels, Utf8CP rootSubjectDescription);

    //! @private
    //! Open an existing BIM for read-write, possibly doing a schema upgrade on it.
    //! @param[out] dbres  If the BIM cannot be opened or upgraded, return the error status here.
    //! @param[out] madeSchemaChanges Set to true if OpenDgnDb imported or upgraded schemas.
    //! @return Opened BIM or an invalid ptr if the BIM could not be opened.
    IMODEL_BRIDGE_EXPORT DgnDbPtr OpenBim(BeSQLite::DbResult& dbres, bool& madeSchemaChanges, bool& hasDynamicSchemaChange);

    //! @private
    //! Convert source data to an existing BIM. This is called by the framework as part of a normal conversion.
    //! @param[in] db The BIM to be updated
    //! @param[in] detectDeletedFiles If true, the bridge will also detect deleted files and delete content that was extracted from them.
    //! @return non-zero error status if the bridge cannot convert the BIM. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    //! @note The caller must check the return status and call SaveChanges on success or AbandonChanges on error.
    //! @see OpenBim
    IMODEL_BRIDGE_EXPORT BentleyStatus DoConvertToExistingBim(DgnDbR db, bool detectDeletedFiles);

    //! @}

public:
    //! Supply the relative path to the .db3 file that contains translatable strings used by the bridge.
    //! Must be relative to the directory returned by iModelBridge::Params::GetAssetsDir.
    //! @see GetString
    virtual WString _SupplySqlangRelPath() = 0;

    struct L10N
        {
        //! Initialize the bridge L10N with the specified files. This is normally called by iModelBridgeFwk on behalf of a bridge.
        //! @see _SupplySqlangRelPath
        IMODEL_BRIDGE_EXPORT static BentleyStatus Initialize(BeSQLite::L10N::SqlangFiles const&);

        IMODEL_BRIDGE_EXPORT static void Terminate();

        //! Retrieve a localized string by Namespace and Name. This function looks in both the db3 file specified by _SupplySqlangRelPath and in the platform/framework db3.
        //! @param[in] nameSpace the namespace of the string.
        //! @param[in] name the name of the string.
        //! @note internal framework strings are also searched
        //! @see _SupplySqlangRelPath
        IMODEL_BRIDGE_EXPORT static Utf8String GetString(BeSQLite::L10N::NameSpace nameSpace, BeSQLite::L10N::StringId name);
        };

    //! Return the Params struct that holds all of the parameters for the bridge.
    //! @note The @ref GROUP_iModelBridgeFwk "framework" may use this function to update bridge parameters in the course of executing a job.
    //! (For example, the framework may call iModelBridge::_GetParams().SetIsUpdating (false) after discoverting that there is no existing
    //! job.) Therefore, _GetParams must return a reference to the live copy of the params, and the bridge must get parameters from this live copy as it runs,
    //! in order to see the correct values.
    virtual Params& _GetParams() = 0;

    Params const& GetParamsCR() const {return const_cast<iModelBridge*>(this)->_GetParams();}

    //! Print a message describing command line arguments
    virtual void _PrintUsage() {}

    //! Parse an individual command line argument. This is the way to handle bridge-specific arguments.
    //! @note standalone converters only
    //! @param iarg the index of the command line argument to be parsed
    //! @param argc the number of command line arguments
    //! @param argv the command line arguments
    //! @return an indication of whether the argument was handled and if so if it was valid
    virtual CmdLineArgStatus _ParseCommandLineArg(int iarg, int argc, WCharCP argv[]) {return CmdLineArgStatus::NotRecognized;}

    //! The bridge should parse any bridge-specific command-line arguments. The framework takes care of the standard
    //! bridge command-line arguments. Note that the BIM is not yet open.
    //! @note standalone converters only
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
    //! @param db   The BIM or local DgnDb that is being updated
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

    //! Try to find an existing @ref ANCHOR_BridgeJobSubject "job subject" in the BIM.
    //! This is called prior to _ConvertToBim.
    //! <p>Normally, the bridge should look up a job subject element by its code.
    //! <p>If _FindJob does detect an existing job, that means that this is an update.
    //! In this case, the bridge should prepare for update-style change detection.
    //! @return null if the BIM does not contain a job subject for this bridge and this data source.
    virtual SubjectCPtr _FindJob() = 0;

    //! Create a @ref ANCHOR_BridgeJobSubject "job subject" element and all fixed infrastructure elements and models below it.
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
    //! @note All other subjects, partitions, and definitions created by a bridge for a given data source should be scoped to its job subject. See @ref ANCHOR_BridgeJobSubject "bridge job subject".
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
    //! @note The bridge should create all subjects, views, categories, and other definitions as children (perhaps indirectly) of its @ref ANCHOR_BridgeJobSubject "job subject".
    //! @note _ConvertToBim must not call SaveChanges on the BIM. If it does, the job will be terminated and all changes rolled back and lost.
    //! @note _ConvertToBim must not attempt to create temp tables or call Db::AttachDb. See #_OnConvertToBim.
    //! @param[in] jobSubject The bridge's job subject, as returned by _FindJob or _IntializeJob.
    //! @return non-zero error status if the bridge cannot conversion the BIM. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    //! @see _OnConvertToBim
    virtual BentleyStatus _ConvertToBim(SubjectCR jobSubject) = 0;

    //! Returns true if the DgnDb itself is being generated from an empty file (rare).
    bool IsCreatingNewDgnDb() {return _GetParams().IsCreatingNewDgnDb();}

    //! Get the transform, if any, that the bridge should pre-multiply to the normal transform that it computes and applied to all converted spatial data.
    //! The job's spatial data transform can come from a command-line parameter or from a property of the job subject element in the iModel. In case both are specified,
    //! the transform specified on the command line is pre-multiplied to the transform specified by the job subject.
    //! @param[in] params The bridge's parameters
    //! @param[in] jobSubject The bridge's job subject.
    //! @return the transform to apply to spatial data or the identity matrix if no job transform was found
    IMODEL_BRIDGE_EXPORT static Transform GetSpatialDataTransform(Params const& params, SubjectCR jobSubject);

    Transform GetSpatialDataTransform(SubjectCR jobSubject) {return GetSpatialDataTransform(_GetParams(), jobSubject);}

    //!This function called before _ConvertToBim method. It provides bridges an oppurtunity to post a schema change Changeset into the imodelhub. This makes the
    //!revision comparison operations work well in sqlite. Return true if a schema change was detected.
    virtual bool _UpgradeDynamicSchema(DgnDbR db) { return false; }

    //! @name Document Properties Helper Functions
    //! @{

    //! Check if a document is assigned to this job or not.
    //! @param docId    Identifies the document uniquely in the source document management system. Normally, this will be a GUID (in string form). Some standalone converters may use local filenames instead.
    bool IsDocumentAssignedToJob(Utf8StringCR docId) const {return GetParamsCR().IsDocumentAssignedToJob(docId);}

    //! Check if the specified file is assigned to this bridge or not.
    bool IsFileAssignedToBridge(BeFileNameCR fn) const {return GetParamsCR().IsFileAssignedToBridge(fn);}

    //! Get the document GUID for the specified file, if available.
    //! @param localFileName    The filename of the source file.
    //! @return the document GUID, if available.
    BeSQLite::BeGuid QueryDocumentGuid(BeFileNameCR localFileName) const {return GetParamsCR().QueryDocumentGuid(localFileName);}

    // @private
    IMODEL_BRIDGE_EXPORT static LinkModelPtr GetRepositoryLinkModel(DgnDbR db, bool createIfNecessary = true);

    // @private
    //! Make a RepositoryLink Element that refers to a specified source file. 
    //! This function will attempt to set the properties of the RepositoryLink element from the DMS document properties of the source file. 
    //! This function will call Params::IDocumentPropertiesAccessor to get the document properties. If found, 
    //! the CodeValue for the RepositoryLink Element will be set to the document's GUID, and the element's URI property will be set to the document's URN.
    //! If DMS document properties cannot be found, this function will use the supplied defaultCode and defaultURN to set up the RepositoryLink.
    //! @param db               The briefcase.
    //! @param params           The bridge params
    //! @param localFileName    The filename of the source file.
    //! @param defaultCode      The CodeValue to use if a document GUID cannot be found
    //! @param defaultURN       The URN to use if no URN can be found
    //! @return An editable RepositoryLink element. It will have a valid DgnElementId if a RepositoryLink Element with the same code already exists in db.
    IMODEL_BRIDGE_EXPORT static RepositoryLinkPtr MakeRepositoryLink(DgnDbR db, Params const& params, BeFileNameCR localFileName, Utf8StringCR defaultCode, Utf8StringCR defaultURN);

    // @private
    IMODEL_BRIDGE_EXPORT static void GetRepositoryLinkInfo(DgnCode& code, iModelBridgeDocumentProperties& docProps, DgnDbR db, Params const& params, 
                                                BeFileNameCR localFileName, Utf8StringCR defaultCode, Utf8StringCR defaultURN, LinkModelR lmodel);
    // @private
    IMODEL_BRIDGE_EXPORT static SHA1 ComputeRepositoryLinkHash(RepositoryLinkCR);

    //! Utility to create an instance of an ECRelationship (for non-Navigation relationships).
    IMODEL_BRIDGE_EXPORT static DgnDbStatus InsertLinkTableRelationship(DgnDbR db, Utf8CP relClassName, DgnElementId source, DgnElementId target, Utf8CP schemaName = BIS_ECSCHEMA_NAME);

    //! Create a "PartitionOriginatesFromRepository" relationship between a partition model and a RepositoryLink element.
    //! @param db               The briefcase.
    //! @param informationPartitionElementId  The element that represents the partition model.
    //! @param repoLinkElementId The RepositoryLinkElement
    //! @return non-zero status if the relationship instance could not be inserted in the briefcase.
    static DgnDbStatus InsertPartitionOriginatesFromRepositoryRelationship(DgnDbR db, DgnElementId informationPartitionElementId, DgnElementId repoLinkElementId)
        {
        return InsertLinkTableRelationship(db, BIS_REL_PartitionOriginatesFromRepository, informationPartitionElementId, repoLinkElementId);
        }

    //! @}

    //! @name Helper functions
    //! @{

    IMODEL_BRIDGE_EXPORT static WString GetArgValueW (WCharCP arg);
    IMODEL_BRIDGE_EXPORT static Utf8String GetArgValue (WCharCP arg);

    //! Write a message to the issues file. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    IMODEL_BRIDGE_EXPORT void ReportIssue(WStringCR);

    //! Write a message to the issues file. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    IMODEL_BRIDGE_EXPORT void ReportIssue(Utf8StringCR msg) {ReportIssue(WString(msg.c_str(), true));}

    //! @}

    //! @name Post-processing Callbacks
    //! @{

    //! The bridge should detect deleted documents and delete all all models and elements in the briefcase that came from deleted documents.
    //! @return non-zero error status if the bridge cannot conversion the BIM. See @ref ANCHOR_BridgeIssuesAndLogging "reporting issues"
    virtual BentleyStatus _DetectDeletedDocuments() = 0;

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

    //! Look up the job Subject element
    SubjectCPtr GetJobSubject() const {return m_db->Elements().Get<Subject>(m_params.GetJobSubjectId());}

    //! Return the job's spatial data transform
    Transform GetSpatialDataTransform() {return iModelBridge::GetSpatialDataTransform(*GetJobSubject());}
    };

END_BENTLEY_DGN_NAMESPACE

/*! \typedef typedef BentleyApi::Dgn::iModelBridge* T_iModelBridge_getInstance(wchar_t const* regSubKey);
 *  \brief The signature of the <code>iModelBridge_getInstance</code> function that a shared library must implement in order to @ref iModelBridge_getInstance "supply a bridge to the framework".
 *  Note that the iModelBridge_getInstance function must have extern "C" linkage and must be exported.
 *  \param[in] regSubKey The @ref ANCHOR_BridgeRegistration "subkey" of the bridge to load. This is the string returned by the T_iModelBridge_getAffinity function.
 *  \return An iModelBridge object.
 * @ingroup GROUP_iModelBridge
 */
extern "C"
    {
    typedef BentleyApi::Dgn::iModelBridge* T_iModelBridge_getInstance(wchar_t const* regSubKey);
    };

