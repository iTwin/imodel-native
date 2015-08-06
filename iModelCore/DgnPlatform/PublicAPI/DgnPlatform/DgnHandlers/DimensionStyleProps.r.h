/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DimensionStyleProps.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

// ------------------------------------------------------------
//  This file is included by both .h/cpp and .r files
// ------------------------------------------------------------

/* Accuracy constants                                                       */
#define DIMACC_INVALID   0xff   /* not a valid accuracy value               */

#define DIMACC_0         0x00   /* display decimal to nearest integer       */
#define DIMACC_1         0x81   /* display decimal to nearest .1            */
#define DIMACC_2         0x82   /* display decimal to nearest .01           */
#define DIMACC_3         0x84   /* display decimal to nearest .001          */
#define DIMACC_4         0x88   /* display decimal to nearest .0001         */
#define DIMACC_5         0x90   /* display decimal to nearest .00001        */
#define DIMACC_6         0xa0   /* display decimal to nearest .000001       */
#define DIMACC_7         0xc0   /* display decimal to nearest .0000001      */
#define DIMACC_8         0x80   /* display decimal to nearest .00000001     */
#define DIMACC_HALF      0x01   /* display fraction to nearest 1/2          */
#define DIMACC_4th       0x02   /* display fraction to nearest 1/4          */
#define DIMACC_8th       0x04   /* display fraction to nearest 1/8          */
#define DIMACC_16th      0x08   /* display fraction to nearest 1/16         */
#define DIMACC_32nd      0x10   /* display fraction to nearest 1/32         */
#define DIMACC_64th      0x20   /* display fraction to nearest 1/64         */

#define DIMACC_Alt       0x40   /* apply alternate formatting      */

#define DIMACC_SCI_1    (DIMACC_Alt | 0x00)    /* display scientific to nearest 1.0        */
#define DIMACC_SCI_2    (DIMACC_Alt | 0x01)    /* display scientific to nearest 1.00       */
#define DIMACC_SCI_3    (DIMACC_Alt | 0x02)    /* display scientific to nearest 1.000      */
#define DIMACC_SCI_4    (DIMACC_Alt | 0x03)    /* display scientific to nearest 1.0000     */
#define DIMACC_SCI_5    (DIMACC_Alt | 0x04)    /* display scientific to nearest 1.00000    */
#define DIMACC_SCI_6    (DIMACC_Alt | 0x05)    /* display scientific to nearest 1.000000   */
#define DIMACC_SCI_7    (DIMACC_Alt | 0x06)    /* display scientific to nearest 1.0000000  */
#define DIMACC_SCI_8    (DIMACC_Alt | 0x07)    /* display scientific to nearest 1.00000000 */

/*---------------------------------------------------------------------------+
|                                                                            |
|   Dimension Types                                                          |
|                                                                            |
+---------------------------------------------------------------------------*/
#define DGNPLATFORM_RESOURCE_DIMTYPE_SIZE_ARROW         1
#define DGNPLATFORM_RESOURCE_DIMTYPE_SIZE_STROKE        2
#define DGNPLATFORM_RESOURCE_DIMTYPE_LOCATE_SINGLE      3
#define DGNPLATFORM_RESOURCE_DIMTYPE_LOCATE_STACKED     4
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_SIZE         5
#define DGNPLATFORM_RESOURCE_DIMTYPE_ARC_SIZE           6
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_LOCATION     7
#define DGNPLATFORM_RESOURCE_DIMTYPE_ARC_LOCATION       8
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_LINES        9
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_AXIS        10
#define DGNPLATFORM_RESOURCE_DIMTYPE_RADIUS            11
#define DGNPLATFORM_RESOURCE_DIMTYPE_DIAMETER          12
#define DGNPLATFORM_RESOURCE_DIMTYPE_DIAMETER_PARA     13
#define DGNPLATFORM_RESOURCE_DIMTYPE_DIAMETER_PERP     14
#define DGNPLATFORM_RESOURCE_DIMTYPE_CUSTOM_LINEAR     15
#define DGNPLATFORM_RESOURCE_DIMTYPE_ORDINATE          16
#define DGNPLATFORM_RESOURCE_DIMTYPE_RADIUS_EXTENDED   17
#define DGNPLATFORM_RESOURCE_DIMTYPE_DIAMETER_EXTENDED 18
#define DGNPLATFORM_RESOURCE_DIMTYPE_CENTER            19
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_AXIS_X      50
#define DGNPLATFORM_RESOURCE_DIMTYPE_ANGLE_AXIS_Y      51
#define DGNPLATFORM_RESOURCE_DIMTYPE_LABEL_LINE        52
#define DGNPLATFORM_RESOURCE_DIMTYPE_NOTE              53

#define DGNPLATFORM_RESOURCE_DIMTYPE_MAX               53  /* please update this if you add more DIMTYPEs */


//!@addtogroup  "Dimension Styles"

//!<p>
//!A dimension style contains values for all the properties needed to create a dimension element. They can be accessed
//!by calling functions on the DimensionStyle object.
//!</p>
//!
//!<p>
//!For information regarding getting or setting of particular properties of a dimension style,
//!see ~tDimStyleProp.
//!</p>
//!
//!Related functions that may be of interest are:
//!<p>
//!    <UL>
//!    <LI><code>DimensionStyle::GetActive</code></LI>
//!    <LI><code>DimensionStyle::GetByName</code></LI>
//!    </UL>
//!</p>

BEGIN_BENTLEY_DGN_NAMESPACE

//!---------------------------------------------------------------------------------**//**
//!Valid values for ball and chain modes.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_BallAndChain_Mode_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
//!--------------+---------------+---------------+---------------+---------------+------*/
enum DimStyleProp_BallAndChain_Mode
    {
    DIMSTYLE_VALUE_BallAndChain_Mode_None                    = 0,
    DIMSTYLE_VALUE_BallAndChain_Mode_On                      = 1,
    DIMSTYLE_VALUE_BallAndChain_Mode_Auto                    = 2,
    DIMSTYLE_VALUE_BallAndChain_MODE_COUNT                   = 3,

    };

//! Valid values for ball and chain text alignment.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_BallAndChain_Alignment
    {
    DIMSTYLE_VALUE_BallAndChain_Alignment_Auto                          = 0,
    DIMSTYLE_VALUE_BallAndChain_Alignment_Left                          = 1,
    DIMSTYLE_VALUE_BallAndChain_Alignment_Right                         = 2,
    DIMSTYLE_VALUE_BallAndChain_Alignment_COUNT                         = 3,

    };

//! Valid values for terminators.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER
//!  DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG
//!  DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG
//!  DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG
//!  DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG
//!  DIMSTYLE_PROP_Terminator_Note_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Terminator_Type
    {
    DIMSTYLE_VALUE_Terminator_Type_None                                 = 0,
    DIMSTYLE_VALUE_Terminator_Type_Arrow                                = 1,
    DIMSTYLE_VALUE_Terminator_Type_Stroke                               = 2,
    DIMSTYLE_VALUE_Terminator_Type_Origin                               = 3,
    DIMSTYLE_VALUE_Terminator_Type_Dot                                  = 4,
    DIMSTYLE_VALUE_Terminator_Type_Note                                 = 5,
    DIMSTYLE_VALUE_Terminator_Type_COUNT                                = 6,

    };

//! Valid values for ball and chain leader type.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_BallAndChain_ChainType
    {
    DIMSTYLE_VALUE_BallAndChain_ChainType_None                          = 0,
    DIMSTYLE_VALUE_BallAndChain_ChainType_Line                          = 1,
    DIMSTYLE_VALUE_BallAndChain_ChainType_Arc                           = 2,
    DIMSTYLE_VALUE_BallAndChain_ChainType_BSpline                       = 3,
    DIMSTYLE_VALUE_BallAndChain_ChainType_COUNT                         = 4,

    };

//! Valid values for dimension alignment.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_General_Alignment_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_General_Alignment
    {
    DIMSTYLE_VALUE_General_Alignment_View                               = 0,
    DIMSTYLE_VALUE_General_Alignment_Drawing                            = 1,
    DIMSTYLE_VALUE_General_Alignment_True                               = 2,
    DIMSTYLE_VALUE_General_Alignment_Arbitrary                          = 3,
    DIMSTYLE_VALUE_General_Alignment_COUNT                              = 4,

    };

//! Valid values for radial mode.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_General_RadialMode_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_General_RadialMode
    {
    DIMSTYLE_VALUE_General_RadialMode_CenterMark                        = 0,
    DIMSTYLE_VALUE_General_RadialMode_Radius                            = 1,
    DIMSTYLE_VALUE_General_RadialMode_RadiusExtended                    = 2,
    DIMSTYLE_VALUE_General_RadialMode_Diameter                          = 3,
    DIMSTYLE_VALUE_General_RadialMode_DiameterExtended                  = 4,
    DIMSTYLE_VALUE_General_RadialMode_COUNT                             = 5,

    };

//! Valid values for MuliLine Note frame type.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_FrameType_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_FrameType
    {
    DIMSTYLE_VALUE_MLNote_FrameType_None                                = 0,
    DIMSTYLE_VALUE_MLNote_FrameType_Line                                = 1,
    DIMSTYLE_VALUE_MLNote_FrameType_Box                                 = 2,
    DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox                          = 3,
    DIMSTYLE_VALUE_MLNote_FrameType_Circle                              = 4,
    DIMSTYLE_VALUE_MLNote_FrameType_Capsule                             = 5,
    DIMSTYLE_VALUE_MLNote_FrameType_Hexagon                             = 6,
    DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon                      = 7,
    DIMSTYLE_VALUE_MLNote_FrameType_Triangle                            = 8,
    DIMSTYLE_VALUE_MLNote_FrameType_Pentagon                            = 9,
    DIMSTYLE_VALUE_MLNote_FrameType_Octagon                             = 10,
    DIMSTYLE_VALUE_MLNote_FrameType_COUNT                               = 11,

    };

//!Valid values for MultiLine Note horizontal justification.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_Justification_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_Justification
    {
    DIMSTYLE_VALUE_MLNote_Justification_Left                            = 0,
    DIMSTYLE_VALUE_MLNote_Justification_Right                           = 1,
    DIMSTYLE_VALUE_MLNote_Justification_Dynamic                         = 2,
    DIMSTYLE_VALUE_MLNote_Justification_Center                          = 3,
    DIMSTYLE_VALUE_MLNote_Justification_COUNT                           = 4,

    };

//!Valid values for MultiLine Note vertical justification.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_VerticalJustification : uint16_t
    {
    DIMSTYLE_VALUE_MLNote_VerticalJustification_Top                     = 0,
    DIMSTYLE_VALUE_MLNote_VerticalJustification_Center                  = 1,
    DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom                  = 2,
    DIMSTYLE_VALUE_MLNote_VerticalJustification_Dynamic                 = 3,
    DIMSTYLE_VALUE_MLNote_VerticalJustification_COUNT                   = 4,

    };

