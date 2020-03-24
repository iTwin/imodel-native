/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#include <BeHttp/HttpClient.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/LineStyle.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <BRepCore/PSolidUtil.h>

#include <BeRapidJson/BeRapidJson.h>
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
    bool                m_syncSpatialView;
    bool                m_isViewRotationValid;
    RotMatrix           m_spatialViewRotation;

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
    // attach a 3D view of a GraphicalModel3d to a sheet model
    DgnElementId Attach3dViewToSheetModel (DgnModelCR sheetModel, DgnViewId view3dId);
    RotMatrixCR GetSpatialViewRotationMatrix ();

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
    // create a sheet model, a sheet view attached by a 3D view of a GraphicalModel3d created from the modelspace
    DgnViewId       CreateOrUpdate2dViewFor3dModel (DgnModelCR model3d, DgnViewId view3dId);
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
    ECSchemaPtr     m_dwgAppDataSchema;
    bool IsXrefAlreadyLoaded (DwgDbBlockTableRecordCR xrefblock);
    void ReportMissingFile (BeFileNameCR filename, DwgStringCR blockName) const;

public:
    XRefLoader (DwgImporter& importer) : m_importer(importer) {}
    BentleyStatus LoadXrefsInMasterFile ();
    BentleyStatus CacheUnresolvedXrefs ();
    ECSchemaPtr GetAttrdefSchema () { return m_attrdefSchema; }
    ECSchemaPtr GetDwgAppDataSchema () { return m_dwgAppDataSchema; }
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
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DrawParameters : public IDwgDrawParameters
{
friend struct GeometryFactory;

// Effective symbology used for child's ByBlock symbology
struct EffectiveByBlock
    {
    DwgCmEntityColor    m_color;
    DwgDbObjectId       m_linetypeId;
    DwgDbObjectId       m_materialId;
    DwgDbLineWeight     m_weight;
    DwgDbObjectId       m_layerId;
public:
    void Set (DwgCmEntityColorCR color, DwgDbObjectIdCR ltype, DwgDbObjectIdCR material, DwgDbLineWeight weight, DwgDbObjectIdCR layer);
    void CopyFrom (EffectiveByBlock const& other);
    };  // EffectiveByBlock

private:
    DwgCmEntityColor    m_color;
    DwgDbObjectId       m_layerId;
    DwgDbObjectId       m_linetypeId;
    std::ptrdiff_t      m_markerId;
    DwgGiFillType       m_filltype;
    DwgGiFillPtr        m_fill;
    DwgDbLineWeight     m_weight;
    uint32_t            m_mappedDgnWeight;  // only a performance need
    double              m_linetypeScale;
    double              m_thickness;
    DwgTransparency     m_transparency;
    DwgDbObjectId       m_materialId;
    DwgImporter&        m_dwgImporter;
    DwgDbEntityCP       m_sourceEntity;
    EffectiveByBlock    m_effectiveByBlock;
    DwgDbDatabasePtr    m_dwgdb;
    bool                m_isLayerByBlock;
    bool                m_isParentLayerFrozen;
    bool                m_isDisplayed;
    DwgDbObjectId       m_layer0Id;
    bool                m_canOverrideEntityMaterial;

protected:
    void CopyFrom (DrawParameters const& params);
    void Initialize (DwgDbEntityCR ent, DrawParameters const* parentParams = nullptr, DwgDbEntityCP templateEntity = nullptr);
    double InitThicknessFromEntity (DwgDbEntityCR ent);

    // implementation of IDwgDrawParameters - methods called from DwgDb
    virtual DwgCmEntityColorCR  _GetColor () const override { return m_color; }
    virtual DwgDbObjectIdCR     _GetLayer () const override { return m_layerId; }
    virtual DwgDbObjectIdCR     _GetLineType () const override { return m_linetypeId; }
    virtual DwgGiFillType       _GetFillType () const override { return m_filltype; }
    virtual DwgGiFillCP         _GetFill () const override { return m_fill.get(); }
    virtual DwgDbLineWeight     _GetLineWeight () const override { return m_weight; }
    virtual double              _GetLineTypeScale () const override { return m_linetypeScale; }
    virtual double              _GetThickness () const override { return m_thickness; }
    virtual DwgTransparencyCR   _GetTransparency () const override { return m_transparency; }
    virtual DwgDbObjectIdCR     _GetMaterial () const override { return m_materialId; }
    DWG_EXPORT virtual void _SetColor (DwgCmEntityColorCR color) override;
    DWG_EXPORT virtual void _SetLayer (DwgDbObjectIdCR layerId) override;
    DWG_EXPORT virtual void _SetLineType (DwgDbObjectIdCR linetypeId) override;
    DWG_EXPORT virtual void _SetSelectionMarker (std::ptrdiff_t markerId) override;
    DWG_EXPORT virtual void _SetFillType (DwgGiFillType filltype) override;
    DWG_EXPORT virtual void _SetFill (DwgGiFillCP fill) override;
    DWG_EXPORT virtual void _SetLineWeight (DwgDbLineWeight weight) override;
    DWG_EXPORT virtual void _SetLineTypeScale (double scale) override;
    DWG_EXPORT virtual void _SetThickness (double thickness) override;
    DWG_EXPORT virtual void _SetTransparency (DwgTransparency transparency) override;
    DWG_EXPORT virtual void _SetMaterial (DwgDbObjectIdCR materialId) override;

public:
    DrawParameters (DrawParameters const& params);
    DrawParameters (DwgDbEntityCR ent, DwgImporter& importer, DwgDbEntityCP parent = nullptr, DwgDbEntityCP templateEntity = nullptr);

    // methods called from DwgImporter
    DwgDbEntityCP   GetSourceEntity () const { return m_sourceEntity; }
    DwgDbDatabaseP  GetDatabase () { return m_dwgdb.get(); }
    DwgImporter&    GetDwgImporter () { return  m_dwgImporter; }
    bool            IsDisplayed () { return m_isDisplayed; }
    void            SetLinetypeContinuous () { m_linetypeId = m_dwgdb->GetLinetypeContinuousId(); }
    void            SetGradientColorFromHatch (DwgDbHatchCP hatch);
    void            ResolveDisplayStatus (DrawParameters const* parentParams);
    void            ResolveRootEffectiveByBlockSymbology ();
    void            ResolveEffectiveByBlockSymbology (DrawParameters const& newParent);
    bool            IsColorByBlock () const;
    bool            IsLinetypeByBlock () const;
    bool            IsMaterialByBlock () const;
    bool            IsWeightByBlock () const;
    Render::FillDisplay GetFillDisplay () const;
    void            GetDisplayParams (Render::GeometryParams& params);
    ColorDef        GetDgnColor ();
    bool            GetColorFromLayer (DwgCmEntityColorR colorOut) const;
    uint32_t        GetDgnWeight ();
    DwgDbLineWeight GetWeightFromLayer () const;
    DgnStyleId      GetDgnLineStyle (bool& isContinuous, double& effectiveScale) const;
    bool            GetLinetypeFromLayer (DwgDbObjectIdR ltypeOut) const;
    double          GetEffectiveLinetypeScale () const;
    RenderMaterialId GetDgnMaterial () const;
    bool            GetMaterialFromLayer (DwgDbObjectIdR materialOut) const;
    DwgDbObjectId   GetEffectiveLayerId () const;
    bool            CanUseByLayer (bool isByLayer);
};  // DrawParameters
    
