/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ExternalToolsUI.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ExternalToolsUI.h,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ExternalToolsUI
//-----------------------------------------------------------------------------
// Windows Form to configure the available external tools in the results list.
//-----------------------------------------------------------------------------

#pragma once

#include "ExternalTools.h"

namespace ImageHunter {

	/// <summary>
	/// Summary for ExternalToolsUI
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class ExternalToolsUI : public System::Windows::Forms::Form
	{
	public:
		ExternalToolsUI(void)
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
		~ExternalToolsUI()
		{
			if (components)
			{
				delete components;
			}
		}

    protected: 

    private: System::Windows::Forms::Button^            btnAdd;
    private: System::Windows::Forms::Button^            btnMoveUp;
    private: System::Windows::Forms::Button^            btnMoveDown;
    private: System::Windows::Forms::Button^            btnRemove;
    private: System::Windows::Forms::Label^             lblName;
    private: System::Windows::Forms::Label^             lblCommand;
    private: System::Windows::Forms::Label^             lblArguments;
    private: System::Windows::Forms::TextBox^           txtName;
    private: System::Windows::Forms::TextBox^           txtArguments;
    private: System::Windows::Forms::TextBox^           txtCommand;
    private: System::Windows::Forms::GroupBox^          grpExternalTools;
    private: System::Windows::Forms::GroupBox^          grpAddModifyItem;
    private: System::Windows::Forms::Button^            btnSave;
    private: System::Windows::Forms::Button^            btnCancel;
    private: System::Windows::Forms::Button^            btnCommandBrowse;
    private: System::Windows::Forms::OpenFileDialog^    OpenCommandDialog;
    private: System::Windows::Forms::ListBox^           lstExternalTools;
    private: System::Collections::SortedList^           externalTools;
    private: System::Windows::Forms::PictureBox^        pictureBox1;
    private: System::Windows::Forms::ToolTip^           toolTip1;
    private: System::ComponentModel::IContainer^        components;
    private: System::Windows::Forms::ToolTip^           ToolTip;
    private: System::Windows::Forms::PictureBox^        picArgsInfo;
    private: System::Windows::Forms::Label^             label2;


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(ExternalToolsUI::typeid));
            this->btnAdd = (gcnew System::Windows::Forms::Button());
            this->btnMoveUp = (gcnew System::Windows::Forms::Button());
            this->btnMoveDown = (gcnew System::Windows::Forms::Button());
            this->btnRemove = (gcnew System::Windows::Forms::Button());
            this->lblName = (gcnew System::Windows::Forms::Label());
            this->lblCommand = (gcnew System::Windows::Forms::Label());
            this->lblArguments = (gcnew System::Windows::Forms::Label());
            this->txtName = (gcnew System::Windows::Forms::TextBox());
            this->txtArguments = (gcnew System::Windows::Forms::TextBox());
            this->txtCommand = (gcnew System::Windows::Forms::TextBox());
            this->grpExternalTools = (gcnew System::Windows::Forms::GroupBox());
            this->lstExternalTools = (gcnew System::Windows::Forms::ListBox());
            this->grpAddModifyItem = (gcnew System::Windows::Forms::GroupBox());
            this->picArgsInfo = (gcnew System::Windows::Forms::PictureBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->btnCommandBrowse = (gcnew System::Windows::Forms::Button());
            this->btnSave = (gcnew System::Windows::Forms::Button());
            this->btnCancel = (gcnew System::Windows::Forms::Button());
            this->OpenCommandDialog = (gcnew System::Windows::Forms::OpenFileDialog());
            this->ToolTip = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->grpExternalTools->SuspendLayout();
            this->grpAddModifyItem->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->picArgsInfo))->BeginInit();
            this->SuspendLayout();
            // 
            // btnAdd
            // 
            this->btnAdd->FlatAppearance->BorderSize = 0;
            this->btnAdd->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnAdd->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnAdd->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAdd.Image")));
            this->btnAdd->Location = System::Drawing::Point(294, 19);
            this->btnAdd->Name = L"btnAdd";
            this->btnAdd->Size = System::Drawing::Size(25, 25);
            this->btnAdd->TabIndex = 1;
            this->ToolTip->SetToolTip(this->btnAdd, L"Add new tool");
            this->btnAdd->UseVisualStyleBackColor = true;
            this->btnAdd->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnAdd_Click);
            // 
            // btnMoveUp
            // 
            this->btnMoveUp->FlatAppearance->BorderSize = 0;
            this->btnMoveUp->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnMoveUp->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnMoveUp->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnMoveUp.Image")));
            this->btnMoveUp->Location = System::Drawing::Point(294, 98);
            this->btnMoveUp->Name = L"btnMoveUp";
            this->btnMoveUp->Size = System::Drawing::Size(25, 25);
            this->btnMoveUp->TabIndex = 3;
            this->ToolTip->SetToolTip(this->btnMoveUp, L"Move up selected tool");
            this->btnMoveUp->UseVisualStyleBackColor = true;
            this->btnMoveUp->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnMoveUp_Click);
            // 
            // btnMoveDown
            // 
            this->btnMoveDown->FlatAppearance->BorderSize = 0;
            this->btnMoveDown->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnMoveDown->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnMoveDown->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnMoveDown.Image")));
            this->btnMoveDown->Location = System::Drawing::Point(294, 128);
            this->btnMoveDown->Name = L"btnMoveDown";
            this->btnMoveDown->Size = System::Drawing::Size(25, 25);
            this->btnMoveDown->TabIndex = 4;
            this->ToolTip->SetToolTip(this->btnMoveDown, L"Move down selected tool");
            this->btnMoveDown->UseVisualStyleBackColor = true;
            this->btnMoveDown->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnMoveDown_Click);
            // 
            // btnRemove
            // 
            this->btnRemove->FlatAppearance->BorderSize = 0;
            this->btnRemove->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnRemove->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnRemove->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnRemove.Image")));
            this->btnRemove->Location = System::Drawing::Point(293, 50);
            this->btnRemove->Name = L"btnRemove";
            this->btnRemove->Size = System::Drawing::Size(25, 25);
            this->btnRemove->TabIndex = 2;
            this->ToolTip->SetToolTip(this->btnRemove, L"Remove selected tool");
            this->btnRemove->UseVisualStyleBackColor = true;
            this->btnRemove->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnRemove_Click);
            // 
            // lblName
            // 
            this->lblName->AutoSize = true;
            this->lblName->Location = System::Drawing::Point(6, 32);
            this->lblName->Name = L"lblName";
            this->lblName->Size = System::Drawing::Size(35, 13);
            this->lblName->TabIndex = 0;
            this->lblName->Text = L"Name";
            // 
            // lblCommand
            // 
            this->lblCommand->AutoSize = true;
            this->lblCommand->Location = System::Drawing::Point(6, 59);
            this->lblCommand->Name = L"lblCommand";
            this->lblCommand->Size = System::Drawing::Size(54, 13);
            this->lblCommand->TabIndex = 2;
            this->lblCommand->Text = L"Command";
            // 
            // lblArguments
            // 
            this->lblArguments->AutoSize = true;
            this->lblArguments->Location = System::Drawing::Point(6, 88);
            this->lblArguments->Name = L"lblArguments";
            this->lblArguments->Size = System::Drawing::Size(57, 13);
            this->lblArguments->TabIndex = 5;
            this->lblArguments->Text = L"Arguments";
            // 
            // txtName
            // 
            this->txtName->Location = System::Drawing::Point(81, 25);
            this->txtName->Name = L"txtName";
            this->txtName->Size = System::Drawing::Size(206, 20);
            this->txtName->TabIndex = 1;
            this->txtName->TextChanged += gcnew System::EventHandler(this, &ExternalToolsUI::txtName_TextChanged);
            // 
            // txtArguments
            // 
            this->txtArguments->Location = System::Drawing::Point(81, 85);
            this->txtArguments->Name = L"txtArguments";
            this->txtArguments->Size = System::Drawing::Size(206, 20);
            this->txtArguments->TabIndex = 6;
            this->ToolTip->SetToolTip(this->txtArguments, L"Use %s to specify where to insert the filename");
            this->txtArguments->TextChanged += gcnew System::EventHandler(this, &ExternalToolsUI::txtArguments_TextChanged);
            // 
            // txtCommand
            // 
            this->txtCommand->Location = System::Drawing::Point(81, 56);
            this->txtCommand->Name = L"txtCommand";
            this->txtCommand->Size = System::Drawing::Size(206, 20);
            this->txtCommand->TabIndex = 3;
            this->txtCommand->TextChanged += gcnew System::EventHandler(this, &ExternalToolsUI::txtCommand_TextChanged);
            // 
            // grpExternalTools
            // 
            this->grpExternalTools->Controls->Add(this->lstExternalTools);
            this->grpExternalTools->Controls->Add(this->btnAdd);
            this->grpExternalTools->Controls->Add(this->btnRemove);
            this->grpExternalTools->Controls->Add(this->btnMoveUp);
            this->grpExternalTools->Controls->Add(this->btnMoveDown);
            this->grpExternalTools->Location = System::Drawing::Point(12, 14);
            this->grpExternalTools->Name = L"grpExternalTools";
            this->grpExternalTools->Size = System::Drawing::Size(326, 168);
            this->grpExternalTools->TabIndex = 0;
            this->grpExternalTools->TabStop = false;
            this->grpExternalTools->Text = L"Contextual Menu Elements";
            // 
            // lstExternalTools
            // 
            this->lstExternalTools->FormattingEnabled = true;
            this->lstExternalTools->Location = System::Drawing::Point(7, 19);
            this->lstExternalTools->Name = L"lstExternalTools";
            this->lstExternalTools->Size = System::Drawing::Size(281, 134);
            this->lstExternalTools->TabIndex = 0;
            this->lstExternalTools->SelectedIndexChanged += gcnew System::EventHandler(this, &ExternalToolsUI::lstExternalTools_SelectedIndexChanged);
            // 
            // grpAddModifyItem
            // 
            this->grpAddModifyItem->Controls->Add(this->picArgsInfo);
            this->grpAddModifyItem->Controls->Add(this->label2);
            this->grpAddModifyItem->Controls->Add(this->btnCommandBrowse);
            this->grpAddModifyItem->Controls->Add(this->txtCommand);
            this->grpAddModifyItem->Controls->Add(this->txtArguments);
            this->grpAddModifyItem->Controls->Add(this->txtName);
            this->grpAddModifyItem->Controls->Add(this->lblArguments);
            this->grpAddModifyItem->Controls->Add(this->lblCommand);
            this->grpAddModifyItem->Controls->Add(this->lblName);
            this->grpAddModifyItem->Location = System::Drawing::Point(13, 188);
            this->grpAddModifyItem->Name = L"grpAddModifyItem";
            this->grpAddModifyItem->Size = System::Drawing::Size(325, 132);
            this->grpAddModifyItem->TabIndex = 1;
            this->grpAddModifyItem->TabStop = false;
            this->grpAddModifyItem->Text = L"Tool Details";
            // 
            // picArgsInfo
            // 
            this->picArgsInfo->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"picArgsInfo.Image")));
            this->picArgsInfo->Location = System::Drawing::Point(293, 82);
            this->picArgsInfo->Name = L"picArgsInfo";
            this->picArgsInfo->Size = System::Drawing::Size(24, 24);
            this->picArgsInfo->SizeMode = System::Windows::Forms::PictureBoxSizeMode::CenterImage;
            this->picArgsInfo->TabIndex = 9;
            this->picArgsInfo->TabStop = false;
            this->ToolTip->SetToolTip(this->picArgsInfo, L"Use %s to specify where to insert the filename");
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(340, 84);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(0, 13);
            this->label2->TabIndex = 8;
            // 
            // btnCommandBrowse
            // 
            this->btnCommandBrowse->FlatAppearance->BorderSize = 0;
            this->btnCommandBrowse->FlatAppearance->MouseOverBackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->btnCommandBrowse->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnCommandBrowse->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnCommandBrowse.Image")));
            this->btnCommandBrowse->Location = System::Drawing::Point(293, 52);
            this->btnCommandBrowse->Name = L"btnCommandBrowse";
            this->btnCommandBrowse->Size = System::Drawing::Size(24, 24);
            this->btnCommandBrowse->TabIndex = 4;
            this->ToolTip->SetToolTip(this->btnCommandBrowse, L"Select the application to execute from your system files");
            this->btnCommandBrowse->UseVisualStyleBackColor = true;
            this->btnCommandBrowse->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnCommandBrowse_Click);
            // 
            // btnSave
            // 
            this->btnSave->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSave.Image")));
            this->btnSave->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnSave->Location = System::Drawing::Point(116, 326);
            this->btnSave->Name = L"btnSave";
            this->btnSave->Size = System::Drawing::Size(108, 32);
            this->btnSave->TabIndex = 2;
            this->btnSave->Text = L"Save";
            this->btnSave->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->ToolTip->SetToolTip(this->btnSave, L"Save the external tools");
            this->btnSave->UseVisualStyleBackColor = true;
            this->btnSave->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnSave_Click);
            // 
            // btnCancel
            // 
            this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->btnCancel->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnCancel.Image")));
            this->btnCancel->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnCancel->Location = System::Drawing::Point(230, 326);
            this->btnCancel->Name = L"btnCancel";
            this->btnCancel->Size = System::Drawing::Size(108, 32);
            this->btnCancel->TabIndex = 3;
            this->btnCancel->Text = L"Cancel";
            this->btnCancel->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->ToolTip->SetToolTip(this->btnCancel, L"Cancel the changes made");
            this->btnCancel->UseVisualStyleBackColor = true;
            this->btnCancel->Click += gcnew System::EventHandler(this, &ExternalToolsUI::btnCancel_Click);
            // 
            // OpenCommandDialog
            // 
            this->OpenCommandDialog->DefaultExt = L"*.exe";
            this->OpenCommandDialog->Filter = L"Executables|*.exe|Batch Files|*.bat|All files|*.*";
            this->OpenCommandDialog->Title = L"Select an executable to launch as an external tool";
            // 
            // ExternalToolsUI
            // 
            this->AcceptButton = this->btnSave;
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->CancelButton = this->btnCancel;
            this->ClientSize = System::Drawing::Size(350, 369);
            this->Controls->Add(this->btnSave);
            this->Controls->Add(this->btnCancel);
            this->Controls->Add(this->grpAddModifyItem);
            this->Controls->Add(this->grpExternalTools);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"ExternalToolsUI";
            this->ShowInTaskbar = false;
            this->Text = L"External Tools";
            this->Load += gcnew System::EventHandler(this, &ExternalToolsUI::ExternalToolsUI_Load);
            this->grpExternalTools->ResumeLayout(false);
            this->grpAddModifyItem->ResumeLayout(false);
            this->grpAddModifyItem->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->picArgsInfo))->EndInit();
            this->ResumeLayout(false);

        }
