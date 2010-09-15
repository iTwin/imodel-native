/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECType.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstalledTypeHandlerMgrR           ECInstalledTypeHandlerMgr::GetManager()
    {
    static ECInstalledTypeHandlerMgrP   s_typeHandlerManager = NULL;

    if (NULL == s_typeHandlerManager)
        s_typeHandlerManager = new ECInstalledTypeHandlerMgr();
        
    return *s_typeHandlerManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstalledTypeHandlerMgr::ECInstalledTypeHandlerMgr ()
    {
    DgnColorValue* ctHandler = new DgnColorValue ();
    DgnFillColorValue* fctHandler = new DgnFillColorValue ();

    // Add default installed types
    AddIECInstalledTypeValue (ctHandler->GetTypeName(), ctHandler);
    AddIECInstalledTypeValue (fctHandler->GetTypeName(), fctHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstalledTypeValueP   ECInstalledTypeHandlerMgr::GetIECInstalledTypeValue (wchar_t const* typeName) const
    {
    NameECInstalledTypeHandlerMap::const_iterator it = m_installTypeHandlers.find (typeName);
    if (it == m_installTypeHandlers.end())
        return NULL;

    return it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            ECInstalledTypeHandlerMgr::AddIECInstalledTypeValue (wchar_t const* typeName, IECInstalledTypeValueP typeHandler)
    {
    if (GetIECInstalledTypeValue (typeName))
        return ECOBJECTS_STATUS_InstalledTypeHandlerAlreadyDefined;

    m_installTypeHandlers[WString(typeName)] = typeHandler;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTypeDescriptor& ECTypeDescriptor::operator= (ECTypeDescriptor const & rhs)
        {
        m_typeKind = rhs.m_typeKind; 

        if (VALUEKIND_Array == m_typeKind)
            m_arrayKind = rhs.m_arrayKind; 
        else
            m_primitiveType = rhs.m_primitiveType; 

        if (rhs.m_installedTypeName.empty())
            m_installedTypeName.clear();
        else
            m_installedTypeName = rhs.m_installedTypeName;

        return *this;
        }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTypeDescriptor                ECTypeDescriptor::CreatePrimitiveTypeDescriptor 
(
PrimitiveType primitiveType
) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Primitive; 
    type.m_primitiveType = primitiveType; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_primitiveType = primitiveType; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructArrayTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_arrayKind = ARRAYKIND_Struct; 
    return type; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateStructTypeDescriptor () 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Struct; 
    type.m_arrayKind = (ArrayKind)0; 
    return type; 
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTypeDescriptor   ECTypeDescriptor::CreateInstalledPrimitiveTypeDescriptor (wchar_t const* installedTypeName)
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Primitive; 
    type.m_primitiveType = PRIMITIVETYPE_Installed; 
    type.m_installedTypeName = installedTypeName; 
    return type; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECTypeDescriptor                ECTypeDescriptor::CreateInstalledPrimitiveArrayTypeDescriptor (wchar_t const* installedTypeName) 
    { 
    ECTypeDescriptor type; 
    type.m_typeKind = VALUEKIND_Array; 
    type.m_primitiveType = PRIMITIVETYPE_Installed; 
    type.m_installedTypeName = installedTypeName; 
    return type; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstalledTypeValueP      ECTypeDescriptor::GetInstalledPrimitiveTypeValue() const
    {
    if (PRIMITIVETYPE_Installed != m_primitiveType)
        return NULL;

    return ECInstalledTypeHandlerMgr::GetManager().GetIECInstalledTypeValue (m_installedTypeName.c_str());
    }
       
//////////////////////////////////////////////////////////////////////////////////////////
//   DgnColorValue
//////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColorValue::DgnColorValue ()
    {
    m_colorSource = 0; 
    m_colorType   = 0; 
    m_colorIndex  = 0;  
    m_red         = 0;
    m_green       = 0;
    m_blue        = 0;
    m_data        = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColorValue::~DgnColorValue ()
    {
    ClearData ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorValue::ClearData ()
    {
    if (m_data)
        delete[] m_data;

    m_data = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
byte const*    DgnColorValue::GetBytePointer (UInt32& numBytes)
    {
    ClearData ();

    Int32     offset          = 0;
    Int32     bookNameLength  = (Int32)m_colorBook.length();
    Int32     colorNameLength = (Int32)m_colorName.length();
    size_t    bookNameSize    = bookNameLength * sizeof(wchar_t);
    size_t    colorNameSize   = colorNameLength * sizeof(wchar_t);
    size_t    totalSize       = 6*sizeof(UInt8) + 2*sizeof(Int32) + bookNameSize + colorNameSize;

    m_data    = new UInt8[totalSize];

    m_data[0] = (UInt8) m_colorSource;
    m_data[1] = (UInt8) m_colorType;
    m_data[2] = (UInt8) m_colorIndex;
    m_data[3] = (UInt8) m_red;
    m_data[4] = (UInt8) m_green;
    m_data[5] = (UInt8) m_blue;
    offset = 6;

    // write length of colorbook and colorname entries.
    memcpy (&m_data[offset], &bookNameLength, sizeof(Int32));
    offset += sizeof(Int32);

    memcpy (&m_data[offset], &colorNameLength, sizeof(Int32));
    offset += sizeof(Int32);

    if (bookNameLength > 0)
        {
        memcpy (&m_data[offset], m_colorBook.data(), bookNameSize);
        offset += (Int32)bookNameSize;
        }
    if (colorNameLength > 0)
        {
        memcpy (&m_data[offset], m_colorName.data(), colorNameSize);
        offset += (Int32)colorNameSize;
        }

    numBytes = (UInt32)totalSize;
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                  DgnColorValue::LoadFromByteData (byte const * byteBuffer)
    {
    m_colorSource = (UInt8)byteBuffer[0];
    m_colorType   = (UInt8)byteBuffer[1];
    m_colorIndex  = (UInt8)byteBuffer[2];
    m_red         = (Int8)byteBuffer[3];
    m_green       = (Int8)byteBuffer[4];
    m_blue        = (Int8)byteBuffer[5];

    Int32 bookNameLength = 0;
    Int32 colorNameLength = 0;
    Int32 offset = 6;

    memcpy (&bookNameLength, &byteBuffer[offset], sizeof(Int32));
    offset += sizeof(Int32);

    memcpy (&colorNameLength, &byteBuffer[offset], sizeof(Int32));
    offset += sizeof(Int32);

    if (bookNameLength > 0)
        {
        size_t bookSize = bookNameLength*sizeof(wchar_t);
        wchar_t* buffer = (wchar_t*)_alloca (bookSize + sizeof(wchar_t));
        memcpy (buffer, &byteBuffer[offset], bookSize);
        buffer[bookNameLength]=0;
        offset += (Int32)bookSize;
        m_colorBook = buffer;
        }

    if (colorNameLength > 0)
        {
        size_t colorSize = colorNameLength*sizeof(wchar_t);
        wchar_t* buffer = (wchar_t*)_alloca (colorSize + sizeof(wchar_t));
        memcpy (buffer, &byteBuffer[offset], colorSize);
        buffer[colorNameLength]=0;
        offset += (Int32)colorSize;
        m_colorName = buffer;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString                    DgnColorValue::GetStringValue () const
    {
    WString stringVal;
#ifdef NOT_YET
    if (nullptr == m_color)
        return System::String::Empty;

    System::String^  colorBook = (nullptr == m_color->BookName) ? System::String::Empty : m_color->BookName;
    System::String^  colorName = (nullptr == m_color->ColorName) ? System::String::Empty : m_color->ColorName;
    DECLARE_AND_ALLOCATE_ARRAY(args, System::Object^, 8)
    args[0] = System::Int32 (m_color->Source).ToString(System::Globalization::CultureInfo::InvariantCulture);
    args[1] = System::Int32 (m_color->ElementColorType).ToString(System::Globalization::CultureInfo::InvariantCulture);
    args[2] = m_color->Index;
    args[3] = m_color->Red;
    args[4] = m_color->Green;
    args[5] = m_color->Blue;
    args[6] = colorBook;
    args[7] = colorName;
    return  System::String::Format (System::Globalization::CultureInfo::InvariantCulture, L"{0},{1},{2}:[{3},{4},{5}]:\\{6}\\{7}\\", args);
#endif
    return stringVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 DgnColorValue::LoadFromStringValue (wchar_t const* stringValue)
    {
#ifdef NOT_YET
        array <System::Char>^ semicolon = {L':'};
    array <System::String^>^ substrings = value->Split(semicolon, 3);
    if (substrings->Length == 3)
        {
        // Source, Type, and Index are in first string.
        array <System::Char>^ comma = {L','};
        array <System::String^>^ vals = substrings[0]->Split (comma, 3);
        byte        source, type, index;

        source  = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[0]);
        type    = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[1]);
        index   = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[2]);

        // rgb triad in second string.
        array <System::Char>^ delimiters = {L'[', L',', L']'};
        array <System::String^>^ rgbs = substrings[1]->Split (delimiters); //"", "red#", "green#", "blue#" ""
        byte        red   = 0;
        byte        green = 0;
        byte        blue  = 0;
        if (rgbs->Length == 5)
            {
            red   = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[1]);
            green = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[2]);
            blue  = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[3]);
            }

        // colorbook\colorname in third string.
        System::String^      colorBook = nullptr;
        System::String^      colorName = nullptr;
        array <System::Char>^ bookDelimiters = {L'\\'};
        array <System::String^>^  names = substrings[2]->Split (bookDelimiters);
        if (names->Length == 4)
            {
            colorBook = names[1];
            colorName = names[2];
            }

        DgnModelPointer^ cacheRef = dynamic_cast <DgnModelPointer^>(Instance->ReferenceObject);
        if (nullptr == cacheRef)
            NativeValue = gcnew DECP::ElementColor (source, type, index, red, green, blue, colorBook, colorName);
        else
            NativeValue = gcnew DECP::ElementColor (source, type, index, red, green, blue, colorBook, colorName, *(cacheRef->DgnModelP));
        }
    else if (substrings->Length == 1)
        {
        int                     colorIndex = Bentley::ECObjects::ECObjects::StringToInt (substrings[0]);
        DgnModelPointer^            cacheRef  = dynamic_cast <DgnModelPointer^>(Instance->ReferenceObject);
        DECP::ElementColor^     color = gcnew DECP::ElementColor((DECP::ColorSupports)(DECP::ColorSupports::Indexed | DECP::ColorSupports::Rgb | DECP::ColorSupports::ColorBook));
        color->SetIndex (colorIndex, *(cacheRef->DgnModelP));
        NativeValue = color;
        }
#endif
    return ECOBJECTS_STATUS_Success;
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstalledTypeValueP     DgnColorValue::Clone ()
    {
    DgnColorValue* result          = new DgnColorValue ();
    result->m_colorSource          = m_colorSource;
    result->m_colorType            = m_colorType;
    result->m_colorIndex           = m_colorIndex;
    result->m_red                  = m_red;
    result->m_green                = m_green;
    result->m_blue                 = m_blue;
    result->m_colorBook            = m_colorBook;
    result->m_colorName            = m_colorName;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*     DgnColorValue::GetInstalledTypeName () const
    {
    return DgnColorValue::GetTypeName();
    }

//////////////////////////////////////////////////////////////////////////////////////////
//   DgnFillColorValue
//////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFillColorValue::DgnFillColorValue ()
    {
    m_gradientAngle   = 0.0;              
    m_tint            = 0.0;              
    m_shift           = 0.0;              
    m_gradientFlags   = 0;                
    m_gradientMode    = 0;                
    m_activeColorKeys = 0;                
    m_keys            = NULL;             
    }                 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFillColorValue::~DgnFillColorValue ()
    {
    if (m_keys)
        {
        delete m_keys;
        m_keys = NULL;
        }
    }                 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnFillColorValue::ClearData ()
    {
    if (m_data)
        delete[] m_data;

    m_data = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
byte const*    DgnFillColorValue::GetBytePointer (UInt32& numBytes)
    {
    ClearData ();

    Int32     offset          = 0;
    Int32     bookNameLength  = (Int32)m_colorBook.length();
    Int32     colorNameLength = (Int32)m_colorName.length();
    size_t    bookNameSize    = bookNameLength * sizeof(wchar_t);
    size_t    colorNameSize   = colorNameLength * sizeof(wchar_t);
    size_t    totalSize       = 6*sizeof(UInt8) + 2*sizeof(Int32) + bookNameSize + colorNameSize;

    m_data    = new UInt8[totalSize];

    m_data[0] = (UInt8) m_colorSource;
    m_data[1] = (UInt8) m_colorType;
    m_data[2] = (UInt8) m_colorIndex;
    m_data[3] = (UInt8) m_red;
    m_data[4] = (UInt8) m_green;
    m_data[5] = (UInt8) m_blue;
    offset = 6;

    // write length of colorbook and colorname entries.
    memcpy (&m_data[offset], &bookNameLength, sizeof(Int32));
    offset += sizeof(Int32);

    memcpy (&m_data[offset], &colorNameLength, sizeof(Int32));
    offset += sizeof(Int32);

    if (bookNameLength > 0)
        {
        memcpy (&m_data[offset], m_colorBook.data(), bookNameSize);
        offset += (Int32)bookNameSize;
        }
    if (colorNameLength > 0)
        {
        memcpy (&m_data[offset], m_colorName.data(), colorNameSize);
        offset += (Int32)colorNameSize;
        }

    numBytes = (UInt32)totalSize;
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                  DgnFillColorValue::LoadFromByteData (byte const * byteBuffer)
    {
    m_colorSource = (UInt8)byteBuffer[0];
    m_colorType   = (UInt8)byteBuffer[1];
    m_colorIndex  = (UInt8)byteBuffer[2];
    m_red         = (Int8)byteBuffer[3];
    m_green       = (Int8)byteBuffer[4];
    m_blue        = (Int8)byteBuffer[5];

    Int32 bookNameLength = 0;
    Int32 colorNameLength = 0;
    Int32 offset = 6;

    memcpy (&bookNameLength, &byteBuffer[offset], sizeof(Int32));
    offset += sizeof(Int32);

    memcpy (&colorNameLength, &byteBuffer[offset], sizeof(Int32));
    offset += sizeof(Int32);

    if (bookNameLength > 0)
        {
        size_t bookSize = bookNameLength*sizeof(wchar_t);
        wchar_t* buffer = (wchar_t*)_alloca (bookSize + sizeof(wchar_t));
        memcpy (buffer, &byteBuffer[offset], bookSize);
        buffer[bookNameLength]=0;
        offset += (Int32)bookSize;
        m_colorBook = buffer;
        }

    if (colorNameLength > 0)
        {
        size_t colorSize = colorNameLength*sizeof(wchar_t);
        wchar_t* buffer = (wchar_t*)_alloca (colorSize + sizeof(wchar_t));
        memcpy (buffer, &byteBuffer[offset], colorSize);
        buffer[colorNameLength]=0;
        offset += (Int32)colorSize;
        m_colorName = buffer;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString                    DgnFillColorValue::GetStringValue () const
    {
    WString stringVal;

    // xxx,xxx,xxx:[xxx,xxx,xxx]:\\colorBook\\colorName\\ 
    wchar_t buffer[31];

    _snwprintf(buffer, 30, L"%u,%u,%u:[%d,%d,%d]:\\", (UInt32)m_colorSource, (UInt32)m_colorType, (UInt32)m_colorIndex, 
                                                      (Int32)m_red, (Int32) m_green, (Int32) m_blue);
    stringVal = buffer;
    stringVal += m_colorBook;
    stringVal +=L"\\";
    stringVal += m_colorName;
    stringVal +=L"\\";

    return stringVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            DgnFillColorValue::LoadFromStringValue (wchar_t const* stringValue)
    {
#ifdef NOT_YET
    int wscanf (L"" )
        array <System::Char>^ semicolon = {L':'};
    array <System::String^>^ substrings = value->Split(semicolon, 3);
    if (substrings->Length == 3)
        {
        // Source, Type, and Index are in first string.
        array <System::Char>^ comma = {L','};
        array <System::String^>^ vals = substrings[0]->Split (comma, 3);
        byte        source, type, index;

        source  = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[0]);
        type    = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[1]);
        index   = (byte) Bentley::ECObjects::ECObjects::StringToInt (vals[2]);

        // rgb triad in second string.
        array <System::Char>^ delimiters = {L'[', L',', L']'};
        array <System::String^>^ rgbs = substrings[1]->Split (delimiters); //"", "red#", "green#", "blue#" ""
        byte        red   = 0;
        byte        green = 0;
        byte        blue  = 0;
        if (rgbs->Length == 5)
            {
            red   = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[1]);
            green = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[2]);
            blue  = (byte) Bentley::ECObjects::ECObjects::StringToInt (rgbs[3]);
            }

        // colorbook\colorname in third string.
        System::String^      colorBook = nullptr;
        System::String^      colorName = nullptr;
        array <System::Char>^ bookDelimiters = {L'\\'};
        array <System::String^>^  names = substrings[2]->Split (bookDelimiters);
        if (names->Length == 4)
            {
            colorBook = names[1];
            colorName = names[2];
            }

        DgnModelPointer^ cacheRef = dynamic_cast <DgnModelPointer^>(Instance->ReferenceObject);
        if (nullptr == cacheRef)
            NativeValue = gcnew DECP::ElementColor (source, type, index, red, green, blue, colorBook, colorName);
        else
            NativeValue = gcnew DECP::ElementColor (source, type, index, red, green, blue, colorBook, colorName, *(cacheRef->DgnModelP));
        }
    else if (substrings->Length == 1)
        {
        int                     colorIndex = Bentley::ECObjects::ECObjects::StringToInt (substrings[0]);
        DgnModelPointer^            cacheRef  = dynamic_cast <DgnModelPointer^>(Instance->ReferenceObject);
        DECP::ElementColor^     color = gcnew DECP::ElementColor((DECP::ColorSupports)(DECP::ColorSupports::Indexed | DECP::ColorSupports::Rgb | DECP::ColorSupports::ColorBook));
        color->SetIndex (colorIndex, *(cacheRef->DgnModelP));
        NativeValue = color;
        }
#endif
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstalledTypeValueP     DgnFillColorValue::Clone ()
    {
    DgnFillColorValue* result      = new DgnFillColorValue ();
#ifdef NOT_YET
    result->m_colorSource           = m_colorSource;
    result->m_colorType             = m_colorType;
    result->m_colorIndex            = m_colorIndex;
    result->m_rgb                   = m_rgb;
    result->m_colorBook             = m_colorBook;
    result->m_colorName             = m_colorName;
#endif
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*     DgnFillColorValue::GetInstalledTypeName () const
    {
    return DgnFillColorValue::GetTypeName();
    }



END_BENTLEY_EC_NAMESPACE