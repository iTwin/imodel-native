/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/LandXMLImportForm.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include    "DataGridViewElementTemplateCell.h"

BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

namespace BUIA = Bentley::UI::Attributes;

/// <summary>
/// Summary for LandXMLImportForm
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
/// </summary>
[BUIA::DialogInformation (OpenKeyin = "TerrainModel import landxml @@", CloseKeyin = "", EnglishTitle = "landXML Import", FeatureTrackingId = "45FEAB96-50A7-4A0B-A1E8-54B7C27F4862",
                          SourceFile = "$Source: ElementHandler/Commands/LandXMLImportForm.h $")]
public ref class LandXMLImportForm : public System::Windows::Forms::Form
{

public: void grid_CellValueChanged ( System::Object ^sender, System::Windows::Forms::DataGridViewCellEventArgs ^e );
public: void grid_CellBeginEdit( System::Object^ sender, System::Windows::Forms::DataGridViewCellCancelEventArgs^ e );
public: void grid_CellEndEdit ( System::Object ^sender, System::Windows::Forms::DataGridViewCellEventArgs ^e );
private: void grid_EditedFormattedValueChanged (System::Windows::Forms::DataGridViewCheckBoxCell ^sender);

public: LandXMLImportForm ( array<System::String^>^ surfaces );

protected:
    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    ~LandXMLImportForm ( void ) { delete components; }

protected: 


private: System::Windows::Forms::Label^  m_label;
private: System::Windows::Forms::DataGridView^  m_dataGridSurfaces;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  colSurfaceCB;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  colSurface;
private: System::Windows::Forms::DataGridViewComboBoxColumn^  colElmTemplate;

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
    System::Windows::Forms::Button^  m_butImport;
    System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(LandXMLImportForm::typeid));
    System::Windows::Forms::Button^  m_butCancel;
    this->m_label = (gcnew System::Windows::Forms::Label());
    this->m_dataGridSurfaces = (gcnew System::Windows::Forms::DataGridView());
    this->colSurfaceCB = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
    this->colSurface = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
    this->colElmTemplate = (gcnew System::Windows::Forms::DataGridViewComboBoxColumn());
    m_butImport = (gcnew System::Windows::Forms::Button());
    m_butCancel = (gcnew System::Windows::Forms::Button());
    (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_dataGridSurfaces))->BeginInit();
    this->SuspendLayout();
    // 
    // m_butImport
    // 
    resources->ApplyResources(m_butImport, L"m_butImport");
    m_butImport->DialogResult = System::Windows::Forms::DialogResult::OK;
    m_butImport->Name = L"m_butImport";
    m_butImport->UseVisualStyleBackColor = true;
    // 
    // m_butCancel
    // 
    resources->ApplyResources(m_butCancel, L"m_butCancel");
    m_butCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
    m_butCancel->Name = L"m_butCancel";
    m_butCancel->UseVisualStyleBackColor = true;
    // 
    // m_label
    // 
    resources->ApplyResources(this->m_label, L"m_label");
    this->m_label->Name = L"m_label";
    // 
    // m_dataGridSurfaces
    // 
    this->m_dataGridSurfaces->AllowUserToAddRows = false;
    this->m_dataGridSurfaces->AllowUserToDeleteRows = false;
    resources->ApplyResources(this->m_dataGridSurfaces, L"m_dataGridSurfaces");
    this->m_dataGridSurfaces->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
    this->m_dataGridSurfaces->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(3) {this->colSurfaceCB, 
        this->colSurface, this->colElmTemplate});
    this->m_dataGridSurfaces->Name = L"m_dataGridSurfaces";
    this->m_dataGridSurfaces->RowHeadersVisible = false;
    this->m_dataGridSurfaces->DataError += gcnew DataGridViewDataErrorEventHandler (this, &LandXMLImportForm::DataGridView_DataError);
    // 
    // colSurfaceCB
    // 
    this->colSurfaceCB->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::ColumnHeader;
    resources->ApplyResources(this->colSurfaceCB, L"colSurfaceCB");
    this->colSurfaceCB->Name = L"colSurfaceCB";
    this->colSurfaceCB->Resizable = System::Windows::Forms::DataGridViewTriState::True;
    // 
    // colSurface
    // 
    resources->ApplyResources(this->colSurface, L"colSurface");
    this->colSurface->Name = L"colSurface";
    // 
    // colElmTemplate
    // 
    this->colElmTemplate->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
    this->colElmTemplate->DisplayStyle = System::Windows::Forms::DataGridViewComboBoxDisplayStyle::ComboBox;
    resources->ApplyResources(this->colElmTemplate, L"colElmTemplate");
    this->colElmTemplate->Name = L"colElmTemplate";
    this->colElmTemplate->Resizable = System::Windows::Forms::DataGridViewTriState::True;
    this->colElmTemplate->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::Automatic;
    // 
    // LandXMLImportForm
    // 
    this->AcceptButton = m_butImport;
    this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
    resources->ApplyResources(this, L"$this");
    this->CancelButton = m_butCancel;
    this->Controls->Add(m_butCancel);
    this->Controls->Add(m_butImport);
    this->Controls->Add(this->m_dataGridSurfaces);
    this->Controls->Add(this->m_label);
    this->MaximizeBox = false;
    this->MinimizeBox = false;
    this->Name = L"LandXMLImportForm";
    this->ShowInTaskbar = false;
    this->Load += gcnew System::EventHandler(this, &LandXMLImportForm::LandXMLImportForm_Load);
    (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->m_dataGridSurfaces))->EndInit();
    this->ResumeLayout(false);
    this->PerformLayout();

        }
#pragma endregion

private:  System::Void LandXMLImportForm_Load (System::Object^  sender, System::EventArgs^  e) 
        {}

private:  void DataGridView_DataError (Object^ sender, DataGridViewDataErrorEventArgs^ anError);

    /// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
    internal: ref class SurfaceData
    {
    private: String     ^m_surface;
    private: String     ^m_original_surface;
    private: String     ^m_elementTemplate;
    public: SurfaceData ( String ^surface, String ^originalSurface, String ^elementTemplate ) : m_surface ( surface ), m_original_surface ( originalSurface ), m_elementTemplate ( elementTemplate )
        {}
    public: property String^ Surface
        {
        String^ get ( void ) { return m_surface; }
        }
    public: property String^ ElementTemplate
        {
        String^ get ( void ) { return m_elementTemplate; }
        }
    public: property String^ OriginalSurface
        {
        String^ get ( void ) { return m_original_surface; }
        }

    }; // End Surf class

internal: property array<SurfaceData^>^ SelectedSurfaces
        {
        array<SurfaceData^>^ get ( void );
        }

}; // End LandXMLImportForm class

END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE
