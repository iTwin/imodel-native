/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <Geom/GeomApi.h>
#include <printfuncs.h>

#include <math.h>
#include <string.h>

FILE *s_outfile;
typedef struct
    {
    double absTol;
    double relTol;

    double GetTol (double s)
        {
        return absTol + s * relTol;
        }
    } Tolerances;

#define NUM_TOLERANCE 4
static Tolerances sToleranceTable[NUM_TOLERANCE] =
    {
        {5.0e-15, 1.0e-15},
        {5.0e-6,  1.0e-6},
        {5.0e-14,  1.0e-14},
        {5.0e-12,  1.0e-12},
    }; 
static Tolerances sTolerances = {5.0e-15, 1.0e-15};

static int s_numOK = 0;
static int s_numERR = 0;
static int s_numOTHER = 0;
static char *ssOK = "OK";
static char *ssERROR = "ERROR";

/*---------------------------------------------------------------------*//**
@description Select an indexed set of tolerances.
@param index IN 0 for default (near machines precision), 1 for 6 digits.
---------------------------------------------------------------------*/
Public void selectTolerances (int index)
    {
    if (index < 0 || index >= NUM_TOLERANCE)
        index = 0;
    sTolerances = sToleranceTable[index];
    }

/*---------------------------------------------------------------------*//**
@description Count error tags.
---------------------------------------------------------------------*/
Public void countTag (char const *pTagName)
    {
    if (NULL == pTagName)
        {
        }
    else if (strcmp (pTagName, ssOK) == 0)
        s_numOK++;
    else if (strcmp (pTagName, ssERROR) == 0)
        s_numERR++;
    else
        s_numOTHER++;
    }

/*---------------------------------------------------------------------*//**
@description Initialize error counts.  Process args.
---------------------------------------------------------------------*/
Public bool    initErrorTracking
(
const char *pDefaultOutfileName,
int argc,
char ** argv
)
    {
    int i, iFirst;
    int showArgs = 0;
    int optionsPrinted = 0;
    selectTolerances (0);
    const char *pOutfileName = pDefaultOutfileName;
    if (argc > 1)
        printf (" geomlibs test environment -- command line arguments:\n");
    // In arg 0, only print what is past the last backslash
    int kLast = -1;
    for (int k = 0; argv[0][k] != 0; k++)
        if (argv[0][k] == '\\')
            kLast = k;

    if (argc > 1)
        printf (" Tail of argv[0] = %s\n", argv[0] + kLast + 1);
    for (i = 1; i < argc; i++)
            printf (" argv[%d] = %s\n", i, argv[i]);


    if (argc > 1 && 0 == strcmp (argv[1], "USER"))
        {
	// We started from inside microstation
	iFirst = 2;
	}
    else if (argc > 1 && 0 == strcmp (argv[1], "MS_INITAPPS"))
        {
	// Running from a console
	iFirst = 4;
	}
    else
        {
	iFirst = 1;
	}

    for (i = iFirst; i < argc; i++)
        {
	if (0 == strcmp (argv[i], "@debug"))
            {
	    //printf ("dvec3dtest -- debug request from command line \n");
	    //mdlSystem_enterDebug ();
	    }
	else if (i+1 < argc && 0 == strcmp (argv[i], "@outfile"))
            {
	    pOutfileName = argv[i+1];
	    i++;
	    }
	else
            {
	    if (!optionsPrinted)
                {
                printf (" Options:\n");
                //printf ("     @debug --- start mdl console window debugger\n");
                printf ("     @outfile filename --- name for output file\n");
		optionsPrinted = 1;
		}
	    showArgs = 1;
	    //mdlSystem_enterDebug ();
	    }
	if (showArgs)
            printf (" argv[%d] = %s\n", i, argv[i]);
	}

    if (pOutfileName)
        {
        printf (" printfuncs: opening output file %s\n", pOutfileName);

        if (NULL == (s_outfile = fopen (pOutfileName, "w")))
            {
            printf ("  ERROR -- dvec3dtest.ma unable to open output file %s\n", pOutfileName);
	    s_numERR++;
	    }
	}
    else
        s_outfile = stdout;

    return NULL != s_outfile;
    }

