//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFunctionFilters.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPFunctionFilter.h"
#include "HRPAlphaRange.h"
#include "HGFRGBCube.h"
#include "HGFRGBSet.h"
#include "HGFLUVCube.h"

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRPColortwistFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Colortwist, HRPFunctionFilter)
    
public:

    // Primary methods
    IMAGEPP_EXPORT                 HRPColortwistFilter();
    IMAGEPP_EXPORT                 HRPColortwistFilter(const double pi_Matrix[4][4]);

    IMAGEPP_EXPORT virtual         ~HRPColortwistFilter();

    IMAGEPP_EXPORT const double*  GetMatrix() const;

    // Cloning
    virtual HRPFilter* Clone() const override;

protected:

    // methods

    virtual void Function  (const void* pi_pSrcRawData,
                            void* po_pDestRawData,
                            uint32_t PixelsCount) const;

private:
    HRPColortwistFilter(const HRPColortwistFilter& pi_rFilter);
    HRPColortwistFilter& operator = (const HRPColortwistFilter& pi_rFilter);

    double         m_Matrix[16];
    };

//-----------------------------------------------------------------------------
// HRPAlphaReplacer
//-----------------------------------------------------------------------------

class HRPAlphaReplacer : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_AlphaReplacer, HRPFunctionFilter)
    

public:

    // Primary methods
    IMAGEPP_EXPORT                 HRPAlphaReplacer();
    IMAGEPP_EXPORT                 HRPAlphaReplacer(Byte pi_DefaultAlpha);
    IMAGEPP_EXPORT                 HRPAlphaReplacer(Byte pi_DefaultAlpha,
                                            const ListHRPAlphaRange& pi_rRanges);

    IMAGEPP_EXPORT virtual         ~HRPAlphaReplacer();

    Byte                GetDefaultAlpha() const;
    void                SetDefaultAlpha(Byte pi_DefaultAlpha);

    ListHRPAlphaRange   GetAlphaRanges() const;
    void                SetAlphaRanges(const ListHRPAlphaRange& pi_rRanges);

    virtual HRPFilter* Clone() const override;

protected:

    // methods
    virtual void    Function(   const void* pi_pSrcRawData,
                                void* po_pDestRawData,
                                uint32_t PixelsCount) const;

private:
    HRPAlphaReplacer(const HRPAlphaReplacer& pi_rObj);
    HRPAlphaReplacer& operator = (const HRPAlphaReplacer& pi_rFilter);

    Byte                m_DefaultAlpha;
    ListHRPAlphaRange   m_Ranges;

    void            DeepDelete();
    void            DeepCopy(const HRPAlphaReplacer& pi_rObj);
    };

//-----------------------------------------------------------------------------
// HRPAlphaComposer
//-----------------------------------------------------------------------------

class HRPAlphaComposer : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_AlphaComposer, HRPFunctionFilter)
    

public:

    // Primary methods
    IMAGEPP_EXPORT                 HRPAlphaComposer();
    IMAGEPP_EXPORT                 HRPAlphaComposer(Byte pi_DefaultAlpha);
    IMAGEPP_EXPORT                 HRPAlphaComposer(Byte pi_DefaultAlpha,
                                            const ListHRPAlphaRange& pi_rRanges);

    IMAGEPP_EXPORT virtual         ~HRPAlphaComposer();

    Byte                   GetDefaultAlpha() const;
    IMAGEPP_EXPORT void            SetDefaultAlpha(Byte pi_DefaultAlpha);

    ListHRPAlphaRange      GetAlphaRanges() const;

    virtual HRPFilter* Clone() const override;

protected:

    // methods
    virtual void    Function(   const void* pi_pSrcRawData,
                                void* po_pDestRawData,
                                uint32_t PixelsCount) const;

private:
    HRPAlphaComposer(const HRPAlphaComposer& pi_rObj);
    HRPAlphaComposer& operator = (const HRPAlphaComposer& pi_rFilter);

    Byte          m_DefaultAlpha;
    ListHRPAlphaRange
    m_Ranges;

    void            DeepDelete();
    void            DeepCopy(const HRPAlphaComposer& pi_rObj);
    };

//-----------------------------------------------------------------------------
// HRPColorReplacerFilter
//-----------------------------------------------------------------------------

class HRPColorReplacerFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_ColorReplacer, HRPFunctionFilter)
    

public:
    typedef list<HGFRGBCube> RGBCubeList;
    typedef list<HGFRGBSet>  RGBSetList;
    typedef list<HGFLUVCube> LUVCubeList;

    // Primary methods
    IMAGEPP_EXPORT HRPColorReplacerFilter();

    IMAGEPP_EXPORT virtual  ~HRPColorReplacerFilter();

    IMAGEPP_EXPORT void AddColors(const HGFRGBCube& pi_rCube);
    IMAGEPP_EXPORT void AddColors(const HGFRGBSet&  pi_rCube);
    IMAGEPP_EXPORT void AddColors(const HGFLUVCube& pi_rCube);

    IMAGEPP_EXPORT void RemoveColors(const HGFRGBCube& pi_rCube);
    IMAGEPP_EXPORT void RemoveColors(const HGFRGBSet&  pi_rCube);
    IMAGEPP_EXPORT void RemoveColors(const HGFLUVCube& pi_rCube);


    IMAGEPP_EXPORT void SetNewColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue);
    const Byte* GetNewColor() const;
    const RGBSetList& GetSelectedRGBSet() const;
    const RGBSetList& GetSelectedRemoveRGBSet() const;

    virtual HRPFilter* Clone() const override;

protected:
    // methods
    virtual void  Function( const void*  pi_pSrcRawData, void*  po_pDestRawData, uint32_t PixelsCount ) const;
    

private:
    HRPColorReplacerFilter(const HRPColorReplacerFilter& pi_rFilter);
    HRPColorReplacerFilter& operator = (const HRPColorReplacerFilter& pi_rFilter);

    bool LookForColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const;

    RGBCubeList m_RGBCubeList;
    RGBSetList  m_RGBSetList;
    LUVCubeList m_LUVCubeList;

    RGBCubeList m_RGBCubeRemoveList;
    RGBSetList  m_RGBSetRemoveList;
    LUVCubeList m_LUVCubeRemoveList;

    Byte      m_NewRGBColor[3];

    // Used for debugging purpose only.
#ifdef __HMR_DEBUG_MEMBER
    bool   m_NewColorInitialized;
#endif
    };
END_IMAGEPP_NAMESPACE
