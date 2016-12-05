/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/CompressionOptions.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

struct CompressionOptions
    {
    private:
        bool m_isRequestCmpressionEnabled       = false;
        uint64_t m_minimumRequestSizeToCompress =  1024;

    public:
        CompressionOptions(){}
        virtual ~CompressionOptions() {}

        //! Set enabled in order to send compressed request. The request body will be compressed automaticly
        //! and appropiate headers will be set.
        //! @param[in] enable to enable request compression
        //! @param[in] minimumSize a size limit, to be compared to request size before enabling compression
        BEHTTP_EXPORT void EnableRequestCompression(bool enable, uint64_t minimumSize = 1024)
            {m_isRequestCmpressionEnabled = enable; m_minimumRequestSizeToCompress = minimumSize;}

        BEHTTP_EXPORT bool IsRequestCompressionEnabled() const {return m_isRequestCmpressionEnabled;}
        BEHTTP_EXPORT uint64_t GetMinimumSizeToCompress() const {return m_minimumRequestSizeToCompress;}
    };

typedef const CompressionOptions& CompressionOptionsCR;

END_BENTLEY_HTTP_NAMESPACE