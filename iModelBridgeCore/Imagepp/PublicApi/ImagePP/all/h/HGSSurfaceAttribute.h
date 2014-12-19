//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceAttribute.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HPMClassKey.h"
#include "HGFScanlineOrientation.h"
#include "HGSTypes.h"

//-----------------------------------------------------------------------------
// Class : HGSSurfaceAttribute
//-----------------------------------------------------------------------------
class HNOVTABLEINIT HGSSurfaceAttribute : public HFCShareableObject<HGSSurfaceAttribute>
    {
    HDECLARE_BASECLASS_ID(8100)

public:

    _HDLLg virtual         ~HGSSurfaceAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const = 0;

    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const = 0;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const = 0;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const = 0;

    // operator
    HGSSurfaceAttribute&
    operator=(const HGSSurfaceAttribute& pi_rObj);

protected:
    _HDLLg                 HGSSurfaceAttribute();
    _HDLLg                 HGSSurfaceAttribute(const HGSSurfaceAttribute& pi_rObj);

private:

    // disabled method
    bool           operator==(const HGSSurfaceAttribute& pi_rObj) const;
    bool           operator!=(const HGSSurfaceAttribute& pi_rObj) const;

    };



//-----------------------------------------------------------------------------
// Class : HGSPixelTypeAttribute
//-----------------------------------------------------------------------------
class HGSPixelTypeAttribute : public HGSSurfaceAttribute
    {
    HDECLARE_CLASS_ID(8101, HGSSurfaceAttribute)

public :
    _HDLLg                 HGSPixelTypeAttribute(HCLASS_ID pi_PixelType);
    _HDLLg                 HGSPixelTypeAttribute(const HGSPixelTypeAttribute& pi_rObj);
    _HDLLg virtual         ~HGSPixelTypeAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const;

    // utility
    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const;

    // operator
    HGSPixelTypeAttribute&
    operator=(const HGSPixelTypeAttribute& pi_rObj);

    HCLASS_ID     GetPixelTypeClassID() const;

private:

    // members
    HCLASS_ID     m_PixelType;

    // disabled method
    HGSPixelTypeAttribute();
    };


//-----------------------------------------------------------------------------
// Class : HGSCompressionAttribute
//-----------------------------------------------------------------------------
class HGSCompressionAttribute : public HGSSurfaceAttribute
    {
    HDECLARE_CLASS_ID(8102, HGSSurfaceAttribute)

public :
    // constructor/destructor
    _HDLLg                 HGSCompressionAttribute(HCLASS_ID pi_Compression);
    _HDLLg                 HGSCompressionAttribute(const HGSCompressionAttribute& pi_rObj);
    _HDLLg virtual         ~HGSCompressionAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const;

    // utility
    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const;

    // operator
    HGSCompressionAttribute&
    operator=(const HGSCompressionAttribute& pi_rObj);

    HCLASS_ID     GetCompressionClassKey() const;

private:

    // members
    HCLASS_ID     m_Compression;

    // disabled method
    HGSCompressionAttribute();
    };


//-----------------------------------------------------------------------------
// Class : HGSSLOAttribute
//-----------------------------------------------------------------------------
class HGSSLOAttribute : public HGSSurfaceAttribute
    {
    HDECLARE_CLASS_ID(8103, HGSSurfaceAttribute)

public:
    _HDLLg                 HGSSLOAttribute(HGFSLO pi_SLO);
    _HDLLg                 HGSSLOAttribute(const HGSSLOAttribute& pi_rObj);
    _HDLLg virtual         ~HGSSLOAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const;

    // utility
    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const;

    HGSSLOAttribute&
    operator=(const HGSSLOAttribute& pi_rObj);

    HGFSLO          GetSLO() const;

private:

    // members
    HGFSLO          m_SLO;

    // disabled method
    HGSSLOAttribute();

    };


//-----------------------------------------------------------------------------
// Class : HGSSurfaceTypeAttribute
//-----------------------------------------------------------------------------
class HGSSurfaceTypeAttribute : public HGSSurfaceAttribute
    {
    HDECLARE_CLASS_ID(8104, HGSSurfaceAttribute)

public:
    // constructor/destructor
    _HDLLg                 HGSSurfaceTypeAttribute(const HGSSurfaceType& pi_rSurfaceType);
    _HDLLg                 HGSSurfaceTypeAttribute(const HGSSurfaceTypeAttribute& pi_rObj);
    _HDLLg virtual         ~HGSSurfaceTypeAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const;

    // utility
    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const;

    // operator
    HGSSurfaceTypeAttribute&
    operator=(const HGSSurfaceTypeAttribute& pi_rObj);

    const HGSSurfaceType&
    GetSurfaceType() const;

private:

    // members
    HGSSurfaceType  m_SurfaceType;

    // disabled method
    HGSSurfaceTypeAttribute();

    };


//-----------------------------------------------------------------------------
// Class : HGSMemoryAlignmentAttribute
//-----------------------------------------------------------------------------
class HGSMemoryAlignmentAttribute : public HGSSurfaceAttribute
    {
    HDECLARE_CLASS_ID(8105, HGSSurfaceAttribute)

public:
    // constructor/destructor
    _HDLLg                 HGSMemoryAlignmentAttribute(const HGSMemoryAlignment& pi_rMemoryAlignment);
    _HDLLg                 HGSMemoryAlignmentAttribute(const HGSMemoryAlignmentAttribute& pi_rObj);
    _HDLLg virtual         ~HGSMemoryAlignmentAttribute();

    virtual HGSSurfaceAttribute*
    Clone() const;

    // utility
    virtual bool   IsSameAs(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   IsEqual(const HGSSurfaceAttribute& pi_rAttribute) const;
    virtual bool   Supports(const HGSSurfaceAttribute& pi_rAttribute) const;

    // operator
    HGSMemoryAlignmentAttribute&
    operator=(const HGSMemoryAlignmentAttribute& pi_rObj);

    const HGSMemoryAlignment&
    GetMemoryAlignment() const;

private:

    // members
    HGSMemoryAlignment  m_MemoryAlignment;

    // disabled method
    HGSMemoryAlignmentAttribute();
    };

