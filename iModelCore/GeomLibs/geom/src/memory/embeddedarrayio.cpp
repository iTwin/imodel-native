/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* @dllName mtg */
END_BENTLEY_GEOMETRY_NAMESPACE
#include <Mstn/Tools/ToolsAPI.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*======================================================================+
|                                                                       |
|   Major Public Code Section:                                          |
|       (loadFrom/storeToDataStream for embedded struct arrays)         |
|                                                                       |
+======================================================================*/
/**
* Writes the array to the given stream.  Compatible with loadToDataStream.
*
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint2dArray_storeToDataStream
(
const EmbeddedDPoint2dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
)
    {
    int numComponents;
    if (!pArrayHdr)
        return false;

    // write count
    numComponents = 3 * omdlVArray_getCount (&pArrayHdr->vbArray);

    if (1 != pFuncs->pWriteInts (pStream, &numComponents, 1))
        return false;

    // write buffer contents
    if (numComponents > 0)
        {
        const double *pArray;
        pArray = (const double *)omdlVArray_getConstPtr (&pArrayHdr->vbArray, 0);
        if (numComponents != pFuncs->pWriteDoubles (pStream, pArray, numComponents))
            return false;
        }

    return true;
    }

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr  <=> header for DPoint3d array to read from data stream
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @param bAppend    => true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint2dArray_loadFromDataStream
(
EmbeddedDPoint2dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
)
    {
    int numComponents, numPoints;
    if (!pArrayHdr)
        return false;

    // read count
    if (1 != pFuncs->pReadInts (&numComponents, pStream, 1))
        return false;
    numPoints = numComponents / 2;

    // fill buffer
    if (numComponents > 0)
        {
        double *pArray;
        if (!bAppend)
            omdlVArray_empty (&pArrayHdr->vbArray);
        pArray = (double *) omdlVArray_getNewBlock (&pArrayHdr->vbArray, numPoints);
        if (numComponents != pFuncs->pReadDoubles (pArray, pStream, numComponents))
            return false;
        }

    return true;
    }

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint3dArray_storeToDataStream
(
const EmbeddedDPoint3dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
)
    {
    int numComponents;
    if (!pArrayHdr)
        return false;

    // write count
    numComponents = 3 * omdlVArray_getCount (&pArrayHdr->vbArray);

    if (1 != pFuncs->pWriteInts (pStream, &numComponents, 1))
        return false;

    // write buffer contents
    if (numComponents > 0)
        {
        const double *pArray;
        pArray = (const double *)omdlVArray_getConstPtr (&pArrayHdr->vbArray, 0);
        if (numComponents != pFuncs->pWriteDoubles (pStream, pArray, numComponents))
            return false;
        }

    return true;
    }

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr  <=> header for DPoint3d array to read from data stream
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @param bAppend    => true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedDPoint3dArray_loadFromDataStream
(
EmbeddedDPoint3dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
)
    {
    int numComponents, numPoints;
    if (!pArrayHdr)
        return false;

    // read count
    if (1 != pFuncs->pReadInts (&numComponents, pStream, 1))
        return false;
    numPoints = numComponents / 3;

    // fill buffer
    if (numComponents > 0)
        {
        double *pArray;
        if (!bAppend)
            omdlVArray_empty (&pArrayHdr->vbArray);
        pArray = (double *) omdlVArray_getNewBlock (&pArrayHdr->vbArray, numPoints);
        if (numComponents != pFuncs->pReadDoubles (pArray, pStream, numComponents))
            return false;
        }

    return true;
    }

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint2dArray_storeToDataStream
(
const EmbeddedFPoint2dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
)
    {
    int numComponents;
    if (!pArrayHdr)
        return false;

    // write count
    numComponents = 3 * omdlVArray_getCount (&pArrayHdr->vbArray);

    if (1 != pFuncs->pWriteInts (pStream, &numComponents, 1))
        return false;

    // write buffer contents
    if (numComponents > 0)
        {
        const float *pArray;
        pArray = (const float *)omdlVArray_getConstPtr (&pArrayHdr->vbArray, 0);
        if (numComponents != pFuncs->pWriteFloats (pStream, pArray, numComponents))
            return false;
        }

    return true;
    }

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr  <=> header for DPoint3d array to read from data stream
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @param bAppend    => true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint2dArray_loadFromDataStream
(
EmbeddedFPoint2dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
)
    {
    int numComponents, numPoints;
    if (!pArrayHdr)
        return false;

    // read count
    if (1 != pFuncs->pReadInts (&numComponents, pStream, 1))
        return false;
    numPoints = numComponents / 2;

    // fill buffer
    if (numComponents > 0)
        {
        float *pArray;
        if (!bAppend)
            omdlVArray_empty (&pArrayHdr->vbArray);
        pArray = (float *) omdlVArray_getNewBlock (&pArrayHdr->vbArray, numPoints);
        if (numComponents != pFuncs->pReadFloats (pArray, pStream, numComponents))
            return false;
        }

    return true;
    }

/**
* Writes the array to the given stream.  Compatible with loadFromDataStream.
*
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint3dArray_storeToDataStream
(
const EmbeddedFPoint3dArray     *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs
)
    {
    int numComponents;
    if (!pArrayHdr)
        return false;

    // write count
    numComponents = 3 * omdlVArray_getCount (&pArrayHdr->vbArray);

    if (1 != pFuncs->pWriteInts (pStream, &numComponents, 1))
        return false;

    // write buffer contents
    if (numComponents > 0)
        {
        const float *pArray;
        pArray = (const float *)omdlVArray_getConstPtr (&pArrayHdr->vbArray, 0);
        if (numComponents != pFuncs->pWriteFloats (pStream, pArray, numComponents))
            return false;
        }

    return true;
    }

/**
* Reads the array from the given stream.  Compatible with storeToDataStream.
*
* @param pArrayHdr  <=> header for DPoint3d array to read from data stream
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @param bAppend    => true to append new points; false to overwrite old points
* @return false if error; true if success
* @bsihdr                                       EarlinLutz      01/00
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedFPoint3dArray_loadFromDataStream
(
EmbeddedFPoint3dArray           *pArrayHdr,
void                            *pStream,
const MTG_IOFuncsExt            *pFuncs,
bool                            bAppend
)
    {
    int numComponents, numPoints;
    if (!pArrayHdr)
        return false;

    // read count
    if (1 != pFuncs->pReadInts (&numComponents, pStream, 1))
        return false;
    numPoints = numComponents / 3;

    // fill buffer
    if (numComponents > 0)
        {
        float *pArray;
        if (!bAppend)
            omdlVArray_empty (&pArrayHdr->vbArray);
        pArray = (float *) omdlVArray_getNewBlock (&pArrayHdr->vbArray, numPoints);
        if (numComponents != pFuncs->pReadFloats (pArray, pStream, numComponents))
            return false;
        }

    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
