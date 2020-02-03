/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageViewMode.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageViewMode.h,v 1.1 2003/02/26 20:01:03 SebastienGosselin Exp $
//-----------------------------------------------------------------------------
// Class : CActiveImageViewMode
//-----------------------------------------------------------------------------

#ifndef __CActiveImageViewMode_H__
#define __CActiveImageViewMode_H__

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "ActiveImageFunctionMode.h"

class CActiveImageDoc;
class CActiveImageView;


//-----------------------------------------------------------------------------
// Class Declaration
//-----------------------------------------------------------------------------
class CActiveImageViewMode : public CActiveImageFunctionMode
{
    public:

        ////////////////////////////////////////
        // Construction/Destruction
        ////////////////////////////////////////
                     CActiveImageViewMode();
        virtual      ~CActiveImageViewMode();


        ////////////////////////////////////////
        // Toolbar and menu Methods
        ////////////////////////////////////////

        // These static functions will modify the
        // menu and add a toolbar.
        static bool    SetupMenu(CMenu* pi_pEditMenu, CMenu* pi_pViewMenu);
        static CToolBar* 
                        SetupToolbar(CActiveImageFrame* pi_pFrame,
                                     uint32_t            pi_ID);

        
        ////////////////////////////////////////
        // Basic Methods
        ////////////////////////////////////////
        virtual uint32_t GetType() const;
        virtual void    OnDraw(CDC* pDC);
        virtual void    OnUndraw(CDC* pDC);
        virtual void    Setup();


        ////////////////////////////////////////
        // Cursor commands
        ////////////////////////////////////////
        virtual HCURSOR GetCursor() const;


        ////////////////////////////////////////
        // Mouse Commands
        ////////////////////////////////////////
        virtual void    OnLButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnLButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnLButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonDblClk (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonDown   (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnRButtonUp     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);
        virtual void    OnMouseMove     (uint32_t pi_Flags, 
                                         CPoint& pi_rPoint);

        ////////////////////////////////////////
        // Keyboard Commands
        ////////////////////////////////////////
        virtual void    OnKeyDown(uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags);
        virtual void    OnKeyUp  (uint32_t pi_Char, 
                                  uint32_t pi_RepeatCount, 
                                  uint32_t pi_Flags);
    

        ////////////////////////////////////////
        // WindowProc, for trapping messages
        ////////////////////////////////////////

        // The follwing methods respond to commands while
        // the object is active
        virtual bool   OnCommand(uint32_t pi_CommandID,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc);
        virtual bool   OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                       CActiveImageView* pi_pView,
                                       CActiveImageDoc*  pi_pDoc);

        virtual void    EndCommand ();

        void                FitAllToView();
        
        void                StartMagnifier();

    private:

        ////////////////////////////////////////
        // Private attributes
        ////////////////////////////////////////
        
        // Pointers to the document and its view
        CActiveImageDoc*    m_pDoc;
        CActiveImageView*   m_pView;
                         
        // Toolbar for default mode
        static CToolBar     m_Toolbar;

        uint32_t            m_CurFunction;
        bool                m_MagnifierState;            

        // Update Command Handlers

        // Command Handlers

        // Drawing methods
        


};


#endif
