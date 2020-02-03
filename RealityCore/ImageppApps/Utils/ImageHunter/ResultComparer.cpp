/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ResultComparer.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ResultComparer.cpp,v 1.2 2010/08/27 18:54:33 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class ResultComparer, FileSizeComparer
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "ResultComparer.h"

using namespace System::Windows::Forms;
using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ResultComparer::ResultComparer(void)
{
    col = 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ResultComparer::ResultComparer(int column, SortOrder order)
{
    col = column;
    m_Order = order;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int ResultComparer::Compare(System::Object^ x, System::Object^ y)
{
    int result = String::Compare(((ListViewItem^) x)->SubItems[col]->Text, ((ListViewItem^) y)->SubItems[col]->Text);
    
    if (m_Order == SortOrder::Descending)
    {
        result *= -1;
    }

    return result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FileSizeComparer::FileSizeComparer(void)
{
    col = 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FileSizeComparer::FileSizeComparer(int column, SortOrder order)
{
    col = column;
    m_Order = order;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int FileSizeComparer::Compare(System::Object^ x, System::Object^ y)
{
    int result = 0;
    ListViewItem^ itemX = (ListViewItem^) x;
    ListViewItem^ itemY = (ListViewItem^) y;

    int spacePosX = itemX->SubItems[col]->Text->LastIndexOf(L" ");
    int spacePosY = itemY->SubItems[col]->Text->LastIndexOf(L" ");

    FileSize typeX = ConvertTextToEnum(itemX->SubItems[col]->Text->Substring(spacePosX + 1));
    FileSize typeY = ConvertTextToEnum(itemY->SubItems[col]->Text->Substring(spacePosY + 1));

    if (typeX < typeY)
    {
        result = -1;
    }
    else if (typeX > typeY)
    {
        result = 1;
    }
    else
    {
        String^ txtX = itemX->SubItems[col]->Text->Substring(0, spacePosX);
        String^ txtY = itemY->SubItems[col]->Text->Substring(0, spacePosY);
        double valueX = double::Parse(txtX);
        double valueY = double::Parse(txtY);

        if (valueX < valueY)
        {
           result = -1;
        }
        else if (valueX == valueY)
        {
            result = 0;
        }
        else
        {
            result = 1;
        }
    }

    if (m_Order == SortOrder::Descending)
    {
        result *= -1;
    }

    return result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FileSize FileSizeComparer::ConvertTextToEnum(String^ fileFormat)
{
    FileSize sizeType;

    if (String::Compare(fileFormat, L"Bytes") == 0)
    {
        sizeType = FileSize::BYTES;
    }
    else if (String::Compare(fileFormat, L"KB") == 0)
    {
        sizeType = FileSize::KILOBYTES;
    }
    else if (String::Compare(fileFormat, L"MB") == 0)
    {
        sizeType = FileSize::MEGABYTES;
    }
    else if (String::Compare(fileFormat, L"GB") == 0)
    {
        sizeType = FileSize::GIGABYTES;
    }
    else
    {
        sizeType = FileSize::GIGABYTES;
    }

    return sizeType;
}