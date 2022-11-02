/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <Bentley/BeStringUtilities.h>

#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/RefCounted.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <map>
#include <Bentley/WString.h>

#if defined (__BEXMLDLL_BUILD__)
    #define BEXMLDLL_EXPORT EXPORT_ATTRIBUTE
#else
    #define BEXMLDLL_EXPORT IMPORT_ATTRIBUTE
#endif

BENTLEY_NAMESPACE_TYPEDEFS (BeXmlDom)
BENTLEY_NAMESPACE_TYPEDEFS (BeXmlReader)
BENTLEY_NAMESPACE_TYPEDEFS (BeXmlWriter)
BENTLEY_NAMESPACE_TYPEDEFS (BeXmlNode)
BENTLEY_NAMESPACE_TYPEDEFS (IBeXmlWriter)

BEGIN_BENTLEY_NAMESPACE

typedef RefCountedPtr<BeXmlDom>       BeXmlDomPtr;
typedef RefCountedPtr<BeXmlReader>    BeXmlReaderPtr;
typedef RefCountedPtr<BeXmlWriter>    BeXmlWriterPtr;

//! Possible status values returned from BeLibxml methods.
#define BEXML_ERROR_BASE    0x1000
enum BeXmlStatus
    {
    BEXML_Success                   = 0,                            //!< The method succeeded
    BEXML_FileNotFound              = BEXML_ERROR_BASE + 1,
    BEXML_ParseError                = BEXML_ERROR_BASE + 2,         //!< Parse error
    BEXML_ReadError                 = BEXML_ERROR_BASE + 3,         //!< Reader error
    BEXML_AttributeNotFound         = BEXML_ERROR_BASE + 4,
    BEXML_UnexpectedValue           = BEXML_ERROR_BASE + 5,
    BEXML_UnownedPath               = BEXML_ERROR_BASE + 6,
    BEXML_NodeNotFound              = BEXML_ERROR_BASE + 7,
    BEXML_MultipleNodes             = BEXML_ERROR_BASE + 8,
    BEXML_ArgumentError             = BEXML_ERROR_BASE + 9,
    BEXML_NotElementNode            = BEXML_ERROR_BASE + 10,
    BEXML_NoMoreAttributes          = BEXML_ERROR_BASE + 11,
    BEXML_NullNodeName              = BEXML_ERROR_BASE + 12,
    BEXML_NullNodeValue             = BEXML_ERROR_BASE + 13,
    BEXML_NullOuterXml              = BEXML_ERROR_BASE + 14,
    BEXML_CantWrite                 = BEXML_ERROR_BASE + 15,
    BEXML_UnimplementedType         = BEXML_ERROR_BASE + 16,
    BEXML_ContentWrongType          = BEXML_ERROR_BASE + 17,
    BEXML_CDATANotFound             = BEXML_ERROR_BASE + 18,
    BEXML_ErrorWritingToFile        = BEXML_ERROR_BASE + 19,
    BEXML_InvalidEncoding           = BEXML_ERROR_BASE + 20,
    BEXML_NullInnerXml              = BEXML_ERROR_BASE + 21,
    };

enum BeXmlNodeType
    {
    BEXMLNODE_Any               =   0,
    BEXMLNODE_Element           =   XML_ELEMENT_NODE,
    BEXMLNODE_Attribute         =   XML_ATTRIBUTE_NODE,
    BEXMLNODE_Text              =   XML_TEXT_NODE,
    BEXMLNODE_Section           =   XML_CDATA_SECTION_NODE,
    BEXMLNODE_Reference         =   XML_ENTITY_REF_NODE,
    BEXMLNODE_Entity            =   XML_ENTITY_NODE,
    BEXMLNODE_PI                =   XML_PI_NODE,
    BEXMLNODE_Comment           =   XML_COMMENT_NODE,
    BEXMLNODE_Document          =   XML_DOCUMENT_NODE,
    BEXMLNODE_DocumentType      =   XML_DOCUMENT_TYPE_NODE,
    BEXMLNODE_DocumentFragment  =   XML_DOCUMENT_FRAG_NODE,
    BEXMLNODE_Notation          =   XML_NOTATION_NODE,
    BEXMLNODE_HTMLDocument      =   XML_HTML_DOCUMENT_NODE,
    BEXMLNODE_DTD               =   XML_DTD_NODE,
    BEXMLNODE_ElementDecl       =   XML_ELEMENT_DECL,
    BEXMLNODE_AttributeDecl     =   XML_ATTRIBUTE_DECL,
    BEXMLNODE_EntityDecl        =   XML_ENTITY_DECL,
    BEXMLNODE_NamespaceDecl     =   XML_NAMESPACE_DECL,
    BEXMLNODE_XIncludeStart     =   XML_XINCLUDE_START,
    BEXMLNODE_XIncludeEnd       =   XML_XINCLUDE_END,
    };

