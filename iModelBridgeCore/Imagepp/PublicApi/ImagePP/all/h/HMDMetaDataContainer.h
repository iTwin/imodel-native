//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaDataContainer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HMDMetaDataContainer : public HFCShareableObject<HMDMetaDataContainer>
    {
    HDECLARE_BASECLASS_ID(HMDMetaDataId_Container);

public :

    typedef enum
        {
        HMD_LAYER_INFO,
        HMD_ANNOTATION_INFO,
        HMD_ANNOT_ICON_RASTERING_INFO,
        HMD_PDF_DRAW_OPTIONS
        } Type;

    IMAGEPP_EXPORT HMDMetaDataContainer(Type pi_Type);
    IMAGEPP_EXPORT virtual ~HMDMetaDataContainer();

    IMAGEPP_EXPORT HMDMetaDataContainer(const HMDMetaDataContainer& pi_rObj);

    IMAGEPP_EXPORT HMDMetaDataContainer::Type    GetType()    const;
    IMAGEPP_EXPORT bool                         HasChanged() const;
    IMAGEPP_EXPORT void                          SetModificationStatus(bool pi_HasChanged);

    virtual HFCPtr<HMDMetaDataContainer> Clone() const = 0;

protected :

    bool m_HasChanged;

private :
    HMDMetaDataContainer& operator=(const HMDMetaDataContainer& pi_rObj);

    Type  m_Type;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HMDDrawOptionsPDF : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(HMDDrawOptionsPDFId, HMDDrawOptionsPDF);

public :
    IMAGEPP_EXPORT          HMDDrawOptionsPDF();
    IMAGEPP_EXPORT  virtual ~HMDDrawOptionsPDF();

    IMAGEPP_EXPORT  virtual HFCPtr<HMDMetaDataContainer> Clone() const override;

    IMAGEPP_EXPORT  bool GetSmoothText() const;
    IMAGEPP_EXPORT  void SetSmoothText(bool val);

    IMAGEPP_EXPORT  bool GetSmoothLineArt() const;
    IMAGEPP_EXPORT  void SetSmoothLineArt(bool val);

    IMAGEPP_EXPORT  bool GetSmoothImage() const;
    IMAGEPP_EXPORT  void SetSmoothImage(bool val);

private :
    HMDDrawOptionsPDF(const HMDDrawOptionsPDF& pi_rObj);
    HMDDrawOptionsPDF& operator=(const HMDDrawOptionsPDF& pi_rObj);

    bool m_smoothText;
    bool m_smoothLineArt;
    bool m_smoothImage;
    };
END_IMAGEPP_NAMESPACE