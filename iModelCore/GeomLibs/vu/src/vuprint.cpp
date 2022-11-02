/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//#define NoGeomPrintFuncs
struct GeomPrintFuncs;

// Virtuals for debug output.
typedef GeomPrintFuncs *GeomPrintFuncsP;

struct GEOMDLLIMPEXP GeomPrintFuncs
    {
    public:
        // Default implementation of the lowest level EmitChar does nothing -- derived class must override and output to actual device.
        GEOMAPI_VIRTUAL void EmitChar (char ch);
        // Default implementations reduce to EmitChar ...
        GEOMAPI_VIRTUAL void EmitString (char const *pString);
        GEOMAPI_VIRTUAL void EmitInt (int value);
        GEOMAPI_VIRTUAL void EmitDouble (double value);
        GEOMAPI_VIRTUAL void EmitLineBreak ();

   };

struct GEOMDLLIMPEXP GeomPrintToBeConsole : public GeomPrintFuncs
{
void EmitChar (char ch) override;
void EmitString (char const *pString) override;
};



// Default implementations of the lowest level EmitChar does nothing -- derived class must override and output to actual device.
void GeomPrintFuncs::EmitChar (char ch)
    {
    }
        // Default implementations reduce to EmitChar ...
void GeomPrintFuncs::EmitString (char const *pString)
    {
    for (int i = 0; pString[i] != 0; i++)
        EmitChar (pString[i]);
    }

void GeomPrintFuncs::EmitInt (int value)
    {
    char buffer[100];
    snprintf (buffer, sizeof(buffer), "%i", value);
    EmitString (buffer);
    }

void GeomPrintFuncs::EmitDouble (double value)
    {
    char buffer[100];
    snprintf (buffer, sizeof(buffer), "%g", value);
    EmitString (buffer);
    }

void GeomPrintFuncs::EmitLineBreak ()
    {
    EmitChar ('\n');
    }


void GeomPrintToBeConsole::EmitChar (char ch)
    {
#ifdef GEOMAPI_PRINTF
    GEOMAPI_PRINTF ("%c", ch);
#else
    BeConsole::Printf ("%c", ch);
#endif
    }
