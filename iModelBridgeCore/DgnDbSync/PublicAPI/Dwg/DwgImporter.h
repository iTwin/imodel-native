/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>

// Namespaces for DwgImporter
#define DWG_NAMESPACE_NAME Dwg
#define BEGIN_DWG_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace DWG_NAMESPACE_NAME {
#define END_DWG_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_DWG using namespace BENTLEY_NAMESPACE_NAME::DWG_NAMESPACE_NAME;
//__PUBLISH_SECTION_END__

#ifndef DWG_EXPORT
    #ifdef __DWGIMPORTER_BUILD__
    #define DWG_EXPORT  EXPORT_ATTRIBUTE
    #endif
#endif

//__PUBLISH_SECTION_START__
#ifndef DWG_EXPORT
    #define DWG_EXPORT  IMPORT_ATTRIBUTE
#endif


#include <Dwg/DwgDb/DwgDbDatabase.h>
#include <Dwg/DwgDb/DwgResBuf.h>
#include <Dwg/DwgDb/DwgDbObjects.h>
#include <Dwg/DwgDb/DwgDbEntities.h>
#include <Dwg/DwgDb/DwgDbSymbolTables.h>
#include <Dwg/DwgDb/DwgDrawables.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <BeXml/BeXml.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Dwg/DwgSyncInfo.h>
#include <Dwg/DwgL10N.h>

USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE


//=======================================================================================
//! Base class for options that control how to merge various named data structures that match specified properties
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
    //! @param[in] baseFileName The file that is being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, BeFileNameCR baseFileName) const;

    //! Compute new name
    //! @param[out] newName The new name, if successful
    //! @param[in] modelName The model that is being imported
    //! @param[in] baseFilename The base name of the parent file containing the model being imported
    //! @return non-zero error status if no new name could be computed. In case of error, \a newName is not modified.
    BentleyStatus ComputeNewName(Utf8StringR newName, Utf8StringCR modelName, BeFileNameCR baseFilename) const;
};  // ImportRule

//=======================================================================================
//! Mapping of a DWG model(modelspace, paperspace, xRef, raster, etc) to a DgnModel.
//=======================================================================================
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
    DwgSyncInfo::DwgModelMapping const& GetMapping () const { return m_mapping; }
    void            SetMapping (DwgSyncInfo::DwgModelMapping const& m) { m_mapping=m; }
    TransformCR     GetTransform () const { return m_mapping.GetTransform(); }
    void            SetTransform (TransformCR t) { return m_mapping.SetTransform(t); }
    DwgSyncInfo::DwgModelSyncInfoId GetModelSyncInfoId () const { return m_mapping.GetDwgModelSyncInfoId(); }
};  // ResolvedModelMapping
typedef bmultiset<ResolvedModelMapping>     T_DwgModelMapping;

//=======================================================================================
//! An import "job" definition, including its subject element.
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

//=======================================================================================
//! An interface for a change detector that detects changes in a DWG file
//=======================================================================================
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
    //! @note DwgImporter must not call this during the model-discovery step but only skip it during the element-conversion step.
    virtual bool _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const&,  DwgDbDatabaseCP xref = nullptr) = 0;

    //! Used to choose one of many existing entries in DwgSyncInfo
    typedef std::function<bool(DwgSyncInfo::ElementIterator::Entry const&, DwgImporter& converter)> T_DwgSyncInfoElementFilter;

    //! Called by a DwgImporter to detect if a DWG object is changed or new.
    //! @param[out] results The information about what the change detector has found
    //! @param[in] importer An instance of the DwgImporter
    //! @param[in] id       The object ID of the DWG entity to check
    //! @param[in] map      Mapping info for the DWG model that contains this DWG object
    //! @param[in] filter   Optional. Chooses among existing elements in DwgSyncInfo
    //! @return true if the element is new or has changed.
    virtual bool _IsElementChanged (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR id, ResolvedModelMapping const& map, T_DwgSyncInfoElementFilter* filter = nullptr) = 0;
    //! @}

    //! @name Recording DWG content seen (so that we can deduce deletes)
    //! @{
    //! Called whenever a DWG object is encountered, regardless of whether it is converted or not.
    virtual void _OnElementSeen (DwgImporter&, DgnElementId) = 0;

    void OnElementSeen (DwgImporter& importer, DgnElementP el) { if (el != nullptr) _OnElementSeen(importer, el->GetElementId()); }

    //! Called when a DWG model is discovered. This callback should be invoked during the model-discovery phase,
    //! before the elements in the specified model are converted.
    //! @param[in] importer An instance of the DwgImporter
    //! @param[in] map      Mapping info for the DWG model
    virtual void _OnModelSeen (DwgImporter& importer, ResolvedModelMapping const& map) = 0;
    //! Called when a DWG model is first mapped into the BIM.
    //! @param[in] importer An instance of the DwgImporter
    //! @param[in] map      The DWG model and the DgnModel to which it is mapped
    //! @param[in] xRef     The xref attachment if the DWG model is a root model, this will be nullptr. Otherwise, this would be the attachment that was used to reach the DWG model.
    virtual void _OnModelInserted (DwgImporter& importer, ResolvedModelMapping const& map, DwgDbDatabaseCP xRef) = 0;

    //! Called when a DWG modelspace viewport or a paperspace viewport is dicovered.
    virtual void  _OnViewSeen (DwgImporter&, DgnViewId) = 0;

    //! Called when a DWG dictionary group is discovered.
    virtual void  _OnGroupSeen (DwgImporter&, DgnElementId) = 0;

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit. Don't forget to call the ...End function when done.
    //! @{
    virtual void _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) = 0;  //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) = 0;          //!< don't forget to call _DetectDeletedElementsEnd when done
    virtual void _DetectDeletedElementsEnd (DwgImporter&) = 0;
    virtual void _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) = 0;      //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) = 0;             //!< don't forget to call _DetectDeletedModelsEnd when done
    virtual void _DetectDeletedModelsEnd (DwgImporter&) = 0;
    virtual void _DetectDeletedMaterials (DwgImporter&) = 0;
    virtual void _DetectDeletedViews (DwgImporter&) = 0;
    virtual void _DetectDeletedGroups (DwgImporter&) = 0;
    virtual void _DetectDetachedXrefs (DwgImporter&) = 0;
    //! @}
};  // IDwgChangeDetector
typedef std::unique_ptr <IDwgChangeDetector>    T_DwgChangeDetectorPtr;


