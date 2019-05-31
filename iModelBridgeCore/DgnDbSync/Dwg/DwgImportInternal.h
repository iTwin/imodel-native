/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

// only include this before including OpenDWG stuff - RealDWG has an MFC dependency:
#if defined(BENTLEY_WIN32) && defined(DWGTOOLKIT_OpenDwg)
#include    <Windows.h>                 // LOGFONTW etc
#include    <Shlwapi.h>                 // PathIsURL
#endif

#include <Bentley/Bentley.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/Desktop/FileSystem.h>

#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/LineStyle.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <DgnPlatform/DgnBrep/PSolidUtil.h>

#include <Raster/RasterApi.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

#include <Dwg/DwgDb/DwgDbCommon.h>
#include <Dwg/DwgDb/BasicTypes.h>
#include <Dwg/DwgDb/DwgResBuf.h>
#include <Dwg/DwgDb/DwgDbDatabase.h>
#include <Dwg/DwgDb/DwgDbObjects.h>
#include <Dwg/DwgDb/DwgDbEntities.h>
#include <Dwg/DwgDb/DwgDbSymbolTables.h>
#include <Dwg/DwgDb/DwgDrawables.h>
#include <Dwg/DwgDb/DwgDbHost.h>
#include <Dwg/DwgImporter.h>
#include <Dwg/DwgSourceAspects.h>
#include <Dwg/DwgHelper.h>
#include <Dwg/ProtocalExtensions.h>
#include <Dwg/DwgL10N.h>

#include <Logging/BentleyLogging.h>

#define LOG             DwgImportLogging::GetLogger (DwgImportLogging::Namespace::General)
#define LOG_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::General,sev)

#define LOG_LAYER       DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Layer)
#define LOG_LAYER_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Layer,sev)

#define LOG_LINETYPE    DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Linetype)
#define LOG_LINETYPE_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Linetype,sev)

#define LOG_TEXTSTYLE   DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Textstyle)
#define LOG_TEXTSTYLE_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Textstyle,sev)

#define LOG_MATERIAL    DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Material)
#define LOG_MATERIAL_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Material,sev)

#define LOG_MODEL       DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Model)
#define LOG_MODEL_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Model,sev)

#define LOG_ENTITY      DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Entity)
#define LOG_ENTITY_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Entity,sev)