/*---------------------------------------------------------------------*//**
@return status for exit from mdl app.
------------------------------------------------------------------------*/
Public StatusInt getExitStatus
(
void
)
    {
    printf ("<Exit>%d errors, %d OK</Exit>\n", s_numERR, s_numOK);
    return s_numERR == 0 ? SUCCESS : ERROR;
    }

Public void xml_startTag
(
const char *tagName
)
    {
    fprintf (s_outfile, "<%s", tagName);
    }

Public void xml_endHeader
(
)
    {
    fprintf (s_outfile, ">\n");
    }

Public void xml_endTag
(
const char *tagName
)
    {
    fprintf (s_outfile, "</%s>\n", tagName);
    }

Public void xml_printAttribute
(
const char *pName,
const char *pValue
)
    {
    fprintf (s_outfile, "\n\t\t%s=\"%s\"", pName, pValue);
    }

static char *s_pName = "GenericReport";
Public void startTag ()
    {
    xml_startTag (s_pName);
    }

Public void endTag ()
    {
    xml_endTag (s_pName);
    }




/*---------------------------------------------------------------------*//**
@description 
    Print an OK message and increment the error count.
------------------------------------------------------------------------*/
Public void startOK
(
const char *pType,
const char *pDescr
)
    {
    s_numOK++;
    xml_startTag (ssOK);
    xml_printAttribute ("testName",pDescr);
    xml_endHeader ();
    }


/*---------------------------------------------------------------------*//**
@description 
    Print an error message and increment the error count.
------------------------------------------------------------------------*/
Public void startERROR
(
const char *pType,
const char *pDescr
)
    {
    s_numERR++;
    xml_startTag (ssERROR);
    xml_printAttribute ("testName",pDescr);
    xml_endHeader ();
    }

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
)
    {
    startTag ();
    if (s_numERR == numExpectedError)
        {
	startOK ("", "Intentional errors counted correctly!!!\n");
	s_numERR = 0;
	}
    else
        {
	startERROR ("", "CALIBRATION FAILED -- Intentional errors were not properly counted");
	}
    endTag ();
    }



/*---------------------------------------------------------------------*//**

@description 
------------------------------------------------------------------------*/
Public void printDatum
(
const char *pTagName,
const char *pNameString,
const char *pContentString,
Tag *pParent
)
    {
    if (pParent)
        pParent->EmitCompleteTag (pTagName, pNameString, pContentString);
    else
        {
        fprintf (s_outfile, "<%s", pTagName);
        if (pNameString)
            fprintf (s_outfile, " name=\"%s\"", pNameString);
        if (pContentString)
            fprintf (s_outfile, ">%s</%s>\n", pContentString, pTagName);
        else
            fprintf (s_outfile, "/>\n");
        }
    }


