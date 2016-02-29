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

    RasterQuadTreePtr m_rasterTreeP;
    
    //! Destruct a RasterModel object.
    ~RasterModel();
        
    virtual void _AddGraphicsToScene(Dgn::SceneContextR) override;
    virtual void _WriteJsonProperties(Json::Value&) const override;
    virtual void _ReadJsonProperties(Json::Value const&) override;

    virtual BentleyStatus _LoadQuadTree() {return BSIERROR;}

    RasterQuadTreeP GetTree();

    //&&MM TODO 
    virtual void _OnFitView(Dgn::FitContextR) {}

    //&&MM TODO this how we make our raster pick-able
    virtual void _DrawModel(Dgn::ViewContextR) {}

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
    RASTERMODELHANDLER_DECLARE_MEMBERS (RASTER_CLASSNAME_RasterModel, RasterModel, RasterModelHandler, Dgn::dgn_ModelHandler::Spatial, RASTERSCHEMA_EXPORT)
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
