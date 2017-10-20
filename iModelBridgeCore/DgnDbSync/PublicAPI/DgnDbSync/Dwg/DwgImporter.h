/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgImporter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnDbSync/Dwg/DwgDb/DwgDbDatabase.h>
#include <DgnDbSync/Dwg/DwgDb/DwgResBuf.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbObjects.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbEntities.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbSymbolTables.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDrawables.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <BeSQLite/L10N.h>
#include <BeXml/BeXml.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnDbSync/DgnDbSync.h>
#include <DgnDbSync/Dwg/DwgSyncInfo.h>
#include <iModelBridge/iModelBridge.h>

USING_NAMESPACE_DWGDB

BEGIN_DGNDBSYNC_DWG_NAMESPACE


//=======================================================================================
//! Base class for options that control how to merge various named data structures that match specified properties
// @bsiclass                                                    Sam.Wilson      12/13
//=======================================================================================
struct ImportRule
{
protected:

    bool m_hasNewName;    //!< Was a new name specified?
    Utf8String m_newName; //!< The new name to assign to matching items.

    union
        {
        struct
            {
            uint32_t file:2; //!< Match only levels from the specified file?
            uint32_t name:2; //!< Match only levels with the specified name?
            };
        uint32_t allBits;
        } m_matchOnBase;

    Utf8String m_name; //!< The name to match
    Utf8String m_file; //!< The name of the file to match

public:
    //! Constructs an empty options object with no matching criteria specified.
    ImportRule() {m_hasNewName=false;m_matchOnBase.allBits=0;}

    Utf8String ToString() const;

    //! Parse from XML configuration
    void InitFromXml(BeXmlNode&);

    //! Test if this merge option should be applied to the item with the specified name
    //! @return \a true if this merge option should be applied to \a upgradeLevel
    bool Matches(Utf8StringCR name, BeFileNameCR filename) const;

    //! Compute new name
    //! @param[out] newName The new name, if successful
    //! @param[in] baseFilename The file that is being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, BeFileNameCR baseFileName) const;

    //! Compute new name
    //! @param[out] newName The new name, if successful
    //! @param[in] modelName The model that is being imported
    //! @param[in] baseFilename The base name of the parent file containing the model being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, Utf8StringCR modelName, BeFileNameCR baseFilename) const;
};  // ImportRule

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/17
+===============+===============+===============+===============+===============+======*/
struct ResolvedModelMapping
{
private:
    // the key: an object ID of model/paperspace block, xref insert or raster attachment.
    DwgDbObjectId                   m_dwgModelInstanceId;
    DgnModelP                       m_model;
    DwgSyncInfo::DwgModelMapping    m_mapping;

public:
    //! construct invalid mappings
    ResolvedModelMapping () : m_model(nullptr) {}
    ResolvedModelMapping (DwgDbObjectIdCR id) : m_dwgModelInstanceId(id), m_model(nullptr) {}
    //! construct valid mapping
    ResolvedModelMapping (DwgDbObjectIdCR id, DgnModelP model, DwgSyncInfo::DwgModelMapping const& map) : m_dwgModelInstanceId(id), m_model(model), m_mapping(map) {}

    bool            operator < (ResolvedModelMapping const &o) const { return m_dwgModelInstanceId < o.m_dwgModelInstanceId; }
    bool            IsValid () const { return m_dwgModelInstanceId.IsValid() && m_mapping.IsValid(); }
    DwgDbObjectId   GetModelInstanceId () const { return m_dwgModelInstanceId; }
    void            SetModelInstanceId (DwgDbObjectIdCR id) { m_dwgModelInstanceId=id; }
    DgnModelP       GetModel () { return m_model; }
    void            SetModel (DgnModelP model) { m_model=model; }
    DwgSyncInfo::DwgModelMapping GetMapping () const { return m_mapping; }
    void            SetMapping (DwgSyncInfo::DwgModelMapping const& m) { m_mapping=m; }
    Transform       GetTransform () const { return  m_mapping.GetTransform(); }
    DwgSyncInfo::DwgModelSyncInfoId GetModelSyncInfoId () const { return m_mapping.GetDwgModelSyncInfoId(); }
};  // ResolvedModelMapping
typedef bmultiset<ResolvedModelMapping>     T_DwgModelMapping;

//=======================================================================================
//! An import "job" definition, including its subject element.
//! @bsiclass                                                    Sam.Wilson      11/16
//=======================================================================================
struct ResolvedImportJob
{
protected:
    DwgSyncInfo::ImportJob m_mapping;
    SubjectCPtr m_jobSubject;
public:
    ResolvedImportJob () {}
    ResolvedImportJob (DwgSyncInfo::ImportJob const& j, SubjectCR s) : m_mapping(j), m_jobSubject(&s) {}
    ResolvedImportJob (SubjectCR s) : m_jobSubject(&s) {}

    bool IsValid() const {return m_jobSubject.IsValid();}
    void FromSelect(BeSQLite::Statement& stmt) {m_mapping.FromSelect(stmt); /* WIP_IMPORT_JOB -- resolve the subject */}
    DwgSyncInfo::ImportJob& GetImportJob() {return m_mapping;}
    //! Get the root model for this job
    DwgSyncInfo::DwgModelSyncInfoId GetDwgModelSyncInfoId() const { return m_mapping.GetDwgModelSyncInfoId(); }
    //! Get the job subject
    SubjectCR GetSubject() const {BeAssert(IsValid()); return *m_jobSubject;}
    //! Get the type of converter that created this job
    DwgSyncInfo::ImportJob::Type GetImporterType() const {return m_mapping.GetType();}
    //! Get the name prefix that is used by this job
    Utf8StringCR GetNamePrefix() const { return m_mapping.GetPrefix(); }
};  // ResolvedImportJob

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/17
+===============+===============+===============+===============+===============+======*/
struct IDwgChangeDetector
{
    enum class ChangeType
    {
        None,       // element needs no change
        Update,     // element is changed and needs update
        Insert      // new element needs inserted
    };

    //! The results of calling _IsElementChanged
    struct DetectionResults
    {
    private:
        ChangeType m_changeType;
        DwgSyncInfo::DwgObjectMapping m_dwgObjectMapping;
        DwgSyncInfo::DwgObjectProvenance m_currentObjectProvenance;

    public:
        DetectionResults() : m_changeType(ChangeType::None) {}
        DgnElementId    GetExistingElementId() const { return m_dwgObjectMapping.m_elementId; }
        ChangeType  GetChangeType () const { return m_changeType; }
        void        SetChangeType (ChangeType c) { m_changeType = c; }
        DwgSyncInfo::DwgObjectMapping const& GetObjectMapping () const { return m_dwgObjectMapping; }
        void SetObjectMapping (DwgSyncInfo::DwgObjectMapping const& m) { m_dwgObjectMapping = m; }
        DwgSyncInfo::DwgObjectProvenance const& GetObjectProvenance () const { return m_currentObjectProvenance; }
        void SetObjectProvenance (DwgSyncInfo::DwgObjectProvenance const& p) { m_currentObjectProvenance = p; }
    };  // DetectionResults

    virtual ~IDwgChangeDetector () {}

    //! @name Setup/teardown 
    //! @{
    //! Called at the very start of conversion, before any files, models, or elements are read.
    virtual void _Prepare (DwgImporter&) = 0;
    //! Called at the very end of conversion, after all files, models, or elements have been processed by all phases.
    virtual void _Cleanup (DwgImporter&) = 0;
    //! @}

