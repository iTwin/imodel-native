/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
  
bool ECTestFixture::s_isLoggerInitialized = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    //LoggingConfig::ActivateProvider(CONSOLE_LOGGING_PROVIDER);

    #if defined (WIP_ECOBJECTS_TEST)
    if (!s_isLoggerInitialized)
        {
        LoggingConfig::ActivateProvider(LOG4CXX_LOGGING_PROVIDER);
        LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, GetLogConfigurationFilename().c_str());

        LoggingConfig::SetSeverity(L"ECObjectsNative", LOG_WARNING);
        s_isLoggerInitialized = true;
        }
    #endif

    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECN::ECSchemaReadContext::Initialize (assetsDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::SetUp ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Josh.Schifter   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECTestFixture::TearDown ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (testData);
    testData.AppendToPath (L"SeedData");
    testData.AppendToPath (dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTempDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot (testData);
    testData.AppendToPath (dataFile);
    return testData;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestResultsFilePath (WCharCP fileName)
    {
    BeFileName filePath;
    BeTest::GetHost().GetOutputRoot (filePath);

    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    filePath.AppendToPath(processorArchitecture);
    filePath.AppendToPath(L"TestResults");
    BeFileName::CreateNewDirectory (filePath.c_str());
    
    filePath.AppendToPath(fileName);
    
    return filePath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetLogConfigurationFilename()
    {
    /* ***WIP_ECOBJECTS_TEST
    wchar_t filepath[MAX_PATH];

    if ((0 != GetEnvironmentVariableW(L"BENTLEY_LOGGING_CONFIG", filepath, MAX_PATH)) && (0 ==_waccess(filepath, 0)))
        {
        wprintf (L"ECObjects.dll configuring logging with %s (Set by BENTLEY_LOGGING_CONFIG environment variable.)\n", filepath);
        return filepath;
        }
    else if (SUCCESS == CheckProcessDirectory(filepath, sizeof(filepath)))
        {
        wprintf (L"ECObjects.dll configuring logging using %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
        return filepath;
        }
    else if (0 != GetEnvironmentVariableW(L"OutRoot", filepath, MAX_PATH))
        {
        WCharP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
        wcscat (filepath, processorArchitecture);
        wcscat (filepath, L"\\Product\\ECObjectsTests\\logging.config.xml");
        
        if (0 ==_waccess(filepath, 0))
            {
            wprintf (L"ECObjects.dll configuring logging with %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
            return filepath;
            }
        }
    */

    return L"";
    }       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString  ECTestFixture::GetDateTime ()
    {
    struct tm timeinfo;
    BeTimeUtilities::ConvertUnixMillisToTm (timeinfo, BeTimeUtilities::GetCurrentTimeAsUnixMillis());   // GMT

    char buff[32];
    strftime (buff, sizeof(buff), "%y/%m/%d", &timeinfo);
    Utf8String dateTime (buff);
    dateTime.append (" ");
    strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);
    dateTime.append (buff);
    return WString(dateTime.c_str(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, WCharCP value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_STREQ (value, v.GetString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    return VerifyString (instance, v, accessString, false, 0, value);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyString (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value)
    {
    v.SetString(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyString (instance, v, accessString, value);
    }
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, bool useIndex, UInt32 index, UInt32 value)
    {
    v.Clear();
    if (useIndex)
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, index));
    else
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetInteger());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    return VerifyInteger (instance, v, accessString, false, 0, value);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyInteger (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 value)
    {
    v.SetInteger(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyInteger (instance, v, accessString, value);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetDouble());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyDouble (IECInstanceR instance, ECValueR v, WCharCP accessString, double value)
    {
    v.SetDouble(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyDouble (instance, v, accessString, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (value, v.GetLong());
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyLong (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt64 value)
    {
    v.SetLong(value);
    EXPECT_TRUE (SUCCESS == instance.SetValue (accessString, v));
    VerifyLong (instance, v, accessString, value);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyArrayInfo (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 count, bool isFixedCount)
    {
    v.Clear();
    EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString));
    EXPECT_EQ (count, v.GetArrayInfo().GetCount());
    EXPECT_EQ (isFixedCount, v.GetArrayInfo().IsFixedCount());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyOutOfBoundsError (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 index)
    {
    v.Clear();    
    EXPECT_TRUE (ECOBJECTS_STATUS_IndexOutOfRange == instance.GetValue (v, accessString, index));
    EXPECT_TRUE (ECOBJECTS_STATUS_IndexOutOfRange == instance.SetValue (accessString, v, index));
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 start, UInt32 count)
    {
    WString incrementingString = value;
   
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        incrementingString.append (L"X");
        VerifyString (instance, v, accessString, true, i, incrementingString.c_str());
        }
    }  
              
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyStringArray (IECInstanceR instance, ECValueR v, WCharCP accessString, WCharCP value, UInt32 count)
    {
    WString incrementingString = value;
    for (UInt32 i=0 ; i < count ; i++)        
        {
        incrementingString.append (L"X");
        v.SetString(incrementingString.c_str());

        // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECOBJECTS_STATUS_PropertyValueMatchesNoChange 
        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status || ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status);
        }
    
    VerifyStringArray (instance, v, accessString, value, 0, count);
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::VerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 start, UInt32 count)
    {       
    for (UInt32 i=start ; i < start + count ; i++)        
        {
        VerifyInteger (instance, v, accessString, true, i, baseValue++);
        }
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
void ECTestFixture::SetAndVerifyIntegerArray (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 baseValue, UInt32 count)
    {
    for (UInt32 i=0 ; i < count ; i++)        
        {
        v.SetInteger(baseValue + i); 

        // since the test sets some of the array values more than once to the same value we must check SUCCESS || ECOBJECTS_STATUS_PropertyValueMatchesNoChange 
        ECObjectsStatus status = instance.SetValue (accessString, v, i);
        EXPECT_TRUE (SUCCESS == status || ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status);
        }
        
    VerifyIntegerArray (instance, v, accessString, baseValue, 0, count);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECTestFixture::VerifyIsNullArrayElements (IECInstanceR instance, ECValueR v, WCharCP accessString, UInt32 start, UInt32 count, bool isNull)
    {
    for (UInt32 i = start ; i < start + count ; i++)    
        {
        v.Clear();
        EXPECT_TRUE (SUCCESS == instance.GetValue (v, accessString, i));
        EXPECT_TRUE (isNull == v.IsNull());        
        }
    }

END_BENTLEY_ECOBJECT_NAMESPACE

