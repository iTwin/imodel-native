/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//  BeTestiOSTests.h
//  BeTestiOSTests
//
//  Created by bsidev on 2/22/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <XCTest/XCTest.h>
#define BETEST_NO_INCLUDE_BOOST_FOR_EACH
#import <Bentley/BeTest.h>
#import "BeTestiOSAppDelegate.h"

// structure to pass viewport data to tests
struct IosViewData
{
    void*           m_uiView;
};

// This class is used by the .mm files included below
struct BeTestHost : RefCounted<BeTest::Host>
{
    BeFileName m_docs;
    BeFileName m_assetsDir;
    BeFileName m_tempDir;
    BeFileName m_output;
    XCTestCase* m_currentTestCase;
    
    BeTestHost (char const* docs, char const* assets, char const* temp, char const* localState)
    {
    m_docs.SetNameUtf8 (docs);
    m_assetsDir.SetNameUtf8 (assets);
    m_tempDir.SetNameUtf8 (temp);
    m_output.SetNameUtf8 (localState);

    // for consistency with DgnClientFx, ensure trailing separators
    m_docs.AppendSeparator();
    m_assetsDir.AppendSeparator();
    m_tempDir.AppendSeparator();
    m_output.AppendSeparator();
    }
    
    virtual void _GetDocumentsRoot (BeFileName& path) override
    {
    path = m_docs;
    }
    
    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) override
    {
    path = m_assetsDir;
    }
    
    virtual void _GetOutputRoot (BeFileName& path) override
    {
    path = m_output;
    }
    
    virtual void _GetTempDir (BeFileName& path) override
    {
    path = m_tempDir;
    }
    
    virtual void*  _InvokeP (char const* function, void* args) override
    {
    if (0==strncmp (function, "foo", 3))
        {
        // WIP make a call on m_currentTestCase
        return new Utf8String ("foo");
        }
    if (0==strncmp (function, "getViewData", 11))
        {
        id<UIApplicationDelegate> d = [[UIApplication sharedApplication] delegate];
        BeTestiOSAppDelegate* app   = (BeTestiOSAppDelegate*) d;
        IosViewData* viewData       = new IosViewData ();
        viewData->m_uiView          = (__bridge void*)[app View];
        return viewData;
        }
    return NULL;
    }
    
    static RefCountedPtr<BeTestHost> Create (char const* docs, char const* assets, char const* temp, char const* localState) {return new BeTestHost (docs,assets,temp,localState);}
};
