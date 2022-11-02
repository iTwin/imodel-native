/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Compress this instance by deleting obsolete/unused nodes and the corresponding
* coordinates/normals, and by dropping the last numLabelsToDrop labels of each
* remaining node.
* <P>
* The facets coordinate array is compressed if vertex data is present;
* otherwise, its allocated memory is released.
* <P>
* If a bool    parameter is false then the corresponding facets array(s) will
* be ignored, allowing for external custom compression by the caller.  Note that
* after this is done, any preexisting parallelism to the facets coordinate array
* will have been lost.
* <P>
* If a bool    parameter is true then the normal mode (IVertexFormat) of this
* instance determines the action on the corresponding facets array(s):
* <UL>
* <LI> <EM>No vertex data:</EM> release allocated memory
* <LI> <EM>Vertex coordinate data only:</EM>
*      <UL>
*      <LI> compressNormalArray is true: release allocated memory of normal
*           array
*      <LI> compressParamArrays is true: compress param arrays parallel to
*           vertex array
*      </UL>
* <LI> <EM>Normal per vertex:</EM> compress parallel to vertex array
* <LI> <EM>Separate normals:</EM>
*      <UL>
*      <LI> compressNormalArray is true: compress normal array independently
*      <LI> compressParamArrays is true: if compressNormalArray is also true,
*           then compress param arrays parallel to normal array; otherwise,
*           ignore param arrays
*      </UL>
* </UL>
* Warning: external references to or undropped labels that store MTG node IDs
* will no longer be valid.
*
* @param    pFacets             IN OUT  on output: compressed facets
* @param    obsoleteNodeMask     IN      mask of obsoleted nodes
* @param    numLabelsToDrop      IN      #labels to drop from end of each label block
* @param    compressNormalArray  IN      compress or release normalArray iff true
* @param    compressParamArrays  IN      compress paramArrays iff true
* @return false iff error (i.e., too many labels dropped for normal mode)
* @see MTGGraph#compress
* @see MTGFacets.IVertexFormat
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_compress
(
MTGFacets       *pFacets,
MTGMask         obsoleteNodeMask,
int             numLabelsToDrop,
bool            compressNormalArray,
bool            compressParamArrays
);

END_BENTLEY_GEOMETRY_NAMESPACE

