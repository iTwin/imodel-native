/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/AddFolderUI.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/AddFolderUI.cpp,v 1.4 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class AddFolderUI
//-----------------------------------------------------------------------------
#include "StdAfx.h"

#include <lm.h>

#include "AddFolderUI.h"
#include "HuntTools.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace ImageHunter;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::AddFolderUI_Load(System::Object^  sender, System::EventArgs^  e)
{
    // Loading folders history
    array<Object^>^ history = m_BrowsingHistory->ToArray();
    cboFolders->Items->AddRange(history);

    if (cboFolders->Items->Count <= 0)
    {
        // Defaulting to Home directory
        cboFolders->Items->Add(Environment::GetFolderPath(Environment::SpecialFolder::DesktopDirectory));
    }

    // Loading selected folders
    for each (String^ source in m_SearchSources)
    {
        lstSelectedFolders->Items->Add(source);
    }

    // Opening the first selected folder if available
    if (lstSelectedFolders->Items->Count > 0)
    {
        cboFolders->Text = lstSelectedFolders->Items[0]->ToString();
        cboFolders_SelectedIndexChanged(nullptr, EventArgs::Empty);
    }
    else
    {
        // Defaulting to last used folder
        cboFolders->SelectedIndex = cboFolders->Items->Count - 1;
    }
    // Updating UI
    btnAdd->Enabled = false;
    btnRemove->Enabled = false;
    btnMoveUp->Enabled = false;
    btnMoveDown->Enabled = false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::LoadBrowsingHistory(ArrayList^ history)
{
    m_BrowsingHistory->AddRange(history);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::LoadSearchSources(ArrayList^ sources)
{
    m_SearchSources->AddRange(sources->ToArray());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ AddFolderUI::GetSearchSources()
{
    return m_SearchSources;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ AddFolderUI::GetBrowsingHistory()
{
    return m_BrowsingHistory;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::AddFolderUI_Closing(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e)
{
    // Saving Browsing History
    m_BrowsingHistory->Clear();
    for each (String^ dir in cboFolders->Items)
    {
        m_BrowsingHistory->Add(dir);
    }
}

//-----------------------------------------------------------------------------
// Method: btnFolderUp_Click
// Goes one folder up in the current hierarchy.
//-----------------------------------------------------------------------------
System::Void AddFolderUI::btnFolderUp_Click(System::Object^  sender, System::EventArgs^  e)
{
    String^ directory = cboFolders->Text;
    String^ newDirectory = L"";
    int slashPos = directory->LastIndexOf('\\');

    if (!directory->Equals((String^) MY_NETWORK_PLACES))
    {
        if (slashPos != -1)
        {
            if (directory->Substring(0, 2)->Equals(L"\\\\") && slashPos < 2)
            {
                newDirectory = (String^) MY_NETWORK_PLACES;
            }
            else if (!(slashPos == 2 && directory->Length == 3))
            // If the slash position equals 2 and the current
            // directory length is 3, that means that we are
            // at the root of a drive. We can only go to My
            // Computer from here so we leave the new directory
            // empty.

            {
                newDirectory = directory->Substring(0, slashPos);
                if (newDirectory->Length == 2)
                {
                    // Adding a slash when we only have the drive letter
                    newDirectory += L"\\";
                }
            }
        }
    }    
    cboFolders->Text = newDirectory;
    OpenDirectory(newDirectory);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnAdd_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (lstFolders->SelectedItems->Count > 0)
    {
        // We add this folder to the combo box list
        if (!cboFolders->Items->Contains(cboFolders->Text))
        {
            if (cboFolders->Items->Count >= MAX_BROWSING_HISTORY)
            {
                cboFolders->Items->Remove(cboFolders->Items[0]);
            }
            cboFolders->Items->Add(cboFolders->Text);
        }

        for each (ListViewItem^ item in lstFolders->SelectedItems)
        {
            String^ dir = static_cast<String^>(item->Tag);
            if (!lstSelectedFolders->Items->Contains(dir) &&
                !dir->Equals((String^) MY_NETWORK_PLACES))
            {
                lstSelectedFolders->Items->Add(dir);
                item->Selected = false;
            }
        }
    }
    lstSelectedFolders_SelectedIndexChanged(nullptr, EventArgs::Empty);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnRemove_Click(System::Object^  sender, System::EventArgs^  e)
{
    int index = lstSelectedFolders->SelectedIndices[0];
    // Removing every selected folder
    while (lstSelectedFolders->SelectedItems->Count > 0)
    {
        lstSelectedFolders->Items->Remove(lstSelectedFolders->SelectedItem);
    }
    
    // Selecting the first selected index or the last item
    if (index < lstSelectedFolders->Items->Count)
    {
        lstSelectedFolders->SelectedIndex = index;
    }
    else
    {
        lstSelectedFolders->SelectedIndex = lstSelectedFolders->Items->Count - 1;
    }
    lstSelectedFolders_SelectedIndexChanged(nullptr, EventArgs::Empty);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnMoveUp_Click(System::Object^  sender, System::EventArgs^  e)
{
    int index = lstSelectedFolders->SelectedIndices[0];
    String^ dir = static_cast<String^>(lstSelectedFolders->SelectedItem);
    lstSelectedFolders->Items->RemoveAt(index);
    lstSelectedFolders->Items->Insert(index - 1, dir);
    lstSelectedFolders->SelectedIndex = index - 1;
    lstSelectedFolders_SelectedIndexChanged(nullptr, EventArgs::Empty);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnMoveDown_Click(System::Object^  sender, System::EventArgs^  e)
{
    int index = lstSelectedFolders->SelectedIndices[0];
    String^ dir = static_cast<String^>(lstSelectedFolders->SelectedItem);
    lstSelectedFolders->Items->RemoveAt(index);
    lstSelectedFolders->Items->Insert(index + 1, dir);
    lstSelectedFolders->SelectedIndex = index + 1;
    lstSelectedFolders_SelectedIndexChanged(nullptr, EventArgs::Empty);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnAccept_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Saving selected search folders
    m_SearchSources->Clear();
    for each (String^ dir in lstSelectedFolders->Items)
    {
        m_SearchSources->Add(dir);
    }

    // Closing the form
    this->DialogResult = System::Windows::Forms::DialogResult::OK;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::btnCancel_Click(System::Object^  sender, System::EventArgs^  e)
{
    this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
    this->Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::lstSelectedFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
{
    // Enabling buttons depending on the selection made
    if (lstSelectedFolders->SelectedItems->Count > 0)
    {
        btnRemove->Enabled = true;

        if (lstSelectedFolders->SelectedItems->Count == 1)
        {
            // Enabling Move Up/Down buttons
            if (lstSelectedFolders->SelectedItems->Count == 1)
            {
                if (lstSelectedFolders->SelectedIndices[0] != 0)
                {
                    btnMoveUp->Enabled = true;
                }
                else
                {
                    btnMoveUp->Enabled = false;
                }

                if (lstSelectedFolders->SelectedIndices[0] != lstSelectedFolders->Items->Count - 1)
                {
                    btnMoveDown->Enabled = true;
                }
                else
                {
                    btnMoveDown->Enabled = false;
                }
            }
        }
        else
        {
            btnMoveUp->Enabled = false;
            btnMoveDown->Enabled = false;
        }
    }
    else
    {
        btnRemove->Enabled = false;
        btnMoveUp->Enabled = false;
        btnMoveDown->Enabled = false;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::lstFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
{
    if (lstFolders->SelectedItems->Count > 0)
    {
        if (lstFolders->SelectedItems[0]->Text == (String^) MY_NETWORK_PLACES)
        {
            btnAdd->Enabled = false;
        }
        else
        {
            btnAdd->Enabled = true;
        }
    }
    else
    {
        btnAdd->Enabled = false;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::lstFolders_DoubleClick(System::Object^ sender, System::EventArgs^ e)
{
    if (lstFolders->SelectedItems->Count == 1)
    {
        String^ directory = static_cast<String^>(lstFolders->SelectedItems[0]->Tag);
        cboFolders->Text = directory;
        OpenDirectory(directory);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::cboFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
{
    OpenDirectory(cboFolders->Text);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::cboFolders_KeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
{
    if (e->KeyCode == Keys::Enter)
    {
        OpenDirectory(cboFolders->Text);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::OpenDirectory(String^ directory)
{
    if (directory->Equals(String::Empty))
    {
        // When the directory to open is empty, we act like opening My Computer
        OpenMyComputer(directory);
    }
    else if (directory->Equals((String^) MY_NETWORK_PLACES))
    {
        OpenMyNetworkPlaces(directory);
    }
    else if (directory->Substring(0, 2) == L"\\\\" &&
             (directory->Substring(2)->IndexOf('\\') == -1 ||
             directory->Substring(2)->IndexOf('\\') == directory->Length - 3))
    {
        String^ networkName = directory->Trim(gcnew array<wchar_t>(1){'\\'});
        OpenNetworkComputer(networkName);
    }
    else
    {
        OpenLocalDirectory(directory);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::OpenMyComputer(String^ directory)
{
    if (directory == String::Empty)
    {
        cboFolders->Text = L"My Computer";
        btnFolderUp->Enabled = false;
        lstFolders->Items->Clear();
        array<System::IO::DriveInfo^>^ drives = System::IO::DriveInfo::GetDrives();
        
        for each (System::IO::DriveInfo^ drive in drives)
        {
            ListViewItem^ item = gcnew ListViewItem(drive->Name);
            switch (drive->DriveType)
            {
            case DriveType::CDRom:
                item->ImageIndex = 2;
                break;
            case DriveType::Fixed:
                item->ImageIndex = 1;
                break;
            case DriveType::Network:
                item->ImageIndex = 3;
                break;
            case DriveType::Removable:
                item->ImageIndex = 4;
                break;
            case DriveType::Unknown:
                item->ImageIndex = 5;
                break;
            default:
                item->ImageIndex = 1;
                break;
            }
            item->Tag = (Object^) drive->Name;
            lstFolders->Items->Add(item);
        }

        // Adding My Network Places manually
        String^ network = (String^) MY_NETWORK_PLACES;
        ListViewItem^ networkPlaces = gcnew ListViewItem(network);
        networkPlaces->Tag = network;
        networkPlaces->ImageIndex = 6;
        lstFolders->Items->Add(networkPlaces);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::OpenMyNetworkPlaces(String^ directory)
{
    // Retrieving computer names
    _SERVER_INFO_100 * buffer = NULL;
    _SERVER_INFO_100 * tmpBuffer;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resHandle = 0;
    int ret = NetServerEnum(
                nullptr, 
                100, 
                (LPBYTE *) &buffer, 
                -1, 
                &entriesRead, 
                &totalEntries, 
                SV_TYPE_WORKSTATION , 
                nullptr, 
                &resHandle);

    if (ret == NERR_Success)
    {
        tmpBuffer = buffer;
        btnFolderUp->Enabled = true;
        lstFolders->Items->Clear();
        for (System::UInt32 i = 0; i < totalEntries; ++i)
        {
            String^ networkName = gcnew String(tmpBuffer->sv100_name);
            ListViewItem^ item = gcnew ListViewItem(networkName);
            item->Tag = L"\\\\" + networkName;
            item->ImageIndex = 6;
            lstFolders->Items->Add(item);
            tmpBuffer++;
        }
    }
    NetApiBufferFree(static_cast<LPVOID>(buffer));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::OpenNetworkComputer(String^ computerName)
{
    // Enumerating the network share for the current computer
    PSHARE_INFO_0 buffer;
    PSHARE_INFO_0 tmpBuffer;
    NET_API_STATUS res;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD hRes = 0;
    WString LPWComputerName;
    HuntTools::MarshalString(computerName, LPWComputerName);

    res = NetShareEnum((LPWSTR) LPWComputerName.c_str(), 0, (LPBYTE *) &buffer, -1, &entriesRead, &totalEntries, &hRes);
    if (res == ERROR_SUCCESS)
    {
        lstFolders->Items->Clear();
        btnFolderUp->Enabled = true;
        tmpBuffer = buffer;
        for (System::UInt32 i = 0; i < totalEntries; ++i)
        {
            String^ networkName = gcnew String(tmpBuffer->shi0_netname);
            ListViewItem^ item = gcnew ListViewItem(networkName);
            item->ImageIndex = 0;
            item->Tag = L"\\\\" + computerName + L"\\" + networkName;
            lstFolders->Items->Add(item);
            tmpBuffer++;
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void AddFolderUI::OpenLocalDirectory(String^ directory)
{
    // If the directory exists and we have sufficient rights, we list the 
    // directories it contains
    if (Directory::Exists(directory))
    {
        try
        {
            array<String^>^ dirs = System::IO::Directory::GetDirectories(directory);
            lstFolders->Items->Clear();
            btnFolderUp->Enabled = true;

            for each (String^ dir in dirs)
            {
                ListViewItem^ item = gcnew ListViewItem(dir->Substring(dir->LastIndexOf('\\') + 1));
                item->ImageIndex = 0;
                item->Tag = dir;
                lstFolders->Items->Add(item);
            }
        }
        catch (System::UnauthorizedAccessException^)
        {
            MessageBox::Show(
            L"Sorry, access is denied to " + directory + L".", 
            L"Unable to open folder", 
            MessageBoxButtons::OK, 
            MessageBoxIcon::Warning);
        }
    } else {
        MessageBox::Show(
            L"The directory " + directory + L" does not exist!", 
            L"Unable to open folder", 
            MessageBoxButtons::OK, 
            MessageBoxIcon::Exclamation);
    }
}