using namespace BentleyApi::Dgn;
using namespace BentleyApi::BeSQLite;
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/16
+===============+===============+===============+===============+===============+======*/
struct DwgImportLogging
    {
    enum class Namespace
        {
        General,
        Model,
        Layer,
        Linetype,
        Textstyle,
        Material,
        Entity,
        Dictionary,
        Performance,
        MaxLoggers
        };

    static NativeLogging::ILogger&  GetLogger (Namespace ns);
    static bool                     IsSeverityEnabled (Namespace ns, NativeLogging::SEVERITY);
    static void                     LogPerformance (StopWatch& stopWatch, Utf8CP description, ...);
    };  // DwgImportLogging

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgProgressMeter : public IDwgDbProgressMeter
    {
private:
    DgnProgressMeter*   m_meter;

public:
    DwgProgressMeter (DgnProgressMeter* newMeter) : m_meter(newMeter) { }

    virtual void        _Start (WStringCR displayString = WString()) override { /* do nothing */ }
    virtual void        _Stop () override { /* do nothing */ }
    virtual void        _Progress () override;
    virtual void        _SetLimit (int max) override { /* do nothing */ }
    };  // IDwgDbProgressMeter

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgImportHost : public IDwgDbHost
{
private:
    DwgImporter*            m_importer;
    DwgProgressMeter*       m_progressMeter;
    WString                 m_lastShxFontName;
    WString                 m_registryRootKey;

    bool                    FindFontFile (WStringR outPath, WCharCP fontName, AcadFileType hint);
    bool                    FindXrefFile (WStringR outPath, WCharCP fileName, DwgDbDatabaseP dwg = nullptr);

public:
    virtual DwgDbStatus     _FindFile (WStringR fullpathOut, WCharCP filenameIn, DwgDbDatabaseP dwg = nullptr, AcadFileType hint = AcadFileType::Default) override;
    virtual bool            _GetAlternateFontName (WStringR altFont) const override;
    virtual bool            _GetPassword (WCharCP dwgName, PasswordChoice choice, WCharP password, const size_t bufSize) override;
    virtual WCharCP         _GetRegistryProductRootKey (RootRegistry type) override;
    virtual LCID            _GetRegistryProductLCID () override;
    virtual WCharCP         _Product () const override;
    virtual void            _FatalError (WCharCP format, ...) override;
    virtual void            _Alert (WCharCP message) const override;
    virtual void            _Message (WCharCP message, int numChars) const override;
    virtual void            _DebugPrintf (WCharCP format, ...) const override;
    virtual bool            _IsValid () const override;

    void                    Initialize (DwgImporter& importer);
    void                    NewProgressMeter ();

    DWG_EXPORT static DwgImportHost& GetHost ();
};  // DwgImportHost

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct PolylineFactory
    {
private:
    DwgDbPolylineCP         m_polyline;
    size_t                  m_numPoints;
    DPoint3dArray           m_points;
    DPoint2dArray           m_widths;
    DwgDbDoubleArray        m_bulges;
    double                  m_constantWidth;
    bool                    m_hasBulges;
    bool                    m_hasConstantWidth;
    bool                    m_hasWidths;
    double                  m_elevation;
    double                  m_thickness;
    bool                    m_isClosed;
    bool                    m_hasAppliedWidths;
    bool                    m_hasEcs;
    Transform               m_ecs;
    CurveVector::BoundaryType   m_boundaryType;

    void WidenLineToSides(DSegment3dR rightSeg, DSegment3dR leftSeg, size_t fromIndex, bool useElevation) const;
    void WidenPointToSides(DPoint3dR rightPoint, DPoint3dR leftPoint, DPoint3dCR origin, DVec3dCR right2Left, double width) const;
    void IntersectLines(DPoint3dR jointPoint, DSegment3dCR line1, DSegment3dCR line2) const;
    void ConnectAndChainToPath(bvector<DPoint3d>& path, bvector<DSegment3d>const& right, bvector<DSegment3d>const& left) const;

public:
    PolylineFactory ();
    PolylineFactory (DwgDbPolylineCR polyline, bool allowClosed = true);
    PolylineFactory (size_t nPoints, DPoint3dCP points, bool closed);

    // create profile polyline curve
    CurveVectorPtr          CreateCurveVector (bool useElevation);
    // Apply constant width to the curve created via above method
    CurveVectorPtr          ApplyConstantWidthTo (CurveVectorPtr const& plineCurve);
    // apply thickness to the polyline curve created via above method
    GeometricPrimitivePtr   ApplyThicknessTo (CurveVectorPtr const& plineCurve);
    // Hash this polyline data and add it to output MD5:
    void                    HashAndAppendTo (BentleyApi::MD5& hashOut) const;
    // Transform polyline data
    void                    TransformData (TransformCR transform);
    // Set desired boundary type - default is BOUNDARY_TYPE_Outer if closed or BOUNDARY_TYPE_Open otherwise.
    void                    SetBoundaryType (CurveVector::BoundaryType type);
    // Called by CreateCurveVector for non-bulged polyline with variable widths
    CurveVectorPtr CreateShapeFromVariableWidths (bool useElevation);
    bool    HasAppliedWidths () const { return  m_hasAppliedWidths; }

    static bool     IsValidBulgeFactor (double bulge);
    };  // PolylineFactory

