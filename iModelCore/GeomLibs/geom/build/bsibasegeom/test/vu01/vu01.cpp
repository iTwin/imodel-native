//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Vu/vuApi.h>
//#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>
#include <vuprint.cpp>
static int s_debug = 1;

void printMarkup (VuSetP graph, char *title)
    {
    if (s_debug > 2)
        {
        vu__printFaceLabels  (graph, title);
        vu__printMaskedNodes (graph, VU_KNOT_EDGE, "MASK_BOUNDARY_A");
        vu__printMaskedNodes (graph, VU_SEAM_EDGE, "MASK_BOUNDARY_B");
        vu__printMaskedNodes (graph, VU_RULE_EDGE, "MASK_EXTERIOR_A");
        vu__printMaskedNodes (graph, VU_GRID_EDGE, "MASK_EXTERIOR_B");
        }
    }
#include <vuPolygonClassifier.h>
static double s_scale=100000.0;
static void TRACE (char *pString)
    {
    if (s_debug)
        printf (pString);
    }


static void TRACE (char *pString, double x, double y)
    {
    if (s_debug)
        printf (pString, s_scale * x, s_scale * y);
    }


void Translate (vuVectorDPoint3d& poly, double x, double y, double z = 0.0)
    {
    for (size_t i = 0; i < poly.size (); i++)
        {
        poly[i].x += x;
        poly[i].y += y;
        poly[i].z += z;
        }
    }

static void Add (vuVectorDPoint3d& poly, double x, double y, double z = 0.0);
static void AddDisconnect (vuVectorDPoint3d& poly);
static void DumpFace (vuVectorDPoint3d& poly);
static void DumpFaces (VuPolygonClassifier& vu);
 
void LoadOriginal (vuVectorDPoint3dR polyA, vuVectorDPoint3dR polyB)
    {
    polyA.clear ();
    polyB.clear ();

    Add (polyA, 22.17513199254067000000,32.62665127700191200000);
    Add (polyA, 22.16513530933738000000,32.62665127700191200000);
    Add (polyA, 0.00000000000002842171,18.59498635601892500000);
    Add (polyA, 0.00000000000000000000,0.00000000000000000000);
    Add (polyA, 22.17513199254066300000,-0.37329886944742585000);
    Add (polyA, 22.17513199254067000000,31.58290127700193000000);
    Add (polyA, 22.13550268430353900000,31.58290127700193000000);
    Add (polyA, 22.13550268430353900000,32.58706794366860000000);
    Add (polyA, 22.17513199254067000000,32.58706794366860000000);
    AddDisconnect (polyA);

    Add (polyA, 23.17929865920733300000,-0.39020313326554212000);
    Add (polyA, 59.48772642406876800000,-1.00142362298328540000);
    Add (polyA, 58.52343601763688200000,32.58460292758938000000);
    Add (polyA, 58.52343601763688200000,31.58290127700193000000);
    Add (polyA, 23.17929865920733300000,31.58290127700193000000);
    AddDisconnect (polyA);
    
    Add (polyA, 23.17929865920733300000,32.58706794366860000000);
    Add (polyA, 58.52336524441358300000,32.58706794366858600000);
    Add (polyA, 58.52222876497531000000,32.62665127700194000000);
    Add (polyA, 23.17929865920733300000,32.62665127700191200000);

    Add (polyB, 59.48653601763687200000,-0.96111218491066808000);
    Add (polyB, 59.48653601763687200000,0.04305448175600190600);
    Add (polyB, 22.13550268430355300000,0.04305448175600190600);
    Add (polyB, 22.13550268430355300000,-0.96111218491066808000);

    }