//!Valid values for MultiLine Note text rotation.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_TextRotation_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_TextRotation
    {
    DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal                       = 0,
    DIMSTYLE_VALUE_MLNote_TextRotation_Vertical                         = 1,
    DIMSTYLE_VALUE_MLNote_TextRotation_Inline                           = 2,
    DIMSTYLE_VALUE_MLNote_TextRotation_COUNT                            = 3,

    };

//!Valid values for MultiLine Note horizontal attachment.
//!Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_HorAttachment
    {
    DIMSTYLE_VALUE_MLNote_HorAttachment_Auto                            = 0,
    DIMSTYLE_VALUE_MLNote_HorAttachment_Left                            = 1,
    DIMSTYLE_VALUE_MLNote_HorAttachment_Right                           = 2,
    DIMSTYLE_VALUE_MLNote_HorAttachment_COUNT                           = 3,

    };

//!Valid values for MultiLine Note vertical attachment location.
//!Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER
//!  DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_MLNote_VerAttachment
    {
    DIMSTYLE_VALUE_MLNote_VerAttachment_Top                             = 0,
    DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine                         = 1,
    DIMSTYLE_VALUE_MLNote_VerAttachment_Middle                          = 2,
    DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine                      = 3,
    DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom                          = 4,
    DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine                     = 5,
    DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner                   = 6,
    DIMSTYLE_VALUE_MLNote_VerAttachment_Underline                       = 7,
    DIMSTYLE_VALUE_MLNote_VerAttachment_COUNT                           = 8,

    };

//!Valid values for standard symbols.  Used for properties:
//! <pre>
//! DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG
//! DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG
//! </pre>
//!See <code>DimensionStyle::GetTemplateFlagProp DimensionStyle::SetTemplateFlagProp</code>
//!
enum  DimStyleProp_Symbol_Standard
    {
    DIMSTYLE_VALUE_Symbol_Standard_None                                 = 0,
    DIMSTYLE_VALUE_Symbol_Standard_Diameter                             = 1,
    DIMSTYLE_VALUE_Symbol_Standard_Radius                               = 2,
    DIMSTYLE_VALUE_Symbol_Standard_Area                                 = 3,
    DIMSTYLE_VALUE_Symbol_Standard_SphericalRadius                      = 4,
    DIMSTYLE_VALUE_Symbol_Standard_SphericalDiameter                    = 5,
    DIMSTYLE_VALUE_Symbol_Standard_COUNT                                = 6,

    };

//!Valid values for custom symbol type.  Used for properties:
//! <pre>
//! DIMSTYLE_PROP_Symbol_DiameterType_INTEGER
//! DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER
//! </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Symbol_CustomType
    {
    DIMSTYLE_VALUE_Symbol_CustomType_Default                            = 0,
    DIMSTYLE_VALUE_Symbol_CustomType_Character                          = 1,
    DIMSTYLE_VALUE_Symbol_CustomType_COUNT                              = 2,

    };

//!Valid values for prefix and suffix symbol type.  Used for properties:
//! <pre>
//! DIMSTYLE_PROP_Symbol_PrefixType_INTEGER
//! DIMSTYLE_PROP_Symbol_SuffixType_INTEGER
//! </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Symbol_PreSufType
    {
    DIMSTYLE_VALUE_Symbol_PreSufType_None                               = 0,
    DIMSTYLE_VALUE_Symbol_PreSufType_Character                          = 1,
    DIMSTYLE_VALUE_Symbol_PreSufType_Cell                               = 2,
    DIMSTYLE_VALUE_Symbol_PreSufType_COUNT                              = 3,

    };

//! Valid values for terminator symbol type.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Terminator_ArrowType_INTEGER
//!  DIMSTYLE_PROP_Terminator_DotType_INTEGER
//!  DIMSTYLE_PROP_Terminator_OriginType_INTEGER
//!  DIMSTYLE_PROP_Terminator_StrokeType_INTEGER
//!  DIMSTYLE_PROP_Terminator_NoteType_INTEGER
//! </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Symbol_TermType
    {
    DIMSTYLE_VALUE_Symbol_TermType_Default                              = 0,
    DIMSTYLE_VALUE_Symbol_TermType_Character                            = 1,
    DIMSTYLE_VALUE_Symbol_TermType_Cell                                 = 2,
    DIMSTYLE_VALUE_Symbol_TermType_COUNT                                = 3,

    };

//!Valid values for terminator mode.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Terminator_Mode_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Terminator_Mode
    {
    DIMSTYLE_VALUE_Terminator_Mode_Auto                                 = 0,
    DIMSTYLE_VALUE_Terminator_Mode_Reversed                             = 1,
    DIMSTYLE_VALUE_Terminator_Mode_Inside                               = 2,
    DIMSTYLE_VALUE_Terminator_Mode_Outside                              = 3,
    DIMSTYLE_VALUE_Terminator_Mode_COUNT                                = 4,

    };

//!Valid values for default arrowhead.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Terminator_Arrowhead
    {
    DIMSTYLE_VALUE_Terminator_Arrowhead_Open                            = 0,
    DIMSTYLE_VALUE_Terminator_Arrowhead_Closed                          = 1,
    DIMSTYLE_VALUE_Terminator_Arrowhead_Filled                          = 2,
    DIMSTYLE_VALUE_Terminator_Arrowhead_COUNT                           = 3,

    };

//!Valid values for text justification.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Text_Justification_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum  DimStyleProp_Text_Justification
    {
    DIMSTYLE_VALUE_Text_Justification_Offset                            = 0,
    DIMSTYLE_VALUE_Text_Justification_Left                              = 1,
    DIMSTYLE_VALUE_Text_Justification_CenterLeft                        = 2,
    DIMSTYLE_VALUE_Text_Justification_Right                             = 3,
    DIMSTYLE_VALUE_Text_Justification_CenterRight                       = 4,
    DIMSTYLE_VALUE_Text_Justification_COUNT                             = 5,
    };


//!Valid values for angle format.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_AngleFormat_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum  DimStyleProp_Value_AngleFormat
    {
    DIMSTYLE_VALUE_Value_AngleFormat_Degrees                            = (uint32_t) AngleFormatVals::Degrees,
    DIMSTYLE_VALUE_Value_AngleFormat_DegMinSec                          = (uint32_t) AngleFormatVals::DegMinSec,
    DIMSTYLE_VALUE_Value_AngleFormat_Centesimal                         = (uint32_t) AngleFormatVals::Centesimal,
    DIMSTYLE_VALUE_Value_AngleFormat_Radians                            = (uint32_t) AngleFormatVals::Radians,
    DIMSTYLE_VALUE_Value_AngleFormat_DegMin                             = (uint32_t) AngleFormatVals::DegMin,
    DIMSTYLE_VALUE_Value_AngleFormat_COUNT                              = 5,
    };

//!Valid values for angle precision.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_AnglePrecision_INTEGER
//!  </pre>
//! This property is used when Angle Format is either degree, centesimal, or
//!  radian.
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Value_AnglePrecision
    {
    DIMSTYLE_VALUE_Value_AnglePrecision_Whole                           = 0,
    DIMSTYLE_VALUE_Value_AnglePrecision_1_Place                         = 1,
    DIMSTYLE_VALUE_Value_AnglePrecision_2_Place                         = 2,
    DIMSTYLE_VALUE_Value_AnglePrecision_3_Place                         = 3,
    DIMSTYLE_VALUE_Value_AnglePrecision_4_Place                         = 4,
    DIMSTYLE_VALUE_Value_AnglePrecision_5_Place                         = 5,
    DIMSTYLE_VALUE_Value_AnglePrecision_6_Place                         = 6,
    DIMSTYLE_VALUE_Value_AnglePrecision_COUNT                           = 7,
    };

//!Valid values for DMS Accuracy Mode.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DIMSTYLE_PROP_Value_DMSPrecisionMode
    {
    /** Standard precision interpretation. */
    DIMSTYLE_VALUE_Value_DMSPrecisionMode_Fixed                          = 0,

    /** Precision is interpreted according to the following rules: <pre>
                         PREC = 0, display whole Degrees only.
                         PREC = 1, display whole Degrees and Minutes only.
                         PREC = 2, display whole Degrees, Minutes, and Seconds.
                         PREC > 2, display DMS with decimal seconds PREC - 2</pre>*/
    DIMSTYLE_VALUE_Value_DMSPrecisionMode_Floating                       = 1,

    DIMSTYLE_VALUE_Value_DMSPrecisionMode_COUNT                          = 2,
    };

//!Valid values for stacked fraction alignment.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
enum DimStyleProp_Text_StackedFractionAlignment
    {
    DIMSTYLE_VALUE_Text_StackedFractionAlignment_Top                    = 0,
    DIMSTYLE_VALUE_Text_StackedFractionAlignment_Center                 = 1,
    DIMSTYLE_VALUE_Text_StackedFractionAlignment_Bottom                 = 2,
    DIMSTYLE_VALUE_Text_StackedFractionAlignment_COUNT                  = 3,
    };

//!Valid values for stacked fraction type.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_StackedFractionType_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
typedef enum DimStyleProp_Text_StackedFractionType
    {
    DIMSTYLE_VALUE_Text_StackedFractionType_FromFont                    = 0,
    DIMSTYLE_VALUE_Text_StackedFractionType_Horizontal                  = 1,
    DIMSTYLE_VALUE_Text_StackedFractionType_Diagonal                    = 2,
    DIMSTYLE_VALUE_Text_StackedFractionType_COUNT                       = 3,

    } DimStyleProp_Text_StackedFractionType;

//!Valid values for text positioning.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Placement_TextPosition_INTEGER
//!  </pre>
//!This property controls how the dimension creation tools position the dimension text.
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
typedef enum DimStyleProp_Placement_TextPosition
    {
    /** The user is always prompted to position text */
    DIMSTYLE_VALUE_Placement_TextPosition_Manual                        = 0,

    /** The user is only prompted if the text will not fit between
                     the extension lines */
    DIMSTYLE_VALUE_Placement_TextPosition_SemiAuto                      = 1,

    /** The user is never prompted to position text */
    DIMSTYLE_VALUE_Placement_TextPosition_Auto                          = 2,

    DIMSTYLE_VALUE_Placement_TextPosition_COUNT                         = 3,

    } DimStyleProp_Placement_TextPosition;

//!Valid values for text location relative to dimension line.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Text_Location_INTEGER
//!  </pre>
//!This property controls the location of dimension text relative to the dimension line.
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
typedef enum DimStyleProp_Text_Location
    {
    /** The text is located inline with the dimension line */
    DIMSTYLE_VALUE_Text_Location_Inline                                 = 0,

    /** The text is located above the dimension line */
    DIMSTYLE_VALUE_Text_Location_Above                                  = 1,

    /** The text is located on the opposite side of the extension lines */
    DIMSTYLE_VALUE_Text_Location_Outside                                = 2,

    /** The text is located on the top or left side of the dimension line */
    DIMSTYLE_VALUE_Text_Location_TopLeft                                = 3,

    DIMSTYLE_VALUE_Text_Location_COUNT                                  = 4,

    } DimStyleProp_Text_Location;