//=======================================================================================
//! The main class that imports a root DWG file to a DgnDb project
//=======================================================================================
struct DwgImporter
    {
//__PUBLISH_SECTION_END__
    friend struct DwgBridge;
    friend struct DwgSyncInfo;
    friend struct DwgImportHost;
    friend struct ViewportFactory;
    friend struct AttributeFactory;
    friend struct MaterialFactory;
    friend struct LineStyleFactory;
    friend struct LayoutFactory;
    friend struct LayoutXrefFactory;
    friend struct GroupFactory;
    friend struct ElementFactory;
    friend class DwgProtocolExtension;
    friend class DwgRasterImageExt;
    friend class DwgPointCloudExExt;
    friend class DwgViewportExt;
    friend class DwgLightExt;
    friend class DwgBrepExt;
    friend class DwgBlockReferenceExt;

//__PUBLISH_SECTION_START__
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
        DWG_EXPORT BeXmlDom* GetDom() const;
        DWG_EXPORT bool OptionExists(BentleyApi::Utf8CP optionName) const;
        DWG_EXPORT Utf8String GetOptionValueString(BentleyApi::Utf8CP optionName, Utf8CP defaultVal) const;
        DWG_EXPORT bool GetOptionValueBool(BentleyApi::Utf8CP optionName, bool defaultVal) const;
        DWG_EXPORT double GetOptionValueDouble(BentleyApi::Utf8CP optionName, double defaultVal) const;
        DWG_EXPORT int64_t GetOptionValueInt64(BentleyApi::Utf8CP optionName, int64_t defaultVal) const;
        DWG_EXPORT Utf8String GetXPathString(BentleyApi::Utf8CP xpathExpression, Utf8CP defaultVal) const;
        DWG_EXPORT bool GetXPathBool(BentleyApi::Utf8CP xpathExpression, bool defaultVal) const;
        DWG_EXPORT double GetXPathDouble(BentleyApi::Utf8CP xpathExpression, double defaultVal) const;
        DWG_EXPORT int64_t GetXPathInt64(BentleyApi::Utf8CP xpathExpression, int64_t defaultVal) const;
        DWG_EXPORT BentleyStatus EvaluateXPath(Utf8StringR value, Utf8CP xpathExpression) const;
        };  // Config

    //! DwgImporter params extended from iModelBridge::Params
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
        BeFileName          m_changesFile;
        Utf8String          m_password;
        DateTime            m_time;
        DateTime            m_expirationDate;
        bool                m_changesFileNameFromGuid;
        bool                m_skipUnchangedFiles;
        CopyLayer           m_copyLayer;
        StandardUnit        m_unspecifiedBlockUnits;
        T_DwgWeightMap      m_lineweightMapping;
        bool                m_syncBlockChanges;
        bool                m_syncAsmBodyInFull;
        bool                m_syncDwgVersionGuid;
        bool                m_importRasters;
        bool                m_importPointClouds;
        uint16_t            m_pointCloudLevelOfDetails;
        bool                m_preferRenderableGeometry;
        bool                m_asmAsParasolid;
        Utf8String          m_namePrefix;
        bool                m_includeDwgPathInMaterialSearchPaths;
        bool                m_runAsStandaloneApp;
        bool                m_blockAsSharedParts;

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
            m_syncAsmBodyInFull = false;
            m_syncDwgVersionGuid = false;
            m_importRasters = false;
            m_importPointClouds = false;
            m_pointCloudLevelOfDetails = 1;
            m_preferRenderableGeometry = false;
            m_asmAsParasolid = false;
            m_includeDwgPathInMaterialSearchPaths = false;
            m_runAsStandaloneApp = false;
            m_blockAsSharedParts = true;
            }

        void SetInputRootDir (BentleyApi::BeFileNameCR fileName) {m_rootDir = fileName;}
        void SetConfigFile (BentleyApi::BeFileNameCR fileName) {m_configFile = fileName;}
        void SetChangesFile (BentleyApi::BeFileNameCR fileName) {m_changesFile = fileName;}
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
        void SetSyncAsmBodyInFull (bool fullSync) { m_syncAsmBodyInFull = fullSync; }
        void SetSyncDwgVersionGuid (bool checkGuid) { m_syncDwgVersionGuid = checkGuid; }
        void SetImportRasterAttachments (bool allow) { m_importRasters = allow; }
        void SetImportPointClouds (bool allow) { m_importPointClouds = allow; }
        void SetPointCloudLevelOfDetails (uint16_t lod) { if (lod <= 100) m_pointCloudLevelOfDetails = lod; }
        void SetPreferRenderableGeometry (bool forRendering) { m_preferRenderableGeometry = forRendering; }
        void SetAsmAsParasolid (bool toBrep) { m_asmAsParasolid = toBrep; }
        void SetBlockAsSharedParts (bool v) { m_blockAsSharedParts = v; }
        void SetNamePrefix (Utf8CP prefix) { m_namePrefix.assign(prefix); }
        void SetDwgPathInMaterialSearch (bool v) { m_includeDwgPathInMaterialSearchPaths = v; }
        void SetRunAsStandaloneApp (bool v) { m_runAsStandaloneApp = v; }

        BeFileNameCR GetInputRootDir() const {return m_rootDir;}
        BeFileNameCR GetConfigFile() const {return m_configFile;}
        BeFileNameCR GetChangesFile() const {return m_changesFile;}
        StableIdPolicy GetStableIdPolicy() const {return m_stableIdPolicy;}
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
        //! Fully sync ASM Brep data? Recommended for detail editing such as imprinting or edge coloring etc.
        bool GetSyncAsmBodyInFull () const { return m_syncAsmBodyInFull; }
        //! Can DWG VersionGuid be used for sync?  Recommended for DWG files changed only by AutoCAD based products.
        bool GetSyncDwgVersionGuid () const { return m_syncDwgVersionGuid; }
        bool GetImportRasterAttachments () const { return m_importRasters; }
        bool GetImportPointClouds () const { return m_importPointClouds; }
        uint16_t GetPointCloudLevelOfDetails () const { return m_pointCloudLevelOfDetails; }
        bool IsRenderableGeometryPrefered () const { return m_preferRenderableGeometry; }
        bool IsAsmAsParasolid () const { return m_asmAsParasolid; }
        bool IsBlockAsSharedParts () const { return m_blockAsSharedParts; }
        Utf8StringCR GetNamePrefix () const { return m_namePrefix; }
        bool IsDwgPathInMaterialSearch () const { return m_includeDwgPathInMaterialSearchPaths; }
        bool IsRunAsStandaloneApp () const { return m_runAsStandaloneApp; }
        };  // Options : iModelBridge::Params

    //! Options to control object enablers to draw entities that results in desired geometry
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

    //! A text font cache for the importer
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

    //! Default fonts used for texts which have missing fonts
    struct FallbackFonts
        {
        BeFileName          m_shxForText;
        BeFileName          m_shxForShape;
        BeFileName          m_truetype;
        };

    //! Parameters used to create a element in a geometric model
    struct ElementCreateParams
        {
        DgnModelR                       m_targetModel;
        DefinitionModelPtr              m_geometryPartsModel;
        DgnCategoryId                   m_categoryId;
        DgnSubCategoryId                m_subCategoryId;
        DgnCode                         m_elementCode;
        Transform                       m_transform;
        DPoint3d                        m_placementPoint;

        explicit ElementCreateParams (DgnModelR model) : m_targetModel(model) {}

        DgnModelCR          GetModel () { return m_targetModel; }
        DgnModelR           GetModelR () { return m_targetModel; }
        DefinitionModelPtr  GetGeometryPartsModel () { return m_geometryPartsModel; }
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
        BeFileName          m_resolvedPath;
        BeFileName          m_savedPath;
        WString             m_prefixInRootFile;
        DwgDbObjectId       m_blockIdInRootFile;
        DgnModelIdSet       m_dgnModels;
        DRange3d            m_computedRange;

    public:
        DwgXRefHolder () : m_xrefDatabase() { }
        explicit DwgXRefHolder (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer) { InitFrom(xrefBlock, importer); }

        bool            IsValid() const { return  m_xrefDatabase.IsValid(); }
        BentleyStatus   InitFrom (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer);
        DwgDbDatabaseCR GetDatabase() const { BeAssert(IsValid()); return *m_xrefDatabase.get(); }
        DwgDbDatabaseR  GetDatabaseR() { BeAssert(IsValid()); return *m_xrefDatabase.get(); }
        DwgDbDatabaseP  GetDatabaseP() { return m_xrefDatabase.get(); }
        DwgDbObjectId   GetModelspaceId () { BeAssert(IsValid()); return m_xrefDatabase->GetModelspaceId(); }
        DwgDbObjectIdCR GetBlockIdInRootFile () const { return  m_blockIdInRootFile; }
        WStringCR       GetPrefixInRootFile () const { return m_prefixInRootFile; }
        BeFileNameCR    GetResolvedPath () const { return m_resolvedPath; }
        BeFileNameCR    GetSavedPath () const { return m_savedPath; }
        bool            HasDgnModel (DgnModelId id) const { return m_dgnModels.Contains(id); }
        void            AddDgnModel (DgnModelId id) { m_dgnModels.insert(id); }
        DgnModelIdSet&  GetDgnModelsR () { return m_dgnModels; }
        //! Set a computed range for xRef's modelspace
        void            SetComputedRange (DRange3dCR range) { m_computedRange = range; }
        //! Get the computed range for xRef's modelspace
        DRange3dCR      GetComputedRange () const { return m_computedRange; }
        };  // DwgXRefHolder
    typedef bvector<DwgXRefHolder>    T_LoadedXRefFiles;

    //! An xRef-DgnModel mapping per instance for paperspace
    struct DwgXRefInPaperspace
        {
        DwgDbObjectId       m_xrefInsertId;
        DwgDbObjectId       m_paperspaceId;
        DgnModelId          m_dgnModelId;
        explicit DwgXRefInPaperspace (DwgDbObjectId xrefId, DwgDbObjectId layoutId, DgnModelId modelId) : m_xrefInsertId(xrefId), m_paperspaceId(layoutId), m_dgnModelId(modelId) {}
        DwgDbObjectIdCR GetXrefInsertId () const { return m_xrefInsertId; }
        DwgDbObjectIdCR GetPaperSpaceId () const { return m_paperspaceId; }
        DgnModelId GetPaperSpaceModelId () const { return m_dgnModelId; }
        };
    typedef bvector<DwgXRefInPaperspace>    T_DwgXRefsInPaperspaces;
    typedef bpair<DgnViewId,DwgDbObjectId>  T_PaperspaceView;
    typedef bmap<DgnViewId,DwgDbObjectId>   T_PaperspaceViewMap;

    //! A data context for a modelspace or paperspace entity ready to be imported into a target DgnDb model.
    struct ElementImportInputs
        {
    public:
        DgnModelR               m_targetModel;
        DgnClassId              m_dgnClassId;
        Transform               m_transformToDgn;
        DwgDbObjectId           m_entityId;
        DwgDbEntityPtr          m_entity;
        DwgDbEntityCP           m_parentEntity;
        DwgDbEntityCP           m_templateEntity;
        DwgDbSpatialFilterP     m_spatialFilter;
        ResolvedModelMapping    m_modelMapping;
        
    public:
        //! Constructor to begin building the input context for a valid target model
        DWG_EXPORT ElementImportInputs (DgnModelR model);
        //! Constructor to copy from a valid input context to a different valid target model
        //! @param[in] model The target DgnModel in which elements created from the entity will be added
        //! @param[in] entity The input source DWG entity to be acquired by m_entity
        //! @param[in] other The other input context to be copied
        DWG_EXPORT ElementImportInputs (DgnModelR model, DwgDbEntityP entity, ElementImportInputs const& other);
        DgnModelCR              GetTargetModel () const { return m_targetModel; }
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
        //! Get the template entity for basic entity properties such as layer, linetype, etc.  Used when the source entity is not a database resident.
        DwgDbEntityCP           GetTemplateEntity () const { return m_templateEntity; }
        //! Set a template entity for basic entity properties when they are not available from the source entity.
        //! @param[in] dbent A DWG database entity as a template for layer, linetype, etc.  Must be a database resident entity, or nullptr.
        void                    SetTemplateEntity (DwgDbEntityCP dbent) { if(dbent==nullptr || dbent->GetObjectId().IsValid()) m_templateEntity = dbent; else BeAssert(false && "Input entity not a db resident!"); }
        void                    SetSpatialFilter (DwgDbSpatialFilterP filter) { m_spatialFilter = filter; }
        DwgDbSpatialFilterP     GetSpatialFilter () const { return m_spatialFilter; }
        void                    SetModelMapping (ResolvedModelMapping const& m) { m_modelMapping = m; }
        ResolvedModelMapping    GetModelMapping () const { return m_modelMapping; }
        DwgSyncInfo::DwgModelSyncInfoId GetModelSyncInfoId () const { return m_modelMapping.GetModelSyncInfoId(); }
        };  // ElementImportInputs

    //! A data context for output DgnElement's imported from a modelspace or paperspace entity.
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

    //! Cache entry for geometry collection
    struct GeometryEntry
        {
    private:
        GeometricPrimitivePtr       m_geometry;
        Render::GeometryParams      m_display;
        Transform                   m_transform;
        Utf8String                  m_blockName;
        DwgDbObjectId               m_blockId;
        DwgSyncInfo::DwgFileId      m_fileId;
    public:
        bool    IsValid () const { return m_geometry.IsValid(); }
        GeometricPrimitiveCR GetGeometry () const { BeAssert(IsValid()); return *m_geometry; }
        GeometricPrimitiveP GetGeometryP () { BeAssert(IsValid()); return m_geometry.get(); }
        void    SetGeometry (GeometricPrimitiveP geom) { m_geometry = geom; }
        Render::GeometryParamsCR GetGeometryParams () const { return m_display; }
        Render::GeometryParamsR GetGeometryParamsR () { return m_display; }
        void    SetGeometryParams (Render::GeometryParamsCR params) { m_display = params; }
        TransformCR GetTransform () const { return m_transform; }
        void    SetTransform (TransformCR trans) { m_transform = trans; }
        Utf8StringCR    GetBlockName () const { return m_blockName; }
        void    SetBlockName (Utf8StringCR name) { m_blockName = name; }
        DwgDbObjectIdCR GetBlockId () const { return m_blockId; }
        void    SetBlockId (DwgDbObjectIdCR id) { m_blockId = id; }
        DwgSyncInfo::DwgFileId  GetDwgFileId () const { return m_fileId; }
        void    SetDwgFileId (DwgSyncInfo::DwgFileId  id) { m_fileId = id; }
        };  // GeometryEntry
    typedef bvector<GeometryEntry>                      T_GeometryList;
    typedef bpair<DwgDbObjectId, T_GeometryList>        T_BlockGeometryEntry;
    typedef bmap<DwgDbObjectId, T_GeometryList>         T_BlockGeometryMap;

    //! Cache entry for a shared part collection
    struct SharedPartEntry
        {
    private:
        DgnGeometryPartId           m_partId;
        Render::GeometryParams      m_display;
        DRange3d                    m_partRange;
        double                      m_partScale;
        Transform                   m_geometryToPart;
    public:
        SharedPartEntry () : m_partScale(0.0) { m_partId.Invalidate(); m_partRange.Init(); m_geometryToPart.InitIdentity(); }
        bool        IsValid () const { return m_partId.IsValid(); }
        DgnGeometryPartId GetPartId () const { return m_partId; }
        void        SetPartId (DgnGeometryPartId id) { m_partId = id; }
        Render::GeometryParamsCR    GetGeometryParams () const { return m_display; }
        void        SetGeometryParams (Render::GeometryParamsCR params) { m_display = params; }
        DRange3dCR  GetPartRange () const { return m_partRange; }
        void        SetPartRange (DRange3dCR range) { m_partRange = range; }
        TransformCR GetTransform () const { return m_geometryToPart; }
        void        SetTransform (TransformCR trans) { m_geometryToPart = trans; }
        double      GetPartScale () const { return m_partScale; }
        void        SetPartScale (double scale) { m_partScale = scale; }
        };  // SharedPartEntry
    typedef bvector<SharedPartEntry>                    T_SharedPartList;

    //! Cache key for a shared part collection
    struct SharedPartKey
        {
    private:
        MD5::HashVal                m_keyValue;
    public:
        //! A unique key value consists of blockId, layerId, ByBlock symbology and scale
        DWG_EXPORT SharedPartKey (DwgDbBlockReferenceCR insert, double scale);
        DWG_EXPORT SharedPartKey ();

        MD5::HashVal const& GetKeyValue () const { return m_keyValue; }
        DWG_EXPORT bool IsValid () const;
        //! The left-hand operand of the key
        DWG_EXPORT bool operator < (SharedPartKey const& rho) const;
        };  // SharedPartKey
    typedef bpair<SharedPartKey, T_SharedPartList>      T_BlockPartsEntry;
    typedef bmap<SharedPartKey, T_SharedPartList>       T_BlockPartsMap;

    typedef bpair<DwgDbObjectId, DgnElementId>          T_DwgDgnTextStyleId;
    typedef bmap<DwgDbObjectId, DgnElementId>           T_TextStyleIdMap;
    typedef bpair<DwgDbObjectId, DgnStyleId>            T_DwgDgnLineStyleId;
    typedef bmap<DwgDbObjectId, DgnStyleId>             T_LineStyleIdMap;
    typedef bpair<DwgDbObjectId, RenderMaterialId>      T_DwgRenderMaterialId;
    typedef bmap<DwgDbObjectId, RenderMaterialId>       T_MaterialIdMap;
    typedef bmap<Utf8String, DgnTextureId>              T_MaterialTextureIdMap;

    //! Category cache for fast layer-category retrieving
    struct CategoryEntry
        {
    private:
        DgnCategoryId       m_categoryId;
        DgnSubCategoryId    m_subcategoryId;
    public:
        CategoryEntry (DgnCategoryId cat, DgnSubCategoryId sub) : m_categoryId(cat), m_subcategoryId(sub) {}
        CategoryEntry () {}
        DgnCategoryId       GetCategoryId () const { return m_categoryId; }
        DgnSubCategoryId    GetSubCategoryId () const { return m_subcategoryId; }
        };  // CategoryEntry
    typedef bpair<DwgDbObjectId, CategoryEntry>         T_DwgDgnLayer;
    typedef bmap<DwgDbObjectId, CategoryEntry>          T_DwgDgnLayerMap;

    //! ElementAspect's host element cache for PresentationRules
    struct PresentationRuleContent
        {
    private:
        Utf8String  m_attrdefClassName;
        Utf8String  m_hostElementClass;
        Utf8String  m_hostElementSchema;
    public:
        explicit PresentationRuleContent (Utf8StringCR a, Utf8StringCR c, Utf8StringCR s) : m_attrdefClassName(a), m_hostElementClass(c), m_hostElementSchema(s) {}
        Utf8StringCR    GetAttrdefClass () const { return m_attrdefClassName; }
        Utf8StringCR    GetHostElementClass () const { return m_hostElementClass; }
        Utf8StringCR    GetHostElementSchema () const { return m_hostElementSchema; }
        bool operator==(PresentationRuleContent const& c) const
            {
            return m_attrdefClassName.Equals(c.GetAttrdefClass()) && m_hostElementClass.Equals(c.GetHostElementClass()) && m_hostElementSchema.Equals(c.GetHostElementSchema());
            }
        };  // PresentationRuleContent
    typedef bvector<PresentationRuleContent>    T_PresentationRuleContents;

    //! An ID collection of constant block attribute definitions
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

    //! Information about the root model transformation.
    struct RootTransformInfo
        {
    private:
        Transform   m_rootTransform;        // the effective root model transform
        Transform   m_jobTransform;         // the spatial model transform initiated from iModelBridge
        Transform   m_changeTransform;      // the delta transform from old to new root model
        bool        m_changed;              // tells us if the root transform has been changed from last import
    public:
        RootTransformInfo () : m_changed(false) { m_rootTransform.InitIdentity(); m_changeTransform.InitIdentity(); m_jobTransform.InitIdentity(); }
        //! @return True the root transform has been changed from last import when updating; false otherwise;
        bool    HasChanged() const { return m_changed; }
        void    SetHasChanged(bool changed) { m_changed = changed; }
        //! @return The current root model transform: compounded with the spatial transform from iModelBridge for modelspace, and inverted for paperspaces.
        TransformCR GetRootTransform () const { return m_rootTransform; }
        TransformR  GetRootTransformR () { return m_rootTransform; }
        void    SetRootTransform (TransformCR t) { m_rootTransform = t; }
        //! @return The delta transform that changes the root transform from old to new - valid only if HasChanged returns true.
        TransformCR GetChangeTransformFromOldToNew () const { return m_changeTransform; }
        Transform   GetChangeTransformFromNewToOld () const { return m_changeTransform.ValidatedInverse().Value(); }
        //! @param t The delta transform that changes the root transform from old to new.
        void    SetChangeTransformFromOldToNew (TransformCR t) { m_changeTransform = t; }
        //! @return The spatial model transform initiated for current import job from iModelBridge.
        TransformCR GetJobTransform () const { return m_jobTransform; }
        void    SetJobTransform (TransformCR t) { m_jobTransform = t; }
        };  // RootTransformInfo

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
        bset<WString>       m_displayedMessageCollection;
        WString             m_displayMessage;

        void    ProcessInputMessage (WCharCP message, int numChars);
        void    ProcessInputMessage (WStringCR message);
        void    DisplayAndFlush ();

    public:
        MessageCenter () : m_listMessageCollection(nullptr) {}

        DWG_EXPORT void   StartListMessageCollection (T_WStringVectorP out);
        DWG_EXPORT void   StopListMessageCollection ();
        };  // MessageCenter

    //! The severity of an issue
    enum class IssueSeverity
        {
        Fatal       = 1,
        Error       = 2,
        Warning     = 3,
        Info        = 4,
        };

    //! Report and/or log issues seen during a conversion
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
            };  // ECDbIssueListener

    private:
        bool                m_triedOpenReport;
        BeFileName          m_reportFileName;
        FILE*               m_reportFile;
        ECDbIssueListener   m_ecdbIssueListener;

        BentleyStatus OpenReportFile();

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
        static Utf8CP ToString (IssueSeverity);
        };  // IssueReporter