// ECClass requires unique property names
typedef bset<Utf8String>    ECPropertyNameSet;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          09/16
+===============+===============+===============+===============+===============+======*/
struct  AttributeFactory
    {
private:
    DwgImporter::ElementImportResults&  m_outputResults;
    DwgImporter::ElementImportInputs&   m_inputContext;
    DwgImporter&                        m_importer;
    DgnElementR                         m_hostElement;
    IECInstancePtr                      m_ecInstance;
    size_t                              m_propertyCount;
    size_t                              m_adhocCount;
    ECPropertyNameSet                   m_ecPropertyNames;

    bool            AddPropertyOrAdhocFromAttribute (DwgDbAttributeCR attrib);
    ECObjectsStatus AddConstantProperty (DwgDbAttributeDefinitionCR attrdef);
    BentleyStatus   CreateECInstance (DwgDbObjectIdCR blockId);
    BentleyStatus   ProcessVariableAttributes (DwgDbObjectIterator& attribIter);
    BentleyStatus   ProcessConstantAttributes (DwgDbObjectIdCR blockId, DwgDbObjectIdCR blockRefId, TransformCR toBlockRef);
    size_t          GetPropertyCount () { return m_propertyCount; }
    size_t          GetAdhocCount () { return m_adhocCount; }
    size_t          GetTotalCount () { return m_propertyCount + m_adhocCount; }
    
public:
    explicit AttributeFactory (DwgImporter& importer, DgnElementR hostElement, DwgImporter::ElementImportResults& reults, DwgImporter::ElementImportInputs& inputs);

    // create DgnElements, along with its ElementAspect's, from DWG attributes
    BentleyStatus   CreateElements (DwgDbBlockReferenceCR blockReference);
    }; // AttributeFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/16
+===============+===============+===============+===============+===============+======*/
struct  ViewportFactory
    {
    static Utf8CP GetSpatialViewNameInsert () { return " Viewport-"; }

private:
    // DWG common viewport data
    DPoint3d            m_center;
    DPoint3d            m_target;
    DVec3d              m_viewDirection;
    bool                m_hasCamera;
    bool                m_isFrontClipped;
    bool                m_isBackClipped;
    double              m_frontClipDistance;
    double              m_backClipDistance;
    double              m_height;
    double              m_width;
    double              m_lensLength;
    double              m_viewTwistAngle;
    bool                m_isGridOn;
    bool                m_isUcsIconOn;
    bool                m_isDefaultLightingOn;
    double              m_brightness;
    DwgCmColor          m_ambientLightColor;
    DwgDbObjectId       m_clipEntityId;
    DwgDbObjectId       m_backgroundId;
    DwgDbObjectId       m_visualStyleId;
    DwgDbObjectId       m_sunId;
    double              m_customScale;
    DwgDbObjectCP       m_inputViewport;
    // BIM members
    ColorDef            m_backgroundColor;
    bool                m_isPrivate;
    Transform           m_transform;
    CategorySelectorPtr m_categories;
    DwgImporter&        m_importer;
    DwgSourceAspects::ViewAspect::SourceType m_viewportType;

    void ComputeSpatialView (SpatialViewDefinitionR dgnView);
    void ComputeSheetView (ViewDefinitionR dgnView);
    void ComputeSpatialDisplayStyle (DisplayStyle3dR displayStyle);
    void ComputeSheetDisplayStyle (DisplayStyleR displayStyle);
    bool ComputeViewAttachment (Placement2dR placement);
    bool ComposeLayoutTransform (TransformR trans, DwgDbObjectIdCR blockId);
    void TransformDataToBim ();
    bool IsLayerDisplayed (DwgDbHandleCR layer, DwgDbObjectIdArrayCR vpfrozenLayers, DwgDbDatabaseR dwg) const;
    void UpdateModelspaceCategories (DgnCategoryIdSet& catIds, DwgDbObjectIdArrayCR frozenLayers, DwgDbDatabaseR dwg) const;
    void AddModelspaceCategories (Utf8StringCR viewName);
    // clip SpatialView attached to ViewAttachment - legacy clipping, may be removed.
    void ApplyViewportClipping (SpatialViewDefinitionR dgnView, double frontClip, double backClip);
    // clip Sheet::ViewAttachment directly.
    void ApplyViewportClipping (Sheet::ViewAttachmentR viewAttachment);
    bool ComputeClipperTransformation (TransformR toClipper, RotMatrixCR viewRotation);
    void ComputeEnvironment (DisplayStyle3dR displayStyle);
    DgnTextureId FindEnvironmentImageFile (BeFileNameCR filename) const;
    bool UpdateViewName (ViewDefinitionR view, Utf8StringCR proposedName);
    void UpdateViewAspect (ViewDefinitionR view, Utf8StringCR viewName, bool isNew);

public:
    // constructor for a modelspace viewport
    ViewportFactory (DwgImporter& importer, DwgDbViewportTableRecordCR viewportRecord);
    // constructor for the overall layout viewport and a paperspace viewport entity
    ViewportFactory (DwgImporter& importer, DwgDbViewportCR viewportEntity, DwgDbLayoutCP layout = nullptr);

    // create a model view based on input model type: spatial, drawing or sheet
    DgnViewId       CreateModelView (DgnModelCR targetModel, Utf8StringCR proposedName);
    // create a camera or orthoganal view for a modelspace viewport or a viewport entity
    DgnViewId       CreateSpatialView (DgnModelId modelId, Utf8StringCR proposedName);
    // create a sheet view for the overall layout viewport
    DgnViewId       CreateSheetView (DgnModelId sheetId, Utf8StringCR proposedName);
    // create a drawing view for a modelspace viewport when an app wants 2d model
    DgnViewId       CreateDrawingView (DgnModelId modelId, Utf8StringCR proposedName);
    // create a view attachment element in a sheet model for a viewport entity in a paperspace
    DgnElementPtr   CreateViewAttachment (DgnModelCR sheetModel, DgnViewId viewId);
    // Corresponding Update methods
    BentleyStatus   UpdateModelView (DgnModelCR targetModel, DgnViewId viewId, Utf8StringCR proposedName);
    BentleyStatus   UpdateSpatialView (DgnViewId viewId, Utf8StringCR proposedName);
    BentleyStatus   UpdateSheetView (DgnViewId viewId, Utf8StringCR proposedName);
    BentleyStatus   UpdateDrawingView (DgnViewId viewId, Utf8StringCR proposedName);
    DgnElementPtr   UpdateViewAttachment (DgnElementId attachId, DgnViewId viewId);
    void            UpdateModelspaceCategories (DgnCategoryIdSet& categoryIds) const;

    bool    ValidateViewName (Utf8StringR viewNameInOut);
    void    SetBackgroundColor (ColorDefCR color) { m_backgroundColor = color; }
    void    SetViewSourcePrivate (bool viewSourcePrivate) { m_isPrivate = viewSourcePrivate; }
    };  // ViewportFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/19
