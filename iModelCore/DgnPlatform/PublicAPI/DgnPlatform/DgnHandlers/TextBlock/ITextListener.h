/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/ITextListener.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include "TextAPICommon.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector<WChar> WCharArray;

//=======================================================================================
// @bsiclass                                                    Jyoti.Swarup   07/06
//=======================================================================================
struct ITextListener : public RefCountedBase
    {
    public: virtual         ~ITextListener      () { }

    public: virtual void    PlayDocBegin        (TextParamAndScaleP, UInt32 linelength) = 0;
    public: virtual void    PlayDocEnd          () = 0;
    public: virtual void    PlayParagraphBegin  (double paraIndent, double firstLineIndent, T_DoubleVectorCR tabStops, bool paraEmpty) = 0;
    public: virtual void    PlayParagraphEnd    () = 0;
    public: virtual void    PlayTab             (double height, double width) = 0;
    public: virtual void    PlayLinefeed        () = 0;
    public: virtual void    PlayParagraphBreak  () = 0;
    public: virtual void    PlayText            (TextParamAndScaleP, WCharArray& text) = 0;
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    public: virtual void    PlayField           (TextParamAndScaleP, WCharArray const& spec, WCharArray const& fieldStr) = 0;
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    public: virtual void    PlayEnterDataField  (TextParamAndScaleP, EdfJustification just, WCharArray const& contents) = 0;
    public: virtual void    PlayStaticFraction  (TextParamAndScaleP, int numerator, int denominator) = 0;
    public: virtual void    PlayFraction        (TextParamAndScaleP, StackedFractionType, StackedFractionAlignment, DPoint2dCP fractionScale, WStringR numContents, WStringR denomContents) = 0;

    }; // ITextListener

END_BENTLEY_DGNPLATFORM_NAMESPACE
