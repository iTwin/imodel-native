//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class IFileReaderHandler : public HFCShareableObject<IFileReaderHandler>
    {
public:
    virtual ~IFileReaderHandler()   {}
    virtual int32_t Seek(uint64_t offset) = 0;
    virtual int32_t GetSize(uint64_t& size) = 0;
    virtual void Read(void*, size_t size, size_t& bytesRead) = 0;

    };

END_IMAGEPP_NAMESPACE