protected:
    mutable DgnDbPtr            m_dgndb;
    mutable DwgDbDatabasePtr    m_dwgdb;
    ResolvedImportJob           m_importJob;
    SubjectCPtr                 m_spatialParentSubject;
    BeFileName                  m_rootFileName;
    RootTransformInfo           m_rootTransformInfo;
    DwgDbObjectId               m_modelspaceId;
    DwgDbObjectId               m_currentspaceId;
    T_DwgModelMapping           m_dwgModelMap;
    ResolvedModelMapping        m_rootDwgModelMap;
    bool                        m_hasBegunProcessing;
    bool                        m_isProcessingDwgModelMap;
    StandardUnit                m_modelspaceUnits;
    bool                        m_wasAborted;
    Options&                    m_options;
    GeometryOptions             m_currentGeometryOptions;
    DwgSyncInfo                 m_syncInfo;
    T_DwgChangeDetectorPtr      m_changeDetector;
    uint32_t                    m_fileCount;
    DgnCategoryId               m_uncategorizedCategoryId;
    CodeSpecId                  m_businessKeyCodeSpecId;
    DwgDbObjectId               m_activeViewportId;
    DgnViewId                   m_defaultViewId;
    bvector<ImportRule>         m_modelImportRules;
    size_t                      m_errorCount;
    IssueReporter               m_issueReporter;
    bset<Utf8String>            m_reportedIssues;
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
    DwgDbObjectIdArray          m_paperspaceBlockIds;
    T_TextStyleIdMap            m_importedTextstyles;
    T_LineStyleIdMap            m_importedLinestyles;
    T_MaterialIdMap             m_importedMaterials;
    T_MaterialTextureIdMap      m_materialTextures;
    bvector<BeFileName>         m_materialSearchPaths;
    T_DwgDgnLayerMap            m_layersInSync;
    uint32_t                    m_entitiesImported;
    uint32_t                    m_layersImported;
    MessageCenter               m_messageCenter;
    ECN::ECSchemaCP             m_attributeDefinitionSchema;
    T_ConstantBlockAttrdefList  m_constantBlockAttrdefList;
    DgnModelId                  m_sheetListModelId;
    DgnModelId                  m_drawingListModelId;
    DgnModelId                  m_groupModelId;
    DefinitionModelPtr          m_geometryPartsModel;
    DefinitionModelPtr          m_jobDefinitionModel;
    T_BlockPartsMap             m_blockPartsMap;
    T_PresentationRuleContents  m_presentationRuleContents;