/*=================================================================================**//**
* GeometryFactory collects geometries from the entity draw callback of a working toolkit, 
* and caches them into a local per block-geometry map.  A separate block stack helps tracking 
* & setting the effective block that is currently being drawn by the toolkit.  The nested 
* The flat block-geometry map does not capture the nestedness of blocks, but is more 
* efficient for building a geometry map for shared geometry parts used in next step.  
* When a same nested block is drawn more than once, the output map will have "duplicated" 
* geometries per block.  It is for this reason, only the outermost block, i.e. all geometries 
* collected for the root block from which this class is instantiated can be used for importer's 
* block-geometry map reuse purpose.
*
* @see ElementFactory
*
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct GeometryFactory : public IDwgDrawGeometry
{
// Block information saved off in a stack to track currently drawn block.
struct BlockInfo
    {
private:
    DwgDbObjectId           m_blockId;
    Utf8String              m_blockName;
    DgnElementId            m_fileId;
    size_t                  m_geometryCount;
public:
    BlockInfo (DwgDbObjectIdCR id, DwgStringCR name, DgnElementId file) : m_blockId(id), m_fileId(file)
        {
        m_blockName.Assign (name.c_str());
        m_geometryCount = 0;
        }
    DwgDbObjectIdCR GetBlockId () const { return m_blockId; }
    Utf8StringCR    GetBlockName () const { return m_blockName; }
    DgnElementId    GetFileId () const { return m_fileId; }
    size_t  GetAndIncrementGeometryCount () { return m_geometryCount++; }
    };  // BlockInfo

private:
    // members used by the toolkit
    bvector<Transform>                  m_transformStack;
    Transform                           m_baseTransform;
    Transform                           m_currentTransform;
    DrawParameters&                     m_drawParams;
    // members used locally
    DwgImporter::ElementCreateParams&   m_createParams;
    DwgImporter::GeometryOptions*       m_geometryOptions;
    DwgImporter::T_BlockGeometryMap     m_outputGeometryMap;
    DwgDbEntityCP                       m_entity;
    BentleyStatus                       m_status;
    Transform                           m_worldToElement;
    DwgDbSpatialFilterCP                m_spatialFilter;
    bvector<BlockInfo>                  m_blockStack;
    bvector<int64_t>                    m_parasolidBodies;
    bool                                m_isTargetModel2d;

    void PushDrawingBlock (DwgDbBlockTableRecordCR block);
    bool IsDrawingBlock () { return m_blockStack.size() > 1; }
    void PopDrawingBlock () { m_blockStack.pop_back(); }
    BlockInfo& GetCurrentBlock () { return m_blockStack.back(); }
    bool ApplyThickness (ICurvePrimitivePtr const& curvePrimitive, DVec3dCR normal, bool closed = false);
    bool ApplyThickness (GeometricPrimitivePtr const& geomPrimitive, DVec3dCR normal, bool closed = false);
    void ApplyColorOverride (DwgGiFaceDataCP faceData, DwgGiEdgeDataCP edgeData);
    BentleyStatus SetText (Dgn::TextStringPtr& dgnText, DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool* mirrored = nullptr);
    bool DropText (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle);
    bool PreprocessBlockChildGeometry (DwgDbEntityP entity, DrawParameters& savedParams);
    bool SkipBlockChildGeometry (DwgDbEntityCP entity);
    BentleyStatus CreateBlockChildGeometry (DwgDbEntityCP entity);
    BentleyStatus DrawBlockAttributes (DwgDbBlockReferenceCR blockRef);
    void ComputeCategory (DgnCategoryId& categoryId, DgnSubCategoryId& subcategoryId);
    void AppendGeometry (ICurvePrimitiveCR primitive);
    void AppendGeometry (CurveVectorCR curveVector);
    void AppendGeometry (GeometricPrimitiveR geometry);

protected:
    // implementation of IDwgDrawGeometry
    virtual void _Circle (DPoint3dCR center, double radius, DVec3dCR normal) override;
    virtual void _Circle (DPoint3dCR point1, DPoint3dCR point2, DPoint3dCR point3) override;
    virtual void _CircularArc (DPoint3dCR center, double rad, DVec3dCR normal, DVec3dCR start, double swept, DwgGiArcType arcType = DwgGiArcType::Simple) override;
    virtual void _CircularArc (DPoint3dCR start, DPoint3dCR point, DPoint3dCR end, DwgGiArcType arcType = DwgGiArcType::Simple) override;
    virtual void _Curve (MSBsplineCurveCR curve) override;
    virtual void _Ellipse (DEllipse3dCR ellipse, DwgGiArcType arcType = DwgGiArcType::Simple) override;
    virtual void _Edge (CurveVectorCR edges) override;
    virtual void _Polyline (size_t nPoints, DPoint3dCP points, DVec3dCP normal = nullptr, int64_t subentMarker = -1) override;
    virtual void _Pline (DwgDbPolylineCR pline, size_t fromIndex = 0, size_t numSegs = 0) override;
    virtual void _Polygon (size_t nPoints, DPoint3dCP points) override;
    virtual void _Mesh (size_t nRows, size_t nColumns, DPoint3dCP points, DwgGiEdgeDataCP edges = nullptr, DwgGiFaceDataCP faces = nullptr, DwgGiVertexDataCP verts = nullptr, bool autonorm = false) override;
    virtual void _Shell (size_t nPoints, DPoint3dCP points, size_t nFaceList, int32_t const* faces, DwgGiEdgeDataCP edgeData = nullptr, DwgGiFaceDataCP faceData = nullptr, DwgGiVertexDataCP vertData = nullptr, DwgResBufCP xData = nullptr, bool autonorm = false) override;
    virtual void _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, double h, double w, double oblique, DwgStringCR string) override;
    virtual void _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle) override;
    virtual void _Xline (DPoint3dCR point1, DPoint3dCR point2) override;
    virtual void _Ray (DPoint3dCR origin, DPoint3dCR point) override;
    virtual void _Draw (DwgGiDrawableR drawable) override;
    virtual void _Image (DwgGiImageBGRA32CR image, DPoint3dCR pos, DVec3dCR u, DVec3dCR v, DwgGiTransparencyMode mode = DwgGiTransparencyMode::Alpha8Bit) override;
    virtual void _RowOfDots (size_t count, DPoint3dCR start, DVec3dCR step) override;
    virtual void _WorldLine (DPoint3d points[2]) override;
    virtual void _PushModelTransform (TransformCR newTransform) override;
    virtual void _PopModelTransform () override;
    virtual void _GetModelTransform (TransformR outTransform) override;
    virtual void _PushClipBoundary (DwgGiClipBoundaryCP boundary) override;
    virtual void _PopClipBoundary () override;

public:
    // the constructor
    GeometryFactory (DwgImporter::ElementCreateParams& createParams, DrawParameters& drawParams, DwgImporter::GeometryOptions& opts, DwgDbEntityCP ent);
    // the destructor
    ~GeometryFactory ();

    DwgImporter::T_BlockGeometryMap& GetOutputGeometryMap () { return m_outputGeometryMap; }
    BentleyStatus   GetStatus () { return m_status; }
    void SetSpatialFilter (DwgDbSpatialFilterCP filter) { m_spatialFilter = filter; }
};  // GeometryFactory

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
    size_t              m_elementCount;

    void            SetDefaultCreation ();
    Utf8String      BuildPartCodeValue (DwgImporter::GeometryEntry const& geomEntry);
    void            TransformGeometry (GeometricPrimitiveR geometry, TransformR geomTrans, double* partScale = nullptr) const;
    bool            Validate2dTransform (TransformR transform) const;
    void            ApplyPartScale (TransformR transform, double scale, bool invert) const;
    bool            NeedsSeparateElement (DgnCategoryId id) const;
    DgnGeometryPartId CreateGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, Utf8StringCR partTag, DwgImporter::GeometryEntry const& geomEntry);
    BentleyStatus   GetGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, DgnGeometryPartId partId, DwgImporter::GeometryEntry const& geomEntry);
    BentleyStatus   GetOrCreateGeometryPart (DwgImporter::SharedPartEntry& part, DwgImporter::GeometryEntry const& geomEntry);
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

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
struct AecPsetSchemaFactory
{
private:
    DgnDbR  m_dgndb;
    ECSchemaPtr& m_targetSchema;
    DwgImporterR m_importer;
    bset<Utf8String> m_userPropertyNames;
    bset<Utf8String> m_foundUserClassNames;

    ECObjectsStatus CreateTargetSchema ();
    ECPropertyP CreateECProperty (ECEntityClassP aecpsetClass, rapidjson::Value::ConstMemberIterator const& iter);
    void TrackPropertiesForElementMapping (Utf8StringCR propName, rapidjson::Value::ConstMemberIterator const& iter);
    void PrepareForElementMapping ();
    BentleyStatus ImportFromDictionaries (DwgDbDictionaryIteratorR iter);

public:
    AecPsetSchemaFactory (ECSchemaPtr& s, DwgImporterR i) : m_targetSchema(s), m_importer(i), m_dgndb(i.GetDgnDb()) {}
    BentleyStatus ImportFromDwg (DwgDbDatabaseR dwg);
    BentleyStatus CreateUserElementClasses ();
};  // AecPsetSchemaFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/20
+===============+===============+===============+===============+===============+======*/
struct XDataFactory
{
private:
    ECSchemaCP      m_sourceSchema;
    DwgImporterR    m_importer;
    IECInstancePtr  m_ecInstance;

