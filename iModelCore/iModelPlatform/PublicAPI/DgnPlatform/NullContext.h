/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ViewContext.h"

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* Context that doesn't draw anything.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NullContext : ViewContext
{
    DEFINE_T_SUPER(ViewContext)

    //=======================================================================================
    // @bsistruct
    //=======================================================================================
    struct NullGraphic : Render::Graphic
    {
        NullGraphic(DgnDbR db) : Render::Graphic(db) { }
    };

    /*=================================================================================**//**
      @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct NullGraphicBuilder : Render::GraphicBuilder
    {
        bool m_isOpen = true;

        NullGraphicBuilder(Render::GraphicBuilder::CreateParams const& params) : GraphicBuilder(params) { }

        bool _IsOpen() const override {return m_isOpen;}
        void _ActivateGraphicParams(Render::GraphicParamsCR, Render::GeometryParamsCP) override {}
        void _AddLineString(int numPoints, DPoint3dCP points) override {}
        void _AddLineString2d(int numPoints, DPoint2dCP points, double zDepth) override {}
        void _AddPointString(int numPoints, DPoint3dCP points) override {}
        void _AddPointString2d(int numPoints, DPoint2dCP points, double zDepthe) override {}
        void _AddShape(int numPoints, DPoint3dCP points, bool filled) override {}
        void _AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth) override {}
        void _AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags) override {}
        void _AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double zDepth) override {}
        void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override {}
        void _AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth) override {}
        void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override {}
        void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override {}
        void _AddCurveVector(CurveVectorCR curves, bool isFilled) override {}
        void _AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) override {}
        void _AddSolidPrimitive(ISolidPrimitiveCR primitive) override {}
        void _AddBSplineSurface(MSBsplineSurfaceCR surface) override {}
        void _AddPolyface(PolyfaceQueryCR meshData, bool filled = false) override {}
        void _AddBody(IBRepEntityCR) override {}
        void _AddTextString(TextStringCR text) override {}
        void _AddTextString2d(TextStringCR text, double zDepth) override {}
        void _AddSubGraphic(Render::GraphicR, TransformCR, Render::GraphicParamsCR, ClipVectorCP) override {}
        void AddImage(ImageGraphicCR) override {}
        void AddImage2d(ImageGraphicCR, double) override {}
        Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR transform, ClipVectorCP) const override {return new NullGraphicBuilder(GetCreateParams().SubGraphic(transform));}
        Render::GraphicPtr _Finish() override { m_isOpen = false; return new NullGraphic(GetDgnDb()); }
    };

protected:
    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override {return new NullGraphicBuilder(params);}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, DgnDbR db, TransformCR tf, ClipVectorCP clips) override {return new NullGraphic(db);}

public:
    NullContext() {m_ignoreViewRange = true;}
};

END_BENTLEY_DGN_NAMESPACE
