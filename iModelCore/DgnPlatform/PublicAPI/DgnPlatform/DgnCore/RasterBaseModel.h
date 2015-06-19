/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RasterBaseModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_REF_COUNTED_PTR(RasterBaseModel)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct RasterBaseModelHandler;

//=======================================================================================
//! Obtain and display raster data.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterBaseModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)

protected:
    friend struct RasterBaseModelHandler;

public:
    //! Create a new RasterBaseModel object, in preparation for loading it from the DgnDb.
    RasterBaseModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
};

namespace dgn_ModelHandler
{
    //=======================================================================================
    //! The ModelHandler for RasterBaseModel.
    // @bsiclass                                                    Eric.Paquet     04/2015
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Raster : Model
    {
        MODELHANDLER_DECLARE_MEMBERS ("RasterBaseModel", RasterBaseModel, Raster, Model, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
