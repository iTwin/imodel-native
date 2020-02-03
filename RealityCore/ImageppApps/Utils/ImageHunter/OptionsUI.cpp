/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/OptionsUI.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/OptionsUI.cpp,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class OptionsUI
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "OptionsUI.h"

using namespace System;
using namespace ImageHunter;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void OptionsUI::btnAccept_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Saving preferences and closing form
    this->m_ExcludedExtensions = txtExcludedExtensions->Text;
    this->m_IsSendToMenuDisplayed = chkDisplaySendTo->Checked;
	this->DialogResult = System::Windows::Forms::DialogResult::OK;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void OptionsUI::btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Closing form
    this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void OptionsUI::OptionsUI_Load(System::Object^  sender, System::EventArgs^  e)
{
    // Loading preferences in the UI
    chkDisplaySendTo->Checked = m_IsSendToMenuDisplayed;
    txtExcludedExtensions->Text = m_ExcludedExtensions;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void OptionsUI::SetExcludedExtensions(String^ excludedExtensions)
{
    this->m_ExcludedExtensions = excludedExtensions;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::String^ OptionsUI::GetExcludedExtensions()
{
    return this->m_ExcludedExtensions;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void OptionsUI::SetIsSendToMenuDisplayed(bool isChecked)
{
    this->m_IsSendToMenuDisplayed = isChecked;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Boolean OptionsUI::IsSendToMenuDisplayed()
{
    return this->m_IsSendToMenuDisplayed;
}
