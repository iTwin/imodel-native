/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/DataGridViewElementTemplateCellEditingControl.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"
#include    "DataGridViewElementTemplateCellEditingControl.h"
#include    "DataGridViewElementTemplateCell.h"

USING_NAMESPACE_BENTLEY_TERRAINMODEL_COMMANDS
using namespace Bentley::MstnPlatformNET::XDataTree;
using namespace System::Drawing;
using namespace System::Windows::Forms;
namespace DGNET = Bentley::DgnPlatformNET;

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
ref class ElementTemplateHelper
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
internal: static void LoadTemplatesIntoTree 
(
TreeNodeCollection^             treeNodes,
System::Collections::Generic::IEnumerable <DGNET::XDataTree::XDataTreeNode^>^   dataTreeNodes,
String^                         selected,
TreeNode^%                       ppSelectedNode
)
    {
    for each ( DGNET::XDataTree::XDataTreeNode^ data in dataTreeNodes )
        {
        if ( nullptr != data && ( ( data->AllowChildren && data->ChildNodes->Count ) || !data->AllowChildren ) )
            {
            TreeNode^ childNode = gcnew TreeNode ( data->Name, 0, 0 ); 

            if ( selected != nullptr && nullptr == ppSelectedNode )
                {
                if ( 0 == System::String::Compare ( data->FullPath, selected, true ) )
                    ppSelectedNode = childNode;
                }
            childNode->Tag = data;
            if ( data->ChildNodes->Count > 0 )
                {
                LoadTemplatesIntoTree ( childNode->Nodes, data->ChildNodes, selected, ppSelectedNode );
                }

            treeNodes->Add ( childNode );
            }
        }
    }

}; // End ElementtemplateHelper class

/*---------------------------------------------------------------------------------**//**
* Handle Tree Node Selection
* @bsimethod						Bill.Steinbock
+---------------+---------------+---------------+---------------+---------------+------*/
void DataGridViewElementTemplateCellEditingControl::TreeViewNodeSelect ( Object ^sender, TreeNodeMouseClickEventArgs ^e ) 
    {
    TreeNode^ node = e->Node;

    if (nullptr == node)
        return;

    // --- only close tree if the node selected has no children ---
    if ( 0 == node->Nodes->Count ) 
        {
        DGNET::XDataTree::XDataTreeNode^ templateData = dynamic_cast<DGNET::XDataTree::XDataTreeNode^>( node->Tag );

        if ( nullptr != templateData )
            {
            if ( !templateData->AllowChildren )
                {
                m_templatePath = templateData->FullPath;
//                m_edSvc->CloseDropDown();
                }
            }
        else
            {
            // --- user selected "None" --- 
            m_templatePath = node->Text;
//            m_edSvc->CloseDropDown();
            }
        }
    }

