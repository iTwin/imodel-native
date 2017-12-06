/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/BackDoor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECPresentation/BackDoor.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsinamespace                                Grigas.Petraitis                11/2017
//=======================================================================================
namespace BackDoor
    {
    /*-----------------------------------------------------------------------------**//**
    * @bsinamespace                             Grigas.Petraitis                11/2017
    +-----------+---------------+---------------+---------------+---------------+------*/
    namespace RulesDrivenECPresentationManager
        {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                            Grigas.Petraitis                11/2017
        +-------+---------------+---------------+---------------+---------------+------*/
        folly::Executor& GetExecutor(ECPresentation::RulesDrivenECPresentationManager const& manager)
            {
            return manager.GetExecutor();
            }
        }
    }

END_BENTLEY_ECPRESENTATION_NAMESPACE