//!Valid values for superscript mode.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
//!
typedef enum
    {
    /** The text is located inline with the dimension line */
    DIMSTYLE_VALUE_Text_SuperscriptMode_FromFont                        = 0,

    /** The text is located above the dimension line */
    DIMSTYLE_VALUE_Text_SuperscriptMode_Generated                       = 1,

    DIMSTYLE_VALUE_Text_SuperscriptMode_COUNT                           = 2,

    } DimStyleProp_Text_SuperscriptMode;


//!Valid values for value format.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_Format_INTEGER
//!  DIMSTYLE_PROP_Value_AltFormat_INTEGER
//!  DIMSTYLE_PROP_Value_SecFormat_INTEGER
//!  DIMSTYLE_PROP_Value_AltSecFormat_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>

typedef enum DimStyleProp_Value_Format
    {
    /** Ex. 10'6":  10 */
    DIMSTYLE_VALUE_Value_Format_MU                                      = 0,

    /** Ex. 10'6":  10' */
    DIMSTYLE_VALUE_Value_Format_MU_Label                                = 1,

    /** Ex. 10'6":  6 */
    DIMSTYLE_VALUE_Value_Format_SU                                      = 2,

    /** Ex. 10'6":  6" */
    DIMSTYLE_VALUE_Value_Format_SU_Label                                = 3,

    /** Ex. 10'6":  10-6 */
    DIMSTYLE_VALUE_Value_Format_MU_dash_SU                              = 4,

    /** Ex. 10'6":  10' 6' */
    DIMSTYLE_VALUE_Value_Format_MU_Label_SU_Label                       = 5,

    /** Ex. 10'6":  10'-6' */
    DIMSTYLE_VALUE_Value_Format_MU_Label_dash_SU_Label                  = 6,

    DIMSTYLE_VALUE_Value_Format_COUNT                                   = 7,

    } DimStyleProp_Value_Format;


//! Valid values for threshold comparisons.  Used for properties:
//!   <pre>
//!   DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER
//!   DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER
//!   </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>

typedef enum DimStyleProp_Value_Comparison
    {
    DIMSTYLE_VALUE_Value_Compare_Less                                   = 0,
    DIMSTYLE_VALUE_Value_Compare_Greater                                = 1,
    DIMSTYLE_VALUE_Value_Compare_LessOrEqual                            = 2,
    DIMSTYLE_VALUE_Value_Compare_GreaterOrEqual                         = 3,
    DIMSTYLE_VALUE_Value_Compare_COUNT                                  = 4,

    } DimStyleProp_Value_Comparison;


//! Valid values for controlling vertical text.  Used for properties:
//!   <pre>
//!   DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG
//!   </pre>
//! See <code>DimensionStyle::GetTemplateFlagProp DimensionStyle::SetTemplateFlagProp</code>
//!

typedef enum DimStyleProp_Text_Vertical
    {
    /** The text is not vertical */
    DIMSTYLE_VALUE_Text_Vertical_Never                                  = 0,

    /** The text is always vertical */
    DIMSTYLE_VALUE_Text_Vertical_Always                                 = 1,

    /** The text is vertical only if it does not fit */
    DIMSTYLE_VALUE_Text_Vertical_NoFit                                  = 2,

    DIMSTYLE_VALUE_Text_Vertical_COUNT                                  = 3,

    } DimStyleProp_Text_Vertical;


//!Valid values for thousands delimeter.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>


typedef enum DimStyleProp_Value_ThousandsOpts
    {
    /** The thousands place is not delimited, ex: 1000 */
    DIMSTYLE_VALUE_Value_ThousandsSep_None                              = 0,

    /** The thousands place is delimited by a space, ex: 1 000 */
    DIMSTYLE_VALUE_Value_ThousandsSep_Space                             = 1,

    /** The thousands place is delimited by a comma, ex: 1,000.0 or in
                     decimal comma mode, by a period 1.000,0 */
    DIMSTYLE_VALUE_Value_ThousandsSep_Comma                             = 2,

    DIMSTYLE_VALUE_Value_Thousands_COUNT                                = 3,

    } DimStyleProp_Value_ThousandsOpts;


//!Valid values for text frame type.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Text_FrameType_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>

typedef enum DimStyleProp_Text_FrameType
    {
    DIMSTYLE_VALUE_Text_FrameType_None                                  = 0,
    DIMSTYLE_VALUE_Text_FrameType_Box                                   = 1,
    DIMSTYLE_VALUE_Text_FrameType_Capsule                               = 2,
    DIMSTYLE_VALUE_Text_FrameType_COUNT                                 = 3,

    } DimStyleProp_Text_FrameType;


//!* Valid values for label line format.  Used for properties:
//!*   <pre>
//!*   DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER
//!*   </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>

typedef enum
    {
    /** Length value above, Angle value below  */
    DIMSTYLE_VALUE_Value_LabelLineFormat_Standard                       = 0,

    /** Angle value above, Length value below */
    DIMSTYLE_VALUE_Value_LabelLineFormat_AngleOverLength                = 1,

    /** Length value above, No Angle value     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAbove                    = 2,

    /** Angle value above, No Length value     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_AngleAbove                     = 3,

    /** Length value below, No Angle value     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_LengthBelow                    = 4,

    /** Angle value below, No Length value     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_AngleBelow                     = 5,

    /** Length and Angle values both above     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleAbove               = 6,

    /** Length and Angle values both below     */
    DIMSTYLE_VALUE_Value_LabelLineFormat_LengthAngleBelow               = 7,

    DIMSTYLE_VALUE_Value_LabelLineFormat_COUNT                          = 8,

    } DimStyleProp_Value_LabelLineFormat;


//!Valid values for accuracy.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_Value_Accuracy_ACCURACY
//!  DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY
//!  DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY
//!  DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY
//!  DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY
//!  DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY
//!  </pre>
//!See <code>DimensionStyle::GetAccuracyProp DimensionStyle::SetAccuracyProp</code>


typedef enum DimStyleProp_Value_Accuracy
    {
    /** Display decimal to nearest integer        Ex: 10              */
    DIMSTYLE_VALUE_Value_Accuracy_Whole                                 = DIMACC_0,

    /** Display decimal to nearest .1             Ex: 10.1            */
    DIMSTYLE_VALUE_Value_Accuracy_1_Decimal                             = DIMACC_1,

    /** Display decimal to nearest .01            Ex: 10.12           */
    DIMSTYLE_VALUE_Value_Accuracy_2_Decimal                             = DIMACC_2,

    /** Display decimal to nearest .001           Ex: 10.123          */
    DIMSTYLE_VALUE_Value_Accuracy_3_Decimal                             = DIMACC_3,

    /** Display decimal to nearest .0001          Ex: 10.1234         */
    DIMSTYLE_VALUE_Value_Accuracy_4_Decimal                             = DIMACC_4,

    /** Display decimal to nearest .00001         Ex: 10.12345        */
    DIMSTYLE_VALUE_Value_Accuracy_5_Decimal                             = DIMACC_5,

    /** Display decimal to nearest .000001        Ex: 10.123456       */
    DIMSTYLE_VALUE_Value_Accuracy_6_Decimal                             = DIMACC_6,

    /** Display decimal to nearest .0000001       Ex: 10.1234567      */
    DIMSTYLE_VALUE_Value_Accuracy_7_Decimal                             = DIMACC_7,

    /** Display decimal to nearest .00000001      Ex: 10.12345678     */
    DIMSTYLE_VALUE_Value_Accuracy_8_Decimal                             = DIMACC_8,

    /** Display fraction to nearest 1/2           Ex: 10 1/2          */
    DIMSTYLE_VALUE_Value_Accuracy_Half                                  = DIMACC_HALF,

    /** Display fraction to nearest 1/4           Ex: 10 1/4          */
    DIMSTYLE_VALUE_Value_Accuracy_Quarter                               = DIMACC_4th,

    /** Display fraction to nearest 1/8           Ex: 10 1/8          */
    DIMSTYLE_VALUE_Value_Accuracy_Eighth                                = DIMACC_8th,

    /** Display fraction to nearest 1/16          Ex: 10 1/16         */
    DIMSTYLE_VALUE_Value_Accuracy_Sixteenth                             = DIMACC_16th,

    /** Display fraction to nearest 1/32          Ex: 10 1/32         */
    DIMSTYLE_VALUE_Value_Accuracy_ThirtySecond                          = DIMACC_32nd,

    /** Display fraction to nearest 1/64          Ex: 10 1/64         */
    DIMSTYLE_VALUE_Value_Accuracy_SixtyFourth                           = DIMACC_64th,

    /** Display scientific to nearest 1.0         Ex: 1.1E+01         */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_1_Decimal                  = DIMACC_SCI_1,

    /** Display scientific to nearest 1.00        Ex: 1.12E+01        */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_2_Decimal                  = DIMACC_SCI_2,

    /** Display scientific to nearest 1.000       Ex: 1.123E+01       */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_3_Decimal                  = DIMACC_SCI_3,

    /** Display scientific to nearest 1.0000      Ex: 1.1234E+01      */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_4_Decimal                  = DIMACC_SCI_4,

    /** Display scientific to nearest 1.00000     Ex: 1.12345E+01     */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_5_Decimal                  = DIMACC_SCI_5,

    /** Display scientific to nearest 1.000000    Ex: 1.123456E+01    */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_6_Decimal                  = DIMACC_SCI_6,

    /** Display scientific to nearest 1.0000000   Ex: 1.1234567E+01   */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_7_Decimal                  = DIMACC_SCI_7,

    /** Display scientific to nearest 1.00000000  Ex: 1.12345678E+01  */
    DIMSTYLE_VALUE_Value_Accuracy_Scientific_8_Decimal                  = DIMACC_SCI_8,

    } DimStyleProp_Value_Accuracy;


//!Valid values for fit options.  Used for properties:
//!  <pre>
//!  DIMSTYLE_PROP_General_FitOption_INTEGER
//!  </pre>
//!See <code>DimensionStyle::GetIntegerProp DimensionStyle::SetIntegerProp</code>
typedef enum DimStyleProp_FitOptions
    {
    /* keep values 0 - 3 in such order that they are the same as terminator locations */
    /* If not enough room, first flip out terms, then do text. Merged with legacy term. auto=0 */
    DIMSTYLE_VALUE_FitOption_MoveTermsFirst       = 0,
    /* legacy term. reversed=1 */
    DIMSTYLE_VALUE_FitOption_ReverseTerms         = 1,
    /* legacy fixed term. inside=2 */
    DIMSTYLE_VALUE_FitOption_KeepTermsInside      = 2,
    /* legacy fixed term. outside=3 */
    DIMSTYLE_VALUE_FitOption_KeepTermsOutside     = 3,
    /* Always keep text inside, but flip terminators if not enough room */
    DIMSTYLE_VALUE_FitOption_KeepTextInside       = 4,
    /* If not enough room, first flip out text, then do terms */
    DIMSTYLE_VALUE_FitOption_MoveTextFirst        = 5,
    /* If not enough room, flip out both text and terms */
    DIMSTYLE_VALUE_FitOption_MoveBoth             = 6,
    /* Best fit: flip out the smaller of text and terms */
    DIMSTYLE_VALUE_FitOption_MoveEither           = 7,
    DIMSTYLE_VALUE_FitOption_COUNT                = 8,
    } DimStyleProp_FitOptions;


