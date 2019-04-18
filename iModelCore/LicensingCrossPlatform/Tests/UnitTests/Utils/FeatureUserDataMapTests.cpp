/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "FeatureUserDataMapTests.h"

#include <Licensing/Utils/FeatureUserDataMap.h>

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

TEST_F(FeatureUserDataMapTests, AddAttribute_Success)
    {
    FeatureUserDataMap featureAttribute;

    EXPECT_SUCCESS(featureAttribute.AddAttribute("Manufacturer", "Bentley Systems, Inc."));    
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Website", "https://www.w3schools.com"));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Title", "Mobile App"));

    EXPECT_EQ(3, featureAttribute.GetCount());

    Utf8String value;
    featureAttribute.GetValue("Title", value);
    EXPECT_EQ(value, "Mobile App");
    }

TEST_F(FeatureUserDataMapTests, GetKeys_Success)
    {
    FeatureUserDataMap featureAttribute;

    EXPECT_SUCCESS(featureAttribute.AddAttribute("Manufacturer", "Bentley Systems, Inc."));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Website", "https://www.w3schools.com"));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Title", "Mobile App"));

    EXPECT_EQ(3, featureAttribute.GetCount());

    Utf8StringVector keys;
    EXPECT_EQ(featureAttribute.GetKeys(keys), 3);

    bool keyFound = false;
    for (Utf8String key : keys)
        {
        if (key.Equals("Website"))
            {
            keyFound = true;
            break;
            }
        }
   
    EXPECT_TRUE(keyFound);
    }

TEST_F(FeatureUserDataMapTests, GetMapEntries_Success)
    {
    FeatureUserDataMap featureAttribute;

    EXPECT_SUCCESS(featureAttribute.AddAttribute("Manufacturer", "Bentley Systems, Inc."));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Website", "https://www.w3schools.com"));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Title", "Mobile App"));

    EXPECT_EQ(3, featureAttribute.GetCount());

    Utf8StringVector attributes;
    EXPECT_EQ(featureAttribute.GetMapEntries(attributes), 3);

    bool attributeFound = false;
    for (Utf8String attribute : attributes)
        {
        if (attribute.Equals("Website=https://www.w3schools.com"))
            {
            attributeFound = true;
            break;
            }
        }

    EXPECT_TRUE(attributeFound);
    }

TEST_F(FeatureUserDataMapTests, GetValue_Success)
    {
    FeatureUserDataMap featureAttribute;

    EXPECT_SUCCESS(featureAttribute.AddAttribute("Manufacturer", "Bentley Systems, Inc."));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Website", "https://www.w3schools.com"));
    EXPECT_SUCCESS(featureAttribute.AddAttribute("Title", "Mobile App"));

    EXPECT_EQ(3, featureAttribute.GetCount());

    Utf8String value;
    featureAttribute.GetValue("Manufacturer", value);
    EXPECT_EQ(value, "Bentley Systems, Inc.");
    }