#pragma region " Constructors "

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
DataGridViewElementTemplateCellEditingControl::DataGridViewElementTemplateCellEditingControl ( void )
    {
    m_isDroppingDown = false;
    m_focusOnMouseLeave = false;
    m_focusOnVisibleChanged = false;

#ifdef DEBUG_USE_COMBO_CONTROL
    m_treeView = gcnew TreeView();
    m_treeView->Visible = false;
    Controls->Add ( m_treeView );
    DrawMode = DrawMode::OwnerDrawFixed;

    TreeView->Nodes->Add ( gcnew TreeNode ( /*TemplateExtensionManager::GetLocalizedString ( L"LABEL_None")*/ "None", 0, 0 ) );
    TreeView->Nodes->Add ( gcnew TreeNode ( /*TemplateExtensionManager::GetLocalizedString ( L"LABEL_None")*/ "Active", 0, 0 ) );

    m_templatePath = TreeView->Nodes [0]->FullPath;

    //BorderStyle = BorderStyle::None;
    TreeView->NodeMouseClick += gcnew TreeNodeMouseClickEventHandler ( this, &DataGridViewElementTemplateCellEditingControl::TreeViewNodeSelect );
    Location = SD::Point ( 0,0 );
    //Dock = DockStyle::Fill;
    Dock = DockStyle::None;

    TreeNode ^selectedNode = nullptr;
    if ( nullptr != TemplateExtensionManager::ElementTemplateDataTree->CurrentFile )
        {
        // --- define the processing order to merge the template list ---
        FileNameList^ fileList = gcnew FileNameList ();
        fileList->Add (gcnew FileName (Session::Instance->GetActiveFileName()));
        fileList->LoadUsingCfgVariable (L"MS_DGNLIBLIST", true);
        fileList->LoadUsingCfgVariable (L"_USTN_SYSTEMDGNLIBLIST", true);

        //Nodes->Add (gcnew TreeNode (TemplateExtensionManager::GetLocalizedString(L"LABEL_None"), 0, 0));

        ElementTemplateHelper::LoadTemplatesIntoTree ( TreeView->Nodes, ToolBoxManager::GenerateMergedListOfTreeNodes ( TemplateExtensionManager::ElementTemplateDataTree, false, true, fileList, nullptr ), m_templatePath, &selectedNode );
        }
    //__super::VisibleCount = 5;
#else
    m_vw = nullptr;
    TreeNode^ tn = gcnew TreeNode (GET_LOCALIZED ("ITEM_None"), 0, 0);
    TreeView->Nodes->Add (tn);
    SelectedNode = tn;
    TreeView->Nodes->Add (gcnew TreeNode (GET_LOCALIZED ("ITEM_Active"), 0, 0));

    m_templatePath = TreeView->Nodes [0]->FullPath;

    //BorderStyle = BorderStyle::None;
    TreeView->NodeMouseClick += gcnew TreeNodeMouseClickEventHandler ( this, &DataGridViewElementTemplateCellEditingControl::TreeViewNodeSelect );
    //Location = SD::Point ( 0,0 );
    //Dock = DockStyle::Fill;
    Dock = DockStyle::None;

    TreeNode^ selectedNode = nullptr;
    ElementTemplateHelper::LoadTemplatesIntoTree ( TreeView->Nodes, XDataTreeSessionMgr::s_TemplateDataTrees->GetRootNodeCollection(), m_templatePath, selectedNode );
//__super::VisibleCount = 5;
//#ifdef DEBUG_USE_COMBO_CONTROL
//    m_treeView->Show ();
#endif
    EditingControlValueChanged = false;
    TreeView->BeforeSelect += gcnew TreeViewCancelEventHandler ( this, &DataGridViewElementTemplateCellEditingControl::treeView_BeforeSelect );
    TreeView->AfterSelect += gcnew TreeViewEventHandler ( this, &DataGridViewElementTemplateCellEditingControl::treeView_AfterSelect );

    }

#pragma endregion " Constructors "

#pragma region " TreeView Methods "

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::treeView_BeforeSelect ( Object ^sender, TreeViewCancelEventArgs ^e )
    {
    //__super::OnBeforeSelect ();
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::treeView_AfterSelect ( Object ^sender, TreeViewEventArgs ^e )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    Items->Clear ();
    //Items->Add ( e->Node->Text );
    Items->Add ( e->Node->FullPath );
    SelectedIndex = 0;
#endif
    if ( e->Action == TreeViewAction::ByMouse )
        {
        m_focusOnMouseLeave = true;
        DroppedDown = false;
        }
    //    RaiseEvent AfterSelect()
    }

#pragma endregion " TreeView Methods "

#pragma region " ComboBox Properties and Methods "

#ifdef DEBUG_USE_COMBO_CONTROL

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::OnDrawItem ( DrawItemEventArgs ^e )
    {
    return __super::OnDrawItem ( e );
    if ( e->Index == -1 )
        return;

    // Draw only the combo edit box
    if ( ( e->State & DrawItemState::ComboBoxEdit ) != DrawItemState::ComboBoxEdit )
        return;

    TreeNodeEx ^comboBoxSelection = nullptr;
    try {

        Object ^item = Items [ e->Index ];

        comboBoxSelection = safe_cast<TreeNodeEx^>( item );
        e->DrawFocusRectangle ();
        e->DrawBackground ();

        int textLeft = e->Bounds.Left;

        if ( m_treeView->ImageList != nullptr )
            {
            int imageIndex = comboBoxSelection->SelectedImageIndex;

            if ( imageIndex < 0 )
                imageIndex = m_treeView->SelectedImageIndex;
            if ( imageIndex >= 0 )
                {
                SD::Rectangle  ^iconRectangle;

                if ( m_treeView->ImageList->ImageSize.Height > ItemHeight )
                    iconRectangle = gcnew SD::Rectangle ( e->Bounds.Left + 2, e->Bounds.Top, ItemHeight, ItemHeight );
                else
                    iconRectangle = gcnew SD::Rectangle ( SD::Point ( e->Bounds.Left + 2, e->Bounds.Top ), m_treeView->ImageList->ImageSize );

                m_treeView->ImageList->Draw ( e->Graphics, iconRectangle->Left, iconRectangle->Top, iconRectangle->Width, iconRectangle->Height, imageIndex );
                textLeft += iconRectangle->Width + 2;
                delete iconRectangle;
                }
            }
            e->Graphics->DrawString ( comboBoxSelection->Text, Font, gcnew SolidBrush ( e->ForeColor), float ( textLeft ), float ( e->Bounds.Top + 2 ) );
        }
    catch ( Exception ^ex )
        {
        throw ex;
        }
    finally
        {
        comboBoxSelection = nullptr;
        __super::OnDrawItem ( e );
        }
    }
