//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/OCIGeoRasterWrapper.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a oracle SDOGeoRaster.
//-----------------------------------------------------------------------------
#pragma once

#include "SDOGeoRasterWrapper.h"

BEGIN_IMAGEPP_NAMESPACE
class OCIConnection;
typedef OCIConnection* OracleOCIConnectionP;

class OCIGeoRasterWrapper : public SDOGeoRasterWrapper
    {
public:

    OCIGeoRasterWrapper  (const Utf8String&      pi_rTableName,
                           const Utf8String&      pi_rColumnName,
                           const Utf8String&      pi_RasterID,
                           const Utf8String&      pi_rRasterDataTableName = Utf8String(""),
                           const Utf16Char*         pi_pXMLGeoRasterHeader = 0,
                           size_t              pi_XMLSize = 0);


    virtual             ~OCIGeoRasterWrapper  ();


    virtual void                GetBlock   (uint16_t pi_Resolution,
                                            uint16_t pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            Byte*         po_pBuffer,
                                            size_t         pi_BufferSize) override;

    virtual void                GetBlock   (uint16_t pi_Resolution,
                                            uint16_t pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            Byte**        po_ppBuffer,
                                            size_t*        po_pBufferSize) override;

    virtual bool                SetBlock   (uint16_t pi_Resolution,
                                            uint16_t pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            const Byte*   pi_pBuffer,
                                            size_t         pi_BufferSize) override;

    virtual bool       GetWkt      (uint32_t       pi_SRID,
                                    Utf8StringR       po_rWKT) override;

    static bool       IsConnected     ();

    static bool       Connect         (const Utf8String&      pi_rUser,
                                        const Utf8String&     pi_rPassword,
                                        const Utf8String&     pi_rDatabase,
                                        OracleError*       po_pError = 0);

    static bool       Connect         (const Utf8String&      pi_rConnectionString,
                                        OracleError*       po_pError = 0);

    static bool       Disconnect      ();



private:
    OracleOCIConnectionP       m_pConnection; 
    Utf8String                    m_getBlockRequest;
    };
END_IMAGEPP_NAMESPACE