void GeomPrintToBeConsole::EmitString (char const *pString)
    {
#ifdef GEOMAPI_PRINTF
    GEOMAPI_PRINTF  ("%s", pString);
#else
    BeConsole::Printf ("%s", pString);
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vu_printNode
(
VuSetP          graphP,
VuP             currP,
bool            bPrintVertexLoop,
GeomPrintFuncs&     gs
)
    {
    VuP mateP = vu_edgeMate (currP);
    /*printf (" %5d|%5d ", VU_ID(currP), VU_ID(mateP));
    printf ("%c", vu_getMask (currP, VU_BOUNDARY_EDGE) ? 'B' : ' ');
    printf ("%c", vu_getMask (mateP, VU_BOUNDARY_EDGE) ? 'b' : ' ');
    printf ("%c", vu_getMask (currP, VU_EXTERIOR_EDGE) ? 'X' : ' ');
    printf ("%c", vu_getMask (mateP, VU_EXTERIOR_EDGE) ? 'x' : ' ');
    printf (" %4d", VU_GET_LABEL_AS_INT (currP));
    printf (" (%10lg,%10lg)", VU_U(currP), VU_V(currP));
    printf (" (");*/

    char str1[1024];
    snprintf (str1, sizeof(str1), "(%5d ", VU_ID(currP));
    gs.EmitString (str1);

    vu_getMask (currP, VU_RULE_EDGE) ? gs.EmitChar ('R') : gs.EmitChar (' ');
    vu_getMask (currP, VU_GRID_EDGE) ? gs.EmitChar ('G') : gs.EmitChar (' ');

    vu_getMask (currP, VU_KNOT_EDGE) ? gs.EmitChar ('K') : gs.EmitChar (' ');
    vu_getMask (currP, VU_SEAM_EDGE) ? gs.EmitChar ('S') : gs.EmitChar (' ');

    vu_getMask (currP, VU_U_SLICE) ? gs.EmitChar ('U') : gs.EmitChar (' ');
    vu_getMask (currP, VU_V_SLICE) ? gs.EmitChar ('V') : gs.EmitChar (' ');
    vu_getMask (currP, VU_BOUNDARY_EDGE) ? gs.EmitChar ('B') : gs.EmitChar (' ');
    vu_getMask (currP, VU_EXTERIOR_EDGE) ? gs.EmitChar ('X') : gs.EmitChar (' ');
    vu_getMask (currP, VU_NULL_FACE) ? gs.EmitChar ('N') : gs.EmitChar (' ');
    
    gs.EmitChar (')');
    snprintf (str1, sizeof(str1), "(M%5d", VU_ID(mateP));
    gs.EmitString (str1);
    vu_getMask (mateP, VU_RULE_EDGE) ? gs.EmitChar ('r') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_GRID_EDGE) ? gs.EmitChar ('g') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_U_SLICE) ? gs.EmitChar ('u') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_V_SLICE) ? gs.EmitChar ('v') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_BOUNDARY_EDGE) ? gs.EmitChar ('b') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_EXTERIOR_EDGE) ? gs.EmitChar ('x') : gs.EmitChar (' ');
    vu_getMask (mateP, VU_NULL_FACE) ? gs.EmitChar ('n') : gs.EmitChar (' ');
    gs.EmitChar (')');

    char str2[1024];
    snprintf (str2, sizeof(str2), " %8d %8d", vu_getUserDataPAsInt (currP),
                (int)vu_getUserData1 (currP));
    gs.EmitString (str2);
    char str3[1024];
    snprintf (str3, sizeof(str3)," (%10lg,%10lg)", VU_U(currP), VU_V(currP));
    gs.EmitString (str3);
    gs.EmitString (" (");

    if (bPrintVertexLoop)
        {
        VU_VERTEX_LOOP (vertP, currP)
            {
            //printf (" %d", VU_ID(vertP));
            gs.EmitInt (VU_ID(vertP));
            gs.EmitChar (' ');
            }
        END_VU_VERTEX_LOOP (vertP, currP)
        }
    //printf (")");
    //printf ("\n");
    gs.EmitString (")");
    gs.EmitLineBreak ();
    }