//=======================================================================================
//! A basic wrapper around xmlDocPtr and some related DOM and XPath operations to provide automatic memory management, as well as some other low-level utilities.
//! @note At this time, your library must manually call LIBXML_TEST_VERSION (from BeLibxml2) before using this wrapper (or any other BeLibxml2 functionality).
//
// ***This class is meant to be a relatively basic wrapper. While you may enhance it as required, it should NOT be a dumping ground for XML utilities.***
//
// @bsiclass
//=======================================================================================
struct BeXmlDom : public RefCountedBase, public NonCopyableClass
    {
    friend struct BeXmlNode;
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    public: enum ToStringOption ENUM_UNDERLYING_TYPE(uint64_t)
        {
        TO_STRING_OPTION_Default            = 0,            // no indent, no carriage returns.
        TO_STRING_OPTION_OmitXmlDeclaration = (1 << 0),
                                                            // no longer supported: TO_STRING_OPTION_OmitByteOrderMark  = (1 << 1); BOM is never emitted by BeXml... it's up to you to do that when you persist in your particular format.
        TO_STRING_OPTION_Formatted          = (1 << 2),     // has carriage returns.
        TO_STRING_OPTION_Indent             = (1 << 3),     // has 2 character indentation. Valid only if TO_STRING_OPTION_Formatted also set.
        }; // ToStringOption
    
    public: enum FileEncodingOption
        {
        FILE_ENCODING_Utf16                 = 0,
        FILE_ENCODING_Utf8                  = 1,
        }; // FileEncodingOption

    public: enum ParseOptions
        {
        XMLPARSE_OPTION_None                        = 0,
        XMLPARSE_OPTION_FixDuplicateAttributes      = (1 << 0),
        XMLPARSE_OPTION_AssertOnParseError          = (1 << 1), // applies only to debug builds.
        };

    //-----------------------------------------------------------------------------------------------------------------------------------------------
private:
    xmlDocPtr                       m_doc;
    bvector<xmlXPathContextPtr>     m_xpathContexts;
    bvector<xmlXPathObjectPtr>      m_xpathObjects;
    xmlError                        m_error;
    bmap<Utf8String, Utf8String>    m_namespaces;
    ParseOptions                    m_options;

    static BeXmlDomPtr CreateAndReadFromString (BeXmlStatus& xmlStatus, CharCP buffer, size_t bufferSize, CharCP encoding);
    BeXmlDom();

protected:  
    ~BeXmlDom();

    //! Gets a direct reference to this wrapper's document.
public: 
    BEXMLDLL_EXPORT xmlDoc& GetDocument ();
    private:    BeXmlDom     (ParseOptions options);
    
    private: static BeXmlDomPtr CreateAndReadFromString (BeXmlStatus& xmlStatus, CharCP source, size_t characterCount, CharCP encoding, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);
    public:  static void        FormatErrorMessage (WStringP errorMsg, xmlErrorPtr xmlError);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Basic memory management

    //! Attempts to open and parse the XML document at the given file path.
    //! @param[out] status      BEXML_Success if the file was opened and parsed successfully. Otherwise, BEXML_FileNotFound or BEXML_ParseError.
    //! @param[in]  fileName    The name of the file to parse.
    //! @param[out] errorMsg    Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in]  options     Optional. If supplied, specifies parsing options.
    //! @return NULL if the file cannot be found or sucessfully parsed; a valid DOM wrapper otherwise.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromFile (BeXmlStatus& status, WCharCP fileName, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Attempts to open and parse the XML document at the given file path.
    //! @param[out] status      BEXML_Success if the file was opened and parsed successfully. Otherwise, BEXML_FileNotFound or BEXML_ParseError.
    //! @param[in]  fileName    The name of the file to parse.
    //! @param[out] errorMsg    Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in]  options     Optional. If supplied, specifies parsing options.
    //! @return NULL if the file cannot be found or sucessfully parsed; a valid DOM wrapper otherwise.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromFile (BeXmlStatus& status, Utf8CP fileName, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Attempts to parse the XML document provided.
    //! @return NULL if the buffer cannot be sucessfully parsed; a valid DOM wrapper otherwise.
    //! @param[out] status  BEXML_Success if the buffer was parsed successfully. Otherwise, BEXML_ParseError.
    //! @param[in]  xmlBuffer   The buffer to parse
    //! @param[in]  bufferSize  The size of the buffer to parse
    //! @param[out] errorMsg    Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in]  options     Optional. If supplied, specifies parsing options.
    //! @note bufferSize is in bytes.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromMemory(BeXmlStatus& status, void const* xmlBuffer, size_t bufferSize, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Attempts to parse the UTF-16 XML fragment provided.
    //! @param[out] xmlStatus   The result of the attempt to read an XML DOM from the specified string.
    //! @param[in]  source      The Utf16-encoded string from which the XML DOM is parsed.
    //! @param[in]  utf16Count  Optional, The number of UInt16s in the input Utf16 string. You can get this from wcslen - don't use utf16len!
    //! @param[out] errorMsg    Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in]  options     Optional. If supplied, specifies parsing options.
    //! @return NULL if the fragment cannot be sucessfully parsed; a valid DOM wrapper otherwise.
    //! @note If you already know the number of UInt16 in the input string, pass it in as utf16Count to save the work of getting that length again. Otherwise pass 0.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromString (BeXmlStatus& xmlStatus, Utf16CP source, size_t utf16Count=0, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Attempts to parse the XML fragment provided. Unless specified in the XML stream, the encoding is assumed to be UTF-16 on Windows, and UCS-4 on other platforms.
    //! @param[out] xmlStatus       The result of the attempt to read an XML DOM from the specified string.
    //! @param[in]  source          The WChar string from which the XML DOM is parsed.
    //! @param[in]  characterCount  Optional, The length (in characters) of the input Utf16 string.
    //! @param[out] errorMsg        Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in]  options         Optional. If supplied, specifies parsing options.
    //! @return NULL if the fragment cannot be sucessfully parsed; a valid DOM wrapper otherwise.
    //! @note If you already know the number of characters in the input string, pass it in as characterCount to save the work of getting the string length again. Otherwise pass 0.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromString (BeXmlStatus& xmlStatus, WCharCP source, size_t characterCount=0, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Attempts to parse the UTF-8 XML fragment provided.
    //! @param[out] xmlStatus       The result of the attempt to read an XML DOM from the specified string.
    //! @param[in] source           The Utf8-encoded string from which the XML DOM is parsed.
    //! @param[in] characterCount   Optional, The length (in characters) of the input Utf8 string.
    //! @param[out] errorMsg        Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @param[in] options          Optional. If supplied, specifies parsing options.
    //! @return NULL if the fragment cannot be sucessfully parsed; a valid DOM wrapper otherwise.
    //! @note If you already know the number of characters in the input string, pass it in as characterCount to save the work of getting the string length again. Otherwise pass 0.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr  CreateAndReadFromString (BeXmlStatus& xmlStatus, Utf8CP source, size_t characterCount = 0, WStringP errorMsg = NULL, ParseOptions options = XMLPARSE_OPTION_None);

    //! Gets the last error message from libxml, which will be valid any time one of the CreateAndReadFromXxxx methods returns BEXML_ParseError.
    //! @note The error message can also be returned to any of the CreateAndReadFromXxxx methods by passing a non-NULL errorMsg parameter.
    public: BEXMLDLL_EXPORT static void         GetLastErrorString (WStringR errorMessage);

    //! Creates a new, empty, DOM.
    public: BEXMLDLL_EXPORT static BeXmlDomPtr CreateEmpty ();

    //! Register a namespace expected in the DOM. Once registered, the prefix can be used within XPath expressions.
    public: BEXMLDLL_EXPORT void RegisterNamespace (Utf8CP prefix, Utf8CP xmlNamespace);

    //! Allocates and retains an xmlXPathContextPtr.
    //! @note You can pass NULL for the root parameter to start at the root node of the document.
    //! @note Do <b>not</b> manually free the xmlXPathContextPtr; if you wish to free it deterministically, use FreeXPathContext. Otherwise, it is freed later with this object.
    public: BEXMLDLL_EXPORT xmlXPathContextPtr AcquireXPathContext (BeXmlNodeP root);

    //! Deterministically frees an xmlXPathContextPtr allocated with AcquireXPathContext.
    //! @return ERROR if the object was not owned by this DOM wrapper; SUCCESS otherwise.
    //! @note All xmlXPathContextPtr objects acquired with AcquireXPathContext will automatically be freed with this object; you can use this manually, however, if you wish to deterministically clean up.
    public: BEXMLDLL_EXPORT BeXmlStatus FreeXPathContext (xmlXPathContext&);

    //! Evaluates an XPath query.
    //! @note If the provided context is NULL, the most recently allocated context is used; if no contexts are currently allocated, a new blank one is allocated.
    //! @note Do <b>not</b> manually free the xmlXPathObjectPtr; if you wish to free it deterministically, use FreeXPathObject. Otherwise, it is freed later with this object.
    public: BEXMLDLL_EXPORT xmlXPathObjectPtr   EvaluateXPathExpression (Utf8CP expression, xmlXPathContextPtr);

    //! Deterministically frees an xmlXPathObjectPtr allocated with EvaluateXPathExpression.
    //! @return ERROR if the object was not owned by this DOM wrapper; SUCCESS otherwise.
    //! @note All xmlXPathObjectPtr objects acquired with EvaluateXPathExpression will automatically be freed with this object; you can use this manually, however, if you wish to deterministically clean up.
    public: BEXMLDLL_EXPORT BeXmlStatus FreeXPathObject (xmlXPathObject&);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Errors

    //! Returns the line number and column of the last error. Both are set to 0 if there is no error.
    public: BEXMLDLL_EXPORT void GetErrorLocation (int& lineNumber, int& column);

    //! Returns the LibXML error message
    public: BEXMLDLL_EXPORT void GetErrorMessage (WStringR message);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Higher level DOM utilities

    //! Provides the root node of the document. Can be NULL (if you created a blank DOM and haven't added one yet).
    public: BEXMLDLL_EXPORT BeXmlNodeP GetRootElement ();

    //! Creates a new element node with the given name and optional content, then adds it to the provided parent. If the parent is NULL, it is set as the root of the document (potentially replacing an existing root).
    //! @return NULL if name is NULL or empty; the new node otherwise.
    //! @note This wrapper will <b>not</b> directly manage the memory allocated; you must either add it to a document (which will automatically manage it), or manually call xmlFree. Passing a NULL parent implicitly adds it to the document, thus managing its memory. Adding it to a parent already in a document has the same effect.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddNewElement (Utf8CP name, WCharCP content, BeXmlNodeP parent);

    //! Generates a string representation of this DOM.
    public: BEXMLDLL_EXPORT void ToString (Utf16BufferR, ToStringOption) const;

    //! Generates a WString representation of this DOM.
    public: BEXMLDLL_EXPORT void ToString (WStringR, ToStringOption) const;

    //! Generates a Utf8String representation of this DOM.
    public: BEXMLDLL_EXPORT void ToString (Utf8StringR, ToStringOption) const;

    //! Writes this DOM to a file.
    public: BEXMLDLL_EXPORT BeXmlStatus         ToFile (WStringCR fileName, ToStringOption, FileEncodingOption encoding);


    public: ParseOptions                        GetOptions ();
    
    //! Validate this document against the provided XSD. 
    //! @return BEXML_Success or BEXML_ParseError. On error, you can call GetLastErrorString().
    public: BEXMLDLL_EXPORT BeXmlStatus SchemaValidate(WCharCP xsdSchemaFileName);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Higher level XPath utilities

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    public: enum NodeBias
        {
        NODE_BIAS_First,
        NODE_BIAS_Last,
        NODE_BIAS_FailIfMultiple
        
        }; // NodeBias

    public: struct IterableNodeSet;

    //=======================================================================================
    //! A utility class so for-each can be used across an xmlXPathObject.
    //! @see GetNodesMatchingXPath, for example.
    // @bsiclass
    //=======================================================================================
    public: struct IterableNodeSetIter
        {
    using iterator_category=std::forward_iterator_tag;
    using value_type=BeXmlNodeP;
    using difference_type=std::ptrdiff_t;
    using pointer=BeXmlNodeP*;
    using reference=BeXmlNodeP&;

        friend struct IterableNodeSet;

        private:    xmlNodeSetPtr   m_nodeSet;
        private:    size_t          m_index;
        
        private:    BEXMLDLL_EXPORT                          IterableNodeSetIter (xmlNodeSetPtr, size_t index);

        public:     BEXMLDLL_EXPORT  IterableNodeSetIter&    operator++          ();
        public:     BEXMLDLL_EXPORT  BeXmlNodeP&             operator*           ();
        public:     BEXMLDLL_EXPORT  bool                    operator==          (IterableNodeSetIter const&) const;
        public:     BEXMLDLL_EXPORT  bool                    operator!=          (IterableNodeSetIter const&) const;

        }; // IterableNodeSetIter
    
    //=======================================================================================
    //! A utility class so for-each can be used across an xmlXPathObject.
    //! @see GetNodesMatchingXPath, for example.
    // @bsiclass
    //=======================================================================================
    public: struct IterableNodeSet : public NonCopyableClass
        {
        private:    BeXmlDomPtr   m_dom;
        private:    xmlXPathObjectPtr       m_result;
        private:    xmlNodeSetPtr           m_nodeSet;
        
        public: typedef IterableNodeSetIter iterator;
        public: typedef IterableNodeSetIter const_iterator;
        
        public:     BEXMLDLL_EXPORT                IterableNodeSet     ();
        public:     BEXMLDLL_EXPORT                ~IterableNodeSet    ();

        public:     BEXMLDLL_EXPORT    void        Init                (xmlNodeSetPtr);
        public:     BEXMLDLL_EXPORT    void        Init                (BeXmlDomR, xmlXPathObjectPtr);
        public:     BEXMLDLL_EXPORT    iterator    begin               ();
        public:     BEXMLDLL_EXPORT    iterator    end                 ();
        public:     BEXMLDLL_EXPORT    size_t      size                () const;
        public:     BEXMLDLL_EXPORT    BeXmlNodeP& front               ();
        public:     BEXMLDLL_EXPORT    BeXmlNodeP& back                ();
        
        }; // IterableNodeSet

    //! Gets the node found via an XPath query (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNode (BeXmlNodeP&, Utf8CP expression, xmlXPathContextPtr, NodeBias);
    
    //! Gets the content of the node found via an XPath query (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise, even if content was empty.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    //! @note The result is always cleared, even in an error state.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNodeContent (WStringR resultContent, Utf8CP expression, xmlXPathContextPtr, NodeBias);
    
    //! Gets the content of the node found via an XPath query (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise, even if content was empty.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    //! @note The result is always cleared, even in an error state.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNodeContent (Utf8StringR resultContent, Utf8CP expression, xmlXPathContextPtr, NodeBias);

    //! Gets the content of the node found via an XPath query as a UInt32 (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise, even if content was empty.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    //! @note This method first looks for a "0x" or "0X" prefix to detect hex values.
    //! @note The result is always set to 0, even in an error state.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNodeContentAsUInt32 (uint32_t& resultContent, Utf8CP expression, xmlXPathContextPtr, NodeBias);

    //! Gets the content of the node found via an XPath query as a UInt16 (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise, even if content was empty.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    //! @note This method first looks for a "0x" or "0X" prefix to detect hex values.
    //! @note The result is always set to 0, even in an error state.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNodeContentAsUInt16 (uint16_t& resultContent, Utf8CP expression, xmlXPathContextPtr, NodeBias);

    //! Gets the content of the node found via an XPath query as a boolean (according to the node bias if more than one exist).
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise, even if content was empty.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    //! @note This method does a case insensitive compare to L"true" and L"1"; if that succeeds, the result is considered true, otherwise false.
    //! @note The result is always set to false, even in an error state.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectNodeContentAsBool (bool& resultContent, Utf8CP expression, xmlXPathContextPtr, NodeBias);

    //! Gets the set of all nodes matching the given XPath query.
    //! @note The behavior of a NULL xmlXPathContextPtr is explained in EvaluateXPathExpression.
    public: BEXMLDLL_EXPORT void SelectNodes (IterableNodeSet&, Utf8CP expression, xmlXPathContextPtr);

    //! Gets the child node found by-name (according to the node bias if more than one exists). If no name is provided, only the bias is used to find a node.
    //! @return ERROR if no nodes were found, or the found node was not an element; SUCCESS otherwise.
    public: BEXMLDLL_EXPORT BeXmlStatus SelectChildNodeByName (BeXmlNodeP& result, BeXmlNodeR parent, Utf8CP childName, NodeBias);
    
    }; // BeXmlDom


