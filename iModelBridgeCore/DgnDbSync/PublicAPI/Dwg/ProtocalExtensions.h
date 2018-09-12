/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Dwg/ProtocalExtensions.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <Dwg/DwgDb/DwgRxObjects.h>

USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

#ifdef DWGTOOLKIT_OpenDwg

#define DWG_PROTOCALEXT_DECLARE_MEMBERS(_className_)                                    \
    DWG_EXPORT static void            _className_::RxInit ();                           \
    DWG_EXPORT static OdRxObjectPtr   _className_::CreateObject();

#define DWG_PROTOCALEXT_DEFINE_MEMBERS(_className_)                                     \
    void            _className_::RxInit () { _className_::rxInit(); }                   \
    OdRxObjectPtr   _className_::CreateObject() { return _className_::createObject(); } \
    DWGRX_CONS_DEFINE_MEMBERS(##_className_##,DwgProtocalExtension)

#elif DWGTOOLKIT_RealDwg

#define DWG_PROTOCALEXT_DECLARE_MEMBERS(_className_)                                    \
    DWG_EXPORT static void            _className_::RxInit () {;}                        \
    DWG_EXPORT static DwgRxObjectP    _className_::CreateObject();

#define DWG_PROTOCALEXT_DEFINE_MEMBERS(_className_)                                     \
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
*
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
struct ProtocalExtensionContext
    {
private:
    ElementInputsR          m_elementInputs;
    ElementResultsR         m_elementResults;
    DgnModelP               m_resultantModel;

public:
    explicit ProtocalExtensionContext (ElementInputsR inputs, ElementResultsR results) :
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
    };  // ProtocalExtensionContext


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgProtocalExtension : public DwgRxObject
    {
public:
    DEFINE_T_SUPER (DwgRxObject)
    DWGRX_DECLARE_MEMBERS (DwgProtocalExtension)

    DWG_EXPORT static DWG_TypeP(RxClass)      Desc ();
    DWG_EXPORT static DwgProtocalExtension*   Cast (DWG_TypeCP(RxObject) obj);
    DWG_EXPORT static void                    RxInit ();

    //! Must implement this method to either create a new or update an existing element from the input entity.
    //! This method is called only when an entity is in the modelspace or a paperspace.
    DWG_EXPORT virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) = 0;
    //! Optional method to create a geometry from an entity in a block.
    //! This method is called only when an entity is in a block definition.
    DWG_EXPORT virtual GeometricPrimitivePtr _ConvertToGeometry (DwgDbEntityCP entity, DwgImporter& importer) { return nullptr; }
    };  // DwgProtocalExtension

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgRasterImageExt : public DwgProtocalExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocalExtension)
    DWGRX_DECLARE_MEMBERS (DwgRasterImageExt)
    DWG_PROTOCALEXT_DECLARE_MEMBERS (DwgRasterImageExt)

    virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) override;

private:
    mutable ProtocalExtensionContext*   m_toBimContext;
    mutable DwgImporter*                m_importer;
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
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgPointCloudExExt : public DwgProtocalExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocalExtension)
    DWGRX_DECLARE_MEMBERS (DwgPointCloudExExt)
    DWG_PROTOCALEXT_DECLARE_MEMBERS (DwgPointCloudExExt)

    virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) override;

private:
    ColorDef    GetDgnColor (DwgDbEntityCR entity) const;
    };  // DwgPointCloudExExt

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/16
+===============+===============+===============+===============+===============+======*/
class DwgViewportExt : public DwgProtocalExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocalExtension)
    DWGRX_DECLARE_MEMBERS (DwgViewportExt)
    DWG_PROTOCALEXT_DECLARE_MEMBERS (DwgViewportExt)

    virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) override;

private:
    BentleyStatus   UpdateBim (ProtocalExtensionContext& context, DwgImporter& importer, DgnModelCR rootModel, DgnModelCR sheetModel, Utf8StringCR viewName);
    };  // DwgViewportExt

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/17
+===============+===============+===============+===============+===============+======*/
class DwgLightExt : public DwgProtocalExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocalExtension)
    DWGRX_DECLARE_MEMBERS (DwgLightExt)
    DWG_PROTOCALEXT_DECLARE_MEMBERS (DwgLightExt)

    virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) override;

private:
    mutable ProtocalExtensionContext*   m_toBimContext;
    mutable DwgImporter*                m_importer;
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
* A shared protocal extension for solid3d, region, body and surface entities.
* @bsiclass                                                     Don.Fu          07/18
+===============+===============+===============+===============+===============+======*/
class DwgBrepExt : public DwgProtocalExtension
    {
public:
    DEFINE_T_SUPER (DwgProtocalExtension)
    DWGRX_DECLARE_MEMBERS (DwgBrepExt)
    DWG_PROTOCALEXT_DECLARE_MEMBERS (DwgBrepExt)

    virtual BentleyStatus  _ConvertToBim (ProtocalExtensionContext& context, DwgImporter& importer) override;
    virtual GeometricPrimitivePtr _ConvertToGeometry (DwgDbEntityCP entity, DwgImporter& importer) override;

private:
    mutable ProtocalExtensionContext*   m_toBimContext;
    mutable DwgImporter*                m_importer;
    mutable DwgDbEntityCP               m_entity;

    BentleyStatus   CreateElement (GeometricPrimitiveR geometry, DwgImporter::ElementCreateParams& params);
    ColorDef    GetEffectiveColor () const;

#if defined (BENTLEYCONFIG_PARASOLID)
    GeometricPrimitivePtr CreateGeometry (PK_BODY_create_topology_2_r_t& brep);
    void        FreeBrep (PK_BODY_create_topology_2_r_t& brep) const;
#endif
    };  // DwgBrep


END_DWG_NAMESPACE
//__PUBLISH_SECTION_END__
