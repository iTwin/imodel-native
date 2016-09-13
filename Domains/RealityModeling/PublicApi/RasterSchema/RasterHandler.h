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
// @bsiclass                                                    Mathieu.Marchand  7/2016
//=======================================================================================
struct RasterClip
 {   
public:
    typedef bvector<CurveVectorPtr> MaskVector;

private:
    CurveVectorPtr  m_pBoundary;        // Of type CurveVector::BOUNDARY_TYPE_Outer
    MaskVector      m_masks;            // Of type CurveVector::BOUNDARY_TYPE_Inner

    mutable Dgn::ClipVectorPtr m_clipVector;       // optimized for display.
public:
    void ToBlob(bvector<uint8_t>& blob, Dgn::DgnDbR dgndb) const;
    void FromBlob(void const* blob, size_t size, Dgn::DgnDbR dgndb);

    void InitFrom(RasterClip const& other);

    Dgn::ClipVectorCP GetClipVector() const;

    //! Create an empty clip
    RASTERSCHEMA_EXPORT RasterClip();
    
    RASTERSCHEMA_EXPORT ~RasterClip();

    RASTERSCHEMA_EXPORT void Clear();

    RASTERSCHEMA_EXPORT bool IsEmpty() const { return !HasBoundary() && GetMasks().empty(); }

    RASTERSCHEMA_EXPORT bool HasBoundary() const { return GetBoundaryCP() != nullptr; }

    //! Get the clip boundary.  Might be null.
    RASTERSCHEMA_EXPORT CurveVectorCP GetBoundaryCP() const;

    //! Set the clip boundary. Curve is not copied, its refcount will be incremented. Curve must be of outer type. Use null to remove.
    RASTERSCHEMA_EXPORT StatusInt SetBoundary(CurveVectorP pBoundary);

    //! Get the clip mask list.
    RASTERSCHEMA_EXPORT MaskVector const& GetMasks() const;

    //! Set the clip mask list.  Curve must be of inner type.
    RASTERSCHEMA_EXPORT StatusInt SetMasks(MaskVector const&);

    //! Add a single clip mask to the list. Curve is not copied, its refcount will be incremented. Curve must be of inner type.
    RASTERSCHEMA_EXPORT StatusInt AddMask(CurveVectorR curve);
 };


//=======================================================================================
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, Dgn::SpatialModel)

private:
    Dgn::DgnDbStatus BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    friend struct RasterModelHandler;

    enum class LoadRasterStatus
        {
        Unloaded,
        Loaded,
        UnknownError,
        };

    //&&MM TODO avoid trying to load over and over in case of error. mutable LoadRasterStatus  m_loadStatus;
    mutable RasterRootPtr m_root;

    RasterClip m_clips;
    
    //! Destruct a RasterModel object.
    ~RasterModel();
    // We now use _AddTerrainGraphics which will be called for every frame. This is required when zooming out otherwise we get flickering.
    //virtual void _AddSceneGraphics(Dgn::SceneContextR) const override;
    virtual void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    virtual BentleyStatus _Load(Dgn::Render::SystemP renderSys) const { return BSIERROR; }

    virtual void _OnFitView(Dgn::FitContextR) override;

    //This how we make our raster pick-able
//    virtual void _DrawModel(Dgn::ViewContextR) override;
    
    virtual void _DropGraphicsForViewport(Dgn::DgnViewportCR viewport) override;

    Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParamsCR params) override;
    Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
    Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
    void _InitFrom(Dgn::DgnModelCR other) override;

    virtual DMatrix4dCR  _GetSourceToWorld() const
        {
        //&&MM I have to implement this method so we can RASTERMODELHANDLER_DECLARE_MEMBERS on RasterModelHandler. We are not expecting to 
        // instantiate RasterModel directly so do we need to add RASTERMODELHANDLER_DECLARE_MEMBERS.
        static DMatrix4d s_identity;
        s_identity.InitIdentity();
        return s_identity;
        }

public:
    //! Create a new RasterModel object, in preparation for loading it from the DgnDb.
    RasterModel(CreateParams const& params);
    
    DMatrix4dCR  GetSourceToWorld() const;

    //! Get the clips of this RasterModel.
    RASTERSCHEMA_EXPORT RasterClipCR GetClip() const;

    //! Set the clips of this RasterModel.
    RASTERSCHEMA_EXPORT void SetClip(RasterClipCR);
};

//=======================================================================================
// Model handler for raster
// Instances of RasterModel must be able to assume that their handler is a RasterModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    RASTERMODELHANDLER_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, RasterModel, RasterModelHandler, Dgn::dgn_ModelHandler::Spatial, RASTERSCHEMA_EXPORT)

    virtual void _GetClassParams(Dgn::ECSqlClassParamsR params) override;
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE
