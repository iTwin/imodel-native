//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/interface/IFileReaderHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


class IFileReaderHandler : public HFCShareableObject<IFileReaderHandler>
    {
public:
    virtual ~IFileReaderHandler()   {}
    virtual int Seek(uint64_t offset) = 0;
    virtual int GetSize(uint64_t& size) = 0;
    virtual void Read(void*, size_t size, size_t& bytesRead) = 0;

    };