//!  DimStyleProp lists all the properties a dimension style. To access a property,
//!  call the DimensionStyle::Get (or Set) function that corresponds to the data type of
//!  that property.  For each property, the data type is included at the end of the
//!  property name.  For example, the property <code>DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE</code>
//!  is of type double, so the functions <code>DimensionStyle::Get(Set)DoubleProp</code>
//!  would be used to query(change) this property.  The following table lists the
//!  property get/set functions along with the corresponding property name suffix.
//!<p>
//!<table border="1">
//!  <tr> <td>ACCURACY</td>     <td>DimensionStyle::GetAccuracyProp</td>     <td>DimensionStyle::SetAccuracyProp</td> </tr>
//!  <tr> <td>BOOLINT</td>      <td>DimensionStyle::GetBoolIntProp</td>      <td>DimensionStyle::SetBoolIntProp</td> </tr>
//!  <tr> <td>CHAR</td>         <td>DimensionStyle::GetCharProp</td>         <td>DimensionStyle::SetCharProp</td> </tr>
//!  <tr> <td>COLOR</td>        <td>DimensionStyle::GetColorProp</td>        <td>DimensionStyle::SetColorProp</td> </tr>
//!  <tr> <td>DOUBLE</td>       <td>DimensionStyle::GetDoubleProp</td>       <td>DimensionStyle::SetDoubleProp</td> </tr>
//!  <tr> <td>INTEGER</td>      <td>DimensionStyle::GetIntegerProp</td>      <td>DimensionStyle::SetIntegerProp</td> </tr>
//!  <tr> <td>FONT</td>         <td>DimensionStyle::GetFontProp</td>         <td>DimensionStyle::SetFontProp</td> </tr>
//!  <tr> <td>LEVEL</td>        <td>DimensionStyle::GetLevelProp</td>        <td>DimensionStyle::SetLevelProp</td> </tr>
//!  <tr> <td>LINESTYLE</td>    <td>DimensionStyle::GetLineStyleProp</td>    <td>DimensionStyle::SetLineStyleProp</td> </tr>
//!  <tr> <td>MSWCHAR</td>      <td>DimensionStyle::GetMSWCharProp</td>      <td>DimensionStyle::SetMSWCharProp</td> </tr>
//!  <tr> <td>TEMPLATEFLAG</td> <td>DimensionStyle::GetTemplateFlagProp</td> <td>DimensionStyle::SetTemplateFlagProp</td> </tr>
//!  <tr> <td>UNITS</td>        <td>DimensionStyle::GetUnitsProp</td>        <td>DimensionStyle::SetUnitsProp</td> </tr>
//!  <tr> <td>WEIGHT</td>       <td>DimensionStyle::GetWeightProp</td>       <td>DimensionStyle::SetWeightProp</td> </tr>
//!</table>

