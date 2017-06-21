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
#include <Dgnplatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <BeSQLite/L10N.h>
#include <BeXml/BeXml.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnDbSync/DgnDbSync.h>
#include <DgnDbSync/Dwg/DwgSyncInfo.h>

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
};

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgImporter
    {
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

    struct Options
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
        BeFileName          m_reportFile;
        BeFileName          m_configFile;
        BeFileName          m_configFile2;
        BeFileName          m_changesFile;
        Utf8String          m_description;
        Utf8String          m_password;
        DateTime            m_time;
        DateTime            m_expirationDate;
        DgnProgressMeter*   m_meter;
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
        void SetReportFile (BentleyApi::BeFileNameCR fileName) {m_reportFile = fileName;}
        void SetConfigFile (BentleyApi::BeFileNameCR fileName) {m_configFile = fileName;}
        void SetConfigFile2 (BentleyApi::BeFileNameCR fileName) {m_configFile2 = fileName;}
        void SetChangesFile (BentleyApi::BeFileNameCR fileName) {m_changesFile = fileName;}
        void SetDescription (BentleyApi::Utf8CP descr) {m_description=descr;}
        void SetTime (DateTime tm) {m_time=tm;}
        void SetPassword (BentleyApi::Utf8CP pw) {m_password=pw;}
        void SetProgressMeter (DgnProgressMeter* meter) {m_meter = meter;}
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

        BeFileNameCR GetInputRootDir() const {return m_rootDir;}
        BeFileNameCR GetReportFile() const {return m_reportFile;}
        BeFileNameCR GetConfigFile() const {return m_configFile;}
        BeFileNameCR GetConfigFile2() const {return m_configFile2;}
        BeFileNameCR GetChangesFile() const {return m_changesFile;}
        DgnProgressMeter* GetProgressMeter() const {return m_meter;}
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
        uint32_t GetDgnLineWeight (DwgDbLineWeight dwgWeight);
        bool GetSyncBlockChanges () { return m_syncBlockChanges; }
        bool GetImportRasterAttachments () { return m_importRasters; }
        bool GetImportPointClouds () { return m_importPointClouds; }
        uint16_t GetPointCloudLevelOfDetails () { return m_pointCloudLevelOfDetails; }
        bool IsRenderableGeometryPrefered () { return m_preferRenderableGeometry; }
        };

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
        };
    typedef bmultiset<ResolvedModelMapping>     T_DwgModelMapping;

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
        DwgDbDatabaseP  GetDatabase() { return m_xrefDatabase.get(); }
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
        bool                            m_isUpdating;
        DgnElementId                    m_existingElementId;
        DgnElementPtr                   m_importedElement;
        bvector<ElementImportResults>   m_childElements;
    public:
        ElementImportResults () : m_importedElement(nullptr), m_isUpdating(false) { m_childElements.clear();}
        ElementImportResults (DgnElementP newElement) : m_importedElement(newElement), m_isUpdating(false) { m_childElements.clear(); }
        bool            IsUpdating () { return  m_isUpdating; }
        void            SetIsUpdating (bool update) { m_isUpdating = update; }
        DgnElementP     GetImportedElement () { return m_importedElement.IsValid() ? m_importedElement.get() : nullptr; }
        void            AddChildResults (ElementImportResults& child) { m_childElements.push_back(child); }
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
        DgnMaterialId               m_materialId;
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
    typedef bpair<DwgDbObjectId, DgnMaterialId>         T_DwgDgnMaterialId;
    typedef bmap<DwgDbObjectId, DgnMaterialId>          T_MaterialIdMap;
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
    BENTLEY_TRANSLATABLE_STRINGS_START(IssueCategory, dwg_issueCategory)
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
    BENTLEY_TRANSLATABLE_STRINGS_END

    //! A problem in the conversion process
    BENTLEY_TRANSLATABLE_STRINGS_START(Issue, dwg_issue)
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
        L10N_STRING(CantCreateRaster)            // =="Cannot create raster attachment [%s]."==
        L10N_STRING(RootModelChanged)            // =="The original root model was deleted or has changed units."==
        L10N_STRING(RootModelMustBePhysical)     // =="Root model [%s] is not a physical model."==
        L10N_STRING(SaveError)                   // =="An error occurred when saving changes (%s)"==
        L10N_STRING(SeedFileMismatch)            // =="Seed file [%s] does not match target [%s]"==
        L10N_STRING(SyncInfoInconsistent)        // =="The syncInfo file [%s] is inconsistent with the project"==
        L10N_STRING(SyncInfoTooNew)              // =="Sync info was created by a later version"==
        L10N_STRING(ViewNoneFound)               // =="No view was found"==
    BENTLEY_TRANSLATABLE_STRINGS_END

    //! Progress messages for the conversion process
    BENTLEY_TRANSLATABLE_STRINGS_START(ProgressMessage, dwg_progress)
        L10N_STRING(STEP_CLEANUP_EMPTY_TABLES)         // =="Cleaning up empty tables"==
        L10N_STRING(STEP_OPENINGFILE)                  // =="Opening File %ls"==
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
        L10N_STRING(TASK_LOADING_FONTS)                // =="Loading %s Fonts"==
        L10N_STRING(STEP_UPDATING)                     // =="Updating DgnDb"==
        L10N_STRING(TASK_IMPORTING_MODEL)              // =="Model: %s"==
        L10N_STRING(TASK_IMPORTING_RASTERDATA)         // =="Importing raster data file: %s"==
        L10N_STRING(TASK_CREATING_THUMBNAIL)           // =="Creating thumbnail for: %s"==
        L10N_STRING(STEP_IMPORTING_MATERIALS)          // =="Importing Materials"==
        L10N_STRING(STEP_IMPORTING_ATTRDEFSCHEMA)      // =="Importing Attribute Definition Schema [%d classes]"==
    BENTLEY_TRANSLATABLE_STRINGS_END

    //! Miscellaneous strings needed for DwgImporter
    BENTLEY_TRANSLATABLE_STRINGS_START(DataStrings, dwg_dataStrings)
        L10N_STRING(AttrdefsSchemaDescription)         // =="Block attribute definitions created from DWG file %ls"==
        L10N_STRING(BlockAttrdefDescription)           // =="Attribute definitions created from block %ls"==
    BENTLEY_TRANSLATABLE_STRINGS_END
    
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
    DgnDbPtr                    m_dgndb;
    DwgDbDatabasePtr            m_dwgdb;
    BeFileName                  m_rootFileName;
    Transform                   m_rootTransform;
    DwgDbObjectId               m_modelspaceId;
    DwgDbObjectId               m_currentspaceId;
    T_DwgModelMapping           m_dwgModelMap;
    bool                        m_isProcessingDwgModelMap;
    StandardUnit                m_modelspaceUnits;
    bool                        m_wasAborted;
    Options                     m_options;
    GeometryOptions             m_currentGeometryOptions;
    DwgSyncInfo                 m_syncInfo;
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

    static void             RegisterProtocalExtensions ();

