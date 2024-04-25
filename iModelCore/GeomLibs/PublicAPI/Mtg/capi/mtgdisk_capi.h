/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Writes a Graph structure to the stream.
*
* @param pStream IN OUT data stream ptr
* @param pGraph IN graph structure to write to data stream
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @see #jmdlMTGGraph_loadFromDataStream
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlMTGGraph_storeToDataStream
(
void                *pStream,
const MTGGraph      *pGraph,
const MTG_IOFuncs   *pFuncs
);

/**
* Populates this instance from the given stream, as written by
* jmdlMTGGraph_storeToDataStream.
*
* @param    pGraph IN OUT graph structure to fill from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @see #jmdlMTGGraph_storeToDataStream
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlMTGGraph_loadFromDataStream
(
MTGGraph            *pGraph,
void                *pStream,
const MTG_IOFuncs   *pFuncs
);

END_BENTLEY_GEOMETRY_NAMESPACE

