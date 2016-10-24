/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/AwsTraverser/AwsTraverser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    void ReadPage(Utf8CP url, float& redSize, float& greenSize, float& blueSize, float& panSize);
public:

    AwsPinger();
    ~AwsPinger();
    float m_redSize, m_blueSize, m_greenSize, m_panSize;
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