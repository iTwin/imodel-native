/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <RasterSchema/RasterSchemaTypes.h>

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterModelHandler;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, Dgn::SpatialModel)

protected:
    friend struct RasterModelHandler;

    enum class LoadRasterStatus
        {
        Unloaded,
        Loaded,
        UnknownError,
        };

    mutable LoadRasterStatus  m_loadStatus;
    mutable RasterQuadTreePtr m_rasterTreeP;
    
    //! Destruct a RasterModel object.
    ~RasterModel();
    // We now use _AddTerrainGraphics which will be called for every frame. This is required when zooming out otherwise we get flickering.
    //virtual void _AddSceneGraphics(Dgn::SceneContextR) const override;
    virtual void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    virtual BentleyStatus _LoadQuadTree() const {return BSIERROR;}

    RasterQuadTreeP GetTree() const;

    virtual void _OnFitView(Dgn::FitContextR) override;

    //This how we make our raster pick-able
//    virtual void _DrawModel(Dgn::ViewContextR) override;
    
    virtual void _DropGraphicsForViewport(Dgn::DgnViewportCR viewport) override;

public:
    //! Create a new RasterModel object, in preparation for loading it from the DgnDb.
    RasterModel(CreateParams const& params);
};

//=======================================================================================
// Model handler for raster
// Instances of RasterModel must be able to assume that their handler is a RasterModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    RASTERMODELHANDLER_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, RasterModel, RasterModelHandler, Dgn::dgn_ModelHandler::Spatial, RASTERSCHEMA_EXPORT)
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
