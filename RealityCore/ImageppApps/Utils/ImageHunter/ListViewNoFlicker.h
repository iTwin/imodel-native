/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ListViewNoFlicker.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ListViewNoFlicker.h,v 1.1 2010/06/02 13:16:46 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ListViewNoFlicker
//-----------------------------------------------------------------------------
// Defines an inherited Windows Form ListView with double buffering support.
//-----------------------------------------------------------------------------

#pragma once

public ref class ListViewNoFlicker : System::Windows::Forms::ListView
{
public:
    ListViewNoFlicker();
protected:
    virtual void OnNotifyMessage(System::Windows::Forms::Message m) override;
};