#endif 

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::OnCreateControl ( void )
    {
    __super::OnCreateControl ();
//    TreeView->Visible = false;
//    TreeView->Height = 500;
//    TreeView->ImageIndex = 0;
//    TreeView->SelectedImageIndex = 1;
//#ifdef DEBUG_USE_COMBO_CONTROL
//    TreeView->BorderStyle = BorderStyle::FixedSingle;
//#endif
//    TreeView->TabIndex = TabIndex;
//    TreeView->TabStop = false;
//    TreeView->HideSelection = false;
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
bool DataGridViewElementTemplateCellEditingControl::DroppedDown::get ( void )
    {
    return TreeView->Visible;
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::DroppedDown::set ( bool value )
    {
    if ( m_isDroppingDown )
        return;
    m_isDroppingDown = true;
    if ( value )
        {
        if ( !TreeView->Visible )
            {
            TreeView->BringToFront ();
            TreeView->Show ();
            Focus ();
            TreeView->Focus ();
    #ifdef DEBUG_USE_COMBO_CONTROL
            __super::OnDropDown ( gcnew EventArgs() ) ;
    #endif
            }
        }
    else if ( TreeView->Visible )
        {
        EditingControlValueChanged = true;
        DataGridViewElementTemplateCell ^elmTemplateCell = safe_cast<DataGridViewElementTemplateCell ^>( EditingControlDataGridView->CurrentCell );
        elmTemplateCell->Items->Clear ();
        elmTemplateCell->Items->Add ( m_templatePath );
        elmTemplateCell->Value = m_templatePath;
        TreeView->Hide ();
        EditingControlDataGridView->NotifyCurrentCellDirty ( true );
        //m_vw;
        //OnCloseUp ();
        }
    m_isDroppingDown = false;
    }

#pragma endregion " ComboBox Properties and Methods "

#pragma region " IDataGridViewEditingControl "

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::PrepareEditingControlForEdit ( bool selectAll )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    TreeView->Location = Parent->Location;
#else
    //if ( !DroppedDown )
    //    DroppedDown = true;
#endif
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::EditingControlFormattedValue::set ( Object ^value )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    __super::EditingControlFormattedValue = value;
#else
#endif
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
Object^ DataGridViewElementTemplateCellEditingControl::EditingControlFormattedValue::get ( void )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    Object ^value = __super::EditingControlFormattedValue;
    return value;
#else
    return m_templatePath;
#endif
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::EditingControlValueChanged::set ( bool value )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    if ( !value )
        {
        //Items->Clear ();
        //for each ( TreeNode ^node in TreeView->Nodes )
        //    {
        //    Items->Add ( node );
        //    }
        }
    __super::EditingControlValueChanged = value;
#else
    m_valueChanged = value;
#endif
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void DataGridViewElementTemplateCellEditingControl::EditingControlDataGridView::set ( DataGridView ^vw )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    TreeView->Parent = vw;
    __super::EditingControlDataGridView = vw;
#else
    m_vw = vw;
#endif
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
DataGridView^ DataGridViewElementTemplateCellEditingControl::EditingControlDataGridView::get ( void )
    {
#ifdef DEBUG_USE_COMBO_CONTROL
    DataGridView^ vw = __super::EditingControlDataGridView;
    return vw;
#else
    return m_vw;
#endif
    }

#pragma endregion " IDataGridViewEditingControl "