protected:
    virtual bool                        _IsUpdating() { return false; }
    virtual bool                        _IsCreatingNewDgnDb() { return true; }
    virtual void                        _BeginImport () { ; }
    DGNDBSYNC_EXPORT virtual void       _FinishImport ();
    virtual void                        _OnFatalError() { m_wasAborted = true; }
    virtual GeometryOptions&            _GetCurrentGeometryOptions () { return m_currentGeometryOptions; }
    DGNDBSYNC_EXPORT virtual bool       _ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity = nullptr);
    BeFileNameCR                        GetRootDwgFileName () const { return m_rootFileName; }

    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportSpaces ();

    //! @name  Creating DgnModels for DWG
    //! @{
    // Modelspace and xRef blocks as Physical Models, layout blocks as sheet models
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportDwgModels ();
    //! Call this to check if all of the elements in the specified model may be skipped during update processing.
    //! @note do not call this during the model-discovery phase(_ImportDwgModels) but only during the entity import phase.
    //! @note for an xref attachment, pass a valid xref dwg.
    virtual bool                        _ShouldSkipModel (ResolvedModelMapping const& modelMap) { return false; }

    //! @name  Importing models from blocks
    //! @{
    // The block section contains all block definitions of a DWG file
    virtual void                        _OnModelSeen (ResolvedModelMapping const& m) {}
    virtual void                        _OnModelInserted (ResolvedModelMapping const& m) {}
    DGNDBSYNC_EXPORT virtual void       _SetModelUnits (GeometricModel::Formatter& displayInfo, DwgDbBlockTableRecordCR block);
    // get or create a new DgnModel from a model/paperspace or an xref (when xrefInsert!=nullptr & xrefDwg!=nullptr)
    ResolvedModelMapping                GetOrCreateModelFromBlock (DwgDbBlockTableRecordCR block, TransformCR trans, DwgDbBlockReferenceCP xrefInsert = nullptr, DwgDbDatabaseP xrefDwg = nullptr);
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
    DGNDBSYNC_EXPORT virtual BentleyStatus  _OnUpdateLayer (DgnCategoryId&, DwgDbLayerTableRecordCR) { return BSISUCCESS; }
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
    DGNDBSYNC_EXPORT virtual BentleyStatus          _OnUpdateLineType (DgnStyleId&, DwgDbLinetypeTableRecordCR) { return BSISUCCESS; }

    //! @name  Importing materials
    //! @{
    // The materials dictionay contains all materials used in a DWG file
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportMaterialSection ();
    DGNDBSYNC_EXPORT virtual BentleyStatus          _ImportMaterial (DwgDbMaterialPtr& material, Utf8StringCR paletteName, Utf8StringCR materialName);
    DGNDBSYNC_EXPORT virtual BentleyStatus          _OnUpdateMaterial (DwgSyncInfo::Material const& syncMaterial, DwgDbMaterialPtr& dwgMaterial) { return BSISUCCESS; }

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
    DGNDBSYNC_EXPORT virtual DgnDbStatus    _InsertResults (ElementImportResults& results, DgnElementId parentId);
    //! Insert imported DgnElement and source DWG entity into DwgSynchInfo
    BentleyStatus                           InsertResultsInSyncInfo (ElementImportResults& results, DwgDbEntityCR entity, DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId);

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
    bool                                WasAborted () const { return m_wasAborted; }
    void                                AddTasks (int32_t n);
    DGNDBSYNC_EXPORT void               SetStepName (ProgressMessage::StringId, ...);
    DGNDBSYNC_EXPORT void               SetTaskName (ProgressMessage::StringId, ...);

    //! @name DWG-DgnDb sync info
    //! @{
    DGNDBSYNC_EXPORT virtual void       _AddFileInSyncInfo (DwgDbDatabaseP, StableIdPolicy);
    DGNDBSYNC_EXPORT virtual void       _AddDefaultRootGuestSyncInfo ();
    BentleyStatus                       AttachSyncInfo ();
    DGNDBSYNC_EXPORT virtual StableIdPolicy _GetDwgFileIdPolicy (DwgDbDatabaseCR) const;

