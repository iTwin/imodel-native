/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/MainUI.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/MainUI.h,v 1.5 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Class : MainUI
//-----------------------------------------------------------------------------
// Represents the Main User Interface of the application
//-----------------------------------------------------------------------------

#pragma once

#include "Hunter.h"
#include "HuntTools.h"
#include "Criterias.h"
#include "ExternalTools.h"
#include "ExternalToolsUI.h"
#include "Settings.h"
#include "EnumOption.h"
#include "ListViewNoFlicker.h"
#include "AddFolderUI.h"
#include "OptionsUI.h"
#include "ImageFile.h"
#include "ResultComparer.h"


namespace ImageHunter {
	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class MainUI : public System::Windows::Forms::Form
	{
	public:
		MainUI(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainUI()
		{
			if (components)
			{
				delete components;
			}
		}

    // Constants
    private: static const int MIN_HEIGHT = 390;
	private: static const int MAX_HEIGHT = 680;
	private: static const int REFRESH_INTERVAL = 29;
    private: static const int MAX_LENGTH_IN_STATUS_BAS = 80;
    ///////////////////////////////////////////////////////////////////////////////////
    //  PLEASE DO NOT FORGET TO UPDATE APP.CONFIG WITH THE SAME EXCLUDED EXTENSIONS  //
    ///////////////////////////////////////////////////////////////////////////////////
    private: static const System::String^ DEFAULT_EXCLUDED_EXTENSION = L"exe; ini; txt; zip; pdf; log; bat; doc; xls; docx; xlsx; ppt; pptx";
    private: static const System::String^ CONFIG_TRUE = L"true";
    private: static const System::String^ CONFIG_FALSE = L"false";
    private: static const System::String^ DESKTOP_INI = L"Desktop.ini";
    private: static const int MAX_HISTORY_FOLDERS = 10;


    // UI constant
    private: static const System::String^ START_SEARCHING = L"Start hunting!";
    private: static const System::String^ WAITING_FOR_FILTERS = L"Waiting for filters...";
    private: static const System::String^ READY_TO_HUNT = L"Ready to hunt!";

    // Configuration Settings
    private: static const System::String^ EXTERNAL_TOOLS = "ExternalTools";
    private: static const System::String^ RECURSIVE_SEARCH = "RecursiveSearch";
    private: static const System::String^ BROWSING_HISTORY = "BrowsingHistory";
    private: static const System::String^ LAST_USED_FOLDERS = "LastUsedFolders";
    private: static const System::String^ DISPLAY_SENDTO_MENU = "DisplaySendToMenu";
    private: static const System::String^ EXCLUDED_EXTENSIONS = "ExcludedExtensions";

    // Class attributes
	private: bool m_IsMaximized;
    private: bool m_IsSearching;
    private: bool m_AutoScrollResults;
    private: System::String^ m_BrowsingHistory;
    private: bool m_IsSendToMenuDisplayed;
    private: System::String^ m_ExcludedExtensions;
    private: System::DateTime m_ExecutionStart;
    private: System::DateTime m_ExecutionStop;
    private: int m_LastColumnClicked;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
	private: System::Windows::Forms::GroupBox^  grpImagesDirectories;

	private: System::Windows::Forms::Button^  btnAddFolder;

	private: System::Windows::Forms::GroupBox^  grpFilters;
	private: System::Windows::Forms::Button^  btnAddFilter;
	private: System::Windows::Forms::ComboBox^  cboFilterName;
	private: System::Windows::Forms::Label^  lblFilter;
	private: System::Windows::Forms::Button^  btnSearch;
	private: System::Windows::Forms::MenuStrip^  MainMenu;
    private: System::Windows::Forms::ToolStripMenuItem^  fileMenuItem;
	private: System::Windows::Forms::GroupBox^  grpResults;
    private: ListViewNoFlicker^  lstResults;
	private: System::Windows::Forms::StatusStrip^  SearchStatus;
    private: System::Windows::Forms::ToolStripStatusLabel^  StatusToolStrip;
    private: System::Windows::Forms::ToolStripProgressBar^  StatusProgressToolStrip;
    private: System::Windows::Forms::ToolStripStatusLabel^  CurrentFileToolStrip;

    private: System::Windows::Forms::ToolStripMenuItem^  addFolderMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
    private: System::Windows::Forms::ToolStripMenuItem^  quitMenuItem;
	private: System::Windows::Forms::ListView^  lstFilters;
	private: System::Windows::Forms::CheckBox^  chkRecursiveSearch;
	private: System::Windows::Forms::Timer^  UIRefreshTimer;
	private: System::ComponentModel::IContainer^  components;
	private: System::Windows::Forms::Button^  btnClearResults;
	private: System::Windows::Forms::ColumnHeader^  columnHeader1;
	private: System::Windows::Forms::ColumnHeader^  columnHeader2;
    private: System::Windows::Forms::ToolStripMenuItem^  settingsMenuItem;
	private: System::Windows::Forms::ComboBox^  cboOperator;
 	private: System::Windows::Forms::TextBox^  txtTextFilter;
	private: System::Windows::Forms::Label^  lblTextFilter;

	private: System::Windows::Forms::ComboBox^  cboEnumFilter;



	private: System::Windows::Forms::Button^  btnRemoveAllFilters;
	private: System::Windows::Forms::Button^  btnRemoveSelectedFilter;
    private: System::Windows::Forms::ColumnHeader^  colFilename;
    private: System::Windows::Forms::ContextMenuStrip^  ResultsContextMenu;
    private: System::Windows::Forms::ToolStripMenuItem^  externalToolsMenuItem;
    private: System::Windows::Forms::ComboBox^  cboEnumWithFieldsFilter;
    private: System::Windows::Forms::Label^  lblEnumX;
    private: System::Windows::Forms::TextBox^  txtEnumY;
    private: System::Windows::Forms::Label^  lblEnumY;
    private: System::Windows::Forms::TextBox^  txtEnumX;

    private: System::Windows::Forms::Panel^  panelEnumFilter;
    private: System::Windows::Forms::Label^  lblEnumFilter;
    private: System::Windows::Forms::Panel^  panelEnumWithFieldsFilter;
    private: System::Windows::Forms::Label^  lblEnumWithFieldsFilter;
    private: System::Windows::Forms::ToolTip^  ToolTip;
    private: System::Windows::Forms::FolderBrowserDialog^  ImagesFolderBrowser;
    private: System::Windows::Forms::ToolStripMenuItem^  optionsToolMenuItem;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
    private: System::Windows::Forms::ToolStripMenuItem^  refreshSendToMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  openToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  copyToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  copyAllToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  cancelToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem1;
    private: System::Windows::Forms::ToolStripMenuItem^  exportResultsMenuItem;
    private: System::Windows::Forms::SaveFileDialog^  saveFileDialog;
    private: System::Windows::Forms::Label^  lblHuntingStats;
    private: System::Windows::Forms::Label^  lblAccuracyRate;


    private: System::Windows::Forms::ColumnHeader^  colImageName;
    private: System::Windows::Forms::ColumnHeader^  colImageSize;
    private: System::Windows::Forms::ColumnHeader^  colImagePath;
    private: System::Windows::Forms::ComboBox^  cboFolders;
    private: System::Windows::Forms::Panel^  panelXYFilter;
    private: System::Windows::Forms::ComboBox^  cboXFilter;
    private: System::Windows::Forms::TextBox^  txtYFilter;
    private: System::Windows::Forms::TextBox^  txtXFilter;
    private: System::Windows::Forms::Label^  lblYFilter;
    private: System::Windows::Forms::Label^  lblXFilter;
    private: System::Windows::Forms::Panel^  panelTextFilter;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainUI::typeid));
            this->grpImagesDirectories = (gcnew System::Windows::Forms::GroupBox());
            this->cboFolders = (gcnew System::Windows::Forms::ComboBox());
            this->chkRecursiveSearch = (gcnew System::Windows::Forms::CheckBox());
            this->btnAddFolder = (gcnew System::Windows::Forms::Button());
            this->grpFilters = (gcnew System::Windows::Forms::GroupBox());
            this->btnRemoveAllFilters = (gcnew System::Windows::Forms::Button());
            this->btnRemoveSelectedFilter = (gcnew System::Windows::Forms::Button());
            this->cboOperator = (gcnew System::Windows::Forms::ComboBox());
            this->lstFilters = (gcnew System::Windows::Forms::ListView());
            this->columnHeader1 = (gcnew System::Windows::Forms::ColumnHeader());
            this->columnHeader2 = (gcnew System::Windows::Forms::ColumnHeader());
            this->btnAddFilter = (gcnew System::Windows::Forms::Button());
            this->cboFilterName = (gcnew System::Windows::Forms::ComboBox());
            this->lblFilter = (gcnew System::Windows::Forms::Label());
            this->panelEnumFilter = (gcnew System::Windows::Forms::Panel());
            this->lblEnumFilter = (gcnew System::Windows::Forms::Label());
            this->cboEnumFilter = (gcnew System::Windows::Forms::ComboBox());
            this->panelEnumWithFieldsFilter = (gcnew System::Windows::Forms::Panel());
            this->lblEnumWithFieldsFilter = (gcnew System::Windows::Forms::Label());
            this->cboEnumWithFieldsFilter = (gcnew System::Windows::Forms::ComboBox());
            this->lblEnumX = (gcnew System::Windows::Forms::Label());
            this->txtEnumX = (gcnew System::Windows::Forms::TextBox());
            this->txtEnumY = (gcnew System::Windows::Forms::TextBox());
            this->lblEnumY = (gcnew System::Windows::Forms::Label());
            this->panelTextFilter = (gcnew System::Windows::Forms::Panel());
            this->lblTextFilter = (gcnew System::Windows::Forms::Label());
            this->txtTextFilter = (gcnew System::Windows::Forms::TextBox());
            this->btnSearch = (gcnew System::Windows::Forms::Button());
            this->MainMenu = (gcnew System::Windows::Forms::MenuStrip());
            this->fileMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->addFolderMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->exportResultsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->quitMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->settingsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->externalToolsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->optionsToolMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->refreshSendToMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->grpResults = (gcnew System::Windows::Forms::GroupBox());
            this->colFilename = (gcnew System::Windows::Forms::ColumnHeader());
            this->ResultsContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->copyToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->copyAllToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->cancelToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->SearchStatus = (gcnew System::Windows::Forms::StatusStrip());
            this->StatusToolStrip = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->StatusProgressToolStrip = (gcnew System::Windows::Forms::ToolStripProgressBar());
            this->CurrentFileToolStrip = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->UIRefreshTimer = (gcnew System::Windows::Forms::Timer(this->components));
            this->btnClearResults = (gcnew System::Windows::Forms::Button());
            this->ToolTip = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->ImagesFolderBrowser = (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->saveFileDialog = (gcnew System::Windows::Forms::SaveFileDialog());
            this->lblHuntingStats = (gcnew System::Windows::Forms::Label());
            this->lblAccuracyRate = (gcnew System::Windows::Forms::Label());
            this->colImageName = (gcnew System::Windows::Forms::ColumnHeader());
            this->colImageSize = (gcnew System::Windows::Forms::ColumnHeader());
            this->colImagePath = (gcnew System::Windows::Forms::ColumnHeader());
            this->panelXYFilter = (gcnew System::Windows::Forms::Panel());
            this->lblXFilter = (gcnew System::Windows::Forms::Label());
            this->lblYFilter = (gcnew System::Windows::Forms::Label());
            this->txtXFilter = (gcnew System::Windows::Forms::TextBox());
            this->txtYFilter = (gcnew System::Windows::Forms::TextBox());
            this->cboXFilter = (gcnew System::Windows::Forms::ComboBox());
            this->grpImagesDirectories->SuspendLayout();
            this->grpFilters->SuspendLayout();
            this->panelEnumFilter->SuspendLayout();
            this->panelEnumWithFieldsFilter->SuspendLayout();
            this->panelTextFilter->SuspendLayout();
            this->MainMenu->SuspendLayout();
            this->ResultsContextMenu->SuspendLayout();
            this->SearchStatus->SuspendLayout();
            this->panelXYFilter->SuspendLayout();
            this->SuspendLayout();
            // 
            // grpImagesDirectories
            // 
            this->grpImagesDirectories->Controls->Add(this->cboFolders);
            this->grpImagesDirectories->Controls->Add(this->chkRecursiveSearch);
            this->grpImagesDirectories->Controls->Add(this->btnAddFolder);
            this->grpImagesDirectories->Location = System::Drawing::Point(14, 27);
            this->grpImagesDirectories->Name = L"grpImagesDirectories";
            this->grpImagesDirectories->Size = System::Drawing::Size(646, 69);
            this->grpImagesDirectories->TabIndex = 1;
            this->grpImagesDirectories->TabStop = false;
            this->grpImagesDirectories->Text = L"Search folders";
            // 
            // cboFolders
            // 
            this->cboFolders->FormattingEnabled = true;
            this->cboFolders->Location = System::Drawing::Point(10, 18);
            this->cboFolders->Name = L"cboFolders";
            this->cboFolders->Size = System::Drawing::Size(592, 21);
            this->cboFolders->TabIndex = 0;
            // 
            // chkRecursiveSearch
            // 
            this->chkRecursiveSearch->AutoSize = true;
            this->chkRecursiveSearch->Location = System::Drawing::Point(10, 45);
            this->chkRecursiveSearch->Name = L"chkRecursiveSearch";
            this->chkRecursiveSearch->Size = System::Drawing::Size(109, 17);
            this->chkRecursiveSearch->TabIndex = 2;
            this->chkRecursiveSearch->Text = L"Recursive search";
            this->chkRecursiveSearch->UseVisualStyleBackColor = true;
            // 
            // btnAddFolder
            // 
            this->btnAddFolder->FlatAppearance->BorderSize = 0;
            this->btnAddFolder->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnAddFolder->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnAddFolder->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAddFolder.Image")));
            this->btnAddFolder->Location = System::Drawing::Point(608, 12);
            this->btnAddFolder->Name = L"btnAddFolder";
            this->btnAddFolder->Size = System::Drawing::Size(32, 32);
            this->btnAddFolder->TabIndex = 1;
            this->ToolTip->SetToolTip(this->btnAddFolder, L"Add folder to search");
            this->btnAddFolder->UseVisualStyleBackColor = true;
            this->btnAddFolder->Click += gcnew System::EventHandler(this, &MainUI::btnAddFolder_Click);
            // 
            // grpFilters
            // 
            this->grpFilters->Controls->Add(this->panelXYFilter);
            this->grpFilters->Controls->Add(this->btnRemoveAllFilters);
            this->grpFilters->Controls->Add(this->btnRemoveSelectedFilter);
            this->grpFilters->Controls->Add(this->cboOperator);
            this->grpFilters->Controls->Add(this->lstFilters);
            this->grpFilters->Controls->Add(this->btnAddFilter);
            this->grpFilters->Controls->Add(this->cboFilterName);
            this->grpFilters->Controls->Add(this->lblFilter);
            this->grpFilters->Controls->Add(this->panelTextFilter);
            this->grpFilters->Controls->Add(this->panelEnumFilter);
            this->grpFilters->Controls->Add(this->panelEnumWithFieldsFilter);
            this->grpFilters->Location = System::Drawing::Point(14, 102);
            this->grpFilters->Name = L"grpFilters";
            this->grpFilters->Size = System::Drawing::Size(647, 194);
            this->grpFilters->TabIndex = 2;
            this->grpFilters->TabStop = false;
            this->grpFilters->Text = L"Filters";
            // 
            // btnRemoveAllFilters
            // 
            this->btnRemoveAllFilters->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
            this->btnRemoveAllFilters->FlatAppearance->BorderSize = 0;
            this->btnRemoveAllFilters->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnRemoveAllFilters->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnRemoveAllFilters->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnRemoveAllFilters.Image")));
            this->btnRemoveAllFilters->Location = System::Drawing::Point(608, 115);
            this->btnRemoveAllFilters->Name = L"btnRemoveAllFilters";
            this->btnRemoveAllFilters->Size = System::Drawing::Size(32, 32);
            this->btnRemoveAllFilters->TabIndex = 12;
            this->ToolTip->SetToolTip(this->btnRemoveAllFilters, L"Remove all search filters");
            this->btnRemoveAllFilters->UseVisualStyleBackColor = true;
            this->btnRemoveAllFilters->Click += gcnew System::EventHandler(this, &MainUI::btnRemoveAllFilters_Click);
            // 
            // btnRemoveSelectedFilter
            // 
            this->btnRemoveSelectedFilter->FlatAppearance->BorderSize = 0;
            this->btnRemoveSelectedFilter->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnRemoveSelectedFilter->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnRemoveSelectedFilter->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnRemoveSelectedFilter.Image")));
            this->btnRemoveSelectedFilter->Location = System::Drawing::Point(608, 77);
            this->btnRemoveSelectedFilter->Name = L"btnRemoveSelectedFilter";
            this->btnRemoveSelectedFilter->Size = System::Drawing::Size(32, 32);
            this->btnRemoveSelectedFilter->TabIndex = 11;
            this->ToolTip->SetToolTip(this->btnRemoveSelectedFilter, L"Remove selected filter");
            this->btnRemoveSelectedFilter->UseVisualStyleBackColor = true;
            this->btnRemoveSelectedFilter->Click += gcnew System::EventHandler(this, &MainUI::btnRemoveSelectedFilter_Click);
            // 
            // cboOperator
            // 
            this->cboOperator->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cboOperator->FormattingEnabled = true;
            this->cboOperator->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"==", L"!=", L">", L">=", L"<", L"<="});
            this->cboOperator->Location = System::Drawing::Point(208, 19);
            this->cboOperator->Name = L"cboOperator";
            this->cboOperator->Size = System::Drawing::Size(36, 21);
            this->cboOperator->TabIndex = 2;
            // 
            // lstFilters
            // 
            this->lstFilters->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->columnHeader1, this->columnHeader2});
            this->lstFilters->FullRowSelect = true;
            this->lstFilters->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
            this->lstFilters->Location = System::Drawing::Point(10, 48);
            this->lstFilters->Name = L"lstFilters";
            this->lstFilters->Size = System::Drawing::Size(593, 140);
            this->lstFilters->TabIndex = 10;
            this->lstFilters->UseCompatibleStateImageBehavior = false;
            this->lstFilters->View = System::Windows::Forms::View::Details;
            this->lstFilters->SelectedIndexChanged += gcnew System::EventHandler(this, &MainUI::lstFilters_SelectedIndexChanged);
            this->lstFilters->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::lstFilters_KeyUp);
            // 
            // columnHeader1
            // 
            this->columnHeader1->Text = L"Filter name";
            this->columnHeader1->Width = 146;
            // 
            // columnHeader2
            // 
            this->columnHeader2->Text = L"Search criteria";
            this->columnHeader2->Width = 421;
            // 
            // btnAddFilter
            // 
            this->btnAddFilter->FlatAppearance->BorderSize = 0;
            this->btnAddFilter->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnAddFilter->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnAddFilter->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAddFilter.Image")));
            this->btnAddFilter->Location = System::Drawing::Point(560, 15);
            this->btnAddFilter->Name = L"btnAddFilter";
            this->btnAddFilter->Size = System::Drawing::Size(81, 29);
            this->btnAddFilter->TabIndex = 9;
            this->btnAddFilter->Text = L"Add filter";
            this->btnAddFilter->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->ToolTip->SetToolTip(this->btnAddFilter, L"Add filter");
            this->btnAddFilter->UseVisualStyleBackColor = true;
            this->btnAddFilter->Click += gcnew System::EventHandler(this, &MainUI::btnAddFilter_Click);
            // 
            // cboFilterName
            // 
            this->cboFilterName->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cboFilterName->FormattingEnabled = true;
            this->cboFilterName->ImeMode = System::Windows::Forms::ImeMode::NoControl;
            this->cboFilterName->Location = System::Drawing::Point(71, 19);
            this->cboFilterName->Name = L"cboFilterName";
            this->cboFilterName->Size = System::Drawing::Size(130, 21);
            this->cboFilterName->Sorted = true;
            this->cboFilterName->TabIndex = 1;
            this->cboFilterName->SelectedIndexChanged += gcnew System::EventHandler(this, &MainUI::cboFilterName_SelectedIndexChanged);
            // 
            // lblFilter
            // 
            this->lblFilter->AutoSize = true;
            this->lblFilter->Location = System::Drawing::Point(7, 23);
            this->lblFilter->Name = L"lblFilter";
            this->lblFilter->Size = System::Drawing::Size(58, 13);
            this->lblFilter->TabIndex = 0;
            this->lblFilter->Text = L"Filter name";
            // 
            // panelEnumFilter
            // 
            this->panelEnumFilter->Controls->Add(this->lblEnumFilter);
            this->panelEnumFilter->Controls->Add(this->cboEnumFilter);
            this->panelEnumFilter->Location = System::Drawing::Point(246, 10);
            this->panelEnumFilter->Name = L"panelEnumFilter";
            this->panelEnumFilter->Size = System::Drawing::Size(311, 36);
            this->panelEnumFilter->TabIndex = 6;
            // 
            // lblEnumFilter
            // 
            this->lblEnumFilter->AutoSize = true;
            this->lblEnumFilter->Location = System::Drawing::Point(27, 13);
            this->lblEnumFilter->Name = L"lblEnumFilter";
            this->lblEnumFilter->Size = System::Drawing::Size(43, 13);
            this->lblEnumFilter->TabIndex = 4;
            this->lblEnumFilter->Text = L"Choose";
            // 
            // cboEnumFilter
            // 
            this->cboEnumFilter->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cboEnumFilter->FormattingEnabled = true;
            this->cboEnumFilter->Location = System::Drawing::Point(76, 9);
            this->cboEnumFilter->Name = L"cboEnumFilter";
            this->cboEnumFilter->Size = System::Drawing::Size(163, 21);
            this->cboEnumFilter->TabIndex = 3;
            this->cboEnumFilter->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::FilterControls_KeyUp);
            // 
            // panelEnumWithFieldsFilter
            // 
            this->panelEnumWithFieldsFilter->Controls->Add(this->lblEnumWithFieldsFilter);
            this->panelEnumWithFieldsFilter->Controls->Add(this->cboEnumWithFieldsFilter);
            this->panelEnumWithFieldsFilter->Controls->Add(this->lblEnumX);
            this->panelEnumWithFieldsFilter->Controls->Add(this->txtEnumX);
            this->panelEnumWithFieldsFilter->Controls->Add(this->txtEnumY);
            this->panelEnumWithFieldsFilter->Controls->Add(this->lblEnumY);
            this->panelEnumWithFieldsFilter->Location = System::Drawing::Point(246, 10);
            this->panelEnumWithFieldsFilter->Name = L"panelEnumWithFieldsFilter";
            this->panelEnumWithFieldsFilter->Size = System::Drawing::Size(311, 36);
            this->panelEnumWithFieldsFilter->TabIndex = 3;
            // 
            // lblEnumWithFieldsFilter
            // 
            this->lblEnumWithFieldsFilter->AutoSize = true;
            this->lblEnumWithFieldsFilter->Location = System::Drawing::Point(13, 13);
            this->lblEnumWithFieldsFilter->Name = L"lblEnumWithFieldsFilter";
            this->lblEnumWithFieldsFilter->Size = System::Drawing::Size(43, 13);
            this->lblEnumWithFieldsFilter->TabIndex = 0;
            this->lblEnumWithFieldsFilter->Text = L"Choose";
            // 
            // cboEnumWithFieldsFilter
            // 
            this->cboEnumWithFieldsFilter->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cboEnumWithFieldsFilter->FormattingEnabled = true;
            this->cboEnumWithFieldsFilter->Location = System::Drawing::Point(62, 10);
            this->cboEnumWithFieldsFilter->Name = L"cboEnumWithFieldsFilter";
            this->cboEnumWithFieldsFilter->Size = System::Drawing::Size(60, 21);
            this->cboEnumWithFieldsFilter->TabIndex = 13;
            this->cboEnumWithFieldsFilter->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::FilterControls_KeyUp);
            // 
            // lblEnumX
            // 
            this->lblEnumX->AutoSize = true;
            this->lblEnumX->Location = System::Drawing::Point(128, 13);
            this->lblEnumX->Name = L"lblEnumX";
            this->lblEnumX->Size = System::Drawing::Size(35, 13);
            this->lblEnumX->TabIndex = 14;
            this->lblEnumX->Text = L"Width";
            // 
            // txtEnumX
            // 
            this->txtEnumX->Location = System::Drawing::Point(169, 10);
            this->txtEnumX->Name = L"txtEnumX";
            this->txtEnumX->Size = System::Drawing::Size(41, 20);
            this->txtEnumX->TabIndex = 15;
            this->txtEnumX->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::FilterControls_KeyUp);
            // 
            // txtEnumY
            // 
            this->txtEnumY->Location = System::Drawing::Point(260, 10);
            this->txtEnumY->Name = L"txtEnumY";
            this->txtEnumY->Size = System::Drawing::Size(41, 20);
            this->txtEnumY->TabIndex = 17;
            this->txtEnumY->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::FilterControls_KeyUp);
            // 
            // lblEnumY
            // 
            this->lblEnumY->AutoSize = true;
            this->lblEnumY->Location = System::Drawing::Point(216, 13);
            this->lblEnumY->Name = L"lblEnumY";
            this->lblEnumY->Size = System::Drawing::Size(38, 13);
            this->lblEnumY->TabIndex = 0;
            this->lblEnumY->Text = L"Height";
            // 
            // panelTextFilter
            // 
            this->panelTextFilter->Controls->Add(this->lblTextFilter);
            this->panelTextFilter->Controls->Add(this->txtTextFilter);
            this->panelTextFilter->Location = System::Drawing::Point(246, 10);
            this->panelTextFilter->Name = L"panelTextFilter";
            this->panelTextFilter->Size = System::Drawing::Size(311, 36);
            this->panelTextFilter->TabIndex = 4;
            // 
            // lblTextFilter
            // 
            this->lblTextFilter->AutoSize = true;
            this->lblTextFilter->Location = System::Drawing::Point(57, 11);
            this->lblTextFilter->Name = L"lblTextFilter";
            this->lblTextFilter->Size = System::Drawing::Size(34, 13);
            this->lblTextFilter->TabIndex = 6;
            this->lblTextFilter->Text = L"Value";
            // 
            // txtTextFilter
            // 
            this->txtTextFilter->Location = System::Drawing::Point(97, 8);
            this->txtTextFilter->Name = L"txtTextFilter";
            this->txtTextFilter->Size = System::Drawing::Size(163, 20);
            this->txtTextFilter->TabIndex = 3;
            this->txtTextFilter->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &MainUI::FilterControls_KeyUp);
            // 
            // btnSearch
            // 
            this->btnSearch->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSearch.Image")));
            this->btnSearch->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnSearch->Location = System::Drawing::Point(510, 302);
            this->btnSearch->Name = L"btnSearch";
            this->btnSearch->Size = System::Drawing::Size(152, 31);
            this->btnSearch->TabIndex = 3;
            this->btnSearch->Text = L"Start hunting!";
            this->btnSearch->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->ToolTip->SetToolTip(this->btnSearch, L"Search in folders for images that respect the filters");
            this->btnSearch->UseVisualStyleBackColor = true;
            this->btnSearch->Click += gcnew System::EventHandler(this, &MainUI::btnSearch_Click);
            // 
            // MainMenu
            // 
            this->MainMenu->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Center;
            this->MainMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->fileMenuItem, this->settingsMenuItem});
            this->MainMenu->Location = System::Drawing::Point(0, 0);
            this->MainMenu->Name = L"MainMenu";
            this->MainMenu->Size = System::Drawing::Size(674, 24);
            this->MainMenu->TabIndex = 0;
            this->MainMenu->Text = L"MainMenu";
            // 
            // fileMenuItem
            // 
            this->fileMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->addFolderMenuItem, 
                this->exportResultsMenuItem, this->toolStripSeparator1, this->quitMenuItem});
            this->fileMenuItem->Name = L"fileMenuItem";
            this->fileMenuItem->Size = System::Drawing::Size(37, 20);
            this->fileMenuItem->Text = L"File";
            // 
            // addFolderMenuItem
            // 
            this->addFolderMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"addFolderMenuItem.Image")));
            this->addFolderMenuItem->Name = L"addFolderMenuItem";
            this->addFolderMenuItem->Size = System::Drawing::Size(186, 22);
            this->addFolderMenuItem->Text = L"Add folder";
            this->addFolderMenuItem->Click += gcnew System::EventHandler(this, &MainUI::addFolderMenuItem_Click);
            // 
            // exportResultsMenuItem
            // 
            this->exportResultsMenuItem->Name = L"exportResultsMenuItem";
            this->exportResultsMenuItem->Size = System::Drawing::Size(186, 22);
            this->exportResultsMenuItem->Text = L"Export results to file...";
            this->exportResultsMenuItem->Click += gcnew System::EventHandler(this, &MainUI::exportResultsMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this->toolStripSeparator1->Name = L"toolStripSeparator1";
            this->toolStripSeparator1->Size = System::Drawing::Size(183, 6);
            // 
            // quitMenuItem
            // 
            this->quitMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"quitMenuItem.Image")));
            this->quitMenuItem->Name = L"quitMenuItem";
            this->quitMenuItem->Size = System::Drawing::Size(186, 22);
            this->quitMenuItem->Text = L"Quit";
            this->quitMenuItem->Click += gcnew System::EventHandler(this, &MainUI::quitToolStripMenuItem_Click);
            // 
            // settingsMenuItem
            // 
            this->settingsMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->externalToolsMenuItem, 
                this->optionsToolMenuItem, this->toolStripSeparator2, this->refreshSendToMenuItem});
            this->settingsMenuItem->Name = L"settingsMenuItem";
            this->settingsMenuItem->Size = System::Drawing::Size(61, 20);
            this->settingsMenuItem->Text = L"Settings";
            // 
            // externalToolsMenuItem
            // 
            this->externalToolsMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"externalToolsMenuItem.Image")));
            this->externalToolsMenuItem->Name = L"externalToolsMenuItem";
            this->externalToolsMenuItem->Size = System::Drawing::Size(187, 22);
            this->externalToolsMenuItem->Text = L"External Tools";
            this->externalToolsMenuItem->Click += gcnew System::EventHandler(this, &MainUI::externalToolsMenuItem_Click);
            // 
            // optionsToolMenuItem
            // 
            this->optionsToolMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"optionsToolMenuItem.Image")));
            this->optionsToolMenuItem->Name = L"optionsToolMenuItem";
            this->optionsToolMenuItem->Size = System::Drawing::Size(187, 22);
            this->optionsToolMenuItem->Text = L"Options";
            this->optionsToolMenuItem->Click += gcnew System::EventHandler(this, &MainUI::optionsToolMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->Size = System::Drawing::Size(184, 6);
            // 
            // refreshSendToMenuItem
            // 
            this->refreshSendToMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"refreshSendToMenuItem.Image")));
            this->refreshSendToMenuItem->Name = L"refreshSendToMenuItem";
            this->refreshSendToMenuItem->Size = System::Drawing::Size(187, 22);
            this->refreshSendToMenuItem->Text = L"Reload SendTo Menu";
            this->refreshSendToMenuItem->Click += gcnew System::EventHandler(this, &MainUI::refreshSendToMenuItem_Click);
            // 
            // grpResults
            // 
            this->grpResults->Location = System::Drawing::Point(15, 339);
            this->grpResults->Name = L"grpResults";
            this->grpResults->Size = System::Drawing::Size(647, 244);
            this->grpResults->TabIndex = 4;
            this->grpResults->TabStop = false;
            this->grpResults->Text = L"Results";
            // 
            // colFilename
            // 
            this->colFilename->Text = L"Filename";
            this->colFilename->Width = 600;
            // 
            // ResultsContextMenu
            // 
            this->ResultsContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->openToolStripMenuItem, 
                this->copyToolStripMenuItem, this->copyAllToolStripMenuItem, this->cancelToolStripMenuItem});
            this->ResultsContextMenu->Name = L"ResultsContextMenu";
            this->ResultsContextMenu->Size = System::Drawing::Size(116, 92);
            // 
            // openToolStripMenuItem
            // 
            this->openToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"openToolStripMenuItem.Image")));
            this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
            this->openToolStripMenuItem->Size = System::Drawing::Size(115, 22);
            this->openToolStripMenuItem->Text = L"open";
            // 
            // copyToolStripMenuItem
            // 
            this->copyToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyToolStripMenuItem.Image")));
            this->copyToolStripMenuItem->Name = L"copyToolStripMenuItem";
            this->copyToolStripMenuItem->Size = System::Drawing::Size(115, 22);
            this->copyToolStripMenuItem->Text = L"copy";
            // 
            // copyAllToolStripMenuItem
            // 
            this->copyAllToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyAllToolStripMenuItem.Image")));
            this->copyAllToolStripMenuItem->Name = L"copyAllToolStripMenuItem";
            this->copyAllToolStripMenuItem->Size = System::Drawing::Size(115, 22);
            this->copyAllToolStripMenuItem->Text = L"copy all";
            // 
            // cancelToolStripMenuItem
            // 
            this->cancelToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"cancelToolStripMenuItem.Image")));
            this->cancelToolStripMenuItem->Name = L"cancelToolStripMenuItem";
            this->cancelToolStripMenuItem->Size = System::Drawing::Size(115, 22);
            this->cancelToolStripMenuItem->Text = L"cancel";
            // 
            // SearchStatus
            // 
            this->SearchStatus->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->StatusToolStrip, 
                this->StatusProgressToolStrip, this->CurrentFileToolStrip});
            this->SearchStatus->Location = System::Drawing::Point(0, 629);
            this->SearchStatus->Name = L"SearchStatus";
            this->SearchStatus->Size = System::Drawing::Size(674, 22);
            this->SearchStatus->SizingGrip = false;
            this->SearchStatus->TabIndex = 5;
            this->SearchStatus->Text = L"SearchStatus";
            // 
            // StatusToolStrip
            // 
            this->StatusToolStrip->Name = L"StatusToolStrip";
            this->StatusToolStrip->Size = System::Drawing::Size(90, 17);
            this->StatusToolStrip->Text = L"Search progress";
            // 
            // StatusProgressToolStrip
            // 
            this->StatusProgressToolStrip->AutoToolTip = true;
            this->StatusProgressToolStrip->Name = L"StatusProgressToolStrip";
            this->StatusProgressToolStrip->Size = System::Drawing::Size(100, 16);
            this->StatusProgressToolStrip->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
            // 
            // CurrentFileToolStrip
            // 
            this->CurrentFileToolStrip->AutoToolTip = true;
            this->CurrentFileToolStrip->Name = L"CurrentFileToolStrip";
            this->CurrentFileToolStrip->Padding = System::Windows::Forms::Padding(20, 0, 0, 0);
            this->CurrentFileToolStrip->Size = System::Drawing::Size(569, 17);
            this->CurrentFileToolStrip->Spring = true;
            this->CurrentFileToolStrip->Text = L"C:\\Documents and Settings\\User\\My Pictures\\DSC000012.JPG";
            this->CurrentFileToolStrip->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // UIRefreshTimer
            // 
            this->UIRefreshTimer->Interval = 10;
            this->UIRefreshTimer->Tick += gcnew System::EventHandler(this, &MainUI::UIRefreshTimer_Tick);
            // 
            // btnClearResults
            // 
            this->btnClearResults->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnClearResults.Image")));
            this->btnClearResults->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnClearResults->Location = System::Drawing::Point(508, 589);
            this->btnClearResults->Name = L"btnClearResults";
            this->btnClearResults->Size = System::Drawing::Size(152, 32);
            this->btnClearResults->TabIndex = 5;
            this->btnClearResults->Text = L"Clear results";
            this->btnClearResults->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->ToolTip->SetToolTip(this->btnClearResults, L"Clear the search results");
            this->btnClearResults->UseVisualStyleBackColor = true;
            this->btnClearResults->Click += gcnew System::EventHandler(this, &MainUI::btnClearResults_Click);
            // 
            // ImagesFolderBrowser
            // 
            this->ImagesFolderBrowser->ShowNewFolderButton = false;
            // 
            // saveFileDialog
            // 
            this->saveFileDialog->DefaultExt = L"txt";
            this->saveFileDialog->Filter = L"Text files|*.txt|All files|*.*";
            this->saveFileDialog->Title = L"Export results as";
            // 
            // lblHuntingStats
            // 
            this->lblHuntingStats->AutoSize = true;
            this->lblHuntingStats->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->lblHuntingStats->Location = System::Drawing::Point(21, 589);
            this->lblHuntingStats->Name = L"lblHuntingStats";
            this->lblHuntingStats->Size = System::Drawing::Size(69, 13);
            this->lblHuntingStats->TabIndex = 6;
            this->lblHuntingStats->Text = L"Hunting stats";
            // 
            // lblAccuracyRate
            // 
            this->lblAccuracyRate->AutoSize = true;
            this->lblAccuracyRate->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->lblAccuracyRate->Location = System::Drawing::Point(21, 608);
            this->lblAccuracyRate->Name = L"lblAccuracyRate";
            this->lblAccuracyRate->Size = System::Drawing::Size(73, 13);
            this->lblAccuracyRate->TabIndex = 7;
            this->lblAccuracyRate->Text = L"Accuracy rate";
            // 
            // colImageName
            // 
            this->colImageName->Text = L"Filename";
            this->colImageName->Width = 207;
            // 
            // colImageSize
            // 
            this->colImageSize->Text = L"Size";
            // 
            // colImagePath
            // 
            this->colImagePath->Text = L"Directory";
            this->colImagePath->Width = 333;
            // 
            // panelXYFilter
            // 
            this->panelXYFilter->Controls->Add(this->cboXFilter);
            this->panelXYFilter->Controls->Add(this->txtYFilter);
            this->panelXYFilter->Controls->Add(this->txtXFilter);
            this->panelXYFilter->Controls->Add(this->lblYFilter);
            this->panelXYFilter->Controls->Add(this->lblXFilter);
            this->panelXYFilter->Location = System::Drawing::Point(246, 9);
            this->panelXYFilter->Name = L"panelXYFilter";
            this->panelXYFilter->Size = System::Drawing::Size(311, 36);
            this->panelXYFilter->TabIndex = 13;
            // 
            // lblXFilter
            // 
            this->lblXFilter->AutoSize = true;
            this->lblXFilter->Location = System::Drawing::Point(19, 13);
            this->lblXFilter->Name = L"lblXFilter";
            this->lblXFilter->Size = System::Drawing::Size(35, 13);
            this->lblXFilter->TabIndex = 0;
            this->lblXFilter->Text = L"Width";
            this->lblXFilter->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // lblYFilter
            // 
            this->lblYFilter->AutoSize = true;
            this->lblYFilter->Location = System::Drawing::Point(168, 14);
            this->lblYFilter->Name = L"lblYFilter";
            this->lblYFilter->Size = System::Drawing::Size(38, 13);
            this->lblYFilter->TabIndex = 1;
            this->lblYFilter->Text = L"Height";
            this->lblYFilter->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // txtXFilter
            // 
            this->txtXFilter->Location = System::Drawing::Point(60, 11);
            this->txtXFilter->Name = L"txtXFilter";
            this->txtXFilter->Size = System::Drawing::Size(89, 20);
            this->txtXFilter->TabIndex = 2;
            // 
            // txtYFilter
            // 
            this->txtYFilter->Location = System::Drawing::Point(212, 10);
            this->txtYFilter->Name = L"txtYFilter";
            this->txtYFilter->Size = System::Drawing::Size(89, 20);
            this->txtYFilter->TabIndex = 3;
            // 
            // cboXFilter
            // 
            this->cboXFilter->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cboXFilter->DropDownWidth = 200;
            this->cboXFilter->FormattingEnabled = true;
            this->cboXFilter->Location = System::Drawing::Point(62, 11);
            this->cboXFilter->Name = L"cboXFilter";
            this->cboXFilter->Size = System::Drawing::Size(87, 21);
            this->cboXFilter->Sorted = true;
            this->cboXFilter->TabIndex = 4;
            // 
            // MainUI
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(674, 651);
            this->Controls->Add(this->lblAccuracyRate);
            this->Controls->Add(this->lblHuntingStats);
            this->Controls->Add(this->SearchStatus);
            this->Controls->Add(this->grpImagesDirectories);
            this->Controls->Add(this->grpResults);
            this->Controls->Add(this->grpFilters);
            this->Controls->Add(this->MainMenu);
            this->Controls->Add(this->btnClearResults);
            this->Controls->Add(this->btnSearch);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->Name = L"MainUI";
            this->Text = L"Image Hunter";
            this->Load += gcnew System::EventHandler(this, &MainUI::MainUI_Load);
            this->Closing += gcnew System::ComponentModel::CancelEventHandler(this, &MainUI::MainUI_Closing);
            this->grpImagesDirectories->ResumeLayout(false);
            this->grpImagesDirectories->PerformLayout();
            this->grpFilters->ResumeLayout(false);
            this->grpFilters->PerformLayout();
            this->panelEnumFilter->ResumeLayout(false);
            this->panelEnumFilter->PerformLayout();
            this->panelEnumWithFieldsFilter->ResumeLayout(false);
            this->panelEnumWithFieldsFilter->PerformLayout();
            this->panelTextFilter->ResumeLayout(false);
            this->panelTextFilter->PerformLayout();
            this->MainMenu->ResumeLayout(false);
            this->MainMenu->PerformLayout();
            this->ResultsContextMenu->ResumeLayout(false);
            this->SearchStatus->ResumeLayout(false);
            this->SearchStatus->PerformLayout();
            this->panelXYFilter->ResumeLayout(false);
            this->panelXYFilter->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

