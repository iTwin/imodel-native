//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMTranslucentImageCreator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRPAlphaRange.h"
#include "HRARaster.h"

BEGIN_IMAGEPP_NAMESPACE

class HIMTranslucentImageCreator
    {
    HDECLARE_SEALEDCLASS_ID(HIMTranslucentImageCreatorId_Base)

public:

    // The TranslucencyMethod indicates how the translucency is applied
    enum TranslucencyMethod
        {
        REPLACE,    // The alpha of the source is replaced by the applied alpha value
        COMPOSE     // The alpha of the source is composed with the applied alpha value
        };


    // Primary methods

    IMAGEPP_EXPORT                 HIMTranslucentImageCreator(const HFCPtr<HRARaster>& pi_pSource,
                                                      Byte pi_DefaultAlpha = 255);
    IMAGEPP_EXPORT                ~HIMTranslucentImageCreator();



    Byte          GetDefaultAlpha() const;
    void            SetDefaultAlpha(Byte pi_DefaultValue);

    void            AddAlphaRange(const HRPAlphaRange& pi_rRange);
    const ListHRPAlphaRange&
    GetAlphaRanges() const;
    void            RemoveAllRanges();

    IMAGEPP_EXPORT void                 SetTranslucencyMethod(TranslucencyMethod pi_Method);
    IMAGEPP_EXPORT TranslucencyMethod   GetTranslucencyMethod() const;

    IMAGEPP_EXPORT HRARaster*      CreateTranslucentRaster(Byte const* pi_pAlphaValues = 0) const;


private:

    void            InitObject(Byte pi_AlphaValue = 255);

    TranslucencyMethod
    m_TranslucencyMethod;
    Byte          m_DefaultAlpha;
    ListHRPAlphaRange
    m_Ranges;
    HFCPtr<HRARaster>
    m_pSourceRaster;


    // Disabled
    HIMTranslucentImageCreator(const HIMTranslucentImageCreator& pi_rObj);
    HIMTranslucentImageCreator&
    operator=(const HIMTranslucentImageCreator& pi_rObj);
    };


END_IMAGEPP_NAMESPACE