/*---------------------------------------------------------------------------------**//**
* @description Print statistics for a single face.
* @param graphP     IN  vu graph
* @param faceSeedP  IN  node in face loop
* @group "VU Debugging"
* @see vu_printFaceLabels
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_printFaceLabelsOneFace
(
VuSetP          graphP,
VuP             faceSeedP,
GeomPrintFuncs&     gs
)
    {
    if (nullptr == faceSeedP)
        return;
    VU_FACE_LOOP (currP, faceSeedP)
        {
        vu_printNode (graphP, currP, true, gs);
        }
    END_VU_FACE_LOOP (currP, faceSeedP)
    }

static void emitInt
(
GeomPrintFuncs&     gs,
char const *name,
int value,
bool skipZero = false
)
    {
    if (skipZero && value == 0)
        return;
    gs.EmitString (" (");
    gs.EmitString (name);
    gs.EmitString (" ");
    gs.EmitInt (value);
    gs.EmitString (")");
    }

static void emitDouble
(
GeomPrintFuncs&     gs,
char const *name,
double value,
bool skipZero = false
)
    {
    if (skipZero && value == 0)
        return;
    gs.EmitString (" (");
    gs.EmitString (name);
    gs.EmitString (" ");
    gs.EmitDouble (value);
    gs.EmitString (")");
    }


static void emitFaceTurnData
(
VuSetP          graphP,
VuP             faceSeedP,
GeomPrintFuncs&     gs,
double          smallAngle = 1.0e-8
)
    {
    int numLeft = 0;
    int numRight = 0;
    int numStraight = 0;
    int numReverse = 0;
    int numTotal = 0;

    VU_FACE_LOOP (currP, faceSeedP)
        {
        DVec2d vector0, vector1;
        vu_getDPoint2dDXY (&vector0, currP);
        vu_getDPoint2dDXY (&vector1, vu_fsucc (currP));
        double theta = vector0.AngleTo (vector1);
        numTotal++;
        if (fabs (theta) < smallAngle)
            numStraight++;
        else if (fabs (theta) > Angle::Pi () - smallAngle)
            numReverse++;
        else if (theta > 0.0)
            numLeft++;
        else
            numRight++;
        }
    END_VU_FACE_LOOP (currP, faceSeedP)
    if (numTotal == 2)
        gs.EmitString ("(SLIVER)");
    else
        {
        emitInt (gs, "left", numLeft, true);
        emitInt (gs, "right", numRight, true);
        emitInt (gs, "forward", numStraight, true);
        emitInt (gs, "reverse", numReverse, true);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @description Print statistics for all faces of the graph.
* @param graphP     IN  vu graph
* @param pTitle     IN  text to print at start of report
* @group "VU Debugging"
* @see vu_printFaceLabelsOneFace
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_printFaceLabels
(
VuSetP          graphP,
char const      *pTitle,
GeomPrintFuncs&     gs
)
    {
    int count = 0;
    int numPositive = 0;
    int numNegative = 0;
    int numZero     = 0;
    double area;
    VuMask visitMask = vu_grabMask (graphP);

    if (pTitle)
        {
        //printf (" *********** %s ***************\n", pTitle);
        gs.EmitString (" *********** ");
        gs.EmitString (pTitle);
        gs.EmitString (" ***************");
        gs.EmitLineBreak ();
        }
    vu_clearMaskInSet (graphP, visitMask);
    VU_SET_LOOP (faceSeedP, graphP)
        {
        if (!vu_getMask (faceSeedP, visitMask))
            {
            vu_setMaskAroundFace (faceSeedP, visitMask);
            area = vu_area (faceSeedP);
            /*printf (" (FACE %d) (edges %d) (area %le)\n", count++,
                        vu_faceLoopSize (faceSeedP),
                        area);*/
            emitInt (gs, "FACE", count++);
            emitInt (gs, "edges", vu_faceLoopSize (faceSeedP));
            emitDouble (gs, "area", area);
            emitFaceTurnData (graphP, faceSeedP, gs);
            gs.EmitLineBreak ();
            if (area > 0.0)
                numPositive++;
            else if (area < 0.0)
                numNegative++;
            else
                numZero++;
            vu_printFaceLabelsOneFace (graphP, faceSeedP, gs);
            }
        }
    END_VU_SET_LOOP (faceSeedP, graphP)
    /*printf (" *** (area counts +0- %d %d %d)\n",
                    numPositive, numZero, );*/
    gs.EmitString (" *** (area counts +0- ");
    gs.EmitChar (' ');
    gs.EmitInt (numPositive);
    gs.EmitChar (' ');
    gs.EmitInt (numZero);
    gs.EmitChar (' ');
    gs.EmitInt (numNegative);
    gs.EmitLineBreak ();
    vu_returnMask (graphP, visitMask);
    }


Public GEOMDLLIMPEXP void     vu_printFaceLabels
(
VuSetP          graphP,
char const      *pTitle
)
    {
    GeomPrintToBeConsole printFuncs;
    vu_printFaceLabels (graphP, pTitle, printFuncs);
    }

Public GEOMDLLIMPEXP void     vu_printFaceLabelsOneFace
(
VuSetP          graphP,
VuP             nodeP
)
    {
    GeomPrintToBeConsole printFuncs;
    vu_printFaceLabelsOneFace (graphP, nodeP, printFuncs);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
