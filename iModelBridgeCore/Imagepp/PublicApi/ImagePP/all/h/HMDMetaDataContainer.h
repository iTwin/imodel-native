//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaDataContainer.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

class HMDMetaDataContainer : public HFCShareableObject<HMDMetaDataContainer>
    {
    HDECLARE_BASECLASS_ID(7005);

public :

    typedef enum
        {
        HMD_LAYER_INFO,
        HMD_GEOCODING_INFO,
        HMD_ANNOTATION_INFO,
        HMD_ANNOT_ICON_RASTERING_INFO,
        HMD_PDF_DRAW_OPTIONS
        } Type;

    _HDLLu HMDMetaDataContainer(Type pi_Type);
    _HDLLu virtual ~HMDMetaDataContainer();

    _HDLLu HMDMetaDataContainer(const HMDMetaDataContainer& pi_rObj);

    _HDLLu HMDMetaDataContainer::Type    GetType()    const;
    _HDLLu bool                         HasChanged() const;
    _HDLLu void                          SetModificationStatus(bool pi_HasChanged);

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
    HDECLARE_CLASS_ID(7015, HMDDrawOptionsPDF);

public :
    _HDLLu          HMDDrawOptionsPDF();
    _HDLLu  virtual ~HMDDrawOptionsPDF();

    _HDLLu  virtual HFCPtr<HMDMetaDataContainer> Clone() const override;

    _HDLLu  bool GetSmoothText() const;
    _HDLLu  void SetSmoothText(bool val);

    _HDLLu  bool GetSmoothLineArt() const;
    _HDLLu  void SetSmoothLineArt(bool val);

    _HDLLu  bool GetSmoothImage() const;
    _HDLLu  void SetSmoothImage(bool val);

private :
    HMDDrawOptionsPDF(const HMDDrawOptionsPDF& pi_rObj);
    HMDDrawOptionsPDF& operator=(const HMDDrawOptionsPDF& pi_rObj);

    bool m_smoothText;
    bool m_smoothLineArt;
    bool m_smoothImage;
    };