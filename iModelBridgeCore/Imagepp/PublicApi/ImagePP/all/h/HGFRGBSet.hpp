//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRGBSet.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HGFRGBSet
//---------------------------------------------------------------------------

// Order of declaration of methods is important for proper inlining.
BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool HGFRGBSet::IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const
    {
    return IsIn(pi_R, pi_G, pi_B, 7);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HGFRGBSet::Add(Byte pi_R, Byte pi_G, Byte pi_B)
    {
    Add(pi_R, pi_G, pi_B, 7);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HGFRGBSet::Remove(Byte pi_R, Byte pi_G, Byte pi_B)
    {
    Remove(pi_R, pi_G, pi_B, 7);
    }

END_IMAGEPP_NAMESPACE