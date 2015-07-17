/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/DataGridViewElementTemplateCellEditingControl.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace Bentley::MstnPlatformNET::Templates::Support;

//#define DEBUG_USE_COMBO_CONTROL

#ifdef DEBUG_USE_COMBO_CONTROL
public ref class DataGridViewElementTemplateCellEditingControl : public DataGridViewComboBoxEditingControl
#else
public ref class DataGridViewElementTemplateCellEditingControl : public TreeView, public IDataGridViewEditingControl
//public ref class DataGridViewElementTemplateCellEditingControl : public IDataGridViewEditingControl
//public ref class DataGridViewElementTemplateCellEditingControl : public DataGridViewComboBoxEditingControl
#endif
{

#pragma region " Variables "

private: String         ^m_templatePath;
private: bool           m_isDroppingDown;
private: bool           m_focusOnMouseLeave;
private: bool           m_focusOnVisibleChanged;
#ifdef DEBUG_USE_COMBO_CONTROL
protected: System::Windows::Forms::TreeView     ^m_treeView;
#else
//private: DataGridViewComboBoxEditingControl ^m_control;
private: DataGridView   ^m_vw;
private: int            m_rowIndex;
private: bool           m_valueChanged;
#endif

#pragma endregion " Variables "

#pragma region " Constructors "

public: DataGridViewElementTemplateCellEditingControl ( void );

#pragma endregion " Constructors "

#pragma region " IDataGridViewEditingControl "

#ifdef DEBUG_USE_COMBO_CONTROL
    public: virtual void PrepareEditingControlForEdit ( bool selectAll ) override;
#else
    public: virtual void PrepareEditingControlForEdit ( bool selectAll );
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
    public: virtual property DataGridView^ EditingControlDataGridView
            {
            virtual DataGridView^ get ( void ) override;
            virtual void set ( DataGridView ^vw ) override;
            }
#else
    public: property DataGridView^ EditingControlDataGridView
            {
            virtual DataGridView^ get ( void );
            virtual void set ( DataGridView ^vw );
            }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
    public: virtual property int EditingControlRowIndex
            {
            virtual void set( int row ) override { __super::EditingControlRowIndex = row; }
            virtual int get( void ) override
                {
                int rowIndex = __super::EditingControlRowIndex;
                return rowIndex;
                }
            }
#else
    public: property int EditingControlRowIndex
            {
            virtual void set( int row ) { m_rowIndex = row; }
            virtual int get( void ) { return m_rowIndex; }
            }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
public: virtual property Object^ EditingControlFormattedValue
        {
        virtual Object^ get ( void ) override;
        virtual void set ( Object ^o ) override;
        }
#else
public: property Object^ EditingControlFormattedValue
        {
        virtual Object^ get ( void );
        virtual void set ( System::Object ^o );
        }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
public: virtual property bool EditingControlValueChanged
        {
        virtual void set ( bool value ) override;
        virtual bool get ( void ) override
            {
            bool value = __super::EditingControlValueChanged;
            return value;
            }
        }
#else
public: property bool EditingControlValueChanged
        {
        virtual void set ( bool b );
        virtual bool get ( void )
            {
            return m_valueChanged;
            }
        }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
public: virtual Object^ GetEditingControlFormattedValue ( DataGridViewDataErrorContexts context ) override
        {
        Object ^o = __super::GetEditingControlFormattedValue ( context );
        return o;
        }
#else
public: virtual Object^ GetEditingControlFormattedValue ( DataGridViewDataErrorContexts context )
        {
        return EditingControlFormattedValue;
        }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
public: virtual property bool RepositionEditingControlOnValueChange
        {
        virtual bool get ( void ) override
            {
            bool value = __super::RepositionEditingControlOnValueChange;
            return value;
            }
        }
#else
public: property bool RepositionEditingControlOnValueChange
        {
        virtual bool get ( void )
            {
            return false;
            }
        }
#endif

#ifndef DEBUG_USE_COMBO_CONTROL
public: property System::Windows::Forms::Cursor^ EditingPanelCursor
        {
        virtual System::Windows::Forms::Cursor^ get ( void )
            {
            return TreeView->Cursor;
            }
        }
#endif

#ifndef DEBUG_USE_COMBO_CONTROL
public: virtual void ApplyCellStyleToEditingControl ( System::Windows::Forms::DataGridViewCellStyle ^style )
        {
        TreeView->BackColor = style->BackColor;
        // TBD
        }
#endif

#ifndef DEBUG_USE_COMBO_CONTROL
public: virtual bool EditingControlWantsInputKey ( Keys key, bool dataGridViewWantsInputKey )
        {
        return !dataGridViewWantsInputKey; // ??
        }
#endif

#pragma endregion " IDataGridViewEditingControl "

#pragma region " ComboBox Properties and Methods "

protected: virtual void OnCreateControl ( void ) override;

#ifdef DEBUG_USE_COMBO_CONTROL
public: property bool DroppedDown
        {
        bool get ( void ) new;
        void set ( bool value ) new;
        }
#else
public: property bool DroppedDown
        {
        bool get ( void );
        void set ( bool value );
        }
#endif

#ifdef DEBUG_USE_COMBO_CONTROL
protected: virtual void OnDropDown ( System::EventArgs ^e ) override
        {
        //System::Diagnostics::Debug::WriteLine ( "OnDropDown" );
        //__super::OnDropDown ( e );
        if ( !DroppedDown )
            DroppedDown = true;
        }

protected: virtual void OnKeyDown ( KeyEventArgs ^e ) override
        {
        if ( ( e->KeyCode == Keys::Up || e->KeyCode == Keys::Down ) && e->Modifiers == Keys::Alt )
            {
            e->Handled = true;
            DroppedDown = true;
            }
        }

protected: virtual void OnClick( EventArgs ^e ) override
        {
        if ( DroppedDown )
            {
            DroppedDown = false;
            Focus ();
            FindForm()->Focus ();
            Focus ();
            }
        }

protected: virtual void WndProc ( Message %m ) override
        {
        //System::Diagnostics::Debug::WriteLine ( String::Format ( "Msg: {0:x} HWnd: {1} WParam: {2} LParam: {3}", m.Msg, m.HWnd, m.WParam, m.LParam ) );
        if ( m.Msg == WM_CTLCOLORLISTBOX )
            ShowWindow ( reinterpret_cast<HWND>( m.LParam.ToPointer() ), SW_HIDE );
        __super::WndProc ( m );
        }

public: property Object^ SelectedItem
        {
        Object^ get ( void ) new
            {
            TreeNode ^node = safe_cast< TreeNode^ >( __super::SelectedItem );
            return node;
            }
        void set ( Object ^node ) new
            {
            __super::SelectedItem = node;
            }
        }

protected: virtual void OnDrawItem ( DrawItemEventArgs ^e ) override;

//protected: virtual void SetBoundsCore ( System::Int32 ^x, System::Int32 ^y, System::Int32 ^width, System::Int32 ^height, \
//                                        BoundsSpecified ^specified ) override
//        {
//        __super::SetBoundsCore ( x, y, width, height, specified );
//        }
#endif


#pragma endregion " ComboBox Properties and Methods "

#ifdef DEBUG_USE_COMBO_CONTROL

private: typedef TreeNode   TreeNodeEx;

//public: delegate void BeforeSelectHandler ( TreeNode ^node, bool %cancel );
//
//public: event void BeforeSelect(  );
//public: event AfterSelect ( void ) new;
//public: event DropDown ( void ) new;
//public: event CloseUp ( void ) new;

#else

//--------------------------------------------------------------------------------------

//protected: virtual void OnValueChanged ( EventArgs ^eventargs ) override
//    {
//    // Notify the DataGridView that the contents of the cell
//    // have changed.
//    m_valueChanged = true;
//    this->EditingControlDataGridView->NotifyCurrentCellDirty ( true );
//    __super::OnValueChanged ( eventargs );
//    }

#endif

#pragma region " TreeView Methods "

private: property System::Windows::Forms::TreeView^ TreeView
        {
        System::Windows::Forms::TreeView^ get ( void )
            {
#ifdef DEBUG_USE_COMBO_CONTROL
            return m_treeView;
#else
            return this;
#endif
            }
        }

/*---------------------------------------------------------------------------------**//**
* Handle Tree Node Selection
* @bsimethod						Bill.Steinbock
+---------------+---------------+---------------+---------------+---------------+------*/
private: void TreeViewNodeSelect
        (
        System::Object^                                      sender, 
        System::Windows::Forms::TreeNodeMouseClickEventArgs^ e
        ); 

private: void treeView_BeforeSelect ( System::Object ^sender, System::Windows::Forms::TreeViewCancelEventArgs ^e );
private: void treeView_AfterSelect ( System::Object ^sender, System::Windows::Forms::TreeViewEventArgs ^e );

#pragma endregion " TreeView Methods "


}; // End DataGridViewElementTemplateCellEditingControl class
