/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <Geom/dspiral2dbase.h>
#include <printfuncs.h>
#include <stdlib.h>




void Emit(DGNKeyins &writer, DEllipse3dCR ellipse)
    {
    DPoint3d xyz;
    int numStep = 1 + (int)((fabs (ellipse.sweep) / 0.15) + 0.99999);
    writer.StartPolyline ();
    double df = 1.0 / (double)numStep;
    for (int i = 0; i <= numStep; i++)
        {
        double f = i * df;
        ellipse.fractionParameterToPoint (&xyz, f);
        writer.EmitXY (xyz);
        }
    writer.EndPolyline ();
    }

void Emit(DGNKeyins &writer, DPoint3dCR xyz0, DSpiral2dBase &spiral)
    {
    DPoint3d xyz[1000];
    double errorBound;
    int numOut;
    DSpiral2dBase::Stroke (spiral,
        DSpiral2dBase::DefaultStrokeAngle (),
        xyz0,
        xyz, numOut, 1000, errorBound);
    writer.StartPolyline ();
    for (int i = 0; i < numOut; i++)
        {
        writer.EmitXY (xyz[i]);
        }
    writer.EndPolyline ();
    }



void main (void)
    {
    DSpiral2dClothoid *pClothoidA = new DSpiral2dClothoid ();
    DSpiral2dClothoid *pClothoidB = new DSpiral2dClothoid ();

    DPoint3d lineA0, lineB0, focus0;
    DPoint3d lineToSpiralA, lineToSpiralB;
    DPoint3d spiralAToArc, spiralBToArc;
    lineA0.init ( 30, -1, 0);
    lineB0.init (  2,30, 0);
    focus0.init (  3,  4, 0);
    double xStep = 40.0;
    double yStep = 40.0;
    for (double lengthA = 10.0; lengthA <= 30.0; lengthA += 10.0)
        {
        lineA0.y += yStep;
        lineB0.y += yStep;
        focus0.y += yStep;
        DPoint3d lineA = lineA0;
        DPoint3d lineB = lineB0;
        DPoint3d focus = focus0;
        for (double radius = 10.0; radius <= 25.0; radius += 5.0)
            {
            double lengthB = 10;
            DEllipse3d ellipse;
            DSpiral2dBase::LineSpiralArcSpiralLineTransition
                (
                lineA, lineB, focus,
                radius, lengthA, lengthB,
                *pClothoidA, *pClothoidB,
                lineToSpiralA, lineToSpiralB,
                spiralAToArc, spiralBToArc,
                ellipse
                );

            DGNKeyins writer;
            writer.EmitColor (0);
            writer.EmitStyle (2);
            writer.EmitWeight (0);
            writer.EmitLine (focus, lineA);
            writer.EmitLine (focus, lineB);

            writer.EmitLine (ellipse.center, spiralAToArc);
            writer.EmitLine (ellipse.center, spiralBToArc);

            writer.EmitColor (3);
            writer.EmitStyle (0);
            writer.EmitWeight (0);

            Emit (writer, lineToSpiralA, *pClothoidA);
            Emit (writer, lineToSpiralB, *pClothoidB);

            writer.EmitColor (4);
            writer.EmitStyle (0);
            writer.EmitWeight (0);
            Emit (writer, ellipse);

            focus.x += xStep;
            lineA.x += xStep;
            lineB.x += xStep;
            }
        }
    }