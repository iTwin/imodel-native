/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/BePointCloudApi.h>  //&&MM I would like to hide the dependency on BePointCloud and pointools
#include <BePointCloud/PointCloudHandle.h>
#include <BePointCloud/PointCloudScene.h>
#include <DgnPlatform/ElementTileTree.h>


USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(PointCloudModel)

struct PointCloudModelHandler;
struct PtViewport;


//=======================================================================================
// Obtain and display point cloud data from POD files. 
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModel : Dgn::SpatialModel, Dgn::Render::IGenerateMeshTiles 
{
DGNMODEL_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, Dgn::SpatialModel)

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(Dgn::SpatialModel::CreateParams);

        Utf8String m_fileUri;

        public:
            //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
            CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

            //! Parameters to create a new instance of a PointCloudModel.
            //! @param[in] dgndb The DgnDb for the new DgnModel
            //! @param[in] modeledElementId The new DgnModel will model this element
            //! @param[in] fileUri File URI of the PointCloud file.
            CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId, Utf8StringCR fileUri) :
                T_Super(dgndb, PointCloudModel::QueryClassId(dgndb), modeledElementId), m_fileUri(fileUri)
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

        Utf8String      m_fileUri;          //! File URI provided by the application. Used to resolve the local file name.
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

protected:
    friend struct PointCloudModelHandler;
    friend struct PointCloudProgressiveDisplay;

    Properties m_properties;

    //! Destruct a PointCloudModel object.
    ~PointCloudModel();

    void _OnFitView(Dgn::FitContextR) override;
    void _DropGraphicsForViewport(Dgn::DgnViewportCR viewport) override;
    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;
    AxisAlignedBox3d _QueryModelRange() const override;
    AxisAlignedBox3d _QueryNonElementModelRange() const override { return _QueryModelRange(); }

    POINTCLOUD_EXPORT Dgn::TileTree::RootPtr _CreateTileTree(Dgn::Render::SystemP) override;
    Dgn::TileTree::RootPtr _GetTileTree(Dgn::RenderContextR) override;

    TileGeneratorStatus _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) override;

public:
    //! Create a new PointCloudModel object, in preparation for loading it from the DgnDb.
    PointCloudModel(CreateParams const& params);
    PointCloudModel(CreateParams const& params, PointCloudModel::Properties const& properties) ;

    BePointCloud::PointCloudSceneP GetPointCloudSceneP () const;
    
    enum class Unit { Scene = 1, World = 2 };

    //! Return true is successful. Will fail if we cannot open the point cloud.
    bool GetRange(DRange3dR, Unit const&) const;

    POINTCLOUD_EXPORT Utf8StringCR GetSpatialReferenceWkt() const;
    POINTCLOUD_EXPORT void SetSpatialReferenceWkt(Utf8CP wktString);

    //! Gets the description of the point cloud.
    //! @return         Description of the point cloud.
    POINTCLOUD_EXPORT Utf8StringCR GetDescription() const;
    
    //! Sets the description of the point cloud.
    //! @param[in]      description     Description of the point cloud.
    POINTCLOUD_EXPORT void SetDescription(Utf8CP description);

    //! Gets the transformation matrix. 
    //! @return         A transformation matrix.
    POINTCLOUD_EXPORT TransformCR GetSceneToWorld() const;

    //! Set the transformation matrix.
    POINTCLOUD_EXPORT void SetSceneToWorld(TransformCR trans);

    //! Gets the density of a point cloud. This represents the density of points displayed for this point cloud.
    //! @return    The density (a float value between 0.0 and 1.0).
    POINTCLOUD_EXPORT float GetViewDensity() const;

    //! Sets the density of a point cloud. This represents the density of points displayed for this point cloud. Default is 1.0.
    //! @param[in]  density  The view density expressed as percentage (a float value between 0.0 and 1.0).
    POINTCLOUD_EXPORT void SetViewDensity(float density);

    POINTCLOUD_EXPORT ColorDef GetColor() const;
    POINTCLOUD_EXPORT void SetColor(ColorDef const& newColor);

    POINTCLOUD_EXPORT uint32_t GetWeight() const;
    POINTCLOUD_EXPORT void SetWeight(uint32_t const& newWeight);

    //! Query the DgnClassId of the PointCloudModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the PointCloudModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(POINTCLOUD_SCHEMA_NAME, POINTCLOUD_CLASSNAME_PointCloudModel)); }
};

//=======================================================================================
// Model handler for point clouds.
// Instances of PointCloudModel must be able to assume that their handler is a PointCloudModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, PointCloudModel, PointCloudModelHandler, Dgn::dgn_ModelHandler::Spatial, POINTCLOUD_EXPORT)

public:
    POINTCLOUD_EXPORT static PointCloudModelPtr CreatePointCloudModel(PointCloudModel::CreateParams const& params);
};

END_BENTLEY_POINTCLOUD_NAMESPACE