typedef enum DimStyleProp
    {
    /** Non-existant property. */
    DIMSTYLE_PROP_Invalid                                               = 0,

    /*------------------------------------------------------------------------
    Properties that effect ball and chain dimensioning
    ------------------------------------------------------------------------*/

    /** Controls the alignment of ball and chain text.  See the
                     enumeration ~!tDimStyleProp_BallAndChain_Alignment for
                     valid values. */
    DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER                        = 101,

    /** Controls the terminator for ball and chain leader. See
                     the enumeration ~!tDimStyleProp_Terminator_Type for valid
                     values. */
    DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER                  = 102,

    /** Controls the type of leader for ball and chain. See
                     the enumeration ~!tDimStyleProp_BallAndChain_ChainType for
                     valid values. */
    DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER                        = 103,

    /** Enables ball and chain - Deprecated.
                     Use ~!tDIMSTYLE_PROP_BallAndChain_Mode_INTEGER instead. */
    DIMSTYLE_PROP_BallAndChain_IsActive_BOOLINT                         = 104,

    /** A short line will connect text and leader. */
    DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT                   = 105,

    /** Dock the text on dimension line if within tolerance. */
    DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT                  = 106,

    /** Ball & Chain elbow line length in text height units. */
    DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE                       = 107,

    /** Multiline note elbow line length in terms of text height. */
    DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE                             = 108,

    /** Use ball&chain elbow length. */
    DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT                   = 109,

    /** ball & chain modes made from values of see the enumeration
                    ~!tDimStyleProp_BallAndChain_Mode for valid values */
    DIMSTYLE_PROP_BallAndChain_Mode_INTEGER                             = 110,

    /*-------------------------------------------------------------------*//**
    @Line Properties that effect extension lines
    *//*--------------------------------------------------------------------*/

    /** Color of extension lines. */
    DIMSTYLE_PROP_ExtensionLine_Color_COLOR                             = 201,

    /** Extension lines extend past dim line by this distance. */
    DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE                           = 202,

    /** Join the extension lines when the text is not between. */
    DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT                            = 203,

    /** Show/Hide the left extension line. */
    DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG                       = 204,

    /** LineStyle of extension lines. */
    DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE                     = 205,

    /** Extension lines begin at this distance from dim point. */
    DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE                           = 206,

    /** true  = Use ExtensionLine_Color, false  = Use General_Color. */
    DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT                   = 207,

    /** true  = Use ExtensionLine_LineStyle, false  = Use General_LineStyle. */
    DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT               = 208,

    /** true  = Use ExtensionLine_Weight, false  = Use General_Weight. */
    DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT                  = 209,

    /** Show/Hide the right extension line. */
    DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG                      = 210,

    /** Show/Hide the all the extension lines. */
    DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT                         = 211,

    /** Weight of extension lines. */
    DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT                           = 212,

    /** Extension lines will not be aligned radially, used only
                     for Arc Size dimensions using length measure, not angle. */
    DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG            = 213,

    /*------------------------------------------------------------------------
    General Properties
    ------------------------------------------------------------------------*/

    /** Dimension alignment. See the enumeration
                     ~!tDimStyleProp_General_Alignment for valid values. */
    DIMSTYLE_PROP_General_Alignment_INTEGER                             = 301,

    /** Size of Center Mark for radial dimensions. */
    DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE                         = 302,

    /** Color for dimension element ehdr. */
    DIMSTYLE_PROP_General_Color_COLOR                                   = 303,

    /** Scale applied for display of length. */
    DIMSTYLE_PROP_General_DimensionScale_DOUBLE                         = 304,

    /** Description of dimension style. */
    DIMSTYLE_PROP_General_DimStyleDescription_MSWCHAR                   = 305,

    /** Name of dimension style. */
    DIMSTYLE_PROP_General_DimStyleName_MSWCHAR                          = 306,

    /** Font used for dimension value text. */
    DIMSTYLE_PROP_General_Font_FONT                                     = 307,

    /** If true, level symbology ignored when displaying dimensions. */
    DIMSTYLE_PROP_General_IgnoreLevelSymbology_BOOLINT                  = 308,

    /** LineStyle for dimension element ehdr. */
    DIMSTYLE_PROP_General_LineStyle_LINESTYLE                           = 309,

    /** true  = Use General_Color, false  = Use active. */
    DIMSTYLE_PROP_General_OverrideColor_BOOLINT                         = 310,

    /** true  = Use General_LineStyle, false  = Use active. */
    DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT                     = 311,

    /** true  = Use General_Weight, false  = Use active. */
    DIMSTYLE_PROP_General_OverrideWeight_BOOLINT                        = 312,

    /** Mode for radial dimension tool. See the enumeration
                     ~!tDimStyleProp_General_RadialMode for valid values. */
    DIMSTYLE_PROP_General_RadialMode_INTEGER                            = 313,

    /** True  = dimline height preserved on associative change. */
    DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT                       = 314,

    /** Display center mark for radial dimensions. */
    DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG                   = 315,

    /** Stack segments for linear or angular dimensions. */
    DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG                          = 316,

    /** Minimum distance between stacked segments. */
    DIMSTYLE_PROP_General_StackOffset_DOUBLE                            = 317,

    /** Weight for dimension element ehdr. */
    DIMSTYLE_PROP_General_Weight_WEIGHT                                 = 318,

    /** Fit options for dimension line, text and terminators. */
    DIMSTYLE_PROP_General_FitOption_INTEGER                             = 319,

    /** true to suppress terminators and dimension lines
                     if terminators don't, but text does, fit between extension lines. */
    DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT              = 320,

    /** true to push text to the "right" side instead of "left" side when fitting */
    DIMSTYLE_PROP_General_PushTextRight_BOOLINT                         = 321,

    /** true to fit text that is above or aside dim line
                     without min. leader when fittting */
    DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT                     = 322,

    /** true to use input minimum leader for fitting */
    DIMSTYLE_PROP_General_UseMinLeader_BOOLINT                          = 323,

    /** true to fit both text intersection size and text box */
    DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT                    = 324,

    /** This property is deprecated. Do Not Use. */
    DIMSTYLE_PROP_General_FrozenInSharedCell_BOOLINT                    = 325,

    /** true to extend dimension line under text for angular dimension
                     when text is pushed outside */
    DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT                = 326,

    /*------------------------------------------------------------------------
    Properties that effect Multi line notes
    ------------------------------------------------------------------------*/

    /** Type of frame to display on multiline note text. See
                     the enumeration ~!tDimStyleProp_MLNote_FrameType for
                     valid values. */
    DIMSTYLE_PROP_MLNote_FrameType_INTEGER                              = 401,

    /** Controls the horizontal positioning of multiline note text.
                     See the enumeration ~!tDimStyleProp_MLNote_Justification
                     for valid values. */
    DIMSTYLE_PROP_MLNote_Justification_INTEGER                          = 402,

    /** Display a short leader for multiline note text. */
    DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT                             = 403,

    /** Controls the vertical positioning of multiline note text.
                     See the enumeration ~!tDimStyleProp_MLNote_VerticalJustification
                     for valid values. */
    DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER                  = 404,

    /** Controls the multiline note leader type - Line/Curve */
    DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT                             = 405,

    /** Controls the rotation of multiline note text.
                     See the enumeration ~!tDimStyleProp_MLNote_TextRotation
                     for valid values. */
    DIMSTYLE_PROP_MLNote_TextRotation_INTEGER                           = 406,

    /** Controls the horizontal attachment of leader to multiline note.
                     See the enumeration ~!tDimStyleProp_MLNote_HorAttachment
                     for valid values. */
    DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER                          = 407,

    /** Controls the vertical attachment of leader to multiline note
                     when the leader is on the leftside. See the enumeration
                     ~!tDimStyleProp_MLNote_VerAttachment for valid values. */
    DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER                      = 408,

    /** Controls the vertical attachment of leader to multiline note
                     when the leader is on the rightside. See the enumeration
                     ~!tDimStyleProp_MLNote_VerAttachment for valid values. */
    DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER                     = 409,

    /** Controls the left margin of multiline note. */
    DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE                              = 410,

    /** Controls the lower margin of multiline note. */
    DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE                             = 411,

    /** Controls the scaling of multiline note frames. */
    DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT                             = 412,

    /** Controls the scale of multiline note frames. */
    DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE                              = 413,

    /*------------------------------------------------------------------------
    Properties that effect dimensions at placement time
    ------------------------------------------------------------------------*/
    /** If true, dimensions are dropped on placement */
    DIMSTYLE_PROP_Placement_CompatibleV3_BOOLINT                        = 501,

    /** Level on which dimensions are created */
    DIMSTYLE_PROP_Placement_Level_LEVEL                                 = 502,

    /** If true, Placement_Level_LEVEL is used */
    DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT                       = 503,

    /** Controls positioning of text in placement tools.  See
                     the enumeration ~!tDimStyleProp_Placement_TextPosition
                     for valid values */
    DIMSTYLE_PROP_Placement_TextPosition_INTEGER                        = 504,

    /** If true, Value is scaled for true size of referenced geometry */
    DIMSTYLE_PROP_Placement_UseReferenceScale_BOOLINT                   = 505,

    /** If false, Dimension geometry is scaled by the model's annotation scale
                     If true, Dimension geometry is scaled by an annotation scale specified
                     in the Dimension Style */
    DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT          = 506,

    /** Annotation scale factor to use if DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT
                     is ON */
    DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE                      = 507,

    /*------------------------------------------------------------------------
    Properties that effect dimension symbols
    ------------------------------------------------------------------------*/
    /** Character for diameter symbol, used if DIMSTYLE_PROP_Symbol_DiameterType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_DiameterChar_CHAR                              = 601,

    /** Font for diameter symbol, used if DIMSTYLE_PROP_Symbol_DiameterType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_DiameterFont_FONT                              = 602,

    /** Type for diameter symbol, see the enumeration
                     ~!tDimStyleProp_Symbol_Type for valid values. */
    DIMSTYLE_PROP_Symbol_DiameterType_INTEGER                           = 603,

    /** Character for secondary value prefix. Only used for dual dims. */
    DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR                           = 604,

    /** Character for secondary value suffix. Only used for dual dims. */
    DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR                           = 605,

    /** Character for value prefix. Not used for dual dims. */
    DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR                            = 606,

    /** Character for value suffix. Not used for dual dims. */
    DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR                            = 607,

    /** Character for plus minus symbol, used if ~!tDIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR                             = 608,

    /** Type for plus minus symbol, see the enumeration ~!tDimStyleProp_Symbol_Type
                     for valid values. */
    DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER                          = 609,

    /** Standard prefix symbol, see the enumeration ~!tDimStyleProp_Symbol_Standard
                     for valid values. */
    DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG                            = 610,

    /** Cell name for custom prefix symbol, used if ~!tDIMSTYLE_PROP_Symbol_PrefixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Cell */
    DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR                         = 611,

    /** Character for custom prefix symbol, used if ~!tDIMSTYLE_PROP_Symbol_PrefixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_PrefixChar_CHAR                                = 612,

    /** Font for custom prefix symbol, used if ~!tDIMSTYLE_PROP_Symbol_PrefixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_PrefixFont_FONT                                = 613,

    /** Type for custom prefix symbol, see the enumeration ~!tDimStyleProp_Symbol_Type
                     for valid values. */
    DIMSTYLE_PROP_Symbol_PrefixType_INTEGER                             = 614,

    /** Standard suffix symbol, see the enumeration ~!tDimStyleProp_Symbol_Standard
                     for valid values. */
    DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG                            = 615,

    /** Cell name for custom suffix symbol, used if ~!tDIMSTYLE_PROP_Symbol_SuffixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Cell */
    DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR                         = 616,

    /** Character for custom suffix symbol, used if ~!tDIMSTYLE_PROP_Symbol_SuffixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_SuffixChar_CHAR                                = 617,

    /** Font for custom suffix symbol, used if ~!tDIMSTYLE_PROP_Symbol_SuffixType_INTEGER
                     is set to DIMSTYLE_VALUE_Symbol_Type_Character */
    DIMSTYLE_PROP_Symbol_SuffixFont_FONT                                = 618,

    /** Type for custom suffix symbol, see the enumeration ~!tDimStyleProp_Symbol_Type
                     for valid values. */
    DIMSTYLE_PROP_Symbol_SuffixType_INTEGER                             = 619,

    /** Tolerance prefix character. */
    DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR                             = 620,

    /** Tolerance suffix character. */
    DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR                             = 621,

    /** Character for primary value prefix. Only used for dual dims. */
    DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR                           = 622,

    /** Character for primary value suffix. Only used for dual dims. */
    DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR                           = 623,

    /*------------------------------------------------------------------------
    Properties that effect dimension terminators
    ------------------------------------------------------------------------*/
    /** Cell to use for arrow terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_ArrowType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Cell */
    DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR                      = 701,

    /** Character to use for arrow terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_ArrowType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character */
    DIMSTYLE_PROP_Terminator_ArrowChar_CHAR                             = 702,

    /** Font to use for character based arrow terminators, used only if the
                     property DIMSTYLE_PROP_Terminator_ArrowType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character */
    DIMSTYLE_PROP_Terminator_ArrowFont_FONT                             = 703,

    /** Symbol Type for arrow terminators. See the enumeration
                     ~!tDimStyleProp_Symbol_TermType for valid values. Also
                     use the following properties to control the arrow terminator:
                     DIMSTYLE_PROP_Terminator_ArrowChar_CHAR
                     DIMSTYLE_PROP_Terminator_ArrowFont_FONT
                     DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR */
    DIMSTYLE_PROP_Terminator_ArrowType_INTEGER                          = 704,

    /** Color for terminators.  Used only if
                     DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT is true */
    DIMSTYLE_PROP_Terminator_Color_COLOR                                = 705,

    /** Cell to use for dot terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_DotType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Cell */
    DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR                        = 706,

    /** Character to use for dot terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_DotType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character */
    DIMSTYLE_PROP_Terminator_DotChar_CHAR                               = 707,

    /** Font to use for character based dot terminators, used only if the
                     property DIMSTYLE_PROP_Terminator_DotType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character */
    DIMSTYLE_PROP_Terminator_DotFont_FONT                               = 708,

    /** Symbol Type for dot terminators. See the enumeration
                     DimStyleProp_Symbol_TermType for valid values. Also
                     use the following properties to control the dot terminator:
                     DIMSTYLE_PROP_Terminator_DotChar_CHAR
                     DIMSTYLE_PROP_Terminator_DotFont_FONT
                     DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR */
    DIMSTYLE_PROP_Terminator_DotType_INTEGER                            = 709,

    /** Type for first terminator, see the enumeration ~!tDimStyleProp_Terminator_Type
                     for valid values.  If type None is specified, then the first terminator is
                     determined by DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG */
    DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG                         = 710,

    /** Height of terminators expressed as a fraction of text height. */
    DIMSTYLE_PROP_Terminator_Height_DOUBLE                              = 711,

    /** Type for joint terminators, see the enumeration ~!tDimStyleProp_Terminator_Type
                     for valid values.  Joint terminators occur where two chained dimension segments meet */
    DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG                         = 712,

    /** Type for left terminators, see the enumeration ~!tDimStyleProp_Terminator_Type
                     for valid values.  The 'left' terminator for a dimension segment is the
                     one closest to the lower indexed point, not necessarily the left side. */
    DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG                          = 713,

    /** LineStyle for terminators.  Used only if
                     DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT is true */
    DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE                        = 714,

    /** Minimum gap between the dimension's text and the terminators.  This value
                     is a distance expressed as a fraction of the text WIDTH. */
    DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE                           = 715,

    /** Terminator location, see the enumeration ~!tDimStyleProp_Terminator_Mode
                     for valid values. Deprecated! Use ~!tDIMSTYLE_PROP_General_FitOption_INTEGER */
    DIMSTYLE_PROP_Terminator_Mode_INTEGER                               = 716,

    /** If true, the dimension line will not pass through arrow terminators.
                     Deprecated!  Use ~!tDIMSTYLE_PROP_Terminator_DimLineThruArrow_BOOLINT. */
    DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT                    = 717,

    /** If true, the dimension line will not pass through dot terminators.
                     Deprecated!  Use ~!tDIMSTYLE_PROP_Terminator_DimLineThruDot_BOOLINT. */
    DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT                      = 718,

    /** If true, the dimension line will not pass through origin terminators.
                     Deprecated!  Use ~!tDIMSTYLE_PROP_Terminator_DimLineThruOrigin_BOOLINT. */
    DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT                   = 719,

    /** If true, the dimension line will not pass through dot terminators.
                     Deprecated!  Use ~!tDIMSTYLE_PROP_Terminator_DimLineThruStroke_BOOLINT. */
    DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT                   = 720,

    /** Cell to use for origin terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_OriginType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Cell. */
    DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR                     = 721,

    /** Character to use for origin terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_OriginType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_OriginChar_CHAR                            = 722,

    /** Font to use for character based origin terminators, used only if the
                     property DIMSTYLE_PROP_Terminator_OriginType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_OriginFont_FONT                            = 723,

    /** Symbol Type for origin terminators. See the enumeration
                     DimStyleProp_Symbol_TermType for valid values. Also
                     use the following properties to control the origin terminator:
                     DIMSTYLE_PROP_Terminator_OriginChar_CHAR
                     DIMSTYLE_PROP_Terminator_OriginFont_FONT
                     DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR. */
    DIMSTYLE_PROP_Terminator_OriginType_INTEGER                         = 724,

    /** true  = Use Terminator_Color, false = Use ehdr color. */
    DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT                      = 725,

    /** true  = Use Terminator_LineStyle, false = Use ehdr linestyle. */
    DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT                  = 726,

    /** true  = Use Terminator_Weight, false = Use ehdr weight. */
    DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT                     = 727,

    /** Type for right terminators, see the enumeration ~!tDimStyleProp_Terminator_Type
                     for valid values.  The 'right' terminator for a dimension segment is the
                     one closest to the higher indexed point, not necessarily the right side. */
    DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG                         = 728,

    /** Further type specifier for standard arrow terminators, see the enumeration
                     ~!tDimStyleProp_Terminator_Arrowhead for valid values.  Only used if
                     DIMSTYLE_PROP_Terminator_ArrowType_INTEGER is set to Default. */
    DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER                          = 729,

    /** Cell to use for stroke terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_StrokeType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Cell. */
    DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR                     = 730,

    /** Character to use for stroke terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_StrokeType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_StrokeChar_CHAR                            = 731,

    /** Font to use for character based stroke terminators, used only if the
                     property DIMSTYLE_PROP_Terminator_StrokeType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_StrokeFont_FONT                            = 732,

    /** Symbol Type for stroke terminators. See the enumeration
                     ~!tDimStyleProp_Symbol_TermType for valid values. Also
                     use the following properties to control the stroke terminator:
                     DIMSTYLE_PROP_Terminator_StrokeChar_CHAR
                     DIMSTYLE_PROP_Terminator_StrokeFont_FONT
                     DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR. */
    DIMSTYLE_PROP_Terminator_StrokeType_INTEGER                         = 733,

    /** Weight for terminators.  Used only if
                     DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT is true. */
    DIMSTYLE_PROP_Terminator_Weight_WEIGHT                              = 734,

    /** Width of terminators expressed as a fraction of text height. */
    DIMSTYLE_PROP_Terminator_Width_DOUBLE                               = 735,

    /** Type for note terminators. See the enumeration
                     ~!tDimStyleProp_Terminator_Type for valid values. */
    DIMSTYLE_PROP_Terminator_Note_INTEGER                               = 736,

    /** Symbol type for note terminators.  See the enumeration
                     ~!tDimStyleProp_Symbol_TermType for valid values. Also
                     use the following properties to control the note terminator:
                     DIMSTYLE_PROP_Terminator_NoteChar_CHAR
                     DIMSTYLE_PROP_Terminator_NoteFont_FONT
                     DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR. */
    DIMSTYLE_PROP_Terminator_NoteType_INTEGER                           = 737,

    /** Cell to use for note terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_NoteType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Cell. */
    DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR                       = 738,

    /** Character to use for note terminators, used only if the property
                     DIMSTYLE_PROP_Terminator_NoteType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_NoteChar_CHAR                              = 739,

    /** Font to use for character based note terminators, used only if the
                     property DIMSTYLE_PROP_Terminator_NoteType_INTEGER is set to
                     the value DIMSTYLE_VALUE_Symbol_TermType_Character. */
    DIMSTYLE_PROP_Terminator_NoteFont_FONT                              = 740,

    /** true to uniformly scale terminator cells.  When true, terminator
                     width is a scale factor and terminator height is ignored. */
    DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT                   = 741,

    /** If true, the dimension line will pass through arrow terminators. */
    DIMSTYLE_PROP_Terminator_DimLineThruArrow_BOOLINT                   = 742,

    /** If true, the dimension line will pass through dot terminators. */
    DIMSTYLE_PROP_Terminator_DimLineThruDot_BOOLINT                     = 743,

    /** If true, the dimension line will pass through origin terminators. */
    DIMSTYLE_PROP_Terminator_DimLineThruOrigin_BOOLINT                  = 744,

    /** If true, the dimension line will pass through dot terminators. */
    DIMSTYLE_PROP_Terminator_DimLineThruStroke_BOOLINT                  = 745,

    /*------------------------------------------------------------------------
    Properties that effect dimension text
    ------------------------------------------------------------------------*/
    /** true to display an arc length symbol above the dimension value. */
    DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG                     = 801,

    /** true to enable automatic text lift which offsets dimension text if the
                     text would overlap with text from the previous segment. */
    DIMSTYLE_PROP_Text_AutoLift_BOOLINT                                 = 802,

    /** This property is deprecated. Use DIMSTYLE_PROP_Text_FrameType_INTEGER instead. */
    DIMSTYLE_PROP_Text_Box_BOOLINT                                      = 803,

    /** This property is deprecated. Use DIMSTYLE_PROP_Text_FrameType_INTEGER instead. */
    DIMSTYLE_PROP_Text_Capsule_BOOLINT                                  = 804,

    /** Color for dimension text.  Used only if DIMSTYLE_PROP_Text_OverrideColor_BOOLINT
                     is true.  */
    DIMSTYLE_PROP_Text_Color_COLOR                                      = 805,

    /** true to display a comma for decimal point.  Also, a point will be used for thousands
                     separator.*/
    DIMSTYLE_PROP_Text_DecimalComma_BOOLINT                             = 806,

    /** This property is deprecated. Use DIMSTYLE_PROP_Text_Location_INTEGER instead. */
    DIMSTYLE_PROP_Text_Embed_BOOLINT                                    = 807,

    /** true to use font specified in DIMSTYLE_PROP_General_Font_FONT, false to
                     inherit font from text style */
    DIMSTYLE_PROP_Text_Font_BOOLINT                                     = 808,

    /** Height of dimension text.  Used only if DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT
                     is true. */
    DIMSTYLE_PROP_Text_Height_DOUBLE                                    = 809,

    /** true to align dimension text to the horizontal, false to align along the dimension line. */
    DIMSTYLE_PROP_Text_Horizontal_BOOLINT                               = 810,

    /** Distance to define the horizontal margin surrounding the text expressed as a fraction
                     of text height.  The margin is used to control cutting of dimension line, size of
                     box, capsule frame, etc.  */
    DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE                          = 811,

    /** Controls the positioning of the text relative to the dimension.  See the enumeration
                     ~!tDimStyleProp_Text_Justification for valid values. */
    DIMSTYLE_PROP_Text_Justification_INTEGER                            = 812,

    /** true to allow the display of leading zeros, ex. 0.5 vs .5 */
    DIMSTYLE_PROP_Text_LeadingZero_BOOLINT                              = 813,

    /** This property is deprecated. Use DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG instead. */
    DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG                       = 814,

    /** If true and DIMSTYLE_PROP_Text_LeadingZero_BOOLINT is false and the dimension value
                     is less than one, the leading decimal point is not displayed, ex. 0.500 -> 500 */
    DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT                     = 815,

    /** true to use the color specified by DIMSTYLE_PROP_Text_Color_COLOR. false to inherit
                     from the text style. */
    DIMSTYLE_PROP_Text_OverrideColor_BOOLINT                            = 816,

    /** true to use the height specified by DIMSTYLE_PROP_Text_Height_DOUBLE. false to inherit
                     from the text style. */
    DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT                           = 817,

    /** true to use the weight specified by DIMSTYLE_PROP_Text_Weight_WEIGHT. false to inherit
                     from the text style. */
    DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT                           = 818,

    /** true to use the width specified by DIMSTYLE_PROP_Text_Width_DOUBLE. false to inherit
                     from the text style. */
    DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT                            = 819,

    /** true to allow the display of leading zeros for secondary values, ex. 0.5 vs .5 */
    DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT                           = 820,

    /** true to enable the display of secondary dimension values.  Typically, secondary
                     values are used to display the dimension's value is a different unit system,
                     for example if the primary value is displayed in feet and inches, the secondary
                     value might be displayed in meters. */
    DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT                            = 821,

    /** true to underline the dimension value.  Used only if DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT
                     is true. */
    DIMSTYLE_PROP_Text_Underline_BOOLINT                                = 822,

    /** This property is deprecated. Use DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG instead. */
    DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG                            = 823,

    /** The distance between the text and the dimension line for non-inline dimensions expressed
                     in fractions of text height. */
    DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE                            = 824,

    /** The weight of the dimension text.  Used only for resource fonts. */
    DIMSTYLE_PROP_Text_Weight_WEIGHT                                    = 825,

    /** Width of dimension text.  Used only if DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT
                     is true. */
    DIMSTYLE_PROP_Text_Width_DOUBLE                                     = 826,

    /** The text style that controls various properties of the dimensions text display. */
    DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE                              = 827,

    /** The ID of the dimStyle's text style.  Do not 'set' this value directly.  To set
                     the text style use DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE. */
    DIMSTYLE_PROP_Text_TextStyleID_INTEGER                              = 828,

    /** Controls the alignment for stacked fractions. See
                     the enumeration ~!tDimStyleProp_Text_StackedFractionAlignment
                     for valid values. */
    DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER                 = 829,

    /** Controls the formatting of fractions in the dimension value */
    DIMSTYLE_PROP_Text_StackedFractions_BOOLINT                         = 830,

    /** Controls the alignment for stacked fractions. See
                     the enumeration ~!tDimStyleProp_Text_StackedFractionType
                     for valid values. */
    DIMSTYLE_PROP_Text_StackedFractionType_INTEGER                      = 831,

    /** Controls the size of stacked fraction text relative to
                     main dimension text.  Must be positive. */
    DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE                      = 832,

    /** true = Use DIMSTYLE_PROP_Text_StackedFractions_BOOLINT, false  = Use Active. */
    DIMSTYLE_PROP_Text_OverrideStackedFractions_BOOLINT                 = 833,

    /** true = Use DIMSTYLE_PROP_Text_Underline_BOOLINT, false  = Use Active. */
    DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT                        = 834,

    /** Controls the location of dimension text relative to the dimension line.
                     See the enumeration ~!tDimStyleProp_Text_Location for valid values.*/
    DIMSTYLE_PROP_Text_Location_INTEGER                                 = 835,

    /** Controls the use of vertical dimension text.
                     See the enumeration ~!tDimStyleProp_Text_Vertical for valid values.*/
    DIMSTYLE_PROP_Text_VerticalOpts_TEMPLATEFLAG                        = 836,

    /** Controls the frame surrounding the dimension text.
                     See the enumeration ~!tDimStyleProp_Text_FrameType for valid values.*/
    DIMSTYLE_PROP_Text_FrameType_INTEGER                                = 837,

    /** Controls the text vertical position for horizontal inline text case */
    DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE                            = 838,

    /** Controls the mode by which superscripts are generated for the case
                     where DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT is true.  See the
                     enumeration ~!tDimStyleProp_Text_SuperscriptMode for valid values. */
    DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER                          = 839,

    /*------------------------------------------------------------------------
    Properties that effect tolerance dimensioning
    ------------------------------------------------------------------------*/
    /** Distance for lower tolerance value */
    DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE                           = 901,

    /** true for limit tolerance, false for plus/minus tolerance */
    DIMSTYLE_PROP_Tolerance_Mode_BOOLINT                                = 902,

    /** true to enable tolerance */
    DIMSTYLE_PROP_Tolerance_Show_BOOLINT                                = 903,

    /** true to display equal plus/minus tolerance as separate stacked values */
    DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT                          = 904,

    /** Distance between dimension value and tolerance value expressed as a
                     fraction of text height.  Not used for limit tolerance */
    DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE                 = 905,

    /** Scale factor applied to dimension text size to determine tolerance text size */
    DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE                            = 906,

    /** Not currently used */
    DIMSTYLE_PROP_Tolerance_TextVerticalMargin_DOUBLE                   = 907,

    /** Distance between stacked tolerance values expressed as a
                     fraction of text height */
    DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE               = 908,

    /** Distance in text height units, for upper tolerance value */
    DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE                           = 909,

    /** Accuracy for primary tolerance values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values. */
    DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY                           = 910,

    /** Accuracy for secondary tolerance values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values. */
    DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY                        = 911,

    /** true to display plus or minus sign for zero tolerance */
    DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT                     = 912,

    /*------------------------------------------------------------------------
    Properties that effect formatting of dimension values
    ------------------------------------------------------------------------*/
    /** Accuracy for primary values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values. */
    DIMSTYLE_PROP_Value_Accuracy_ACCURACY                               = 1001,

    /** Accuracy for alternate primary values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values. */
    DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY                            = 1002,

    /** true to enable alternate primary value formatting.  Alternate
                     formatting is activated when the dimension value passes the criteria
                     specified by DIMSTYLE_PROP_Value_AltThreshold_DOUBLE and
                     DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER. */
    DIMSTYLE_PROP_Value_AltIsActive_BOOLINT                             = 1003,

    /** Accuracy for alternate secondary values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values.*/
    DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY                         = 1004,

    /** true to enable alternate secondary value formatting.  Alternate
                     formatting is activated when the dimension value passes the criteria
                     specified by DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE and
                     DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER. */
    DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT                          = 1005,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT                     = 1006,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT                    = 1007,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT                       = 1008,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT                     = 1009,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT            = 1010,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT             = 1011,

    /** false to disable display of alternate secondary master unit value if that value is zero, even if
                     DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT is true. */
    DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT                = 1012,

    /** The threshold distance to determine if alternate value formatting should be used
                     for the primary dimension value. See DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT for more info. */
    DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE                          = 1013,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT                        = 1014,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT                       = 1015,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT                          = 1016,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT                        = 1017,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT               = 1018,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER instead. */
    DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT                = 1019,

    /** false to disable display of alternate master unit value if that value is zero, even if
                     DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT is true. */
    DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT                   = 1020,

    /** The threshold distance to determine if alternate value formatting should be used.
                     See DIMSTYLE_PROP_Value_AltIsActive_BOOLINT for more info. */
    DIMSTYLE_PROP_Value_AltThreshold_DOUBLE                             = 1021,

    /** Controls the units for the display of angle values. See the enumeration
                     ~!tDimStyleProp_Value_AngleFormat for valid values.  */
    DIMSTYLE_PROP_Value_AngleFormat_INTEGER                             = 1023,

    /** true to allow the display of leading zeros for angles, ex. 0.5 vs .5 */
    DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT                        = 1024,

    /** true for angular dimensions to measure the subtended angle.  false to measure arc length. */
    DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT                            = 1025,

    /** The number of decimal places to show for angle values, when angle format is not
                    Degree-Minute-Second. See the enumeration ~!tDimStyleProp_Value_AnglePrecision for
                    valid values. When the angle format is Deg-Min-Sec, refer to the property
                    DIMSTYLE_PROP_Value_DMSAnglePrecision_INTEGER, and the enumeration ~!tDimStyleProp_Value_DMSAnglePrecision. */
    DIMSTYLE_PROP_Value_AnglePrecision_INTEGER                          = 1026,

    /** true to include trailing zeros for angle values.  Ex. 1.5 -> 1.500 */
    DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT                      = 1027,

    /** true to enable DIN style rounding according to the following rules: <pre>
                         values from XX.00 <  val <= XX.25 are rounded to XX
                         values from XX.25 <  val <  XX.75 are rounded to XX.5
                         values from XX.75 <= val <  XX+1  are rounded to XX+1 </pre>
                     This flag is used only if DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT is
                     also true. */
    DIMSTYLE_PROP_Value_RoundLSD_BOOLINT                                = 1028,

    /** Accuracy for secondary dimension values.  See the enumeration
                     ~!tDimStyleProp_Value_Accuracy for valid values.*/
    DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY                            = 1029,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_SecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT                        = 1030,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_SecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT                       = 1031,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_SecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT                          = 1032,

    /** false to suppress display of trailing zeros in secondary dimension values, ex. 1.500 -> 1.5 */
    DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT                    = 1033,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_SecFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT                        = 1034,

    /** false to suppress the display the secondary master unit when its value is zero. */
    DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT                   = 1035,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_Format_INTEGER instead. */
    DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT                           = 1036,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_Format_INTEGER instead. */
    DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT                          = 1037,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_Format_INTEGER instead. */
    DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT                             = 1038,

    /** false to suppress display of trailing zeros in dimension values, ex. 1.500 -> 1.5 */
    DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT                       = 1039,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_Format_INTEGER instead. */
    DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT                           = 1040,

    /** false to suppress the display the master unit when its value is zero. */
    DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT                      = 1041,

    /** true = Do not reduce primary dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT                        = 1042,

    /** true = Do not reduce primary alternate dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT                     = 1043,

    /** true = Do not reduce primary tolerance dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT                     = 1044,

    /** true to enable superscripting of the least significant digit to support the
                     DIN standard that requires this, ex. 1.5 -> 1^5 */
    DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT                          = 1045,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER instead. */
    DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT                      = 1046,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER instead. */
    DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT                          = 1047,

    /** The units of measurement for linear dimensions.  This property is redundant with the
                     following properties which allow the master and sub unit to be controlled independently:
                     DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT and DIMSTYLE_PROP_Value_UnitSub_ONEUNIT.
                     The dimStyle units are only used when DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT
                     is false. */
    DIMSTYLE_PROP_Value_Unit_UNITS                                      = 1048,

    /** The label for master unit values. */
    DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR                         = 1049,

    /** The label for secondary master unit values. */
    DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR                      = 1050,

    /** The label for secondary sub unit values. */
    DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR                         = 1051,

    /** The label for sub unit values. */
    DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR                            = 1052,

    /** The units of measurement for secondary linear dimensions.  This property is redundant with
                     the following properties which allow the master and sub unit to be controlled independently:
                     DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT and DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT. */
    DIMSTYLE_PROP_Value_UnitSec_UNITS                                   = 1053,

    /** true to use working units, false to use dimStyle units.  To access the dimStyle units,
                     see DIMSTYLE_PROP_Value_Unit_UNITS. */
    DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT                         = 1054,

    /** For ordinate dimensions only, display lower values for points to below the datum.  For
                     example, this allows negative values for points to the 'left' of a zero datum. */
    DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT                     = 1055,

    /** For ordinate dimensions only, true to use the datum value.  To access the datum value,
                     see DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE */
    DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT                        = 1056,

    /** For ordinate dimensions only, this value will be displayed for the first dimensioned
                     point.  If DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT is not true, then a datum
                     value of zero is used. */
    DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE                            = 1057,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT                  = 1058,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT                 = 1059,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT                   = 1060,

    /** true = Do not reduce secondary dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT                     = 1061,

    /** true = Do not reduce secondary alternate dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT                  = 1062,

    /** true = Do not reduce secondary tolerance dim fraction to smallest denominator. */
    DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT                  = 1063,

    /** This property is deprecated. Use DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER instead. */
    DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT                 = 1064,

    /** true = Place the ordinate text freely along x and y. */
    DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT                         = 1065,

    /** Controls the formatting of primary linear dimension values. See
                     the enumeration ~!tDimStyleProp_Value_Format for valid values. */
    DIMSTYLE_PROP_Value_Format_INTEGER                                  = 1066,

    /** Controls the formatting of primary alternate linear dimension values. See
                     the enumeration ~!tDimStyleProp_Value_Format for valid values. */
    DIMSTYLE_PROP_Value_AltFormat_INTEGER                               = 1067,

    /** Controls the formatting of secondary linear dimension values. See
                     the enumeration ~!tDimStyleProp_Value_Format for valid values. */
    DIMSTYLE_PROP_Value_SecFormat_INTEGER                               = 1068,

    /** Controls the formatting of secondary alternate linear dimension values. See
                     the enumeration ~!tDimStyleProp_Value_Format for valid values. */
    DIMSTYLE_PROP_Value_AltSecFormat_INTEGER                            = 1069,

    /** Controls how the threshold value is compared to determine if
                     primary alternate formatting should be used. See the enumeration
                     ~!tDimStyleProp_Value_Comparison for valid values. */
    DIMSTYLE_PROP_Value_AltThresholdComparison_INTEGER                  = 1070,

    /** Controls how the threshold value is compared to determine if
                     secondary alternate formatting should be used. See the enumeration
                     ~!tDimStyleProp_Value_Comparison for valid values. */
    DIMSTYLE_PROP_Value_AltSecThresholdComparison_INTEGER               = 1071,

    /** Controls how the thousands place is delimited for metric values.
                     See the enumeration ~!tDimStyleProp_Value_ThousandsOpts for valid values. */
    DIMSTYLE_PROP_Value_ThousandsOpts_INTEGER                           = 1072,

    /** The master unit of measurement for linear dimensions.  This property is
                     redundant with DIMSTYLE_PROP_Value_Unit_UNITS. The dimStyle units are only used
                     when DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT is false. */
    DIMSTYLE_PROP_Value_UnitMaster_ONEUNIT                              = 1073,

    /** The sub unit of measurement for linear dimensions.  This property is
                     redundant with DIMSTYLE_PROP_Value_Unit_UNITS. The dimStyle units are only used
                     when DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT is false. */
    DIMSTYLE_PROP_Value_UnitSub_ONEUNIT                                 = 1074,

    /** The master unit of measurement for secondary linear dimensions.  This property is
                     redundant with DIMSTYLE_PROP_Value_UnitSec_UNITS. */
    DIMSTYLE_PROP_Value_SecUnitMaster_ONEUNIT                           = 1075,

    /** The sub unit of measurement for secondary linear dimensions.  This property is
                     redundant with DIMSTYLE_PROP_Value_UnitSec_UNITS. */
    DIMSTYLE_PROP_Value_SecUnitSub_ONEUNIT                              = 1076,

    /** Controls how the label line dimension's length and angle values are formatted.
                     See the enumeration ~!tDimStyleProp_Value_LabelLineFormat for valid values. */
    DIMSTYLE_PROP_Value_LabelLineFormat_INTEGER                         = 1077,

    /** false to suppress the display the sub unit when its value is zero. */
    DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT                         = 1078,

    /** false to suppress the display the alternate sub unit when its value is zero. */
    DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT                      = 1079,

    /** false to suppress the display the secondary sub unit when its value is zero. */
    DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT                      = 1080,

    /** false to suppress the display the secondary alternate sub unit when its value is zero. */
    DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT                   = 1081,

    /** Changes the way that DIMSTYLE_PROP_Value_AnglePrecision_INTEGER is interpreted for
                     angular dimensions in Degree Minute Seconds mode. See the enumeration
                     ~!tDIMSTYLE_PROP_Value_DMSPrecisionMode for valid values. */
    DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER                        = 1082,

    /** Round off value for primary units */
    DIMSTYLE_PROP_Value_RoundOff_DOUBLE                                 = 1083,

    /** Round off value for secondary units */
    DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE                              = 1084,

    /** Display a space after a nonstacked fraction, ex. 1/2_" vs 1/2" */
    DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT            = 1085,

    //__PUBLISH_SECTION_END__
    //These are values for legacy support. Do not use them unless for round tripping
    DIMSTYLE_PROP_General_ShowCenterMarkLeft_TEMPLATEFLAG               =1086,
    DIMSTYLE_PROP_General_ShowCenterMarkRight_TEMPLATEFLAG              =1087,
    DIMSTYLE_PROP_General_ShowCenterMarkTop_TEMPLATEFLAG                =1088,
    DIMSTYLE_PROP_General_ShowCenterMarkBottom_TEMPLATEFLAG             =1089,
    //__PUBLISH_SECTION_START__
    } DimStyleProp;

