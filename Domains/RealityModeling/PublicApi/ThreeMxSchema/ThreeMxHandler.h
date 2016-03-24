/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreeMxModel)
DEFINE_POINTER_SUFFIX_TYPEDEFS(S3SceneInfo)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MRMeshScene)
DEFINE_REF_COUNTED_PTR(ThreeMxModel)
DEFINE_REF_COUNTED_PTR(MRMeshScene)

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct ThreeMxDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, )

public:
    ThreeMxDomain();
};


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct TileCallback
{
    virtual void _OnTile(uint32_t x, uint32_t y, bvector<bpair<PolyfaceHeaderPtr, int>>& meshes, bvector<bpair<Byte*, Point2d>>& textures) = 0;
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ThreeMxModelHandler;
    friend struct ThreeMxProgressiveDisplay;

private:
    Utf8String m_sceneUrl;
    mutable MRMeshScenePtr m_scene;

    DRange3d GetSceneRange();
    static MRMeshScenePtr ReadScene(BeFileNameCR fileName, DgnDbR db, SystemP);

public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {}
    void GetTiles(TileCallback&, double resolution);
    double GetDefaultExportResolution() const {return 0.0;}
    void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    virtual void _WriteJsonProperties(Json::Value&) const override;
    virtual void _ReadJsonProperties(Json::Value const&) override;
    Dgn::AxisAlignedBox3d _QueryModelRange() const override;
    void SetSceneUrl(Utf8CP url) {m_sceneUrl = url;}
    void SetScene(MRMeshSceneP scene) {m_scene = scene;}
    MRMeshSceneP GetScene() const {return m_scene.get();}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxModelHandler :  dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ThreeMxModelHandler, dgn_ModelHandler::Spatial, )
    static DgnModelId CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP fileName);
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
