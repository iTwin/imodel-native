/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MainUI.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MainUI.cpp,v 1.5 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class MainUI
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "MainUI.h"

#include <Shellapi.h>
#undef __IConnectionPoint_FWD_DEFINED__
#include <shlwapi.h>

using namespace ImageHunter;
using namespace System; 
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Diagnostics;
using namespace System::Configuration;

/*---------------------------------------------------------------------------------**//**
* @bsifunction                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// Works for VC2010 and VC2012 only
#if (_MSC_VER >= 1600)
/* The .NET main function cannot be used here since we are using the /clr switch. */
/* We need to use the old MFC style WinMain function to be able to initialize     */
/* our unmanaged code (Image++). If we fail at doing so, the heap got corrupted   */
/* for an unknown reason so far. Note that it is not working in VS2005, it throws */
/* an error about using managed code in unmanaged function.                       */
[STAThreadAttribute]                 
int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
    {
#ifdef IPP_USING_STATIC_LIBRARIES
    #error TODO resource support.
    //HFCResourceLoader::GetInstance()->SetModuleInstance(GetModuleHandle(NULL));
#endif

    // Enabling Windows XP visual effects before any controls are created
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false); 

    // Create the main window and run it
    Application::Run(gcnew MainUI());
    return 0;
    }
#else
[STAThreadAttribute]
int main(array<System::String ^> ^args)
    {
#ifdef IPP_USING_STATIC_LIBRARIES
    #error TODO resource support.
    //HFCResourceLoader::GetInstance()->SetModuleInstance(GetModuleHandle(NULL));
#endif

    // Enabling Windows XP visual effects before any controls are created
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false); 

    // Create the main window and run it
    Application::Run(gcnew MainUI());
    return 0;
    }
#endif

#pragma region Form Events

IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::MainUI_Load(System::Object^  sender, System::EventArgs^  e)
{
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    // Creating custom ListView for the results (Double buffer enabled ListView)
    InitResultsListView();

    m_BrowsingHistory = L"";
    m_LastColumnClicked = -1;

    // Loading settings from app.config
    LoadAppSettings();
    PopulateResultsContextMenu();

    // User Interface configuration
    this->m_IsMaximized = false;
    this->exportResultsMenuItem->Enabled = false;
    this->Size = System::Drawing::Size(this->Size.Width, MIN_HEIGHT);
    cboOperator->SelectedIndex = 0;
    btnRemoveSelectedFilter->Enabled = false;
    lblHuntingStats->Text = L"";
    lblAccuracyRate->Text = L"";
    ResetStatusBar();
    HideFilterOptions();
    if (!m_IsSendToMenuDisplayed)
        {
        refreshSendToMenuItem->Enabled = false;
        }

    // Loading capabilities supported by Hunter
    Hunter^ hunter = Hunter::GetInstance();
    hunter->RegisterBuilder(gcnew ImageFormatBuilder(hunter->GetSupportedCapabilities(SupportedCapability::FileFormat)));
    hunter->RegisterBuilder(gcnew BlockTypeBuilder(hunter->GetSupportedCapabilities(SupportedCapability::BlockType)));
    hunter->RegisterBuilder(gcnew PixelTypeBuilder(hunter->GetSupportedCapabilities(SupportedCapability::PixelType)));
    hunter->RegisterBuilder(gcnew CodecBuilder(hunter->GetSupportedCapabilities(SupportedCapability::Codec)));
    hunter->RegisterBuilder(gcnew HistogramBuilder());
    hunter->RegisterBuilder(gcnew ImageSizeBuilder());
    hunter->RegisterBuilder(gcnew ScanlineOrientationBuilder(hunter->GetSupportedCapabilities(SupportedCapability::ScanlineOrientation)));
    hunter->RegisterBuilder(gcnew TransfoModelBuilder(hunter->GetSupportedCapabilities(SupportedCapability::TransfoModel)));
    hunter->RegisterBuilder(gcnew MultiPagesBuilder());
    hunter->RegisterBuilder(gcnew MultiResolutionsBuilder());
    hunter->RegisterBuilder(gcnew TagBuilder(hunter->GetSupportedCapabilities(SupportedCapability::Tag)));
    hunter->RegisterBuilder(gcnew GeocodingBuilder());

    // Adding available filters to the list
    ArrayList^ criteriaBuilders = hunter->GetCriteriaBuilders();
    for each (ICriteriaBuilder^ builder in criteriaBuilders)
        {
        cboFilterName->Items->Add(builder->GetName());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnAddFolder_Click(System::Object^  sender, System::EventArgs^  e)
{
    AddFolder();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnSearch_Click(System::Object^  sender, System::EventArgs^  e) 
{
    System::ComponentModel::ComponentResourceManager^  resources = 
            (gcnew System::ComponentModel::ComponentResourceManager(MainUI::typeid));
            
    if (!m_IsSearching)
    {
        // Looking for folders
        if (cboFolders->Text == L"")
        {
            MessageBox::Show(
                L"Please select one or many folders where your images are located.", 
                L"No folder specified", 
                MessageBoxButtons::OK,
                MessageBoxIcon::Exclamation
                );
        } 
        else if (lstFilters->Items->Count <= 0) 
        {
            // Looking for filters
            MessageBox::Show(
                L"Please select at least one filter from the filters list.", 
                L"No filters specified", 
                MessageBoxButtons::OK,
                MessageBoxIcon::Exclamation
                );
        } 
        else 
        {
            // Analyzing selected folders to remove unavailable ones
            array<String^>^ foldersList = cboFolders->Text->Split(gcnew array<wchar_t>(1){';'});
            array<String^>^ paths = gcnew array<String^>(foldersList->Length);
            array<String^>^ invalidPaths = gcnew array<String^>(foldersList->Length);
            int i = 0;
            int j = 0;
            for each (String^ path in foldersList)
            {
                if (System::IO::Directory::Exists(path) == true) 
                {
                    paths[i] = path;
                    ++i;
                } 
                else 
                {
                    invalidPaths[j] = path;
                    ++j;
                }
            }

            // If there are some missing folders, we concatenate then
            // and we display an error message before launching the search.
            if (j > 0)
            {
                String^ message = L"The following folders do not exist anymore.";
                for (int k = 0; k < j; ++k)
                {
                    message += L"\n" + invalidPaths[k];
                }
                message += L"\nClick OK to start the search.";

                if (i > 0) 
                {
                    if (MessageBox::Show(
                        message, 
                        L"Invalid folders", 
						System::Windows::Forms::MessageBoxButtons::OKCancel,
						System::Windows::Forms::MessageBoxIcon::Exclamation) == System::Windows::Forms::DialogResult::Cancel)
                    {
                        return;
                    }
                } 
                else 
                {
                    MessageBox::Show(
                        message, 
                        L"Invalid folders", 
						System::Windows::Forms::MessageBoxButtons::OK,
						System::Windows::Forms::MessageBoxIcon::Exclamation);
                    return;
                }
            }

            // Preparing the Excluded extensions Hashtable
            array<String^>^ excludedExtensions = m_ExcludedExtensions->Split(gcnew array<wchar_t>(1){';'});
            Hashtable^ excludedExtHash = gcnew Hashtable();
            for each (String^ extension in excludedExtensions)
            {
                if (!extension->Equals(String::Empty))
                {
                    excludedExtHash->Add(L"." + extension->Trim()->ToUpper(), extension);
                }
            }

            // Updating the form's elements before the search begin
            this->exportResultsMenuItem->Enabled = false;
            m_IsSearching = true;
            UIRefreshTimer->Enabled = true;
            UIRefreshTimer->Start();
            this->m_IsMaximized = true;
            lstResults->Items->Clear();
            StatusProgressToolStrip->Value = 0;
            StatusProgressToolStrip->Visible = true;
            CurrentFileToolStrip->Visible = true;
            btnClearResults->Enabled = false;
            StatusToolStrip->Text = L"Loading ammo...";
            btnSearch->Text = L"Cancel...";
            btnSearch->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"cancelToolStripMenuItem.Image")));
            this->CancelButton = btnSearch;
            lblHuntingStats->Text = L"";
            lblAccuracyRate->Text = L"";

            m_ExecutionStart = System::DateTime::Now;

            if (!cboFolders->Items->Contains(cboFolders->Text))
            {
                if (cboFolders->Items->Count > MAX_HISTORY_FOLDERS)
                {
                    cboFolders->Items->RemoveAt(0);
                }
                cboFolders->Items->Add(cboFolders->Text);
            }

            // Starting the search
            Hunter::GetInstance()->Hunt(
                paths, 
                chkRecursiveSearch->Checked, 
                excludedExtHash,
                gcnew System::ComponentModel::ProgressChangedEventHandler(this, &MainUI::UpdateStatus_Callback), 
                gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MainUI::EndOfSearch_Callback));
        }
    } 
    else 
        {
        // If there was already an ongoing search, we cancel it
        m_IsSearching = false;
        Hunter::GetInstance()->CancelHunt();
        // Gather some stats
        m_ExecutionStop = System::DateTime::Now;
        CompileSearchStats();
        // Then, we revert back the UI to its state before the search
        this->Text = L"Image Hunter";
        btnSearch->Text = (String^) START_SEARCHING;
        btnSearch->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSearch.Image")));
        this->CancelButton = nullptr;
        StatusToolStrip->Text = L"Search cancelled";
        btnClearResults->Enabled = true;
        ResetStatusBar();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::UIRefreshTimer_Tick(System::Object^  sender, System::EventArgs^  e)
{
    if ((m_IsMaximized == true && this->Size.Height >= MAX_HEIGHT)
        || (m_IsMaximized == false && this->Size.Height <= MIN_HEIGHT)) 
    {
        UIRefreshTimer->Stop();
    } 
    else 
    {
        if (m_IsMaximized == true) 
        {
            this->Size = System::Drawing::Size(this->Size.Width, this->Size.Height + REFRESH_INTERVAL);
        } 
        else 
        {
            this->Size = System::Drawing::Size(this->Size.Width, this->Size.Height - REFRESH_INTERVAL);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnClearResults_Click(System::Object^  sender, System::EventArgs^  e) 
{
    this->exportResultsMenuItem->Enabled = false;
    this->m_IsMaximized = false;
    this->UIRefreshTimer->Start();
    lstResults->Items->Clear();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::quitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
{
    SaveAppSettings();
    Application::Exit();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::cboFilterName_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
{
    if (cboFilterName->SelectedIndex != -1)
    {
        ICriteriaBuilder^ builder = Hunter::GetInstance()->GetBuilder(cboFilterName->SelectedItem->ToString());
        HideFilterOptions();
        ArrayList^ options = builder->GetOptions();

        switch (builder->GetFilterType())
        {
        case FilterType::Enum:
            ShowEnumFilterControls(options);
            ShowTextOperators();
            break;
        case FilterType::EnumWithFields:
            ShowEnumWithFieldsFiltercontrols(options);
            ShowAllOperators();
            break;
        case FilterType::KeyValue:
            ShowXYFilterControls(L"Tag", L"Value");
            if (options != nullptr)
            {
                cboXFilter->Items->Clear();
                //txtXFilter->AutoCompleteCustomSource->Clear();
                for (int i = 0; i < options->Count; ++i)
                {
                    //txtXFilter->AutoCompleteCustomSource->Add(((EnumOption^) options[i])->GetLabel());
                    cboXFilter->Items->Add(((EnumOption^) options[i])->GetLabel());
                }
                //txtXFilter->AutoCompleteMode = System::Windows::Forms::AutoCompleteMode::SuggestAppend;
                //txtXFilter->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::CustomSource;
            }
            ShowTextOperators();
            cboXFilter->Visible = true;
            txtXFilter->Visible = false;
            break;
        case FilterType::Number:
            ShowTextFilterControls(L"Value");
            ShowAllOperators();
            break;
        case FilterType::Text:
            ShowTextFilterControls(L"Value");
            ShowTextOperators();
            break;
        case FilterType::XY:
            ShowXYFilterControls("X", "Y");
            ShowAllOperators();
            break;
        case FilterType::Size:
            ShowXYFilterControls("Width", "Height");
            ShowAllOperators();
            break;
        case FilterType::Boolean:
            ShowTextOperators();
            break;
        default:
            break;
        }
    } else {
        HideFilterOptions();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnAddFilter_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if (cboFilterName->SelectedIndex != -1)
    {
        ICriteriaBuilder^ builder = Hunter::GetInstance()->GetBuilder(
            cboFilterName->SelectedItem->ToString());
        ICriteria^ criteria;
        CriteriaOperator criteriaOp = CriteriaOperator::Equal;

        if (cboOperator->SelectedItem->ToString() == L"==")
            criteriaOp = CriteriaOperator::Equal;

        if (cboOperator->SelectedItem->ToString() == L"!=")
            criteriaOp = CriteriaOperator::NotEqual;

        if (cboOperator->SelectedItem->ToString() == L"<")
            criteriaOp = CriteriaOperator::LessThan;

        if (cboOperator->SelectedItem->ToString() == L"<=")
            criteriaOp = CriteriaOperator::LessThanOrEqual;

        if (cboOperator->SelectedItem->ToString() == L">")
            criteriaOp = CriteriaOperator::GreaterThan;

        if (cboOperator->SelectedItem->ToString() == L">=")
            criteriaOp = CriteriaOperator::GreaterThanOrEqual;

        try
        {
            switch (builder->GetFilterType())
            {
            case FilterType::Enum:
                if (cboEnumFilter->SelectedIndex == -1)
                {
                    MessageBox::Show(
                        L"You must choose a value from the drop down list for this filter.", 
                        L"Empty filter", 
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Exclamation);
                    cboEnumFilter->Focus();
                } 
                else 
                {
                    criteria = builder->BuildCriteria(
                        criteriaOp,
                        gcnew array<Object^>(1) {cboEnumFilter->SelectedItem});
                }
                break;
            case FilterType::EnumWithFields:
                if (cboEnumWithFieldsFilter->SelectedIndex == -1)
                {
                    MessageBox::Show(
                        L"You must choose a value from the drop down list for this filter.", 
                        L"Empty filter", 
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Exclamation);
                    cboEnumWithFieldsFilter->Focus();
                } 
                else 
                {
                    criteria = builder->BuildCriteria(
                        criteriaOp,
                        gcnew array<Object^>(3) {cboEnumWithFieldsFilter->SelectedItem, txtEnumX->Text, txtEnumY->Text});
                }
                break;
            case FilterType::Text:
            case FilterType::Number:
                if (txtTextFilter->Text == String::Empty)
                {
                    MessageBox::Show(
                        L"You must enter a value this filter.",
                        L"Empty filter", 
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Exclamation);
                    txtTextFilter->Focus();
                } 
                else 
                {
                    criteria = builder->BuildCriteria(
                        criteriaOp,
                        gcnew array<String^>(1) {txtTextFilter->Text});
                }
                break;
            case FilterType::KeyValue:
                if (cboXFilter->SelectedIndex == -1)
                {
                    MessageBox::Show(
                        L"You must select a value from the list for this filter.",
                        L"Empty filter", 
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Exclamation);
                    cboXFilter->Focus();
                }
                else
                {
                    criteria = builder->BuildCriteria(
                        criteriaOp, 
                        gcnew array<String^>(2) {cboXFilter->SelectedItem->ToString(), txtYFilter->Text});
                }
                break;
            case FilterType::XY:
            case FilterType::Size:
                if (txtXFilter->Text == String::Empty && txtYFilter->Text == String::Empty)
                {
                    MessageBox::Show(
                        L"You must enter at least one value for this filter.",
                        L"Empty filter", 
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Exclamation);
                    txtXFilter->Focus();
                } 
                else 
                {
                    criteria = builder->BuildCriteria(
                        criteriaOp,
                        gcnew array<String^>(2) {txtXFilter->Text, txtYFilter->Text});
                }
                break;
            case FilterType::Boolean:
                criteria = builder->BuildCriteria(criteriaOp, nullptr);
                break;
            default:
                break;
            }
        } catch (ApplicationException^ ex) 
        {
            MessageBox::Show(ex->Message, L"Invalid entered value", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
        } catch (System::FormatException^ ex) 
        {
            MessageBox::Show(ex->Message, L"Invalid entered value", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
        }

        if (criteria != nullptr)
        {
            Hunter::GetInstance()->AddCriteria(builder->GetCapabilityType(), criteria);
            ListViewItem^ lstItem = gcnew ListViewItem(builder->GetName());
            lstItem->Tag = criteria;
            lstItem->SubItems->Add(criteria->ToString());
            lstFilters->Items->Add(lstItem);
            StatusToolStrip->Text = (String^) READY_TO_HUNT;

            switch (builder->GetFilterType())
            {
            case FilterType::Boolean:
                cboFilterName->SelectedIndex = -1;
                break;
            case FilterType::Enum:
                if (cboEnumFilter->SelectedIndex < cboEnumFilter->Items->Count - 1)
                {
                    cboEnumFilter->SelectedIndex += 1;
                } 
                else 
                {
                    cboEnumFilter->SelectedIndex = 0;
                }
                break;
            case FilterType::EnumWithFields:
                cboEnumWithFieldsFilter->SelectedIndex = -1;
                txtEnumX->Text = L"";
                txtEnumY->Text = L"";
                break;
            case FilterType::Number:
            case FilterType::Text:
                txtTextFilter->Text = L"";
                break;
            case FilterType::KeyValue:
            case FilterType::Size:
            case FilterType::XY:
                txtXFilter->Text = L"";
                txtYFilter->Text = L"";
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnRemoveSelectedFilter_Click(System::Object^  sender, System::EventArgs^  e) 
{
    if (lstFilters->SelectedItems != nullptr)
    {                
        // We keep in memory the selected index if only one item is selected
        int index = -1;
        if (lstFilters->SelectedItems->Count == 1)
        {
            index = lstFilters->SelectedIndices[0];
        }

        // Removing the selected criteria from the list and the Hunter
        for each (ListViewItem^ filter in lstFilters->SelectedItems)
        {
            ICriteriaBuilder^ builder = Hunter::GetInstance()->GetBuilder(filter->Text);
            Hunter::GetInstance()->RemoveCriteria(builder->GetCapabilityType(), static_cast<ICriteria^>(filter->Tag));
            lstFilters->Items->Remove(filter);
        }

        // if there was only one selection made, we select an item in the list
        if (index != -1)
        {
            // Selecting the same index as the one removed or the last item from the list
            lstFilters->SelectedItems->Clear();
            if (index < lstFilters->Items->Count)
            {
                lstFilters->Items[index]->Selected = true;
            }
            else if (lstFilters->Items->Count > 0)
            {
                lstFilters->Items[lstFilters->Items->Count - 1]->Selected = true;
            }
            lstFilters->Select();
        }

        // Updating the UI if no filter are selected
        if (lstFilters->Items->Count == 0)
        {
            StatusToolStrip->Text = (String^) WAITING_FOR_FILTERS;
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::btnRemoveAllFilters_Click(System::Object^  sender, System::EventArgs^  e)
{
    lstFilters->Items->Clear();
    StatusToolStrip->Text = (String^) WAITING_FOR_FILTERS;
    Hunter::GetInstance()->RemoveAllCriterias();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::addFolderMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
{
    AddFolder();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ExternalTools_Click(System::Object^ sender, System::EventArgs^ e)
{
    ToolStripMenuItem^ item = (ToolStripMenuItem^) sender;
    ExternalTool^ tool = (ExternalTool^)item->Tag;

    try
    {
        for each (ListViewItem^ item in lstResults->SelectedItems)
        {
            ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
            String^ exe = tool->GetExecutable();
            String^ arguments = tool->GetArguments();
            Process^ externalProcess = gcnew Process();
            externalProcess->StartInfo->UseShellExecute = false;
            externalProcess->StartInfo->FileName = exe;
            if (arguments->Contains(L"%s"))
            {
                arguments = arguments->Replace(L"%s", fileItem->GetFullName());
            } 
            else 
            {
                arguments += item->Text;
            }
            externalProcess->StartInfo->Arguments = arguments;
            externalProcess->Start();
        }
    } 
    catch (...) 
    {
        MessageBox::Show(
            L"The executable " + tool->GetExecutable() + L" cannot be found.", 
            L"Cannot launch external tool", 
            MessageBoxButtons::OK, 
            MessageBoxIcon::Exclamation);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::MainUI_Closing(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e)
{
    SaveAppSettings();
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::UpdateStatus_Callback(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e)
{
    Hunter^ hunter = Hunter::GetInstance();
    StatusToolStrip->Text = L"Hunting...";

    // Updating the progress bar
    String^ progress = e->ProgressPercentage + L"% completed (" + 
        hunter->GetCurrentFileNo() + L"/" + hunter->GetTotalNumberOfFiles();
    StatusToolStrip->ToolTipText = progress;
    StatusProgressToolStrip->Value = e->ProgressPercentage;
    StatusProgressToolStrip->ToolTipText = progress;

    // Updating title bar
    this->Text = L"Image Hunter - " + e->ProgressPercentage + L"%";

    // Updating the current file displayed in the status bar
    WCHAR shortName[MAX_LENGTH_IN_STATUS_BAS];
    WString filename;
    HuntTools::MarshalString((String^) hunter->GetCurrentScannedImage(), filename);
    PathCompactPathEx(shortName, filename.c_str(), MainUI::MAX_LENGTH_IN_STATUS_BAS, 0);
    String^ file = gcnew String(shortName);

    CurrentFileToolStrip->Text = file;

    // Populating the results list
    RefreshResults();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::EndOfSearch_Callback(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e)
{
    m_ExecutionStop = System::DateTime::Now;
    CompileSearchStats();

    // Reverting back the UI to the state it was before the search occured
    this->Text = L"Image Hunter";
    System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainUI::typeid));
    btnClearResults->Enabled = true;
    m_IsSearching = false;
    btnSearch->Text = (String^) START_SEARCHING;
    btnSearch->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSearch.Image")));
    this->CancelButton = nullptr;
    RefreshResults();
    ResetStatusBar();

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::externalToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Opening the External Tools UI
    ExternalToolsUI^ extTools = gcnew ExternalToolsUI();
    if (extTools->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        // Saving changes made
        ExternalTools::GetInstance()->DropSavedState();
        PopulateResultsContextMenu();
        SaveAppSettings();
    } 
    else 
    {
        // Reverting back to the previous state
        ExternalTools::GetInstance()->RestoreFromSavedState();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::OpenContainingFolderItem_Click(System::Object^ sender, System::EventArgs^ e)
{
    for each (ListViewItem^ item in lstResults->SelectedItems)
    {
        ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
        Process::Start(L"explorer.exe", fileItem->GetPath());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::CopySelectedItem_Click(System::Object^ sender, System::EventArgs^ e)
{
    if (lstResults->SelectedItems->Count > 0)
    {
        ImagesFolderBrowser->ShowNewFolderButton = true;
        if (ImagesFolderBrowser->ShowDialog() == System::Windows::Forms::DialogResult::OK)
        {
            for each (ListViewItem^ item in lstResults->SelectedItems)
            {
                ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
                if (IO::File::Exists(fileItem->GetFullName()))
                {
                    IO::File::Copy(
                        fileItem->GetFullName(), 
                        ImagesFolderBrowser->SelectedPath + L"\\" + System::IO::Path::GetFileName(fileItem->GetFullName()));
                } 
                else 
                {
                    MessageBox::Show(
                        L"The file " + fileItem->GetFullName() + L" does not exist.", 
                        L"Cannot copy the selected image",
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Error);
                }
            }
        }
        ImagesFolderBrowser->ShowNewFolderButton = false;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::CopyAllItems_Click(System::Object^ sender, System::EventArgs^ e)
{
    ImagesFolderBrowser->ShowNewFolderButton = true;
    if (ImagesFolderBrowser->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        for each (ListViewItem^ item in lstResults->Items)
        {
            ImageFile^ imageItem = static_cast<ImageFile^>(item->Tag);
            if (IO::File::Exists(imageItem->GetFullName()))
            {
                IO::File::Copy(
                    imageItem->GetFullName(), 
                    ImagesFolderBrowser->SelectedPath + L"\\" + System::IO::Path::GetFileName(imageItem->GetFullName()));
            } 
            else 
            {
                MessageBox::Show(
                    L"The file " + imageItem->GetFullName() + L" does not exist.", 
                    L"Cannot copy the selected image",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
    }
    ImagesFolderBrowser->ShowNewFolderButton = false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::CopyImagePath_Click(System::Object^ sender, System::EventArgs^ e)
{
    // Copy to path of selected image to Windows Clipboard
    if (lstResults->SelectedItems->Count == 1)
    {
        ImageFile^ fileItem = static_cast<ImageFile^>(lstResults->SelectedItems[0]->Tag);
        Clipboard::SetText(fileItem->GetFullName());
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::CopyEntireList_Click(System::Object^ sender, System::EventArgs^ e)
{
    String^ entireList = L"";
    for each (ListViewItem^ item in lstResults->Items)
    {
        ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
        entireList += fileItem->GetFullName() + L"\r\n";
    }
    Clipboard::SetText(entireList);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::SendToMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
{
    // Launching the selected executable
    ToolStripMenuItem^ item = (ToolStripMenuItem^) sender;
    String^ exe = static_cast<String^>(item->Tag);

    try
    {
        for each (ListViewItem^ item in lstResults->SelectedItems)
        {
            ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
            Process::Start(exe, fileItem->GetFullName());
        }
    }
    catch (...)
    {
        MessageBox::Show(
            L"Unable to launch " + exe + L".",
            L"Unable to launch executable",
            MessageBoxButtons::OK,
            MessageBoxIcon::Warning);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::lstFilters_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
{
    if (lstFilters->SelectedIndices->Count == 0)
    {
        btnRemoveSelectedFilter->Enabled = false;
    } 
    else 
    {
        btnRemoveSelectedFilter->Enabled = true;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::lstFilters_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
    if (e->KeyCode == Keys::Delete || e->KeyCode == Keys::Back)
    {
        btnRemoveSelectedFilter_Click(nullptr, System::Windows::Forms::KeyEventArgs::Empty);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::FilterControls_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
{
    if (e->KeyCode == Keys::Enter)
    {
        //btnAddFilter_Click(nullptr, EventArgs::Empty);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::lstResults_DoubleClick(System::Object^  sender, System::EventArgs^  e)
{
    // Launching default Explorer action for the selected files
    if (lstResults->SelectedItems->Count > 0)
    {
        for each (ListViewItem^ item in lstResults->SelectedItems)
        {
            ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
            Process::Start(L"explorer.exe", fileItem->GetFullName());
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::lstResults_ItemDrag(System::Object^ sender, System::Windows::Forms::ItemDragEventArgs^ e)
{
    // Activating DragDrop if at least one element has been selected
    if (lstResults->SelectedItems->Count > 0)
    {
        DataObject^ data = gcnew DataObject();
        Collections::Specialized::StringCollection^ files = gcnew Collections::Specialized::StringCollection();
        for each (ListViewItem^ item in lstResults->SelectedItems)
        {
            ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
            files->Add(fileItem->GetFullName());
        }
        data->SetFileDropList(files);
        lstResults->DoDragDrop(data, DragDropEffects::Copy);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::optionsToolMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
    // Loading Options UI
    OptionsUI^ options = gcnew OptionsUI();
    options->SetIsSendToMenuDisplayed(m_IsSendToMenuDisplayed);
    options->SetExcludedExtensions(m_ExcludedExtensions);
    if (options->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        // Updating settings
        m_ExcludedExtensions = options->GetExcludedExtensions();
        m_IsSendToMenuDisplayed = options->IsSendToMenuDisplayed();

        // Refreshing context menu
        PopulateResultsContextMenu();

        // Enabling the Refresh SendTo Menu if the SendTo context menu is enabled
        if (m_IsSendToMenuDisplayed)
        {
            refreshSendToMenuItem->Enabled = true;
        }
        else
        {
            refreshSendToMenuItem->Enabled = false;
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::refreshSendToMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
    PopulateResultsContextMenu();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::exportResultsMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
    if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        IO::StreamWriter^ writer = gcnew IO::StreamWriter(saveFileDialog->FileName);

        for each (ListViewItem^ item in lstResults->Items)
        {
            ImageFile^ fileItem = static_cast<ImageFile^>(item->Tag);
            writer->WriteLine(fileItem->GetFullName());
        }

        writer->Close();
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::lstResults_ColumnClick(System::Object^ sender, System::Windows::Forms::ColumnClickEventArgs^ e)
{
    if (e->Column != m_LastColumnClicked)
    {
        m_LastColumnClicked = e->Column;
        lstResults->Sorting = SortOrder::Ascending;
    }
    else
    {
        if (lstResults->Sorting == SortOrder::Ascending)
            lstResults->Sorting = SortOrder::Descending;
        else
            lstResults->Sorting = SortOrder::Ascending;
    }

    if (e->Column == 0 || e->Column == 2)
    {
        this->lstResults->ListViewItemSorter = gcnew ResultComparer(e->Column, lstResults->Sorting);
    }
    else
    {
        this->lstResults->ListViewItemSorter = gcnew FileSizeComparer(e->Column, lstResults->Sorting);
    }
    lstResults->Sort();
}

#pragma endregion

#pragma region SubRoutines
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::InitResultsListView()
{
    lstResults = gcnew ListViewNoFlicker();
    this->lstResults->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->colImageName, this->colImageSize, 
        this->colImagePath});
    this->lstResults->ContextMenuStrip = this->ResultsContextMenu;
    this->lstResults->FullRowSelect = true;
    this->lstResults->Location = System::Drawing::Point(10, 20);
    this->lstResults->Name = L"lstResults";
    this->lstResults->Size = System::Drawing::Size(631, 217);
    this->lstResults->TabIndex = 0;
    this->lstResults->UseCompatibleStateImageBehavior = false;
    this->lstResults->View = System::Windows::Forms::View::Details;
    this->lstResults->ItemDrag += gcnew System::Windows::Forms::ItemDragEventHandler(this, &MainUI::lstResults_ItemDrag);
    this->lstResults->DoubleClick += gcnew System::EventHandler(this, &MainUI::lstResults_DoubleClick);
    this->lstResults->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &MainUI::lstResults_ColumnClick);
    this->grpResults->Controls->Add(this->lstResults);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::LoadExternalTools()
{
    try 
    {
        ExternalTools^ externalTools = ExternalTools::GetInstance();
        ImageHunter::ExternalToolsSection^ tools = (ImageHunter::ExternalToolsSection^) System::Configuration::ConfigurationManager::GetSection((String^) EXTERNAL_TOOLS);
        if (tools != nullptr)
        {
            for each (ImageHunter::ExternalToolElement^ tool in tools->Tools)
            {
                externalTools->AddTool(tool->ID, tool->Executable, tool->Name, tool->Arguments);
            }
        }
    } 
    catch (ConfigurationErrorsException^ e) 
    {
        MessageBox::Show(e->Message);
    }        
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayList^ MainUI::LoadSendToMenu()
{
    ArrayList^ SendToItems = gcnew ArrayList();
    Hashtable^ blackList = gcnew Hashtable();

    // Getting the path to SendTo menu and retrieving filenames
    String^ pathToSendTo = Environment::GetFolderPath(Environment::SpecialFolder::SendTo);
    array<String^>^ SendToFiles = IO::Directory::GetFiles(pathToSendTo);

    // Reading the desktop.ini to get Windows-only shortcut
    IO::StreamReader^ fstream = gcnew System::IO::StreamReader(pathToSendTo + L"\\" + (String^) DESKTOP_INI);
    String^ line;        
    while ((line = fstream->ReadLine()) != nullptr)
    {
        // Parsing line for extension and '=' character
        int eqIndex = line->IndexOf('=');
        int dotIndex = line->IndexOf('.');
        String^ ext = L"";
        if (eqIndex != -1 && dotIndex != -1)
        {
            ext = line->Substring(dotIndex + 1, eqIndex - dotIndex - 1);
        }

        // if the line does not have an '=' char and is not a .lnk file, 
        // we blacklist it since we will not be able to use it directly.
        if (eqIndex > 0 && !ext->Equals(L"lnk"))
        {
            String^ name = line->Substring(0, eqIndex);
            String^ exe = line->Substring(line->IndexOf('@') + 1);
            blackList->Add(name, exe);
        }
    }

    // Populating the SendTo items array
    for each (String^ shortcut in SendToFiles)
    {
        int start = shortcut->LastIndexOf('\\');
        int end = shortcut->LastIndexOf('.');
        String^ shortcutName = shortcut->Substring(start + 1, end - start - 1);

        if (!(shortcut->Contains((String^) DESKTOP_INI) || blackList->ContainsKey(shortcut->Substring(start + 1))))
        {
            // Creating the Menu item
            ToolStripMenuItem^ sendToShortcut = gcnew ToolStripMenuItem(shortcutName);                
            sendToShortcut->Tag = shortcut;
            sendToShortcut->Click += gcnew System::EventHandler(this, &MainUI::SendToMenuItem_Click);

            SendToItems->Add(sendToShortcut);
        }
    }

    return SendToItems;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::PopulateResultsContextMenu()
{
    System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainUI::typeid));
    ResultsContextMenu->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Center;
    ResultsContextMenu->Items->Clear();

    // Building standard options for the context menu
    ToolStripMenuItem^ OpenFileItem = gcnew ToolStripMenuItem(L"Open file(s)");
    ToolStripMenuItem^ OpenContainingFolderItem = gcnew ToolStripMenuItem(L"Open containing folder");
    ToolStripMenuItem^ CopySelectedItem = gcnew ToolStripMenuItem(L"Copy selected image to...");
    ToolStripMenuItem^ CopyAllItem = gcnew ToolStripMenuItem(L"Copy all images...");
    ToolStripMenuItem^ CopyImagePath = gcnew ToolStripMenuItem(L"Copy selected image path");
    ToolStripMenuItem^ SendTo = gcnew ToolStripMenuItem(L"Send To...");
    ToolStripMenuItem^ CopyEntireList = gcnew ToolStripMenuItem(L"Copy entire list to clipboard");
    // Separators
    ToolStripSeparator^ AfterOpenItems = gcnew ToolStripSeparator();
    ToolStripSeparator^ AfterExternalTools = gcnew ToolStripSeparator();
    ToolStripSeparator^ AfterSendTo = gcnew ToolStripSeparator();

    // Icons
    OpenFileItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"openToolStripMenuItem.Image")));
    CopySelectedItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyToolStripMenuItem.Image")));
    CopyAllItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyAllToolStripMenuItem.Image")));

    // Events
    OpenFileItem->Click += gcnew EventHandler(this, &MainUI::lstResults_DoubleClick);
    OpenContainingFolderItem->Click += gcnew EventHandler(this, &MainUI::OpenContainingFolderItem_Click);
    CopySelectedItem->Click += gcnew EventHandler(this, &MainUI::CopySelectedItem_Click);
    CopyAllItem->Click += gcnew EventHandler(this, &MainUI::CopyAllItems_Click);
    CopyImagePath->Click += gcnew EventHandler(this, &MainUI::CopyImagePath_Click);
    CopyEntireList->Click += gcnew EventHandler(this, &MainUI::CopyEntireList_Click);

    // Adding default options to the menu
    ResultsContextMenu->Items->Add(OpenFileItem);
    ResultsContextMenu->Items->Add(OpenContainingFolderItem);
    ResultsContextMenu->Items->Add(AfterOpenItems);

    // Adding external tools from the configuration file if any
    SortedList^ tools = ExternalTools::GetInstance()->GetExternalTools();
    if (tools->Count > 0)
    {
        for (int i = 0; i < tools->Count; ++i)
        {
            ExternalTool^ externalTool = (ExternalTool^)tools[i];
            WString exe;
            HuntTools::MarshalString(externalTool->GetExecutable(), exe);
            HICON pIcon = ExtractIcon(NULL, exe.c_str(), 0);
            Drawing::Icon^ icon = nullptr;
            if (pIcon != NULL)
            {
                icon = Drawing::Icon::FromHandle((IntPtr) pIcon);
            }

            ToolStripMenuItem^ item = gcnew ToolStripMenuItem(externalTool->GetName());
            item->Tag = externalTool;
            if (icon != nullptr)
            {
                item->Image = icon->ToBitmap();
            }
            item->Click += gcnew EventHandler(this, &MainUI::ExternalTools_Click);
            ResultsContextMenu->Items->Add(item);
        }
        ResultsContextMenu->Items->Add(AfterExternalTools);
    }

    // Adding SendTo Menu if activated
    if (m_IsSendToMenuDisplayed)
    {
        ArrayList^ SendToItems = LoadSendToMenu();
        for each (ToolStripMenuItem^ sendToItem in SendToItems)
        {
            SendTo->DropDownItems->Add(sendToItem);
        }

        ResultsContextMenu->Items->Add(SendTo);
        ResultsContextMenu->Items->Add(AfterSendTo);
    }

    // Adding Copy Items
    ResultsContextMenu->Items->Add(CopySelectedItem);
    ResultsContextMenu->Items->Add(CopyAllItem);
    ResultsContextMenu->Items->Add(CopyImagePath);
    ResultsContextMenu->Items->Add(CopyEntireList);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::AddFolder()
{
    AddFolderUI^ addFolder = gcnew AddFolderUI();

    // Loading search folders
    ArrayList^ actualFolders = gcnew ArrayList();
    if (cboFolders->Text != L"")
    {
        actualFolders->AddRange(cboFolders->Text->Split(gcnew array<wchar_t>(1){';'}));
        addFolder->LoadSearchSources(actualFolders);
    }

    // Loading Browsing History
    ArrayList^ browsingHistory = gcnew ArrayList();
    if (m_BrowsingHistory != L"")
    {
        browsingHistory->AddRange(m_BrowsingHistory->Split(gcnew array<wchar_t>(1){';'}));
        addFolder->LoadBrowsingHistory(browsingHistory);
    }

    // Launching Add Folder UI
    if (addFolder->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        // Updating search folders 
        String^ searchFolders = L"";
        for each (String^ folder in addFolder->GetSearchSources())
        {
            searchFolders += folder + L";";
        }
        searchFolders = searchFolders->TrimEnd(gcnew array<wchar_t>(1) {';'});
        cboFolders->Text = searchFolders;
    }

    // Saving browsing history
    m_BrowsingHistory = L"";
    for each (String^ folder in addFolder->GetBrowsingHistory())
    {
        m_BrowsingHistory += folder + L";";
    }
    m_BrowsingHistory = m_BrowsingHistory->TrimEnd(gcnew array<wchar_t>(1) {';'});
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ResetStatusBar()
{
    if (lstFilters->Items->Count > 0)
    {
        StatusToolStrip->Text = (String^) READY_TO_HUNT;
    }
    else
    {
        StatusToolStrip->Text = (String^) WAITING_FOR_FILTERS;
    }        
    StatusProgressToolStrip->Visible = false;
    CurrentFileToolStrip->Text = L"";
    CurrentFileToolStrip->Visible = false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::RefreshResults()
{
    ArrayList^ results = Hunter::GetInstance()->GetValidImages();

    // The new results are added at the end of the list
    if (lstResults->Items->Count != results->Count)
    {
        for (int i = lstResults->Items->Count; i < results->Count; ++i)
        {
            ImageFile^ file = (ImageFile^)results[i];
            // Formatting the file size
            System::UInt64 size = file->GetFilesize();
            String^ sizeFormat;
            double humanSize;
            if (size < 1024)
            {
                humanSize = static_cast<double>(size);
                sizeFormat = L" Bytes";
            }
            else if (size < 1024*1024)
            {
                humanSize = static_cast<double>(size) / 1024;
                sizeFormat = L" KB";
            }
            else if (size < 1024*1024*1024)
            {
                humanSize = static_cast<double>(size) / (1024*1024);
                sizeFormat = L" MB";
            }
            else 
            {
                humanSize = static_cast<double>(size) / (1024*1024*1024);
                sizeFormat = L" GB";
            }

            // Creating and adding the file element to the list
            ListViewItem^ item = gcnew ListViewItem(file->GetShortName());
            item->SubItems->Add(String::Format(L"{0:0.#}", humanSize) + sizeFormat);
            item->SubItems->Add(file->GetPath());
            item->Tag = file;
            lstResults->Items->Add(item);
        }
    }

    if (lstResults->Items->Count > 0)
    {
        this->exportResultsMenuItem->Enabled = true;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowTextOperators()
{
    cboOperator->Items->Clear();
    cboOperator->Items->Add(L"==");
    cboOperator->Items->Add(L"!=");
    cboOperator->SelectedIndex = 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowAllOperators()
{
    ShowTextOperators();
    cboOperator->Items->Add(L"<");
    cboOperator->Items->Add(L"<=");
    cboOperator->Items->Add(L">");
    cboOperator->Items->Add(L">=");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowTextFilterControls(String^ label)
{
    lblTextFilter->Text = label;
    panelTextFilter->Visible = true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowEnumWithFieldsFiltercontrols(ArrayList^ options)
{
    cboEnumWithFieldsFilter->Items->Clear();
    if (options != nullptr)
    {
        for each (Object^ option in options)
        {
            cboEnumWithFieldsFilter->Items->Add(option);
        }
    }
    panelEnumWithFieldsFilter->Visible = true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowEnumFilterControls(ArrayList^ options)
{
    lblEnumFilter->Text = L"Choose";
    cboEnumFilter->Items->Clear();
    if (options != nullptr)
    {
        for each (Object^ option in options)
        {
            cboEnumFilter->Items->Add(option);
        }
    }
    panelEnumFilter->Visible = true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::ShowXYFilterControls(String^ XLabel, String^ YLabel)
{
    lblXFilter->Text = XLabel;
    lblYFilter->Text = YLabel;
    txtXFilter->AutoCompleteMode = System::Windows::Forms::AutoCompleteMode::None;
    txtXFilter->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::None;
    txtXFilter->Visible = true;
    panelXYFilter->Visible = true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::HideFilterOptions()
{
    txtXFilter->Text = L"";
    txtYFilter->Text = L"";
    cboXFilter->Visible = false;
    lblTextFilter->Text = L"";
    txtEnumX->Text = L"";
    txtEnumY->Text = L"";
    panelEnumFilter->Visible = false;
    panelEnumWithFieldsFilter->Visible = false;
    panelTextFilter->Visible = false;
    panelXYFilter->Visible = false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::LoadAppSettings()
{
    // Do not block hunter because raster file assert for data integrity.
    putenv("MS_IGNORE_DATA_ASSERTS=1");

    // Recursive Search Checkbox Setting
    String^ recursiveSearch = System::Configuration::ConfigurationSettings::AppSettings[(String^) RECURSIVE_SEARCH];
    chkRecursiveSearch->Checked = true;
    if (recursiveSearch != nullptr)
    {
        if (recursiveSearch->ToUpper() == (String^) CONFIG_TRUE) 
        {
            chkRecursiveSearch->Checked = true;
        } 
        else if (recursiveSearch->ToUpper() == (String^) CONFIG_FALSE) 
        {
                chkRecursiveSearch->Checked = false;
        }
    }

    // Browsing History Setting
    String^ browsingHistory = System::Configuration::ConfigurationSettings::AppSettings[(String^) BROWSING_HISTORY];
    if (browsingHistory != nullptr)
    {
        m_BrowsingHistory = browsingHistory;
    }

    // Last Used Folders Setting
    String^ folders = System::Configuration::ConfigurationSettings::AppSettings[(String^) LAST_USED_FOLDERS];
    if (folders != nullptr)
    {
        array<String^>^ splitFolders = folders->Split(gcnew array<wchar_t>(1) {'|'});
        for each (String^ folder in splitFolders)
        {
            if (String::Compare(folder, String::Empty) != 0)
                cboFolders->Items->Add(folder);
        }
        if (cboFolders->Items->Count > 0)
            cboFolders->Text = (String^)cboFolders->Items[cboFolders->Items->Count - 1];
    }

    // SendTo Menu Setting
    String^ sendToMenuDisplayed = System::Configuration::ConfigurationSettings::AppSettings[(String^) DISPLAY_SENDTO_MENU];
    if (sendToMenuDisplayed != nullptr)
    {
        if (sendToMenuDisplayed->ToUpper() == (String^) CONFIG_TRUE) 
        {
            m_IsSendToMenuDisplayed = true;
        } 
        else if (sendToMenuDisplayed->ToUpper() == (String^) CONFIG_FALSE) 
        {
            m_IsSendToMenuDisplayed = false;
        } 
        else
        {
            m_IsSendToMenuDisplayed = true;
        }
    }
    else
    {
        m_IsSendToMenuDisplayed = true;
    }

    // Excluded Extensions Setting
    String^ excludedExtensions = System::Configuration::ConfigurationSettings::AppSettings[(String^) EXCLUDED_EXTENSIONS];
    if (excludedExtensions != nullptr)
    {
        m_ExcludedExtensions = excludedExtensions;
    }
    else
    {
        m_ExcludedExtensions = (String^) DEFAULT_EXCLUDED_EXTENSION;
    }

    // External Tools Section
    LoadExternalTools();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::SaveAppSettings()
{
    System::Configuration::Configuration^ config = 
        System::Configuration::ConfigurationManager::OpenExeConfiguration(System::Configuration::ConfigurationUserLevel::None);

    // Browsing History
    config->AppSettings->Settings->Remove((String^) BROWSING_HISTORY);
    config->AppSettings->Settings->Add((String^) BROWSING_HISTORY, m_BrowsingHistory);

    // Recursive Search
    config->AppSettings->Settings->Remove((String^) RECURSIVE_SEARCH);
    config->AppSettings->Settings->Add((String^) RECURSIVE_SEARCH, chkRecursiveSearch->Checked.ToString());

    // Last Used Folders
    String^ folders = L"";
    if (!cboFolders->Items->Contains(cboFolders->Text))
        folders += cboFolders->Text + L"|";
        
    for each (String^ folder in cboFolders->Items)
    {
        folders += folder + L"|";
    }
    config->AppSettings->Settings->Remove((String^) LAST_USED_FOLDERS);
    config->AppSettings->Settings->Add((String^) LAST_USED_FOLDERS, folders);

    // Display SendTo Menu
    config->AppSettings->Settings->Remove((String^) DISPLAY_SENDTO_MENU);
    config->AppSettings->Settings->Add((String^) DISPLAY_SENDTO_MENU, m_IsSendToMenuDisplayed.ToString());

    // Excluded Extensions
    config->AppSettings->Settings->Remove((String^) EXCLUDED_EXTENSIONS);
    config->AppSettings->Settings->Add((String^) EXCLUDED_EXTENSIONS, m_ExcludedExtensions);

    // External Tools
    SortedList^ tools = ExternalTools::GetInstance()->GetExternalTools();
    ImageHunter::ExternalToolsSection^ toolsSection = (ImageHunter::ExternalToolsSection^) config->GetSection((String^) EXTERNAL_TOOLS);
    if (toolsSection == nullptr)
    {
        toolsSection = gcnew ImageHunter::ExternalToolsSection();
        config->Sections->Add((String^) EXTERNAL_TOOLS, toolsSection);
    }
    toolsSection->Tools->Clear();

    for (int i = 0; i < tools->Count; ++i)
    {
        ExternalTool^ tool = (ExternalTool^)tools[i];
        ImageHunter::ExternalToolElement^ element = gcnew ImageHunter::ExternalToolElement();
        element->ID = tool->GetID();
        element->Name = tool->GetName();
        element->Executable = tool->GetExecutable();
        element->Arguments = tool->GetArguments();
        toolsSection->Tools->Add(element);
    }

    // Saving...
    config->Save(System::Configuration::ConfigurationSaveMode::Minimal);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::Void MainUI::CompileSearchStats()
{
    // Calculating number of files found over time
    TimeSpan diffTime = m_ExecutionStop.Subtract(m_ExecutionStart);
    System::DateTime ElapsedTime = System::DateTime(diffTime.Ticks);
    int NumberOfImagesFound = lstResults->Items->Count;
    int TotalNumberOfFiles = Hunter::GetInstance()->GetCurrentFileNo();
    double AccuracyRate  = 0;
    double AverageKills  = 0;

    if (TotalNumberOfFiles > 0)
    {
        AccuracyRate = static_cast<double>(NumberOfImagesFound) / static_cast<double>(TotalNumberOfFiles);
    }
    if (diffTime.TotalSeconds > 0)
    {
        AverageKills = NumberOfImagesFound / diffTime.TotalSeconds;
    }

    // Displaying stats
    lblHuntingStats->Text = L"Hunting Stats: " + NumberOfImagesFound.ToString() + L" kills in " + 
                            ElapsedTime.ToString("HH:mm:ss") + L" (" + String::Format(L"{0:0.####}", AverageKills) + L" kills/sec)";
    lblAccuracyRate->Text = L"Accuracy Rate: " + AccuracyRate.ToString("P") + L" (" + NumberOfImagesFound.ToString() + 
                            L"/" + TotalNumberOfFiles.ToString() + L")";
}
#pragma endregion