/*------------------------------------------------------------------------------
    These categories are used to organize the dimstyle properties into a tree.
    They are also the IDs of the string list resources that define the tree.
------------------------------------------------------------------------------*/
enum DimStyleProp_Category
    {
    DIMSTYLE_PROPCATEGORY_Root                           = -10000,

    DIMSTYLE_PROPCATEGORY_General                        = -10100,
    DIMSTYLE_PROPCATEGORY_Placement                      = -10101,
    DIMSTYLE_PROPCATEGORY_BallAndChain                   = -10102,
    DIMSTYLE_PROPCATEGORY_Tolerance                      = -10103,

    DIMSTYLE_PROPCATEGORY_Value                          = -10200,
    DIMSTYLE_PROPCATEGORY_Angle                          = -10201,
    DIMSTYLE_PROPCATEGORY_Metric                         = -10202,
    DIMSTYLE_PROPCATEGORY_Fractions                      = -10203,
    DIMSTYLE_PROPCATEGORY_Primary                        = -10204,
    DIMSTYLE_PROPCATEGORY_PrimaryAlt                     = -10205,
    DIMSTYLE_PROPCATEGORY_Secondary                      = -10206,
    DIMSTYLE_PROPCATEGORY_SecondaryAlt                   = -10207,
    DIMSTYLE_PROPCATEGORY_DINDimensioning                = -10208,

    DIMSTYLE_PROPCATEGORY_Graphics                       = -10300,
    DIMSTYLE_PROPCATEGORY_DimLines                       = -10301,
    DIMSTYLE_PROPCATEGORY_ExtLines                       = -10302,
    DIMSTYLE_PROPCATEGORY_Terminators                    = -10303,
    DIMSTYLE_PROPCATEGORY_FitOptions                     = -10304,

    DIMSTYLE_PROPCATEGORY_Text                           = -10400,
    DIMSTYLE_PROPCATEGORY_Format                         = -10401,
    DIMSTYLE_PROPCATEGORY_Style                          = -10402,

    DIMSTYLE_PROPCATEGORY_Notes                          = -10500,

    DIMSTYLE_PROPCATEGORY_Symbols                        = -10600,
    DIMSTYLE_PROPCATEGORY_Prefix                         = -10601,
    DIMSTYLE_PROPCATEGORY_Suffix                         = -10602,
    DIMSTYLE_PROPCATEGORY_Diameter                       = -10603,
    DIMSTYLE_PROPCATEGORY_PlusMinus                      = -10604,
    DIMSTYLE_PROPCATEGORY_ArrowTerm                      = -10605,
    DIMSTYLE_PROPCATEGORY_StrokeTerm                     = -10606,
    DIMSTYLE_PROPCATEGORY_OriginTerm                     = -10607,
    DIMSTYLE_PROPCATEGORY_DotTerm                        = -10608,
    DIMSTYLE_PROPCATEGORY_NoteTerm                       = -10609,

    DIMSTYLE_PROPCATEGORY_Tools                          = -10700,
    DIMSTYLE_PROPCATEGORY_LinearTool                     = -10701,
    DIMSTYLE_PROPCATEGORY_AngleTool                      = -10702,
    DIMSTYLE_PROPCATEGORY_ArcSizeTool                    = -10703,
    DIMSTYLE_PROPCATEGORY_AngleLocTool                   = -10704,
    DIMSTYLE_PROPCATEGORY_RadialTool                     = -10705,
    DIMSTYLE_PROPCATEGORY_OrdinateTool                   = -10706,
    };