    ECObjectsStatus AddProperty (Utf8StringCR name, T_Utf8StringVectorCR strings);
    BentleyStatus   CreateECInstance (Utf8StringCR ecClassName);
    BentleyStatus   ExtractBreadcrumbsFromBlock (T_Utf8StringVectorR strings, DwgDbBlockReferenceCR insert);
    ECObjectsStatus InitializeTargetSchema (ECSchemaPtr& targetSchema);

public:
    // constructor for schema creation
    XDataFactory (DwgImporterR importer);
    BentleyStatus   CreateBreadcrumbs (DgnElementR targetElement, ECSchemaCP sourceSchema, DwgDbBlockReferenceCR insert);
    ECObjectsStatus CreateDwgAppDataSchema (ECSchemaPtr& targetSchema, DwgDbBlockTableRecordCR regapp);
};  // XDataFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
struct ExtendedImportInputs : DwgImporter::ElementImportInputs
{
    DEFINE_T_SUPER (DwgImporter::ElementImportInputs)
public:
    rapidjson::Document m_aecPropertySets;

public:
    ExtendedImportInputs(DgnModelR m, DwgDbEntityP e, ElementImportInputs const& o) : T_Super(m, e, o), m_aecPropertySets(rapidjson::kObjectType) {}
    rapidjson::Document& GetAecPropertySetsR () { return m_aecPropertySets; }
};  // ExtendedElementInputs

END_DWG_NAMESPACE