    //! @name Skipping unchanged data
    //! @{
    //! Called to check if the specified file could be skipped (i.e., because it has not changed) by checking timestamps that may be stored in the file.
    virtual bool _ShouldSkipFile (DwgImporter&, DwgDbDatabaseCR) = 0;
    //! Called to check if an entire model could be skipped (i.e., because no element in the model is changed).
    //! @note DwgImporter must not call this during the model-discovery step but only during the element-conversion step.
    virtual bool _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const&) = 0;

    //! Used to choose one of many existing entries in DwgSyncInfo
    typedef std::function<bool(DwgSyncInfo::ElementIterator::Entry const&, DwgImporter& converter)> T_DwgSyncInfoElementFilter;

    //! Called by a DwgImporter to detect if a DWG object is changed or new.
    //! @param[out] prov    Information about the element that can be used to decide how or if to update it in the bim and how to record the change in syncinfo
    //! @param[in] obj      A DWG object
    //! @param[in] model    Mapping info for the DWG model that contains this DWG object
    //! @param[in] filter   Optional. Chooses among existing elements in DwgSyncInfo
    //! @return true if the element is new or has changed.
    virtual bool _IsElementChanged (DetectionResults&, DwgImporter&, DwgDbObjectCR, ResolvedModelMapping const&, T_DwgSyncInfoElementFilter* f = nullptr) = 0;
    //! @}

    //! @name Recording DWG content seen (so that we can deduce deletes)
    //! @{
    //! Called whenever a DWG object is encountered, regardless of whether it is converted or not.
    virtual void _OnElementSeen (DwgImporter&, DgnElementId) = 0;

    void OnElementSeen (DwgImporter& importer, DgnElementP el) { if (el != nullptr) _OnElementSeen(importer, el->GetElementId()); }

    //! Called when a DWG model is discovered. This callback should be invoked during the model-discovery phase,
    //! before the elements in the specified model are converted.
    virtual void _OnModelSeen (DwgImporter&, ResolvedModelMapping const&) = 0;
    //! Called when a DWG model is first mapped into the BIM.
    //! @param rmm The DWG model and the DgnModel to which it is mapped
    //! @param attachment If the DWG model is a root model, this will be nullptr. Otherwise, this will be the attachment that was used to reach the DWG model.
    virtual void _OnModelInserted (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP xRef) = 0;

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit. Don't forget to call the ...End function when done.
    //! @{
    virtual void _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) = 0;  //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) = 0;          //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsEnd (DwgImporter&) = 0;
    virtual void _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) = 0;      //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) = 0;             //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsEnd (DwgImporter&) = 0;
    virtual void _DeleteDeletedMaterials (DwgImporter&) = 0;
    //! @}
};  // IDwgChangeDetector
typedef std::unique_ptr <IDwgChangeDetector>    T_DwgChangeDetectorPtr;


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgImporter
    {
    friend struct DwgBridge;
    friend struct DwgSyncInfo;
    friend struct DwgImportHost;
    friend struct ViewportFactory;
    friend class DwgProtocalExtension;
    friend class DwgRasterImageExt;
    friend class DwgPointCloudExExt;
    friend class DwgViewportExt;

public:
    //! Configuration for the conversion process
    struct Config
        {
    private:
        BeXmlDomPtr                 m_instanceDom;
        xmlXPathContextPtr          m_xpathContextRoot;
        DwgImporter&                m_dwgImporter;
        bmap<Utf8String,bool>       m_boolLUT;
        bmap<Utf8String,Utf8String> m_utf8LUT;
        bmap<Utf8String,double>     m_doubleLUT;
        bmap<Utf8String,int64_t>    m_int64LUT;
        BeFileName                  m_instanceFilename;

        void CacheOptions ();

    public:
        BeFileNameCR GetInstanceFilename () { return m_instanceFilename; }
        DwgImporter& GetDwgImporter () { return m_dwgImporter; }
        void ReadFromXmlFile ();

        Config (BeFileNameCR configFile, DwgImporter& importer) : m_instanceFilename(configFile), m_dwgImporter(importer) { m_xpathContextRoot=nullptr; }
        ~Config();

    public:
        DGNDBSYNC_EXPORT BeXmlDom* GetDom() const;
        DGNDBSYNC_EXPORT bool OptionExists(BentleyApi::Utf8CP optionName) const;
        DGNDBSYNC_EXPORT Utf8String GetOptionValueString(BentleyApi::Utf8CP optionName, Utf8CP defaultVal) const;
        DGNDBSYNC_EXPORT bool GetOptionValueBool(BentleyApi::Utf8CP optionName, bool defaultVal) const;
        DGNDBSYNC_EXPORT double GetOptionValueDouble(BentleyApi::Utf8CP optionName, double defaultVal) const;
        DGNDBSYNC_EXPORT int64_t GetOptionValueInt64(BentleyApi::Utf8CP optionName, int64_t defaultVal) const;
        DGNDBSYNC_EXPORT Utf8String GetXPathString(BentleyApi::Utf8CP xpathExpression, Utf8CP defaultVal) const;
        DGNDBSYNC_EXPORT bool GetXPathBool(BentleyApi::Utf8CP xpathExpression, bool defaultVal) const;
        DGNDBSYNC_EXPORT double GetXPathDouble(BentleyApi::Utf8CP xpathExpression, double defaultVal) const;
        DGNDBSYNC_EXPORT int64_t GetXPathInt64(BentleyApi::Utf8CP xpathExpression, int64_t defaultVal) const;
        DGNDBSYNC_EXPORT BentleyStatus EvaluateXPath(Utf8StringR value, Utf8CP xpathExpression) const;
        };  // Config

    struct Options : iModelBridge::Params
        {
    typedef bmap<DwgDbLineWeight,uint32_t>      T_DwgWeightMap;

    enum class CopyLayer
        {
        Never,
        IfDifferent,
        Always,
        UseConfig
        };

    private:
        StableIdPolicy      m_stableIdPolicy;
        BeFileName          m_rootDir;
        BeFileName          m_configFile;
        BeFileName          m_configFile2;
        BeFileName          m_changesFile;
        Utf8String          m_description;
        Utf8String          m_password;
        DateTime            m_time;
        DateTime            m_expirationDate;
        bool                m_changesFileNameFromGuid;
        bool                m_skipUnchangedFiles;
        CopyLayer           m_copyLayer;
        StandardUnit        m_unspecifiedBlockUnits;
        T_DwgWeightMap      m_lineweightMapping;
        bool                m_syncBlockChanges;
        bool                m_importRasters;
        bool                m_importPointClouds;
        uint16_t            m_pointCloudLevelOfDetails;
        bool                m_preferRenderableGeometry;
        Utf8String          m_namePrefix;

    public:
        Options ()
            {
            m_time = DateTime::GetCurrentTimeUtc();
            m_changesFileNameFromGuid = false;
            m_skipUnchangedFiles = true;
            m_copyLayer = CopyLayer::UseConfig;
            m_unspecifiedBlockUnits = StandardUnit::MetricMeters;
            m_lineweightMapping.clear ();
            m_syncBlockChanges = false;
            m_importRasters = false;
            m_importPointClouds = false;
            m_pointCloudLevelOfDetails = 1;
            m_preferRenderableGeometry = false;
            }

        void SetInputRootDir (BentleyApi::BeFileNameCR fileName) {m_rootDir = fileName;}
        void SetConfigFile (BentleyApi::BeFileNameCR fileName) {m_configFile = fileName;}
        void SetConfigFile2 (BentleyApi::BeFileNameCR fileName) {m_configFile2 = fileName;}
        void SetChangesFile (BentleyApi::BeFileNameCR fileName) {m_changesFile = fileName;}
        void SetDescription (BentleyApi::Utf8CP descr) {m_description=descr;}
        void SetTime (DateTime tm) {m_time=tm;}
        void SetPassword (BentleyApi::Utf8CP pw) {m_password=pw;}
        void SetStableIdPolicy (StableIdPolicy val) {m_stableIdPolicy=val;}
        void SetExpirationDate (DateTime const& d) {m_expirationDate=d;}
        void SetChangesFileNameFromGuid (bool val) {m_changesFileNameFromGuid = val;}
        void SetSkipUnchangedFiles (bool v) {m_skipUnchangedFiles = v;}
        void SetCopyLayer (CopyLayer v) {m_copyLayer = v;}
        void SetUnspecifiedBlockUnits (StandardUnit v) {m_unspecifiedBlockUnits = v;}
        void SetLineWeightMapping (T_DwgWeightMap const& map) { m_lineweightMapping = map; }
        void SetSyncBlockChanges (bool syncBlocks) { m_syncBlockChanges = syncBlocks; }
        void SetImportRasterAttachments (bool allow) { m_importRasters = allow; }
        void SetImportPointClouds (bool allow) { m_importPointClouds = allow; }
        void SetPointCloudLevelOfDetails (uint16_t lod) { if (lod <= 100) m_pointCloudLevelOfDetails = lod; }
        void SetPreferRenderableGeometry (bool forRendering) { m_preferRenderableGeometry = forRendering; }
        void SetNamePrefix (Utf8CP prefix) { m_namePrefix.assign(prefix); }

        BeFileNameCR GetInputRootDir() const {return m_rootDir;}
        BeFileNameCR GetConfigFile() const {return m_configFile;}
        BeFileNameCR GetConfigFile2() const {return m_configFile2;}
        BeFileNameCR GetChangesFile() const {return m_changesFile;}
        StableIdPolicy GetStableIdPolicy() const {return m_stableIdPolicy;}
        Utf8String GetDescription() const {return m_description;}
        Utf8String GetPassword () const {return m_password;}
        DateTime GetTime() const {return m_time;}
        DateTime GetExpirationDate() const {return m_expirationDate;}
        bool GetChangesFileNameFromGuid() const {return m_changesFileNameFromGuid;}
        bool GetSkipUnchangedFiles() const {return m_skipUnchangedFiles;}
        CopyLayer GetCopyLayer() const {return m_copyLayer;}
        StandardUnit GetUnspecifiedBlockUnits() const {return m_unspecifiedBlockUnits;}
        bool AlwaysCopyLayer() const {return m_copyLayer == CopyLayer::Always;}
        bool NeverCopyLayer() const {return m_copyLayer == CopyLayer::Never;}
        bool CopyLayerIfDifferent() const {return m_copyLayer == CopyLayer::IfDifferent;}
        uint32_t GetDgnLineWeight (DwgDbLineWeight dwgWeight) const;
        bool GetSyncBlockChanges () const { return m_syncBlockChanges; }
        bool GetImportRasterAttachments () const { return m_importRasters; }
        bool GetImportPointClouds () const { return m_importPointClouds; }
        uint16_t GetPointCloudLevelOfDetails () const { return m_pointCloudLevelOfDetails; }
        bool IsRenderableGeometryPrefered () const { return m_preferRenderableGeometry; }
        Utf8StringCR GetNamePrefix () const { return m_namePrefix; }
        };  // Options : iModelBridge::Params

    struct GeometryOptions : public IDwgDrawOptions
        {
        private:
            DwgGiRegenType      m_regenType;
            DwgDbDatabaseP      m_targetDatabase;
            DwgDbObjectId       m_viewportId;
            DVec3d              m_viewDirection;
            DVec3d              m_cameraUpDirection;
            DPoint3d            m_cameraLocation;
            size_t              m_numberIsolines;
            DRange2d            m_viewportRange;

        public:
            virtual DwgGiRegenType  _GetRegenType () override { return m_regenType; }
            virtual DwgDbDatabaseP  _GetDatabase () override { return m_targetDatabase; }
            virtual DwgDbObjectId   _GetViewportId () override { return m_viewportId; }
            virtual DVec3d          _GetViewDirection () override { return m_viewDirection; }
            virtual DVec3d          _GetCameraUpDirection () override { return m_cameraUpDirection; }
            virtual DPoint3d        _GetCameraLocation () override { return m_cameraLocation; }
            virtual size_t          _GetNumberOfIsolines () override { return m_numberIsolines; }
            virtual bool            _GetViewportRange (DRange2dR range) override { range = m_viewportRange; return true; }

        GeometryOptions ()
            {
            m_regenType = DwgGiRegenType::StandardDisplay;
            m_targetDatabase = nullptr;
            m_viewportId.SetNull ();
            m_viewDirection.Init (0.0, 0.0, 1.0);
            m_cameraUpDirection.Init (0.0, 1.0, 0.0);
            m_cameraLocation.Init (0.0, 0.0, 0.0);
            m_numberIsolines = 0;
            m_viewportRange.Init ();
            }

        DwgGiRegenType  GetRegenType () { return _GetRegenType(); }
        DwgDbDatabaseP  GetDatabase () { return _GetDatabase(); }
        DwgDbObjectId   GetViewportId () { return _GetViewportId(); }
        DVec3d          GetViewDirection () { return _GetViewDirection(); }
        DVec3d          GetCameraUpDirection () { return _GetCameraUpDirection(); }
        DPoint3d        GetCameraLocation () { return _GetCameraLocation(); }
        size_t          GetNumberOfIsolines () { return _GetNumberOfIsolines(); }
        bool            GetViewportRange (DRange2dR range) { return _GetViewportRange(range); }

        void            SetRegenType (DwgGiRegenType regen) { m_regenType = regen; }
        void            SetDatabase (DwgDbDatabaseP dwg) { m_targetDatabase = dwg; }
        void            SetViewportId (DwgDbObjectIdCR id) { m_viewportId = id; }
        void            SetViewDirection (DVec3dCR xDir) { m_viewDirection = xDir; }
        void            SetCameraUpDirection (DVec3dCR up) { m_cameraUpDirection = up; }
        void            SetCameraLocation (DPoint3dCR loc) { m_cameraLocation = loc; }
        void            SetNumberOfIsolines (size_t n) { m_numberIsolines = n; }
        void            SetViewportRange (DRange2dCR range) { m_viewportRange = range; }
        };  // GeometryOptions

    struct WorkingFonts
        {
    public:
        struct WorkingFont
            {
            BeFileName      m_path;
            DgnFontPtr      m_font;
            WorkingFont (BeFileNameCR path, DgnFontP font) : m_path(path), m_font(font) {}
            WorkingFont () {}
            };

    private:
        typedef bpair<Utf8String, WorkingFont>   T_FontEntry;
        typedef bmap<Utf8String, WorkingFont>    T_FontMap;

        bool                m_loaded;
        T_FontMap           m_truetypeFonts;
        T_FontMap           m_shxFonts;
        bset<Utf8String>    m_missingFonts;
        Utf8String          m_searchPaths;
        DwgImporter&        m_dwgImporter;

    public:
        WorkingFonts (DwgImporter& importer) : m_dwgImporter(importer), m_loaded(false) {}
        ~WorkingFonts ()
            {
            m_truetypeFonts.clear();
            m_shxFonts.clear();
            m_missingFonts.clear();
            }

        DgnFontCP       FindDgnFont (DgnFontType type, Utf8StringCR name, bool warning = true);
        bool            FindFontPath (BeFileNameR path, DgnFontType type, Utf8StringCR name) const;
        size_t          LoadFonts ();
        BentleyStatus   LoadTrueTypeFont (BeFileNameCR ttf);
        BentleyStatus   LoadShxFont (BeFileNameCR shx);
        BentleyStatus   LoadOSFonts ();
        }; // WorkingFonts

    struct FallbackFonts
        {
        BeFileName          m_shxForText;
        BeFileName          m_shxForShape;
        BeFileName          m_truetype;
        };

    struct ElementCreateParams
        {
        DgnModelR                       m_targetModel;
        DgnCategoryId                   m_categoryId;
        DgnSubCategoryId                m_subCategoryId;
        DgnCode                         m_elementCode;
        Transform                       m_transform;
        DPoint3d                        m_placementPoint;

        explicit ElementCreateParams (DgnModelR model) : m_targetModel(model) {}

        DgnModelCR          GetModel () { return m_targetModel; }
        DgnModelR           GetModelR () { return m_targetModel; }
        DgnCategoryId       GetCategoryId () { return m_categoryId; }
        DgnSubCategoryId    GetSubCategoryId () { return m_subCategoryId; }
        TransformR          GetTransformR () { return m_transform; }
        DPoint3dR           GetPlacementPointR () { return m_placementPoint; }
        DgnCode             GetElementCode () const { return  m_elementCode; }
        };

    //! An xRef holder per block, containing DgnModels imported from all xref instances.
    struct DwgXRefHolder
        {
    private:
        DwgDbDatabasePtr    m_xrefDatabase;
        BeFileName          m_path;
        WString             m_prefixInRootFile;
        DwgDbObjectId       m_blockIdInParentFile;
        DwgDbObjectId       m_spaceIdInRootFile;
        DgnModelIdSet       m_dgnModelIds;

    public:
        DwgXRefHolder () : m_xrefDatabase() { }
        explicit DwgXRefHolder (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer) { InitFrom(xrefBlock, importer); }

        bool            IsValid() const { return  m_xrefDatabase.IsValid(); }
        BentleyStatus   InitFrom (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer);
        DwgDbDatabaseCR GetDatabase() const { BeAssert(IsValid()); return *m_xrefDatabase.get(); }
        DwgDbDatabaseR  GetDatabaseR() { BeAssert(IsValid()); return *m_xrefDatabase.get(); }
        DwgDbDatabaseP  GetDatabaseP() { return m_xrefDatabase.get(); }
        DwgDbObjectId   GetModelspaceId () { BeAssert(IsValid()); return m_xrefDatabase->GetModelspaceId(); }
        DwgDbObjectIdCR GetBlockIdInParentFile () const { return  m_blockIdInParentFile; }
        DwgDbObjectIdCR GetLayoutspaceIdInRootFile () const { return  m_spaceIdInRootFile; }
        WStringCR       GetPrefixInRootFile () const { return m_prefixInRootFile; }
        BeFileNameCR    GetPath () const { return m_path; }
        DgnModelIdSet const&    GetDgnModelIds () const { return m_dgnModelIds; }
        DgnModelIdSet&  GetDgnModelIdsR () { return m_dgnModelIds; }
        void            AddDgnModelId (DgnModelId id) { m_dgnModelIds.insert(id); }
        };  // DwgXRefHolder
    typedef bvector<DwgXRefHolder>    T_LoadedXRefFiles;

    //! Aa xRef-DgnModel mapping per instance for paperspace
    struct DwgXRefInPaperspace
        {
        DwgDbObjectId       m_xrefInsertId;
        DwgDbObjectId       m_paperspaceId;
        DgnModelId          m_dgnModelId;
        explicit DwgXRefInPaperspace (DwgDbObjectId xrefId, DwgDbObjectId layoutId, DgnModelId modelId) : m_xrefInsertId(xrefId), m_paperspaceId(layoutId), m_dgnModelId(modelId) {}
        };
    typedef bvector<DwgXRefInPaperspace>    T_DwgXRefsInPaperspaces;
    typedef bpair<DgnViewId,DwgDbObjectId>  T_PaperspaceView;
    typedef bmap<DgnViewId,DwgDbObjectId>   T_PaperspaceViewMap;

    struct ElementImportInputs
        {
    public:
        DgnModelR               m_targetModel;
        DgnClassId              m_dgnClassId;
        Transform               m_transformToDgn;
        DwgDbObjectId           m_entityId;
        DwgDbEntityPtr          m_entity;
        DwgDbEntityCP           m_parentEntity;
        DwgDbSpatialFilterP     m_spatialFilter;
        ResolvedModelMapping    m_modelMapping;
        
    public:
        ElementImportInputs (DgnModelR model) : m_targetModel(model), m_spatialFilter(nullptr), m_parentEntity(nullptr) { m_transformToDgn.InitIdentity(); }
        DgnModelR               GetTargetModelR () { return m_targetModel; }
        void                    SetClassId (DgnClassId id) { m_dgnClassId = id; }
        DgnClassId              GetClassId () const { return m_dgnClassId; }
        void                    SetTransform (TransformCR t) { m_transformToDgn = t; }
        TransformCR             GetTransform () const { return m_transformToDgn; }
        void                    SetEntityId (DwgDbObjectIdCR eid) { m_entityId = eid; }
        DwgDbObjectIdCR         GetEntityId () const { return m_entityId; }
        DwgDbEntityPtr&         GetEntityPtrR () { return m_entity; }
        DwgDbEntityR            GetEntityR () { return *m_entity.get(); }
        DwgDbEntityP            GetEntityP () { return m_entity.get(); }
        DwgDbEntityCR           GetEntity () const { return *m_entity.get(); }
        DwgDbEntityCP           GetParentEntity () const { return m_parentEntity; }
        void                    SetParentEntity (DwgDbEntityCP parent) { m_parentEntity = parent; }
        void                    SetSpatialFilter (DwgDbSpatialFilterP filter) { m_spatialFilter = filter; }
        DwgDbSpatialFilterP     GetSpatialFilter () const { return m_spatialFilter; }
        void                    SetModelMapping (ResolvedModelMapping const& m) { m_modelMapping = m; }
        ResolvedModelMapping    GetModelMapping () const { return m_modelMapping; }
        DwgSyncInfo::DwgModelSyncInfoId GetModelSyncInfoId () const { return m_modelMapping.GetModelSyncInfoId(); }
        };  // ElementImportInputs

    struct ElementImportResults
        {
    public:
        // output elements from importing an entity
        DgnElementPtr                   m_importedElement;
        bvector<ElementImportResults>   m_childElements;
        // from DwgSyncInfo after importing
        DwgSyncInfo::DwgObjectMapping   m_existingElementMapping;
        bool                            m_wasDiscarded;
    public:
        ElementImportResults () : m_importedElement(nullptr), m_wasDiscarded(false) { m_childElements.clear(); }
        ElementImportResults (DgnElementP newElement) : m_importedElement(newElement), m_wasDiscarded(false) { m_childElements.clear(); }
        bool            WasDiscarded () { return  m_wasDiscarded; }
        void            SetWasDiscarded (bool discarded) { m_wasDiscarded = discarded; }
        DgnElementP     GetImportedElement () { return m_importedElement.IsValid() ? m_importedElement.get() : nullptr; }
        void            SetImportedElement (DgnElementP el) { m_importedElement = el; }
        void            AddChildResults (ElementImportResults& child) { m_childElements.push_back(child); }
        DwgSyncInfo::DwgObjectMapping const&  GetExistingElement () const { return m_existingElementMapping; }
        void SetExistingElement (DwgSyncInfo::DwgObjectMapping const& map) { if (map.IsValid()) m_existingElementMapping = map; }
        };  // ElementImportResults

    // a geometry cache used by both primitive geometry and shared parts
    struct GeometryBuilderInfo
        {
    public:
        // geometry builder containing imported geometry:
        GeometryBuilderPtr          m_geometryBuilder;
        // persistent imported display params:
        DgnCategoryId               m_categoryId;
        DgnSubCategoryId            m_subCategoryId;
        RenderMaterialId            m_materialId;
        uint32_t                    m_weight;
        ColorDef                    m_lineColor;
        ColorDef                    m_fillColor;
        Render::FillDisplay         m_fillDisplay;
        double                      m_transparency;
        double                      m_fillTransparency;
        DgnStyleId                  m_linestyleId;
        double                      m_linestyleScale;
        Render::GradientSymbCPtr    m_gradient;
        // part builder info
        DgnGeometryPartId           m_partId;
        uint64_t                    m_entityHandle;
        DVec3d                      m_scales;
        BentleyApi::MD5::HashVal    m_geometryHashVal;
        Utf8String                  m_partNamespace;
        Utf8String                  m_partCodeValue;
        Transform                   m_transform;
        
    public:
        GeometryBuilderInfo () : m_geometryBuilder(nullptr) { m_partId.Invalidate(); }
        GeometryBuilderInfo (Render::GeometryParamsCR geomParams, uint64_t handle = 0, GeometryBuilderP builder = nullptr);
        bool                IsSameDisplay (GeometryBuilderInfo const& other) const;
        Utf8StringCR        GetPartCodeValue (DwgDbEntityCP entity, size_t partIndex);
        static void         BuildPartCodeValue (Utf8StringR out, DwgDbEntityCP entity, size_t partIndex);
        };  // GeometryBuilderInfo

    typedef bvector<GeometryBuilderInfo>                T_GeometryBuilderList;
    typedef bvector<size_t>                             T_PartIndexList;
    typedef bpair<DwgDbObjectId, DgnElementId>          T_DwgDgnTextStyleId;
    typedef bmap<DwgDbObjectId, DgnElementId>           T_TextStyleIdMap;
    typedef bpair<DwgDbObjectId, DgnStyleId>            T_DwgDgnLineStyleId;
    typedef bmap<DwgDbObjectId, DgnStyleId>             T_LineStyleIdMap;
    typedef bpair<DwgDbObjectId, RenderMaterialId>      T_DwgRenderMaterialId;
    typedef bmap<DwgDbObjectId, RenderMaterialId>       T_MaterialIdMap;
    typedef bmap<Utf8String, DgnTextureId>              T_MaterialTextureIdMap;

    struct ECSqlCache : BeSQLite::Db::AppData
        {
    private:
        DwgImporter& m_importer;
        mutable bmap<ECN::ECClassCP, BeSQLite::EC::ECInstanceInserter*>  m_inserterCache;
        mutable bmap<ECN::ECClassCP, BeSQLite::EC::ECInstanceUpdater*>   m_updaterCache;

    public:
        explicit ECSqlCache (DwgImporter& in) : m_importer(in) {}
        ~ECSqlCache ();

        static ECSqlCache const&    GetCache (DwgImporter& in);
        BeSQLite::EC::ECInstanceInserter const&   GetInserter (ECN::ECClassCR) const;
        BeSQLite::EC::ECInstanceUpdater const&    GetUpdater (ECN::ECClassCR) const;
        };  // ECSqlCache

    struct ConstantBlockAttrdefs
        {
    private:
        DwgDbObjectId           m_blockId;
        DwgDbObjectIdArray      m_constantAttrdefIds;
    public:
        explicit ConstantBlockAttrdefs (DwgDbObjectIdCR id) : m_blockId(id) { m_constantAttrdefIds.clear(); }
        void                    Add (DwgDbObjectId attrdefId) { m_constantAttrdefIds.push_back(attrdefId); }
        DwgDbObjectIdArray&     GetAttrdefIdArrayR () { return m_constantAttrdefIds; }
        size_t                  GetCount () const { return m_constantAttrdefIds.size(); }
        DwgDbObjectIdCR         GetBlockId () const { return m_blockId; }
        };  // ConstantAttrdefs
    typedef bvector<ConstantBlockAttrdefs>              T_ConstantBlockAttrdefList;

    //! The status of attempting to create a new ImportJob
    enum class ImportJobCreateStatus
        {
        Success,                    //!< The ImportJob was created successfully
        FailedExistingRoot,         //!< The ImportJob could not be created, because an ImportJob already exists for the root file. Try update instead.
        FailedExistingNonRootModel, //!< The ImportJob could not be created, because the selected root model for the root file was already brought into the BIM as a reference attachment to some other ImportJob
        FailedInsertFailure,        //!< The ImportJob could not be created for unknown reasons
        };

    //! The status of attempting to load an existing ImportJob
    enum class ImportJobLoadStatus 
        {
        Success,                    //!< The ImportJob was loaded successfully
        FailedNotFound,             //!< The ImportJob could not be loaded, because no ImportJob exists for the root file. Try embed instead.
        };

    // Collect, format, shaffle, & display messages, etc
    struct MessageCenter
        {
        friend struct DwgImportHost;

    private:
        T_WStringVectorP    m_listMessageCollection;
        WString             m_displayMessage;

        void    ProcessInputMessage (WCharCP message, int numChars);
        void    ProcessInputMessage (WStringCR message);
        void    DisplayAndFlush ();

    public:
        MessageCenter () : m_listMessageCollection(nullptr) {}

        DGNDBSYNC_EXPORT void   StartListMessageCollection (T_WStringVectorP out);
        DGNDBSYNC_EXPORT void   StopListMessageCollection ();
        };  // MessageCenter

    //! The severity of an issue
    enum class IssueSeverity
        {
        Fatal       = 1,
        Error       = 2,
        Warning     = 3,
        Info        = 4,
        };

    //! Categories for issues
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(IssueCategory, dwg_issueCategory)
        L10N_STRING(Compatibility)      // =="Compatibility"==
        L10N_STRING(ConfigXml)          // =="Config"==
        L10N_STRING(CorruptData)        // =="Corrupt Data"==
        L10N_STRING(DigitalRights)      // =="Digital Rights"==
        L10N_STRING(DiskIO)             // =="Disk I/O"==
        L10N_STRING(Filtering)          // =="Filtering"==
        L10N_STRING(InconsistentData)   // =="Inconsistent Data"==
        L10N_STRING(MissingData)        // =="Missing Data"==
        L10N_STRING(UnexpectedData)     // =="Unexpected Data"==
        L10N_STRING(Sync)               // =="SyncInfo"==
        L10N_STRING(ToolkitError)       // =="Toolkit Error"==
        L10N_STRING(ToolkitMessage)     // =="From Toolkit"==
        L10N_STRING(Unknown)            // =="Unknown"==
        L10N_STRING(Unsupported)        // =="Unsupported"==
        L10N_STRING(VisualFidelity)     // =="Visual Fidelity"==
        L10N_STRING(Briefcase)          // =="Briefcase"==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! A problem in the conversion process
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(Issue, dwg_issue)
        L10N_STRING(CannotCreateChangesFile)     // =="Cannot create changes file"==
        L10N_STRING(CannotEmbedFont)             // =="Could not embed font type/name %i/'%s'; a different font will used for display."==
        L10N_STRING(CannotOpenModelspace)        // =="Cannot open modelspace for file [%s]"==
        L10N_STRING(CannotUseStableIds)          // =="Cannot use DgnElementIds for this kind of file"==
        L10N_STRING(CantCreateModel)             // =="Cannot create model [%s]"==
        L10N_STRING(CantCreateProject)           // =="Cannot create project file [%s]"==
        L10N_STRING(CantCreateSyncInfo)          // =="Cannot create sync info [%s]"==
        L10N_STRING(CantOpenSyncInfo)            // =="Cannot open sync info [%s]"==
        L10N_STRING(CantOpenObject)              // =="Cannot open DwgDb object [%s]"==
        L10N_STRING(ChangesFileInconsistent)     // =="The changes file [%s] is inconsistent with the syncinfo file"==
        L10N_STRING(ChangesFileInvalid)          // =="The changes file exists but cannot be opened. It may be invalid."==
        L10N_STRING(ConfigFileError)             // =="[%s] error at [%d,%d], %s"==
        L10N_STRING(ConfigUsingDefault)          // =="Using default configuration."==
        L10N_STRING(ImportFailure)               // =="Failed to import [%s]"==
        L10N_STRING(ElementFilteredOut)          // =="Element [%s] was not converted."==
        L10N_STRING(EmptyGeometry)               // ==" No geometry created for entity %s."== <<Leading space is necessary>>
        L10N_STRING(Error)                       // =="Error: %s"==
        L10N_STRING(Exception)                   // =="DWGToolKit Exception: %s"==
        L10N_STRING(FatalError)                  // =="A fatal error is stopping the conversion: %s"==
        L10N_STRING(ProgramExits)                // =="The importer must exit due to a fatal error!"==
        L10N_STRING(FileFilteredOut)             // =="File [%s] was not converted."==
        L10N_STRING(FileInUse)                   // =="File [%s] is in use"==
        L10N_STRING(FileNotFound)                // =="File [%s] was not found"==
        L10N_STRING(FileReadOnly)                // =="The file is read-only"==
        L10N_STRING(FontEmbedError)              // =="Could not embed %s font '%s'. Some elements may not display properly."==
        L10N_STRING(FontIllegalNumber)           // =="Illegal font number %u."==
        L10N_STRING(FontMissing)                 // =="Missing %s font '%s'. Some elements may not display properly."==
        L10N_STRING(FontNotEmbedded)             // =="Did not embed %s font '%s' due to importer configuration. Some elements may not display properly."==
        L10N_STRING(FontNumberError)             // =="Could not resolve font number %u."==
        L10N_STRING(InvalidLayer)                // =="Invalid Layer [%s] could not be converted"==
        L10N_STRING(InvalidRange)                // =="Invalid Range"==
        L10N_STRING(InvisibleElementFilteredOut) // =="Element [%s] is invisible. It was not converted."==
        L10N_STRING(LayerDefinitionChange)       // =="Layer [%s] has changed in [%s]: %s. Update is not possible. You must do a full conversion."==
        L10N_STRING(LayerDisplayInconsistent)    // =="Layer [%s] is turned on for some attachments but is turned off for [%s]"==
        L10N_STRING(LayerNotFoundInRoot)         // =="Layer [%s] found in tile file [%s] but not in root file [%s]. The root file must define all Layers."==
        L10N_STRING(LayerSymbologyInconsistent)  // =="Layer [%s] has a different definition in [%s] than in other files or attachments: %s"==
        L10N_STRING(LinetypeError)               // =="Linetype import error %s: "==
        L10N_STRING(Message)                     // =="%s"==
        L10N_STRING(MissingLayer)                // =="Missing Layer %d"==
        L10N_STRING(MissingLinetype)             // =="Could not find linetype [%s]. Some elements may not display properly."==
        L10N_STRING(MaterialError)               // =="Material import error [%s]"==
        L10N_STRING(ModelAlreadyImported)        // =="Model [%s] has already been converted."==
        L10N_STRING(ModelFilteredOut)            // =="Model [%s] was not imported."==
        L10N_STRING(NotADgnDb)                   // =="The file is not a DgnDb"==
        L10N_STRING(NotRecognizedFormat)         // =="File [%s] is not in a recognized format"==
        L10N_STRING(NewerDwgVersion)             // =="File [%s] is a newer version not currently supported"==
        L10N_STRING(CantCreateRaster)            // =="Cannot create raster attachment [%s]."==
        L10N_STRING(RootModelChanged)            // =="The original root model was deleted or has changed units."==
        L10N_STRING(RootModelMustBePhysical)     // =="Root model [%s] is not a physical model."==
        L10N_STRING(SaveError)                   // =="An error occurred when saving changes (%s)"==
        L10N_STRING(SeedFileMismatch)            // =="Seed file [%s] does not match target [%s]"==
        L10N_STRING(SyncInfoInconsistent)        // =="The syncInfo file [%s] is inconsistent with the project"==
        L10N_STRING(SyncInfoTooNew)              // =="Sync info was created by a later version"==
        L10N_STRING(ViewNoneFound)               // =="No view was found"==
        L10N_STRING(WrongBriefcaseManager)        // =="You must use the UpdaterBriefcaseManager when updating a briefcase with the converter"==
        L10N_STRING(UpdateDoesNotChangeClass)    // =="Update cannot change the class of an element. Element: %s. Proposed class: %s."==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! Progress messages for the conversion process
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(ProgressMessage, dwg_progress)
        L10N_STRING(STEP_INITIALIZING)                 // =="Initializing"==
        L10N_STRING(STEP_CLEANUP_EMPTY_TABLES)         // =="Cleaning up empty tables"==
        L10N_STRING(STEP_OPENINGFILE)                  // =="Opening File %ls [%s]"==
        L10N_STRING(STEP_COMPACTING)                   // =="Compacting File"==
        L10N_STRING(STEP_IMPORTING_ENTITIES)           // =="Importing Entities"==
        L10N_STRING(STEP_IMPORTING_VIEWS)              // =="Importing Views"==
        L10N_STRING(STEP_CREATE_IMODEL)                // =="Creating .imodel File"==
        L10N_STRING(STEP_CREATE_THUMBNAILS)            // =="Creating Thumbnails"==
        L10N_STRING(STEP_CREATING)                     // =="Creating DgnDb [%s]"==
        L10N_STRING(STEP_EMBED_FILES)                  // =="Embedding Files"==
        L10N_STRING(STEP_EMBED_FONTS)                  // =="Embedding Fonts"==
        L10N_STRING(STEP_CREATE_CLASS_VIEWS)           // =="Create ECClass Views"==
        L10N_STRING(STEP_IMPORTING_LAYERS)             // =="Importing Layers"==
        L10N_STRING(STEP_IMPORTING_TEXTSTYLES)         // =="Importing Text Styles"==
        L10N_STRING(STEP_IMPORTING_LINETYPES)          // =="Importing Line Types"==
        L10N_STRING(STEP_UPDATING)                     // =="Updating DgnDb"==
        L10N_STRING(STEP_IMPORTING_MATERIALS)          // =="Importing Materials"==
        L10N_STRING(STEP_IMPORTING_ATTRDEFSCHEMA)      // =="Importing Attribute Definition Schema [%d classes]"==
        L10N_STRING(TASK_LOADING_FONTS)                // =="Loading %s Fonts"==
        L10N_STRING(TASK_IMPORTING_MODEL)              // =="Model: %s"==
        L10N_STRING(TASK_IMPORTING_RASTERDATA)         // =="Importing raster data file: %s"==
        L10N_STRING(TASK_CREATING_THUMBNAIL)           // =="Creating thumbnail for: %s"==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

    //! Miscellaneous strings needed for DwgImporter
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START(DataStrings, dwg_dataStrings)
        L10N_STRING(AttrdefsSchemaDescription)         // =="Block attribute definitions created from DWG file %ls"==
        L10N_STRING(BlockAttrdefDescription)           // =="Attribute definitions created from block %ls"==
    IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END
    
    struct IssueReporter
        {
    public:
        struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
            {
            private:
                IssueReporter& m_issueReporter;

                virtual void _OnIssueReported(BentleyApi::Utf8CP message) const override;

            public:
                explicit ECDbIssueListener(IssueReporter& issueReporter) : BeSQLite::EC::ECDb::IIssueListener(), m_issueReporter(issueReporter) {}
                ~ECDbIssueListener() {}
            };

    private:
        bool                m_triedOpenReport;
        BeFileName          m_reportFileName;
        FILE*               m_reportFile;
        ECDbIssueListener   m_ecdbIssueListener;

        BentleyStatus OpenReportFile();

        static Utf8CP ToString(IssueSeverity);

    public:
        explicit IssueReporter(BeFileNameCR filename) : m_triedOpenReport(false), m_reportFileName(filename), m_reportFile(nullptr), m_ecdbIssueListener(*this) {}
        IssueReporter () : m_triedOpenReport(false), m_reportFileName(), m_reportFile(nullptr), m_ecdbIssueListener(*this) {}
        ~IssueReporter() {CloseReport();}

        void CloseReport();
        bool HasIssues() const { return m_triedOpenReport;}
        BeFileNameCR GetFileName() const {return  m_reportFileName;}
        ECDbIssueListener const& GetECDbIssueListener() const { return m_ecdbIssueListener; }
        void Report(IssueSeverity severity, IssueCategory::StringId category, Utf8CP details, Utf8CP context);

        static Utf8String FmtElement (DgnElementCR el);
        static Utf8String FmtModel (DwgDbBlockTableRecordCR block);
        static Utf8String FmtXReference (DwgDbBlockTableRecordCR block);
        static Utf8String FmtEntity (DwgDbEntityCR entity);
        static Utf8String FmtDouble (double value);
        static Utf8String FmtDoubles (double const* values, size_t count);
        static Utf8String FmtDPoint3d (DPoint3dCR point);
        static Utf8String FmtTransform (BentleyApi::TransformCR trans);
        };

protected:
    mutable DgnDbPtr            m_dgndb;
    mutable DwgDbDatabasePtr    m_dwgdb;
    ResolvedImportJob           m_importJob;
    SubjectCPtr                 m_spatialParentSubject;
    BeFileName                  m_rootFileName;
    Transform                   m_rootTransform;
    DwgDbObjectId               m_modelspaceId;
    DwgDbObjectId               m_currentspaceId;
    T_DwgModelMapping           m_dwgModelMap;
    ResolvedModelMapping        m_rootDwgModelMap;
    bool                        m_isProcessingDwgModelMap;
    StandardUnit                m_modelspaceUnits;
    bool                        m_wasAborted;
    Options                     m_options;
    GeometryOptions             m_currentGeometryOptions;
    DwgSyncInfo                 m_syncInfo;
    T_DwgChangeDetectorPtr      m_changeDetector;
    uint32_t                    m_fileCount;
    DgnCategoryId               m_uncategorizedCategoryId;
    CodeSpecId                  m_businessKeyCodeSpecId;
    DgnViewId                   m_defaultViewId;
    bvector<ImportRule>         m_modelImportRules;
    bool                        m_errorCount;
    IssueReporter               m_issueReporter;
    StableIdPolicy              m_currIdPolicy;
    Config                      m_config;
    AnnotationTextStyleId       m_defaultTextstyleId;
    DgnFontPtr                  m_defaultFont;
    WorkingFonts                m_loadedFonts;
    FallbackFonts               m_fallbackFonts;
    DwgXRefHolder               m_currentXref;
    T_LoadedXRefFiles           m_loadedXrefFiles;
    DgnModelIdSet               m_modelspaceXrefs;
    T_DwgXRefsInPaperspaces     m_paperspaceXrefs;
    T_PaperspaceViewMap         m_paperspaceViews;
    T_TextStyleIdMap            m_importedTextstyles;
    T_LineStyleIdMap            m_importedLinestyles;
    T_MaterialIdMap             m_importedMaterials;
    T_MaterialTextureIdMap      m_materialTextures;
    uint32_t                    m_entitiesImported;
    MessageCenter               m_messageCenter;
    ECN::ECSchemaCP             m_attributeDefinitionSchema;
    T_ConstantBlockAttrdefList  m_constantBlockAttrdefList;
    DgnModelId                  m_sheetListModelId;
    T_GeometryBuilderList       m_sharedGeometryPartList;

private:
    void                    InitUncategorizedCategory ();
    void                    InitBusinessKeyCodeSpec ();
    BentleyStatus           InitSheetListModel ();
    DgnElementId            CreateModelElement (DwgDbBlockTableRecordCR block, Utf8StringCR modelName, DgnClassId modelId);
    BentleyStatus           CreateElement (ElementImportResults& results, GeometryBuilderInfo& builderInfo, DgnElement::CreateParams& elemParams, DgnCodeCR parentCode, ElementHandlerP handler);
    void                    ScaleModelTransformBy (TransformR trans, DwgDbBlockTableRecordCR block);
    void                    CompoundModelTransformBy (TransformR trans, DwgDbBlockReferenceCR insert);
    UnitDefinition          GetModelUnitsFromBlock (double& unitScale, DwgDbBlockTableRecordCR block);
    BentleyStatus           SetModelProperties (DgnModelP model, DPoint2dCR snaps);
    BentleyStatus           SetModelPropertiesFromModelspaceViewport (DgnModelP model);
    BentleyStatus           SetModelPropertiesFromLayout (DgnModelP model, DwgDbObjectIdCR layoutId);
    Utf8String              RemapNameString (Utf8String filename, Utf8StringCR name, Utf8StringCR suffix);
    void                    OpenAndImportEntity (ElementImportInputs& inputs);
    Utf8String              ComputeModelName (Utf8StringR proposedName, BeFileNameCR baseFileName, BeFileNameCR refPath, Utf8CP inSuffix, DgnClassId modelType);
    bool                    AddToDwgModelMap (ResolvedModelMapping const&);
    ECN::ECObjectsStatus    AddAttrdefECClassFromBlock (ECN::ECSchemaPtr& schema, DwgDbBlockTableRecordCR block);
    void                    ImportAttributeDefinitionSchema (ECN::ECSchemaR attrdefSchema);
    void                    ImportDomainSchema (WCharCP fileName, DgnDomain& domain);
    void                    SaveViewDefinition (ViewControllerR viewController);
    void                    CheckSameRootModelAndUnits ();
    void                    ComputeDefaultImportJobName (Utf8StringCR rootModelName);
    Utf8String              GetImportJobNamePrefix () const { return ""; }

    static void             RegisterProtocalExtensions ();

protected:
    DGNDBSYNC_EXPORT virtual void       _BeginImport ();
    DGNDBSYNC_EXPORT virtual void       _FinishImport ();
    virtual void                        _OnFatalError() { m_wasAborted = true; }
    virtual GeometryOptions&            _GetCurrentGeometryOptions () { return m_currentGeometryOptions; }
    DGNDBSYNC_EXPORT virtual bool       _ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity = nullptr);
    BeFileNameCR                        GetRootDwgFileName () const { return m_rootFileName; }
    DGNDBSYNC_EXPORT  virtual void      _SetChangeDetector (bool updating);
    virtual IDwgChangeDetector&         _GetChangeDetector () { return *m_changeDetector; }
    DGNDBSYNC_EXPORT bool               ValidateDwgFile (BeFileNameCR dwgdxfName);

    //! @name The ImportJob
    //! @{
    //! The ImportJob's subject element. Put all documentlist models and PhsyicalPartitions under the job subject (not under the root subject).
    SubjectCR   GetJobSubject () const { return GetImportJob().GetSubject(); }
    void        ValidateJob ();
    SubjectCPtr GetSpatialParentSubject () const { return m_spatialParentSubject; }
    void        SetSpatialParentSubject (SubjectCR subject) { m_spatialParentSubject = &subject; }
    //! Find or create the standard set of job-specific partitions, documentlistmodels, and other definition elements that the converter uses to organize the converted information.
    //! Note that this cannot be done until the ImportJob has been created (when creating a new ImportJob) or read from syncinfo (when updating).
    void        GetOrCreateJobPartitions ();
    //! Get the one and only hierarchy subject for the job
    SubjectCPtr GetJobHierarchySubject ();
    ResolvedImportJob GetResolvedImportJob (DwgSyncInfo::ImportJob const& job);
    ResolvedImportJob FindSoleImportJobForFile (DwgDbDatabaseR dwg);
    // model subjects
    enum class ModelSubjectType {Hierarchy, References};
    SubjectCPtr GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR, ModelSubjectType);
    bool        IsUpdating () const { return GetOptions().IsUpdating(); }
    bool        IsCreatingNewDgnDb () { return GetOptions().IsCreatingNewDgnDb(); }

    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportSpaces ();
    DGNDBSYNC_EXPORT virtual BentleyStatus _DetectDeletedDocuments();

    //! @name  Creating DgnModels for DWG
    //! @{
    // Modelspace and xRef blocks as Physical Models, layout blocks as sheet models
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportDwgModels ();
    DGNDBSYNC_EXPORT virtual void       _SetModelUnits (GeometricModel::Formatter& displayInfo, DwgDbBlockTableRecordCR block);
    // get or create a new DgnModel from a model/paperspace or an xref (when xrefInsert!=nullptr & xrefDwg!=nullptr)
    ResolvedModelMapping                GetOrCreateModelFromBlock (DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert = nullptr, DwgDbDatabaseP xrefDwg = nullptr);
    ResolvedModelMapping                GetOrCreateRootModel ();
    ResolvedModelMapping                GetModelFromSyncInfo (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans);
    //! find DgnModel from DWG model cache
    ResolvedModelMapping                FindModel (DwgDbObjectIdCR dwgModelId, TransformCR trans, DwgSyncInfo::ModelSourceType source);
    Utf8String                          RemapModelName (Utf8StringCR name, BeFileNameCR, Utf8StringCR suffix);
    DGNDBSYNC_EXPORT virtual Utf8String _ComputeModelName (DwgDbBlockTableRecordCR block, Utf8CP suffix = nullptr);
    DGNDBSYNC_EXPORT virtual DgnClassId _GetModelType (DwgDbBlockTableRecordCR block);

    //! @name  Importing layers
    //! @{
    // The layer section contains all layers of a DWG file
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportLayerSection ();
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportLayer (DwgDbLayerTableRecordCR layer);
    DGNDBSYNC_EXPORT virtual BentleyStatus  _OnUpdateLayer (DgnCategoryId&, DwgDbLayerTableRecordCR);
    BentleyStatus                           GetLayerAppearance (DgnSubCategory::Appearance& appearance, DwgDbLayerTableRecordCR layer, DwgDbObjectIdCP viewportId = nullptr);

    //! @name  Importing viewport table
    //! @{
    // The viewport section contains all modelspace viewport table records, each of which may be attached with a view table record.
    // Each layout may have multiple viewports, the first of which is the paperspace viewport which should be used for the sheet model
    // imported from the layout block.
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportModelspaceViewports ();
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportModelspaceViewport (DwgDbViewportTableRecordCR vport);
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportPaperspaceViewport (DgnModelR model, TransformCR transform, DwgDbLayoutCR layout);
    DGNDBSYNC_EXPORT virtual void           _PostProcessViewports ();

    //! @name  Importing text style table
    //! @{
    // The text style section contains all text styles used in a DWG file
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportTextStyleSection ();
    DGNDBSYNC_EXPORT virtual AnnotationTextStyleCP  _ImportTextStyle (DwgDbTextStyleTableRecordCR dwgStyle);
    DGNDBSYNC_EXPORT virtual void                   _EmbedFonts ();

    //! @name  Importing line type table
    //! @{
    // The line type section contains all line types used in a DWG file
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportLineTypeSection ();
    DGNDBSYNC_EXPORT virtual LineStyleStatus        _ImportLineType (DwgDbLinetypeTableRecordPtr& ltype);
    DGNDBSYNC_EXPORT virtual BentleyStatus          _OnUpdateLineType (DgnStyleId&, DwgDbLinetypeTableRecordCR);

    //! @name  Importing materials
    //! @{
    // The materials dictionay contains all materials used in a DWG file
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportMaterialSection ();
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportMaterial (DwgDbMaterialPtr& material, Utf8StringCR paletteName, Utf8StringCR materialName);
    DGNDBSYNC_EXPORT virtual BentleyStatus          _OnUpdateMaterial (DwgSyncInfo::Material const& syncMaterial, DwgDbMaterialPtr& dwgMaterial);

    //! @name  Importing entities
    //! @{
    // DWG entity section is the ModelSpace block containing graphical entities
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportEntitySection ();
    //! Import a database-resident entity
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a block reference entity
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportXReference (DwgDbBlockReferenceCR xref, ElementImportInputs& inputs);
    //! Import a normal block reference entity
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs);
    //! this method is called to setup ElementCreatParams for each entity to be imported by default:
    DGNDBSYNC_EXPORT virtual BentleyStatus  _GetElementCreateParams (ElementCreateParams& params, TransformCR toDgn, DwgDbEntityCR entity, Utf8CP desiredCode = nullptr);
    //! Determine DgnClassId for an entity by its owner block
    DGNDBSYNC_EXPORT virtual DgnClassId     _GetElementType (DwgDbBlockTableRecordCR block);
    //! Determine graphical element label from an entity
    DGNDBSYNC_EXPORT virtual Utf8String     _GetElementLabel (DwgDbEntityCR entity);
    //! After geometry has been built from an entity, create or update a DgnElement from the builder:
    DGNDBSYNC_EXPORT virtual BentleyStatus  _CreateOrUpdateElement (ElementImportResults& results, DgnModelR targetModel, T_GeometryBuilderList const& builders, T_PartIndexList const& parts, ElementCreateParams const& params, DwgDbEntityCR entity, DgnClassId dgnClass);
    //! Should the entity be imported at all?
    DGNDBSYNC_EXPORT virtual bool           _FilterEntity (DwgDbEntityCR entity, DwgDbSpatialFilterP filter=nullptr);
    //! Should create a DgnElement if there is no geometry at all?
    DGNDBSYNC_EXPORT virtual bool           _SkipEmptyElement (DwgDbEntityCP entity);
    //! Insert imported DgnElement into DgnDb.  This method is called after _ImportEntity.
    DgnDbStatus     InsertResults (ElementImportResults& results);
    DgnDbStatus     UpdateResults (ElementImportResults& results, DgnElementId existingElement);
    //! Insert or update imported DgnElement and source DWG entity in DwgSynchInfo
    BentleyStatus   InsertOrUpdateResultsInSyncInfo (ElementImportResults& results, IDwgChangeDetector::DetectionResults const& updatePlan, DwgDbEntityCR entity, DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId);

    //! @name  Importing layouts
    //! @{
    // A DWG layout is made up by a Paperspace block containing graphical entities and a sheet/plot definition.
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportLayouts ();
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportLayout (ResolvedModelMapping& modelMap, DwgDbBlockTableRecordR block, DwgDbLayoutCR layout);

    //! @name Options and configs
    //! @{
    virtual void                        _OnConfigurationRead (BeXmlDomR configDom) {}
    void                                ParseConfigurationFile (T_Utf8StringVectorR userObjectEnablers);
    BentleyStatus                       SearchForMatchingRule (ImportRule& entryOut, Utf8StringCR modelName, BeFileNameCR baseFilename);
    virtual Utf8String                  _GetFontSearchPaths() const { return m_config.GetXPathString("/ConvertConfig/Fonts/@searchPaths", ""); }
    WorkingFonts const&                 GetLoadedFonts () const { return m_loadedFonts; }

    //! @name Error and Progress Reporting
    //! @{
    DGNDBSYNC_EXPORT virtual void       _ReportIssue (IssueSeverity, IssueCategory::StringId, Utf8CP message, Utf8CP context);
    void                                ReportDbFileStatus (BeSQLite::DbResult fileStatus, BeFileNameCR projectFileName);
    void                                AddTasks (int32_t n);
    DGNDBSYNC_EXPORT void               SetStepName (ProgressMessage::StringId, ...);
    DGNDBSYNC_EXPORT void               SetTaskName (ProgressMessage::StringId, ...);

    //! @name DWG-DgnDb sync info
    //! @{
    virtual DwgSyncInfo::DwgFileId      _AddFileInSyncInfo (DwgDbDatabaseR, StableIdPolicy);
    virtual StableIdPolicy              _GetDwgFileIdPolicy () const;
    DwgSyncInfo::DwgFileId              GetDwgFileId (DwgDbDatabaseR, bool setIfNotExist = true);

