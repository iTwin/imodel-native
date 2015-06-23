/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PointCloudBaseModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_REF_COUNTED_PTR(PointCloudBaseModel)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct PointCloudBaseModelHandler;

//=======================================================================================
// Obtain and display point cloud data from POD files. 
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudBaseModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)

protected:
    friend struct PointCloudBaseModelHandler;

public:
    //! Create a new PointCloudBaseModel object, in preparation for loading it from the DgnDb.
    PointCloudBaseModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
};

//=======================================================================================
// Model handler for point clouds.
// Instances of PointCloudBaseModel must be able to assume that their handler is a PointCloudBaseModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudBaseModelHandler : ModelHandler
{
    MODELHANDLER_DECLARE_MEMBERS ("PointCloudBaseModel", PointCloudBaseModel, PointCloudBaseModelHandler, ModelHandler, DGNPLATFORM_EXPORT)
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
