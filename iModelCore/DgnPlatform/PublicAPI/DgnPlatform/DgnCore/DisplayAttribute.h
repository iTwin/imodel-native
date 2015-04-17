/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayAttribute.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include "ElementGeom.h"

#define DISPLAY_ATTRIBUTE_ID        65
#define DISPLAYATTR_PADWORDS        100
#define FILL_ATTRIBUTE              8
#define TEXTSTYLE_ATTRIBUTE         18
// Attrib used to be short, this will ensure no conflicts with existing values.
#define BASE_FOR_NEW_ATTRIBUTES         (1<<16)
#define COLORRGBA_ATTRIBUTE             (BASE_FOR_NEW_ATTRIBUTES+1)
#define LINEWIDTHMM_ATTRIBUTE           (BASE_FOR_NEW_ATTRIBUTES+2)
#define SCREENING_ATTRIBUTE             (BASE_FOR_NEW_ATTRIBUTES+3)
#define FILLCOLORRGBA_ATTRIBUTE         (BASE_FOR_NEW_ATTRIBUTES+4)
#define PLOTSTYLE_ATTRIBUTE             (BASE_FOR_NEW_ATTRIBUTES+5)
#define LINEJOIN_ATTRIBUTE              (BASE_FOR_NEW_ATTRIBUTES+6)
#define LINECAP_ATTRIBUTE               (BASE_FOR_NEW_ATTRIBUTES+7)
#define GRADIENT_ATTRIBUTE              (BASE_FOR_NEW_ATTRIBUTES+8)
#define TRANSPARENCY_ATTRIBUTE          (BASE_FOR_NEW_ATTRIBUTES+9)
#define UNUSED_ATTRIBUTE1               (BASE_FOR_NEW_ATTRIBUTES+10)    // Was ColorBook.
#define UNUSED_ATTRIBUTE2               (BASE_FOR_NEW_ATTRIBUTES+11)    // Was Fill Colorbook.
#define LVL_MATERIAL_ATTRIBUTE          (BASE_FOR_NEW_ATTRIBUTES+12)
#define LVL_ELEMENT_MATERIAL_ATTRIBUTE  (BASE_FOR_NEW_ATTRIBUTES+13)
#define MATERIAL_ATTRIBUTE              (BASE_FOR_NEW_ATTRIBUTES+14)
#define NONUNIFORMSYMB_ATTRIBUTE        (BASE_FOR_NEW_ATTRIBUTES+15)
#define DISPLAYSTYLE_ATTRIBUTE          (BASE_FOR_NEW_ATTRIBUTES+16)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   DisplayAttributeUnion                                               |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct      display_attribute_fill
    {
    uint32_t        color;
    unsigned short  transparency;
#if !defined (BITFIELDS_REVERSED)
    unsigned short  alwaysFilled:1;     // Ignore view fill property
    unsigned short  parityRule:1;       // 4 byte alignment
    unsigned short  reserved:14;        // 4 byte alignment
#else
    unsigned short  reserved:14;
    unsigned short  parityRule:1;       // 4 byte alignment
    unsigned short  alwaysFilled:1;     // Ignore view fill property
#endif
    } Display_attribute_fill;

typedef struct      display_attribute_textStyle
    {
    unsigned short  style;
    unsigned short  reserved;           // 4 byte alignment
    } Display_attribute_textStyle;

// These values are also matched against values on the pentable UI
//   so don't change casually.
#define COLORLINKAGE_TYPE_BYINDEX   0
#define COLORLINKAGE_TYPE_GRAYSCALE 1
#define COLORLINKAGE_TYPE_RGB       2

typedef struct      display_attribute_colorRGBA
    {
    int             colorType;
    int             originalPenIndex;
    RgbaColorDef    rgba;
    } Display_attribute_colorRGBA;

typedef struct      display_attribute_lineWidthMM
    {
    double          width;
    } Display_attribute_lineWidthMM;

typedef struct      display_attribute_screening
    {
    int             value;
    } Display_attribute_screening;

typedef struct      display_attribute_linejoin
    {
    int             value;
    } Display_attribute_linejoin;

typedef struct      display_attribute_linecap
    {
    int             value;
    } Display_attribute_linecap;

typedef struct      display_attribute_plotStyle
    {
    uint64_t     styleId;
    } Display_attribute_plotStyle;

typedef struct      gradientKey
    {
    double      value;
    unsigned    red:8;
    unsigned    blue:8;
    unsigned    green:8;
    unsigned    unused1:8;
    unsigned    unused2:32;
    } GradientKey;

typedef struct      Display_attribute_gradient
    {
    double          angle;
    double          tint;
    double          shift;
    uint16_t        nKeys;
    uint16_t        mode;
    uint16_t        flags;
    uint16_t        reserved;
    GradientKey     keys[MAX_GRADIENT_KEYS];
    } Display_attribute_gradient;