//__PUBLISH_SECTION_END__
private:
    void                    InitUncategorizedCategory ();
    void                    InitBusinessKeyCodeSpec ();
    BentleyStatus           InitSheetListModel ();
    BentleyStatus           InitDrawingListModel ();
    BentleyStatus           InitGroupModel ();
    DgnElementId            CreateModelElement (DwgDbBlockTableRecordCR block, Utf8StringCR modelName, DgnClassId modelId);
    void                    ScaleModelTransformBy (TransformR trans, DwgDbBlockTableRecordCR block);
    void                    AlignSheetToPaperOrigin (TransformR trans, DwgDbObjectIdCR layoutId);
    void                    CompoundModelTransformBy (TransformR trans, DwgDbBlockReferenceCR insert);
    UnitDefinition          GetModelUnitsFromBlock (double& unitScale, DwgDbBlockTableRecordCR block);
    BentleyStatus           SetModelProperties (DgnModelP model, DPoint2dCR snaps);
    BentleyStatus           SetModelPropertiesFromModelspaceViewport (DgnModelP model);
    BentleyStatus           SetModelPropertiesFromLayout (DgnModelP model, DwgDbObjectIdCR layoutId);
    Utf8String              RemapNameString (Utf8String filename, Utf8StringCR name, Utf8StringCR suffix);
    void                    OpenAndImportEntity (ElementImportInputs& inputs);
    Utf8String              ComputeModelName (Utf8StringR proposedName, BeFileNameCR baseFileName, BeFileNameCR refPath, Utf8CP inSuffix, DgnClassId modelType);
    BentleyStatus           ImportXrefModelsFrom (DwgXRefHolder& xref, SubjectCR parentSubject, bool& hasPushedReferencesSubject);
    ECN::ECObjectsStatus    AddAttrdefECClassFromBlock (ECN::ECSchemaPtr& schema, DwgDbBlockTableRecordCR block);
    void                    ImportAttributeDefinitionSchema (ECN::ECSchemaR attrdefSchema);
    void                    ImportDomainSchema (WCharCP fileName, DgnDomain& domain);
    void                    SaveViewDefinition (ViewControllerR viewController);
    void                    CheckSameRootModelAndUnits ();
    void                    ComputeDefaultImportJobName (Utf8StringCR rootModelName);
    Utf8String              GetImportJobNamePrefix () const { return ""; }
    bool                    IsXrefInsertedInPaperspace (DwgDbObjectIdCR xrefInsertId) const;
    bool                    ShouldSkipAllXrefs (ResolvedModelMapping const& ownerModel, DwgDbObjectIdCR ownerSpaceId);
    DgnDbStatus             UpdateElementName (DgnElementR editElement, Utf8StringCR newValue, Utf8CP label = nullptr, bool save = true);
    bool                    UpdateModelspaceView (ViewControllerP view);
    bool                    UpdatePaperspaceView (ViewControllerP view, DwgDbObjectIdCR viewportId);
    BentleyStatus           UpdateRepositoryLink (DwgDbDatabaseP dwg = nullptr);
    BentleyStatus           InsertElementHasLinks (DgnModelR model, DwgDbDatabaseR dwg);
    DgnCategoryId           FindCategoryFromSyncInfo (DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg = nullptr);
    DgnSubCategoryId        FindSubCategoryFromSyncInfo (DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg = nullptr);

    static void             RegisterProtocolExtensions ();

