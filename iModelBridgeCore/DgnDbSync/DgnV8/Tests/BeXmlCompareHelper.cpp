/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "BeXmlCompareHelper.h"
#include <algorithm>
#include <Bentley/BeConsole.h>
 
#pragma optimize( "", off )
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
XmlCompareOptions::XmlCompareOptions()
    {
    m_willSkipNode = false;
    m_willSkipAttribute = false;
    m_failOnFirst = false;
    m_matchChildren = false;
    }

XmlCompareOptions::~XmlCompareOptions()
{
    ResetAll();
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlCompareOptions::ResetAll()
    {
    m_willSkipNode = false;
    m_willSkipAttribute = false;
    m_failOnFirst = false;
    m_blackListNodes.clear();
    m_blackListAttributes.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlCompareOptions::AddSkipNodeName(BentleyApi::WString nodeName)
    {
    m_blackListNodes.push_back(nodeName);
    m_willSkipNode = true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlCompareOptions::ShouldSkipNode(BentleyApi::WString nodeName )
    {
    if ( m_blackListNodes.end() != std::find_if ( m_blackListNodes.begin(), m_blackListNodes.end(), [nodeName](BentleyApi::WString key) ->bool { return key == nodeName; } ) )
        return true;
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlCompareOptions::AddSkipAttributeCriteria(BentleyApi::WString nodeName , BentleyApi::WString attribName)
    {
    AttributeSkipCriteria newNode ;
    newNode.m_nodeName = nodeName;
    newNode.m_attributeName = attribName;
    m_blackListAttributes.push_back(newNode);
    m_willSkipAttribute = true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlCompareOptions::ShouldSkipAttribute(BentleyApi::WString nodeName , BentleyApi::WString attribName )
    {
    for each(XmlCompareOptions::AttributeSkipCriteria node in m_blackListAttributes)
        {
        if (node.m_nodeName == nodeName  && node.m_attributeName == attribName )
            return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCompare::SetComparisonOption(XmlCompareOptionsP options)
    {
    m_comparisonOptions = options;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCompare::RegisterComparisonFailedEvent( ComparisonFailedEvent failedEvent )
    {
    m_failedEvent = failedEvent;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCompare::Log(BentleyApi::WString message ) 
    {
    m_errorMessage += message + L"\nLocation:\n" + GetErrorLocation() + L"\n";
    fwprintf(stdout,L"Error: %s\nLocation:%s\n", message.c_str(),GetErrorLocation().c_str());
    m_isComparisonFailed = true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCompare::LogDebug(BentleyApi::WString message ) 
    {
    //fwprintf(stdout,L"Debug: %s\n", message.c_str());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString BeXmlCompare::GetErrorLocation()
    {
    BentleyApi::WString result;
    size_t size = m_path.size();
    for (unsigned int i = 0; i<size; i++)
        result  += m_path[i];
    return result;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlCompare::BeXmlCompare()
    {
    compareALL              = false;
    m_skipAttibute          = false;
    m_skipNode              = false;
    m_skipNodeCriteria      = L"";
    m_comparisonOptions     = NULL;
    m_failedEvent           = NULL;
    m_isComparisonFailed    = false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCompare::GetAttribList(BeXmlNodeR node, T_AttribMap  &attribMap)
    {
    LogDebug( L"GetAttribList");
    struct _xmlAttr *attrib =NULL;
    attrib = node.properties;
    while ( attrib )
        {
        BentleyApi::WString attribName;
        attribName.Sprintf(L"%hs",attrib->name);
        char *out = new char[attribName.size() + 1];
        attribName.ConvertToLocaleChars(out);
        BentleyApi::WString value = L"";
        node.GetAttributeStringValue(value, out);
        attribMap.Insert(attribName, value);
        attrib = attrib->next;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BeXmlCompare::CompareAttributes (BeXmlNodeR firstNode,BeXmlNodeR secondNode, BentleyApi::WString &nodeName )
    {
    LogDebug(L"CompareAttributes\n");
    T_AttribMap firstMap, secondMap;
    GetAttribList(firstNode, firstMap);
    GetAttribList(secondNode, secondMap);
    // Compare both
    for ( T_AttribMap::iterator iter = firstMap.begin(); iter != firstMap.end(); ++iter)
        {
        BentleyApi::WString currentKey = iter.key();
        T_AttribMap::iterator iter2 = secondMap.find(currentKey);
        if ( iter2 != secondMap.end() )
            {
            if (m_comparisonOptions && m_comparisonOptions->m_willSkipAttribute && m_comparisonOptions->ShouldSkipAttribute(nodeName, currentKey))
                {
                secondMap.erase(iter2);
                continue;
                }
            if ( firstMap[currentKey] != secondMap[currentKey] )
                {
                if ( ! m_failedEvent )
                    Log ( L"For Node:"+ nodeName +L" attribute "+ currentKey +L"'s value is not matching.\nActual: \"" + secondMap[currentKey] + L"\"\nExpected : \""+ firstMap[currentKey] + L"\"" );
                else if (  m_failedEvent(firstMap[currentKey],secondMap[currentKey],GetErrorLocation(), currentKey) == ERROR )
                    Log ( L"For Node:"+ nodeName +L" attribute "+ currentKey +L"'s value is not matching.\nActual: \"" + secondMap[currentKey] + L"\"\nExpected : \""+ firstMap[currentKey] + L"\"" );
                }
            secondMap.erase(iter2);
            }
        else //This attribute is missing in second node
            Log ( L"For Node:"+ nodeName +L" attribute "+ currentKey +L" is missing in second xml");
        }
    for ( T_AttribMap::iterator iter2 = secondMap.begin(); iter2 != secondMap.end(); ++iter2)
        {
        BentleyApi::WString currentKey = iter2.key();
        Log ( L"For Node:"+ nodeName +L" attribute "+ currentKey +L" is missing in first xml");
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BeXmlCompare::CompareText (BeXmlNodeP firstXMLElement,BeXmlNodeP secondXMLElement ,BentleyApi::WString nodeName)
    {
    BeXmlNodeP firstXMLNode = NULL , secondXMLNode = NULL;
    firstXMLNode  = firstXMLElement->GetFirstChild(BEXMLNODE_Text);
    secondXMLNode = secondXMLElement->GetFirstChild(BEXMLNODE_Text);

    if ((!firstXMLNode) && (!secondXMLNode))
        {
        return SUCCESS;
        }
    if ( ((!firstXMLNode) ^ (!secondXMLNode)))
        {
        Log(BentleyApi::WString(L"Text Not found ") + (!firstXMLNode ? L"in first xml" : L"in second xml"));
        return ERROR;
        }

    if (firstXMLNode->GetType() == BEXMLNODE_Text)
        {
        LogDebug(L"Text \n");
        m_path.push_back(L"/text()");
        BentleyApi::WString wsText1(L""),wsText2(L"");
        if (BEXML_Success == firstXMLNode->GetContent(wsText1) && BEXML_Success == secondXMLNode->GetXmlString(wsText2))
            {
            LogDebug(wsText1 + L":" + wsText2);
            if (false == wsText1.Equals(wsText2))
                {
                Log(L"Xml Node text is not matching.\nActual: \"" + wsText1 + +L"\"\nExpected : \"" + wsText2 + L"\"");
                m_path.pop_back();
                return ERROR;
                }
            m_path.pop_back();
            return SUCCESS;
            }
        else
            {
            Log(L"Unable to get Text from element.");
            m_path.pop_back();
            return ERROR;
            }
        }
    else
        printf("Not a Text Element");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Majd.Uddin   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlNodeP BeXmlCompare::FindMatchingNode(BeXmlNodeP parentNode, BentleyApi::WString nodeName)
{
    BeXmlNodeP foundNode = NULL;
    BeXmlNodeP child = parentNode->GetFirstChild();
    while (child)
    {
        Utf8CP childNodeName = child->GetName();
        BentleyApi::WString name(childNodeName, true);
        if (nodeName.Equals(name))
        {
            foundNode = child;
            break;
        }
        else
            child = child->GetNextSibling();
    }

    return foundNode;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BeXmlCompare::CompareNode(BeXmlNodeP firstXMLNode, BeXmlNodeP secondXMLNode, uint64_t count)
    {
    LogDebug(L"CompareNode\n");
    Utf8CP uName1 = firstXMLNode->GetName();
    Utf8CP uName2 = secondXMLNode->GetName();
    BentleyApi::WString n1(uName1 , true), n2(uName2 , true);
    if ( n1 != n2 )
        {
        Log( L"Xml Node names are not matching.\n Actual: \"" + n1 + + L"\" Expected : \""+ n2 + L"\"" );
        return ERROR; // XML Structure is different 
        }

    // return from here if the node matches the skip criteria
    if (m_comparisonOptions && m_comparisonOptions->ShouldSkipNode(n1))
        return SUCCESS;
    
    m_path.push_back(WPrintfString(L"/%s[%lld]",n1.c_str(), count).c_str());

    // Compare attributes for the element
    CompareAttributes(*firstXMLNode,*secondXMLNode,n1);
    // Compare text of element
    CompareText(firstXMLNode, secondXMLNode, n1);

    // Compare Childs
    BeXmlNodeP child1=NULL, child2=NULL;
    child1 = firstXMLNode->GetFirstChild (BEXMLNODE_Element);
    child2 = secondXMLNode->GetFirstChild (BEXMLNODE_Element);
    uint64_t childCounter = 0;
    while ( child1 && child2 )
        {
        Utf8CP child1NodeName = child1->GetName();
        Utf8CP child2NodeName = child2->GetName();
        BentleyApi::WString cn1(child1NodeName, true), cn2(child2NodeName, true);
        if (cn1 != cn2 && m_comparisonOptions->m_matchChildren) // Find matching child if next node is different
            {
            BentleyApi::WString nameToFind(child1NodeName, true);
            BeXmlNodeP matchingNode = FindMatchingNode(secondXMLNode, nameToFind);
            if (matchingNode != NULL)
                child2 = matchingNode;
            else
                Log(L"Xml Node couldn't be found in Actual. Expected Node name: \"" + nameToFind + L"\"");
            }
        CompareNode(child1, child2, childCounter);
        child1 = child1->GetNextSibling(BEXMLNODE_Element);
        child2 = child2->GetNextSibling(BEXMLNODE_Element);
        childCounter++;
        }
    //  If still some childs are left in either of them
    if (child1 || child2)
        {
        Log(BentleyApi::WString(L"More nodes presents in ") + (child1 ? L"first xml.": L"second xml"));
        }

    m_path.pop_back();
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BeXmlCompare::CompareXmlFiles (BentleyApi::WString firstFilePath , BentleyApi::WString secondFilePath)
    {
    BentleyApi::WString errorMsg = L"";
    BeXmlDomPtr dom1 , dom2 ;
    BeXmlStatus beStatus1 , beStatus2 ; 
    dom1 = BeXmlDom::CreateAndReadFromFile(beStatus1,firstFilePath.c_str(), &errorMsg);
    if ( beStatus1 != BEXML_Success )
        {
        Log( BentleyApi::WString(L"Unable to load file.\nERROR Details : ") + errorMsg );
        return ERROR;
        }
    dom2 = BeXmlDom::CreateAndReadFromFile(beStatus2,secondFilePath.c_str() ,&errorMsg);
    if ( beStatus2 != BEXML_Success )
        {
        Log( BentleyApi::WString(L"Unable to load file.\nERROR Details : ") + errorMsg );
        return ERROR;
        }

    if (SUCCESS != CompareXml(dom1,dom2) )
        return ERROR;
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BeXmlCompare::CompareXmlString (BentleyApi::WString firstString , BentleyApi::WString secondString)
    {
    BentleyApi::WString errorMsg = L"";
    BeXmlDomPtr dom1 , dom2 ;
    BeXmlStatus beStatus1 , beStatus2 ; 
    dom1 = BeXmlDom::CreateAndReadFromString(beStatus1,firstString.c_str(),0,&errorMsg);
    if ( beStatus1 != BEXML_Success )
        {
        Log( BentleyApi::WString(L"Unable to load Xml.\nERROR Details : ") + errorMsg );
        return ERROR;
        }
    dom2 = BeXmlDom::CreateAndReadFromString(beStatus2,secondString.c_str(),0,&errorMsg);
    if ( beStatus2 != BEXML_Success )
        {
        Log ( BentleyApi::WString(L"Unable to load Xml.\nERROR Details : ") + errorMsg );
        return ERROR;
        }

    if (SUCCESS != CompareXml(dom1,dom2) )
        return ERROR;
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                        Umar.Hayat 07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BeXmlCompare::CompareXml(BentleyApi::BeXmlDomPtr pXMLDom1, BentleyApi::BeXmlDomPtr pXMLDom2)
    {
    m_path.clear();
    BeXmlNodeP          root1 = pXMLDom1->GetRootElement ();
    BeXmlNodeP          root2 = pXMLDom2->GetRootElement ();
    if( root1 && root2 )    
        {
        if ( SUCCESS != CompareNode(root1,root2))
            return ERROR;
        }
    else
        {
        m_errorMessage = L"Unable to get root element";
        return ERROR;
        }
    return SUCCESS;
    }

#pragma optimize( "", on )