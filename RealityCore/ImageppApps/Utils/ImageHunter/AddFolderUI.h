/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/AddFolderUI.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

namespace ImageHunter {

	/// <summary>
	/// Summary for AddFolderUI
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class AddFolderUI : public System::Windows::Forms::Form
	{
	public:
		AddFolderUI(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
            m_BrowsingHistory = gcnew System::Collections::ArrayList();
            m_SearchSources = gcnew System::Collections::ArrayList();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~AddFolderUI()
		{
			if (components)
			{
				delete components;
			}
		}

    private: System::Collections::ArrayList^ m_SearchSources;
    private: System::Collections::ArrayList^ m_BrowsingHistory;
    private: static const System::UInt32 MAX_BROWSING_HISTORY = 15;
    private: static const System::String^ MY_NETWORK_PLACES = L"My Network Places";

    private: System::Windows::Forms::GroupBox^  grpAvailableFolders;
    private: System::Windows::Forms::Button^  btnFolderUp;
    private: System::Windows::Forms::ComboBox^  cboFolders;
    private: System::Windows::Forms::GroupBox^  grpSelectedFolders;
    private: System::Windows::Forms::Button^  btnAdd;
    private: System::Windows::Forms::Button^  btnRemove;
    private: System::Windows::Forms::ListBox^  lstSelectedFolders;
    private: System::Windows::Forms::ListView^  lstFolders;
    private: System::Windows::Forms::Button^  btnMoveUp;
    private: System::Windows::Forms::Button^  btnMoveDown;
    private: System::Windows::Forms::Button^  btnAccept;
    private: System::Windows::Forms::Button^  btnCancel;
    private: System::Windows::Forms::ImageList^  icons;
    private: System::Windows::Forms::ColumnHeader^  colFilename;
    private: System::ComponentModel::IContainer^  components;


    protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(AddFolderUI::typeid));
            this->grpAvailableFolders = (gcnew System::Windows::Forms::GroupBox());
            this->lstFolders = (gcnew System::Windows::Forms::ListView());
            this->colFilename = (gcnew System::Windows::Forms::ColumnHeader());
            this->icons = (gcnew System::Windows::Forms::ImageList(this->components));
            this->btnFolderUp = (gcnew System::Windows::Forms::Button());
            this->cboFolders = (gcnew System::Windows::Forms::ComboBox());
            this->grpSelectedFolders = (gcnew System::Windows::Forms::GroupBox());
            this->btnMoveUp = (gcnew System::Windows::Forms::Button());
            this->btnMoveDown = (gcnew System::Windows::Forms::Button());
            this->lstSelectedFolders = (gcnew System::Windows::Forms::ListBox());
            this->btnRemove = (gcnew System::Windows::Forms::Button());
            this->btnAdd = (gcnew System::Windows::Forms::Button());
            this->btnAccept = (gcnew System::Windows::Forms::Button());
            this->btnCancel = (gcnew System::Windows::Forms::Button());
            this->grpAvailableFolders->SuspendLayout();
            this->grpSelectedFolders->SuspendLayout();
            this->SuspendLayout();
            // 
            // grpAvailableFolders
            // 
            this->grpAvailableFolders->Controls->Add(this->lstFolders);
            this->grpAvailableFolders->Controls->Add(this->btnFolderUp);
            this->grpAvailableFolders->Controls->Add(this->cboFolders);
            this->grpAvailableFolders->Location = System::Drawing::Point(12, 12);
            this->grpAvailableFolders->Name = L"grpAvailableFolders";
            this->grpAvailableFolders->Size = System::Drawing::Size(287, 293);
            this->grpAvailableFolders->TabIndex = 1;
            this->grpAvailableFolders->TabStop = false;
            this->grpAvailableFolders->Text = L"Available folders";
            // 
            // lstFolders
            // 
            this->lstFolders->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(1) {this->colFilename});
            this->lstFolders->FullRowSelect = true;
            this->lstFolders->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::None;
            this->lstFolders->Location = System::Drawing::Point(7, 52);
            this->lstFolders->Name = L"lstFolders";
            this->lstFolders->Size = System::Drawing::Size(274, 231);
            this->lstFolders->SmallImageList = this->icons;
            this->lstFolders->TabIndex = 3;
            this->lstFolders->UseCompatibleStateImageBehavior = false;
            this->lstFolders->View = System::Windows::Forms::View::Details;
            this->lstFolders->SelectedIndexChanged += gcnew System::EventHandler(this, &AddFolderUI::lstFolders_SelectedIndexChanged);
            this->lstFolders->DoubleClick += gcnew System::EventHandler(this, &AddFolderUI::lstFolders_DoubleClick);
            // 
            // colFilename
            // 
            this->colFilename->Text = L"Folder name";
            this->colFilename->Width = 252;
            // 
            // icons
            // 
            this->icons->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"icons.ImageStream")));
            this->icons->TransparentColor = System::Drawing::Color::Transparent;
            this->icons->Images->SetKeyName(0, L"Folder");
            this->icons->Images->SetKeyName(1, L"HardDrive");
            this->icons->Images->SetKeyName(2, L"CD");
            this->icons->Images->SetKeyName(3, L"Network");
            this->icons->Images->SetKeyName(4, L"Removable");
            this->icons->Images->SetKeyName(5, L"Unknown");
            this->icons->Images->SetKeyName(6, L"Network");
            // 
            // btnFolderUp
            // 
            this->btnFolderUp->FlatAppearance->BorderSize = 0;
            this->btnFolderUp->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnFolderUp.Image")));
            this->btnFolderUp->Location = System::Drawing::Point(253, 19);
            this->btnFolderUp->Name = L"btnFolderUp";
            this->btnFolderUp->Size = System::Drawing::Size(28, 28);
            this->btnFolderUp->TabIndex = 2;
            this->btnFolderUp->UseVisualStyleBackColor = true;
            this->btnFolderUp->Click += gcnew System::EventHandler(this, &AddFolderUI::btnFolderUp_Click);
            // 
            // cboFolders
            // 
            this->cboFolders->FormattingEnabled = true;
            this->cboFolders->Location = System::Drawing::Point(6, 24);
            this->cboFolders->Name = L"cboFolders";
            this->cboFolders->Size = System::Drawing::Size(241, 21);
            this->cboFolders->TabIndex = 1;
            this->cboFolders->SelectedIndexChanged += gcnew System::EventHandler(this, &AddFolderUI::cboFolders_SelectedIndexChanged);
            this->cboFolders->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &AddFolderUI::cboFolders_KeyUp);
            // 
            // grpSelectedFolders
            // 
            this->grpSelectedFolders->Controls->Add(this->btnMoveUp);
            this->grpSelectedFolders->Controls->Add(this->btnMoveDown);
            this->grpSelectedFolders->Controls->Add(this->lstSelectedFolders);
            this->grpSelectedFolders->Location = System::Drawing::Point(343, 12);
            this->grpSelectedFolders->Name = L"grpSelectedFolders";
            this->grpSelectedFolders->Size = System::Drawing::Size(281, 293);
            this->grpSelectedFolders->TabIndex = 2;
            this->grpSelectedFolders->TabStop = false;
            this->grpSelectedFolders->Text = L"Selected folders";
            // 
            // btnMoveUp
            // 
            this->btnMoveUp->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnMoveUp.Image")));
            this->btnMoveUp->Location = System::Drawing::Point(242, 24);
            this->btnMoveUp->Name = L"btnMoveUp";
            this->btnMoveUp->Size = System::Drawing::Size(32, 32);
            this->btnMoveUp->TabIndex = 6;
            this->btnMoveUp->UseVisualStyleBackColor = true;
            this->btnMoveUp->Click += gcnew System::EventHandler(this, &AddFolderUI::btnMoveUp_Click);
            // 
            // btnMoveDown
            // 
            this->btnMoveDown->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnMoveDown.Image")));
            this->btnMoveDown->Location = System::Drawing::Point(242, 62);
            this->btnMoveDown->Name = L"btnMoveDown";
            this->btnMoveDown->Size = System::Drawing::Size(32, 32);
            this->btnMoveDown->TabIndex = 5;
            this->btnMoveDown->UseVisualStyleBackColor = true;
            this->btnMoveDown->Click += gcnew System::EventHandler(this, &AddFolderUI::btnMoveDown_Click);
            // 
            // lstSelectedFolders
            // 
            this->lstSelectedFolders->FormattingEnabled = true;
            this->lstSelectedFolders->HorizontalScrollbar = true;
            this->lstSelectedFolders->Location = System::Drawing::Point(6, 19);
            this->lstSelectedFolders->Name = L"lstSelectedFolders";
            this->lstSelectedFolders->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
            this->lstSelectedFolders->Size = System::Drawing::Size(230, 264);
            this->lstSelectedFolders->TabIndex = 0;
            this->lstSelectedFolders->SelectedIndexChanged += gcnew System::EventHandler(this, &AddFolderUI::lstSelectedFolders_SelectedIndexChanged);
            // 
            // btnRemove
            // 
            this->btnRemove->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnRemove.Image")));
            this->btnRemove->Location = System::Drawing::Point(305, 158);
            this->btnRemove->Name = L"btnRemove";
            this->btnRemove->Size = System::Drawing::Size(32, 32);
            this->btnRemove->TabIndex = 3;
            this->btnRemove->UseVisualStyleBackColor = true;
            this->btnRemove->Click += gcnew System::EventHandler(this, &AddFolderUI::btnRemove_Click);
            // 
            // btnAdd
            // 
            this->btnAdd->FlatAppearance->BorderSize = 0;
            this->btnAdd->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->btnAdd->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAdd.Image")));
            this->btnAdd->Location = System::Drawing::Point(305, 120);
            this->btnAdd->Name = L"btnAdd";
            this->btnAdd->Size = System::Drawing::Size(32, 32);
            this->btnAdd->TabIndex = 4;
            this->btnAdd->UseVisualStyleBackColor = true;
            this->btnAdd->Click += gcnew System::EventHandler(this, &AddFolderUI::btnAdd_Click);
            // 
            // btnAccept
            // 
            this->btnAccept->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAccept.Image")));
            this->btnAccept->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnAccept->Location = System::Drawing::Point(402, 311);
            this->btnAccept->Name = L"btnAccept";
            this->btnAccept->Size = System::Drawing::Size(108, 32);
            this->btnAccept->TabIndex = 5;
            this->btnAccept->Text = L"Accept";
            this->btnAccept->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->btnAccept->UseVisualStyleBackColor = true;
            this->btnAccept->Click += gcnew System::EventHandler(this, &AddFolderUI::btnAccept_Click);
            // 
            // btnCancel
            // 
            this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->btnCancel->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnCancel.Image")));
            this->btnCancel->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnCancel->Location = System::Drawing::Point(516, 311);
            this->btnCancel->Name = L"btnCancel";
            this->btnCancel->Size = System::Drawing::Size(108, 32);
            this->btnCancel->TabIndex = 6;
            this->btnCancel->Text = L"Cancel";
            this->btnCancel->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->btnCancel->UseVisualStyleBackColor = true;
            this->btnCancel->Click += gcnew System::EventHandler(this, &AddFolderUI::btnCancel_Click);
            // 
            // AddFolderUI
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->CancelButton = this->btnCancel;
            this->ClientSize = System::Drawing::Size(638, 353);
            this->Controls->Add(this->btnCancel);
            this->Controls->Add(this->btnAccept);
            this->Controls->Add(this->btnAdd);
            this->Controls->Add(this->btnRemove);
            this->Controls->Add(this->grpSelectedFolders);
            this->Controls->Add(this->grpAvailableFolders);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"AddFolderUI";
            this->ShowInTaskbar = false;
            this->Text = L"Choose Search Folders";
            this->Load += gcnew System::EventHandler(this, &AddFolderUI::AddFolderUI_Load);
            this->Closing += gcnew System::ComponentModel::CancelEventHandler(this, &AddFolderUI::AddFolderUI_Closing);
            this->grpAvailableFolders->ResumeLayout(false);
            this->grpSelectedFolders->ResumeLayout(false);
            this->ResumeLayout(false);

        }
#pragma endregion

#pragma region Windows Forms Events

private:
    System::Void AddFolderUI_Load(System::Object^  sender, System::EventArgs^  e);
    System::Void AddFolderUI_Closing(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e);
    System::Void btnFolderUp_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnAdd_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnRemove_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnMoveUp_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnMoveDown_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnAccept_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e);
    System::Void lstSelectedFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
    System::Void lstFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
    System::Void lstFolders_DoubleClick(System::Object^ sender, System::EventArgs^ e);
    System::Void cboFolders_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
    System::Void cboFolders_KeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
#pragma endregion

#pragma region Subroutines

public:
    System::Void                        LoadBrowsingHistory(System::Collections::ArrayList^ history);
    System::Void                        LoadSearchSources(System::Collections::ArrayList^ sources);
    System::Collections::ArrayList^     GetBrowsingHistory();
    System::Collections::ArrayList^     GetSearchSources();

private:
    System::Void    OpenDirectory(System::String^ directory);
    System::Void    OpenMyComputer(System::String^ directory);
    System::Void    OpenMyNetworkPlaces(System::String^ directory);
    System::Void    OpenNetworkComputer(System::String^ computerName);
    System::Void    OpenLocalDirectory(System::String^ directory);
#pragma endregion
};
}
