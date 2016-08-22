//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFxChEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFxChFile;

/** ---------------------------------------------------------------------------
    This class handles xCh raster file I/O operations.

    @see HRFRasterFile
    @see HRFxChEditor
    ---------------------------------------------------------------------------
 */
class HRFxChEditorRGBA : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFxChFile;

    virtual       ~HRFxChEditorRGBA();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t   pi_PosBlockX,
                              uint64_t   pi_PosBlockY,
                              Byte*      po_pData);

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              HFCPtr<HCDPacket>&     po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFxChEditorRGBA(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     uint16_t              pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFxChEditorRGBA(const HRFxChEditorRGBA& pi_rObj);
    HRFxChEditorRGBA& operator=(const HRFxChEditorRGBA& pi_rObj);
    };


class HRFxChEditorPanchromatic : public HRFResolutionEditor
{
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

        friend class HRFxChFile;

    virtual       ~HRFxChEditorPanchromatic();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t   pi_PosBlockX,
        uint64_t   pi_PosBlockY,
        Byte*      po_pData);

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
        uint64_t               pi_PosBlockY,
        HFCPtr<HCDPacket>&     po_rpPacket)
    {
        return T_Super::ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
    }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFxChEditorPanchromatic(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                            uint32_t              pi_Page,
                            uint16_t       pi_Resolution,
                            HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFxChEditorPanchromatic(const HRFxChEditorPanchromatic& pi_rObj);
    HRFxChEditorPanchromatic& operator=(const HRFxChEditorPanchromatic& pi_rObj);
};

END_IMAGEPP_NAMESPACE
