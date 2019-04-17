/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <string.h>

/*----------------------------------------------------------------------+
|                                                                       |
| name          initialize                                              |
|                                                                       |
| author        BarryBentley                            6/93            |
|                                                                       |
|       MicroStation calls the initialization function immediately      |
|       after loading the DLM. If the DLM returns a non-zero value,     |
|       MicroStation aborts the load. If the application is going to    |
|       return a non-zero value, it should also display a message       |
|       using dlmSystem_displayError.                                   |
|                                                                       |
|       The parameter localDlmID is a pointer to this DLM's descriptor. |
|       All of the dlm..._setFunction calls require this as a parameter.|
|                                                                       |
|       The version parameter is taken from the DLS file.  The DLM      |
|       can use this to determine if it is compatible with the MDL      |
|       application.                                                    |
|                                                                       |
+----------------------------------------------------------------------*/
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