//__PUBLISH_SECTION_START__
protected:
    //! @name  Miscellaneous
    //! @{
    DWG_EXPORT virtual void _BeginImport ();
    DWG_EXPORT virtual void _FinishImport ();
    virtual void            _OnFatalError() { m_wasAborted = true; }
    virtual GeometryOptions&    _GetCurrentGeometryOptions () { return m_currentGeometryOptions; }
    DWG_EXPORT virtual bool _ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity = nullptr);
    BeFileNameCR            GetRootDwgFileName () const { return m_rootFileName; }
    DWG_EXPORT bool         ValidateDwgFile (BeFileNameCR dwgdxfName);

    //! @name  Change-Detection
    //! @{
    DWG_EXPORT  virtual void    _SetChangeDetector (bool updating);
    virtual IDwgChangeDetector& _GetChangeDetector () { return *m_changeDetector; }
    virtual bool                _HaveChangeDetector () { return nullptr != m_changeDetector; }
    DWG_EXPORT virtual BentleyStatus _DetectDeletedDocuments();

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
    enum class ModelSubjectType {Hierarchy, References, GeometryParts};
    SubjectCPtr GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR, ModelSubjectType);
    bool        IsUpdating () const { return GetOptions().IsUpdating(); }
    bool        IsCreatingNewDgnDb () { return GetOptions().IsCreatingNewDgnDb(); }

    //! @name Importing schemas
    //! @see public method MakeSchemaChanges
    //! @{
    //! Cache a PresentationRule content of a host element which must be seperated from the modelspace as a PhysicalObject and a paperspace as a DrawingGraphic.
    DWG_EXPORT BentleyStatus  AddPresentationRuleContent (DgnElementCR hostElement, Utf8StringCR attrdefName);
    //! Create and embed PresentationRules for DwgAttributeDefinitions schema:
    DWG_EXPORT virtual BentleyStatus  _EmbedPresentationRules ();

    //! @name  Creating DgnModels for DWG
    //! @{
    //! Modelspace and xRef blocks as Physical Models, layout blocks as sheet models
    //! Set the geo location.  Calculate and cache units, active viewport, etc.
    DWG_EXPORT virtual BentleyStatus  _ImportSpaces ();
    //! Walk through the block section, get or create DgnModel's from layouts and xRef attachments.
    //! @note This is a model discovery phase.  It only creates DgnModel's.  Model filling will take place in _ImportEntitySection and _ImportLayouts, based on model mappings.
    DWG_EXPORT virtual BentleyStatus  _ImportDwgModels ();
    //! Get or create a DgnModel for the input DWG layout/paperspace block
    //! @param[in] block A layout/paperspace block definition
    //! @return A ResolvedModelMapping created or retrieved for the input DWG block.  Return an invalid mapping to skip the layout.
    DWG_EXPORT virtual ResolvedModelMapping _ImportLayoutModel (DwgDbBlockTableRecordCR block);
    //! Get to create a DgnModel for the input DWG xReference attachment
    //! @param[in] block An xRef block definition
    //! @param[in] insertId An instance of the xRef block (aka as insert entity)
    //! @param[in] xRefDwg The xReference file's database created by this importer via DwgXRefHolder
    //! @return A ResolvedModelMapping created or retrieved for the input DWG block.  Return an invalid mapping to skip the xRef instance.
    DWG_EXPORT virtual ResolvedModelMapping _ImportXrefModel (DwgDbBlockTableRecordCR block, DwgDbObjectIdCR insertId, DwgDbDatabaseP xRefDwg);
    //! Create the root model from the modelspace block if importing, or retrieve it from the syncInfo if updating.
    //! @param[in] updating True for updating existing DgnDb; false for the initial creation of DgnDb.
    DWG_EXPORT virtual ResolvedModelMapping _GetOrCreateRootModel (bool updating);
    //! Set unit formats to be displayed in the DgnModel created from the input block.
    //! @param[out] displayInfo Unit formats to be displayed in the model created from the input block
    //! @param[in] block A modelspace, layout/paperspace, or xRef block
    DWG_EXPORT virtual void       _SetModelUnits (GeometricModel::Formatter& displayInfo, DwgDbBlockTableRecordCR block);
    //! Get a DgnModel from the syncInfo for updating, or create a new DgnModel for importing, from a DWG model/paperspace or an xref (when xrefInsert!=nullptr & xrefDwg!=nullptr)
    DWG_EXPORT ResolvedModelMapping GetOrCreateModelFromBlock (DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert = nullptr, DwgDbDatabaseP xrefDwg = nullptr);
    //! Partition the parent subject and create a DefinitionModel dedicated for GeometryParts
    DWG_EXPORT BentleyStatus      GetOrCreateGeometryPartsModel ();
    DWG_EXPORT ResolvedModelMapping GetRootModel () const { return  m_rootDwgModelMap; }
    //! Retrieve model mapping from the sync info
    DWG_EXPORT ResolvedModelMapping GetModelFromSyncInfo (DwgDbObjectIdCR id, DwgDbDatabaseR dwg, TransformCR trans);
    //! Create a new model mapping and insert it to the sync info, returning the new entry
    //! @param[in] model A DgnModel to be mapped
    //! @param[in] block A DWG block to be mapped
    //! @param[in] trans A tranformation for the model mapping
    //! @param[in] xrefInsert An instance of xRef, null if not an xRef model mapping
    //! @param[in] xrefDwg The DWG database of the xRef file, null if not an xRef model mapping
    DWG_EXPORT ResolvedModelMapping CreateAndInsertModelMap (DgnModelP model, DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert = nullptr, DwgDbDatabaseP xrefDwg = nullptr);
    DWG_EXPORT bool               AddToDwgModelMap (ResolvedModelMapping const&);
    DWG_EXPORT ResolvedModelMapping FindRootModelFromImportJob ();
    //! Find a cached DgnModel mapped from a DWG "model", with matching transformation.  Only search the cached map, no attempt to search in the syncInfo.
    DWG_EXPORT ResolvedModelMapping FindModel (DwgDbObjectIdCR dwgModelId, TransformCR trans, DwgSyncInfo::ModelSourceType source);
    //! Find a cached DgnModel mapped from a DWG "model", ignoring transformation. Only search the cached map, no attempt to search in the syncInfo.
    DWG_EXPORT ResolvedModelMapping FindModel (DwgDbObjectIdCR dwgModelId, DwgSyncInfo::ModelSourceType sourceType);
    DWG_EXPORT Utf8String         RemapModelName (Utf8StringCR name, BeFileNameCR, Utf8StringCR suffix);
    DWG_EXPORT virtual Utf8String _ComputeModelName (DwgDbBlockTableRecordCR block, Utf8CP suffix = nullptr);
    DWG_EXPORT virtual DgnClassId _GetModelType (DwgDbBlockTableRecordCR block);

    //! @name  Importing layers
    //! @{
    // The layer section contains all layers of a DWG file
    DWG_EXPORT virtual BentleyStatus  _ImportLayerSection ();
    DWG_EXPORT virtual size_t         _ImportLayersByFile (DwgDbDatabaseP dwg);
    DWG_EXPORT virtual BentleyStatus  _ImportLayer (DwgDbLayerTableRecordCR layer, DwgStringP overrideName = nullptr);
    DWG_EXPORT virtual BentleyStatus  _OnUpdateLayer (DgnCategoryId&, DwgDbLayerTableRecordCR);
    BentleyStatus                     GetLayerAppearance (DgnSubCategory::Appearance& appearance, DwgDbLayerTableRecordCR layer, DwgDbObjectIdCP viewportId = nullptr);

    //! @name  Importing viewport table
    //! @{
    // The viewport section contains all modelspace viewport table records, each of which may be attached with a view table record.
    // Each layout may have multiple viewports, the first of which is the paperspace viewport which should be used for the sheet model
    // imported from the layout block.
    DWG_EXPORT virtual BentleyStatus  _ImportModelspaceViewports ();
    DWG_EXPORT virtual DgnViewId      _ImportModelspaceViewport (DwgDbViewportTableRecordCR vport);
    DWG_EXPORT virtual BentleyStatus  _ImportPaperspaceViewport (DgnModelR model, TransformCR transform, DwgDbLayoutCR layout);
    DWG_EXPORT virtual void           _PostProcessViewports ();

    //! @name  Importing text style table
    //! @{
    // The text style section contains all text styles used in a DWG file
    DWG_EXPORT virtual BentleyStatus          _ImportTextStyleSection ();
    DWG_EXPORT virtual AnnotationTextStyleCP  _ImportTextStyle (DwgDbTextStyleTableRecordCR dwgStyle);
    DWG_EXPORT virtual void                   _EmbedFonts ();

    //! @name  Importing line type table
    //! @{
    // The line type section contains all line types used in a DWG file
    DWG_EXPORT virtual BentleyStatus    _ImportLineTypeSection ();
    DWG_EXPORT virtual LineStyleStatus  _ImportLineType (DwgDbLinetypeTableRecordPtr& ltype);
    DWG_EXPORT virtual BentleyStatus    _OnUpdateLineType (DgnStyleId&, DwgDbLinetypeTableRecordCR);

    //! @name  Importing materials
    //! @{
    // The materials dictionay contains all materials used in a DWG file
    DWG_EXPORT virtual BentleyStatus    _ImportMaterialSection ();
    DWG_EXPORT virtual BentleyStatus    _ImportMaterial (DwgDbMaterialPtr& material, Utf8StringCR paletteName, Utf8StringCR materialName);
    DWG_EXPORT virtual BentleyStatus    _OnUpdateMaterial (DwgSyncInfo::Material const& syncMaterial, DwgDbMaterialPtr& dwgMaterial);
    //! Search material paths specified in the Config file.  If a match found, replace the file name.
    DWG_EXPORT virtual bool             _FindTextureFile (BeFileNameR filename) const;
    DWG_EXPORT bvector<BeFileName> const& GetMaterialSearchPaths () const { return m_materialSearchPaths; }

    //! @name  Importing entities
    //! @{
    // DWG entity section is the ModelSpace block containing graphical entities
    DWG_EXPORT virtual BentleyStatus  _ImportEntitySection ();
    //! Import a database-resident entity
    //! @note This is the default method that converts a modelspace or paperspace entity to BIM.
    //! The import process first tries to convert an entity via an object protocol extension, DwgProtocolExtension.
    //! If the entity does not have DwgProtocolExtension, it gets directly sent into this method for conversion.
    //! If the entity has DwgProtocolExtension, it gets sent into_ImportEntityByProtocolExtension.  The entity extension
    //! may convert the entity data completely on its own. It may also alternatively fallback to call this method 
    //! as it sees appropriate.
    //! @see _ImportEntityByProtocolExtension
    DWG_EXPORT virtual BentleyStatus  _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a database-resident entity that is implemented by DwgProtocolExtension
    //! @see _ImportEntity
    DWG_EXPORT virtual BentleyStatus  _ImportEntityByProtocolExtension (ElementImportResults& results, ElementImportInputs& inputs, DwgProtocolExtension& ext);
    //! Import an xReference entity
    DWG_EXPORT virtual BentleyStatus  _ImportXReference (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a normal block reference entity
    DWG_EXPORT virtual BentleyStatus  _ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs);
    //! This method is called to setup ElementCreatParams for each entity to be imported by default:
    DWG_EXPORT virtual BentleyStatus  _GetElementCreateParams (ElementCreateParams& params, TransformCR toDgn, DwgDbEntityCR entity, Utf8CP desiredCode = nullptr);
    //! Determine DgnClassId for an entity by its owner block
    DWG_EXPORT virtual DgnClassId     _GetElementType (DwgDbBlockTableRecordCR block);
    //! Determine graphical element label from an entity
    DWG_EXPORT virtual Utf8String     _GetElementLabel (DwgDbEntityCR entity);
    //! Should the entity be imported at all?
    DWG_EXPORT virtual bool           _FilterEntity (ElementImportInputs& inputs) const;
    //! Should create a DgnElement if there is no geometry at all?
    DWG_EXPORT virtual bool           _SkipEmptyElement (DwgDbEntityCP entity);
    //! Insert imported DgnElement into DgnDb.  This method is called after _ImportEntity.
    DWG_EXPORT DgnDbStatus    InsertResults (ElementImportResults& results);
    DWG_EXPORT DgnDbStatus    UpdateResults (ElementImportResults& results, DgnElementId existingElement);
    //! Insert or update imported DgnElement and source DWG entity in DwgSynchInfo
    DWG_EXPORT BentleyStatus  InsertOrUpdateResultsInSyncInfo (ElementImportResults& results, IDwgChangeDetector::DetectionResults const& updatePlan, DwgDbEntityCR entity, DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId);
    //! Create a new or update an existing element from an entity based on the sync info
    DWG_EXPORT BentleyStatus  ImportOrUpdateEntity (ElementImportInputs& inputs);

    //! @name  Importing layouts
    //! @{
    //! A DWG layout is made up by a Paperspace block containing graphical entities and a sheet/plot definition.
    DWG_EXPORT virtual BentleyStatus  _ImportLayouts ();
    DWG_EXPORT virtual BentleyStatus  _ImportLayout (ResolvedModelMapping& modelMap, DwgDbBlockTableRecordR block, DwgDbLayoutCR layout);

    //! @name  Importing groups
    //! @{
    //! After models and elements have been processed, this method will be called to process all DWG groups in all files.
    //! The default implementations will import DWG groups as GenericGroup elements.
    //! @note Multiple attachments of the same xRef file are imported as individual DgnModel's. A group element created from a group in such an xRef will contain member elements across these models, by the default implementation.
    DWG_EXPORT virtual BentleyStatus  _ImportGroups ();
    //! Import dictionary groups from a DWG file.  This method is called for the root DWG file, then followed by each of its xRef files.
    //! In an updating job, this method also consults the sync info and calls _OnUpdateGroup on an existing DWG group.
    //! @param[in] dwg Input root or xRef DWG file whose group dictionary will be processed.
    DWG_EXPORT virtual BentleyStatus  _ImportGroups (DwgDbDatabaseCR dwg);
    //! Create and insert a new group element from a DWG group which may be in either a root or an xRef DWG file.
    //! @param dwgGroup Input object of the DWG group dictionary.
    //! @return A new and inserted DgnDb group element.  The default implementation creates a GenericGroup.
    DWG_EXPORT virtual DgnElementPtr  _ImportGroup (DwgDbGroupCR dwgGroup);
    //! Update existing group element.
    //! @param[out] dgnGroup Existing DgnDb group element to be updated.
    //! @param[in] dwgGroup Input object of the DWG group from which the DgnDb group will be updated.
    DWG_EXPORT virtual BentleyStatus  _UpdateGroup (DgnElementR dgnGroup, DwgDbGroupCR dwgGroup);
    //! Detect existing DWG group against its provenance and act according to the detection results.
    //! @param[in] prov Input provenance of the DWG group retrieved from the sync info.
    //! @param[in] dwgGroup Input object of the DWG group dictionary.
    //! @note When a change is detected for a DWG group, _UpdateGroup will be called; otherwise _ImportGroup will be called, by the default implementation.
    DWG_EXPORT virtual BentleyStatus  _OnUpdateGroup (DwgSyncInfo::Group const& prov, DwgDbGroupCR dwgGroup);
    //! Tell DwgSyncInfo how to sync groups: group object alone or members included?
    //! @return True to include members - needed when members are not made persistent elements; False to sync group object only - members are tracked by entity mappings.
    DWG_EXPORT virtual bool _ShouldSyncGroupWithMembers () const { return false; }

    //! @name Options and configs
    //! @{
    virtual void        _OnConfigurationRead (BeXmlDomR configDom) {}
    void                ParseConfigurationFile (T_Utf8StringVectorR userObjectEnablers);
    BentleyStatus       SearchForMatchingRule (ImportRule& entryOut, Utf8StringCR modelName, BeFileNameCR baseFilename);
    virtual Utf8String  _GetFontSearchPaths() const { return m_config.GetXPathString("/ConvertConfig/Fonts/@searchPaths", ""); }
    WorkingFonts const& GetLoadedFonts () const { return m_loadedFonts; }

    //! @name Error and Progress Reporting
    //! @{
    DWG_EXPORT virtual void _ReportIssue (IssueSeverity, IssueCategory::StringId, Utf8CP message, Utf8CP context);
    void                ReportDbFileStatus (BeSQLite::DbResult fileStatus, BeFileNameCR projectFileName);
    void                AddTasks (int32_t n);
    DWG_EXPORT void     SetStepName (ProgressMessage::StringId, ...);
    DWG_EXPORT void     SetTaskName (ProgressMessage::StringId, ...);

    //! @name DWG-DgnDb sync info
    //! @{
    DWG_EXPORT virtual DwgSyncInfo::DwgFileId _AddFileInSyncInfo (DwgDbDatabaseR, StableIdPolicy);
    DWG_EXPORT virtual StableIdPolicy         _GetDwgFileIdPolicy () const;
    DwgSyncInfo::DwgFileId  GetDwgFileId (DwgDbDatabaseR, bool setIfNotExist = true);

    //! @name  Product installations
    //! @{
    //! A callback method which can be overriden by a derivitive product to supply a different root registry key of ObjectDBX - required by and valid for RealDWG only.
    //! @note A product that installs RealDWG differently than the default DwgBridge installer does, must supply the root registry path.
    //! The default method supplies the root registry in this way:
    //! 1) If configuration variable REALDWG_REGISTRY_ROOTKEY is set, the full root key path pointed to by it will be returned, with no validation.
    //! 2) If "HKLM\SOFTWARE\Bentley\ObjectDBX\RealDwgImporter\<ProductCode>\" is installed by the default DwgBridge, its ObjectDBX component registry key will be returned.
    //! 3) Otherwise, an empty root registry key will be returned in all other cases.
    //! @return True if an ObjectDBX root registry is found; false otherwise.
    //! @see IDwgDbHost::_GetRegistryProductRootKey
    DWG_EXPORT virtual bool       _GetRealDwgRootRegistry (WStringR rootKey) const;
    //! @}

