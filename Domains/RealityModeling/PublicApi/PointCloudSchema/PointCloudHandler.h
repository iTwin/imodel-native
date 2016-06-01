/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/BePointCloudApi.h>  //&&MM I would like to hide the dependency on BePointCloud and pointools
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
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(Dgn::SpatialModel::CreateParams);

        Utf8String m_fileId;

        public:
            //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
            CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

            //! Parameters to create a new instance of a PointCloudModel.
            //! @param[in] dgndb The DgnDb for the new DgnModel
            //! @param[in] code The Code for the DgnModel
            //! @param[in] fileId File Id of the PointCloud file.
            CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnCode code, Utf8StringCR fileId) :
                T_Super(dgndb, PointCloudModel::QueryClassId(dgndb), code), m_fileId(fileId)
                {}
        };
        
    enum class LoadStatus
        {
        Unloaded,
        Loaded,
        UnknownError,
        };

    struct Properties
        {
        Properties();
        ~Properties() {}

        Utf8String      m_fileId;           //! File id provided by the application. Used to resolve the local file name.
        Utf8String      m_description;

        Transform       m_sceneToWorld;     //! Including reprojection transformation if any.
        Utf8String      m_wkt;
        float           m_density;

        ColorDef        m_color;
        uint32_t        m_weight;
        
        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

private:
    mutable LoadStatus                          m_loadSceneStatus;
    mutable BePointCloud::PointCloudScenePtr    m_pointCloudScenePtr;

    struct ViewportCacheEntry
        {
        RefCountedPtr<PtViewport> m_ptViewport;         // pointools viewport.
        Dgn::Render::GraphicPtr   m_lowDensityGraphic;  // Low density representation with the latest viewport settings.
        };
    mutable std::map<DgnViewportCP, ViewportCacheEntry> m_viewportCache;
    
    //! May return nullptr when we reach the limit.
    PtViewport* GetPtViewportP(DgnViewportCR) const;

    Dgn::Render::Graphic* GetLowDensityGraphicP(DgnViewportCR) const;
    void SaveLowDensityGraphic(DgnViewportCR, Dgn::Render::Graphic*);


    // POINTCLOUD_WIP_GR06_Json - To remove this, we could move JsonUtils.h to PublicApi and then delete this struct and associated methods.
    struct JsonUtils
        {
        static void DPoint3dToJson(JsonValueR outValue, DPoint3dCR point);
        static void DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue);
        static void TransformRowFromJson(double* row, JsonValueCR inValue);
        static void TransformRowToJson(JsonValueR outValue, double const* row);
        static void TransformFromJson(TransformR trans, JsonValueCR inValue);
        static void TransformToJson(JsonValueR outValue, TransformCR trans);
        };

protected:
    friend struct PointCloudModelHandler;
    friend struct PointCloudProgressiveDisplay;

    Properties m_properties;

    //! Destruct a PointCloudModel object.
    ~PointCloudModel();

    virtual void _AddSceneGraphics(Dgn::SceneContextR) const override;
    virtual void _OnFitView(Dgn::FitContextR) override;
    virtual void _DropGraphicsForViewport(Dgn::DgnViewportCR viewport) override;
    virtual void _WriteJsonProperties(Json::Value&) const override;
    virtual void _ReadJsonProperties(Json::Value const&) override;
    virtual Dgn::AxisAlignedBox3d _QueryModelRange() const override;
public:
    //! Create a new PointCloudModel object, in preparation for loading it from the DgnDb.
    PointCloudModel(CreateParams const& params);
    PointCloudModel(CreateParams const& params, PointCloudModel::Properties const& properties) ;

    BePointCloud::PointCloudSceneP GetPointCloudSceneP () const;
    
    enum class Unit { Scene = 1, World = 2 };

    //! Return true is successful. Will fail if we cannot open the point cloud.
    bool GetRange(DRange3dR, Unit const&) const;

    POINTCLOUDSCHEMA_EXPORT Utf8StringCR GetSpatialReferenceWkt() const;
    POINTCLOUDSCHEMA_EXPORT void SetSpatialReferenceWkt(Utf8CP wktString);

    //! Gets the description of the point cloud.
    //! @return         Description of the point cloud.
    POINTCLOUDSCHEMA_EXPORT Utf8StringCR GetDescription() const;
    
    //! Sets the description of the point cloud.
    //! @param[in]      description     Description of the point cloud.
    POINTCLOUDSCHEMA_EXPORT void SetDescription(Utf8CP description);

    //! Gets the transformation matrix. 
    //! @return         A transformation matrix.
    POINTCLOUDSCHEMA_EXPORT TransformCR GetSceneToWorld() const;

    //! Set the transformation matrix.
    POINTCLOUDSCHEMA_EXPORT void SetSceneToWorld(TransformCR trans);

    //! Gets the density of a point cloud. This represents the density of points displayed for this point cloud.
    //! @return    The density (a float value between 0.0 and 1.0).
    POINTCLOUDSCHEMA_EXPORT float GetViewDensity() const;

    //! Sets the density of a point cloud. This represents the density of points displayed for this point cloud. Default is 1.0.
    //! @param[in]  density  The view density expressed as percentage (a float value between 0.0 and 1.0).
    POINTCLOUDSCHEMA_EXPORT void SetViewDensity(float density);

    POINTCLOUDSCHEMA_EXPORT ColorDef GetColor() const;
    POINTCLOUDSCHEMA_EXPORT void SetColor(ColorDef const& newColor);

    POINTCLOUDSCHEMA_EXPORT uint32_t GetWeight() const;
    POINTCLOUDSCHEMA_EXPORT void SetWeight(uint32_t const& newWeight);

    //! Query the DgnClassId of the PointCloudModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the PointCloudModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(POINTCLOUD_SCHEMA_NAME, POINTCLOUD_CLASSNAME_PointCloudModel)); }
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
    POINTCLOUDSCHEMA_EXPORT static PointCloudModelPtr CreatePointCloudModel(PointCloudModel::CreateParams const& params);
};

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
