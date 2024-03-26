/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

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
VuP             faceSeedP
);

/*---------------------------------------------------------------------------------**//**
* @description Print statistics for all faces of the graph.  Print goes to the default GeomPrintFuncs object.
* @param graphP     IN  vu grapht
* @param pTitle     IN  text to print at start of report
* @group "VU Debugging"
* @see vu_printFaceLabelsOneFace
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_printFaceLabels
(
VuSetP          graphP,
char const      *pTitle
);
END_BENTLEY_GEOMETRY_NAMESPACE