#pragma endregion


#pragma region Windows Form Events
    private:
        System::Void ExternalToolsUI_Load(System::Object^  sender, System::EventArgs^  e);
        System::Void lstExternalTools_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnSave_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnRemove_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnAdd_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnMoveUp_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnMoveDown_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnCommandBrowse_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void txtArguments_TextChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void txtName_TextChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void txtCommand_TextChanged(System::Object^  sender, System::EventArgs^  e);
        

#pragma endregion


#pragma region Subroutines
    private: System::Void DisableControls()
    {
        // Disabling all controls
        btnRemove->Enabled = false;
        btnMoveUp->Enabled = false;
        btnMoveDown->Enabled = false;
        txtArguments->Enabled = false;
        txtArguments->Text = "";
        txtCommand->Enabled = false;
        txtCommand->Text = "";
        btnCommandBrowse->Enabled = false;
        txtName->Enabled = false;
        txtName->Text = "";
    }

    private: System::Void EnableControls()
    {
        // Enabling all controls
        btnRemove->Enabled = true;
        btnMoveUp->Enabled = true;
        btnMoveDown->Enabled = true;
        txtArguments->Enabled = true;
        txtCommand->Enabled = true;
        btnCommandBrowse->Enabled = true;
        txtName->Enabled = true;
    }

#pragma endregion

};
}
