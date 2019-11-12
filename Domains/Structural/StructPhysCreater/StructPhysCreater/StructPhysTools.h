/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct ShapeTools
    {
    public:
        enum class Shape
            {
            Rectangle,
            HSSRectangle,
            I,
            L
            };

        static CurveVectorPtr GetRectSolidShape(double width, double depth);
        static CurveVectorPtr GetHSSRectShape(double width, double depth);
        static CurveVectorPtr GetIShape(double width, double depth);
        static CurveVectorPtr GetLShape(double xFlangeSize, double yFlangeSize);
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct PhysicalProperties
    {
    private:
        double m_xDimension;
        double m_yDimension;
        double m_zDimension;
        ShapeTools::Shape m_shape;
        Dgn::ColorDef m_lineColor;
        Dgn::ColorDef m_fillColor;

    public:
        PhysicalProperties(double xDimension, double yDimension, double zDimension, ShapeTools::Shape shape, Dgn::ColorDef lineColor, Dgn::ColorDef fillColor) :
            m_xDimension(xDimension), m_yDimension(yDimension), m_zDimension(zDimension), m_shape(shape), m_lineColor(lineColor), m_fillColor(fillColor) { }

        double GetXDimension() { return m_xDimension; }
        double GetYDimension() { return m_yDimension;  }
        double GetZDimension() { return m_zDimension; }
        ShapeTools::Shape GetShape() { return m_shape; }
        Dgn::ColorDef GetLineColor() { return m_lineColor; }
        Dgn::ColorDef GetFillColor() { return m_fillColor; }

    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct GeometricTools
    {
    private:
        static BentleyStatus GeometricTools::AppendMemberToBuilder(Dgn::GeometryBuilderPtr builder, PhysicalProperties* properties, Transform rotationMatrix, Transform linearMatrix);

    public:
        static BentleyStatus CreateStructuralMemberGeometry(
            Dgn::PhysicalElementPtr element,
            BentleyApi::Structural::StructuralPhysicalModelR model,
            ECN::ECSchemaCP schema,
            PhysicalProperties* properties,
            Transform rotationMatrix,
            Transform linearMatrix);
    };

