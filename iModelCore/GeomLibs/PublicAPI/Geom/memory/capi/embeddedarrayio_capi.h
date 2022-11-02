/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/**
* Writes the array to the given stream.  Compatible with loadToDataStream.
*
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint2dArray_storeToDataStream
(
const EmbeddedDPoint2dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
);

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr IN OUT header for DPoint3d array to read from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @param bAppend IN true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint2dArray_loadFromDataStream
(
EmbeddedDPoint2dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
);

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint3dArray_storeToDataStream
(
const EmbeddedDPoint3dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
);

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr IN OUT header for DPoint3d array to read from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @param bAppend IN true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint3dArray_loadFromDataStream
(
EmbeddedDPoint3dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
);

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint2dArray_storeToDataStream
(
const EmbeddedFPoint2dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
);

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr IN OUT header for DPoint3d array to read from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @param bAppend IN true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint2dArray_loadFromDataStream
(
EmbeddedFPoint2dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
);

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint3dArray_storeToDataStream
(
const EmbeddedFPoint3dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
);

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr IN OUT header for DPoint3d array to read from data stream
* @param pStream IN OUT data stream ptr
* @param pFuncs IN callback bundle for reading/writing to the stream
* @param bAppend IN true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint3dArray_loadFromDataStream
(
EmbeddedFPoint3dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
);