/*------------------------------------------------------------------------------
    These are all the possible types for a dimension style property
------------------------------------------------------------------------------*/
enum DimStyleProp_Type
    {
    PROPTYPE_None                                        =  0,
    PROPTYPE_Accuracy                                    =  1,
    PROPTYPE_BoolInt                                     =  2,
    PROPTYPE_Char                                        =  3,
    PROPTYPE_Color                                       =  4,
    PROPTYPE_Distance                                    =  5,
    PROPTYPE_Double                                      =  6,
    PROPTYPE_Integer                                     =  7,
    PROPTYPE_Font                                        =  8,
    PROPTYPE_Level                                       =  9,
    PROPTYPE_LineStyle                                   = 10,
    PROPTYPE_MSWChar                                     = 11,
    PROPTYPE_OneUnit                                     = 12,
    PROPTYPE_TemplateFlag                                = 13,
    PROPTYPE_Units                                       = 14,
    PROPTYPE_Weight                                      = 15,
    };

enum DimensionPartType
    {
    ADTYPE_INHERIT         = 0,
    ADTYPE_EXT_LEFT        = 1,
    ADTYPE_EXT_RIGHT       = 2,
    ADTYPE_TEXT_UPPER      = 3,
    ADTYPE_TEXT_LOWER      = 4,
    ADTYPE_TEXT_SINGLE     = 5,
    ADTYPE_TERM_LEFT       = 6,
    ADTYPE_TERM_RIGHT      = 7,
    ADTYPE_DIMLINE         = 8,
    ADTYPE_CENTER          = 9,
    ADTYPE_TEXT_SYMBOLS    = 10,
    ADTYPE_CHAIN           = 11
    };