/*---------------------------------------------------------------------*//**
@description 
Print a double as a tag with optional name attribute
------------------------------------------------------------------------*/
void printDouble
(
const char *pName,
const double a0,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg", a0);
    printDatum ("double", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print an int as a tag with optional name attribute
------------------------------------------------------------------------*/
Public void printInt
(
const char *pName,
const int a0,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%d", a0);
    printDatum ("int", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a hex int as a tag with optional name attribute
------------------------------------------------------------------------*/
Public void printHex
(
const char *pName,
const int a0,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "0x%x", a0);
    printDatum ("hex", pName, stringVal, pParent);
    }


/*---------------------------------------------------------------------*//**
@description 
Print a bool    with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printBool
(
const char *pName,
const int a0,
Tag *pParent
)
    {
    printDatum (a0 ? "bool::true" : "bool::false", pName, NULL, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a DVec3d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDVec3d
(
const char *pName,
const DVec3d *pVec,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg,%lg,%lg", pVec->x, pVec->y, pVec->z);
    printDatum ("DVec3d", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a DVecd with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDVec2d
(
const char *pName,
const DVec2d *pVec,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg,%lg", pVec->x, pVec->y);
    printDatum ("DVec2d", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a DVec3d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPoint3d
(
const char *pName,
const DPoint3d *pVec,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg,%lg,%lg", pVec->x, pVec->y, pVec->z);
    printDatum ("DPoint3d", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a DVec3d with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPoint4d
(
const char *pName,
const DPoint4d *pVec,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg,%lg,%lg,%lg", pVec->x, pVec->y, pVec->z, pVec->w);
    printDatum ("DPoint4d", pName, stringVal, pParent);
    }


/*---------------------------------------------------------------------*//**
@description 
Compare two DVec3d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDVec3d
(
const DVec3d *pVec0,
const DVec3d *pVec1,
const char *pDescr
)
    {
    double diff = bsiDVec3d_maxAbsDifference (pVec0, pVec1);
    double s  = bsiDVec3d_maxAbs (pVec0) + bsiDVec3d_maxAbs(pVec1);

    if (diff < sTolerances.GetTol (s))
        {
        Tag thisTag(ssOK, pDescr);
	    printDVec3d (NULL, pVec0);
	    }
    else
        {
        Tag thisTag(ssERROR, pDescr);
        thisTag.SetFormat (1);
	    printDVec3d ("vec0", pVec0);
	    printDVec3d ("vec1", pVec1);
	    printDouble ("diff", diff);
	    }
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two DVec2d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDVec2d
(
const DVec2d *pVec0,
const DVec2d *pVec1,
const char *pDescr
)
    {
    double diff = bsiDVec2d_maxAbsDifference (pVec0, pVec1);
    double s  = bsiDVec2d_maxAbs (pVec0) + bsiDVec2d_maxAbs(pVec1);

    if (diff < sTolerances.GetTol (s))
        {
        Tag thisTag(ssOK, pDescr);
	    printDVec2d (NULL, pVec0);
	    }
    else
        {
        Tag thisTag(ssERROR, pDescr);
        thisTag.SetFormat (1);
	    printDVec2d ("vec0", pVec0);
	    printDVec2d ("vec1", pVec1);
	    printDouble ("diff", diff);
	    }
    }


/*---------------------------------------------------------------------*//**
@description 
Compare two DPoint3d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDPoint3d
(
const DPoint3d *pVec0,
const DPoint3d *pVec1,
const char *pDescr
)
    {
    double diff = bsiDPoint3d_maxAbsDifference (pVec0, pVec1);
    double s  = bsiDPoint3d_maxAbs (pVec0) + bsiDPoint3d_maxAbs(pVec1);

    if (diff < sTolerances.GetTol (s))
        {
	    Tag thisTag (ssOK, pDescr);
	    thisTag.Content (pVec0);
	    }
    else
        {
	    Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
	    thisTag.Content (pVec0);
	    thisTag.Content (pVec1);
	    printDouble ("diff", diff);
	    }
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two DPoint2d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDPoint2d
(
DPoint2d const *pVec0,
DPoint2d const *pVec1,
const char *pDescr
)
    {
    double diff = bsiDPoint2d_distance (pVec0, pVec1);
    double s  = bsiDPoint2d_maxAbs (pVec0) + bsiDPoint2d_maxAbs(pVec1);

    if (diff < sTolerances.GetTol (s))
        {
	    Tag thisTag (ssOK, pDescr);
	    thisTag.Content (NULL, pVec0);
	    }
    else
        {
	    Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
	    thisTag.Content (NULL, pVec0);
	    thisTag.Content (NULL, pVec1);
	    printDouble ("diff", diff);
	    }
    }

void checkDRange2d
(
DRange2d const *pRangeA,
DRange2d const *pRangeB,
char *pDescr
)
    {
    bool bSame = false;
    if (pRangeA->isNull () && pRangeB->isNull ())
        {
	    Tag thisTag (ssOK, pDescr);
	    thisTag.Content (NULL, pRangeA);
        }
    else if (pRangeA->isNull () || pRangeB->isNull ())
        {
	    Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
	    thisTag.Content (NULL, pRangeA);
	    thisTag.Content (NULL, pRangeB);
        }
    else
        {
        double diff1 = bsiDPoint2d_distance(&pRangeA->low, &pRangeB->low);
        double diff2 = bsiDPoint2d_distance(&pRangeA->high, &pRangeB->high);
        double s  = bsiDPoint2d_maxAbs (&pRangeA->low )
                  + bsiDPoint2d_maxAbs( &pRangeA->high)
                  + bsiDPoint2d_maxAbs( &pRangeB->low )
                  + bsiDPoint2d_maxAbs( &pRangeB->high);

        if (diff1 + diff2 < sTolerances.GetTol (s))
            {
	        Tag thisTag (ssOK, pDescr);
	        thisTag.Content (NULL, pRangeA);
            }
        else
            {
	        Tag thisTag (ssERROR, pDescr);
            thisTag.SetFormat (1);
	        thisTag.Content (NULL, pRangeA);
	        thisTag.Content (NULL, pRangeB);
            }
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two DPlane3d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDPlane3d
(
const DPlane3d *pPlane0,
const DPlane3d *pPlane1,
const char *pDescr
)
    {
    char descr[2048];
    sprintf (descr, "(DPlane3d.origin)%s", pDescr);
    checkDPoint3d (&pPlane0->origin, &pPlane1->origin, descr);
    sprintf (descr, "(DPlane3d.normal)%s", pDescr);
    checkDVec3d (&pPlane0->normal, &pPlane1->normal, descr);
    }

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
)
    {
    DPoint3d point1;
    bsiDPoint3d_setXYZ (&point1, xx, yy, zz);
    checkDPoint3d (pPoint0, &point1, pDescr);
    }

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
)
    {
    DVec3d vec1;
    bsiDVec3d_setXYZ (&vec1, xx, yy, zz);
    checkDVec3d (pVec0, &vec1, pDescr);
    }

/*---------------------------------------------------------------------*//**
@description 
Print a RotMatrix with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printRotMatrix
(
const char *pName,
const RotMatrix *pM,
Tag *pParent
)
    {
    char stringVal[1024];
    sprintf (stringVal, "%lg,%lg,%lg", pM->form3d[0][0], pM->form3d[0][1], pM->form3d[0][2]);
    printDatum ("row0", pName, stringVal, pParent);
    sprintf (stringVal, "%lg,%lg,%lg", pM->form3d[1][0], pM->form3d[1][1], pM->form3d[1][2]);
    printDatum ("row1", pName, stringVal, pParent);
    sprintf (stringVal, "%lg,%lg,%lg]", pM->form3d[2][0], pM->form3d[2][1], pM->form3d[2][2]);
    printDatum ("row2", pName, stringVal, pParent);
    }

/*---------------------------------------------------------------------*//**
@description 
Print GraphicsPoint content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printGraphicsPoint
(
const char *pName,
const GraphicsPoint *pData,
Tag *pParent
)
    {
    if (pParent)
        pParent->Content (pName, pData);
    else
        {
        Tag myTag = Tag (NULL, NULL, NULL);
        myTag.Content (pName, pData);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Print Ellipse content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDEllipse3d
(
const char *pName,
const DEllipse3d *pData,
Tag *pParent
)
    {
    if (pParent)
        pParent->Content (pName, pData);
    else
        {
        Tag myTag = Tag (NULL, NULL, NULL);
        myTag.Content (pName, pData);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Print Ellipse content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDRange2d
(
const char *pName,
const DRange2d *pData,
Tag *pParent
)
    {
    if (pParent)
        pParent->Content (pName, pData);
    else
        {
        Tag myTag = Tag (NULL, NULL, NULL);
        myTag.Content (pName, pData);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Print plane content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDPlane3d
(
const char *pName,
const DPlane3d *pData,
Tag *pParent
)
    {
    if (pParent)
        pParent->Content (pName, pData);
    else
        {
        Tag myTag = Tag (NULL, NULL, NULL);
        myTag.Content (pName, pData);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Print DSegment3d content with prefix and suffix strings.
------------------------------------------------------------------------*/
Public void printDSegment3d
(
const char *pName,
const DSegment3d *pData,
Tag *pParent
)
    {
    if (pParent)
        pParent->Content (pName, pData);
    else
        {
        Tag myTag = Tag (NULL, NULL, NULL);
        myTag.Content (pName, pData);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two DVec3d's which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkRotMatrix
(
const RotMatrix *pMatrix0,
const RotMatrix *pMatrix1,
const char *pDescr
)
    {
    double diff = bsiRotMatrix_maxDiff (pMatrix0, pMatrix1);
    double s  = bsiRotMatrix_maxAbs (pMatrix0) + bsiRotMatrix_maxAbs(pMatrix1);

    if (diff < sTolerances.GetTol (s))
        {
	Tag thisTag (ssOK, pDescr);
	printRotMatrix (NULL, pMatrix0);
	}
    else
        {
	Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
	printRotMatrix ("matrix0", pMatrix0);
	printRotMatrix ("matrix1", pMatrix1);
	printDouble ("diff", diff);
	}
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two doubles where it is expected that a0 < a1, with toleranced equality case
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDoubleLessThan
(
const double a0,
const double a1,
const char *pDescr,
double messageValue
)
    {
    if (messageValue != DBL_MAX)
        {
        char message[2048];
        sprintf (message, pDescr, messageValue);
        checkDoubleLessThan (a0, a1, message);
        return;
        }
    double diff = fabs (a0 - a1);
    double s  = fabs (a0) + fabs (a1);

    if (a0 + sTolerances.GetTol (s) < a1)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.Content (a0);
        thisTag.Content (a1);
	}
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.Content (a0);
        thisTag.Content (a1);
        thisTag.Content ("LT.diff", diff);
	}
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two doubles which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkDouble
(
const double a0,
const double a1,
const char *pDescr,
double messageValue
)
    {
    if (messageValue != DBL_MAX)
        {
        char message[2048];
        sprintf (message, pDescr, messageValue);
        checkDouble (a0, a1, message);
        return;
        }
    double diff = fabs (a0 - a1);
    double s  = fabs (a0) + fabs (a1);

    if (diff < sTolerances.GetTol (s))
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.Content (a0);
	    }
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.Content (a0);
        thisTag.Content (a1);
        thisTag.Content ("diff", diff);
	    }
    }

/*---------------------------------------------------------------------*//**
@description 
Compare two BoolInts which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkBool
(
const bool    a0,
const bool    a1,
const char *pDescr
)
    {
    int i0 = a0 ? 1 : 0;
    int i1 = a1 ? 1 : 0;

    if (i0 == i1)
        {
        Tag thisTag (ssOK, pDescr);
	    thisTag.Content (a0);
	    }
    else
        {
	    Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
	    thisTag.Content (a0);
	    thisTag.Content (a1);
	    }
    }


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
)
    {
    if (a0 == a1)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.Content (a0);
        }
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.Content (a0);
        thisTag.Content (a1);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Test a bool    expected to be true.
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkTrue
(
const bool    a0,
const char *pDescr,
double messageValue
)
    {
    if (messageValue != DBL_MAX)
        {
        char message[2048];
        sprintf (message, pDescr, messageValue);
        checkTrue (a0, message);
        return;
        }
    if (a0)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.ContentBool (NULL, a0);
	}
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.ContentBool (NULL, a0);
        }
    }

/*---------------------------------------------------------------------*//**
@description 
Test a bool    expected to be false
Announce result via recordOK or recordERROR.
------------------------------------------------------------------------*/
Public void checkFalse
(
const bool    a0,
const char *pDescr,
double messageValue
)
    {
    if (messageValue != DBL_MAX)
        {
        char message[2048];
        sprintf (message, pDescr, messageValue);
        checkFalse (a0, message);
        return;
        }
    if (!a0)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.ContentBool (NULL, a0);
	}
    else
        {
	Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.ContentBool (NULL, a0);
	}
    }


/*---------------------------------------------------------------------*//**
@description 
Compare two ints which are expected to agree.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkInt
(
const int a0,
const int a1,
const char *pDescr
)
    {
    if (a0 == a1)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.Content (a0);
	}
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.Content (a0);
        thisTag.Content (a1);
	}
    }

/*---------------------------------------------------------------------*//**
@description 
Test if an int is positive.
Announce result via startOK or startERROR.
------------------------------------------------------------------------*/
Public void checkPosInt
(
const int a,
const char *pDescr
)
    {
    if (a > 0)
        {
        Tag thisTag (ssOK, pDescr);
        thisTag.Content (a);
	}
    else
        {
        Tag thisTag (ssERROR, pDescr);
        thisTag.SetFormat (1);
        thisTag.Content (a);
	}
    }


/*---------------------------------------------------------------------*//**
@description 
    Output counts of OK, error messages.
    Zero counts.
------------------------------------------------------------------------*/
Public void showErrors
(
bool    bClear
)
    {
    checkInt (s_numERR, 0, "ErrorCount");
    if (bClear)
    	s_numOK = s_numERR = 0;    
    }