public:
    DGNDBSYNC_EXPORT DwgImporter (Options& options);
    DGNDBSYNC_EXPORT ~DwgImporter ();

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
    DgnMaterialId                       GetDgnMaterialFor (DwgDbObjectIdCR materialId);
    DgnTextureId                        GetDgnMaterialTextureFor (Utf8StringCR fileName);
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

    DGNDBSYNC_EXPORT static void        InitializeDgnHost (DgnPlatformLib::Host& dgndbHost);
    DGNDBSYNC_EXPORT static void        TerminateDwgHost ();
    DGNDBSYNC_EXPORT BentleyStatus      OpenDwgFile (BeFileNameCR dwgdxfName);
    DGNDBSYNC_EXPORT BentleyStatus      CreateNewDgnDb (BeFileNameCR projectName);
    DGNDBSYNC_EXPORT BentleyStatus      OpenExistingDgnDb (BeFileNameCR projectName);
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
    DGNDBSYNC_EXPORT Utf8String         GetDescription () const { return m_options.GetDescription(); }
    DgnProgressMeter*                   GetProgressMeter() const { return m_options.GetProgressMeter(); }
    DGNDBSYNC_EXPORT MessageCenter&     GetMessageCenter () { return m_messageCenter; }
    DGNDBSYNC_EXPORT Options&           GetOptions () { return m_options; }
    DGNDBSYNC_EXPORT DgnCategoryId      GetUncategorizedCategory () const { return m_uncategorizedCategoryId; }
    DGNDBSYNC_EXPORT CodeSpecId         GetBusinessKeyCodeSpec () const { return m_businessKeyCodeSpecId; }
    StableIdPolicy                      GetCurrentIdPolicy () const { return m_currIdPolicy; }
    DwgXRefHolder&                      GetCurrentXRefHolder () { return m_currentXref; }
    DwgDbDatabaseP                      FindLoadedXRef (BeFileNameCR path);
    //! Import a database-resident entity
    DGNDBSYNC_EXPORT BentleyStatus      ImportEntity (ElementImportResults& results, ElementImportInputs& inputs);
    //! Import a none database-resident entity in a desired block (must be a valid block)
    DGNDBSYNC_EXPORT BentleyStatus      ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId, Utf8StringCR desiredCode);
    DGNDBSYNC_EXPORT DgnCode            CreateCode (Utf8StringCR value) const;
    
    };  // DwgImporter


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          03/16
+===============+===============+===============+===============+===============+======*/
struct DwgUpdater : DwgImporter
    {
    DEFINE_T_SUPER (DwgImporter)

private:
    DwgSyncInfo::ByDwgObjectIdIter*         m_byIdIter;
    DwgSyncInfo::ByHashIter*                m_byHashIter;
    DgnElementIdSet                         m_elementsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSeen;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_dwgModelsSkipped;
    bset<DwgSyncInfo::DwgModelSyncInfoId>   m_newlyDiscoveredModels;
    uint32_t                                m_elementsDiscarded;
    DwgSyncInfo::ImportJob                  m_rootGuest;

    DGNDBSYNC_EXPORT bool   ShouldSkipFile (DwgDbDatabaseCR);

public:
    //! Check new entity vs old entity in syncInfo and returns true if same
    bool    CheckEntity (DgnElementId& oldId, ElementImportInputs& inputs, DwgSyncInfo::DwgObjectProvenance const& newProv, bool hashBlocks);
    //! Call this whenever a DWG object is encountered, regardless of whether it is converted or not.
    DGNDBSYNC_EXPORT void   OnElementSeen (DgnElementP el) {if (el != nullptr) OnElementSeen(el->GetElementId());}
    //! Call this whenever a DWG object is encountered, regardless of whether it is converted or not.
    DGNDBSYNC_EXPORT void   OnElementSeen (DgnElementId id);

    //! @name  Element Processing
    //! @{
    DgnDbStatus             UpdateResults (ElementImportResults& results, DgnElementId parentId);
    DgnDbStatus             UpdateResultsInSyncInfo (ElementImportResults& results, ElementImportInputs& inputs, DwgSyncInfo::DwgObjectProvenance const& prov);
    //! @}

    //! @name  Model Processing
    //! @{
    DGNDBSYNC_EXPORT void   CheckSameRootModelAndUnits ();
    //! @}

    //! @name  Inferring Deletions - call these methods after processing all models in a conversion unit
    //! @{
    DGNDBSYNC_EXPORT void   DetectDeletedElements (DwgSyncInfo::ElementIterator&);
    DGNDBSYNC_EXPORT void   DetectDeletedModels (DwgSyncInfo::ModelIterator&);
    DGNDBSYNC_EXPORT void   DeleteDeletedMaterials ();
    //! @}

    //! @name  ImportJob
    //! @{
    DwgSyncInfo::ImportJob const&   GetRootGuest() { return m_rootGuest; }
    //! @}

    //! @name  Constructor and Updater Properties
    //! @{
    DwgUpdater (Options& options) : T_Super(options), m_byIdIter(nullptr), m_byHashIter(nullptr), m_elementsDiscarded(0) {;}

    //! @name  Override super class methods
    //! @{
    virtual bool                            _IsUpdating () override { return true; }
    virtual bool                            _IsCreatingNewDgnDb() override { return false; }
    DGNDBSYNC_EXPORT virtual void           _BeginImport () override;
    DGNDBSYNC_EXPORT virtual void           _FinishImport () override;
    DGNDBSYNC_EXPORT virtual bool           _ShouldSkipModel (ResolvedModelMapping const& m) override;
    DGNDBSYNC_EXPORT virtual void           _OnModelSeen (ResolvedModelMapping const& m) override;
    DGNDBSYNC_EXPORT virtual void           _OnModelInserted (ResolvedModelMapping const& m) override;
    DGNDBSYNC_EXPORT virtual BentleyStatus  _OnUpdateLayer (DgnCategoryId&, DwgDbLayerTableRecordCR) override;
    DGNDBSYNC_EXPORT virtual BentleyStatus  _OnUpdateLineType (DgnStyleId&, DwgDbLinetypeTableRecordCR) override;
    DGNDBSYNC_EXPORT virtual BentleyStatus  _OnUpdateMaterial (DwgSyncInfo::Material const& syncMaterial, DwgDbMaterialPtr& dwgMaterial) override;
    //! override these to catch all entities in OnElementSeen
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportEntity (ElementImportResults& results, ElementImportInputs& inputs) override;
    DGNDBSYNC_EXPORT virtual BentleyStatus  _ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs) override;
    virtual void                            _AddDefaultRootGuestSyncInfo () override { ; }
    //! @}
    };  // DwgUpdater

END_DGNDBSYNC_DWG_NAMESPACE