+===============+===============+===============+===============+===============+======*/
struct XRefLoader
    {
private:
    DwgImporter&    m_importer;
    ECSchemaPtr     m_attrdefSchema;
    bool IsXrefAlreadyLoaded (DwgDbBlockTableRecordCR xrefblock);
    void ReportMissingFile (BeFileNameCR filename, DwgStringCR blockName) const;

public:
    XRefLoader (DwgImporter& importer) : m_importer(importer) {}
    BentleyStatus LoadXrefsInMasterFile ();
    BentleyStatus CacheUnresolvedXrefs ();
    ECSchemaPtr GetAttrdefSchema () { return m_attrdefSchema; }
    }; // XRefLoader

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/16
+===============+===============+===============+===============+===============+======*/
struct    XRefLayerResolver
{
private:
    DwgImporter&        m_importer;
    DwgDbDatabaseP      m_xrefDwg;

public:
    XRefLayerResolver (DwgImporter& importer, DwgDbDatabaseP xref) : m_importer(importer), m_xrefDwg(xref) {}
    DwgDbObjectId   ResolveEntityLayer (DwgDbObjectId layerId) const;
    bool    SearchXrefLayerByName (DwgDbObjectIdR layerId, Utf8StringCR masterLayerName);
    bool    SearchMasterLayerFromXrefLayer (DwgDbObjectIdR masterLayerId, DwgDbObjectIdCR xrefLayerId) const;
};  // XRefLayerResolver

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/18
+===============+===============+===============+===============+===============+======*/
struct LayoutXrefFactory
    {
    static Utf8CP GetSpatialViewNamePrefix () { return "XRef "; }

private:
    DwgImporter&            m_importer;
    DwgDbBlockReferenceCR   m_xrefInsert;
    DwgDbObjectId           m_paperspaceId;
    DgnModelP               m_xrefModel;
    DwgDbDatabaseP          m_xrefDwg;
    DefinitionModelP        m_jobDefinitionModel;
    Utf8String              m_viewName;
    Transform               m_xrefTransform;
    DRange3d                m_xrefRange;
    SpatialViewDefinitionPtr m_spatialView;
    Sheet::ViewAttachmentPtr m_viewAttachment;

    void    ValidateViewName ();
    bool    UpdateViewName ();
    size_t  GetViewportFrozenLayers (DwgDbObjectIdArrayR frozenLayers) const;
    void    ComputeSpatialDisplayStyle (DisplayStyle3dR displayStyle) const;
    void    UpdateSpatialCategories (DgnCategoryIdSet& categoryIdSet) const;
    Utf8String      GetXrefLayerPrefix () const;
    BentleyStatus   ComputeSpatialView ();
    BentleyStatus   ComputeViewAttachment (Placement2dR placement);
    BentleyStatus   UpdateSpatialView (bool updateBim);
    BentleyStatus   CreateSpatialView ();
    BentleyStatus   UpdateViewAttachment ();
    BentleyStatus   CreateViewAttachment (DgnModelR sheetModel);

public:
    LayoutXrefFactory (DwgImporter& importer, DwgDbBlockReferenceCR xrefInsert);
    void SetXrefDatabase (DwgDbDatabaseP dwg) { m_xrefDwg = dwg; }
    void SetPaperspace (DwgDbObjectIdCR id) { m_paperspaceId = id; }
    void SetXrefModel (DgnModelP model) { m_xrefModel = model; }
    void SetXrefTransform (TransformCR trans) { m_xrefTransform = trans; }
    void SetXrefRange (DRange3dCR range) { m_xrefRange = range; }
    BentleyStatus ConvertToBim (DwgImporter::ElementImportResults&, DwgImporter::ElementImportInputs&);
    };  // LayoutXrefFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/17