public:
    DGNDBSYNC_EXPORT DwgImporter (Options const& options);
    DGNDBSYNC_EXPORT ~DwgImporter ();

    DGNDBSYNC_EXPORT ImportJobCreateStatus InitializeJob (Utf8CP comment=nullptr, DwgSyncInfo::ImportJob::Type = DwgSyncInfo::ImportJob::Type::RootModels);
    DGNDBSYNC_EXPORT ImportJobLoadStatus FindJob ();
    DGNDBSYNC_EXPORT ResolvedImportJob const& GetImportJob () const { return m_importJob; }
    DGNDBSYNC_EXPORT BentleyStatus      AttachSyncInfo ();
    DwgSyncInfo&                        GetSyncInfo () { return m_syncInfo; }
    DGNDBSYNC_EXPORT bool               ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity = nullptr) { return _ArePointsValid(checkPoints, numPoints, entity); }
    DgnFontCP                           GetDgnFontFor (DwgFontInfoCR fontInfo);
    DgnFontCP                           GetDefaultFont () const { return m_defaultFont.get(); }
    AnnotationTextStyleId               GetDefaultTextStyleId () const { return m_defaultTextstyleId; }
    bool                                GetFallbackFontPathForShape (BeFileNameR filename) const;
    DGNDBSYNC_EXPORT void               SetFallbackFontPathForShape (BeFileNameCR filename);
    bool                                GetFallbackFontPathForText (BeFileNameR outName, DgnFontType type) const;
    DGNDBSYNC_EXPORT void               SetFallbackFontPathForText (BeFileNameCR inName, DgnFontType fontType);
    TransformCR                         GetRootTransform () const { return m_rootTransform; }
    DGNDBSYNC_EXPORT double             GetScaleToMeters () const;
    DwgDbObjectId                       GetCurrentViewportId () { return m_currentGeometryOptions.GetViewportId(); }
    DwgDbObjectIdCR                     GetCurrentSpaceId () const { return m_currentspaceId; }
    DwgDbObjectIdCR                     GetModelSpaceId () const { return m_modelspaceId; }
    StandardUnit                        GetModelSpaceUnits () const { return m_modelspaceUnits; }
    DgnStyleId                          GetDgnLineStyleFor (DwgDbObjectIdCR ltypeId);
    DgnElementId                        GetDgnTextStyleFor (DwgDbObjectIdCR tstyleId);
    RenderMaterialId                    GetDgnMaterialFor (DwgDbObjectIdCR materialId);
    DgnTextureId                        GetDgnMaterialTextureFor (Utf8StringCR fileName);
    T_MaterialIdMap&                    GetImportedDgnMaterials () { return m_importedMaterials; }
    void                                AddDgnMaterialTexture (Utf8StringCR fileName, DgnTextureId texture);
    ECSqlCache const&                   GetECSqlCache () const;
    ECN::ECSchemaCP                     GetAttributeDefinitionSchema () { return m_attributeDefinitionSchema; }
    bool                                GetConstantAttrdefIdsFor (DwgDbObjectIdArray& ids, DwgDbObjectIdCR blockId);
    DgnCategoryId                       FindCategoryFromSyncInfo (DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg = nullptr);
    DgnSubCategoryId                    FindSubCategoryFromSyncInfo (DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg = nullptr);
    DgnCategoryId                       GetOrAddDrawingCategory (DgnSubCategoryId& subCategory, DwgDbObjectIdCR layerId, DwgDbObjectIdCR viewportId, DgnModelCR model, DwgDbDatabaseP xrefDwg = nullptr);
    DgnSubCategoryId                    InsertAlternateSubCategory (DgnSubCategoryCPtr subcategory, DgnSubCategory::Appearance const& appearance, Utf8CP desiredName = nullptr);
    T_GeometryBuilderList&              GetSharedPartListR () { return m_sharedGeometryPartList; }
    T_GeometryBuilderList const&        GetSharedPartList () const { return m_sharedGeometryPartList; }

    //! Call this once before working with DwgImporter, after initializing DgnDb's DgnPlatformLib
    //! @param toolkitDir Installed RealDWG or OpenDWG folder; default to the same folder as the EXE.
    DGNDBSYNC_EXPORT static void        Initialize (BentleyApi::BeFileNameCP toolkitDir = nullptr);
    DGNDBSYNC_EXPORT static void        TerminateDwgHost ();
    DGNDBSYNC_EXPORT BentleyStatus      OpenDwgFile (BeFileNameCR dwgdxfName);
    DGNDBSYNC_EXPORT void               SetDgnDb (DgnDbR bim) const { m_dgndb = &bim; }
    DGNDBSYNC_EXPORT DgnDbR             GetDgnDb () const { return *m_dgndb; }
    DGNDBSYNC_EXPORT DwgDbDatabaseR     GetDwgDb () { return *m_dwgdb.get(); }
    DGNDBSYNC_EXPORT BentleyStatus      Process ();
    DGNDBSYNC_EXPORT void               Progress ();
    DGNDBSYNC_EXPORT DgnModelId         CreateModel (DwgDbBlockTableRecordCR block, Utf8CP modelName, DgnClassId classId);
    DGNDBSYNC_EXPORT void               ReportError (IssueCategory::StringId, Issue::StringId, Utf8CP details);
    void                                ReportError (IssueCategory::StringId category, Issue::StringId issue, WCharCP details) {ReportError(category,issue,Utf8String(details).c_str());}
    DGNDBSYNC_EXPORT void               ReportIssueV (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP context, ...);
    DGNDBSYNC_EXPORT void               ReportIssue (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details, Utf8CP context = nullptr);
    DGNDBSYNC_EXPORT void               ReportSyncInfoIssue (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details);
    DGNDBSYNC_EXPORT BentleyStatus      OnFatalError (IssueCategory::StringId cat=IssueCategory::Unknown(), Issue::StringId num=Issue::ProgramExits(), ...);
    DGNDBSYNC_EXPORT bool               WasAborted () const { return m_wasAborted; }
    DGNDBSYNC_EXPORT Utf8String         GetDescription () const { return m_options.GetDescription(); }
    DgnProgressMeterR                   GetProgressMeter() const;
    DGNDBSYNC_EXPORT MessageCenter&     GetMessageCenter () { return m_messageCenter; }
    DGNDBSYNC_EXPORT Options const&     GetOptions () const { return m_options; }
    DGNDBSYNC_EXPORT DgnCategoryId      GetUncategorizedCategory () const { return m_uncategorizedCategoryId; }
    DGNDBSYNC_EXPORT CodeSpecId         GetBusinessKeyCodeSpec () const { return m_businessKeyCodeSpecId; }
    StableIdPolicy                      GetCurrentIdPolicy () const { return m_currIdPolicy; }
    DwgXRefHolder&                      GetCurrentXRefHolder () { return m_currentXref; }
    DwgDbDatabaseP                      FindLoadedXRef (BeFileNameCR path);
    //! Import a database-resident entity
    DGNDBSYNC_EXPORT BentleyStatus      ImportEntity (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a none database-resident entity in a desired block (must be a valid block)
    DGNDBSYNC_EXPORT BentleyStatus      ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId, Utf8StringCR desiredCode);
    //! Create a new or update an existing element from an entity based on the sync info
    DGNDBSYNC_EXPORT BentleyStatus      ImportOrUpdateEntity (ElementImportInputs& inputs);
    DGNDBSYNC_EXPORT DgnCode            CreateCode (Utf8StringCR value) const;
    
    };  // DwgImporter

