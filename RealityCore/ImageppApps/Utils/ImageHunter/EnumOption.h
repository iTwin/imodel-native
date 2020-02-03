/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/EnumOption.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/EnumOption.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : EnumOption
//-----------------------------------------------------------------------------
// Serves as a message object between the UI and the rest of the application.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// EnumOption class
//-----------------------------------------------------------------------------
ref class EnumOption
{
public:
    EnumOption() {};
	EnumOption(int id, System::String^ label) {
		 m_id = id;
		 m_label = label;
	};
	int GetID() { return m_id; };
	System::String^ GetLabel() { return m_label; };
	virtual System::String^ ToString() override { return m_label; };

protected:
	int m_id;
	System::String^ m_label;
};