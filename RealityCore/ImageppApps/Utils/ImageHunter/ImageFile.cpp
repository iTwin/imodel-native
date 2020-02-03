/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageFile.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ImageFile.cpp,v 1.3 2011/07/18 21:12:39 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class ImageFile
//-----------------------------------------------------------------------------
#include "Stdafx.h"
#include "ImageFile.h"

using namespace System; 
using namespace System::Collections;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ImageFile::ImageFile(String^ filename)
{
    System::IO::FileInfo^ fileInfo = gcnew System::IO::FileInfo(filename);
    m_Filename = fileInfo->Name;
    m_Path = fileInfo->DirectoryName;
    m_Filesize = fileInfo->Length;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ImageFile::GetShortName()
{
    return m_Filename;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ImageFile::GetPath()
{
    return m_Path;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
String^ ImageFile::GetFullName()
{
    return m_Path + L"\\" + m_Filename;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
System::UInt64 ImageFile::GetFilesize()
{
    return m_Filesize;
}