/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/OptionsUI.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

namespace ImageHunter {

	/// <summary>
	/// Summary for OptionsUI
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class OptionsUI : public System::Windows::Forms::Form
	{
	public:
		OptionsUI(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
            m_IsSendToMenuDisplayed = true;
            m_ExcludedExtensions = L"";
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~OptionsUI()
		{
			if (components)
			{
				delete components;
			}
		}

    private: System::String^ m_ExcludedExtensions;
    private: bool m_IsSendToMenuDisplayed;

    private: System::Windows::Forms::Label^  label1;
    private: System::Windows::Forms::TextBox^  txtExcludedExtensions;
    private: System::Windows::Forms::CheckBox^  chkDisplaySendTo;
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::Button^  btnCancel;
    private: System::Windows::Forms::Button^  btnAccept;
    private: System::Windows::Forms::GroupBox^  groupBox1;


    protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(OptionsUI::typeid));
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->txtExcludedExtensions = (gcnew System::Windows::Forms::TextBox());
            this->chkDisplaySendTo = (gcnew System::Windows::Forms::CheckBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->btnCancel = (gcnew System::Windows::Forms::Button());
            this->btnAccept = (gcnew System::Windows::Forms::Button());
            this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
            this->groupBox1->SuspendLayout();
            this->SuspendLayout();
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(21, 35);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(221, 13);
            this->label1->TabIndex = 0;
            this->label1->Text = L"Exclude the following extensions from search:";
            // 
            // txtExcludedExtensions
            // 
            this->txtExcludedExtensions->Location = System::Drawing::Point(24, 52);
            this->txtExcludedExtensions->Name = L"txtExcludedExtensions";
            this->txtExcludedExtensions->Size = System::Drawing::Size(395, 20);
            this->txtExcludedExtensions->TabIndex = 1;
            this->txtExcludedExtensions->Text = L"exe;ini;";
            // 
            // chkDisplaySendTo
            // 
            this->chkDisplaySendTo->AutoSize = true;
            this->chkDisplaySendTo->Location = System::Drawing::Point(24, 122);
            this->chkDisplaySendTo->Name = L"chkDisplaySendTo";
            this->chkDisplaySendTo->Size = System::Drawing::Size(259, 17);
            this->chkDisplaySendTo->TabIndex = 2;
            this->chkDisplaySendTo->Text = L"Display SendTo menu in the results context menu";
            this->chkDisplaySendTo->UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(0)));
            this->label2->Location = System::Drawing::Point(21, 75);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(200, 13);
            this->label2->TabIndex = 3;
            this->label2->Text = L"Separate each value by a semicolon ( ; ).";
            // 
            // btnCancel
            // 
            this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->btnCancel->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnCancel.Image")));
            this->btnCancel->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnCancel->Location = System::Drawing::Point(332, 195);
            this->btnCancel->Name = L"btnCancel";
            this->btnCancel->Size = System::Drawing::Size(108, 32);
            this->btnCancel->TabIndex = 8;
            this->btnCancel->Text = L"Cancel";
            this->btnCancel->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->btnCancel->UseVisualStyleBackColor = true;
            this->btnCancel->Click += gcnew System::EventHandler(this, &OptionsUI::btnCancel_Click);
            // 
            // btnAccept
            // 
            this->btnAccept->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnAccept.Image")));
            this->btnAccept->ImageAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->btnAccept->Location = System::Drawing::Point(218, 195);
            this->btnAccept->Name = L"btnAccept";
            this->btnAccept->Size = System::Drawing::Size(108, 32);
            this->btnAccept->TabIndex = 7;
            this->btnAccept->Text = L"Accept";
            this->btnAccept->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
            this->btnAccept->UseVisualStyleBackColor = true;
            this->btnAccept->Click += gcnew System::EventHandler(this, &OptionsUI::btnAccept_Click);
            // 
            // groupBox1
            // 
            this->groupBox1->Controls->Add(this->label2);
            this->groupBox1->Controls->Add(this->chkDisplaySendTo);
            this->groupBox1->Controls->Add(this->txtExcludedExtensions);
            this->groupBox1->Controls->Add(this->label1);
            this->groupBox1->Location = System::Drawing::Point(13, 13);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(427, 176);
            this->groupBox1->TabIndex = 9;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"Application preferences";
            // 
            // OptionsUI
            // 
            this->AcceptButton = this->btnAccept;
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->CancelButton = this->btnCancel;
            this->ClientSize = System::Drawing::Size(452, 239);
            this->Controls->Add(this->groupBox1);
            this->Controls->Add(this->btnCancel);
            this->Controls->Add(this->btnAccept);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"OptionsUI";
            this->ShowInTaskbar = false;
            this->Text = L"Options";
            this->Load += gcnew System::EventHandler(this, &OptionsUI::OptionsUI_Load);
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion


#pragma region Windows Form Event

    private:
        System::Void btnAccept_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void OptionsUI_Load(System::Object^  sender, System::EventArgs^  e);

#pragma endregion

#pragma region Subroutines

    public:
        System::Void        SetExcludedExtensions(System::String^);
        System::String^     GetExcludedExtensions();
        System::Void        SetIsSendToMenuDisplayed(bool isChecked);
        System::Boolean     IsSendToMenuDisplayed();

#pragma endregion
};
}
