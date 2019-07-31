/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <BeXml/BeXml.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>

typedef BentleyApi::StatusInt ( *ComparisonFailedEvent )
    (
    BentleyApi::WString       value1,
    BentleyApi::WString       value2,
    BentleyApi::WString       currentLocation,
    BentleyApi::WString       currentKey
    );


typedef BentleyApi::bmap< BentleyApi::WString, BentleyApi::WString> T_AttribMap;

/*=================================================================================**//**
* @bsiclass                                             BentleySystems  
+===============+===============+===============+===============+===============+======*/
struct XmlCompareOptions
    {
public:
    // If true Xml comparison will stop comparison on first failure , 
    // otherwise it will log the error info and continue comparison
    bool m_failOnFirst;
    bool m_willSkipNode;
    bool m_willSkipAttribute;
    bool m_matchChildren;
    struct AttributeSkipCriteria
        {
        BentleyApi::WString m_nodeName;
        BentleyApi::WString m_attributeName;
        };
    // List of Attributes that will be ignore completely during comparison
    BentleyApi::bvector<AttributeSkipCriteria> m_blackListAttributes;
    // List Nodes that will be ignore completely during comparison
    BentleyApi::bvector<BentleyApi::WString> m_blackListNodes;

    // Default constructor
    XmlCompareOptions();
    ~XmlCompareOptions();
    // Add Node Name that need to be ignored during comparison
    void AddSkipNodeName(BentleyApi::WString);
    // Query if the provided attribute need to be ignored
    bool ShouldSkipNode(BentleyApi::WString nodeName );
    // Add Node Name that need to be ignored during comparison
    void AddSkipAttributeCriteria(BentleyApi::WString nodeName , BentleyApi::WString attributeName);
    // Query if the provided attribute need to be ignored
    bool ShouldSkipAttribute(BentleyApi::WString nodeName , BentleyApi::WString attributeName );
    // reset all Options to default value and clear all the lists
    void ResetAll();
    };

typedef XmlCompareOptions* XmlCompareOptionsP;

/*=================================================================================**//**
* @bsiclass                                             BentleySystems  
+===============+===============+===============+===============+===============+======*/
struct BeXmlCompare
    {
private:
    BentleyApi::WString m_errorMessage;
    BentleyApi::bvector<BentleyApi::WString> m_path;
    bool compareALL;
    bool m_skipAttibute;
    bool m_skipNode;
    ComparisonFailedEvent m_failedEvent;
    
public:
    XmlCompareOptionsP m_comparisonOptions;
    bool m_isComparisonFailed;
public:
    struct skipCriteria
        {
        BentleyApi::WString m_nodeName;
        BentleyApi::WString m_attributeName;
        } m_skipCriteria;
    BentleyApi::WString m_skipNodeCriteria;

    BeXmlCompare();
    BentleyApi::WString     GetError(){return m_errorMessage;};
    BentleyApi::WString     GetErrorLocation();
    bool        getSkipAttribute(){ return m_skipAttibute;}
    void        setSkipAttribute(bool flag){ m_skipAttibute = flag;} 
    bool        getSkipNode(){ return m_skipNode;}
    void        setSkipNode(bool flag){ m_skipNode = flag;} 
    BentleyApi::StatusInt   CompareAttributes(BeXmlNodeR firstNode, BeXmlNodeR secondNode, BentleyApi::WString &nodeName);
    BentleyApi::StatusInt   CompareText (BeXmlNodeP firstXMLNode,BeXmlNodeP secondXMLNode ,BentleyApi::WString nodeName);
    BentleyApi::StatusInt   CompareXmlFiles (BentleyApi::WString firstFilePath , BentleyApi::WString secondFilePath);
    BentleyApi::StatusInt   CompareXml(BentleyApi::BeXmlDomPtr pXMLDom1, BentleyApi::BeXmlDomPtr pXMLDom2);
    BentleyApi::StatusInt   CompareNode(BeXmlNodeP firstXMLNode, BeXmlNodeP secondXMLNode, uint64_t count = 0);

    BentleyApi::StatusInt   CompareXmlString (BentleyApi::WString firstString , BentleyApi::WString secondString);
    void        RegisterComparisonFailedEvent( ComparisonFailedEvent failedEvent );
    void        SetComparisonOption(XmlCompareOptions* options);
    BeXmlNodeP  FindMatchingNode(BeXmlNodeP parentNode, BentleyApi::WString nodeName);
private:
    void        Log(BentleyApi::WString message);
    void        LogDebug(BentleyApi::WString message);
    void        GetAttribList(BeXmlNodeR node, T_AttribMap &list);
    void        ReportMissingAttrib(bvector<BentleyApi::WString> &list1, BentleyApi::bvector<BentleyApi::WString> &list2);
    };
