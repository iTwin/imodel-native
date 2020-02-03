/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFunctionMode.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFunctionMode.h,v 1.3 2011/07/18 21:10:41 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Class : CActiveImageFunctionMode
//-----------------------------------------------------------------------------

#ifndef __CACTIVEIMAGEFUNCTIONMODE_H__
#define __CACTIVEIMAGEFUNCTIONMODE_H__

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HRARaster.h>
#include "ActiveImageFrame.h"


//-----------------------------------------------------------------------------
// ActiveImage Function Modes
//-----------------------------------------------------------------------------
#define AIFM_SELECTMOVE         0
#define AIFM_EDITCOPYPASTE      1
#define AIFM_NAVIGATION         2
#define AIFM_FENCE              3

#define AIFM_MIN_COMMAND_ID 15000
#define AIFM_MAX_COMMAND_ID 20000

#define AIFM_MIN_RC_COMMAND_ID  30000   // ID define in a String Table.
#define AIFM_MAX_RC_COMMAND_ID  31000

class CActiveImageView;
class CActiveImageDoc;

//-----------------------------------------------------------------------------
// Class Declaration
//-----------------------------------------------------------------------------
class CActiveImageFunctionMode
{
    public:

        ////////////////////////////////////////
        // Construction/Destruction
        ////////////////////////////////////////
                        CActiveImageFunctionMode();
        virtual         ~CActiveImageFunctionMode();


        ////////////////////////////////////////
        // Basic Methods
        ////////////////////////////////////////
        virtual uint32_t GetType() const = 0;
        virtual void    OnDraw(CDC* pDC) = 0;
        virtual void    OnUndraw(CDC* pDC) = 0;
        virtual void    Setup() = 0;


        ////////////////////////////////////////
        // Cursor commands
        ////////////////////////////////////////
        virtual HCURSOR GetCursor() const;


        ////////////////////////////////////////
        // Mouse Commands
        ////////////////////////////////////////
        virtual void    OnLButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnLButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnLButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnRButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnRButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnRButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;
        virtual void    OnMouseMove     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint) = 0;


        ////////////////////////////////////////
        // Keyboard Commands
        ////////////////////////////////////////
        virtual void    OnKeyDown(uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags) = 0;
        virtual void    OnKeyUp  (uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags) = 0;
    

        ////////////////////////////////////////
        // WindowProc, for trapping messages
        ////////////////////////////////////////

        // The follwing methods respond to commands while
        // the object is active
        virtual bool   OnCommand(uint32_t pi_CommandID,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc)
                        { return (false); };
        virtual bool   OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc)
                        { return (false); };

        virtual void    EndCommand ()
                        { };

        // Generic message handling.  (Should rarely be used).
        virtual bool   WindowProc(uint32_t pi_Message, 
                                   WPARAM pi_wParam, LPARAM pi_lParam)
                        { return (false); };


        // Function to obtain a command ID
        static uint32_t  GetCommandID(const char* pi_Prompt, const char* pi_ToolTip);
        static const char* 
                        GetPrompt(UINT_PTR nID);
        static const char* 
                        GetToolTip(UINT_PTR nID);


    protected:

        ////////////////////////////////////////
        // protected attributes
        ////////////////////////////////////////

        // Cursor to display when using this function mode
        HCURSOR         m_Cursor;

    private:

        // Current command ID identifier that can be obtained by
        // GetCommandID()
        static uint32_t  m_CommandID;

        // Command prompt
        static  char* m_PromptMap[AIFM_MAX_COMMAND_ID - AIFM_MIN_COMMAND_ID];
        static  char* m_ToolTipMap[AIFM_MAX_COMMAND_ID - AIFM_MIN_COMMAND_ID];
};

#endif