public:
    //! An app must hold and pass in the reference of a DwgImporter::Options, which may be changed after a DwgImporter is created.
    DWG_EXPORT DwgImporter (Options& options);
    DWG_EXPORT ~DwgImporter ();

    DWG_EXPORT ImportJobCreateStatus InitializeJob (Utf8CP comment=nullptr, DwgSyncInfo::ImportJob::Type = DwgSyncInfo::ImportJob::Type::RootModels);
    DWG_EXPORT ImportJobLoadStatus FindJob ();
    DWG_EXPORT ResolvedImportJob const& GetImportJob () const { return m_importJob; }
    DWG_EXPORT BentleyStatus    AttachSyncInfo ();
    DwgSyncInfo&                GetSyncInfo () { return m_syncInfo; }
    DWG_EXPORT bool             ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity = nullptr) { return _ArePointsValid(checkPoints, numPoints, entity); }
    DgnFontCP                   GetDgnFontFor (DwgFontInfoCR fontInfo);
    DgnFontCP                   GetDefaultFont () const { return m_defaultFont.get(); }
    AnnotationTextStyleId       GetDefaultTextStyleId () const { return m_defaultTextstyleId; }
    bool                        GetFallbackFontPathForShape (BeFileNameR filename) const;
    DWG_EXPORT void             SetFallbackFontPathForShape (BeFileNameCR filename);
    bool                        GetFallbackFontPathForText (BeFileNameR outName, DgnFontType type) const;
    DWG_EXPORT void             SetFallbackFontPathForText (BeFileNameCR inName, DgnFontType fontType);
    //! @return The root transform information.
    DWG_EXPORT RootTransformInfo const& GetRootTransformInfo () const { return m_rootTransformInfo; }
    //! @return True, if the root transform has been changed from previous import; false, otherwise.
    //! @note This happens when iModelBridge changes its spatial transformation for the same import job.
    DWG_EXPORT bool             HasRootTransformChanged () const { return m_rootTransformInfo.HasChanged(); }
    //! @return Current root transform.
    TransformCR                 GetRootTransform () const { return m_rootTransformInfo.GetRootTransform(); }
    DWG_EXPORT double           GetScaleToMeters () const;
    DwgDbObjectId               GetCurrentViewportId () { return m_currentGeometryOptions.GetViewportId(); }
    DwgDbObjectIdCR             GetCurrentSpaceId () const { return m_currentspaceId; }
    DwgDbObjectIdCR             GetModelSpaceId () const { return m_modelspaceId; }
    StandardUnit                GetModelSpaceUnits () const { return m_modelspaceUnits; }
    DgnStyleId                  GetDgnLineStyleFor (DwgDbObjectIdCR ltypeId);
    DgnElementId                GetDgnTextStyleFor (DwgDbObjectIdCR tstyleId);
    RenderMaterialId            GetDgnMaterialFor (DwgDbObjectIdCR materialId);
    DgnTextureId                GetDgnMaterialTextureFor (Utf8StringCR fileName);
    T_MaterialIdMap&            GetImportedDgnMaterials () { return m_importedMaterials; }
    void                        AddDgnMaterialTexture (Utf8StringCR fileName, DgnTextureId texture);
    ECN::ECSchemaCP             GetAttributeDefinitionSchema () { return m_attributeDefinitionSchema; }
    bool                        GetConstantAttrdefIdsFor (DwgDbObjectIdArray& ids, DwgDbObjectIdCR blockId);
    //! Get a spatial category and/or a sub-category for a modelspace entity layer. The syncInfo is read in and cached for fast retrieval.
    DgnCategoryId               GetSpatialCategory (DgnSubCategoryId& subCategoryId, DwgDbObjectIdCR layerId, DwgDbDatabaseP xrefDwg = nullptr);
    //! Get a drawing category for paperspace entity layer. If the category not already exists, a new one will be created.
    //! @param[out] subCategory A sub-category found or created from inputs
    //! @param[in] layerId A DWG layer from which the sub-category is queried
    //! @param[in] viewportId A DWG viewport for which the sub-category is needed
    //! @param[in] model A DgnDb model into elements using the sub-category will be inserted
    //! @param[in] xrefDwg A DWG file in which a source entity is to be imported using the sub-category.  Null if in master file.
    DgnCategoryId               GetOrAddDrawingCategory (DgnSubCategoryId& subCategory, DwgDbObjectIdCR layerId, DwgDbObjectIdCR viewportId, DgnModelCR model, DwgDbDatabaseP xrefDwg = nullptr);
    DgnSubCategoryId            InsertAlternateSubCategory (DgnSubCategoryCPtr subcategory, DgnSubCategory::Appearance const& appearance, Utf8CP desiredName = nullptr);
    //! Get the block-geometry map that caches imported geometries.
    T_BlockPartsMap&            GetBlockPartsR () { return m_blockPartsMap; }
    //! Get the DefinitionModel that stores GeometryParts
    DefinitionModelPtr          GetGeometryPartsModel () { return m_geometryPartsModel; }
    //! Get/create the DefinitionModel that stores all other job specific definitions, expcept for GeometryParts.
    DefinitionModelPtr          GetOrCreateJobDefinitionModel ();

    //! An iModelBridge must call this method from _MakeSchemaChanges, to change schemass.
    //! The default implementation iterates DWG block table for multiple tasks:
    //! 1) Create or update dynamic DwgAttributeDefinitions schema from ATTRDEF's
    //! 2) Load xRef files and cache them in m_loadedXrefFiles
    //! 3) Cache paperspace/layout block ID's in m_paperspaceBlockIds
    DWG_EXPORT BentleyStatus    MakeSchemaChanges ();
    //! An iModelBridge must call this method from _MakeDefinitionChanges, to change dictionaries possibly shared with other bridges.
    DWG_EXPORT BentleyStatus    MakeDefinitionChanges (SubjectCR jobSubject);

    //! Call this once before working with DwgImporter, after initializing DgnDb's DgnPlatformLib
    //! @param toolkitDir Installed RealDWG or OpenDWG folder; default to the same folder as the EXE.
    DWG_EXPORT static void      Initialize (BentleyApi::BeFileNameCP toolkitDir = nullptr);
    DWG_EXPORT static void      TerminateDwgHost ();
    DWG_EXPORT BentleyStatus    OpenDwgFile (BeFileNameCR dwgdxfName);
    DWG_EXPORT void             SetDgnDb (DgnDbR bim) const { m_dgndb = &bim; }
    DWG_EXPORT DgnDbR           GetDgnDb () const { return *m_dgndb; }
    DWG_EXPORT DwgDbDatabaseR   GetDwgDb () { return *m_dwgdb.get(); }
    DWG_EXPORT BentleyStatus    Process ();
    DWG_EXPORT void             Progress ();
    DWG_EXPORT DgnModelId       CreateModel (DwgDbBlockTableRecordCR block, Utf8CP modelName, DgnClassId classId);
    DWG_EXPORT void             ReportError (IssueCategory::StringId, Issue::StringId, Utf8CP details);
    void                        ReportError (IssueCategory::StringId category, Issue::StringId issue, WCharCP details) {ReportError(category,issue,Utf8String(details).c_str());}
    DWG_EXPORT void             ReportIssueV (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP context, ...);
    DWG_EXPORT void             ReportIssue (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details, Utf8CP context = nullptr);
    DWG_EXPORT void             ReportSyncInfoIssue (IssueSeverity, IssueCategory::StringId, Issue::StringId, Utf8CP details);
    DWG_EXPORT BentleyStatus    OnFatalError (IssueCategory::StringId cat=IssueCategory::Unknown(), Issue::StringId num=Issue::ProgramExits(), ...);
    DWG_EXPORT bool             WasAborted () const { return m_wasAborted; }
    DgnProgressMeterR           GetProgressMeter() const;
    DWG_EXPORT MessageCenter&   GetMessageCenter () { return m_messageCenter; }
    DWG_EXPORT Options const&   GetOptions () const { return m_options; }
    DWG_EXPORT Options&         GetOptionsForEdit () { return m_options; }
    DWG_EXPORT DgnCategoryId    GetUncategorizedCategory () const { return m_uncategorizedCategoryId; }
    DWG_EXPORT CodeSpecId       GetBusinessKeyCodeSpec () const { return m_businessKeyCodeSpecId; }
    StableIdPolicy              GetCurrentIdPolicy () const { return m_currIdPolicy; }
    DwgXRefHolder&              GetCurrentXRefHolder () { return m_currentXref; }
    DwgXRefHolder*              FindXRefHolder (DwgDbBlockTableRecordCR xrefBlock, bool createIfNotFound = false);
    DwgDbDatabaseP              FindLoadedXRef (BeFileNameCR path);
    T_LoadedXRefFiles&          GetLoadedXrefs () { return m_loadedXrefFiles; }
    //! Import a database-resident entity
    DWG_EXPORT BentleyStatus    ImportEntity (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a none database-resident entity in a desired block (must be a valid block)
    DWG_EXPORT BentleyStatus    ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId);
    DWG_EXPORT DgnCode          CreateCode (Utf8StringCR value) const;
    DWG_EXPORT uint32_t         GetEntitiesImported () const { return m_entitiesImported; }
    DWG_EXPORT DgnModelId       GetGroupModelId () const { return m_groupModelId; }
    
    };  // DwgImporter

