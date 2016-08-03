/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "WmsSource.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

//----------------------------------------------------------------------------------------
// @bsiclass                                                      Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct RasterBorderGeometrySource : public GeometrySource3d, RefCountedBase
    {
    struct ElemTopology : RefCounted<IElemTopology>
        {
        protected:
            RefCountedPtr<RasterBorderGeometrySource> m_source;

            virtual IElemTopologyP _Clone() const override { return new ElemTopology(*this); }
            virtual bool _IsEqual(IElemTopologyCR rhs) const override { return _ToGeometrySource() == rhs._ToGeometrySource(); }
            virtual GeometrySourceCP _ToGeometrySource() const override { return m_source.get(); }
            virtual IEditManipulatorPtr _GetTransientManipulator(HitDetailCR hit) const { return nullptr; /*TODO*/ }

            ElemTopology(RasterBorderGeometrySource& source) { m_source = &source; }
            ElemTopology(ElemTopology const& from) { m_source = from.m_source; }

        public:

            static RefCountedPtr<ElemTopology> Create(RasterBorderGeometrySource& source) { return new ElemTopology(source); }

        }; // ElemTopology

    RasterBorderGeometrySource(DPoint3dCP corners, RasterModel& model);

    virtual DgnDbR _GetSourceDgnDb() const override { return m_dgnDb; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual Placement3dCR _GetPlacement() const override { return m_placement; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    virtual Render::GraphicSet& _Graphics() const override { return m_graphics; };
    virtual DgnElement::Hilited _IsHilited() const override { return m_hilited; }
    virtual void _GetInfoString(HitDetailCR, Utf8StringR descr, Utf8CP delimiter) const override { descr = m_infoString; }

    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource3dCP _ToGeometrySource3d() const override { return this; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { return DgnDbStatus::Success; }
    virtual DgnDbStatus _SetPlacement(Placement3dCR placement) override { return DgnDbStatus::Success; }
    virtual void _SetHilited(DgnElement::Hilited newState) const override { m_hilited = newState; }

    DgnDbR                      m_dgnDb;
    DgnCategoryId               m_categoryId;
    Placement3d                 m_placement;
    GeometryStream              m_geom;
    Utf8String                  m_infoString;
    mutable Render::GraphicSet  m_graphics;
    mutable DgnElement::Hilited m_hilited;
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
RasterBorderGeometrySource::RasterBorderGeometrySource(DPoint3dCP pCorners, RasterModel& model)
    :m_dgnDb(model.GetDgnDb()),
    m_categoryId(DgnCategory::QueryFirstCategoryId(model.GetDgnDb())),
    m_hilited(DgnElement::Hilited::None),
    m_infoString(model.GetLabel())
    {
    if (m_infoString.empty())
        m_infoString = model.GetCode().GetValueCP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*this);
    Render::GeometryParams geomParams;
    geomParams.SetCategoryId(GetCategoryId());
    geomParams.SetLineColor(ColorDef::LightGrey());
    geomParams.SetWeight(2);
    builder->Append(geomParams);

    DPoint3d box[5];
    box[0] = pCorners[0];
    box[1] = pCorners[1];
    box[2] = pCorners[3];
    box[3] = pCorners[2];
    box[4] = box[0];
    builder->Append(*ICurvePrimitive::CreateLineString(box, 5));

    builder->Finish(*this);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::RasterModel(CreateParams const& params) : T_Super (params)
    {
    m_loadStatus = LoadRasterStatus::Unloaded;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
RasterQuadTreeP RasterModel::GetTree() const
    {
    if (LoadRasterStatus::Unloaded == m_loadStatus)
        {
        BeAssert(!m_rasterTreeP.IsValid());
        if (SUCCESS == _LoadQuadTree() && m_rasterTreeP.IsValid())
            m_loadStatus = LoadRasterStatus::Loaded;
        else
            m_loadStatus = LoadRasterStatus::UnknownError;  // Do not attempt to load again.
        }        
        
    return m_rasterTreeP.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DropGraphicsForViewport(DgnViewportCR viewport)
    {
    if (m_rasterTreeP.IsValid())
        m_rasterTreeP->DropGraphicsForViewport(viewport);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_OnFitView(Dgn::FitContextR context) 
    {
    RasterQuadTreeP pTree = GetTree();
    if (NULL != pTree)
        {
        ElementAlignedBox3d box;
        box.InitFrom(pTree->GetRoot().GetCorners(), 4);
        context.ExtendFitRange(box, Transform::FromIdentity());
        }    
    }

#if 0 // This is how we pick a raster. We do not require this feature for now so disable it.
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DrawModel(Dgn::ViewContextR context)
    {
    if (context.GetDrawPurpose() == DrawPurpose::Pick)
        {
        RasterQuadTreeP pTree = GetTree();
        if (NULL != pTree)
            {
            RefCountedPtr<RasterBorderGeometrySource> pSource(new RasterBorderGeometrySource(pTree->GetRoot().GetCorners(), *this));
            RefCountedPtr<RasterBorderGeometrySource::ElemTopology> pTopology = RasterBorderGeometrySource::ElemTopology::Create(*pSource);

            context.SetElemTopology(pTopology.get());
            context.VisitGeometry(*pSource);
            context.SetElemTopology(nullptr);
            }
        }
    }
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    //Note that this call occurs on the client thread and that is must be fast.
    RasterQuadTreeP pTree = GetTree();
    if(NULL != pTree)
        pTree->Draw(context);
    }
