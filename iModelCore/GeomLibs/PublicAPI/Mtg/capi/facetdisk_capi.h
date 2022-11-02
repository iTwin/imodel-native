/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Writes a Facets structure to the stream.
*
* @param pStream IN OUT data stream ptr
* @param pFacets IN facet structure to write to data stream
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @see #jmdlMTGFacets_loadFromDataStream
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlMTGFacets_storeToDataStream
(
void                *pStream,
const MTGFacets     *pFacets,
const MTG_IOFuncs   *pFuncs
);

/**
* Populates this instance from the given stream, as written by
* jmdlMTGFacets_storeToDataStream.
*
* @param    pFacets IN OUT facet structure to fill from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @see #jmdlMTGFacets_storeToDataStream
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlMTGFacets_loadFromDataStream
(
MTGFacets           *pFacets,
void                *pStream,
const MTG_IOFuncs   *pFuncs
);

END_BENTLEY_GEOMETRY_NAMESPACE