//=======================================================================================
//! A no-op change detector for creating DgnDb from DWG
//=======================================================================================
struct CreatorChangeDetector : IDwgChangeDetector
{
public:
    //! @{
    void    _Prepare (DwgImporter&) override {}
    void    _Cleanup (DwgImporter&) override {}
    bool    _ShouldSkipFile (DwgImporter&, DwgDbDatabaseCR) override { return false; }
    bool    _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const& m,  DwgDbDatabaseCP xref = nullptr) override { return false; }
    void    _OnModelSeen (DwgImporter&, ResolvedModelMapping const& m) override {}
    void    _OnViewSeen (DwgImporter&, DgnViewId) override {}
    void    _OnGroupSeen (DwgImporter&, DgnElementId) override {}
    void    _OnModelInserted (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP) override {}
    void    _OnElementSeen (DwgImporter&, DgnElementId) override {}
    void    _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) override {}
    void    _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) override {}
    void    _DetectDeletedElementsEnd (DwgImporter&) override {}
    void    _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) override {}
    void    _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) override {}
    void    _DetectDeletedModelsEnd (DwgImporter&) override {}
    void    _DetectDeletedMaterials (DwgImporter&) override {}
    void    _DetectDeletedViews (DwgImporter&) override {}
    void    _DetectDeletedGroups (DwgImporter&) override {}
    void    _DetectDetachedXrefs (DwgImporter&) override {}

    //! always fills in element provenence and returns true
    DWG_EXPORT bool   _IsElementChanged (DetectionResults&, DwgImporter&, DwgDbObjectCR, ResolvedModelMapping const&, T_DwgSyncInfoElementFilter*) override;
    CreatorChangeDetector () {}
};  // CreatorChangeDetector