/*=================================================================================**//**
* A no-op detector for creating BIM from DWG
* @bsiclass                                                     Don.Fu          03/16
+===============+===============+===============+===============+===============+======*/
struct CreatorChangeDetector : IDwgChangeDetector
{
public:
    //! @{
    void    _Prepare (DwgImporter&) override {}
    void    _Cleanup (DwgImporter&) override {}
    bool    _ShouldSkipFile (DwgImporter&, DwgDbDatabaseCR) override { return false; }
    bool    _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const& m) override { return false; }
    void    _OnModelSeen (DwgImporter&, ResolvedModelMapping const& m) override {}
    void    _OnModelInserted (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP) override {}
    void    _OnElementSeen (DwgImporter&, DgnElementId) override {}
    void    _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) override {}
    void    _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) override {}
    void    _DetectDeletedElementsEnd (DwgImporter&) override {}
    void    _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) override {}
    void    _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) override {}
    void    _DetectDeletedModelsEnd (DwgImporter&) override {}
    void   _DeleteDeletedMaterials (DwgImporter&) override {}

    //! always fills in element provenence and returns true
    DGNDBSYNC_EXPORT bool   _IsElementChanged (DetectionResults&, DwgImporter&, DwgDbObjectCR, ResolvedModelMapping const&, T_DwgSyncInfoElementFilter*) override;
    CreatorChangeDetector () {}
};  // CreatorChangeDetector