void LoadBridgeTest (vuVectorDPoint3dR polyA, vuVectorDPoint3dR polyB, double yy, bool b0, bool b1, bool b2, bool b3, bool b4)
    {
    polyA.clear ();
    Add (polyA, 0,0,0);
    Add (polyA, 100,   0, 0);
    Add (polyA, 100,yy, 0);
    Add (polyA,   0,100, 0);

    polyB.clear ();
    if (b0)
        {
        Add (polyB, 10,10,0);
        Add (polyB, 90,10,0);
        Add (polyB, 90,20,0);
        Add (polyB, 10,20,0);
        AddDisconnect (polyB);
        }

    if (b1)
        {
        Add (polyB, 10,80,0);
        Add (polyB, 90,80,0);
        Add (polyB, 90,90,0);
        Add (polyB, 10,90,0);
        AddDisconnect (polyB);
        }

    if (b2)
        {
        Add (polyB, 80,30,0);
        Add (polyB, 90,30,0);
        Add (polyB, 90,70,0);
        Add (polyB, 80,70,0);
        AddDisconnect (polyB);
        }

    if (b3)
        {
        Add (polyB, 40,50,0);
        Add (polyB, 50,50,0);
        Add (polyB, 50,60,0);
        Add (polyB, 40,60,0);
        AddDisconnect (polyB);
        }

    if (b4)
        {
        Add (polyB, 10,30,0);
        Add (polyB, 20,30,0);
        Add (polyB, 20,70,0);
        Add (polyB, 10,70,0);
        AddDisconnect (polyB);
        }

    }
void main(void)
{
    vuVectorDPoint3d polyA, polyB;
    //LoadOriginal (polyA, polyB);
    bool bb[10];
    bb[0] = bb[1] = bb[2] = bb[3] = bb[4] = bb[5] = 0;
    double yy[3] = {100, 50, 5};

    double dx = 0.0;
    VuFaceMode faceMode[3] = {FaceMode_Unconnected, FaceMode_Triangulate, FaceMode_BridgeEdges};
    VuPolygonClassifier vu;

    for (int iF = 0; iF < 3; iF++)
        {
        vu.mFaceMode = faceMode[iF];
        for (int iY = 0; iY < 3; iY++)
            {
            for (int numOn = 1; numOn <= 5; numOn += 2)
                {
                for (int k = 0; k < 5; k++)
                    bb[k] = k < numOn;
                LoadBridgeTest(polyA, polyB, yy[iY], bb[0], bb[1], bb[2], bb[3], bb[4]);

                Translate (polyA, dx, 0, 0);
                Translate (polyB, dx, 0, 0);
                //////////////////////////////////////////
                TRACE("active color green\n");
                DumpFace (polyA);
                TRACE("active color blue\n");
                DumpFace (polyB);

                TRACE("active color green\n");
                Translate (polyA, 0,110, 0);
                Translate (polyB, 0,110, 0);

                vu.ClassifyAB (polyA, polyB, true, false, false, false);
                vu.SetupForLoopOverInteriorFaces  ();
                DumpFaces (vu);

                Translate (polyA, 0,110, 0);
                Translate (polyB, 0,110, 0);

                TRACE("active color blue\n");
                vu.ClassifyAB (polyA, polyB, false, true, false, false);
                vu.SetupForLoopOverInteriorFaces  ();
                DumpFaces (vu);

                Translate (polyA, 0,110, 0);
                Translate (polyB, 0,110, 0);

                TRACE("active color red\n");
                vu.ClassifyAB (polyA, polyB, false, false, true, false);
                vu.SetupForLoopOverInteriorFaces  ();
                DumpFaces (vu);

                dx += 110;
                }
            }
        }
    }
//    vu.ClassifyAMinusB (polyA, polyB);

 
static void Add (vuVectorDPoint3d& poly, double x, double y, double z)
    {
    DPoint3d pt;
    pt.init (x,y,z);
    poly.push_back (pt);
    }

static void AddDisconnect (vuVectorDPoint3d& poly)
    {
    DPoint3d pt;
    pt.initDisconnect();
    poly.push_back (pt);
    }

static void DumpFace (vuVectorDPoint3d& poly)
    {
    int n = 0;
    for (size_t i=0; i<poly.size(); i++)
        {
        if (poly[i].isDisconnect ())
            {
            if (n > 0)
                TRACE ("close element\n");
            n = 0;
            }
        else
            {
            if (n == 0)
                TRACE ("place shape\n");
            TRACE("xy=%.15le,%.15le\n",poly[i].x,poly[i].y);
            n++;
            }
        }
    if (n > 0)
        TRACE ("close element\n");
    }

static void DumpFaces (VuPolygonClassifier& vu)
    {
        //TRACE ("Dump faces:\n");
        vuVectorDPoint3d face;
        for (;vu.GetFace (face);)
            {
            DumpFace (face);
            }
    }

