/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/XmlHelper.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

///////////////////////////////////////////////////////////////////////////////////////////
// This file should be included directly from a cpp file and not in a .h file that
// may get used for a PCH. The .cpp file that includes this must also be compiled
// outside of a multi-compile block for the #import statement to work properly.
//////////////////////////////////////////////////////////////////////////////////////////

#include <shlwapi.h>            /* For SHCreateStreamOnFileW() */
#include "DgnPlatform.h"
#include <Bentley/WString.h>

#include    <msxml6/msxml6.tlh>

// Create typedef that make XML processing code easier to read
typedef MSXML2::IXMLDOMDocument2Ptr             XmlDocument;
typedef MSXML2::IXMLDOMNodePtr                  XmlNode;
typedef MSXML2::IXMLDOMNodeListPtr              XmlNodeList;
typedef MSXML2::IXMLDOMElementPtr               XmlElement;
typedef MSXML2::IXMLDOMAttributePtr             XmlAttribute;
typedef MSXML2::IXMLDOMCommentPtr               XmlComment;
typedef MSXML2::IXMLDOMNamedNodeMapPtr          XmlNamedNodeMap;
typedef MSXML2::IXMLDOMDocumentFragmentPtr      XmlDocumentFragment;
typedef MSXML2::IXMLDOMCDATASectionPtr          XmlCDataSection;
typedef MSXML2::IXMLDOMProcessingInstructionPtr XmlProcessingInstruction;
typedef MSXML2::IXMLDOMSchemaCollectionPtr      XmlSchemaCollection;
typedef MSXML2::IXMLDOMParseErrorPtr            XmlParseError;
typedef MSXML2::IXSLProcessorPtr                XslProcessor;
typedef MSXML2::IXSLTemplatePtr                 XslTemplate;
typedef MSXML2::ISAXXMLReaderPtr                SAXXMLReader;
typedef MSXML2::IMXWriterPtr                    MXWriterPtr;

