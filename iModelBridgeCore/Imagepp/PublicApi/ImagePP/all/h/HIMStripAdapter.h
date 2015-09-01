//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMStripAdapter
//-----------------------------------------------------------------------------
// This class describes the interface for any kind of image view.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"
#include "HRABitmap.h"

BEGIN_IMAGEPP_NAMESPACE
class HIMStripAdapterIterator;

/*---------------------------------------------------------------------------------**//**
* HIMStripAdapter
+---------------+---------------+---------------+---------------+---------------+------*/
class HIMStripAdapter : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT, HIMStripAdapterId)

    friend class HIMStripAdapterIterator;

    public:

        //:> Primary methods
        IMAGEPP_EXPORT                 HIMStripAdapter();

        IMAGEPP_EXPORT                 HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                                      const Byte*                 pi_pRGBBackgroundColor,
                                                      double                      pi_QualityFactor = 1.0,
                                                      size_t                      pi_MaxSizeInBytes = (1024 * 1024L));

        IMAGEPP_EXPORT                 HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                                      const HFCPtr<HRPPixelType>& pi_rpStripPixelType,
                                                      double                      pi_QualityFactor = 1.0,
                                                      size_t                      pi_MaxSizeInBytes = (1024 * 1024L));

        IMAGEPP_EXPORT                 HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                                      const Byte*                 pi_pRGBBackgroundColor,
                                                      uint32_t                    pi_StripWidth,
                                                      uint32_t                    pi_StripHeigth);

        IMAGEPP_EXPORT                 HIMStripAdapter(const HIMStripAdapter& pi_rObj);

        IMAGEPP_EXPORT virtual         ~HIMStripAdapter();


        //:> Overriden methods
        virtual HPMPersistentObject* Clone() const;
        virtual HFCPtr<HRARaster>    Clone(HPMObjectStore* pi_pStore, HPMPool* pi_pLog = 0) const override;

        virtual HRARasterIterator*   CreateIterator(const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;
        virtual bool                 IsStoredRaster() const;
        virtual HFCPtr<HRPPixelType> GetPixelType() const;

        //:> Added methods
        HFCPtr<HRABitmap>            GetInputBitmapExample() const;
        size_t                       GetMaxSizeInBytes() const;
        void                         SetMaxSizeInBytes(size_t pi_MaxSize);
        double                       GetQualityFactor() const;
        void                         SetQualityFactor(double pi_Factor);
        bool                         StripsWillBeClipped() const;
        void                         ClipStripsBasedOnSource(bool pi_Clip);

    protected:
        virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;
        virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;


    private:
        size_t                m_MaxSizeInBytes;
        double                m_QualityFactor;
        HFCPtr<HRABitmap>     m_pInputBitmapExample;
        bool                  m_ApplyClipping;
        uint32_t              m_StripWidth;
        uint32_t              m_StripHeigth;

        uint32_t    SetBackgroundColor(const HFCPtr<HRPPixelType>& pio_pPixelType,
                                       const HFCPtr<HRARaster>&    pi_pSource,
                                       const Byte*                 pi_pRGBBackgroundColor);
    };

END_IMAGEPP_NAMESPACE
#include "HIMStripAdapter.hpp"
