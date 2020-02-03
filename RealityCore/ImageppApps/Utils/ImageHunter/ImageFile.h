/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ImageFile.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

ref class ImageFile
{
public:
    ImageFile(System::String^ filename);

    System::String^     GetShortName();
    System::String^     GetPath();
    System::String^     GetFullName();
    System::UInt64      GetFilesize();

private:
    System::String^     m_Filename;
    System::String^     m_Path;
    System::UInt64      m_Filesize;
};