/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ECImageProvider.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <Bentley\BeIconUtilities.h>
#include "ecimagekey.h"
BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECNativeImage : public RefCountedBase
    {
    typedef BeIconUtilities::Icon   NativeImage;
    typedef NativeImage*            NativeImageP;
    typedef NativeImage&            NativeImageR;

    private:
        ECImageKey      m_key;
        NativeImageP    m_nativeImage;
        DPoint2d        m_imageSize;
    
    protected:
        IECNativeImage (ECImageKeyCR key, NativeImageR image, DPoint2dCR imageSize)
            :m_key(key), m_nativeImage(&image), m_imageSize(imageSize)
            {}

    public:
        virtual ~IECNativeImage ()
            {
            }

        NativeImageP    GetImage () const {return m_nativeImage;}
        ECImageKeyCR    GetImageKey() const {return m_key;}
        double          GetHeight () const {return m_imageSize.x;}
        double          GetWidth () const {return m_imageSize.y;}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECNativeImage :public IECNativeImage
    {
    private:
        ECNativeImage (ECImageKeyCR key, NativeImageR image, DPoint2dCR imageSize)
        :IECNativeImage (key, image, imageSize)
        {}
    
    public:

    static IECNativeImagePtr CreateNativeImage(ECImageKeyCR key, NativeImageR image, DPoint2dCR imageSize)
        {
        return new ECNativeImage(key, image, imageSize);
        }
    
#pragma warning(disable:4189) // status unused if NDEBUG set.
    ~ECNativeImage ()
        {
        BentleyStatus status = BeIconUtilities::DestroyIcon(GetImage());
        if (SUCCESS != status)
            BeAssert(SUCCESS == status);
        }
    };
#pragma warning(default:4189)

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECPresentationImageProvider : public IECPresentationProvider
    {
    protected:
        virtual ProviderType      _GetProviderType() const override {return ImageService;}
        virtual IECNativeImagePtr _GetImage (ECImageKeyCR imageKey, DPoint2dCR size) = 0;

    public:
        ECOBJECTS_EXPORT IECNativeImagePtr GetImage (ECImageKeyCR imageKey, DPoint2dCR size);
    };

END_BENTLEY_EC_NAMESPACE
