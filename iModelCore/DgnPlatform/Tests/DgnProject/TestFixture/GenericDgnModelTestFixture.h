/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GenericDgnModelTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Sam.Wilson     02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModelTestFixture : public testing::Test
{
protected:
    bool                     m_is3d;
    Dgn::ScopedDgnHost       m_host;
    DgnDbTestDgnManager      m_testDgnManager;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson     02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
GenericDgnModelTestFixture (char const* sourcefile, bool is3d)
    :
    m_is3d(is3d),
    m_testDgnManager (is3d? L"3dMetricGeneral.idgndb": L"2dMetricGeneral.idgndb", sourcefile)
    {
    BeAssert( NULL != GetDgnModelP() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson     02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~GenericDgnModelTestFixture ()
    {
    }

    DgnModelP       GetDgnModelP() const {return m_testDgnManager.GetDgnModelP();}
    DgnDbP          GetDgnProjectP() const {return m_testDgnManager.GetDgnProjectP();}
    void            CloseTestFile() {m_testDgnManager.CloseTestFile();}
    BentleyStatus   ReopenTestFile() {return m_testDgnManager.OpenTestFile ();}
    bool            Is3d() const {return m_is3d;}
};

END_DGNDB_UNIT_TESTS_NAMESPACE
