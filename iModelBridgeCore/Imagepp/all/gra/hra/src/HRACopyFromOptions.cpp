//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRACopyFromOptions
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRACopyFromOptions.h>


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImagePP::HRACopyFromOptionsImpl
{
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRACopyFromOptionsImpl(bool alphaBlend)
    :m_resampling(HGSResampling::NEAREST_NEIGHBOUR)
        {   
        m_pDestShape = NULL;
        m_pEffectiveCopyRegion = NULL;
        m_pDestReplacingPixelType = NULL;
        m_alphaBlend = false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRACopyFromOptionsImpl(HRACopyFromOptionsImpl const& opts)
    :m_resampling(opts.m_resampling)
        {
        m_pDestShape = opts.m_pDestShape;
        m_pEffectiveCopyRegion = opts.m_pEffectiveCopyRegion;
        m_pDestReplacingPixelType = opts.m_pDestReplacingPixelType;
        m_alphaBlend = opts.m_alphaBlend;
        }

    ~HRACopyFromOptionsImpl(){};

    bool ApplyAlphaBlend() const {return m_alphaBlend;}
    void SetAlphaBlend(bool alphaBlend){m_alphaBlend=alphaBlend;}

    HFCPtr<HRPPixelType> GetDestReplacingPixelType() const {return m_pDestReplacingPixelType;}
    void SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pPixelType) {m_pDestReplacingPixelType = pPixelType;}

    //! Get/Set an extra destination shape. If an effectiveCopyRegion is set this parameter is ignored.
    HVEShape const* GetDestShape() const {return m_pDestShape;}

    void SetDestShape(HVEShape const* pShape) 
        {
        HPRECONDITION(GetEffectiveCopyRegion() == NULL);      // Can't have both at the same time.
        m_pDestShape=pShape;
        }

    HVEShape const* GetEffectiveCopyRegion() const {return m_pEffectiveCopyRegion;}

    void SetEffectiveCopyRegion(HVEShape const* pShape) 
        {
        HPRECONDITION(GetDestShape() == NULL);      // Can't have both at the same time.
        m_pEffectiveCopyRegion=pShape;
        }

    HGSResampling const& GetResamplingMode() const {return m_resampling;}
    void SetResamplingMode(HGSResampling const& resampling) {m_resampling = resampling;}
    
private:
    HVEShape const*       m_pDestShape;  
    HVEShape const*       m_pEffectiveCopyRegion;
    HFCPtr<HRPPixelType>  m_pDestReplacingPixelType;
    bool                  m_alphaBlend;
    HGSResampling         m_resampling;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyFromOptions::HRACopyFromOptions(bool alphaBlend/*=true*/)
:m_pImpl(new HRACopyFromOptionsImpl(alphaBlend))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyFromOptions::HRACopyFromOptions(HRACopyFromOptions const& opts)
:m_pImpl(new HRACopyFromOptionsImpl(*opts.m_pImpl))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyFromOptions& HRACopyFromOptions::operator=(const HRACopyFromOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        *m_pImpl = *pi_rObj.m_pImpl;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRACopyFromOptions::~HRACopyFromOptions(){m_pImpl.reset();}

bool HRACopyFromOptions::ApplyAlphaBlend() const {return m_pImpl->ApplyAlphaBlend();}
void HRACopyFromOptions::SetAlphaBlend(bool alphaBlend) {m_pImpl->SetAlphaBlend(alphaBlend);}

HFCPtr<HRPPixelType> HRACopyFromOptions::GetDestReplacingPixelType() const {return m_pImpl->GetDestReplacingPixelType();}
void HRACopyFromOptions::SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pPixelType) {m_pImpl->SetDestReplacingPixelType(pPixelType);}

HVEShape const* HRACopyFromOptions::GetDestShape() const {return m_pImpl->GetDestShape();}
void HRACopyFromOptions::SetDestShape(HVEShape const* pShape) {m_pImpl->SetDestShape(pShape);}

HVEShape const* HRACopyFromOptions::GetEffectiveCopyRegion() const {return m_pImpl->GetEffectiveCopyRegion();}
void HRACopyFromOptions::SetEffectiveCopyRegion(HVEShape const* pShape) {m_pImpl->SetEffectiveCopyRegion(pShape);}

HGSResampling const& HRACopyFromOptions::GetResamplingMode() const {return m_pImpl->GetResamplingMode();}
void HRACopyFromOptions::SetResamplingMode(HGSResampling const& resampling) {m_pImpl->SetResamplingMode(resampling);}

