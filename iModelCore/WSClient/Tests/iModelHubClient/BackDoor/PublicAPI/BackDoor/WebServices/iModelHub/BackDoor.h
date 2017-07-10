/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/iModelHubClient/BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "iModelHubTests.h"
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/iModelHub/Client/iModelManager.h>
#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
#include <WebServices/iModelHub/Client/BreakHelper.h>
#endif
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

//=======================================================================================
//! Helper functions that published (and non-published) tests can use to check on results.
//! They are in a namespace called "backdoor" to emphasize the fact that they allow published
//! tests to call functions that are not part of the published api.
// @bsiclass                                                 
//=======================================================================================
namespace BackDoor
    {
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             09/2016
    //---------------------------------------------------------------------------------------
    namespace iModelConnection
        {
        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             09/2016
        //---------------------------------------------------------------------------------------
        IWSRepositoryClientPtr GetRepositoryClient(iModelConnectionPtr connection);

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             09/2016
        //---------------------------------------------------------------------------------------
        void SetRepositoryClient(iModelConnectionPtr connection, IWSRepositoryClientPtr client);
        }

    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    09/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace iModelManager
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        iModelConnectionPtr GetiModelConnectionPtr(iModel::Hub::iModelManager* iModelManager);
        }
    
    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    10/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace Briefcase
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    10/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        iModelConnectionPtr GetiModelConnectionPtr(BriefcasePtr briefcase);
        }
#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    09/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace BreakHelper
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetBreakpoint(Breakpoints breakpoint);
        }
#endif
}

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
