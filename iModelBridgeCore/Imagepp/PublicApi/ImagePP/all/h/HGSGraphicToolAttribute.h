//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolAttribute.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// General class for graphic tool attributes.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HPMClassKey.h"
#include "HGFScanlineOrientation.h"
#include "HGSTypes.h"

//-----------------------------------------------------------------------------
// Class : HGSSurfaceAttribute
//-----------------------------------------------------------------------------
class HGSGraphicToolAttribute : public HFCShareableObject<HGSGraphicToolAttribute>
    {
    HDECLARE_BASECLASS_ID(8150)

public:

    virtual         ~HGSGraphicToolAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const = 0;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const = 0;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const = 0;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const = 0;

    // operator
    HGSGraphicToolAttribute&
    operator=(const HGSGraphicToolAttribute& pi_rObj);

protected:
    HGSGraphicToolAttribute();
    HGSGraphicToolAttribute(const HGSGraphicToolAttribute& pi_rObj);

private:
    // disabled method
    bool             operator==(const HGSGraphicToolAttribute& pi_rObj) const;
    bool             operator!=(const HGSGraphicToolAttribute& pi_rObj) const;
    };



//-----------------------------------------------------------------------------
// Class : HGSTransformAttibute
//-----------------------------------------------------------------------------
class HGSTransformAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8151, HGSGraphicToolAttribute)

public :
    HGSTransformAttribute(const HGSTransform& pi_rTransform);
    HGSTransformAttribute(const HGSTransformAttribute& pi_rObj);
    virtual         ~HGSTransformAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSTransformAttribute&
    operator=(const HGSTransformAttribute& pi_rObj);

    const HGSTransform&
    GetTransform() const;

private:

    // members
    HGSTransform    m_Transform;

    // disabled method
    HGSTransformAttribute();
    bool             operator==(const HGSTransformAttribute& pi_rObj) const;
    bool             operator!=(const HGSTransformAttribute& pi_rObj) const;
    };



//-----------------------------------------------------------------------------
// Class : HGSResamplingAttribute
//-----------------------------------------------------------------------------
class HGSResamplingAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8152, HGSGraphicToolAttribute)

public :
    _HDLLg                 HGSResamplingAttribute(const HGSResampling& pi_rResampling);
    _HDLLg                 HGSResamplingAttribute(const HGSResamplingAttribute& pi_rObj);
    _HDLLg virtual         ~HGSResamplingAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSResamplingAttribute&
    operator=(const HGSResamplingAttribute& pi_rObj);

    const HGSResampling&
    GetResampling() const;

private:

    // members
    HGSResampling   m_Resampling;

    // disabled method
    HGSResamplingAttribute();
    bool             operator==(const HGSResamplingAttribute& pi_rObj) const;
    bool             operator!=(const HGSResamplingAttribute& pi_rObj) const;
    };



//-----------------------------------------------------------------------------
// Class : HGSColorConversionAttribute
//-----------------------------------------------------------------------------
class HGSColorConversionAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8153, HGSGraphicToolAttribute)

public :
    _HDLLg                 HGSColorConversionAttribute(const HGSColorConversion& pi_rColorConversion);
    _HDLLg                 HGSColorConversionAttribute(const HGSColorConversionAttribute& pi_rObj);
    _HDLLg virtual         ~HGSColorConversionAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSColorConversionAttribute&
    operator=(const HGSColorConversionAttribute& pi_rObj);

    const HGSColorConversion&
    GetColorConversion() const;

private:

    // members
    HGSColorConversion  m_ColorConversion;

    // disabled method
    HGSColorConversionAttribute();
    bool             operator==(const HGSColorConversionAttribute& pi_rObj) const;
    bool             operator!=(const HGSColorConversionAttribute& pi_rObj) const;
    };


//-----------------------------------------------------------------------------
// Class : HGSColorAttribute
//-----------------------------------------------------------------------------
class HGSColorAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8154, HGSGraphicToolAttribute)

public :
    _HDLLg                 HGSColorAttribute(const HGSColor& pi_rColor);
    _HDLLg                 HGSColorAttribute(const HGSColorAttribute& pi_rObj);
    _HDLLg virtual         ~HGSColorAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSColorAttribute&
    operator=(const HGSColorAttribute& pi_rObj);

    _HDLLg const HGSColor&
    GetColor() const;

private:

    // members
    HGSColor        m_Color;

    // disabled method
    HGSColorAttribute();
    bool             operator==(const HGSColorAttribute& pi_rObj) const;
    bool             operator!=(const HGSColorAttribute& pi_rObj) const;
    };


//-----------------------------------------------------------------------------
// Class : HGSLineStyleAttribute
//-----------------------------------------------------------------------------
class HGSLineStyleAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8155, HGSGraphicToolAttribute)

public:

    _HDLLg                 HGSLineStyleAttribute();
    _HDLLg                 HGSLineStyleAttribute(uint32_t pi_LineWidth);
    _HDLLg                 HGSLineStyleAttribute(const HGSLineStyleAttribute& pi_rObj);
    _HDLLg virtual         ~HGSLineStyleAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;

    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    HGSLineStyleAttribute&
    operator=(const HGSLineStyleAttribute& pi_rObj);

    _HDLLg uint32_t        GetWidth() const;

private:


    uint32_t        m_LineWidth;

    // disabled method
    bool             operator==(const HGSLineStyleAttribute& pi_rObj) const;
    bool             operator!=(const HGSLineStyleAttribute& pi_rObj) const;
    };


//-----------------------------------------------------------------------------
// Class : HGSScanlinesAttribute
//-----------------------------------------------------------------------------
class HGSScanlinesAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8156, HGSGraphicToolAttribute)

public :
    HGSScanlinesAttribute(const HGSScanlineMethod& pi_rScanlineMethod);
    HGSScanlinesAttribute(const HGSScanlinesAttribute& pi_rObj);
    virtual         ~HGSScanlinesAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSScanlinesAttribute&
    operator=(const HGSScanlinesAttribute& pi_rObj);

    const HGSScanlineMethod&
    GetScanlineMethod() const;

private:

    // members
    HGSScanlineMethod   m_ScanlineMethod;

    // disabled method
    HGSScanlinesAttribute();
    bool             operator==(const HGSScanlinesAttribute& pi_rObj) const;
    bool             operator!=(const HGSScanlinesAttribute& pi_rObj) const;
    };

//-----------------------------------------------------------------------------
// Class : HGSPurposeAttribute
//-----------------------------------------------------------------------------
class HGSPurposeAttribute : public HGSGraphicToolAttribute
    {
    HDECLARE_CLASS_ID(8157, HGSGraphicToolAttribute)

public :
    HGSPurposeAttribute(const HGSPurpose& pi_rScanlineMethod);
    HGSPurposeAttribute(const HGSPurposeAttribute& pi_rObj);
    virtual         ~HGSPurposeAttribute();

    virtual HGSGraphicToolAttribute*
    Clone() const;
    virtual bool   IsSameAs(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSGraphicToolAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSGraphicToolAttribute& pi_rAttribute) const;

    // operator
    HGSPurposeAttribute&
    operator=(const HGSPurposeAttribute& pi_rObj);

    const HGSPurpose&
    GetPurpose() const;

private:

    // members
    HGSPurpose      m_Purpose;

    // disabled method
    HGSPurposeAttribute();
    bool             operator==(const HGSPurposeAttribute& pi_rObj) const;
    bool             operator!=(const HGSPurposeAttribute& pi_rObj) const;
    };

