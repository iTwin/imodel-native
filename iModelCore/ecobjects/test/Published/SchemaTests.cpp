
/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SchemaTest, ValidateConstructor)
    {
    SchemaP schema = new Schema(L"wd", L"Widgets", 1, 0);    
    EXPECT_STREQ (L"wd", schema->GetSchemaPrefix());
    EXPECT_STREQ (L"Widgets", schema->GetSchemaName());
    /*TestDgnManager tdm ("2dMetricGeneral.dgn", __FILE__, OPENMODE_READWRITE);
    MSDgnFilePtr dgnFile = tdm.GetLoadedDgnPtr();
    //checks to make sure file was created
    if (dgnFile.IsNull())
        FAIL();
    
    EXPECT_EQ(false, dgnFile->IsReadOnly());*/
    };

END_BENTLEY_EC_NAMESPACE