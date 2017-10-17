/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/PublicAPI/BackDoor/ECPresentation/SelectionManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include <ECPresentation/SelectionManager.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                 Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
namespace BackDoor
    {
    /*-----------------------------------------------------------------------------**//**
    * @bsinamespace                             Grigas.Petraitis                10/2017
    +-----------+---------------+---------------+---------------+---------------+------*/
    namespace SelectionManager
        {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                            Grigas.Petraitis                10/2017
        +-------+---------------+---------------+---------------+---------------+------*/
        void RemapNodeIds(ECPresentation::SelectionManager&, bmap<uint64_t, uint64_t> const&);
        }
    }

END_ECPRESENTATIONTESTS_NAMESPACE