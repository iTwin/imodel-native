/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Tools/MdlCnv.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendRotMatrix (Byte*& buffer, RotMatrixCR value, bool is3d)
    {
    double quat[4];

    if (is3d)
        value.GetQuaternion(quat, true);
    else
        value.GetRowValuesXY(quat);

    memcpy (buffer, quat, sizeof (quat));
    buffer += sizeof (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractRotMatrix (RotMatrixR value, Byte const *& buffer, bool is3d)
    {
    double quat[4];

    memcpy (quat, buffer, sizeof (quat));
    buffer += sizeof (quat);

    if (is3d)
        value.InitTransposedFromQuaternionWXYZ (quat);
    else
        value.InitFromRowValuesXY (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDPoint3d (Byte*& buffer, DPoint3dCR value)
    {
    double rBuf[3];

    rBuf[0] = value.x;
    rBuf[1] = value.y;
    rBuf[2] = value.z;

    memcpy (buffer, rBuf, sizeof (rBuf));
    buffer += sizeof (rBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractDPoint3d (DPoint3dR value, Byte const *& buffer)
    {
    double rBuf[3];

    memcpy (rBuf, buffer, sizeof (rBuf));
    buffer += sizeof (rBuf);

    value.x = rBuf[0];
    value.y = rBuf[1];
    value.z = rBuf[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDouble (Byte*& buffer, double const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractDouble (double& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendLong (Byte*& buffer, long const & value)       { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractLong (long& value, Byte const *& buffer)      { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendShort (Byte*& buffer, short const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractShort (short& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt64 (Byte*& buffer, int64_t const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt64 (int64_t& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt32 (Byte*& buffer, uint32_t const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt32 (uint32_t& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt (Byte*& buffer, int const & value)         { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt (int& value, Byte const *& buffer)        { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt16 (Byte*& buffer, uint16_t const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt16 (uint16_t& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt32 (Byte*& buffer, int32_t const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt32 (int32_t& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUShort (Byte*& buffer, unsigned short const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUShort (unsigned short& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendULong (Byte*& buffer, unsigned long const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractULong (unsigned long& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DML             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     DataConvert::Points3dTo2d (DPoint2dP outP, DPoint3dCP inP, int numPts)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/86
+---------------+---------------+---------------+---------------+---------------+------*/
void     DataConvert::Points2dTo3d (DPoint3dP outP, DPoint2dCP inP, int numPts, double zElev)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        outP->z = zElev;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          DataConvert::ReverseUInt64                              |
|                                                                       |
| author        RayBentley                             04/02k           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     DataConvert::ReverseUInt64
(
uint64_t&       output,
uint64_t        input
)
    {
    uint64_t    tmp = input;
    Byte *pTmpBytes = (Byte *) &tmp, *pOutputBytes = (Byte *) &output;

    pOutputBytes[0] = pTmpBytes[7];
    pOutputBytes[1] = pTmpBytes[6];
    pOutputBytes[2] = pTmpBytes[5];
    pOutputBytes[3] = pTmpBytes[4];
    pOutputBytes[4] = pTmpBytes[3];
    pOutputBytes[5] = pTmpBytes[2];
    pOutputBytes[6] = pTmpBytes[1];
    pOutputBytes[7] = pTmpBytes[0];
    }
