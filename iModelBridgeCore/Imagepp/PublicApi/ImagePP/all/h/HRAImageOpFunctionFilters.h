//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOpFunctionFilters.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HRAImageOp.h>
#include "HRPAlphaRange.h"
#include "HGFRGBCube.h"
#include "HGFRGBSet.h"
#include "HGFLUVCube.h"

IMAGEPP_REF_COUNTED_PTR(HRAImageOpPixelReplacerFilter)

BEGIN_IMAGEPP_NAMESPACE

typedef bvector<HRPAlphaRange, allocator<HRPAlphaRange>> VectorHRPAlphaRange;

/*----------------------------------------------------------------------------+
|struct HRAImageOpPixelReplacerFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpPixelReplacerFilter : public HRAImageOp
{
public:
    //! Create a pixel replacer. This operation will induce a pixel conversion to 'pixeltype' if source is of a different pixeltype.
    //! Pixel values must be provided in 'pixeltype' space.
    static HRAImageOpPixelReplacerFilterPtr CreatePixelReplacer(HRPPixelType const& pixeltype);

    //! Set the value that will be replaced. 'size' must match the pixelsize of 'pixeltype'.
    void SetValue(void const* pValue, size_t size);

    //! Set new value.
    void SetNewValue(void const* pValue, size_t size);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

private:
    HRAImageOpPixelReplacerFilter(HRPPixelType const& pixeltype);
    virtual ~HRAImageOpPixelReplacerFilter();

    template<class Data_T, uint32_t Count_T> 
    ImagePPStatus Process_T(HRAImageSampleR outData, HRAImageSampleCR inputData);

    bvector<Byte> m_oldValue;
    bvector<Byte> m_newValue;
    HFCPtr<HRPPixelType> m_pPixelType;
};


/*----------------------------------------------------------------------------+
|struct HRAImageOpColorReplacerFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpColorReplacerFilter : public HRAImageOp
{
public:
    typedef bvector<HGFRGBCube> RGBCubeList;
    typedef bvector<HGFRGBSet>  RGBSetList;
    typedef bvector<HGFLUVCube> LUVCubeList;

    //! Create a color replacer filter. 
    static HRAImageOpPtr CreateColorReplacerFilter(const Byte pi_newRGBColor[3], const RGBSetList& selectedRGBSet, const RGBSetList& selectedRemoveRGBSet);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

    void AddColors(const HGFRGBCube& pi_rCube);
    void AddColors(const HGFRGBSet&  pi_rCube);
    void AddColors(const HGFLUVCube& pi_rCube);
    void RemoveColors(const HGFRGBCube& pi_rCube);
    void RemoveColors(const HGFRGBSet&  pi_rCube);
    void RemoveColors(const HGFLUVCube& pi_rCube);
    void SetNewColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue);

private:
    HRAImageOpColorReplacerFilter(const Byte pi_newRGBColor[3], const RGBSetList& selectedRGBSet, const RGBSetList& selectedRemoveRGBSet);
    virtual ~HRAImageOpColorReplacerFilter();

    bool LookForColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const;
    const Byte* GetNewColor() const;

    RGBCubeList m_RGBCubeList;
    RGBSetList  m_RGBSetList;
    LUVCubeList m_LUVCubeList;
    RGBCubeList m_RGBCubeRemoveList;
    RGBSetList  m_RGBSetRemoveList;
    LUVCubeList m_LUVCubeRemoveList;
    Byte        m_NewRGBColor[3];
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpColortwistFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpColortwistFilter : public HRAImageOp
{
public:
    //! Create a colortwist filter. Matrix must hold normalized values.
    static HRAImageOpPtr CreateColortwistFilter(const double matrix[4][4]);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

private:
    HRAImageOpColortwistFilter(const double matrix[4][4]);
    virtual ~HRAImageOpColortwistFilter();

    const double*  GetMatrix() const;

    double         m_matrix[16];
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpAlphaReplacerFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpAlphaReplacerFilter: public HRAImageOp
{
public:
    //! Create an AlphaReplacer filter. 
    static HRAImageOpPtr CreateAlphaReplacerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;
    virtual ImagePPStatus _ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params) override;

private:
    HRAImageOpAlphaReplacerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges);
    virtual ~HRAImageOpAlphaReplacerFilter();

    Byte                m_defaultAlpha;
    VectorHRPAlphaRange   m_ranges;
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpAlphaComposerFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpAlphaComposerFilter: public HRAImageOp
{
public:
    //! Create an AlphaComposer filter. 
    static HRAImageOpPtr CreateAlphaComposerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;
    virtual ImagePPStatus _ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params) override;

private:
    HRAImageOpAlphaComposerFilter(Byte defaultAlpha, const VectorHRPAlphaRange& pi_rRanges);
    virtual ~HRAImageOpAlphaComposerFilter();

    Byte                m_defaultAlpha;
    VectorHRPAlphaRange   m_ranges;
};

END_IMAGEPP_NAMESPACE
