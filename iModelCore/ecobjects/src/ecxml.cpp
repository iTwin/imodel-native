/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecxml.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ctype.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseBooleanString (bool & booleanValue, Utf8CP booleanString)
    {
    if (0 == BeStringUtilities::Stricmp(booleanString, ECXML_TRUE))
        booleanValue = true;
    else if (0 == BeStringUtilities::Stricmp (booleanString, ECXML_FALSE))
        booleanValue = false;
    else
        return ECOBJECTS_STATUS_ParseError;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::GetPrimitiveTypeName (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return ECXML_TYPENAME_BINARY;
        case PRIMITIVETYPE_Boolean:
            return ECXML_TYPENAME_BOOLEAN;
        case PRIMITIVETYPE_DateTime:
            return ECXML_TYPENAME_DATETIME;
        case PRIMITIVETYPE_Double:
            return ECXML_TYPENAME_DOUBLE;
        case PRIMITIVETYPE_Integer:
            return ECXML_TYPENAME_INTEGER;
        case PRIMITIVETYPE_Long:
            return ECXML_TYPENAME_LONG;
        case PRIMITIVETYPE_Point2D:
            return ECXML_TYPENAME_POINT2D;
        case PRIMITIVETYPE_Point3D:
            return ECXML_TYPENAME_POINT3D;
        case PRIMITIVETYPE_String:
            return ECXML_TYPENAME_STRING;
        case PRIMITIVETYPE_IGeometry:
            return ECXML_TYPENAME_IGEOMETRY_LEGACY_GENERIC;
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParsePrimitiveType (PrimitiveType& primitiveType, Utf8StringCR typeName)
    {
    if (0 == typeName.length())
        return ECOBJECTS_STATUS_ParseError;

    if (0 == typeName.compare (ECXML_TYPENAME_STRING))
        primitiveType = PRIMITIVETYPE_String;
    else if (0 == typeName.compare (ECXML_TYPENAME_INTEGER))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (0 == typeName.compare (ECXML_TYPENAME_LONG))
        primitiveType = PRIMITIVETYPE_Long;
    else if (0 == typeName.compare (ECXML_TYPENAME_BOOLEAN))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (0 == typeName.compare (ECXML_TYPENAME_BOOL))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (0 == typeName.compare (ECXML_TYPENAME_DOUBLE))
        primitiveType = PRIMITIVETYPE_Double;
    else if (0 == typeName.compare (ECXML_TYPENAME_POINT2D))
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (0 == typeName.compare (ECXML_TYPENAME_POINT3D))
        primitiveType = PRIMITIVETYPE_Point3D;
    else if (0 == typeName.compare (ECXML_TYPENAME_DATETIME))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (0 == typeName.compare (ECXML_TYPENAME_BINARY))
        primitiveType = PRIMITIVETYPE_Binary;
    else if (0 == typeName.compare(0, strlen(ECXML_TYPENAME_IGEOMETRY_GENERIC), ECXML_TYPENAME_IGEOMETRY_GENERIC))
        primitiveType = PRIMITIVETYPE_IGeometry; 
    else if (0 == typeName.compare(0, strlen(ECXML_TYPENAME_IGEOMETRY_LEGACY), ECXML_TYPENAME_IGEOMETRY_LEGACY))
        primitiveType = PRIMITIVETYPE_IGeometry; 
    else
        return ECOBJECTS_STATUS_ParseError;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::StrengthToString (StrengthType strength)
    {
    switch (strength)
        {
        case STRENGTHTYPE_Referencing :
            return ECXML_STRENGTH_REFERENCING;
        case STRENGTHTYPE_Holding:
            return ECXML_STRENGTH_HOLDING;
        case STRENGTHTYPE_Embedding:
            return ECXML_STRENGTH_EMBEDDING;
        default:
            return EMPTY_STRING;
           
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseStrengthType (StrengthType& strength, Utf8StringCR strengthString)
    {
    if (0 == strengthString.length())
        return ECOBJECTS_STATUS_ParseError;
    if (0 == strengthString.compare(ECXML_STRENGTH_EMBEDDING))
        strength = STRENGTHTYPE_Embedding;
    else if (0 == strengthString.compare(ECXML_STRENGTH_HOLDING))
        strength = STRENGTHTYPE_Holding;
    else if (0 == strengthString.compare(ECXML_STRENGTH_REFERENCING))
        strength = STRENGTHTYPE_Referencing;
    else
        return ECOBJECTS_STATUS_ParseError;
        
    return ECOBJECTS_STATUS_Success;
    }
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECXml::DirectionToString (ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward :
            return ECXML_DIRECTION_FORWARD;
        case ECRelatedInstanceDirection::Backward:
            return ECXML_DIRECTION_BACKWARD;
        default:
            return EMPTY_STRING;
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseDirectionString (ECRelatedInstanceDirection& direction, Utf8StringCR directionString)
    {
    if (0 == directionString.length())
        return ECOBJECTS_STATUS_ParseError;
    if (0 == directionString.compare(ECXML_DIRECTION_BACKWARD))
        direction = ECRelatedInstanceDirection::Backward;
    else if (0 == directionString.compare(ECXML_DIRECTION_FORWARD))
        direction = ECRelatedInstanceDirection::Forward;
    else
        return ECOBJECTS_STATUS_ParseError;
        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECXml::ParseCardinalityString (uint32_t &lowerLimit, uint32_t &upperLimit, const Utf8String &cardinalityString)
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    if (0 == cardinalityString.compare("1"))
        {
        LOG.debugv("Legacy cardinality of '1' interpreted as '(1,1)'");
        lowerLimit = 1;
        upperLimit = 1;
        return status;
        }
        
    if ((0 == cardinalityString.compare("UNBOUNDED")) || (0 == cardinalityString.compare("Unbounded")) ||
             (0 == cardinalityString.compare("unbounded")) || (0 == cardinalityString.compare("n")) ||
             (0 == cardinalityString.compare("N")))
        {
        LOG.debugv("Legacy cardinality of '%s' interpreted as '(0,n)'", cardinalityString.c_str());
        lowerLimit = 0;
        upperLimit = UINT_MAX;
        return status;
        }
    
    Utf8String cardinalityWithoutSpaces = cardinalityString;
    cardinalityWithoutSpaces.erase(std::remove_if(cardinalityWithoutSpaces.begin(), cardinalityWithoutSpaces.end(), ::isspace), cardinalityWithoutSpaces.end()); 
    size_t openParenIndex = cardinalityWithoutSpaces.find('(');
    if (openParenIndex == std::string::npos)
        {
        if (0 == BE_STRING_UTILITIES_UTF8_SSCANF(cardinalityWithoutSpaces.c_str(), "%d", &upperLimit))
            return ECOBJECTS_STATUS_ParseError;
        LOG.debugv("Legacy cardinality of '%d' interpreted as '(0,%d)'", upperLimit, upperLimit);
        lowerLimit = 0;
        return status;
        }
        
    if (openParenIndex != 0 && cardinalityWithoutSpaces.find(')') != cardinalityWithoutSpaces.length() - 1)
        {
        LOG.warningv("Cardinality string '%s' is invalid.", cardinalityString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }
     
    int scanned = BE_STRING_UTILITIES_UTF8_SSCANF(cardinalityWithoutSpaces.c_str(), "(%d,%d)", &lowerLimit, &upperLimit);
    if (2 == scanned)
        return ECOBJECTS_STATUS_Success;
        
    if (0 == scanned)
        {
        LOG.warningv("Cardinality string '%s' is invalid.", cardinalityString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }
    
    // Otherwise, we just assume the upper limit is 'n' or 'N' and is unbounded
    upperLimit = UINT_MAX;
    return status;
    }

#if defined (DONT_THINK_WE_NEED)

void FormatXmlNode (MSXML2::IXMLDOMNode& domNode, uint32_t indentLevel)
    {
    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    if (domNode.nodeType == NODE_TEXT)
        return;
    
    bool textOnly = true;
    if (domNode.hasChildNodes())
        {
        MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = domNode.childNodes;
        MSXML2::IXMLDOMNodePtr xmlNodePtr;
        while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()) && textOnly)
            {
            if (xmlNodePtr->nodeType != NODE_TEXT)
                textOnly = false;
            }
        }
        
    if (domNode.hasChildNodes())
        {
        // Add a newline before the children
        if (!textOnly)
            {
            textPtr = domNode.ownerDocument->createTextNode("\n");
            domNode.insertBefore(textPtr, _variant_t(domNode.firstChild.GetInterfacePtr()));
            }
            
        // Format the children
        MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = domNode.childNodes;
        MSXML2::IXMLDOMNodePtr xmlNodePtr;
        while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
            {
            FormatXmlNode(*xmlNodePtr, indentLevel + 4);
            }
        }
        
    // Format this element
    if (indentLevel > 0)
        {
        char *spaces = new char[indentLevel+1];
        for (uint32_t i = 0; i < indentLevel; i++)
            spaces[i] = ' ';
        spaces[indentLevel] = '\0';
        // Indent before this element
        textPtr = domNode.ownerDocument->createTextNode(spaces);
        domNode.parentNode->insertBefore(textPtr, _variant_t(&domNode));

        // Indent after the last child node
        if (!textOnly)
            {
            textPtr = domNode.ownerDocument->createTextNode(spaces);
            domNode.appendChild(textPtr);
            }
        
        textPtr = domNode.ownerDocument->createTextNode("\n");
        IXMLDOMNodePtr sibling = domNode.nextSibling;
        if (NULL == sibling)
            domNode.parentNode->appendChild(textPtr);
        else
            domNode.parentNode->insertBefore(textPtr, _variant_t(sibling.GetInterfacePtr()));
        delete spaces;
        }    
    }
    
#endif // defined (_WIN32) // WIP_NONPORT 


void ECXml::FormatXml (BeXmlDomR pXmlDoc)
    {
    // For now, do nothing. I think this can be controlled using xmlSaveCtxt
    // FormatXmlNode(pXmlDoc->documentElement, 0);
    }


END_BENTLEY_ECOBJECT_NAMESPACE