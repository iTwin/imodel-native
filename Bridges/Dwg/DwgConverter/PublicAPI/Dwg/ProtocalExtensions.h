/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <Dwg/DwgDb/DwgRxObjects.h>
#include <Raster/RasterApi.h>

USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

#ifdef DWGTOOLKIT_OpenDwg

#define DWG_PROTOCOLEXT_DECLARE_MEMBERS(_className_)                                    \
    static void            RxInit ();                                                   \
    static void            RxUnInit ();                                                 \
    static OdRxObjectPtr   CreateObject();

#define DWG_PROTOCOLEXT_DEFINE_MEMBERS(_className_)                                     \
    void            _className_::RxInit ()                                              \
        {                                                                               \
        try { _className_::rxInit(); }                                                  \
        catch (OdError e) { LOG.errorv("Exception thrown initializing protocol extension: %ls", e.description().c_str()); } \
        }                                                                               \
    void            _className_::RxUnInit () { _className_::rxUninit(); }               \
    OdRxObjectPtr   _className_::CreateObject() { return _className_::createObject(); } \
    DWGRX_CONS_DEFINE_MEMBERS(##_className_##,DwgProtocolExtension)

#elif DWGTOOLKIT_RealDwg

#define DWG_PROTOCOLEXT_DECLARE_MEMBERS(_className_)                                    \
    static void            RxInit () { /*Nop*/;}                                        \
    static void            RxUnInit () { /*Nop*/;}                                      \
    static DwgRxObjectP    CreateObject();

#define DWG_PROTOCOLEXT_DEFINE_MEMBERS(_className_)                                     \
    DwgRxObjectP        _className_::CreateObject() { return new _className_(); }       \
    DWG_TypeP(RxClass)  _className_::isA() const { return T_Super::isA(); }

#endif  // DWGTOOLKIT_Open/RealDwg


typedef DwgImporter::ElementImportResults const&    ElementResultsCR;
typedef DwgImporter::ElementImportResults&          ElementResultsR;
typedef DwgImporter::ElementImportInputs const&     ElementInputsCR;
typedef DwgImporter::ElementImportInputs&           ElementInputsR;


/*=================================================================================**//**
//! @brief A data context passed in an object protocal extension that converts a DWG entity 
//! in the modelspace or a paperspace to DgnDb.  It contains the input context for a DWG
//! entity and the output context for DgnDb elements.
+===============+===============+===============+===============+===============+======*/
struct ProtocolExtensionContext
    {
private:
    ElementInputsR          m_elementInputs;
    ElementResultsR         m_elementResults;
    DgnModelP               m_resultantModel;

public:
    explicit ProtocolExtensionContext (ElementInputsR inputs, ElementResultsR results) :
        m_elementInputs(inputs),
        m_elementResults(results)
        {
        // output model
        m_resultantModel = nullptr;
        }

    DWG_EXPORT DgnModelR          GetModel () { return  m_elementInputs.GetTargetModelR(); }
    DWG_EXPORT DwgDbEntityCR      GetEntity () const { return m_elementInputs.GetEntity(); }
    DWG_EXPORT DwgDbEntityPtr&    GetEntityPtrR () { return m_elementInputs.GetEntityPtrR(); }
    DWG_EXPORT TransformCR        GetTransform () const { return m_elementInputs.GetTransform(); }
    DWG_EXPORT void               SetTransform (TransformCR toDgn) { m_elementInputs.SetTransform(toDgn); }
    DWG_EXPORT DgnClassId         GetDgnClassId () const { return m_elementInputs.GetClassId(); }
    DWG_EXPORT void               SetDgnClassId (DgnClassId id) { m_elementInputs.SetClassId(id); }
    DWG_EXPORT ElementResultsCR   GetElementResults () const { return m_elementResults; }
    DWG_EXPORT ElementResultsR    GetElementResultsR () { return m_elementResults; }
    DWG_EXPORT void               SetElementResultsR (ElementResultsR results) { m_elementResults = results; }
    DWG_EXPORT ElementInputsCR    GetElementInputs () const { return m_elementInputs; }
    DWG_EXPORT ElementInputsR     GetElementInputsR () { return m_elementInputs; }
    DWG_EXPORT DgnModelP          GetResultantModel () const { return m_resultantModel; }
    DWG_EXPORT void               SetResultantModel (DgnModelP outModel) { m_resultantModel = outModel; }
    };  // ProtocolExtensionContext


/*=================================================================================**//**
//! An object protocol exntension to convert its data to BIM
+===============+===============+===============+===============+===============+======*/
class DwgProtocolExtension : public DwgRxObject
    {
public:
    DEFINE_T_SUPER (DwgRxObject)
    DWGRX_DECLARE_MEMBERS (DwgProtocolExtension)

    DWG_EXPORT static DWG_TypeP(RxClass)      Desc ();
    DWG_EXPORT static DwgProtocolExtension*   Cast (DWG_TypeCP(RxObject) obj);
    DWG_EXPORT static void                    RxInit ();
    DWG_EXPORT static void                    RxUnInit ();

    //! Must implement this method to either create a new or update an existing element from the input entity.
    //! This method is called only when an entity is in the modelspace or a paperspace.
    DWG_EXPORT virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) = 0;
    //! Optional method to create a geometry from an entity in a block.
    //! This method is called only when an entity is in a block definition.
    DWG_EXPORT virtual GeometricPrimitivePtr _ConvertToGeometry (DwgDbEntityCP entity, bool is2D, DwgImporterR importer, IDwgDrawParametersP params = nullptr) { return nullptr; }
    };  // DwgProtocolExtension

