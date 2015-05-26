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
// Obtain and display raster data.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterBaseModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)

    struct Properties
        {
        AxisAlignedBox3d    m_range;
        Utf8String          m_URL;  

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

protected:
    friend struct RasterBaseModelHandler;

    Properties m_properties;

public:
    //! Create a new RasterBaseModel object, in preparation for loading it from the DgnDb.
    RasterBaseModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
};

//=======================================================================================
// Model handler for rasters.
// Instances of RasterBaseModel must be able to assume that their handler is a RasterBaseModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterBaseModelHandler : ModelHandler
{
    MODELHANDLER_DECLARE_MEMBERS ("RasterBaseModel", RasterBaseModel, RasterBaseModelHandler, ModelHandler, DGNPLATFORM_EXPORT)
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
