/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnRscFontStructures.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

typedef int16_t RscFontVec;
typedef unsigned short RscFontDist;
static const size_t MAX_FRACTION_MAP_COUNT = 76;

//=======================================================================================
// @bsiclass
//=======================================================================================
enum RscGPAMasks
{
    HMASK_RSC_LINE = 0x40000000,
    HMASK_RSC_POLY = 0x80000000,
    HMASK_RSC_HOLE = 0xC0000000
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscFontPoint2d
{
    int16_t x;
    int16_t y;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscFontVersion
{
    Byte version;
    Byte release;
    Byte stage;
    Byte fixes;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscFontDate
{
    int32_t julian;
    int32_t secs;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphBoundingBox
{
    RscFontVec left;
    RscFontVec bottom;
    RscFontVec right;
    RscFontVec top;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscFontFraction
{
    uint8_t numerator;
    uint8_t denominator;
    uint16_t charCode;
};


//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphHeader
{
    uint16_t code;
    RscFontDist width;
    RscFontVec leftEdge;
    RscFontVec rightEdge;
    uint32_t unused;
    int32_t kernOffset;
    int32_t symbJust;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphDataOffset
{
    int32_t offset;
    int32_t size;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphData
{
    uint32_t unused;
    int32_t length;
    int32_t numElems;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscGlyphElement
{
    unsigned char type;
    unsigned char color;
    unsigned short numVerts;
    RscFontPoint2d vertice[1];
};

//=======================================================================================
// @bsiclass
//=======================================================================================
enum RscGlyphElementType
{
    RSCGLYPHELEMENT_Line = 1,
    RSCGLYPHELEMENT_Poly = 2,
    RSCGLYPHELEMENT_PolyHoles = 3,
    RSCGLYPHELEMENT_PolyHole = 4,
    RSCGLYPHELEMENT_PolyHoleFinal = 5,
    RSCGLYPHELEMENT_Spline = 6
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RscFontHeader
{
    RscFontVersion version;
    RscFontDate creation;
    unsigned short machine;
    unsigned short type;
    uint32_t flags;
    uint32_t totalKern;
    uint32_t totalLig;
    uint32_t number;
    uint16_t totalChars;
    uint16_t firstChar;
    uint16_t lastChar;
    uint16_t defaultChar;
    RscFontPoint2d origin;
    RscGlyphBoundingBox maxbrr;
    RscFontVec ascender;
    RscFontVec capHeight;
    RscFontVec xHeight;
    RscFontVec descender;
    RscFontDist N_width;
    RscFontVec kernTracks[4];
    RscFontVec resv0;
    uint32_t fractMap;
    uint32_t unicodeMap;
    uint32_t filledFlag:1;
    uint32_t unused:31;
    int16_t languageId;
    unsigned short nFamilyName;
    unsigned short nFontName;
    unsigned short nFastName;
    unsigned short nDescript;
    unsigned short nName;
    char name[1];
};

END_BENTLEY_DGN_NAMESPACE
