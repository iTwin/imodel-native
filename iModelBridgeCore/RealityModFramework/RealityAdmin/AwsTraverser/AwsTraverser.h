/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <string>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct AwsPinger
    {
    static AwsPinger* s_instance;
    Utf8String m_certificatePath;
    CURL* m_curl;

    void ReadPage(Utf8CP url, uint64_t& redSize, uint64_t& greenSize, uint64_t& blueSize, uint64_t& panSize);
public:

    AwsPinger();
    ~AwsPinger();
    uint64_t m_redSize, m_blueSize, m_greenSize, m_panSize;
    static AwsPinger& GetInstance();
    };

struct AwsLogger
    {
public:
    virtual void Log(std::string message);
    bool ValidateLine(size_t LineIndex, std::string line);
    virtual void Close() {}
    };

struct AwsFileLogger: public AwsLogger
    {
public:
    std::ofstream logFile;
    void Log(std::string message) override;
    void Close() override;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE