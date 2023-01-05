/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/* DO NOT EDIT!  THIS FILE IS GENERATED. */

#pragma once
#include <stdio.h>
#include <float.h>
extern FILE *s_outfile;
class Tag;
/*---------------------------------------------------------------------*//**
@description Initialize error counts.  Process args.
---------------------------------------------------------------------*/
Public bool    initErrorTracking
(
const char *pDefaultOutfileName,
int argc,
char ** argv
);

Public void countTag (char const *pTagName);

/*---------------------------------------------------------------------*//**
@return status for exit from mdl app.
------------------------------------------------------------------------*/
Public StatusInt getExitStatus
(
void
);


/*---------------------------------------------------------------------*//**
@description Select an indexed set of tolerances.
@param index IN 0 for default (near machines precision), 1 for 6 digits.
---------------------------------------------------------------------*/
Public void selectTolerances (int index);

Public void xml_startTag
(
const char *tagName
);

Public void xml_endHeader
(
);

Public void xml_endTag
(
const char *tagName
);

Public void xml_printAttribute
(
const char *pName,
const char *pValue
);

Public void startTag ();

Public void endTag ();

/*---------------------------------------------------------------------*//**
@description 
    Print an OK message and increment the error count.
------------------------------------------------------------------------*/
Public void recordOK
(
const char *pType,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
    Print an error message and increment the error count.
------------------------------------------------------------------------*/
Public void recordERROR
(
const char *pType,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
   Compare the error count to an expected value.
   If it matches, zero out the error count.
   This is to be called just after having made "check" calls which intentionally exercise the
    error handling branches.
------------------------------------------------------------------------*/
Public void confirmCalibration
(
int numExpectedError
);

/*---------------------------------------------------------------------*//**
@description 
Print a bool    with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printBool
(
const char *pName,
const int a0,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print Int with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printInt
(
const char *pName,
const int a0,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**

@description 
------------------------------------------------------------------------*/
Public void printDatum
(
const char *pTagName,
const char *pNameString,
const char *pContentString,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print double with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDouble
(
const char *pName,
const double a0,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDVec2d
(
const char *pName,
const DVec2d *pVec,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print a DVec3d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDVec3d
(
const char *pName,
const DVec3d *pVec,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Compare DPoint3d, xyz points.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDPoint3dXYZ
(
const DPoint3d *pPoint0,
double xx,
double yy,
double zz,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare DVec3d, xyz points.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDVec3dXYZ
(
const DVec3d *pVec0,
double xx,
double yy,
double zz,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two DVec3d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDVec3d
(
const DVec3d *pVec0,
const DVec3d *pVec1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two DVec2d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDVec2d
(
const DVec2d *pVec0,
const DVec2d *pVec1,
const char *pDescr
);


/*---------------------------------------------------------------------*//**
@description 
Print a DPoint3d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPoint3d
(
const char *pName,
const DPoint3d *pVec,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print a DPoint4d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPoint4d
(
const char *pName,
const DPoint4d *pPoint,
Tag *pParent = NULL
);


/*---------------------------------------------------------------------*//**
@description 
Print GraphicsPoint content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printGraphicsPoint
(
const char *pName,
const GraphicsPoint *pGP,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print Ellipse content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDEllipse3d
(
const char *pName,
const DEllipse3d *pEllipse,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print plane content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPlane3d
(
const char *pName,
const DPlane3d *pPlane,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print DSegment3d content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDSegment3d
(
const char *pName,
const DSegment3d *pSegment,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Print a hex int as a tag with optional name attribute
------------------------------------------------------------------------*/
Public void printHex
(
const char *pName,
const int a0,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Compare two DPoint3d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDPoint3d
(
const DPoint3d *pVec0,
const DPoint3d *pVec1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two DPoint2d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDPoint2d
(
const DPoint2d *pVec0,
const DPoint2d *pVec1,
const char *pDescr
);


/*---------------------------------------------------------------------*//**
@description 
Compare two DRange2d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
void checkDRange2d
(
DRange2d const *pRangeA,
DRange2d const *pRangeB,
char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two DPlane3d's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDPlane3d
(
const DPlane3d *pVec0,
const DPlane3d *pVec1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Print a RotMatrix with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printRotMatrix
(
const char *pName,
const RotMatrix *pM,
Tag *pParent = NULL
);

/*---------------------------------------------------------------------*//**
@description 
Compare two RotMatrix's which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkRotMatrix
(
const RotMatrix *pMatrix0,
const RotMatrix *pMatrix1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two doubles which are expected to agree.
@param pDescr IN message string.
@param messageValue IN If this value is anything other than DBL_MAX,
      the pDecr string is used as a format string for sprintf, i.e.
      the message is expanded with sprintf (buffer, pDescr, messageValue)
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkDouble
(
const double a0,
const double a1,
const char *pDescr,
double messageValue = DBL_MAX
);

/*---------------------------------------------------------------------*//**
@description 
Compare two doubles where it is expected that a0 < a1, with toleranced equality case
@param messageValue IN If this value is anything other than DBL_MAX,
      the pDecr string is used as a format string for sprintf, i.e.
      the message is expanded with sprintf (buffer, pDescr, messageValue)
------------------------------------------------------------------------*/
Public void checkDoubleLessThan
(
const double a0,
const double a1,
const char *pDescr,
double messageValue = DBL_MAX
);

/*---------------------------------------------------------------------*//**
@description 
Compare two BoolInts which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkBool
(
const bool    a0,
const bool    a1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Compare two bools which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkBool
(
const bool a0,
const bool a1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Test a bool    expected to be true.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkTrue
(
const bool    a0,
const char *pDescr,
double formatValue = DBL_MAX
);

/*---------------------------------------------------------------------*//**
@description 
Test a bool    expected to be false
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkFalse
(
const bool    a0,
const char *pDescr,
double formatValue = DBL_MAX
);


/*---------------------------------------------------------------------*//**
@description 
Compare two ints which are expected to agree.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkInt
(
const int a0,
const int a1,
const char *pDescr
);

/*---------------------------------------------------------------------*//**
@description 
Test if an int is positive.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkInt
(
const int a,
const char *pDescr
);


/*---------------------------------------------------------------------*//**
@description 
    Output counts of OK, error messages.
    Optionally zero the counts.
------------------------------------------------------------------------*/
Public void showErrors
(
bool    bClear
);

static char * sNamespace = "xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"";

/// <summary>Emit cg xml</summary>
class CGWriter
{
private:
FILE *mpFile;
int mIndent;
public:
CGWriter (FILE *pFile)
    {
    mpFile = pFile;
    mIndent = 0;
    }

void emitXYZ (char *pName, double x, double y, double z)
    {
    emitIndent ();
    fprintf (mpFile, "<%s>%.15g,%.15g,%.15g</%s>", pName, x, y, z, pName);
    }

void emit (char *pName, DPoint3dCR point)
    {
    emitXYZ (pName, point.x, point.y, point.z);
    }
    
void emit (char *pName, DVec3dCR vector)
    {
    emitXYZ (pName, vector.x, vector.y, vector.z);
    }

void emitDouble (char *pName, double x)
    {
    emitIndent ();
    fprintf (mpFile, "<%s>%.15g</%s>", pName, x, pName);
    }

void emitInt (char *pName, int x)
    {
    emitIndent ();
    fprintf (mpFile, "<%s>%d</%s>", pName, x, pName);
    }


void emitIndent ()
    {
    fprintf (mpFile, "\n");
    for (int i = 0; i < mIndent; i++)
        fprintf (mpFile, " ");
    }
    
void emitStart (char *pName, bool newLine = true)
    {
    if (newLine)
        emitIndent ();
    if (mIndent == 0)
        fprintf (mpFile, "<%s %s>", pName, sNamespace);
    else
        fprintf (mpFile, "<%s>", pName);
    mIndent++;
    }

void emitEnd (char *pName, bool newLine = true)
    {
    mIndent--;
    if (newLine)
        emitIndent ();
    fprintf (mpFile, "</%s>", pName);
    if (mIndent == 0)
        fprintf (mpFile, "\n");
    }

void listOfPoint (char *pName, char *pListName, char *pXYZName, DPoint3dCP pXYZ, int n)
    {
    while (n > 0 && pXYZ[n-1].isDisconnect ())
        n--;
    int m = 0;
    for (int i = 0; i <= n; i++)
        {
        if (i == n || pXYZ[i].isDisconnect ())
            {
            if (m > 0)
                {
                emitEnd (pListName);
                emitEnd (pName);
                }
             m = 0;
            }
        else
            {
            if (m == 0)
                {
                emitStart (pName);
                emitStart (pListName);
                }
            emitXYZ (pXYZName, pXYZ[i].x, pXYZ[i].y, pXYZ[i].z);
            m++;
            }
        }
    }


void linestring (DPoint3dCP pXYZ, int n)
    {
    listOfPoint ("LineString", "ListOfPoint", "xyz", pXYZ, n);
    }

void polygon (DPoint3dCP pXYZ, int n)
    {
    listOfPoint ("Polygon", "ListOfPoint", "xyz", pXYZ, n);
    }

void circle (DPoint3dCR xyz, double radius)
    {
    emitStart ("CircularDisk");
    emitStart ("placement");
    emitXYZ ("origin", xyz.x, xyz.y, xyz.z);
    emitXYZ ("vectorZ", 0,0,1);
    emitXYZ ("vectorX", 1,0,0);
    emitEnd ("placement");
    emitDouble ("radius", radius);
    emitEnd ("CircularDisk");
    
    }

void ellipse (DEllipse3d ellipse0)
    {
    DEllipse3d ellipse;
    ellipse.initWithPerpendicularAxes (&ellipse0);
    DVec3d unitX, unitZ;
    unitZ.normalizedCrossProduct (&ellipse.vector0, &ellipse.vector90);
    unitX.normalize (&ellipse.vector0);
    emitStart ("EllipticArc");
    emitStart ("placement");
    emit ("origin", ellipse.center);
    emit ("vectorZ", unitZ);
    emit ("vectorX", unitX);
    emitEnd ("placement");
    emitDouble ("radiusA", ellipse.vector0.magnitude ());
    emitDouble ("radiusB", ellipse.vector90.magnitude ());
    emitDouble ("startAngle", bsiTrig_radiansToDegrees (ellipse.start));
    emitDouble ("sweepAngle", bsiTrig_radiansToDegrees (ellipse.sweep));
    emitEnd ("EllipticArc");    
    }


void bezier (DPoint4dCP pX, int order)
    {
    
    emitStart ("BsplineCurve");
    emitInt ("order", order);
    emitStart ("ListOfControlPoint");
    for (int i = 0; i < order; i++)
        emitXYZ ("xyz", pX[i].x, pX[i].y, pX[i].z);
    emitEnd   ("ListOfControlPoint");
    
    emitStart ("ListOfWeight");
    for (int i = 0; i < order; i++)
        emitDouble ("weight", pX[i].w);    
    emitEnd   ("ListOfWeight");
    
    emitEnd ("BsplineCurve");
    
    }


void coordinate (DPoint3dCR xyz)
    {
    emitStart ("Coordinate");
    emitXYZ ("xyz", xyz.x, xyz.y, xyz.z);
    emitEnd ("Coordinate");
    }

};

class Tag
{
char mTagName[256];
int mState;
int mFormat;
int mDepth;
Tag *mpParent;

public:

Tag (char const *pTagName)
    {
    countTag (pTagName);
    strcpy (mTagName, pTagName);
    fprintf (s_outfile, "<%s", mTagName);
    mState = 0;
    mFormat = 0;
    mpParent = NULL;
    }
    
Tag (char const *pTagName, char const *pNameAttr)
    {
    countTag (pTagName);
    strcpy (mTagName, pTagName);
    fprintf (s_outfile, "<%s", mTagName);
    Attr ("name", pNameAttr);
    mState = 0;
    mFormat = 0;
    mpParent = NULL;
    }

Tag (Tag *pParent, char const *pTagName, char const *pNameAttr)
    {
    countTag (pTagName);
    if (pTagName)
        strcpy (mTagName, pTagName);
    else
        strcpy (mTagName, "");
    mFormat = 1;
    mpParent = pParent;
    if (pParent)
        pParent->StartContentItem ();
    if (pTagName)
        {
        mState = 0;
        fprintf (s_outfile, "<%s", mTagName);
        }
    else
        mState = 1;
    Attr ("name", pNameAttr);
    }

~Tag ()
    {
    if (mpParent)
        mpParent->EndContentItem();
    else
        EndContentItem ();

    if (strlen (mTagName) <= 1)
        {
        //fprintf (s_outfile, "\n");
        }
    else if (mState == 0)
        fprintf (s_outfile, "/>");
    else
        fprintf (s_outfile, "</%s>", mTagName);
    if (GetDepth () <= 1)
        fprintf (s_outfile, "\n");
    }


int GetDepth ()
    {
    if (mpParent)
        return mpParent->GetDepth () + 1;
    else
        return 1;
    }

void SetParent (Tag *pParent = NULL)
    {
    mpParent = pParent;
    }
void Attr (char const *pName, char const *pValue)
    {
    if (pName && pValue)
        fprintf (s_outfile, " %s=\"%s\"", pName, pValue);
    }

void EndHeader ()
    {
    if (mState == 0)
        {
        fprintf (s_outfile, ">");
        mState = 1;
        }
    }

void Advance ()
    {
    if (mFormat > 0)
        {
        int depth = GetDepth ();
        fprintf (s_outfile, "\n");
        while (depth-- > 0)
            fprintf(s_outfile, "  ");            
        }
    }
void StartContentItem ()
    {
    EndHeader ();
    Advance ();
    }

void EndContentItem ()
    {
    Advance();
    }

void SetFormat (int format)
    {
    mFormat = format;
    }

void EmitCompleteTag (char const * pTagName, char const *pNameString, char const *pContentString)
    {
    StartContentItem ();
    fprintf (s_outfile, "<%s", pTagName);
    if (pNameString)
        fprintf (s_outfile, " name=\"%s\"", pNameString);
    if (pContentString)
        fprintf (s_outfile, ">%s</%s>", pContentString, pTagName);
    else
        fprintf (s_outfile, "/>");    
    }


void Content (char const *pName, DPoint3d const *pPoint)
    {
    EmitCompleteTag("DPoint3d", pName, Utf8PrintfString("%lg,%lg,%lg", pPoint->x, pPoint->y, pPoint->z).c_str());
    }

void Content (char const *pName, DVec3d const *pVec)
    {
    EmitCompleteTag("DVec3d", pName, Utf8PrintfString("%lg,%lg,%lg", pVec->x, pVec->y, pVec->z).c_str());
    }

void Content (char const *pName, DPoint2d const *pData)
    {
    EmitCompleteTag("DPoint2d", pName, Utf8PrintfString("%lg,%lg", pData->x, pData->y).c_str());
    }

void Content (char const *pName, RotMatrix const *pM)
    {
    EmitCompleteTag ("row0", pName, Utf8PrintfString("%lg,%lg,%lg", pM->form3d[0][0], pM->form3d[0][1], pM->form3d[0][2]).c_str());
    EmitCompleteTag ("row1", pName, Utf8PrintfString("%lg,%lg,%lg", pM->form3d[1][0], pM->form3d[1][1], pM->form3d[1][2]).c_str());
    EmitCompleteTag ("row2", pName, Utf8PrintfString("%lg,%lg,%lg]", pM->form3d[2][0], pM->form3d[2][1], pM->form3d[2][2]).c_str());
    }

void Content (char const *pName, DPoint4d const *pPoint)
    {
    EmitCompleteTag("DPoint4d", pName, Utf8PrintfString("%lg,%lg,%lg", pPoint->x, pPoint->y, pPoint->z, pPoint->w).c_str());
    }

void Content (char const *pName, GraphicsPoint const *pGP)
    {
    Tag childTag (this, "GraphicsPoint", pName);
    childTag.Content (NULL, &pGP->point);
    childTag.Content ("u", pGP->userData);
    childTag.Content ("a", pGP->a);
    childTag.Content ("m", pGP->mask);
    }


void Content (char const *pName, DEllipse3d const *pEllipse)
    {
    Tag childTag (this, "DEllipse3d", pName);
    childTag.Content ("C", &pEllipse->center);
    childTag.Content ("V", &pEllipse->vector90);
    childTag.Content ("start", pEllipse->start);
    childTag.Content ("sweep", pEllipse->sweep);
    }

void Content (char const *pName, DRange2d const *pData)
    {
    if (pData->isNull ())
        {
        EmitCompleteTag ("DRange2d::NULL", NULL, NULL);
        }
    else
        {
        Tag childTag (this, "DRange2d", pName);
        childTag.Content ("low", &pData->low);
        childTag.Content ("high", &pData->high);
        }
    }


void Content (char const *pName, DPlane3d const *pPlane)
    {
    Tag childTag (this, "DPlane3d", pName);
    childTag.Content ("P", &pPlane->origin);
    childTag.Content ("N", &pPlane->normal);
    }

void Content (char const *pName, DSegment3d const *pSegment)
    {
    Tag childTag (this, "DSegment3d", pName);
    childTag.Content ("Start", &pSegment->point[0]);
    childTag.Content ("End",   &pSegment->point[1]);
    }

void ContentBool (char const *pName, bool    a)
    {
    EmitCompleteTag (a ? "bool::true" : "bool::false", pName, NULL);
    }

void ContentBool (char const *pName, bool a)
    {
    EmitCompleteTag (a ? "bool::true" : "bool::false", pName, NULL);
    }

void Content (double a)
    {
    Content (NULL, a);
    }

void ContentHex (char const *pName, int a)
    {
    EmitCompleteTag ("hex", pName, Utf8PrintfString("0x%x", a).c_str());
    }

void Content (char const *pName, double a)
    {
    EmitCompleteTag ("double", pName, Utf8PrintfString("%20.15lg", a).c_str());
    }

void Content (DPoint3d const *pPoint)
    {
    Content (NULL, pPoint);
    }

void Content (char const *pName, int a)
    {
    EmitCompleteTag ("int", pName, Utf8PrintfString("%d", a).c_str());
    }

void Content (char const *pName, bool a)
    {
    EmitCompleteTag ("bool", pName, Utf8PrintfString("%s", a ? "true" : "false").c_str());
    }

void Content (int a)
    {
    Content (NULL, a);
    }

void Content (bool a)
    {
    Content (NULL, a);
    }

};

struct DGNKeyins
{

void EmitColor (int value){printf ("ACTIVE COLOR %d\n", value);}
void EmitStyle (int value){printf ("ACTIVE STYLE %d\n", value);}
void EmitWeight (int value){printf ("ACTIVE WEIGHT %d\n", value);}

void EmitText (char *pString, double x, double y)
    {
    }

void EmitXY (double x, double y)
    {
    printf ("xy=%g,%g\n", x, y);
    }

void EmitXY (DPoint3dCR xyz)
    {
    printf ("xy=%g,%g\n", xyz.x, xyz.y);
    }

void EmitLine (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    StartPolyline ();
    EmitXY (xyz0);
    EmitXY (xyz1);
    EndPolyline ();
    }


void StartPolyline (){printf ("PLACE SMARTLINE\n");}
void EndPolyline   (){printf ("PLACE SMARTLINE\n");}

void Emit (double x0, double y0, double a, double b)
    {
    StartPolyline ();
    EmitXY (x0, y0);
    EmitXY (x0 + a, y0);
    EmitXY (x0 + a, y0 + b);
    EmitXY (x0, y0 + b);
    EmitXY (x0, y0);
    EndPolyline ();
    }
};


struct CGXmlWriter
{
CGXmlWriter ()
    {
    mDepth = 0;
    }

private: int mDepth;
void EmitIndent (int change)
    {
    if (change < 0)
        mDepth--;
    for (int i = 0; i < mDepth; i++)
        printf ("  ");
    if (change > 0)
        mDepth++;
    }
public: 
void EmitColor (int value){}
void EmitStyle (int value){}
void EmitWeight (int value){}

void EmitText (char *pString, double x, double y)
    {
    }

void EmitXY (double x, double y)
    {
    printf ("  <xyz>%g,%g</xyz>\n", x, y);
    }

void EmitXY (DPoint3dCR xyz)
    {
    printf ("  <xyz>%g,%g,%g</xyz>\n", xyz.x, xyz.y, xyz.z);
    }

void EmitDPoint3d (char *pName, DPoint3dCR xyz)
    {
    printf ("  <%s>%g,%g,%g</%s>\n", pName, xyz.x, xyz.y, xyz.z, pName);
    }

void EmitDVec3d(char *pName, DVec3d xyz)
    {
    printf ("  <%s>%g,%g,%g</%s>\n", pName, xyz.x, xyz.y, xyz.z, pName);
    }

void EmitDouble (char *pName, double a)
    {
    printf ("  <%s>%g</%s>\n", pName, a, pName);
    }

void EmitRadians (char *pName, double a)
    {
    printf ("  <%s>%g</%s>\n", pName, a * 45.0 / atan (1.0), pName);
    }



void StartTag (char * pName, bool includeNamespace = false)
    {
    EmitIndent (1);
    printf ("<%s", pName);
    if (includeNamespace)
        printf(" xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"");
    printf (">\n");
    }

void EndTag (char * pName, bool includeNamespace = false)
    {
    EmitIndent (-1);
    printf ("</%s>\n", pName);
    }

void EmitLine (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    StartTag("LineSegment", true);
    EmitDPoint3d ("startPoint", xyz0);
    EmitDPoint3d ("endPoint", xyz1);
    EndTag ("LineSegment");
    }


void EmitPlacement (DPoint3dCR origin, RotMatrixCR axes)
    {
    DVec3d xAxis, zAxis;
    axes.getColumn (&xAxis, 0);
    axes.getColumn (&zAxis, 2);
    StartTag ("placement", false);
    EmitDPoint3d ("origin", origin);
    EmitDVec3d ("vectorZ", zAxis);
    EmitDVec3d ("vectorX", xAxis);
    EndTag ("placement");
    }

void EmitEllipticArc (DEllipse3dCR ellipse)
    {
    RotMatrix axes;
    DPoint3d center;
    double rA, rB;
    double startRadians, sweepRadians;
    ellipse.getScaledRotMatrix (&center, &axes, &rA, &rB, &startRadians, &sweepRadians);
    StartTag("EllipticArc", true);
    EmitPlacement (center, axes);
    EmitDouble("radiusA", rA);
    EmitDouble("radiusB", rB);
    EmitRadians ("startAngle", startRadians);
    EmitRadians ("sweepAngle", sweepRadians);
    EndTag ("EllipticArc");
    }


void EmitCircleXY (DPoint3dCR center, double radius)
    {
    DEllipse3d ellipse;
    ellipse.center = center;
    ellipse.init (center.x, center.y, center.z,
                radius, 0, 0,
                0, radius, 0,
                0.0, msGeomConst_2pi
                );
    EmitEllipticArc (ellipse);
    }

void StartPolyline (){StartTag ("LineString", true); StartTag("ListOfPoint");}
void EndPolyline   (){EndTag ("ListOfPoint");  EndTag ("LineString");}

void EmitLinestring (DPoint3dCP xyz, int n)
    {
    StartPolyline ();
    for (int i = 0; i < n; i++)
        EmitDPoint3d ("xyz", xyz[i]);
    EndPolyline ();
    }

void EmitDCone3d (DCone3dR cone)
    {
    DSegment3d segment;
    DEllipse3d ellipseA, ellipseB;
    bsiDCone3d_getCrossSection (&cone, &ellipseA, 0.0);
    bsiDCone3d_getCrossSection (&cone, &ellipseB, 1.0);

    EmitEllipticArc (ellipseA);
    EmitEllipticArc (ellipseB);

    double dTheta = 0.5 * atan (1.0);
    for (double theta = 0.0; theta < 6.28; theta += dTheta)
        {
        bsiDCone3d_getRuleLine (&cone, &segment, theta);
        EmitLine (segment.point[0], segment.point[1]);
        }
    }

void EmitBezier (DPoint4dCP pXYZW, int order)
    {
    int n = order;
    if (order > 2)
        n += 6 + 3 * (order - 3);
    if (n > 100)
        n = 100;
    double param[101];
    for (int i = 0; i <= n; i++)
        param[i] = (double)i / (double)(n);
    DPoint3d xyz[101];
    bsiBezierDPoint4d_evaluateDPoint3dArrayExt
        (xyz, NULL, NULL, pXYZW, order, param, n + 1);
    EmitLinestring (xyz, n+1);
    }


void EmitPoints (DPoint3dCP xyz, int n)
    {
    for (int i = 0; i < n; i++)
        EmitCircleXY (xyz[i], 0.02);
//        EmitLine (xyz[i], xyz[i]);
    }

void Emit (double x0, double y0, double a, double b)
    {
    StartPolyline ();
    EmitXY (x0, y0);
    EmitXY (x0 + a, y0);
    EmitXY (x0 + a, y0 + b);
    EmitXY (x0, y0 + b);
    EmitXY (x0, y0);
    EndPolyline ();
    }
};