/*=================================================================================**//**
* A change detector to help updating BIM previously imported from DWG
* @bsiclass                                                     Don.Fu          03/16
+===============+===============+===============+===============+===============+======*/
struct UpdaterChangeDetector : IDwgChangeDetector
{
private:
    DwgSyncInfo::ByDwgObjectIdIter*         m_byIdIter;
    DwgSyncInfo::ByHashIter*                m_byHashIter;
    DgnElementIdSet                         m_elementsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSkipped;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_newlyDiscoveredModels;
    uint32_t                                m_elementsDiscarded;

public:
    UpdaterChangeDetector () : m_byIdIter(nullptr), m_byHashIter(nullptr) {}
    DGNDBSYNC_EXPORT ~UpdaterChangeDetector ();
    DGNDBSYNC_EXPORT void _Prepare(DwgImporter&) override;
    DGNDBSYNC_EXPORT void _Cleanup(DwgImporter&) override;

    //! @name  Override tracking & detection methods
    //! @{
    DGNDBSYNC_EXPORT bool   _ShouldSkipFile (DwgImporter&, DwgDbDatabaseCR) override;
    DGNDBSYNC_EXPORT bool   _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const&) override;
    DGNDBSYNC_EXPORT void   _OnModelSeen (DwgImporter&, ResolvedModelMapping const&) override;
    DGNDBSYNC_EXPORT void   _OnModelInserted (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP xRef) override;
    DGNDBSYNC_EXPORT void   _OnElementSeen (DwgImporter&, DgnElementId) override;
    DGNDBSYNC_EXPORT bool   _IsElementChanged (DetectionResults&, DwgImporter&, DwgDbObjectCR, ResolvedModelMapping const&, T_DwgSyncInfoElementFilter* filter) override;
    //! @}

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit
    //! @{
    //! delete elements
    DGNDBSYNC_EXPORT void   _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) override;
    DGNDBSYNC_EXPORT void   _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) override;  //!< don't forget to call _DetectDeletedElementsEnd when done
    DGNDBSYNC_EXPORT void   _DetectDeletedElementsEnd (DwgImporter&) override { m_elementsSeen.clear(); }
    //! delete models
    DGNDBSYNC_EXPORT void   _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) override;
    DGNDBSYNC_EXPORT void   _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) override;    //!< don't forget to call _DetectDeletedModelsEnd when done
    DGNDBSYNC_EXPORT void   _DetectDeletedModelsEnd (DwgImporter&) override {m_dwgModelsSeen.clear();}
    //! delete tables
    DGNDBSYNC_EXPORT void   _DeleteDeletedMaterials (DwgImporter&);
    //! @}
};  // UpdaterChangeDetector

END_DGNDBSYNC_DWG_NAMESPACE
