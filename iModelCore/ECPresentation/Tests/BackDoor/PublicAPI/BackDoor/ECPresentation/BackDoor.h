/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>

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
        folly::Executor& GetExecutor(ECPresentation::RulesDrivenECPresentationManager const&);
        }
    }

END_BENTLEY_ECPRESENTATION_NAMESPACE