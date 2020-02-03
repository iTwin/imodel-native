/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ResultComparer.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

ref class ResultComparer : public System::Collections::IComparer
{
public:
    ResultComparer();
    ResultComparer(int column, System::Windows::Forms::SortOrder order);

    virtual int                         Compare(System::Object^ x, System::Object^ y);

private:
    int                                 col;
    System::Windows::Forms::SortOrder   m_Order;
};

enum class FileSize
{
    BYTES,
    KILOBYTES,
    MEGABYTES,
    GIGABYTES
};

ref class FileSizeComparer : public System::Collections::IComparer
{

public:
    FileSizeComparer();
    FileSizeComparer(int column, System::Windows::Forms::SortOrder order);

    virtual int                         Compare(System::Object^ x, System::Object^ y);

private:
    FileSize                            ConvertTextToEnum(System::String^ fileFormat);
    int                                 col;
    System::Windows::Forms::SortOrder   m_Order;
};