+===============+===============+===============+===============+===============+======*/
struct LayoutFactory
    {
private:
    bool                m_isValid;
    DwgImporter&        m_importer;
    DwgDbLayoutPtr      m_layout;

public:
    LayoutFactory (DwgImporter& importer, DwgDbObjectId layoutId);
    bool            IsValid () const { return m_isValid; }
    double          GetUserScale () const;
    BentleyStatus   CalculateSheetSize (DPoint2dR sheetSize) const;
    BentleyStatus   AlignSheetToPaperOrigin (TransformR transform) const;
    static DwgDbObjectId FindOverallViewport (DwgDbBlockTableRecordCR block);
    };  // LayoutFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/18
+===============+===============+===============+===============+===============+======*/
struct  GroupFactory
    {
private:
    DwgImporter&        m_importer;
    DwgDbGroupCR        m_dwgGroup;
    DwgDbObjectIdArray  m_dwgMemberIds;

    DgnElementIdSet FindAllElements (DwgDbObjectIdCR objectId) const;
    GenericGroupPtr CreateAndInsert () const;
    void SetGroupName (GenericGroupR genericGroup) const;

public:
    // the constructor
    GroupFactory (DwgImporter& importer, DwgDbGroupCR dwgGroup);
    DgnElementPtr   Create () const;
    BentleyStatus   Update (GenericGroupR dgnGroup) const;
    };  // GroupFactory