enum DimensionPartSubType
    {
    ADSUB_NONE             = 0,
    ADSUB_TOL_UPPER        = 1,
    ADSUB_TOL_LOWER        = 2,
    ADSUB_TOL_SINGLE       = 3,
    ADSUB_LIM_UPPER        = 4,
    ADSUB_LIM_LOWER        = 5,
    ADSUB_LIM_SINGLE       = 6,
    ADSUB_PREFIX           = 7,
    ADSUB_SUFFIX           = 8,
    ADSUB_TERMSYMBOL       = 9,
    ADSUB_LEADER           = 10
    };

enum DimensionTextPartType
    {
    DIMTEXTPART_Primary    = ADTYPE_TEXT_UPPER,
    DIMTEXTPART_Secondary  = ADTYPE_TEXT_LOWER,
    };

enum DimensionTextPartSubType
    {
    DIMTEXTSUBPART_Main            = ADSUB_NONE,
    DIMTEXTSUBPART_Tolerance_Plus  = ADSUB_TOL_UPPER,
    DIMTEXTSUBPART_Tolerance_Minus = ADSUB_TOL_LOWER,
    DIMTEXTSUBPART_Limit_Upper     = ADSUB_LIM_UPPER,
    DIMTEXTSUBPART_Limit_Lower     = ADSUB_LIM_LOWER,
    };

//__PUBLISH_SECTION_END__

/* StringList IDs from -10000 to -19999 RESERVED for dimstyles  */
/* IDs listed in enum DimStyleProp_Catagories, in midimstyle.h  */

enum DimStyleProp_ValueList
    {
    STRINGID_DSPropVal_INVALID                          =    0,
    STRINGID_DSPropVal_BallAndChain_Alignment           = -100,
    STRINGID_DSPropVal_BallAndChain_ChainType           = -101,
    STRINGID_DSPropVal_General_Alignment                = -102,
    STRINGID_DSPropVal_General_RadialMode               = -103,
    STRINGID_DSPropVal_MLNote_FrameType                 = -104,
    STRINGID_DSPropVal_MLNote_Justification             = -105,
    STRINGID_DSPropVal_MLNote_VerticalJustification     = -106,
    STRINGID_DSPropVal_MLNote_TextRotation              = -107,
    STRINGID_DSPropVal_MLNote_HorAttachment             = -108,
    STRINGID_DSPropVal_MLNote_VerAttachment             = -109,
    STRINGID_DSPropVal_Symbol_Standard                  = -110,
    STRINGID_DSPropVal_Symbol_CustomType                = -111,
    STRINGID_DSPropVal_Symbol_PreSufType                = -112,
    STRINGID_DSPropVal_Symbol_TermType                  = -113,
    STRINGID_DSPropVal_Terminator_Mode                  = -114,
    STRINGID_DSPropVal_Terminator_Arrowhead             = -115,
    STRINGID_DSPropVal_Text_Justification               = -116,
    STRINGID_DSPropVal_Value_AngleFormat                = -117,
    STRINGID_DSPropVal_Value_AnglePrecision             = -118,
    STRINGID_DSPropVal_Text_StackedFractionAlignment    = -119,
    STRINGID_DSPropVal_Text_StackedFractionType         = -120,
    STRINGID_DSPropVal_Placement_TextPosition           = -121,
    STRINGID_DSPropVal_Text_Location                    = -122,
    STRINGID_DSPropVal_Accuracy                         = -123,
    STRINGID_DSPropVal_Terminator_TypeLeft              = -124,
    STRINGID_DSPropVal_Terminator_TypeRight             = -125,
    STRINGID_DSPropVal_Terminator_TypeFirst             = -126,
    STRINGID_DSPropVal_Terminator_TypeJoint             = -127,
    STRINGID_DSPropVal_Terminator_TypeNote              = -128,
    STRINGID_DSPropVal_RadialTool_Leader                = -129,
    STRINGID_DSPropVal_Value_Format                     = -130,
    STRINGID_DSPropVal_Value_Comparison                 = -131,
    STRINGID_DSPropVal_Text_Vertical                    = -132,
    STRINGID_DSPropVal_Value_ThousandsOpts              = -133,
    STRINGID_DSPropVal_OnOff                            = -134,
    STRINGID_DSPropVal_Text_Horizontal                  = -135,
    STRINGID_DSPropVal_Value_AngleMeasure               = -136,
    STRINGID_DSPropVal_Text_FrameType                   = -137,
    STRINGID_DSPropVal_Tolerance_Mode                   = -138,
    STRINGID_DSPropVal_MLNote_LeaderType                = -139,
    STRINGID_DSPropVal_Value_LabelLineFormat            = -140,
    STRINGID_DSPropVal_Index                            = -141,
    STRINGID_DSPropVal_Text_SuperScriptMode             = -142,
    STRINGID_DSPropVal_FitOption                        = -143,
    STRINGID_DSPropVal_BallAndChain_Mode                = -144,
    STRINGID_DSPropVal_FitInclinedTextBox               = -145,
    };

#define ICONID_DimTmplBlank                     (-107)
#define ICONID_DimTmplNone                      (-108)
#define ICONID_DimTmplLeftArrow                 (-109)
#define ICONID_DimTmplRightArrow                (-110)
#define ICONID_DimTmplJointNone                 (-111)
#define ICONID_DimTmplLeftNone                  (-112)
#define ICONID_DimTmplRightNone                 (-113)
#define ICONID_DimTmplLeftSlash1                (-114)
#define ICONID_DimTmplRightSlash1               (-115)
#define ICONID_DimTmplJointSlash                (-116)
#define ICONID_DimTmplLeftSlash2                (-117)
#define ICONID_DimTmplRightSlash2               (-118)
#define ICONID_DimTmplLeftCommon                (-119)
#define ICONID_DimTmplRightCommon               (-120)
#define ICONID_DimTmplJointCommon               (-121)
#define ICONID_DimTmplLeftDot                   (-122)
#define ICONID_DimTmplRightDot                  (-123)
#define ICONID_DimTmplCommonDot                 (-124)
#define ICONID_DimTmplSymDiameter               (-125)
#define ICONID_DimTmplSymRadius                 (-126)
#define ICONID_DimTmplSymSpDiameter             (-127)
#define ICONID_DimTmplSymSpRaddius              (-128)
#define ICONID_DimTmplSquare                    (-129)
#define ICONID_DimTmplLeaderOut                 (-130)
#define ICONID_DimTmplLeaderOutExt              (-131)
#define ICONID_DimTmplLeaderIn                  (-132)
#define ICONID_DimTmplLeaderInExt               (-133)
#define ICONID_DimTermNote                      (-134)

//__PUBLISH_SECTION_START__

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
