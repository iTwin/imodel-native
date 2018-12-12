/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/LandXMLImportForm.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"
#include    "LandXMLImportForm.h"

#define     COL_CB                  0
#define     COL_Name                1
#define     COL_ElementTemplate     2

#define     THREE_STATE             (true)

using namespace System::Windows::Forms;

namespace BMG = Bentley::MstnPlatformNET::GUI;

BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
ref class MyDataGridViewCheckBoxCell : DataGridViewCheckBoxCell
{
internal: MyDataGridViewCheckBoxCell (bool threeState) : DataGridViewCheckBoxCell (threeState)
    {}

internal: delegate void EditedFormattedValueChangedEventHandler (DataGridViewCheckBoxCell^ sender);

internal: event EditedFormattedValueChangedEventHandler^ EditedFormattedValueChangedEvent;

public: virtual void OnContentClick (DataGridViewCellEventArgs ^e) override
    {
    bool val = safe_cast<bool>(EditedFormattedValue);
    __super::OnContentClick (e);
    if (safe_cast<bool>(EditedFormattedValue) != val)
        {
        EditedFormattedValueChangedEvent (this);
        }
    }

}; // End MyDataGridViewCheckBoxCell class

LandXMLImportForm::LandXMLImportForm (array<System::String^>^ surfaces)
    {
    InitializeComponent ();
    colSurfaceCB->HeaderText = GET_LOCALIZED ("LABEL_Import");
    colSurface->HeaderText = GET_LOCALIZED ("LABEL_Surface");
    colElmTemplate->HeaderText = GET_LOCALIZED ("LABEL_ElementTemplate");
    m_dataGridSurfaces->CellValueChanged += gcnew DataGridViewCellEventHandler ( this, &LandXMLImportForm::grid_CellValueChanged );
    m_dataGridSurfaces->CellBeginEdit += gcnew DataGridViewCellCancelEventHandler ( this, &LandXMLImportForm::grid_CellBeginEdit );
    m_dataGridSurfaces->CellEndEdit += gcnew DataGridViewCellEventHandler ( this, &LandXMLImportForm::grid_CellEndEdit );
    for each ( System::String^ surface in surfaces )
        {
        DataGridViewRow ^surfaceRow = gcnew DataGridViewRow ();
        MyDataGridViewCheckBoxCell ^surfaceCell = gcnew MyDataGridViewCheckBoxCell (!THREE_STATE);
        surfaceCell->EditedFormattedValueChangedEvent += gcnew MyDataGridViewCheckBoxCell::EditedFormattedValueChangedEventHandler (this, &LandXMLImportForm::grid_EditedFormattedValueChanged);
        DataGridViewTextBoxCell ^surfaceCellName = gcnew DataGridViewTextBoxCell ();
        DataGridViewElementTemplateCell ^elmTemplateCell = gcnew DataGridViewElementTemplateCell ();

        System::String ^active = GET_LOCALIZED ("ITEM_Active");
        elmTemplateCell->Items->Add (active);
        elmTemplateCell->Value = active;

        surfaceCell->Value = true;
        surfaceCellName->Value = surface;
        surfaceCellName->Tag = surface;
        surfaceRow->Cells->Add ( surfaceCell );
        surfaceRow->Cells->Add ( surfaceCellName );
        surfaceRow->Cells->Add ( elmTemplateCell );
        m_dataGridSurfaces->Rows->Add ( surfaceRow );
        }

    // Get DialogInformation and track using the FeatureTrackingId
    BMG::DialogInformation::FeatureTrackOpen (this);
    }

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
void LandXMLImportForm::grid_EditedFormattedValueChanged (DataGridViewCheckBoxCell ^sender)
    {
    Button ^ok = safe_cast<Button^>(AcceptButton);
    bool somethingIsChecked = false;

    for (int i = 0; i < m_dataGridSurfaces->RowCount; ++i)
        {
        if (i != sender->RowIndex && safe_cast<bool>(m_dataGridSurfaces->Rows[i]->Cells[sender->ColumnIndex]->EditedFormattedValue))
            {
            somethingIsChecked = true;
            break;
            }
        }
    ok->Enabled = somethingIsChecked || safe_cast<bool>(sender->EditedFormattedValue);
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void LandXMLImportForm::grid_CellValueChanged ( System::Object ^sender, System::Windows::Forms::DataGridViewCellEventArgs ^e )
    {
    if ( e->ColumnIndex != COL_ElementTemplate )
        return;
//    DataGridViewElementTemplateCell ^elmTemplateCell = safe_cast<DataGridViewElementTemplateCell ^>( m_dataGridSurfaces->Rows[e->RowIndex]->Cells[e->ColumnIndex] );
    //if ( !elmTemplateCell->Items->Contains ( "PMS" ) )
    //    elmTemplateCell->Items->Add ( "PMS" );
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void LandXMLImportForm::grid_CellBeginEdit( System::Object^ sender, System::Windows::Forms::DataGridViewCellCancelEventArgs ^e )
    {
    if ( e->ColumnIndex != COL_ElementTemplate )
        return;
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void LandXMLImportForm::grid_CellEndEdit ( System::Object ^sender, System::Windows::Forms::DataGridViewCellEventArgs ^e )
    {
    if ( e->ColumnIndex != COL_ElementTemplate )
        return;
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
array<LandXMLImportForm::SurfaceData^>^ LandXMLImportForm::SelectedSurfaces::get ( void )
    {
    System::Collections::Generic::List<SurfaceData^> list;

    for each (  DataGridViewRow ^row in m_dataGridSurfaces->Rows )
        {
        DataGridViewCheckBoxCell ^surfaceCell = safe_cast<DataGridViewCheckBoxCell^>( row->Cells[COL_CB] );
        if ( static_cast<bool>( surfaceCell->Value ) )
            {
            DataGridViewTextBoxCell ^surfaceCellName = safe_cast<DataGridViewTextBoxCell^> ( row->Cells[COL_Name] );
            DataGridViewElementTemplateCell ^surfElmTeamplate = safe_cast<DataGridViewElementTemplateCell^> ( row->Cells[COL_ElementTemplate] );
            list.Add ( gcnew SurfaceData ( safe_cast<String ^>( surfaceCellName->FormattedValue ), safe_cast<String ^>( surfaceCellName->Tag ), safe_cast<String ^>( surfElmTeamplate->FormattedValue ) ) );
            }
        }
    return list.ToArray ();
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void LandXMLImportForm::DataGridView_DataError (Object^ sender, DataGridViewDataErrorEventArgs^ anError)
    {}

END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE
