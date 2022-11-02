/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Bentley/Bentley.h>
#include <string.h>

/*---------------------------------------------------------------------------------**//**
* PP calls the initialization function immediately after loading the DLM. If the DLM returns a non-zero value, it aborts
* the load. If the application is going to return a non-zero value, it should also display a message using dlmSystem_displayError.
* The parameter localDlmID is a pointer to this DLM's descriptor. All of the dlm..._setFunction calls require this as a parameter.
* The version parameter is taken from the DLS file. The DLM can use this to determine if it is compatible with the MDL application.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

