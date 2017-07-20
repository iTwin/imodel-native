/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/GeometricTools.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct StructuralMemberGeometricProperties
    {
    private:
        double m_xDimension;
        double m_yDimension;
        double m_zDimension;
        Dgn::ColorDef m_lineColor;
        Dgn::ColorDef m_fillColor;

    public:
        StructuralMemberGeometricProperties(double xDimension, double yDimension, double zDimension, Dgn::ColorDef lineColor, Dgn::ColorDef fillColor) :
            m_xDimension(xDimension), m_yDimension(yDimension), m_zDimension(zDimension), m_lineColor(lineColor), m_fillColor(fillColor) { }

        double GetXDimension() { return m_xDimension; }
        double GetYDimension() { return m_yDimension;  }
        double GetZDimension() { return m_zDimension; }
        Dgn::ColorDef GetLineColor() { return m_lineColor; }
        Dgn::ColorDef GetFillColor() { return m_fillColor; }
    };

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct GeometricTools
    {
    public:
        static BentleyStatus CreateStructuralMemberGeometry(Dgn::PhysicalElementPtr element, StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, StructuralMemberGeometricProperties* properties);
    };
