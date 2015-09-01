//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRACopyFromOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRACopyFromOptions
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRACopyFromOptions.h>


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


//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//                                      HRACopyFromLegacyOptions
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions::HRACopyFromLegacyOptions(const HRACopyFromLegacyOptions& pi_rOptions)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    *this = pi_rOptions;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions::HRACopyFromLegacyOptions()
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = 0;
    m_GridShapeMode = false;

    m_AlphaBlend = false;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions::HRACopyFromLegacyOptions(bool pi_AlphaBlend)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = 0;
    m_GridShapeMode = false;

    m_AlphaBlend = pi_AlphaBlend;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }


//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions::HRACopyFromLegacyOptions(const HVEShape* pi_pDestShape,
                                       bool pi_AlphaBlend)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = pi_pDestShape;
    m_GridShapeMode = false;

    m_AlphaBlend = pi_AlphaBlend;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions::~HRACopyFromLegacyOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRACopyFromLegacyOptions& HRACopyFromLegacyOptions::operator=(const HRACopyFromLegacyOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pDestShape = pi_rObj.m_pDestShape;
        m_GridShapeMode = pi_rObj.m_GridShapeMode;

        m_AlphaBlend = pi_rObj.m_AlphaBlend;

        m_Resampling = pi_rObj.m_Resampling;

        m_MaxResolutionStretchingFactor = pi_rObj.m_MaxResolutionStretchingFactor;

        m_ApplySourceClipping = pi_rObj.m_ApplySourceClipping;
        m_OverviewMode        = pi_rObj.m_OverviewMode;

        m_pDestReplacingPixelType = pi_rObj.m_pDestReplacingPixelType;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// SetDestShape
//-----------------------------------------------------------------------------
void HRACopyFromLegacyOptions::SetDestShape(const HVEShape* pi_pShape)
    {
    m_pDestShape = pi_pShape;
    }

