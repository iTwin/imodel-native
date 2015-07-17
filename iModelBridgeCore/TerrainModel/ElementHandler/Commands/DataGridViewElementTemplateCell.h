/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/DataGridViewElementTemplateCell.h $
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
using namespace System::Globalization;

#include    "DataGridViewElementTemplateCellEditingControl.h"

BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
ref class DataGridViewElementTemplateCell : public DataGridViewComboBoxCell
{
public: enum class ELMTEMPLSelection { SEL_None, SEL_Active, SEL_Custom, };

public: property ELMTEMPLSelection Selection
        {
        ELMTEMPLSelection get ( void ) { return m_selection; }
        void set ( ELMTEMPLSelection sel ) { m_selection = sel; }
        }

private: ELMTEMPLSelection  m_selection;

public: DataGridViewElementTemplateCell ( void ) : m_selection ( ELMTEMPLSelection::SEL_None )
        {
//#ifdef DEBUG_USE_COMBO_CONTROL
//        Items->Add ( "<None>" );
//        Items->Add ( "<Active>" );
//        Items->Add ( "<Select ...>" );
//#endif
        }

    // Override the Clone method so that the Enabled property is copied.
public: virtual System::Object^ Clone ( void ) override
        {
        DataGridViewElementTemplateCell^ cell = safe_cast<DataGridViewElementTemplateCell^>( __super::Clone() );
        cell->m_selection = m_selection;
        return cell;
        }

public: virtual property System::Type^ EditType
        {
        System::Type^ get ( void ) override
            {
            System::Type ^t = DataGridViewElementTemplateCellEditingControl::typeid;
            //System::Type ^t = __super::EditType;
            return t;
            }
        }

public: virtual String^ ToString ( void ) override
        {
        return "DataGridViewElementTemplateCellEditingControl { ColumnIndex=" + ColumnIndex.ToString ( CultureInfo::CurrentCulture) + \
            ",  RowIndex=" + RowIndex.ToString ( CultureInfo::CurrentCulture ) + " }";
        }

public: virtual void InitializeEditingControl ( int rowIndex, Object ^initialFormattedValue, DataGridViewCellStyle ^dataGridViewCellStyle ) override
        {
        __super::InitializeEditingControl ( rowIndex, initialFormattedValue, dataGridViewCellStyle );
        DataGridViewElementTemplateCellEditingControl^ control = safe_cast<DataGridViewElementTemplateCellEditingControl^>( DataGridView->EditingControl );
#ifndef DEBUG_USE_COMBO_CONTROL
        //if ( !control->DroppedDown )
        //    control->DroppedDown = true;
        //control->Location = control->EditingControlDataGridView->CurrentCell->Location;
#endif
        System::String^ value = (System::String^)initialFormattedValue;
        System::Collections::Generic::Queue<TreeNodeCollection^>^ queue = gcnew System::Collections::Generic::Queue<TreeNodeCollection^> ();
        TreeNode^ selectedNode = nullptr;
        queue->Enqueue (control->Nodes);

        while (selectedNode == nullptr && queue->Count)
            {
            for each (TreeNode^ node in queue->Dequeue ())
                {
                if (node->FullPath == value)
                    {
                    selectedNode = node;
                    break;
                    }
                TreeNodeCollection^ nodes = node->Nodes;
                if (nodes != nullptr && nodes->Count != 0)
                    queue->Enqueue (node->Nodes);
                }
            }
        control->SelectedNode = selectedNode;
        }


//protected: virtual void OnDataGridViewChanged ( void ) override
//    {
////    if ( nullptr != Value )
////        {
////        System::String ^val = safe_cast<System::String^>( Value );
////        }
//    __super:: OnDataGridViewChanged ();
//    }
//
//public: virtual void InitializeEditingControl ( int rowIndex, Object^ initialFormattedValue, DataGridViewCellStyle^ dataGridViewCellStyle ) override
//    {
//    return __super::InitializeEditingControl ( rowIndex, initialFormattedValue, dataGridViewCellStyle );
//    }
//
//public: virtual void DetachEditingControl ( void ) override
//    {
//    return __super::DetachEditingControl ();
//    }
//
//
//protected: virtual void OnMouseClick ( System::Windows::Forms::DataGridViewCellMouseEventArgs ^e ) override
//    {
//    return __super::OnMouseClick ( e );
//    }

public: virtual void PositionEditingControl ( bool setLocation, bool setSize, System::Drawing::Rectangle cellBounds, System::Drawing::Rectangle cellClip, DataGridViewCellStyle ^cellStyle, bool singleVerticalBorderAdded, bool singleHorizontalBorderAdded, bool isFirstDisplayedColumn, bool isFirstDisplayedRow ) override
        {
#ifdef GUI_DEBUG
        System::Drawing::Rectangle              clientRect  = DataGridView->ClientRectangle;
#endif
        System::Drawing::Size                   clientSize  = DataGridView->ClientSize;
        cellBounds.Height *= 5;
        cellClip.Height *= 5;
        if ( cellBounds.Bottom > clientSize.Height )
            {
            int offset = clientSize.Height - cellBounds.Bottom;
            if ( cellBounds.Top + offset < 0 )
                offset -= (cellBounds.Top + offset);
            cellBounds.Offset ( 0, offset );
            cellClip.Offset ( 0, offset );
            }
        if ( cellBounds.Right > clientSize.Width )
            {
            int offset = clientSize.Width - cellBounds.Right;
            if ( cellBounds.Left + offset < 0 )
                offset -= (cellBounds.Left + offset);
            cellBounds.Offset ( offset, 0 );
            cellClip.Offset ( offset, 0 );
            }
        __super::PositionEditingControl ( setLocation, setSize, cellBounds, cellClip, cellStyle, singleVerticalBorderAdded, singleHorizontalBorderAdded, isFirstDisplayedColumn, isFirstDisplayedRow );
        }

}; // End DataGridViewElementTemplateCell class

END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE
