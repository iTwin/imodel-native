/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/BasicTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"
#include "ImporterCommandBuilder.h"

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct BasicTests : public ImporterTestBaseFixture
{
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BasicTests, createIBimFromDwg)
    {
    
    LineUpFiles(L"createIBim.ibim", L"basictype.dwg", true); 

    }
