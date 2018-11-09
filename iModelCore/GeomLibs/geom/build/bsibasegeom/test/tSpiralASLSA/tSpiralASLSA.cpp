
//

#include <stdio.h>
#include <msgeomstructs.hpp>
#include <msgeomstructs.h>
#include <Geom/dspiral2dbase.h>
#include <printfuncs.h>
#include <stdlib.h>




void Emit(CGXmlWriter &writer, DEllipse3dCR ellipse)
    {
    writer.EmitEllipticArc (ellipse);
    }

void Emit(CGXmlWriter &writer, DPoint3dCR xyz0, DSpiral2dBase &spiral)
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

struct ASLSACollector : DSpiral2dBase::ASLSACollector
{
CGXmlWriter &mWriter;
ASLSACollector (CGXmlWriter &writer)
    : mWriter (writer)
    {
    }

void Collect
    (
    DPoint3dCR centerA,
    DPoint3dCR xyzA0,
    DSpiral2dBase &spiralA,
    DPoint3dCR xyzA1,
    DPoint3dCR centerB,
    DPoint3dCR xyzB0,
    DSpiral2dBase &spiralB,
    DPoint3dCR xyzB1
    ) override
    {
    Emit (mWriter, xyzA0, spiralA);
    Emit (mWriter, xyzB0, spiralB);
    mWriter.EmitLine (xyzA1, xyzB1);
    }
};


void main (void)
    {
    DEllipse3d circleA, circleB;
    DSpiral2dClothoid *pClothoidA = new DSpiral2dClothoid ();
    DSpiral2dClothoid *pClothoidB = new DSpiral2dClothoid ();

    double rA = 2.0;
    double rB = 8.0;
    double centerDistance = 20.0;
    double lengthA = 8.0;
    double lengthB = 3.5;
    circleA.init (-1,0.5,0,
                rA, 0, 0,
                0, rA, 0,
                0, msGeomConst_2pi);

    circleB.init (10.0,0,0,
                rB, 0, 0,
                0, rB, 0,
                0, msGeomConst_2pi);

    CGXmlWriter cgWriter = CGXmlWriter ();
    ASLSACollector collector (cgWriter);

    double yStep = 30.0;
    double xStep = 10.0;
    double lengthStep = 2.0;
    for (int i = 0; i < 4; i++)
        {
        Emit (cgWriter, circleA);
        Emit (cgWriter, circleB);
        DSpiral2dBase::ArcSpiralLineSpiralArcTransition
                (
                circleA.center, rA, lengthA,
                circleB.center, rB, lengthB,
                *pClothoidA, *pClothoidB, collector
                );
        lengthA += lengthStep;
        circleA.center.y += yStep;
        circleB.center.y += yStep;
        circleB.center.x += xStep;
        }
    circleA.center.init (55.0, 10.0, 0);
    circleB.center.init (85.0, 30.0, 0);
    Emit (cgWriter, circleA);
    Emit (cgWriter, circleB);
    DSpiral2dBase::ArcSpiralLineSpiralArcTransition
            (
            circleA.center, rA, lengthA,
            circleB.center, rB, lengthB,
            *pClothoidA, *pClothoidB, collector
            );


    }