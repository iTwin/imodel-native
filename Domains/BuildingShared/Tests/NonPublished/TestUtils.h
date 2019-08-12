/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>
#include <Tests/DgnProject/Published/FakeRenderSystem.h>

BEGIN_BUILDING_SHARED_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(FakeGraphicBuilder);
DEFINE_REF_COUNTED_PTR(FakeGraphicBuilder);

struct TestUtils
    {
    static void CompareCurves(ICurvePrimitivePtr lhs, ICurvePrimitivePtr rhs);
    static void ComparePoints(bvector<DPoint3d> lhs, bvector<DPoint3d> rhs);
    static ICurvePrimitivePtr CreateSpline(bvector<DPoint3d> poles, int order);
    static ICurvePrimitivePtr CreateInterpolationCurve(bvector<DPoint3d> poles, DVec3d startTangent = DVec3d::From(0, 0, 0), DVec3d endTangent = DVec3d::From(0, 0, 0));
    };

//=======================================================================================
// @bsiclass                                     Martynas.Saulius               05/2018
//=======================================================================================
struct FakeGraphicBuilder : FakeRender::FakeGraphicBuilder
    {
    struct GeometryEntry
        {
        GraphicParams m_graphicParams;
        IGeometryPtr m_geometry;
        bool m_isFilled;
        };

    struct TextStringEntry
        {
        GraphicParams m_graphicParams;
        TextStringCPtr m_textString;
        };

    private:
        typedef FakeRender::FakeGraphicBuilder T_Super;

    protected:
        FakeGraphicBuilder(CreateParamsCR params) : FakeRender::FakeGraphicBuilder(params) {}

        virtual void _AddTextString(TextStringCR text) override { m_textStrings.push_back(TextStringEntry {m_graphicParams, &text}); }
        virtual void _AddCurveVector(CurveVectorCR curves, bool isFilled) override { m_geometry.push_back(GeometryEntry {m_graphicParams, IGeometry::Create(curves.Clone()), isFilled}); }
        virtual void _AddLineString(int numPoints, DPoint3dCP points) override { m_geometry.push_back(GeometryEntry {m_graphicParams, IGeometry::Create(ICurvePrimitive::CreateLineString(points, numPoints)), false}); }
        virtual void _AddPointString(int numPoints, DPoint3dCP points) override { m_geometry.push_back(GeometryEntry {m_graphicParams, IGeometry::Create(ICurvePrimitive::CreatePointString(points, numPoints)), false}); }
        virtual void _AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled) override { m_geometry.push_back(GeometryEntry {m_graphicParams, IGeometry::Create(ICurvePrimitive::CreateArc(ellipse)), filled}); }
        virtual void _ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams) override { m_graphicParams = graphicParams; }

    public:
        GraphicParams m_graphicParams;

        bvector<GeometryEntry> m_geometry;
        bvector<TextStringEntry> m_textStrings;

        bool IsEmpty()
            {
            return m_geometry.empty()
                && m_textStrings.empty();
            }

        static FakeGraphicBuilderPtr Create(CreateParamsCR params) { return new FakeGraphicBuilder(params); }
    };

END_BUILDING_SHARED_NAMESPACE