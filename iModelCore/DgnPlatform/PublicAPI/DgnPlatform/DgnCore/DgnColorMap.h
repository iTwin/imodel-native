/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnColorMap.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ColorUtil.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnColorMap
{
    friend struct DgnColors;
public:

enum Entries
    {
//__PUBLISH_SECTION_END__
    INDEX_Count      = 258, //!< internal use. Note: INDEX_Count is padded beyond INDEX_ByLevel so the color entries can be used as a "light-weight" BSIColorDescr.
    INDEX_ByLevel    = 257, //!< internal use
    INDEX_ByCell     = 256, //!< internal use
//__PUBLISH_SECTION_START__
    INDEX_ColorCount = 256, //!< number of colors in color map
    INDEX_Background = 255, //!< index of background color in map
    INDEX_Invalid    = -1,  //!< index for an rgb that isn't from a color map or extended color list
    };

//__PUBLISH_SECTION_END__
protected:
    typedef bmap<IntColorDef, UInt32> TrueColorMap;
    UInt32  m_colors[INDEX_Count];
    mutable TrueColorMap    m_trueColorCache;    // resolved truecolorid -> color values
    mutable DgnTrueColorId  m_nextTrueColorID;   // next available id
    DgnTrueColorId UseNextTrueColorId(DgnProjectR project) const;
    
public:
    DGNPLATFORM_EXPORT void SetupDefaultColors();
    DGNPLATFORM_EXPORT bool IsSame (DgnColorMapCP otherMap, bool ignoreBG = true) const;
    DGNPLATFORM_EXPORT void AdjustValueAndSaturation (double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed);
    static bool CanBeTrueColor(UInt32 elementColor);
    bool IsTrueColor (DgnProjectR project, IntColorDef& colorDef, UInt32 elementColor) const;
    DgnTrueColorId GetNextTrueColorId(DgnProjectR project) const;

    UInt32* GetTbgrColorsP () {return m_colors;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Called to return color map entries as an array of TBGR values.
    //! @remarks Background color is INDEX_Background. This is the most efficient way to access the color map entries.
    //! @return Direct pointer to color entries, not to be freed by caller.
    DGNPLATFORM_EXPORT UInt32 const* GetTbgrColors() const;

    //! Called to return color map entries converted to an array of RgbColorDef.
    //! @param[out]     colors      Array of RBGColorDef of size INDEX_ColorCount
    //! @remarks Generally used by some older api functions, background color is expected to be index 0.
    DGNPLATFORM_EXPORT void GetRgbColors (RgbColorDef* colors) const; // Background color is index 0

    //! Called to return color map entries converted to an array of HsvColorDef.
    //! @param[out]     colors      Array of HsvColorDef of size INDEX_ColorCount
    //! @remarks Generally used by some older api functions, background color is expected to be index 0.
    DGNPLATFORM_EXPORT void GetHsvColors (HsvColorDef* colors) const; // Background color is index 0

    //! Called to return the information for a specific color index as an IntColorDef.
    //! @param[in]      index       Which color index to return the color information for.
    //! @remarks An index of -1 can also be used to return the background color, INDEX_Background.
    //! The IntColorDef is convenient for accessing the color either as a TBGR or as an RgbColorDef.
    //! @return IntColor for specified color index.
    DGNPLATFORM_EXPORT IntColorDef const& GetColor (int index) const;

    //! Called to return the element color index of the closest matching color map entry to the supplied IntColorDef.
    //! @param[in]      colorDef            Color to find the closest match for.
    //! @param[in]      preComputedHSVTable Optional hsv map for this color table (will be computed if NULL).
    //! @see GetHsvColors.
    //! @return Element color index for closest match.
    DGNPLATFORM_EXPORT UInt32 FindClosestMatch (IntColorDef const& colorDef, HsvColorDef* preComputedHSVTable = NULL) const;

    //! Set color map from array of RgbColorDef.
    //! @param[in] colors Array of RBGColorDef of size INDEX_ColorCount.
    DGNPLATFORM_EXPORT void SetupFromRgbColors (RgbColorDef const* colors);

    //! Called to get the color map used for a DgnProject.
    //! @param[in] project The project to return the color map for.
    //! @remarks This is the stored or default color map for the project, it is used by all models.
    //! @return The color map, should never be NULL for a valid project.
    static DGNPLATFORM_EXPORT DgnColorMapCP Get (DgnProjectCR project);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
