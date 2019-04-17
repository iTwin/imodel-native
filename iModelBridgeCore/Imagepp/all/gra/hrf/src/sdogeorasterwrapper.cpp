//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class SDOGeoRasterWrapper
//-----------------------------------------------------------------------------
#include <ImageppInternal.h>

#include <ImagePP/all/h/SDOGeoRasterWrapper.h>
#include <ImagePP/all/h/OCIGeoRasterWrapper.h>

//-----------------------------------------------------------------------------
// static method
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public static
// IsConnected
//-----------------------------------------------------------------------------
bool SDOGeoRasterWrapper::IsConnected()
    {
    return OCIGeoRasterWrapper::IsConnected();
    }


//-----------------------------------------------------------------------------
// public static
// Connect
//-----------------------------------------------------------------------------
bool SDOGeoRasterWrapper::Connect( Utf8StringCR        pi_rUser,
                                   Utf8StringCR        pi_rPassword,
                                   Utf8StringCR        pi_rDatabase,
                                   OracleError*     po_pError)
    {
    HPRECONDITION(!OCIGeoRasterWrapper::IsConnected());

    return OCIGeoRasterWrapper::Connect(pi_rUser,
                                        pi_rPassword,
                                        pi_rDatabase,
                                        po_pError);

    }

//-----------------------------------------------------------------------------
// public static
// Connect
//-----------------------------------------------------------------------------
bool SDOGeoRasterWrapper::Connect( Utf8StringCR    pi_rConnectionString,
                                   OracleError*   po_pError)
    {
    HPRECONDITION(!OCIGeoRasterWrapper::IsConnected());

    return OCIGeoRasterWrapper::Connect(pi_rConnectionString, po_pError);
    }



//-----------------------------------------------------------------------------
// public static
// Disconnect
//-----------------------------------------------------------------------------
bool SDOGeoRasterWrapper::Disconnect()
    {
    return OCIGeoRasterWrapper::Disconnect();
    }


//-----------------------------------------------------------------------------
// public static
// GetWrapper
//-----------------------------------------------------------------------------
SDOGeoRasterWrapper* SDOGeoRasterWrapper::GetWrapper(Utf8StringCR  pi_rTableName,
                                                     Utf8StringCR  pi_rColumnName,
                                                     Utf8StringCR  pi_rImageID,
                                                     Utf8StringCR  pi_rRasterDataTableName,
                                                     const Utf16Char*   pi_pXMLGeoRasterHeader,
                                                     size_t         pi_XMLSize)
    {
    return new OCIGeoRasterWrapper(pi_rTableName,
        pi_rColumnName,
        pi_rImageID,
        pi_rRasterDataTableName,
        pi_pXMLGeoRasterHeader,
        pi_XMLSize);
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
SDOGeoRasterWrapper::~SDOGeoRasterWrapper()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetHeader
//-----------------------------------------------------------------------------
bool SDOGeoRasterWrapper::GetHeader(Utf16Char**    po_ppHeader,
                                    size_t*    po_pHeaderSize) const
    {
    *po_ppHeader = m_pXMLHeader;
    *po_pHeaderSize = m_XMLHeaderSize;

    return true;
    }

//-----------------------------------------------------------------------------
// protected method
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// Constructor
//-----------------------------------------------------------------------------
SDOGeoRasterWrapper::SDOGeoRasterWrapper(Utf8StringCR  pi_rTableName,
                                         Utf8StringCR  pi_rColumnName,
                                         Utf8StringCR  pi_rImageID,
                                         Utf8StringCR  pi_rRasterDataTableName,
                                         const Utf16Char*   pi_pXMLGeoRasterHeader,
                                         size_t         pi_XMLSize)
    : m_RasterDataTable(pi_rRasterDataTableName),
      m_RasterID(pi_rImageID)
    {
    HPRECONDITION(pi_pXMLGeoRasterHeader == 0 || !pi_rRasterDataTableName.empty());

    if (pi_pXMLGeoRasterHeader != 0)
        {
        m_XMLHeaderSize = pi_XMLSize;
        m_pXMLHeader = new Utf16Char[pi_XMLSize];
        memcpy(m_pXMLHeader, pi_pXMLGeoRasterHeader, pi_XMLSize);
        }
    }



