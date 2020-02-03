/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ExternalTools.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ExternalTools.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ExternalTool, ExternalTools
//-----------------------------------------------------------------------------

#include "Stdafx.h"
#include "ExternalTools.h"

using namespace ImageHunter;
using namespace System; 
using namespace System::Collections;

/////////////////////////////////////////////////////////////////////////////////////////
//   ExternalTool Class
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalTool::ExternalTool(int id, String^ executable, String^ name, String^ arguments)
{
    m_id = id;
    m_executable = executable;
    m_name = name;
    m_arguments = arguments;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ExternalTool::GetExecutable()
{
    return m_executable;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ExternalTool::GetArguments()
{
    return m_arguments;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ExternalTool::GetName()
{
    return m_name;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ExternalTool::GetID()
{
    return m_id;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTool::SetExecutable(String^ exe)
{
    m_executable = exe;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTool::SetArguments(String^ args)
{
    m_arguments = args;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTool::SetName(String^ name)
{
    m_name = name;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTool::SetID(int id)
{
    m_id = id;
}

/////////////////////////////////////////////////////////////////////////////////////////
//   ExternalTools Class
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalTools::ExternalTools()
{
    m_tools = gcnew SortedList();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalTools^ ExternalTools::GetInstance()
{
    if (m_instance == nullptr)
	{
		m_instance = gcnew ExternalTools();
	}
	
	return m_instance;
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ExternalTools::AddTool(int id, String^ executable, String^ name, String^ arguments)
{
    ExternalTool^ tool = gcnew ExternalTool(id, executable, name, arguments);
    m_tools->Add(id, tool);
    return id;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ExternalTools::AddTool(ExternalTool^ tool)
{
    m_tools->Add(tool->GetID(), tool);
    return tool->GetID();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalTool^ ExternalTools::RemoveTool(int index)
{
    ExternalTool^ tool = (ExternalTool^)m_tools[index];
    m_tools->Remove(index);

    return tool;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTools::RemoveTool(ExternalTool^ tool)
{
    m_tools->Remove(tool->GetID());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ExternalTool^ ExternalTools::GetExternalTool(int index)
{
    return static_cast<ExternalTool^>(m_tools[index]);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SortedList^ ExternalTools::GetExternalTools()
{
    return m_tools;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ExternalTools::SaveActualState()
{
    m_savedTools = (SortedList^)m_tools->Clone();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExternalTools::RestoreFromSavedState()
{
    if (m_savedTools != nullptr)
    {
        m_tools = m_savedTools;
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExternalTools::DropSavedState()
{
    if (m_savedTools != nullptr)
    {
        m_savedTools = nullptr;
        return true;
    }
    return false;
}