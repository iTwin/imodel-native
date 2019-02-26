/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/ECPresentationTest.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

bool BeAssertIgnoreContext::s_value = true;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTest::SetUp()
    {
    IECPresentationManager::SetSerializer(new DefaultECPresentationSerializer());
    }
