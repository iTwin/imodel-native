/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/src/CGWriter.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <string>

FILE *OpenGeomTestOutputFile (char *cname, char *extension, char *access = "w");


// If file is NULL, print to new output file.
void PrintCurve (MSBsplineCurveCR curve, char *title, FILE *file = NULL);
void PrintCurveVector (CurveVectorCR curve, char *title, FILE *file = NULL);