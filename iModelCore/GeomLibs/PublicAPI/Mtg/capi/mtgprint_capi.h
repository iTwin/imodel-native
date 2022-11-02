/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Expand an (integer) mask into an ascii buffer, appropriate
*   for use in a printf statement.
* @param pBuffer OUT     buffer of up to 64 ascii characeters (32 bits, various spaces)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlMTGGraph_fillMaskString
(
    char    *pBuffer,
MTGMask     mask
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGraph_printVertexLoops
(
MTGGraph  *pGraph
);

/*---------------------------------------------------------------------------------**//**
* Print face loops for the graph, with per-node callback for application additions.
* @param pGraph IN      graph to search
* @param pFunc IN      (optional) function to be called just after each line is printed
*           (but before its linefeed.)
* @param pContext IN      additional arg for callback
*<pre>
*               pFunc (pGraph, nodeId, pContext)
*</pre>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printFaceLoopsExt
(
MTGGraph    *pGraph,
MTGNodeFunc pFunc,
void        *pContext
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printFaceLoops
(
MTGGraph  *pGraph
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_printLoopCounts
(
MTGGraph  *pGraph
);

/*
Print array contents to console.
@param pArray IN array to print
@param pTitle IN title string
*/
Public GEOMDLLIMPEXP void jmdlEmbeddedIntArray_print
(
EmbeddedIntArray *pArray,
char *pTitle
);

/*
Print array contents to console.
@param pArray IN array to print
@param pTitle IN title string
*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_print
(
EmbeddedDPoint3dArray *pArray,
char *pTitle
);

END_BENTLEY_GEOMETRY_NAMESPACE

