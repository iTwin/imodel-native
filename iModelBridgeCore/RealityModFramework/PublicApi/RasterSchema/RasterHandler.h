/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnCore/RasterBaseModel.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterModelHandler;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModel : BentleyApi::DgnPlatform::RasterBaseModel
{
    DEFINE_T_SUPER(RasterBaseModel)

protected:
    friend struct RasterModelHandler;
    friend struct RasterProgressiveDisplay;

    //! Destruct a RasterModel object.
    ~RasterModel();

public:
    //! Create a new RasterModel object, in preparation for loading it from the DgnDb.
    RasterModel(CreateParams const& params);

    RASTERSCHEMA_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    RASTERSCHEMA_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    RASTERSCHEMA_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    RASTERSCHEMA_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;

    //! Call this after creating a new model, in order to set up subclass-specific properties.
    BentleyStatus SetProperties (BeFileName fileName);
};

//=======================================================================================
// Model handler for raster
// Instances of RasterModel must be able to assume that their handler is a RasterModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModelHandler : DgnPlatform::RasterBaseModelHandler
{
    MODELHANDLER_DECLARE_MEMBERS ("RasterModel", RasterModel, RasterModelHandler, RasterBaseModelHandler, RASTERSCHEMA_EXPORT)

public:
    RASTERSCHEMA_EXPORT static DgnPlatform::DgnModelId CreateRasterModel(DgnDbR db, BeFileName fileName);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
