/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define SupressUnionFindDecls
#include <stdio.h>
#include <Geom/GeomApi.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include <Mtg/GpaApi.h>

// We want to compare jmdlGraphicsPointArray with bvector to omdlVArray.
// The omdlVArray functions are not exported by bsibasegeom, so we have to include the source files
//  reached on a long path through the source tree..


#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>

#include "MyArrayWrapper.h"


struct MyDPoint3dArray  : bvector <DPoint3d> {};
struct MyIntArray       : bvector <int> {};
struct MyDoubleArray    : bvector <double> {};
struct MyDPoint2dArray  : bvector <DPoint2d> {};



//typedef VArrayWrapper<GraphicsPoint> MyDPoint3dArray;
#undef GEOMDLLIMPEXP
#define GEOMDLLIMPEXP
#include "embeddedintarray.cpp"
#include "embeddeddpoint3darray.c"
#include "embeddeddoublearray.cpp"
#include "embeddeddpoint2darray.cpp"
#include "jmdl_iarray.cpp"
#include "jmdl_dpnt3.cpp"

#include "../../../../src/memory/msvarray.fdf"
#include "../../../../src/memory/msvarray.cpp"

void SameGPA (EmbeddedStructArray *varray, GraphicsPointArrayP pGPA, char *title)
    {
    int n0 = omdlVArray_getCount (varray);
    int n1 = jmdlGraphicsPointArray_getCount (pGPA);
    checkInt (n0, n1, title);
    for (int i = 0; i < n0; i++)
        {
        GraphicsPoint gp0, gp1;
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp1, i);
        omdlVArray_get (varray, (char *)&gp0, i);
        checkInt (gp0.userData, gp1.userData, "vector.userData");
        checkDouble (gp0.point.x, gp1.point.x, "vector.point.x");
        checkDouble (gp0.point.y, gp1.point.y, "vector.point.y");
        }
    }


void SameFlatMemory (GraphicsPoint const *pBuffer0, GraphicsPoint const *pBuffer1, int count, char *title)
    {
    for (int i = 0; i < count; i++)
        {
        GraphicsPoint gp0 = pBuffer0[i];
        GraphicsPoint gp1 = pBuffer1[i];
        checkInt (gp0.userData, gp1.userData, "buffer.userData");
        checkDouble (gp0.a, gp1.a, "buffer.a");
        checkDouble (gp0.point.x, gp1.point.x, "buffer.x");
        checkDouble (gp0.point.y, gp1.point.y, "buffer.y");
        }
    }


bool CheckOrderA (GraphicsPointArrayP gpa, int i)
    {
    GraphicsPoint gp0, gp1;
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp0, i);
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp1, i+1);
    return gp0.a < gp1.a;
    }

bool CheckOrderUserDataThenA (GraphicsPointArrayP gpa, int i)
    {
    GraphicsPoint gp0, gp1;
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp0, i);
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp1, i+1);
    return gp0.userData < gp1.userData
        || (gp0.userData == gp1.userData && gp0.a < gp1.a);
    }

bool CheckOrderYThenX (GraphicsPointArrayP gpa, int i)
    {
    GraphicsPoint gp0, gp1;
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp0, i);
    jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gp1, i+1);
    return gp0.point.y < gp1.point.y
        || (gp0.point.y == gp1.point.y && gp0.point.x < gp1.point.x);
    }

void CheckOrder (GraphicsPointArrayP gpa, bool checkFunc(GraphicsPointArrayP, int), char * title)
    {
    int numPoints = jmdlGraphicsPointArray_getCount (gpa);
    int numForward = 0;
    for (int i = 0; i < numPoints - 1; i++)
        {
        if (checkFunc (gpa, i))
            numForward++;

        }
    checkInt (numForward, numPoints - 1, title);
    }



int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);

#define NumPoints 10
    GraphicsPoint myPoints[NumPoints];
    for (int i = 0; i < NumPoints; i++)
        {
        memset (&myPoints[i], 0, sizeof (GraphicsPoint));
        myPoints[i].userData = i / 3;
        myPoints[i].a = (double)i;
        myPoints[i].point.x = (double)(i / 3);
        myPoints[i].point.y = cos ((double)i);
        }

    EmbeddedStructArray varray;
    omdlVArray_init (&varray, sizeof (GraphicsPoint));
    GraphicsPointArrayP gpa0 = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP gpa1 = jmdlGraphicsPointArray_grab ();
    for (int i = 0; i < NumPoints; i++)
        {
        omdlVArray_insert (&varray, (char *) &myPoints[i], -1);
        jmdlGraphicsPointArray_insertGraphicsPoint (gpa0, &myPoints[i], -1);
        jmdlGraphicsPointArray_insertGraphicsPoint (gpa1, &myPoints[NumPoints - 1 - i], 0);
        }
    SameGPA (&varray, gpa0, "insert (-1) (forward)");
    SameGPA (&varray, gpa1, "insert (0)  (reverse)");

    for (int i = 0; i < NumPoints; i++)
        {
        int numCheck = NumPoints - i;
        if (numCheck > 3)
            numCheck = 3;
        SameFlatMemory (&myPoints [i], jmdlGraphicsPointArray_getConstPtr (gpa0, i), numCheck, "getConstPtr shifted");
        SameFlatMemory (&myPoints [i], jmdlGraphicsPointArray_getPtr (gpa0, i), numCheck, "getPtr shifted");
        }

    int trimCount = 7;
    jmdlGraphicsPointArray_trim (gpa0, trimCount);
    checkInt (jmdlGraphicsPointArray_getCount (gpa0), trimCount, "trim");
    SameFlatMemory (myPoints, jmdlGraphicsPointArray_getPtr (gpa0, 0), trimCount, "After trim");

    jmdlGraphicsPointArray_releaseMem (gpa0);
    checkInt (jmdlGraphicsPointArray_getCount (gpa0), 0, "releaseMem");
    jmdlGraphicsPointArray_empty (gpa1);
    checkInt (jmdlGraphicsPointArray_getCount (gpa1), 0, "empty");

    jmdlGraphicsPointArray_empty (gpa0);
    for (int i = 0; i < NumPoints; i++)
        jmdlGraphicsPointArray_addGraphicsPoint (gpa0, &myPoints[i]);
    

    jmdlGraphicsPointArray_sortByA (gpa0);
    CheckOrder (gpa0, CheckOrderA, "SortA");
    jmdlGraphicsPointArray_sortByUserDataAndA (gpa0);
    CheckOrder (gpa0, CheckOrderUserDataThenA, "SortUserDataA");
    jmdlGraphicsPointArray_sortByYThenX (gpa0);
    CheckOrder (gpa0, CheckOrderYThenX, "SortYX");


    jmdlGraphicsPointArray_drop (gpa1);
    jmdlGraphicsPointArray_drop (gpa0);
    return getExitStatus();
    }
