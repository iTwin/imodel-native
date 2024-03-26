/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Appends to each node of this instance the given number of labels with
* the given properties.
*
* @param        pGraph     IN OUT  labels added
* @param        pMask        IN      array of properties of new labels
* @param        pDefault     IN      array of default values for new labels
* @param        pTag         IN      array of tags for new labels
* @param        numToAdd     IN      # labels to add
* @return false iff error
* @see #dropLabels
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_appendLabels
(
MTGGraph        *pGraph,
const int       *pMask,
const int       *pDefault,
const int       *pTag,
int             numToAdd
);

/**
* Deletes the last numToDrop labels from each node of this instance.
*
* @param    pGraph      IN OUT  labels dropped
* @param    numToDrop    IN      #labels to drop from end of each label block
* @return false iff error
* @see #appendLabels
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_dropLabels
(
MTGGraph        *pGraph,
int             numToDrop
);

/**
* Compress this instance by deleting obsolete/unused nodes and by dropping the
* last numDroppedLabels labels of each remaining node.
* Warning: external references to or undropped labels that store MTG node IDs
* will no longer be valid.
*
* @param    pGraph              IN OUT  on output: compressed graph
* @param    obsoleteNodeMask     IN      mask of obsoleted nodes
* @param    numLabelsToDrop      IN      #labels to drop from end of each label block
* @return false iff error
* @see #dropLabels
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_compress
(
MTGGraph        *pGraph,
MTGMask         obsoleteNodeMask,
int             numLabelsToDrop
);

END_BENTLEY_GEOMETRY_NAMESPACE

