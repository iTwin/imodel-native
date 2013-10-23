/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECTestFixture : ::testing::Test
    {
private:
    static bool s_isLoggerInitialized;
    WString GetLogConfigurationFilename();

protected:
    ECTestFixture();
    
public:
    virtual void            SetUp () override;
    virtual void            TearDown () override;

    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static WString GetWorkingDirectoryPath(WCharCP testFixture, WCharCP fileName);
    static WString GetTestResultsFilePath (WCharCP fileName);
    static WString GetDateTime();

    static void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, WCharCP value);
    static void VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value);
    static void SetAndVerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value);
    static void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, UInt32 value);
    static void VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value);
    static void SetAndVerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value);
    static void VerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value);
    static void SetAndVerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value);
    static void VerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value);
    static void SetAndVerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value);
    static void VerifyArrayInfo (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 count, bool isFixedCount);
    static void VerifyOutOfBoundsError (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 index);
    static void VerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 start, UInt32 count);
    static void SetAndVerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 count);
    static void VerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 start, UInt32 count);
    static void SetAndVerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 count);
    static void VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 start, UInt32 count, bool isNull);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
