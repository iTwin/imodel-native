/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/PointCloudHandle.h>
#include <BePointCloud/PointCloudScene.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct PointCloudModelHandler;

//=======================================================================================
// Obtain and display point cloud data from POD files. 
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModel : BentleyApi::DgnPlatform::PointCloudBaseModel
{
    DEFINE_T_SUPER(PointCloudBaseModel)

private:
    BePointCloud::PointCloudScenePtr    m_pointCloudScenePtr;

    DRange3d                            GetSceneRange();

public:
    // POINTCLOUD_WIP_GR06_Json - To remove this, we could move JsonUtils.h to PublicApi and then delete this struct and associated methods.
    struct JsonUtils
        {
        static void DPoint3dToJson (JsonValueR outValue, DPoint3dCR point);
        static void DPoint3dFromJson (DPoint3dR point, Json::Value const& inValue);
        };

    struct Properties
        {
        DRange3d            m_range;        //! Point Cloud range
        Utf8String          m_URL;          //! URL of point cloud file

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

protected:
    friend struct PointCloudModelHandler;
    friend struct PointCloudProgressiveDisplay;

    Properties m_properties;

    //! Destruct a PointCloudModel object.
    ~PointCloudModel();

public:
    //! Create a new PointCloudModel object, in preparation for loading it from the DgnDb.
    PointCloudModel(CreateParams const& params);
    PointCloudModel(CreateParams const& params, PointCloudModel::Properties const& properties) ;

    POINTCLOUDSCHEMA_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    POINTCLOUDSCHEMA_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    POINTCLOUDSCHEMA_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    POINTCLOUDSCHEMA_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
    POINTCLOUDSCHEMA_EXPORT BePointCloud::PointCloudScenePtr GetPointCloudScenePtr ();
    POINTCLOUDSCHEMA_EXPORT DRange3dR GetRangeR() {return m_properties.m_range;}
};

//=======================================================================================
// Model handler for point clouds.
// Instances of PointCloudModel must be able to assume that their handler is a PointCloudModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModelHandler : DgnPlatform::dgn_ModelHandler::PointCloud
{
    MODELHANDLER_DECLARE_MEMBERS ("PointCloudModel", PointCloudModel, PointCloudModelHandler, DgnPlatform::dgn_ModelHandler::PointCloud, POINTCLOUDSCHEMA_EXPORT)

public:
    POINTCLOUDSCHEMA_EXPORT static DgnPlatform::DgnModelId CreatePointCloudModel(DgnDbR db, BeFileName fileName);
};

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