/*=================================================================================**//**
//! Raster object protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgRasterImageExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgRasterImageExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgRasterImageExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable ProtocolExtensionContext*   m_toBimContext;
    mutable DwgImporterP                m_importer;
    mutable DwgDbRasterImageCP          m_dwgRaster;

    BentleyStatus   CreateRasterModel (BeFileNameCR rasterFilename, BeFileNameCR activePath);
    BentleyStatus   UpdateRasterModel (ResolvedModelMapping& modelMap, BeFileNameCR rasterFilename, BeFileNameCR activePath);
    bool        GetExistingModel (ResolvedModelMapping& modelMap);
    void        GetRasterMatrix (DMatrix4dR matrixOut);
    bool        ClipRasterModel (Raster::RasterFileModel& model);
    void        AddModelToViews (DgnModelId modelId);
    void        UpdateViews (DgnModelId modelId, bool isOn);
    bool        CopyRasterToDgnDbFolder (BeFileNameCR rasterFile, BeFileNameCR dbFile, BeFileNameCR altPath);
    //! check and replace an URL path with its local cache file name (download & cache as necessary).
    bool        GetUrlCacheFile (DwgStringR checkPath);
    };  // DwgRasterImageExt

/*=================================================================================**//**
//! Point cloud object protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgPointCloudExExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgPointCloudExExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgPointCloudExExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    ColorDef    GetDgnColor (DwgDbEntityCR entity) const;
    };  // DwgPointCloudExExt

/*=================================================================================**//**
//! Paperspace viewport entity protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgViewportExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgViewportExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgViewportExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    BentleyStatus   UpdateBim (ProtocolExtensionContext& context, DwgImporterR importer, DgnModelCR rootModel, DgnModelCR sheetModel, Utf8StringCR viewName);
    };  // DwgViewportExt

/*=================================================================================**//**
//! Light object protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgLightExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgLightExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgLightExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable ProtocolExtensionContext*   m_toBimContext;
    mutable DwgImporterP                m_importer;
    mutable DwgDbLightP                 m_dwgLight;

    BentleyStatus  CreateOrUpdateLightGlyph (ElementResultsR results, ElementInputsR inputs) const;
    BentleyStatus  ConvertLightParameters (Lighting::ParametersR lightParams) const;
    void    CreateDistantLightGlyph (GeometryBuilderR builder, double lightSize) const;
    void    CreateSpotLightGlyph (GeometryBuilderR builder, double lightSize) const;
    bool    IsLightGlyphDisplayed () const;
    double  CalculateLightGlyphSize () const;
    DVec3d  CalculateLightVector (TransformR lightTransform) const;
    double  CalculateUnitArea (DwgDbLightingUnits lightingUnits) const;
    double  ConvertCandelasToLumen () const;
    ColorDef GetGlyphColor () const;
    uint32_t GetGlyphWeight () const;
    DgnStyleId GetGlyphLinestyle () const;
    };  // DwgLightExt

/*=================================================================================**//**
//! A shared protocal extension for solid3d, region, body and surface entities.
+===============+===============+===============+===============+===============+======*/
class DwgBrepExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgBrepExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgBrepExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;
    virtual GeometricPrimitivePtr _ConvertToGeometry (DwgDbEntityCP entity, bool is2D, DwgImporterR importer, IDwgDrawParametersP params = nullptr) override;

