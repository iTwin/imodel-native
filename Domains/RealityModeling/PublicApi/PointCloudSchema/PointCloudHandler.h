/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct PtViewport;

//=======================================================================================
// Obtain and display point cloud data from POD files. 
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModel : Dgn::SpatialModel
{
DGNMODEL_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, Dgn::SpatialModel)


public:
    enum class LoadStatus
        {
        Unloaded,
        Loaded,
        UnknownError,
        };

    struct Properties
        {
        DRange3d            m_range;        //! Point Cloud range
        Utf8String          m_fileId;       //! File id provided by the application. Used to resolve the local file name.

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

private:
    mutable LoadStatus                          m_loadSceneStatus;
    mutable BePointCloud::PointCloudScenePtr    m_pointCloudScenePtr;
    mutable Transform                           m_sceneToWorld;
    mutable std::map<DgnViewportCP, RefCountedPtr<PtViewport>> m_cachedPtViewport;

    //! May return nullptr when we reach the limit.
    PtViewport* GetPtViewportP(DgnViewportCR) const;

    // POINTCLOUD_WIP_GR06_Json - To remove this, we could move JsonUtils.h to PublicApi and then delete this struct and associated methods.
    struct JsonUtils
        {
        static void DPoint3dToJson(JsonValueR outValue, DPoint3dCR point);
        static void DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue);
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

    virtual void _AddSceneGraphics(Dgn::SceneContextR) const override;

    virtual void _OnFitView(Dgn::FitContextR) override;
    
    virtual void _DropGraphicsForViewport(Dgn::DgnViewportCR viewport) override;

    virtual void _WriteJsonProperties(Json::Value&) const override;
    virtual void _ReadJsonProperties(Json::Value const&) override;
    virtual Dgn::AxisAlignedBox3d _QueryModelRange() const override;
    BePointCloud::PointCloudSceneP GetPointCloudSceneP () const;
    DRange3dCR GetRange() const {return m_properties.m_range;}
    DRange3dR GetRangeR() {return m_properties.m_range;}
    DRange3d GetSceneRange() const;

    TransformCR GetSceneToWorld() const {return m_sceneToWorld;}

};

//=======================================================================================
// Model handler for point clouds.
// Instances of PointCloudModel must be able to assume that their handler is a PointCloudModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, PointCloudModel, PointCloudModelHandler, Dgn::dgn_ModelHandler::Spatial, POINTCLOUDSCHEMA_EXPORT)

public:
    POINTCLOUDSCHEMA_EXPORT static Dgn::DgnModelId CreatePointCloudModel(DgnDbR db, Utf8StringCR fileId);
};

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
