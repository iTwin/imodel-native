/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <string.h>

Public int initialize
(
char        *filenameP,
char        *taskIDP,
void        *localDlmID,    /*  => Uniquely identifies the DLM */
uint32_t     version        /*  => Value is taken from the DLM Spec Source */
)
    {
    return SUCCESS;
    }

