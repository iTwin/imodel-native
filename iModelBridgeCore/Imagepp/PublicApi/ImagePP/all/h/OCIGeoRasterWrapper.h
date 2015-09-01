//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/OCIGeoRasterWrapper.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    OCIGeoRasterWrapper  (const WString&      pi_rTableName,
                           const WString&      pi_rColumnName,
                           const WString&      pi_RasterID,
                           const WString&      pi_rRasterDataTableName = WString(L""),
                           const Utf16Char*         pi_pXMLGeoRasterHeader = 0,
                           size_t              pi_XMLSize = 0);


    virtual             ~OCIGeoRasterWrapper  ();


    virtual void                GetBlock   (unsigned short pi_Resolution,
                                            unsigned short pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            Byte*         po_pBuffer,
                                            size_t         pi_BufferSize) override;

    virtual void                GetBlock   (unsigned short pi_Resolution,
                                            unsigned short pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            Byte**        po_ppBuffer,
                                            size_t*        po_pBufferSize) override;

    virtual bool                SetBlock   (unsigned short pi_Resolution,
                                            unsigned short pi_Band,
                                            uint32_t       pi_PosX,
                                            uint32_t       pi_PosY,
                                            const Byte*   pi_pBuffer,
                                            size_t         pi_BufferSize) override;

    virtual bool       GetWkt      (uint32_t       pi_SRID,
                                    WStringR       po_rWKT) override;

    static bool       IsConnected     ();

    static bool       Connect         (const WString&      pi_rUser,
                                        const WString&     pi_rPassword,
                                        const WString&     pi_rDatabase,
                                        OracleError*       po_pError = 0);

    static bool       Connect         (const WString&      pi_rConnectionString,
                                        OracleError*       po_pError = 0);

    static bool       Disconnect      ();



private:
    OracleOCIConnectionP       m_pConnection; 
    WString                    m_getBlockRequest;
    };
END_IMAGEPP_NAMESPACE