//=======================================================================================
//! A basic wrapper around xmlNode that adds methods. The actual memory is in the DOM that is built by libxml, 
// @bsiclass
//=======================================================================================
struct  BeXmlNode : public xmlNode
    {
    // NO MEMBERS CAN BE ADDED! NO INSTANCES CAN BE CREATED!
    private:    BeXmlNode();
    private:   ~BeXmlNode();

    //! Adds a Child Node, used internally.
    private: BEXMLDLL_EXPORT BeXmlNodeP AddChildNode (BeXmlNodeR childNode);

    //! Gets the node name.
    //! Note: The name is actually UTF8, but node names are customarily within the ASCII range.
    public: BEXMLDLL_EXPORT Utf8CP GetName();

    //! Gets the node type.
    public: BEXMLDLL_EXPORT BeXmlNodeType GetType();

    //! Adds a new namespace for this node. It doesn't set the node namespace but simply add a reference. If you want
    //! to set the node namespace use SetNamespace without calling AddNamespace.
    //! To create a default namespace 'prefix' must be NULL.
    public: BEXMLDLL_EXPORT void AddNamespace (Utf8CP prefix, Utf8CP uri);

    //! Gets the namespace for this node.
    public: BEXMLDLL_EXPORT Utf8CP GetNamespace();

    //! Adds and Sets the namespace for this node.
    public: BEXMLDLL_EXPORT void   SetNamespace (Utf8CP prefix, Utf8CP uri);

    //! Executes stricmp style comparison to the name.
    //! Note: The name is actually UTF8, but node names are customarily within the ASCII range.
    public: BEXMLDLL_EXPORT int NameStricmp(Utf8CP name);

    //! Ask if name matches.  (Case sensitive !!)
    //! Note: The name is actually UTF8, but node names are customarily within the ASCII range.
    public: BEXMLDLL_EXPORT bool IsName (Utf8CP name);

    //! Ask if name matches.  (Case Insensitive !!!)
    //! Note: The name is actually UTF8, but node names are customarily within the ASCII range.
    public: BEXMLDLL_EXPORT bool IsIName (Utf8CP name);

    //! Gets the DOM associated with this node.
    public: BEXMLDLL_EXPORT BeXmlDomP GetDom ();

    //! Gets the Utf16 WString associated with this node.
    public: BEXMLDLL_EXPORT BeXmlStatus GetXmlString (WString& xmlString);

    //! Gets the Utf16 Xml String associated with this node.
    public: BEXMLDLL_EXPORT BeXmlStatus GetXmlString (Utf16BufferR xmlString);
    
    //! Gets the Utf8 Xml String associated with this node.
    public: BEXMLDLL_EXPORT BeXmlStatus GetXmlString (Utf8StringR);

    //! Gets the n'th child element node of the provided parent. Other node types (such as text) are ignored in the iteration.
    //! @note This should act similarly and in concert with xmlChildElementCount, xmlFirstElementChild, and xmlLastElementChild.
    public: BEXMLDLL_EXPORT BeXmlNodeP GetChildElementAt (size_t index);

    //! Gets the parnet node of this node.
    public: BEXMLDLL_EXPORT BeXmlNodeP GetParentNode ();

    //! Gets the first child node of the desired type. If typeWanted is BEXMLNODE_Any, the first node regardless of type is returned.
    //! Returns NULL if there are no nodes matching typeWanted.
    public: BEXMLDLL_EXPORT BeXmlNodeP GetFirstChild (BeXmlNodeType typeWanted = BEXMLNODE_Element);

    //! Gets the next sibling node of the desired type. If typeWanted is BEXMLNODE_Any, the first node regardless of type is returned.
    //! Returns NULL if there are no sibling nodes matching typeWanted.
    public: BEXMLDLL_EXPORT BeXmlNodeP GetNextSibling (BeXmlNodeType typeWanted = BEXMLNODE_Element);

    //! Gets the content of a node (regardless of node type) as a WString. This will work regardless of whether the sole child is a text node, or if the string content has to get concatenated.
    //! @note The result is always cleared, even if an error occured.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContent (WStringR, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a Utf8String. This will work regardless of whether the sole child is a text node, or if the string content has to get concatenated.
    //! @note The result is always cleared, even if an error occured.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContent(Utf8StringR, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a 32-bit integer. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to an integer, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentInt32Value (int32_t&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a 32-bit unsigned integer. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to an integer, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentUInt32Value (uint32_t&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a 64-bit integer. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to an integer, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentInt64Value (int64_t&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a 64-bit unsigned integer. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to an integer, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentUInt64Value (uint64_t&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a Boolean. A path relative to thisNode can be specified. 
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentBooleanValue (bool&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a double. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to a double, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentDoubleValue (double&, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a DPoint2d. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to a double, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentDPoint2dValue (double& x, double& y, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) as a DPoint3d. A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to a double, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentDPoint3dValue (double& x, double& y, double& z, Utf8CP relativePath=NULL);

    //! Gets the content of a node (regardless of node type) and parses as comma-separated doubles . A path relative to thisNode can be specified.
    //! @note This attempts to detect trivial scenarios to avoid any extra string buffer allocations, but can ultimately fall back to xmlNodeGetContent.
    //! @param [out] values array of all successfully parsed doubles.
    //! @param [in] relativePath access modifier.
    //! @returns BEXML_NodeNotFound if relativePath is non-NULL and not found, BEXML_ContentWrongType if the content does not contain a string that parses to a double, or BEXML_Success.
    public: BEXMLDLL_EXPORT BeXmlStatus GetContentDoubleValues (bvector<double> &values, Utf8CP relativePath = NULL);

    //! Adds a CDATA section to this Element Node.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCDATASection (WStringR cDataContents);

    //! Sets the content of a node, clearing any existing content.
    public: BEXMLDLL_EXPORT void   SetContent (WCharCP);

    //! Sets the content of a node, clearing any existing content.
    //! Note: Use this method only if you are certain that there are no special characters such as &, &lt;, &gt;, = or \\r; and there are no entity references in content.
    public: BEXMLDLL_EXPORT void   SetContentFast (WCharCP content);

    //! Sets the content of a node, clearing any existing content.
    //! Note: Use this method only if you are certain that there are no special characters such as &, &lt;, &gt;, = or \\r; there are no entity references in content 
    //! and you are writing only asci characters.
    public: BEXMLDLL_EXPORT void   SetContentFast (Utf8CP content);

    //! Adds an empty Child Element Node with the node name specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP  AddEmptyElement (Utf8CP nodeName);

    //! Adds an empty Child Element Node with the node name specified. The overload that takes the nodeName as Utf8 is more efficient.
    public: BEXMLDLL_EXPORT BeXmlNodeP  AddEmptyElement (WCharCP nodeName);

    //! Adds a Child Element Node with the node name and string value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementStringValue(Utf8CP nodeName, Utf8CP value);

    //! Adds a Child Element Node with the node name and string value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementStringValue (Utf8CP nodeName, WCharCP value);

    //! Adds a Child Element Node with the node name and Uint32 value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementUInt32Value (Utf8CP nodeName, uint32_t value);

    //! Adds a Child Element Node with the node name and Int32 value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementInt32Value (Utf8CP nodeName, int32_t value);

    //! Adds a Child Element Node with the node name and boolean value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementBooleanValue (Utf8CP nodeName, bool value);

    //! Adds a Child Element Node with the node name and double value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementDoubleValue (Utf8CP nodeName, double value);

    //! Adds a Child Element Node with the node name and Uint64 value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementUInt64Value (Utf8CP nodeName, uint64_t value);

    //! Adds a Child Element Node with the node name and Int64 value specified.
    public: BEXMLDLL_EXPORT BeXmlNodeP AddElementInt64Value (Utf8CP nodeName, int64_t value);


    //! Adds a CDATA section to this Element Node.
    public: BEXMLDLL_EXPORT void   AddCDATASection (WCharCP cDataContents);

    //! Gets the string value of a node's attribute.
    //! @note The result is always cleared, even if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeStringValue (WStringR, Utf8CP attributeName);

    //! Gets the string value of a node's attribute.
    //! @note The result is always cleared, even if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeStringValue (Utf8StringR, Utf8CP attributeName);

    //! Gets the string value of a node's attribute.
    //! @note The result is always cleared, even if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeBooleanValue (bool&, Utf8CP attributeName);

    //! Gets the UInt32 value of a node's attribute.
    //! @note The result is set to 0 if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeUInt32Value (uint32_t&, Utf8CP attributeName);

    //! Gets the Int32 value of a node's attribute.
    //! @note The result is set to 0 if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeInt32Value (int32_t&, Utf8CP attributeName);

    //! Gets the UInt64 value of a node's attribute.
    //! @note The result is set to 0 if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeUInt64Value (uint64_t&, Utf8CP attributeName);

    //! Gets the double value of a node's attribute.
    //! @note The result is set to 0 if an error occurs.
    public: BEXMLDLL_EXPORT BeXmlStatus GetAttributeDoubleValue (double&, Utf8CP attributeName);

    //! Get the first child node with the specified name.
    public: BEXMLDLL_EXPORT BeXmlNodeP  SelectSingleNode (Utf8CP childPath);

    //! Get the first child node with the specified name.
    public: BEXMLDLL_EXPORT void SelectChildNodes (BeXmlDom::IterableNodeSet& childNodes, Utf8CP childPath);

    //! Adds a new attribute with the specified name and value. If an attribute with that name already exists, it is replaced. If name or value is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeStringValue (Utf8CP name, WCharCP value);

    //! Adds a new attribute with the specified name and value. If an attribute with that name already exists, it is replaced. If name or value is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeStringValue (Utf8CP name, Utf8CP value);

    //! Adds a new attribute with the specified name and value. If an attribute with that name already exists, it is replaced. If name or value is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeBooleanValue (Utf8CP name, bool value);

    //! Adds a new attribute with the specified name and UInt32 value. If an attribute with that name already exists, it is replaced. If name is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeUInt32Value (Utf8CP name, uint32_t value);

    //! Adds a new attribute with the specified name and Int32 value. If an attribute with that name already exists, it is replaced. If name is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeInt32Value (Utf8CP name, int32_t value);

    //! Adds a new attribute with the specified name and UInt64 value. If an attribute with that name already exists, it is replaced. If name is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeUInt64Value (Utf8CP name, uint64_t value);

    //! Adds a new attribute with the specified name and double value. If an attribute with that name already exists, it is replaced. If name is NULL or empty, no attempt is made to add an attribute.
    public: BEXMLDLL_EXPORT void AddAttributeDoubleValue (Utf8CP name, double value);

    //! Imports a node (recursively) from another document as a subNode of this node.
    public: BEXMLDLL_EXPORT BeXmlNodeP ImportNode (BeXmlNodeP nodeToImport);

    //! Removes a child node from this node.
    public: BEXMLDLL_EXPORT void RemoveChildNode (BeXmlNodeP nodeToRemove);
    };

//=======================================================================================
//! Interface of XmlReader.  This allows users to pass in different types of readers, like the new binary XML reader in ecobjects
//
// @bsiclass
//=======================================================================================
struct IBeXmlReader
    {
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    public: enum ReadResult ENUM_UNDERLYING_TYPE(int32_t)
        {
        READ_RESULT_Success = 1,
        READ_RESULT_Empty   = 0,
        READ_RESULT_Error   = -1
        }; // ReadResult
    
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    public: enum NodeType ENUM_UNDERLYING_TYPE(int32_t)
        {
        NODE_TYPE_None,
        NODE_TYPE_Element,
        NODE_TYPE_Attribute,
        NODE_TYPE_Text,
        NODE_TYPE_CDATA,
        NODE_TYPE_EntityReference,
        NODE_TYPE_Entity,
        NODE_TYPE_ProcessingInstruction,
        NODE_TYPE_Comment,
        NODE_TYPE_Document,
        NODE_TYPE_DocumentType,
        NODE_TYPE_DocumentFragment,
        NODE_TYPE_Notation,
        NODE_TYPE_Whitespace,
        NODE_TYPE_SignificantWhitespace,
        NODE_TYPE_EndElement,
        NODE_TYPE_EndEntity,
        NODE_TYPE_XmlDeclaration
        }; // NodeType

    protected:
        ReadResult virtual _ReadTo (NodeType) = 0;
        ReadResult virtual _ReadTo (NodeType nodeType, Utf8CP name, bool stayInCurrentElement, WStringP value) = 0;
        NodeType virtual _GetCurrentNodeType () = 0;
        BeXmlStatus virtual _GetCurrentNodeName (Utf8StringR) = 0;
        BeXmlStatus virtual _GetCurrentNodeValue(Utf8StringR) = 0;
        BeXmlStatus virtual _GetCurrentNodeValue(WStringR) = 0;
        ReadResult virtual _Read () = 0;
        BeXmlStatus virtual _ReadToNextAttribute (Utf8StringP name, Utf8StringP value) = 0;
        BeXmlStatus virtual _ReadToNextAttribute (Utf8StringP name, WStringP value) = 0;
        NodeType virtual _MoveToContent() = 0;
        ReadResult virtual _ReadToEndOfElement () = 0;
        BeXmlStatus virtual _ReadContentAsString(Utf8StringR str) = 0;
        bool virtual _IsEmptyElement() = 0;

    //! Advances (reads) to the next node of the provided type and optional name. Optionally gets the value of the node.
    //! @note This does a case-sensitive comparison on the name.
    public: BEXMLDLL_EXPORT ReadResult ReadTo (NodeType nodeType);

    //! Advances (reads) to the next node of the provided type and optional name. Optionally gets the value of the node.
    //! @note This does a case-sensitive comparison on the name.
    public: BEXMLDLL_EXPORT ReadResult ReadTo (NodeType nodeType, Utf8CP name, bool stayInCurrentElement, WStringP value);


    //! Gets the type of the current node.
    public: BEXMLDLL_EXPORT NodeType GetCurrentNodeType ();

    //! Gets the name of the current node.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeName (Utf8StringR nodeName);

    //! Gets the value of the current node.
    //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeValue(Utf8StringR nodeValue);

    //! Gets the value of the current node.
    //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeValue (WStringR nodeValue);

    //! Advances (reads) to the next node.
    public: BEXMLDLL_EXPORT ReadResult Read ();

    //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
    public: BEXMLDLL_EXPORT BeXmlStatus ReadToNextAttribute (Utf8StringP name, Utf8StringP value);

    //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
    public: BEXMLDLL_EXPORT BeXmlStatus ReadToNextAttribute (Utf8StringP name, WStringP value);

    public: BEXMLDLL_EXPORT NodeType MoveToContent();

    //! Advances (reads) to the start the end of this element node.
    public: BEXMLDLL_EXPORT ReadResult ReadToEndOfElement ();

    //! Reads the text content at the current position as a String object.
    public: BEXMLDLL_EXPORT BeXmlStatus ReadContentAsString(Utf8StringR str);

    //! If this is an empty element (&lt;Element /&gt; or &lt;Element&gt;&lt;/Element&gt;) returns true.
    public: BEXMLDLL_EXPORT bool IsEmptyElement();
    };

//=======================================================================================
//! A basic wrapper around xmlTextReader and some related operations to provide automatic memory management, as well as some other low-level utilities.
//! @note At this time, your library must manually call LIBXML_TEST_VERSION (from BeLibxml2) before using this wrapper (or any other BeLibxml2 functionality).
//
// ***This class is meant to be a relatively basic wrapper. While you may enhance it as required, it should NOT be a dumping ground for XML utilities.***
//
// @bsiclass
//=======================================================================================
struct BeXmlReader : public RefCountedBase, public NonCopyableClass, IBeXmlReader
    {

    //-----------------------------------------------------------------------------------------------------------------------------------------------
private:
    xmlTextReaderPtr    m_reader;
    xmlError            m_error;
    
    //! Gets a direct reference to this wrapper's reader.
public: 
    BEXMLDLL_EXPORT xmlTextReader& GetReader ();

    xmlError const& GetLastError() const {return m_error;}
            
    private:    BeXmlReader   ();
    protected:  ~BeXmlReader  ();

    protected:
        //! Advances (reads) to the next node.
        ReadResult _Read () override;

        //! Advances (reads) to the next node of the provided type and optional name. Optionally gets the value of the node.
        //! @note This does a case-sensitive comparison on the name.
        ReadResult _ReadTo (NodeType) override;
            
        //! Advances (reads) to the next node of the provided type and optional name. Optionally gets the value of the node.
        //! @note This does a case-sensitive comparison on the name.
        ReadResult _ReadTo (NodeType, Utf8CP name, bool stayInCurrentElement, WStringP value) override;


        //! Advances (reads) to the start the end of this element node.
        ReadResult _ReadToEndOfElement () override;
            
        //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
        BeXmlStatus _ReadToNextAttribute (Utf8StringP name, Utf8StringP value) override;

        //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
        BeXmlStatus _ReadToNextAttribute (Utf8StringP name, WStringP value) override;

        //! Gets the type of the current node.
        NodeType _GetCurrentNodeType () override;

        //! Gets the name of the current node.
        BeXmlStatus _GetCurrentNodeName (Utf8StringR) override;

        //! Gets the value of the current node.
        //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
        BeXmlStatus _GetCurrentNodeValue(Utf8StringR) override;

        //! Gets the value of the current node.
        //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
        BeXmlStatus _GetCurrentNodeValue(WStringR) override;

        //! Checks whether the current node is a content (non-white space text, CDATA, Element, EndElement, EntityReference, or EndEntity) node. 
        //! If the node is not a content node, the reader skips ahead to the next content node or end of file.
        NodeType _MoveToContent() override;

        //! Reads the text content at the current position as a String object.
        BeXmlStatus _ReadContentAsString(Utf8StringR str) override {return _GetCurrentNodeValue(str); }

        //! If this is an empty element (&lt;Element /&gt; or &lt;Element&gt;&lt;/Element&gt;) returns true.
        bool _IsEmptyElement() override;
    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Basic memory management

    //! Attempts to create a reader from the given file path.
    //! @return NULL if the file cannot be found or the first node cannot be processed; a valid reader wrapper otherwise.
    //! @note This will NOT automatically attempt to advance to the root node.
    public: BEXMLDLL_EXPORT static BeXmlReaderPtr CreateAndReadFromFile (BeXmlStatus& status, WCharCP fileName, WStringP errorMsg = NULL);

    //! Attempts to create a reader the XML fragment provided. Unless specified in the XML stream, the encoding is assumed to be UTF-16 on Windows, and UCS-4 on other platforms.
    //! @return NULL if the first node cannot be processed; a valid reader wrapper otherwise.
    //! @note This will NOT automatically attempt to advance to the root node.
    //! @note If you already know the number of characters in the input string, pass it in as characterCount to save the work of getting the string length again. Otherwise pass 0.
    public: BEXMLDLL_EXPORT static BeXmlReaderPtr CreateAndReadFromString (BeXmlStatus&, WCharCP, size_t characterCount=0, WStringP errorMsg = NULL);
    
    //! Attempts to create a reader the XML fragment provided.
    //! @return NULL if the first node cannot be processed; a valid reader wrapper otherwise.
    //! @note This will NOT automatically attempt to advance to the root node.
    //! @note If you already know the number of characters in the input string, pass it in as characterCount to save the work of getting the string length again. Otherwise pass 0.
    public: BEXMLDLL_EXPORT static BeXmlReaderPtr CreateAndReadFromString (BeXmlStatus&, Utf8CP, size_t characterCount=0, WStringP errorMsg = NULL);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    // Higher level reader utilities


    //! Advances (reads) to the end of the current element node.
    public: BEXMLDLL_EXPORT ReadResult ReadToEndOfCurrentElement ();

    //! Advances (reads) to the start the next sibling element node.
    public: BEXMLDLL_EXPORT ReadResult ReadToStartOfNextSiblingElement ();

    //! Skip to the node following the current one in document order while avoiding the subtree if any.
    public: BEXMLDLL_EXPORT ReadResult Skip();

    //! Determines if there are further nodes available.
    //! @return True if the reader is at the end, is closed, or is in an error state; False otherwise.
    public: BEXMLDLL_EXPORT bool IsAtEnd ();

    //! Determines if the current element is empty. If the reader is not on an element or attribute, this will return true.
    //! @warning If the reader is on an attribute, it will rewind to the start of the attribute's element.
    public: BEXMLDLL_EXPORT bool IsCurrentElementEmpty();

    //! Gets the value of the current node and its children. This is most useful, for example, if you're on an element node and want its text content without manually stepping to the text node.
    //! @note This forces all child nodes to be read, and it generates the concatenation of all of their values. If you want the current node's actual value, @see GetCurrentNodeValue.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeString(WStringR);

    //! Gets the outer XML of the current node.
    //! @note This returns un-escaped XML.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeOuterXml (WStringR);

    //! Gets the innt XML of the current node.
    //! @note This returns un-escaped XML.
    public: BEXMLDLL_EXPORT BeXmlStatus GetCurrentNodeInnerXml(WStringR);

    }; // BeXmlReader


//=======================================================================================
//! Interface of XmlWriter.  This allows users to pass in different types of writers, like the new binary XML writer in ecobjects
//
// @bsiclass
//=======================================================================================
struct IBeXmlWriter
{
protected:
    BeXmlStatus virtual _WriteElementStart(Utf8CP name) = 0;
    BeXmlStatus virtual _WriteElementStart(Utf8CP name, Utf8CP namespaceURI) = 0;
    BeXmlStatus virtual _WriteElementEnd() = 0;
    BeXmlStatus virtual _WriteText(WCharCP) = 0;
    BeXmlStatus virtual _WriteText(Utf8CP) = 0;
    BeXmlStatus virtual _WriteAttribute(Utf8CP name, WCharCP value) = 0;
    BeXmlStatus virtual _WriteAttribute(Utf8CP name, Utf8CP value) = 0;

public:
    // WriteElementStart/End + WriteText are classic undifferentiated XML style.
    //! Writes the start of an element node with the provided name.
    BEXMLDLL_EXPORT BeXmlStatus WriteElementStart(Utf8CP name);
    BEXMLDLL_EXPORT BeXmlStatus WriteElementStart(Utf8CP name, Utf8CP namespaceURI);
    BEXMLDLL_EXPORT BeXmlStatus WriteElementEnd();

    //! Writes a text node(plain string as content).
    BEXMLDLL_EXPORT BeXmlStatus WriteText(WCharCP text);

    //! Writes a text node(plain string as content). 
    BEXMLDLL_EXPORT BeXmlStatus WriteText(Utf8CP text);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, WCharCP value);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, Utf8CP value);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, bool value) {return _WriteAttribute(name,(value ? "true" : "false")); }

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, int32_t value);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, uint32_t value);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, uint64_t value);

    //! Writes an attribute node(name and value).
    BEXMLDLL_EXPORT BeXmlStatus WriteAttribute(Utf8CP name, double value);
};

