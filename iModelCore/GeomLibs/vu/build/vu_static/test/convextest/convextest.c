#include <stdio.h>
#include <Bentley/Bentley.h>
#include <basetype.h>
#include <basedefs.h>
#include <Vu.h>

#ifdef abc
typedef struct
    {
    double x,y,z;
    } DPoint3d;
typedef int StatusInt;

#define POLYFILL_EXTERIOR_EDGE 0x01
typedef void (*PFPolygonSplitPolygonFunctionExt)
(
DPoint3d    *pPoints,
int         *pFlags,
int         numverts,
void        *pUserArg
);

/*----------------------------------------------------------------------+
@description  Split a polygon into smaller convex polygons.  Output each convex
 polygon to a callback in form
            polygonFunc (vertexArray, flagArray, numVertex, userDataP)
 where flagArray[i] = true iff the edge from vertex i to i+1 is
            a true boundary edge.
@param pLoopPoints IN array of polygon points.  May include DISCONNECT points.
@param numLoopPoints IN number of points in pLoopPoints array.
@param userDataP IN data pointer for callback.
@param polygonFunc IN function to call with convex polygons.
+----------------------------------------------------------------------*/
StatusInt vu_splitToConvexParts
(
DPoint3d                            *pLoopPoints,
int                                 numLoopPoints,
void                                *userDataP,
PFPolygonSplitPolygonFunctionExt    polygonFunc
);
#endif


void myPolyfunc
(
DPoint3d    *pPoints,
int         *pFlags,
int         numverts,
void        *pUserArg
)
    {
    int i;
    printf ("<polygon>\n");
    for (i = 0; i < numverts; i++)
        printf ("    <point xy=\"%lf,%lf\"/>\n", pPoints[i].x, pPoints[i].y);
    printf ("</polygon>\n");
    }

static int s_noisy = 0;
void graphTrapFunc
(
VuSetP  pGraph,
char    *pAppName,
int     id0,
int     id1
)
    {
    if (s_noisy > 9)
        {
        printf ("\n GRAPH %s %d %d\n", pAppName, id0, id1);
        vu_printFaceLabels (pGraph, pAppName);
        }
    }

void main (int numArg, char **pArg)
{
    int i;
    DPoint3d points[7] = {
        {0,0,0},
        {4,0,0},
        {4,4,0},
        {2,4,0},
        {2,-1,0},
        {0,1,0},
        {0,0,0}
        };

    for (i = 0; i < numArg; i++)
        {
        if (0 == strcmp (pArg[i], "-noisy"))
            s_noisy = 100;
        }
    vu_setGraphTrapFunc (graphTrapFunc);
    printf ("Hello\n");
    vu_splitToConvexParts (points, 7, NULL, myPolyfunc);
    printf ("Goodbye\n");
}