/*=================================================================================**//**
*
* Utility Class to work with MSXML 6
*
* @bsiclass                                                     BentleySystems
+===============+===============+===============+===============+===============+======*/
struct XmlHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    XmlHelper::CreateDomInstance (XmlDocument& pXMLDom, bool bFreeThread=false)
    {
    HRESULT hr = S_FALSE;
    if (bFreeThread)
        hr = pXMLDom.CreateInstance (_uuidof(MSXML2::FreeThreadedDOMDocument60));
    else
        hr = pXMLDom.CreateInstance (_uuidof(MSXML2::DOMDocument60));

    if (FAILED(hr))
        {
        BeAssert (0);
        return false;
        }

    // set deault properties
    pXMLDom->async = VARIANT_FALSE;
    pXMLDom->validateOnParse = VARIANT_FALSE;
    pXMLDom->resolveExternals = VARIANT_TRUE;
    pXMLDom->preserveWhiteSpace = VARIANT_TRUE;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    XmlHelper::LoadXML (XmlDocument& pXMLDom, WCharCP xmlString)
    {
    try
        {
        if (VARIANT_TRUE == pXMLDom->loadXML(_bstr_t(xmlString)))
            return true;
        }
    catch (...)
        {
        // a failure caused an exception.
        return false;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlDocument CreateDomFromText (WCharCP xmlText)
    {
    XmlDocument xmlDomPtr;
    if (!XmlHelper::CreateDomInstance (xmlDomPtr))
        return NULL;

    if (!XmlHelper::LoadXML (xmlDomPtr, xmlText))
        return NULL;

    return xmlDomPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static WString XmlHelper::GetNodeStringValue (XmlNode& nodePtr)
    {
    BSTR    text;
    HRESULT hr = nodePtr->get_text (&text);
    return SUCCEEDED(hr)? (wchar_t*)text: L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeStringValue (WStringR val, XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    val = L""; // initialize return value to empty string

    XmlNode nodePtr = (NULL != path) ? parentNodePtr->selectSingleNode(path) : parentNodePtr;
    if (nodePtr == NULL)
        return false;

    BSTR    text;
    HRESULT hr = nodePtr->get_text (&text);
    if (SUCCEEDED(hr))
        {
        val = (wchar_t*)text;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeDoubleValue (double& val,  XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    WString valString;

    if (GetNodeStringValue (valString, parentNodePtr, path))
        {
        val = _wtof (valString.c_str());
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeInt32Value (Int32& val,  XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    XmlNode nodePtr = (NULL != path) ? parentNodePtr->selectSingleNode(path) : parentNodePtr;
    if (nodePtr == NULL)
        return false;

    BSTR     text;

    HRESULT  hr = nodePtr->get_text (&text);
    if (SUCCEEDED(hr))
        {
        _variant_t var (text);

        hr = VariantChangeType (&var, &var, 0, VT_INT);
        if (SUCCEEDED(hr))
            {
            val = var.intVal;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeUInt32Value (UInt32& val,  XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    XmlNode nodePtr = (NULL != path) ? parentNodePtr->selectSingleNode(path) : parentNodePtr;
    if (nodePtr == NULL)
        return false;

    BSTR     text;
    HRESULT  hr = nodePtr->get_text (&text);
    if (SUCCEEDED(hr))
        {
        _variant_t var (text);

        hr = VariantChangeType (&var, &var, 0, VT_UINT);
        if (SUCCEEDED(hr))
            {
            val = var.uintVal;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeUInt64Value (UInt64& val,  XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    XmlNode nodePtr = (NULL != path) ? parentNodePtr->selectSingleNode(path) : parentNodePtr;
    if (nodePtr == NULL)
        return false;

    BSTR     text;
    HRESULT  hr = nodePtr->get_text (&text);
    if (SUCCEEDED(hr))
        {
        _variant_t var (text);

        hr = VariantChangeType (&var, &var, 0, VT_UI8);
        if (SUCCEEDED(hr))
            {
            val = var.ullVal;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeBoolValue (bool& val,  XmlNode& parentNodePtr, WCharCP path=NULL)
    {
    WString boolVal;

    if (XmlHelper::GetNodeStringValue (boolVal, parentNodePtr, path))
        {
        val = (0 == _wcsicmp(boolVal.c_str(), L"true" ));
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddProcessingInstruction (XmlDocument& pXMLDom, WCharCP target, WCharCP data)
    {
    XmlProcessingInstruction pi = pXMLDom->createProcessingInstruction (target, data);
    pXMLDom->appendChild (pi);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::CreateDocumentComment (XmlDocument& pXMLDom, WCharCP comment)
    {
    XmlComment pc = pXMLDom->createComment (comment);
    pXMLDom->appendChild(pc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::CreateComment (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP comment)
    {
    XmlComment pc = pXMLDom->createComment (comment);
    parentElementPtr->appendChild(pc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::CreateElement (XmlDocument& pXMLDom, WCharCP name, WCharCP namespaceURI=NULL)
    {
    XmlElement elementPtr;

    if (NULL != namespaceURI && 0 != *namespaceURI)
        elementPtr = pXMLDom->createNode(NODE_ELEMENT, name, namespaceURI);
    else
        elementPtr = pXMLDom->createElement (name);

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::CreateRootElement (XmlDocument& pXMLDom, WCharCP name, WCharCP namespaceURI=NULL)
    {
    XmlElement elementPtr = XmlHelper::CreateElement (pXMLDom, name, namespaceURI);

    if (NULL != elementPtr)
        pXMLDom->appendChild (elementPtr);

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::CreateElement (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, WCharCP namespaceURI=NULL)
    {
    XmlElement elementPtr = XmlHelper::CreateElement (pXMLDom, name, namespaceURI);

    if (NULL != elementPtr)
        parentElementPtr->appendChild (elementPtr);

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement XmlHelper::AddElementStringValue (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, WCharCP val, WCharCP namespaceURI=NULL)
    {
    XmlElement elementPtr = XmlHelper::CreateElement (pXMLDom, name, namespaceURI);
    if (NULL == elementPtr)
        return NULL;

    parentElementPtr->appendChild (elementPtr);
    elementPtr->text = val;

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddElementInt32Value (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, Int32 val, WCharCP namespaceURI=NULL)
    {
    _variant_t var (val);

    HRESULT hr = VariantChangeType (&var, &var, 0, VT_BSTR);
    if (!SUCCEEDED(hr))
        return;

    XmlElement elementPtr = XmlHelper::CreateElement (pXMLDom, name, namespaceURI);
    if (NULL != elementPtr)
        {
        parentElementPtr->appendChild (elementPtr);
        elementPtr->text = var.bstrVal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::AddElementUInt32Value (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, UInt32 val)
    {
    _variant_t var (val);

    HRESULT hr = VariantChangeType (&var, &var, 0, VT_BSTR);
    if (!SUCCEEDED(hr))
        return NULL;

    XmlElement elementPtr = pXMLDom->createElement (name);
    if (NULL != elementPtr)
        {
        parentElementPtr->appendChild (elementPtr);
        elementPtr->text = var.bstrVal;
        }

    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::AddElementUInt64Value (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, UInt64 val)
    {
    _variant_t var (val);

    HRESULT hr = VariantChangeType (&var, &var, 0, VT_BSTR);
    if (!SUCCEEDED(hr))
        return NULL;

    XmlElement elementPtr = pXMLDom->createElement (name);
    if (NULL != elementPtr)
        {
        parentElementPtr->appendChild (elementPtr);
        elementPtr->text = var.bstrVal;
        }

    return elementPtr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement     XmlHelper::AddElementBoolValue (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, bool val)
    {
    return XmlHelper::AddElementStringValue (parentElementPtr, pXMLDom, name, val?L"true":L"false");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static XmlElement    XmlHelper::AddElementDoubleValue (XmlElement& parentElementPtr, XmlDocument& pXMLDom, WCharCP name, double val)
    {
    _variant_t var (val);

    ::VariantChangeType (&var, &var, 0, VT_BSTR);

    XmlElement stringElementPtr = pXMLDom->createElement (name);
    if (NULL != stringElementPtr)
        {
        parentElementPtr->appendChild (stringElementPtr);
        stringElementPtr->text = var.bstrVal;
        }

    return stringElementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeStringValue (XmlElement& elementPtr, WCharCP name, WStringR val)
    {
    XmlAttribute attrNodePtr = elementPtr->getAttributeNode (name);
    if (NULL == attrNodePtr)
        return false;

    VARIANT     v;
    attrNodePtr->get_value(&v);
    val = (wchar_t*)v.bstrVal;
    VariantClear (&v);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeInt32Value (XmlElement& elementPtr, WCharCP name, Int32& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtoi (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeUInt32Value (XmlElement& elementPtr, WCharCP name, UInt32& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    _variant_t var (valString.c_str ());

    if (!SUCCEEDED (VariantChangeType (&var, &var, 0, VT_UINT)))
        return false;

    val = var.uintVal;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeInt64Value (XmlElement& elementPtr, WCharCP name, __int64& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtoi64 (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeUInt64Value (XmlElement& elementPtr, WCharCP name, unsigned __int64& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    _variant_t var (valString.c_str ());

    if (!SUCCEEDED (VariantChangeType (&var, &var, 0, VT_UI8)))
        return false;

    val = var.ullVal;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeDoubleValue (XmlElement& elementPtr, WCharCP name, double& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = _wtof (valString.c_str());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeBoolValue (XmlElement& elementPtr, WCharCP name, bool& val)
    {
    WString valString;
    if (!XmlHelper::GetAttributeStringValue (elementPtr, name, valString) || 0 == valString.length())
        return false;

    val = (0 == _wcsicmp(valString.c_str(), L"true" ));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::GetAttributeBoolValue (XmlNode& modelNodePtr, WCharCP name, bool& val)
    {
    // Get ElementPtr from NodePtr
    XmlElement elementPtr (modelNodePtr);

    return XmlHelper::GetAttributeBoolValue (elementPtr, name, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void    XmlHelper::AddAttributeStringValue (XmlElement& elementPtr, WCharCP name, WCharCP val)
    {
    if (NULL == val)
        return;

    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeInt32Value (XmlElement& elementPtr, WCharCP name, Int32 val)
    {
    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void    XmlHelper::AddAttributeDoubleValue (XmlElement& elementPtr, WCharCP name, double val)
    {
    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeUInt32Value (XmlElement& elementPtr, WCharCP name, UInt32 val)
    {
    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeUInt64Value (XmlElement& elementPtr, WCharCP name, unsigned __int64 val)
    {
    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeInt64Value (XmlElement& elementPtr, WCharCP name, const __int64 val)
    {
    elementPtr->setAttribute (name, _variant_t (val));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static void     XmlHelper::AddAttributeBoolValue (XmlElement& elementPtr, WCharCP name, bool val)
    {
    AddAttributeStringValue (elementPtr, name, val?L"true":L"false");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     XmlHelper::GetNodeXmlText (XmlElement& elementPtr, WString& xmlText)
    {
    if (NULL == elementPtr)
        return false;

    xmlText = (const wchar_t*)elementPtr->xml;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BentleySystems
+---------------+---------------+---------------+---------------+---------------+------*/
static bool XmlHelper::Save (XmlDocument& pXMLDomPtr, WCharCP xmlFileName, WCharCP encoding=NULL)
    {
    HRESULT hr;
    MXWriterPtr pWriter(__uuidof(MSXML2::MXXMLWriter60));
    SAXXMLReader pReader(__uuidof(MSXML2::SAXXMLReader60));
    pReader->putContentHandler ((MSXML2::ISAXContentHandlerPtr)pWriter);
    pReader->putDTDHandler ((MSXML2::ISAXDTDHandlerPtr)pWriter);
    pReader->putErrorHandler ((MSXML2::ISAXErrorHandlerPtr)pWriter);

    pReader->putProperty ((unsigned short*)L"http://xml.org/sax/properties/lexical-handler", _variant_t(pWriter.GetInterfacePtr()));
    pReader->putProperty ((unsigned short*)L"http://xml.org/sax/properties/declaration-handler", _variant_t(pWriter.GetInterfacePtr()));

    if (!encoding)
        pWriter->put_encoding (L"UTF-8");

    pWriter->put_indent (VARIANT_TRUE);
    pWriter->put_byteOrderMark (VARIANT_TRUE);
    pWriter->put_omitXMLDeclaration (VARIANT_FALSE);
    pWriter->put_standalone (VARIANT_TRUE); // not dependent on external DTD

    DWORD grfMode = STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE;

    IStream* pStream;

    // ...create a new file opened for write, and return an IStream interface.
    hr = ::SHCreateStreamOnFileW(xmlFileName, grfMode, &pStream);
    if (SUCCEEDED(hr))
        {
        hr = pWriter->put_output (_variant_t(pStream));
        if (SUCCEEDED(hr))
            hr = pReader->parse ((_variant_t)pXMLDomPtr.GetInterfacePtr());
        }

    pStream->Release ();

    if (SUCCEEDED(hr))
        return true;

    return false;
    }

};