//=======================================================================================
//! A basic wrapper around xmlTextWriter and some related operations to provide automatic memory management, as well as some other low-level utilities.
//! @note At this time, your library must manually call LIBXML_TEST_VERSION (from BeLibxml2) before using this wrapper (or any other BeLibxml2 functionality).
//
// ***This class is meant to be a relatively basic wrapper. While you may enhance it as required, it should NOT be a dumping ground for XML utilities.***
//
// @bsiclass
//=======================================================================================
struct BeXmlWriter : RefCountedBase, NonCopyableClass, IBeXmlWriter
{
private:
    xmlBufferPtr m_buffer;
    xmlTextWriterPtr m_writer;
    
    BeXmlWriter();

protected:
    ~BeXmlWriter();

    BeXmlStatus virtual _WriteText(WCharCP) override;
    BeXmlStatus virtual _WriteText(Utf8CP) override;
    BeXmlStatus virtual _WriteElementStart(Utf8CP name) override;
    BeXmlStatus virtual _WriteElementStart(Utf8CP name, Utf8CP namespaceURI) override;
    BeXmlStatus virtual _WriteElementEnd() override;
    BeXmlStatus virtual _WriteAttribute(Utf8CP name, WCharCP value) override;
    BeXmlStatus virtual _WriteAttribute(Utf8CP name, Utf8CP value) override;

public:
    //! Gets a direct reference to this wrapper's reader.
    BEXMLDLL_EXPORT xmlTextWriter& GetWriter();

