//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFunctionFilters.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HRPColortwistFilter : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(1144, HRPFunctionFilter)
    

public:

    // Primary methods
    _HDLLg                 HRPColortwistFilter();
    _HDLLg                 HRPColortwistFilter(const double pi_Matrix[4][4]);

    _HDLLg virtual         ~HRPColortwistFilter();

    _HDLLg const double*  GetMatrix() const;

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
    HDECLARE_CLASS_ID(1213, HRPFunctionFilter)
    

public:

    // Primary methods
    _HDLLg                 HRPAlphaReplacer();
    _HDLLg                 HRPAlphaReplacer(Byte pi_DefaultAlpha);
    _HDLLg                 HRPAlphaReplacer(Byte pi_DefaultAlpha,
                                            const ListHRPAlphaRange& pi_rRanges);

    _HDLLg virtual         ~HRPAlphaReplacer();

    void            SetDefaultAlpha(Byte pi_DefaultAlpha);

    virtual HRPFilter* Clone() const override;

protected:

    // methods
    virtual void    Function(   const void* pi_pSrcRawData,
                                void* po_pDestRawData,
                                uint32_t PixelsCount) const;

private:
    HRPAlphaReplacer(const HRPAlphaReplacer& pi_rObj);
    HRPAlphaReplacer& operator = (const HRPAlphaReplacer& pi_rFilter);

    Byte          m_DefaultAlpha;
    ListHRPAlphaRange
    m_Ranges;

    void            DeepDelete();
    void            DeepCopy(const HRPAlphaReplacer& pi_rObj);
    };

//-----------------------------------------------------------------------------
// HRPAlphaComposer
//-----------------------------------------------------------------------------

class HRPAlphaComposer : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(1261, HRPFunctionFilter)
    

public:

    // Primary methods
    _HDLLg                 HRPAlphaComposer();
    _HDLLg                 HRPAlphaComposer(Byte pi_DefaultAlpha);
    _HDLLg                 HRPAlphaComposer(Byte pi_DefaultAlpha,
                                            const ListHRPAlphaRange& pi_rRanges);

    _HDLLg virtual         ~HRPAlphaComposer();

    _HDLLg void SetDefaultAlpha(Byte pi_DefaultAlpha);

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
    HDECLARE_CLASS_ID(1545, HRPFunctionFilter)
    

public:
    typedef list<HGFRGBCube> RGBCubeList;
    typedef list<HGFRGBSet>  RGBSetList;
    typedef list<HGFLUVCube> LUVCubeList;

    // Primary methods
    _HDLLg HRPColorReplacerFilter();

    _HDLLg virtual  ~HRPColorReplacerFilter();

    _HDLLg void AddColors(const HGFRGBCube& pi_rCube);
    _HDLLg void AddColors(const HGFRGBSet&  pi_rCube);
    _HDLLg void AddColors(const HGFLUVCube& pi_rCube);

    _HDLLg void RemoveColors(const HGFRGBCube& pi_rCube);
    _HDLLg void RemoveColors(const HGFRGBSet&  pi_rCube);
    _HDLLg void RemoveColors(const HGFLUVCube& pi_rCube);


    _HDLLg void SetNewColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue);

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


