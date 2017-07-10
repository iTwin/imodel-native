/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/BackDoor/BackDoor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include <WebServices/iModelHub/Client/Briefcase.h>
#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
#include <WebServices/iModelHub/Client/BreakHelper.h>
#endif
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

namespace BackDoor
    {
    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    09/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace iModelConnection 
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        IWSRepositoryClientPtr GetRepositoryClient(iModelConnectionPtr connection)
            {
            return connection->GetRepositoryClient();
            }
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetRepositoryClient(iModelConnectionPtr connection, IWSRepositoryClientPtr client)
            {
            connection->SetRepositoryClient(client);
            }
        }
    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    09/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace Briefcase 
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        iModelConnectionPtr GetiModelConnectionPtr(BriefcasePtr briefcase)
            {
            return briefcase->GetiModelConnectionPtr();
            }
        }

    /*--------------------------------------------------------------------------------**//**
    * @bsimethod                                             Algirdas.Mikoliunas    10/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace iModelManager
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    10/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        iModelConnectionPtr GetiModelConnectionPtr(iModel::Hub::iModelManager* imodelManager)
            {
            return imodelManager->GetiModelConnectionPtr();
            }
        }
#if defined (ENABLE_IMODELHUB_CRASH_TESTS)
    namespace BreakHelper
        {
        /*--------------------------------------------------------------------------------**//**
        * @bsimethod                                             Algirdas.Mikoliunas    09/16
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetBreakpoint(Breakpoints breakpoint)
            {
            iModel::Hub::BreakHelper::SetBreakpoint(breakpoint);
            }
        }
#endif
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
