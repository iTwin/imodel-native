/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>

struct PerformanceTestSettings
    {
    private:
        int64_t m_codePostSize;
        int64_t m_codeGetSize;
        int64_t m_codeGetByIdSize;
        int64_t m_codeGetSizeSecondCall;
        int64_t m_codeGetByIdSizeSecondCall;
        int64_t m_lockPostSize;
        int64_t m_lockGetSize;
        int64_t m_lockGetByIdSize;
        int64_t m_lockGetSizeSecondCall;
        int64_t m_lockGetByIdSizeSecondCall;
        int64_t m_codeGetAttemptsCount;
        int64_t m_lockGetAttemptsCount;
        int64_t m_codeGetSplitCount;
        int64_t m_lockGetSplitCount;

        void ReadSettings(BeFileNameCR settingsFile);
    public:
        static PerformanceTestSettings& Instance();
        int64_t& GetCodePostRequestSize() {return m_codePostSize;};
        int64_t& GetCodeGetRequestSize() {return m_codeGetSize;};
        int64_t& GetCodeGetByIdRequestSize() {return m_codeGetByIdSize;};
        int64_t& GetCodeGetRequestSizeSecondCall() {return m_codeGetSizeSecondCall;};
        int64_t& GetCodeGetByIdRequestSizeSecondCall() {return m_codeGetByIdSizeSecondCall;};
        int64_t& GetLockPostRequestSize() {return m_lockPostSize;};
        int64_t& GetLockGetRequestSize() {return m_lockGetSize;};
        int64_t& GetLockGetByIdRequestSize() {return m_lockGetByIdSize;};
        int64_t& GetLockGetRequestSizeSecondCall() {return m_lockGetSizeSecondCall;};
        int64_t& GetLockGetByIdRequestSizeSecondCall() {return m_lockGetByIdSizeSecondCall;};
        int64_t& GetCodeGetAttemptsCount() { return m_codeGetAttemptsCount; };
        int64_t& GetLockGetAttemptsCount() { return m_lockGetAttemptsCount; };
        int64_t& GetCodeGetSplitCount() { return m_codeGetSplitCount; };
        int64_t& GetLockGetSplitCount() { return m_lockGetSplitCount; };
    };
