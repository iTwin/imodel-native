//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSResponse.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCSResponse
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline size_t HCSResponse::GetDataSize() const
    {
    return (m_Buffer.GetDataSize());
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline const Byte* HCSResponse::GetData() const
    {
    HPRECONDITION(GetDataSize() > 0);
    return (m_Buffer.GetData());
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HCSResponse::AppendResponse(const Byte* pi_pData,
                                        size_t       pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // Add the data to our buffer
    m_Buffer.AddData(pi_pData, pi_DataSize);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HCSResponse::AppendResponse(const WString& pi_rResponse)
    {
    HPRECONDITION(!pi_rResponse.empty());

    Utf8String utf8Str;
    BeStringUtilities::WCharToUtf8(utf8Str,pi_rResponse.c_str());

    AppendResponse((const Byte*)utf8Str.c_str(), utf8Str.length());
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline void HCSResponse::AppendResponse(const HCSResponse& pi_rResponse)
    {
    HPRECONDITION(&pi_rResponse != this);
    HPRECONDITION(pi_rResponse.GetDataSize() > 0);

    AppendResponse(pi_rResponse.GetData(), pi_rResponse.GetDataSize());
    }
