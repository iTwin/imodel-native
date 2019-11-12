/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "StubLocalState.h"

RuntimeJsonLocalState* StubLocalState::Instance()
    {
    static RuntimeJsonLocalState* s_instance = nullptr;
    if (nullptr == s_instance)
    s_instance = new RuntimeJsonLocalState();
    return s_instance;
    }