private:
    mutable ProtocolExtensionContext*   m_toBimContext;
    mutable DwgImporterP                m_importer;
    mutable DwgDbEntityCP               m_entity;
    mutable DPoint3d                    m_placementPoint;

    BentleyStatus   CreateElement (GeometricPrimitiveR geometry, DwgImporter::ElementCreateParams& params);

#if defined (BENTLEYCONFIG_PARASOLID)
    GeometricPrimitivePtr CreateGeometry (PK_BODY_create_topology_2_r_t& brep);
    void        FreeBrep (PK_BODY_create_topology_2_r_t& brep) const;
#endif
    GeometricPrimitivePtr CreateGeometry (DwgDbPlaneSurfaceP planeSurface);
    GeometricPrimitivePtr CreateGeometry (DwgDbRegionP region);
    GeometricPrimitivePtr PlaceGeometry (CurveVectorPtr& shapes);
    BentleyStatus   SetPlacementPoint (TransformR transform);
    void GetTransparency (Render::GeometryParams& display) const;
    };  // DwgBrep

/*=================================================================================**//**
//! Block reference entity protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgBlockReferenceExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgBlockReferenceExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgBlockReferenceExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;
    };  // DwgBlockReferenceExt

/*=================================================================================**//**
//! Hatch entity protocol extension
+===============+===============+===============+===============+===============+======*/
class DwgHatchExt : public DwgProtocolExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (DwgHatchExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (DwgHatchExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;
    virtual GeometricPrimitivePtr _ConvertToGeometry (DwgDbEntityCP entity, bool is2D, DwgImporterR importer, IDwgDrawParametersP params = nullptr) override;

private:
    mutable ProtocolExtensionContext*   m_toBimContext;
    mutable DwgDbHatchCP m_hatch;
    mutable DVec3d      m_hatchNormal;
    mutable Transform   m_transform;
    mutable bool        m_isIdentityTransform;
    mutable double      m_scaleToMeters;
    mutable uint64_t    m_hatchHandle;

    BentleyStatus ConvertLoop (CurveVectorR basePaths, CurveVectorR textPaths, size_t loopIndex);
    CurveVectorPtr CreatePathFromLoop (size_t loopIndex);
    CurveVectorPtr CreatePathFromPolyline (size_t loopIndex, CurveVector::BoundaryType boundaryType);
    CurveVectorPtr CreatePathFromEdges (size_t loopIndex, CurveVector::BoundaryType boundaryType);
    CurveVector::BoundaryType GetBoundaryType (DwgDbHatch::LoopType) const;
    void ClosePath (CurveVectorPtr& path, double tolerance) const;
    bool ValidatePathDirection (CurveVectorPtr& path, size_t loopIndex) const;
    bool IsValidEdge (ICurvePrimitiveCR edge, double tolerance) const;
    double GetLoopTolerance () const;
    bool ShouldConvertLoop (size_t loopIndex) const;
    bool IsDangling (DPoint3dCR prevEnd, DPoint3dCR start, DPoint3dCR end, CurveVectorCR edges, size_t edgeNo, double tol) const;
    bool IsRepetitive (CurveVectorR path, ICurvePrimitiveCR newCurve, size_t edgeNo, double tol) const;
    void GetFillOrGradientColor (Render::GeometryParams& display) const;
    void GetTransparency (Render::GeometryParams& display) const;
    void GetDrawParameters (IDwgDrawParametersR params) const;
    BentleyStatus PlaceGeometry (GeometricPrimitiveR geom, DPoint3dR placementPoint);
    BentleyStatus SetPlacementPoint (TransformR transform, DPoint3dR placementPoint);
    BentleyStatus CreateElementInModel (GeometricPrimitiveR geometry, DwgImporterR importer);
    BentleyStatus DebugHatchBoundary (CurveVectorCR paths, DwgImporterR importer);
    };  // DwgHatchExt


END_DWG_NAMESPACE
//__PUBLISH_SECTION_END__