//=======================================================================================
//! A change detector to help updating DgnDb previously imported from DWG
//=======================================================================================
struct UpdaterChangeDetector : IDwgChangeDetector
{
private:
    DwgSyncInfo::ByDwgObjectIdIter*         m_byIdIter;
    DwgSyncInfo::ByHashIter*                m_byHashIter;
    DgnElementIdSet                         m_elementsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSkipped;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_newlyDiscoveredModels;
    bset<DgnViewId>                         m_viewsSeen;
    DgnElementIdSet                         m_groupsSeen;
    uint32_t                                m_elementsDiscarded;

    bool    IsUpdateRequired (DetectionResults& results, DwgImporter& importer, DwgDbObjectCR obj) const;

public:
    UpdaterChangeDetector () : m_byIdIter(nullptr), m_byHashIter(nullptr) {}
    DWG_EXPORT ~UpdaterChangeDetector ();
    DWG_EXPORT void _Prepare(DwgImporter&) override;
    DWG_EXPORT void _Cleanup(DwgImporter&) override;

    //! @name  Override tracking & detection methods
    //! @{
    DWG_EXPORT bool   _ShouldSkipFile (DwgImporter&, DwgDbDatabaseCR) override;
    DWG_EXPORT bool   _ShouldSkipModel (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP xref = nullptr) override;
    DWG_EXPORT void   _OnModelSeen (DwgImporter&, ResolvedModelMapping const&) override;
    DWG_EXPORT void   _OnModelInserted (DwgImporter&, ResolvedModelMapping const&, DwgDbDatabaseCP xRef) override;
    DWG_EXPORT void   _OnViewSeen (DwgImporter&, DgnViewId) override;
    DWG_EXPORT void   _OnGroupSeen (DwgImporter&, DgnElementId) override;
    DWG_EXPORT void   _OnElementSeen (DwgImporter&, DgnElementId) override;
    DWG_EXPORT bool   _IsElementChanged (DetectionResults&, DwgImporter&, DwgDbObjectCR, ResolvedModelMapping const&, T_DwgSyncInfoElementFilter* filter) override;
    //! @}

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit
    //! @{
    //! delete elements
    DWG_EXPORT void   _DetectDeletedElements (DwgImporter&, DwgSyncInfo::ElementIterator&) override;
    DWG_EXPORT void   _DetectDeletedElementsInFile (DwgImporter&, DwgDbDatabaseR) override;  //!< don't forget to call _DetectDeletedElementsEnd when done
    DWG_EXPORT void   _DetectDeletedElementsEnd (DwgImporter&) override { m_elementsSeen.clear(); }
    //! delete models
    DWG_EXPORT void   _DetectDeletedModels (DwgImporter&, DwgSyncInfo::ModelIterator&) override;
    DWG_EXPORT void   _DetectDeletedModelsInFile (DwgImporter&, DwgDbDatabaseR) override;    //!< don't forget to call _DetectDeletedModelsEnd when done
    DWG_EXPORT void   _DetectDeletedModelsEnd (DwgImporter&) override {m_dwgModelsSeen.clear();}
    //! delete tables
    DWG_EXPORT void   _DetectDeletedMaterials (DwgImporter&) override;
    DWG_EXPORT void   _DetectDeletedViews (DwgImporter&) override;
    DWG_EXPORT void   _DetectDeletedGroups (DwgImporter&) override;
    DWG_EXPORT void   _DetectDetachedXrefs (DwgImporter&) override;
    //! @}
};  // UpdaterChangeDetector

END_DWG_NAMESPACE
//__PUBLISH_SECTION_END__