typedef struct      display_attribute_transparency
    {
    double          transparency;       // 1.0 == completely transparent.
    } Display_attribute_transparency;

typedef struct      display_attribute_material
    {
    uint64_t     materialId;
    } Display_attribute_material;

typedef struct      display_attribute_nonUniformSymb
    {
    int             value; // Currently unused...presence of linkage means complex shape/chain components don't have uniform symbology...
    } Display_attribute_nonUniformSymb;

typedef struct      display_attribute_displayStyle
    {
    uint32_t        styleIndex;
    } Display_attribute_displayStyle;

typedef union       displayAttributeUnion
    {
    Display_attribute_fill              fill;
    Display_attribute_textStyle         textStyle;
    Display_attribute_colorRGBA         colorRGBA;
    Display_attribute_lineWidthMM       lineWidthMM;
    Display_attribute_screening         screening;
    Display_attribute_colorRGBA         fillColorRGBA;
    Display_attribute_plotStyle         plotStyle;
    Display_attribute_linecap           lineCap;
    Display_attribute_linejoin          lineJoin;
    Display_attribute_gradient          gradient;
    Display_attribute_transparency      transparency;
    Display_attribute_material          material;
    Display_attribute_nonUniformSymb    nonUniformSymb;
    Display_attribute_displayStyle      displayStyle;
    unsigned short                      padding[DISPLAYATTR_PADWORDS];
    } DisplayAttributeUnion;

#define PLTATTR_LINECOLOR               (1<<0)
#define PLTATTR_FILLCOLOR               (1<<1)
#define PLTATTR_SCREENING               (1<<2)
#define PLTATTR_LINEWIDTH               (1<<3)
#define PLTATTR_LINECAP                 (1<<4)
#define PLTATTR_LINEJOIN                (1<<5)

typedef struct      displayAllPlotAttrib
    {
    uint32_t                            modifiers;
    Display_attribute_colorRGBA         colorRGBA;
    Display_attribute_lineWidthMM       lineWidthMM;
    Display_attribute_screening         screening;
    Display_attribute_colorRGBA         fillColorRGBA;
    } DisplayAllPlotAttrib;

#if defined (NEEDS_WORK_DGNITEM)
/*----------------------------------------------------------------------+
|                                                                       |
|   Display_attribute                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct      display_attribute
    {
    LinkageHeader           linkHdr;
    uint32_t                attr_type;
    DisplayAttributeUnion   attr_data;
    } Display_attribute;
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Display Attribute Functions                                         |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (NEEDS_WORK_DGNITEM)
BEGIN_BENTLEY_API_NAMESPACE

DGNPLATFORM_EXPORT bool    mdlElement_attributePresent
(
DgnElementCP         pElm,                       // => Element pointer
int                 attributeID,                // => Attr_type to look for
void*               attributeP                  // <= Buffer large enough to hold type (or NULL)
);

DGNPLATFORM_EXPORT bool    mdlElement_displayAttributePresent
(
DgnElementCP                 pElm,               // => Element pointer
int                         attributeID,        // => Attr_type to look for
DgnPlatform::Display_attribute*   attributeP    // <= Return displayable attribute
);

DGNPLATFORM_EXPORT bool    mdlElement_displayAttributeRemove
(
DgnElementP              pElm,                   // => Element pointer
int                     attributeID
);

DGNPLATFORM_EXPORT StatusInt mdlElement_displayAttributeCreate
(
DgnPlatform::Display_attribute   *pDa,          // <=> Display_attribute buffer
int                         attributeID,
int                         dataBytes,
unsigned short             *attributeDataP
);

DGNPLATFORM_EXPORT StatusInt mdlElement_displayAttributeAdd
(
DgnElementP                  pElm,               // <=> Pointer to DgnElementP
DgnPlatform::Display_attribute   *pDa           //  => Display_attribute to add
);

DGNPLATFORM_EXPORT StatusInt mdlElement_displayAttributeReplace
(
DgnElementP                  pElm,               // => Element pointer
int                         attributeID,        // => Attr_type to look for
DgnPlatform::Display_attribute   *attributeP    // => Pointer to new display attr
);

DGNPLATFORM_EXPORT StatusInt mdlElement_addFillDisplayAttribute
(
DgnElementP      elmP,                           // <=> Element pointer
uint32_t*         fillColor,                      //  => Color (same as element if NULL)
bool*        alwaysFilled                    //  => Ignore fill view flag
);

DGNPLATFORM_EXPORT StatusInt    mdlElement_addGradientDisplayAttribute
(
DgnElementP      elmP,
GradientSymbCR  gradientSymb
);

DGNPLATFORM_EXPORT bool mdlElement_addTransparencyDisplayAttribute (EditElementHandleR eeh, double transparency);

END_BENTLEY_API_NAMESPACE
#endif
