/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFunctionMode.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFunctionMode.cpp,v 1.2 2006/11/01 20:55:55 Donald.Morissette Exp $
//
// Class: ActiveImageFunctionMode
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageFunctionMode.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

// Start the command ID at AIFM_MIN_COMMAND_ID
uint32_t CActiveImageFunctionMode::m_CommandID = AIFM_MIN_COMMAND_ID;

// Initialize the Prompt map
char* CActiveImageFunctionMode::m_PromptMap[AIFM_MAX_COMMAND_ID - AIFM_MIN_COMMAND_ID];
char* CActiveImageFunctionMode::m_ToolTipMap[AIFM_MAX_COMMAND_ID - AIFM_MIN_COMMAND_ID];


//-----------------------------------------------------------------------------
// Public
// Default Constructor
//-----------------------------------------------------------------------------
CActiveImageFunctionMode::CActiveImageFunctionMode()
{
    // Set the cursor to NULL, because by default, we will use a windows
    // default cursor: The Arrow.
    m_Cursor = NULL;
}


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
CActiveImageFunctionMode::~CActiveImageFunctionMode()
{
}


//-----------------------------------------------------------------------------
// Public
// GetCommandID - Returns a unique command ID needed by a function object and
// associate prompt and tooltip strings to the command
//-----------------------------------------------------------------------------
uint32_t CActiveImageFunctionMode::GetCommandID(const char* pi_Prompt, const char* pi_ToolTip)
{
    uint32_t Result = (uint32_t)-1;

    // verify if the current command ID is valid
    if (m_CommandID < AIFM_MAX_COMMAND_ID)
    {
        m_PromptMap [m_CommandID - AIFM_MIN_COMMAND_ID] = const_cast<char*>(pi_Prompt);
        m_ToolTipMap[m_CommandID - AIFM_MIN_COMMAND_ID] = const_cast<char*>(pi_ToolTip);

        // increment the command ID
        Result = m_CommandID++;
    }

    return Result;
}


//-----------------------------------------------------------------------------
// Public
// GetPrompt - Returns the prompt string associated with a command
//-----------------------------------------------------------------------------
const char* CActiveImageFunctionMode::GetPrompt(UINT_PTR nID)
{
    const char* pResult = 0;

    // get a prompt if in the range
    if ((nID >= AIFM_MIN_COMMAND_ID) &&
        (nID < m_CommandID) )
        pResult = m_PromptMap[nID - AIFM_MIN_COMMAND_ID];

    return pResult;
}


//-----------------------------------------------------------------------------
// Public
// GetPrompt - Returns the tooltip string associated with a command
//-----------------------------------------------------------------------------
const char* CActiveImageFunctionMode::GetToolTip(UINT_PTR nID)
{
    const char* pResult = 0;

    // get a prompt if in the range
    if ((nID >= AIFM_MIN_COMMAND_ID) &&
        (nID < m_CommandID) )
        pResult = m_ToolTipMap[nID - AIFM_MIN_COMMAND_ID];

    return pResult;
}
//-----------------------------------------------------------------------------
// Public
// GetCursor
//-----------------------------------------------------------------------------
HCURSOR CActiveImageFunctionMode::GetCursor() const
{
    // return the default cursor, The Arrow
    return AfxGetApp()->LoadStandardCursor(IDC_ARROW);
}