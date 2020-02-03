/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ExternalToolsUI.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ExternalToolsUI.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ExternalToolsUI
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ExternalToolsUI.h"

using namespace ImageHunter;
using namespace System; 
using namespace System::Collections;

#pragma region Form Events

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::ExternalToolsUI_Load(System::Object^  sender, System::EventArgs^  e) 
{
    DisableControls();

    // Backing up the actual state of the external tools
    ExternalTools::GetInstance()->SaveActualState();
    
    // Loading the external tools
    SortedList^ tools = ExternalTools::GetInstance()->GetExternalTools();
    lstExternalTools->Items->Clear();

    if (tools->Count > 0)
    {
        for (int i = 0; i < tools->Count; ++i)
        {
            ExternalTool^ tool = (ExternalTool^)tools[i];
            lstExternalTools->Items->Add(tool->GetName());
        }

        lstExternalTools->SelectedIndex = 0;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::lstExternalTools_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
{
    if (lstExternalTools->SelectedItems->Count <= 0)
    {
        DisableControls();
    } 
    else 
    {
        // Updating UI when at least one tool is selected
        EnableControls();
        ExternalTool^ tool = ExternalTools::GetInstance()->GetExternalTool(lstExternalTools->SelectedIndex);
        txtName->Text = tool->GetName();
        txtCommand->Text = tool->GetExecutable();
        txtArguments->Text = tool->GetArguments();
        txtName->Focus();
        
        // Enabling/Disabling move up/down buttons
        if (lstExternalTools->Items->Count == 1) 
        {
            btnMoveDown->Enabled = false;
            btnMoveUp->Enabled = false;
        } 
        else 
        {
            if (lstExternalTools->SelectedIndices[0] == 0)
            {
                btnMoveUp->Enabled = false;
            } 
            else if (lstExternalTools->SelectedIndex == lstExternalTools->Items->Count - 1) 
            {
                btnMoveDown->Enabled = false;
            }
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Closing the form
    this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnSave_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Closing the form
    this->DialogResult = System::Windows::Forms::DialogResult::OK;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnRemove_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Removing each selected item
    int index = lstExternalTools->SelectedIndex;
    ExternalTools::GetInstance()->RemoveTool(index);
    lstExternalTools->Items->Remove(lstExternalTools->SelectedItem);

    for (int i = index; i < lstExternalTools->Items->Count; ++i)
    {
        ExternalTool^ tool = ExternalTools::GetInstance()->RemoveTool(i+1);
        tool->SetID(i);
        ExternalTools::GetInstance()->AddTool(tool);
    }

    // Selecting the removed index or the last element in the list
    if (index < lstExternalTools->Items->Count)
    {
        lstExternalTools->SelectedIndex = index;
    }
    else
    {
        lstExternalTools->SelectedIndex = lstExternalTools->Items->Count - 1;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnAdd_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Adding a new tool with default values
    ExternalTool^ tool = gcnew ExternalTool(lstExternalTools->Items->Count, L"tool.exe", L"Tool [new]", L"%s");
    ExternalTools::GetInstance()->AddTool(tool);
    lstExternalTools->Items->Add(tool->GetName());
    lstExternalTools->Focus();
    lstExternalTools->SelectedIndex = lstExternalTools->Items->Count - 1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnMoveUp_Click(System::Object^  sender, System::EventArgs^  e) 
{
    // Swapping the selected item with the one above it
    int index = lstExternalTools->SelectedIndex;
    
    lstExternalTools->SelectedIndex = -1;
    lstExternalTools->Items->RemoveAt(index);
    lstExternalTools->Items->RemoveAt(index - 1);

    ExternalTool^ toolToMoveUp = ExternalTools::GetInstance()->RemoveTool(index);
    ExternalTool^ toolToMoveDown = ExternalTools::GetInstance()->RemoveTool(index - 1);
    
    toolToMoveUp->SetID(toolToMoveDown->GetID());
    toolToMoveDown->SetID(toolToMoveDown->GetID() + 1);

    ExternalTools::GetInstance()->AddTool(toolToMoveUp);
    ExternalTools::GetInstance()->AddTool(toolToMoveDown);

    lstExternalTools->Items->Insert(toolToMoveUp->GetID(), toolToMoveUp->GetName());
    lstExternalTools->Items->Insert(toolToMoveDown->GetID(), toolToMoveDown->GetName());

    lstExternalTools->SelectedIndex = toolToMoveUp->GetID();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnMoveDown_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Swapping the selected item with the one below it
    int index = lstExternalTools->SelectedIndex;

    lstExternalTools->SelectedIndex = -1;
    lstExternalTools->Items->RemoveAt(index + 1);
    lstExternalTools->Items->RemoveAt(index);

    ExternalTool^ toolToMoveUp = ExternalTools::GetInstance()->RemoveTool(index + 1);
    ExternalTool^ toolToMoveDown = ExternalTools::GetInstance()->RemoveTool(index);
    
    
    toolToMoveUp->SetID(index);
    toolToMoveDown->SetID(index + 1);

    ExternalTools::GetInstance()->AddTool(toolToMoveUp);
    ExternalTools::GetInstance()->AddTool(toolToMoveDown);

    lstExternalTools->Items->Insert(toolToMoveUp->GetID(), toolToMoveUp->GetName());
    lstExternalTools->Items->Insert(toolToMoveDown->GetID(), toolToMoveDown->GetName());

    lstExternalTools->SelectedIndex = toolToMoveDown->GetID();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::btnCommandBrowse_Click(System::Object^  sender, System::EventArgs^  e) 
{
    // Asking the user to select the executable to associate with the current tool
    if (OpenCommandDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        txtCommand->Text = OpenCommandDialog->FileName;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::txtArguments_TextChanged(System::Object^  sender, System::EventArgs^  e) 
{
    if (lstExternalTools->SelectedIndex != -1)
    {
        // Updating the external tool with the text entered by the user
        ExternalTool^ tool = ExternalTools::GetInstance()->RemoveTool(lstExternalTools->SelectedIndex);
        tool->SetArguments(txtArguments->Text);
        ExternalTools::GetInstance()->AddTool(tool);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::txtName_TextChanged(System::Object^  sender, System::EventArgs^  e) 
{
    if (lstExternalTools->SelectedIndex != -1)
    {
        // The handler is remove to avoid losing the focus on the textbox
        this->lstExternalTools->SelectedIndexChanged -= gcnew System::EventHandler(this, &ExternalToolsUI::lstExternalTools_SelectedIndexChanged);
        ExternalTool^ tool = ExternalTools::GetInstance()->RemoveTool(lstExternalTools->SelectedIndex);
        tool->SetName(txtName->Text);
        ExternalTools::GetInstance()->AddTool(tool);
        lstExternalTools->Items[lstExternalTools->SelectedIndex] = txtName->Text;
        // We now add back the event handler
        lstExternalTools->SelectedIndexChanged += gcnew System::EventHandler(this, &ExternalToolsUI::lstExternalTools_SelectedIndexChanged);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void ExternalToolsUI::txtCommand_TextChanged(System::Object^  sender, System::EventArgs^  e)
{
    if (lstExternalTools->SelectedIndex != -1)
    {
        // Updating the external tool with the text entered by the user
        ExternalTool^ tool = ExternalTools::GetInstance()->RemoveTool(lstExternalTools->SelectedIndex);
        tool->SetExecutable(txtCommand->Text);
        ExternalTools::GetInstance()->AddTool(tool);
    }
}

#pragma endregion