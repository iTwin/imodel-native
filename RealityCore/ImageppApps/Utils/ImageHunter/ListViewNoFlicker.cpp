/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ListViewNoFlicker.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ListViewNoFlicker.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class 
//-----------------------------------------------------------------------------

#include "Stdafx.h"
#include "ListViewNoFlicker.h"

//using namespace System;
using namespace System::Windows::Forms;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ListViewNoFlicker::ListViewNoFlicker()
{
    //Activate double buffering
    this->SetStyle(ControlStyles::OptimizedDoubleBuffer | ControlStyles::AllPaintingInWmPaint, true);

    //Enable the OnNotifyMessage event so we get a chance to filter out 
    // Windows messages before they get to the form's WndProc
    this->SetStyle(ControlStyles::EnableNotifyMessage, true);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ListViewNoFlicker::OnNotifyMessage(Message m)
{
    //Filter out the WM_ERASEBKGND message
    if(m.Msg != 0x0014)
    {
        __super::OnNotifyMessage(m);
    }
}

