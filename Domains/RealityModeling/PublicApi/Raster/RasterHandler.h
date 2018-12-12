/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Raster/RasterHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Raster/RasterTypes.h>
#include <DgnPlatform/CesiumTileTree.h>

BEGIN_BENTLEY_RASTER_NAMESPACE

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
    void FromBlob(void const* blob, size_t size);

    void InitFrom(RasterClip const& other);

    Dgn::ClipVectorCP GetClipVector() const;

    //! Create an empty clip
    RASTER_EXPORT RasterClip();
    
    RASTER_EXPORT ~RasterClip();

    RASTER_EXPORT void Clear();

    RASTER_EXPORT bool IsEmpty() const { return !HasBoundary() && GetMasks().empty(); }

    RASTER_EXPORT bool HasBoundary() const { return GetBoundaryCP() != nullptr; }

    //! Get the clip boundary.  Might be null.
    RASTER_EXPORT CurveVectorCP GetBoundaryCP() const;

    //! Set the clip boundary. Curve is not copied, its refcount will be incremented. Curve must be of CurveVector::BOUNDARY_TYPE_Outer type. 
    //! Use null to remove boundary.
    RASTER_EXPORT StatusInt SetBoundary(CurveVectorP pBoundary);

    //! Get the clip mask list.
    RASTER_EXPORT MaskVector const& GetMasks() const;

    //! Set the clip mask list.  Curve must be of CurveVector::BOUNDARY_TYPE_Inner type.
    RASTER_EXPORT StatusInt SetMasks(MaskVector const&);

    //! Add a single clip mask to the list. Curve is not copied, its refcount will be incremented. Curve must be of inner type.
    RASTER_EXPORT StatusInt AddMask(CurveVectorR curve);
 };


//=======================================================================================
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, Dgn::SpatialModel)
    BE_JSON_NAME(depthBias)

private:
    Dgn::DgnDbStatus BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    friend struct RasterModelHandler;

    double m_depthBias = 0.0;       //! A display offset in world units applied to the raster in the view Z direction. Only relevant when raster is parallel to the ground(XY plane).

    RasterClip m_clips;
    
    //! Destruct a RasterModel object.
    ~RasterModel();
    AxisAlignedBox3d _QueryNonElementModelRange() const override { return _QueryElementsRange(); }


    //This how we make our raster pick-able
//    virtual void _DrawModel(Dgn::ViewContextR) override;
    
    Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParamsCR params) override;
    void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    void _InitFrom(Dgn::DgnModelCR other) override;

    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;

    virtual bool _IsParallelToGround() const {return false;}

public:
    //! Create a new RasterModel object, in preparation for loading it from the DgnDb.
    RasterModel(CreateParams const& params);
    
    //! Get the clips of this RasterModel.
    RASTER_EXPORT RasterClipCR GetClip() const;

    //! Set the clips of this RasterModel.
    RASTER_EXPORT void SetClip(RasterClipCR);

    //! Set the clips of this RasterModel using blob data
    RASTER_EXPORT void SetClip(void const* blob, size_t size);

    Utf8String GetDescription() const;

    RASTER_EXPORT bool IsParallelToGround() const;

    void SetDepthBias(double val) { BeAssert(IsParallelToGround()); m_depthBias = val; }
    double GetDepthBias() const { return m_depthBias; }

    };

//=======================================================================================
// Model handler for raster
// Instances of RasterModel must be able to assume that their handler is a RasterModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    RASTERMODELHANDLER_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterModel, RasterModel, RasterModelHandler, Dgn::dgn_ModelHandler::Spatial, RASTER_EXPORT)

    virtual void _GetClassParams(Dgn::ECSqlClassParamsR params) override;

};

END_BENTLEY_RASTER_NAMESPACE