    //! Attempts to create a writer
    //! @return a valid reader wrapper.
    BEXMLDLL_EXPORT static BeXmlWriterPtr Create();
    
    //! Attempts to create a writer to write to the file.
    //! @return NULL if the file cannot be be opened; a valid reader wrapper otherwise.
    //! @note ToString will return a blank string for this writer.
    BEXMLDLL_EXPORT static BeXmlWriterPtr CreateFileWriter(WCharCP fileName);
            
    //! Writes the current node of the provided reader.
    //! @note Not all node types are supported at this time.
    //! @remarks Similar to XmlLite's WriteNodeShallow.
    BEXMLDLL_EXPORT BeXmlStatus AddFromReader(xmlTextReader&);

    //! Writes the XML header(&lt;?xml ... ?&gt;) with the provided encoding. The encoding you provide here should match the ToString override you intend to use later(as well as how you intend to store the string).
    BEXMLDLL_EXPORT BeXmlStatus WriteDocumentStart(xmlCharEncoding);

    //! Writes the string verbatim, without encoding special characters.
    BEXMLDLL_EXPORT BeXmlStatus WriteRaw(WCharCP);

    //! Writes the string verbatim, without encoding special characters.
    BEXMLDLL_EXPORT BeXmlStatus WriteRaw(Utf8CP);

    //! Writes the string as CData, automatically replacing any instance of ']]&gt;' with ']]]]&gt;&lt;![CDATA[&gt;'.
    BEXMLDLL_EXPORT BeXmlStatus WriteCDataSafe(Utf8CP);

    //! Writes the string a comment, without encoding special characters.
    BEXMLDLL_EXPORT BeXmlStatus WriteComment(Utf8CP);

    //! Writes an empty attribute (e.g. name="").
    BEXMLDLL_EXPORT BeXmlStatus WriteEmptyAttribute(Utf8CP name);

    //! Sets the indentation of the XML. The default(0) is to not indent(or break lines). This only affects the writer from this point on, and will not posthumously indent what was already written.
    //! @note This toggles the writer to perform indentation, and sets the indent string to N spaces, based on indent value.
    BEXMLDLL_EXPORT BeXmlStatus SetIndentation(int indent);
    
    //! Generates a string representation of this DOM.
    //! @note If this is a writer to a file this will return a blank string.
    BEXMLDLL_EXPORT void ToString(WStringR);

    //! Generates a string representation of this DOM.
    //! @note If this is a writer to a file this will return a blank string.
    BEXMLDLL_EXPORT void ToString(Utf8StringR);
};

END_BENTLEY_NAMESPACE