/*=================================================================================**//**
* ElementFactory takes geometries collected from GeometryFactory as input, and creates
* either shared parts or individual elements.  If GeometryFactory sees no trouble creating
* an assembly, it will create shared parts.  If it cannot create parts, individual elements
* will be created instead.
*
* @see GeometryFactory
* @bsiclass                                                     Don.Fu          05/18
+===============+===============+===============+===============+===============+======*/
struct ElementFactory
    {
private:
    DwgImporter::ElementImportResults&  m_results;
    DwgImporter::ElementImportInputs&   m_inputs;
    DwgImporter::ElementCreateParams&   m_createParams;
    DwgImporter::T_BlockGeometryMap const*  m_geometryMap;
    DgnElement::CreateParams            m_elementParams;
    ElementHandlerP     m_elementHandler;
    DgnCode             m_elementCode;
    Utf8String          m_elementLabel;
    DefinitionModelPtr  m_partModel;
    GeometryBuilderPtr  m_geometryBuilder;
    Transform           m_modelTransform;
    Transform           m_baseTransform;
    Transform           m_invBaseTransform;
    double              m_basePartScale;
    bool                m_is3d;
    bool                m_canCreateSharedParts;
    bool                m_hasBaseTransform;
    DwgDbObjectId       m_sourceBlockId;
    DwgDbObjectId       m_sourceLayerId;
    DwgImporter&        m_importer;

    void            SetDefaultCreation ();
    Utf8String      BuildPartCodeValue (DwgImporter::GeometryEntry const& geomEntry, size_t partNo);
    void            TransformGeometry (GeometricPrimitiveR geometry, TransformR geomTrans, double* partScale = nullptr) const;
    bool            Validate2dTransform (TransformR transform) const;
    void            ApplyPartScale (TransformR transform, double scale, bool invert) const;
    bool            NeedsSeparateElement (DgnCategoryId id) const;
    DgnGeometryPartId CreateGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, Utf8StringCR partTag, DwgImporter::GeometryEntry const& geomEntry);
    BentleyStatus   GetGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, DgnGeometryPartId partId, DwgImporter::GeometryEntry const& geomEntry);
    BentleyStatus   GetOrCreateGeometryPart (DwgImporter::SharedPartEntry& part, DwgImporter::GeometryEntry const& geomEntry, size_t partNo);
    BentleyStatus   CreateEmptyElement ();
    BentleyStatus   CreateIndividualElements ();
    BentleyStatus   CreateSharedParts ();

public:
    // Constructor
    ElementFactory (DwgImporter::ElementImportResults& results, DwgImporter::ElementImportInputs& inputs, DwgImporter::ElementCreateParams& params, DwgImporter& importer);

    bool    CanCreateSharedParts () const { return  m_canCreateSharedParts; }
    void    SetCreateSharedParts (bool desired) { m_canCreateSharedParts = desired; }
    // Set base transform from a block tranform
    void    SetBaseTransform (TransformCR blockTrans);
    // Set a prebuilt GeometryBuilder from which elements will be created
    void    SetGeometryBuilder (GeometryBuilderP builder) { BeAssert(!m_geometryBuilder.IsValid()); m_geometryBuilder = builder; }
    // Create db elements from the GeometryBuilder and flush the builder at the end.
    BentleyStatus   CreateElement ();
    // Create shared parts from a part cache per block
    BentleyStatus   CreatePartElements (DwgImporter::T_SharedPartList const& parts);
    // Create db elements from a block-geometry map
    BentleyStatus   CreateElements (DwgImporter::T_BlockGeometryMap const* geometryMap);
    };  // ElementFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/19
+===============+===============+===============+===============+===============+======*/
struct RepositoryLinkFactory
    {
private:
    DgnDbR      m_dgndb;
    DwgImporterR    m_importer;
    iModelBridge::Params const& m_bridgeparams;

    Utf8String  ComputeURN (BeFileNameCR dwgFilename);

public:
    RepositoryLinkFactory (DgnDbR db, DwgImporterR imp) : m_dgndb(db), m_importer(imp), m_bridgeparams(imp.GetOptions()) {}

    DgnElementId    CreateOrUpdate (DwgDbDatabaseR dwg);
    BentleyStatus   DeleteFromDb (BeFileNameCR dwgFileName);
    };  // RepositoryFactory

END_DWG_NAMESPACE
