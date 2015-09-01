//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/SDOGeoRasterWrapper.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a oracle SDOGeoRaster.
//-----------------------------------------------------------------------------
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>

#include <ImagePP/h/HArrayAutoPtr.h>

// -----------------------------------------------------------------------------
// SDOGeoRasterWrapper
// This class is use to read a sdo_geoRaster object into an Oracle database.
//
// October 1, 2012
//      This class support Oracle 11g version 11.2.
// -----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class SDOGeoRasterWrapper : public NonCopyableClass
    {
public:
    struct OracleError
        {
        static const int32_t NO_CODE = -1;
        OracleError() : m_ErrorCode(NO_CODE) {};

        WString  m_ErrorMsg;
        int32_t m_ErrorCode;
        };


    IMAGEPP_EXPORT virtual            ~SDOGeoRasterWrapper   ();

    IMAGEPP_EXPORT virtual bool       GetHeader   (Utf16Char**        po_ppHeader,
                                                     size_t*        po_HeaderSize) const;

    IMAGEPP_EXPORT virtual void       GetBlock    (unsigned short pi_Resolution,
                                                unsigned short pi_Band,
                                                uint32_t       pi_PosX,
                                                uint32_t       pi_PosY,
                                                Byte*         po_pBuffer,
                                                size_t         pi_BufferSize) = 0;

    IMAGEPP_EXPORT virtual void       GetBlock    (unsigned short pi_Resolution,
                                                unsigned short pi_Band,
                                                uint32_t       pi_PosX,
                                                uint32_t       pi_PosY,
                                                Byte**        po_ppBuffer,
                                                size_t*        po_pBufferSize) = 0;

    IMAGEPP_EXPORT virtual bool       SetBlock    (unsigned short pi_Resolution,
                                                unsigned short pi_Band,
                                                uint32_t       pi_PosX,
                                                uint32_t       pi_PosY,
                                                const Byte*   pi_pBuffer,
                                                size_t         pi_BufferSize) = 0;

    IMAGEPP_EXPORT virtual bool        GetWkt      (uint32_t       pi_SRID,
                                                       WStringR        po_rWKT) = 0;

    // static methods
    IMAGEPP_EXPORT static SDOGeoRasterWrapper*
    GetWrapper(WStringCR  pi_rTableName,
               WStringCR  pi_rColumnName,
               WStringCR  pi_rImageID,
               WStringCR  pi_rRasterDataTableName = WString(L""),
               const Utf16Char*   pi_pXMLGeoRasterHeader = 0,
               size_t         pi_XMLSize = 0);

    IMAGEPP_EXPORT static bool       IsConnected     ();

    IMAGEPP_EXPORT static bool       Connect         (          WStringCR      pi_rUser,
                                                        WStringCR      pi_rPassword,
                                                        WStringCR      pi_rDatabase,
                                                        OracleError*       po_pError = 0);

    IMAGEPP_EXPORT static bool       Connect         (          WStringCR      pi_rConnectionString,
                                                        OracleError*       po_pError = 0);

    IMAGEPP_EXPORT static bool       Disconnect      ();

    // for testing only
#if 0
    IMAGEPP_EXPORT static void* CreateEnvironment();
    IMAGEPP_EXPORT static void* CreateConnection(void* pi_pEnvironment,
                                              WStringCR pi_rUserName,
                                              WStringCR pi_rPassword,
                                              WStringCR pi_rService);

    IMAGEPP_EXPORT static void  GetGeoRaster(void*            pi_pConnection,
                                          WStringCR    pi_rTableName,
                                          WStringCR    pi_rColumnName,
                                          WStringCR    pi_rRasterID,
                                          WStringP     po_pRasterDataTable,
                                          Utf16Char**  po_ppHeader,
                                          size_t*      po_pHeaderSize);

#endif


protected:
    SDOGeoRasterWrapper    (WStringCR      pi_rTableName,
                            WStringCR      pi_rColumnName,
                            WStringCR      pi_RasterID,
                            WStringCR      pi_rRasterDataTableName = WString(L""),
                            const Utf16Char* pi_pXMLGeoRasterHeader = 0,
                            size_t           pi_XMLSize = 0);

    HArrayAutoPtr<Utf16Char>    m_pXMLHeader;
    size_t                      m_XMLHeaderSize;

    WString                 m_RasterDataTable;
    WString                 m_RasterID;
    SDOGeoRasterWrapper*    m_pImpl;
    };
END_IMAGEPP_NAMESPACE