#pragma region Windows Form Events
    private:
        System::Void MainUI_Load(System::Object^  sender, System::EventArgs^  e);
        System::Void btnAddFolder_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnSearch_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void UIRefreshTimer_Tick(System::Object^  sender, System::EventArgs^  e);
        System::Void btnClearResults_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void quitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void cboFilterName_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void btnAddFilter_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnRemoveSelectedFilter_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnRemoveAllFilters_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void addFolderMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void ExternalTools_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void MainUI_Closing(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e);
        System::Void UpdateStatus_Callback(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e);
        System::Void EndOfSearch_Callback(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
        System::Void externalToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void OpenContainingFolderItem_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void CopySelectedItem_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void CopyAllItems_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void CopyImagePath_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void CopyEntireList_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void SendToMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        System::Void lstFilters_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void lstFilters_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
        System::Void FilterControls_KeyUp(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e);
        System::Void lstResults_DoubleClick(System::Object^  sender, System::EventArgs^  e);
        System::Void lstResults_ItemDrag(System::Object^ sender, System::Windows::Forms::ItemDragEventArgs^ e);
        System::Void optionsToolMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void refreshSendToMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void exportResultsMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void lstResults_ColumnClick(System::Object^ sender, System::Windows::Forms::ColumnClickEventArgs^ e);
        
#pragma endregion


#pragma region Sub-Routines Declarations
    private:
        System::Void InitResultsListView();
        System::Void LoadExternalTools();
        System::Collections::ArrayList^ LoadSendToMenu();
        System::Void PopulateResultsContextMenu();
        System::Void AddFolder();
        System::Void ResetStatusBar();
        System::Void RefreshResults();
        System::Void ShowTextOperators();
        System::Void ShowAllOperators();
        System::Void ShowTextFilterControls(System::String^ label);
        System::Void ShowEnumWithFieldsFiltercontrols(System::Collections::ArrayList^ options);
        System::Void ShowEnumFilterControls(System::Collections::ArrayList^ options);
        System::Void ShowXYFilterControls(System::String^ XLabel, System::String^ YLabel);
        System::Void HideFilterOptions();
        System::Void LoadAppSettings();
        System::Void SaveAppSettings();
        System::Void CompileSearchStats();

#pragma endregion
};
}