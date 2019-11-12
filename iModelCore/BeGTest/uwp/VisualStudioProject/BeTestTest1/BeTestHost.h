/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <SDKDDKVer.h>
#define BETEST_NO_INCLUDE_BOOST_FOR_EACH
#include <Bentley/BeTest.h>

extern "C" char* sqlite3_temp_directory;

void ExecuteOnUiThread (Windows::UI::Core::DispatchedHandler^ action);

struct BeTestHost : RefCounted<BeTest::Host>
    {
    static Utf8String s_currentTestClassName;

    BeFileName m_home;
    BeFileName m_docs;
    BeFileName m_temp;
    BeFileName m_output;

    BeTestHost (wchar_t const* home);

    virtual void  _GetDocumentsRoot (BeFileName& path) override;
    virtual void  _GetDgnPlatformAssetsDirectory (BeFileName& path) override;
    virtual void  _GetOutputRoot (BeFileName& path) override;
    virtual void  _GetTempDir (BeFileName& path) override;
    virtual void* _InvokeP (char const*, void*) override;
    virtual void  _GetFrameworkSqlangFiles(BeFileName& path) override;

    static RefCountedPtr<BeTestHost> Create (wchar_t const* home);
    };
