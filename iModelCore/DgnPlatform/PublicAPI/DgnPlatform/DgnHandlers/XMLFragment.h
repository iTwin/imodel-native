/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/XMLFragment.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnCore/Linkage.h>

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
struct XmlFragment;

typedef RefCountedPtr<XmlFragment>             XmlFragmentPtr;
#endif

/*=================================================================================**//**
* @addtogroup XMLFragments
* XML Fragments which may be stored in a design file.
*<P>
*    The <i>XmlFragment</i> class provides methods for storing XML-formatted data
*    into a design file. The data may be stored in an XML Fragment element, as a linkage
*    or XAttribute data.
*</P>
*<P>An XML Fragment consists of:</P>
*<UL>
*    <LI>AppID:&nbsp;A UShort identifying your application</LI>
*    <LI>AppType:&nbsp;A UShort used to further classify the use of the&nbsp;XML
*        Fragment&nbsp;by your application</LI>
*    <LI>SchemaURN (optional): If your fragment conforms to an XML Schema, you may
*        record the Schema's URN (Universal Resource Name) here.</LI>
*    <LI>XmlText: The actual XML fragment.</LI>
*</UL>
*<P><EM>For performance, the XML Fragment API does not validate your XML fragments
*        against an XML Schema or check them for well-formedness. You may use an XML
*        parser for validation. The SchemaURN is provided for your reference, but may be
*        utilized in future versions of MicroStation. Also for performance, the XML
*        fragments are not compressed by default. If a large XML Text string must be attached to
*        an element, use XmlFragment::SetIsCompressed to specify that it be compressed.
*</EM></P>
*
* Typical method of attaching XML Fragment Linkage.
* Example:
* @code
UShort const APPID_GasCompany       = 2000;
UShort const APPTYPE_FeatureData    = 10;
UShort const  APPTYPE_StateData     = 11;

WCharCP featureDataSchema = L"www.bentley.com/piping";
WCharCP featureData = L"<FeatureData Name=\"Pipe\"></FeatureData>";

int test_fragmentLinkage (DgnModel& dgnCache)
    {
    EditElementHandle eeh;

    // Create a line
    DPoint3d        linePts[2];
    memset (linePts, 0, sizeof(linePts));
    linePts[1].y = 100;
    if (SUCCESS, LineHandler::CreateLineElement (eeh, NULL, *linePts, false, dgnCache))
        return ERROR;

    // create the XML fragment
    XmlFragmentPtr pXMLFragment = XmlFragment::Construct (APPID_GasCompany, APPTYPE_FeatureData, false, featureData, featureDataSchema);
    if (!pXMLFragment.IsValid ())
        return ERROR;

    // attach the fragment
    if (SUCCESS != pXMLFragment->AttachToElement (eeh))
        return ERROR;

    // write the element
    return AddElementToModel (dgnCache, eeh);
    }

* @endcode
* @beginGroup
*/
//=======================================================================================
//! Class used to define XML Fragments that can be stored in the design file.
//  @bsiclass                                                    Bill.Steinbock  08/09
//=======================================================================================
struct XmlFragment : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    //persistent members
    UShort          m_appID;          // Application user id number...
    UShort          m_appType;        // app type of data stored
    WString         m_SchemaURN;              // optional name
    WString         m_Text;                   // Xml string

    // transient members
    bool            m_isCompressed;           // Stream compressed or not (stored as a TFStream_encoding value)
    byte*           m_pStreamBuffer;          //transient storage for stream buffer
    UInt32          m_ulStreamBufferSize;     //transient storage for size of data in pStreamBuffer

private:
    static XmlFragmentPtr  ConstructFromData (DataInternalizer& source, bool isCompressed, UInt16 appId, UInt16 appType);

public:
    DGNPLATFORM_EXPORT ~XmlFragment  ();
    DGNPLATFORM_EXPORT XmlFragment (UShort appID, UShort appType, bool isCompressed, WCharCP pSchemaURN, WCharCP pText);
    DGNPLATFORM_EXPORT StatusInt StripFromElement (DgnElementP  pElm, UShort* pAppID, UShort* pAppType, bool (*linkFunc)(LinkageHeader *, XmlFragmentPtr, void *), void* pUserParams);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Create a new empty XML Fragment and returns a reference counted pointer to it.
    //! @remarks   The app identifier should be a Bentley assigned application identifier (typically an application's LinkageID).
    //! @param[in] appID        The ID of the "app" to which this XmlFragment belongs
    //! @param[in] appType      The type of this XmlFragment within the "app" to which it belongs
    DGNPLATFORM_EXPORT static  XmlFragmentPtr Construct (UShort appID, UShort appType);

    //! Create a new XML Fragment and returns a reference counted pointer to it.
    //! @remarks   The app identifier should be a Bentley assigned application identifier (typically an application's LinkageID).
    //! @param[in] appID        The ID of the "app" to which this XmlFragment belongs
    //! @param[in] appType      The type of this XmlFragment within the "app" to which it belongs
    //! @param[in] isCompressed Set to true to compressed the XML fragment data
    //! @param[in] pText        XML text to set (or NULL to delete XML text).
    //! @param[in] pSchemaURN   Schema designation.
    DGNPLATFORM_EXPORT static  XmlFragmentPtr Construct (UShort appID, UShort appType, bool isCompressed, WCharCP pSchemaURN, WCharCP pText);

    //! Create a new XML Fragment and returns a reference counted pointer to it. Using this constructor method, the Schema designation
    //! is left blank and the fragment is not to be compressed.
    //! @remarks   The app identifier should be a Bentley assigned application identifier (typically an application's LinkageID).
    //! @param[in] appID        The ID of the "app" to which this XmlFragment belongs
    //! @param[in] appType      The type of this XmlFragment within the "app" to which it belongs
    //! @param[in] pText        XML text to set (or NULL to delete XML text).
    DGNPLATFORM_EXPORT static  XmlFragmentPtr Construct (UShort appID, UShort appType, WCharCP pText);

    //! Test an element to see if it contains an XML Fragment linkage.
    //! @remarks   If the parameters pAppID and pAppType are both NULL then true is returned if any XML Fragment linkages are present.
    //! @param[in] eh        The element to test.
    //! @param[in] pAppID    The ID of the "app" to which this XmlFragment belongs
    //! @param[in] pAppType  The type of this XmlFragment within the "app" to which it belongs, If this is non-NULL then appID must also be specified.
    DGNPLATFORM_EXPORT static bool IsFragmentElement (ElementHandleCR eh, UInt16* pAppID=NULL, UInt16* pAppType=NULL);

    //! Create a new XML Fragment from a bytes buffer and returns a reference counted pointer to it.
    //! @remarks   The app identifier should be a Bentley assigned application identifier (typically an application's LinkageID).
    //! <P>The format of the data buffer is shown below.</P>
    //! \code
    // UInt32     compression flag, 1=uncompressed, 2=compressed
    // UInt32     uncompressed XML Fragment data size + the size in bytes of these first two values (8 bytes)
    // --- start of compressable data ----------------------------------
    // UInt32     total size, in bytes, of uncompressed XML Fragment data
    // UInt32     # of bytes to hold schema including trailing 0
    // byte*      Schema URN text
    // UInt32     # of bytes to hold XML fragment text including trailing 0
    // byte*      XML fragment text
    // --- end of compressable data -------------------------------------
    //! \endcode
    //! @param[in] pBuffer      A byte buffer containing the XML fragment data
    //! @param[in] ulBufferSize The size of data in buffer
    //! @param[in] appId        The ID of the "app" to which this XmlFragment belongs
    //! @param[in] appType      The type of this XmlFragment within the "app" to which it belongs
    DGNPLATFORM_EXPORT static XmlFragmentPtr ConstructFromBuffer (byte* pBuffer, UInt32 ulBufferSize, UInt16 appId, UInt16 appType);

    //! Create a new XML Fragment from an XML Fragment element.
    //! @param[in] eh       The XML Fragment element.
    DGNPLATFORM_EXPORT static XmlFragmentPtr ConstructFromXmlFragmentElement (ElementHandleCR eh);

    //! Remove specific XML Fragment linkages from an element.
    //! @param[in] eeh          The element to process.
    //! @param[in] pAppID       The AppID of the linkage to remove. May be NULL if pAppType is also NULL, which is equivalent to the StripAllFromElement method.
    //! @param[in] pAppType     The AppType of the linkage to remove. If NULL only the pAppID test must be satisfied for the linkage(s) to be removed.
    DGNPLATFORM_EXPORT static StatusInt StripFromElementByAppIDAndType (EditElementHandleR eeh, UShort* pAppID, UShort* pAppType);

    //! Removes all XML Fragment linkages from an element.
    //! @param[in] eeh           The element to process.
    DGNPLATFORM_EXPORT static StatusInt StripAllFromElement (EditElementHandleR eeh);

    //! Test to see if element contains any XML Fragment linkages.
    //! @param[in] eh           The element to process.
    //! @param[in] pAppID       The AppID of the linkage to find. May be NULL if pAppType is also NULL, which means find any XML Fragment linkage.
    //! @param[in] pAppType     The AppType of the linkage to find. If NULL only the pAppID test must be satisfied for a linkage to be found.
    DGNPLATFORM_EXPORT static bool ElementHasXmlFragmentLinkage (ElementHandleCR  eh, const UShort* pAppID, const UShort* pAppType);

    //! Retrieves the app identifier from the given XML Fragment.
    //! @see     SetAppIDC
    DGNPLATFORM_EXPORT UShort GetAppID ();

    //! Sets the app identifier for the given XML Fragment.
    //! @remarks   The app identifier is a Bentley assigned application identifier (typically an application's LinkageID).
    //! @param[in] appID      The ID of the "app" to which this XmlFragment belongs
    //! @see     GetAppID
    DGNPLATFORM_EXPORT void SetAppID  (UShort appID);

    //! Retrieves the app type from the given XML Fragment.
    //! @remarks The app type is an application assigned value which uniquely identifies the
    //! XML Fragment within the application's app.
    //! @see     SetAppType
    DGNPLATFORM_EXPORT UShort GetAppType ();

    //! Sets the app type for the given XML Fragment.
    //! @remarks The app type is an application
    //! assigned value which uniquely identifies the XML Fragment Type within the application's app.
    //! @param[in] appType The type of this XmlFragment within the "app" to which it belongs
    //! @see     GetAppType
    DGNPLATFORM_EXPORT void SetAppType (UShort appType);

    //! Get the Schema URN from an XML Fragment.
    //! @return normally SUCCESS; MDLERR_BADARG if either of Schema URN or XmlFragment pointers is invalid.
    //! @see     SetSchemaURN
    DGNPLATFORM_EXPORT WCharCP GetSchemaURN ();

    //! Sets the Schema URN for an XML Fragment.
    //! @param[in] pSchemaURN      Schema designation.
    //! @see     GetSchemaURN
    DGNPLATFORM_EXPORT void SetSchemaURN (WCharCP pSchemaURN);

    //! Get the XML Text from an XML Fragment.
    //! @return Pointer to XML Text string from XmlFragment. (Do not attempt to free)
    //! @see     SetText
    DGNPLATFORM_EXPORT WCharCP GetText ();

    //! Sets the XML Text for an XML Fragment.
    //! @remarks The text is assumed to be well formed XML.  This function does not validate the XML text.
    //! @param[in] pText       XML text to set (or NULL to delete XML text).
    //! @see     GetText
    DGNPLATFORM_EXPORT void SetText (WCharCP pText);

    //! Get the "is compressed" flag value from an XML Fragment.
    //! @remarks By default, the XML Fragment is NOT compressed before attachment to an element or XmlFragment element creation.
    //! Using compression will not affect file size but will affect memory usage when a file is open.
    //! @return true if the buffer is to be compressed.
    //! @see     SetIsCompressed
    DGNPLATFORM_EXPORT bool IsCompressed ();

    //! Sets the "is compressed" flag value for an XML Fragment.
    //! @remarks By default, the XML Fragment
    //! is NOT compressed before attachment to an element or XmlFragment element creation.
    //! Using compression will not affect file size but will affect memory usage when a file is open.
    //! @param[in] isCompressed   Is compressed flag value to set
    //! @see     IsCompressed
    DGNPLATFORM_EXPORT void SetIsCompressed (bool isCompressed);

    //! Get the stream data byte buffer for the XML Fragment. This data is cached in the XML Fragment to be used for subsequent linkage attachement or XML Fragment elements.
    //! Use FreeStreamData () to free this buffer. The buffer is automatically freed if the XML Fragment is updated or when the XmlFragment object is deleted.
    //! @param[out] ppBuffer        cached stream buffer (FreeStreamData () to free this buffer)
    //! @param[out] pulBufferSize   size of data in buffer
    //! @see     FreeStreamData
    DGNPLATFORM_EXPORT StatusInt GetStreamData (byte** ppBuffer, UInt32* pulBufferSize);

    //! Frees the cached byte buffer data.
    //! @see     GetStreamData
    DGNPLATFORM_EXPORT void FreeStreamData ();

    //! Attach XML Fragment linkage to specified element.
    //! @param[in] eeh        Element to receive linkages
    //! @see     XmlFragmentList::ExtractFromElement
    DGNPLATFORM_EXPORT StatusInt AttachToElement (EditElementHandleR eeh);

    //! Create an XML Fragment element element.
    //! @param[out] eeh             The new XML fragment element
    //! @param[in] model            The model in which the caller intends to store the TagSet
    //! @param[in] setNonModelFlag  Set the non-model flag on the created element. This should be set to true if the element is to be added to the dictionary model.
    //! @see     XmlFragmentList::ExtractFromElement
    DGNPLATFORM_EXPORT StatusInt CreateXmlElement (EditElementHandleR eeh, DgnModelP model, bool setNonModelFlag);
};

/** @endGroup */
#if !defined (DOCUMENTATION_GENERATOR)
struct XmlFragmentList;

typedef RefCountedPtr<XmlFragmentList>         XmlFragmentListPtr;
#endif

/*=================================================================================**//**
* @addtogroup XMLFragments
* List of XML Fragments
*<P>
*    The <i>XmlFragmentList</i> class provides methods for creating, maintaining and
*    processing a list of XML Fragments. This is useful when dealing with multiple
*    XML Fragment linkages on a single element. They are not as useful when dealing
*    with XML Fragment elements which can only store the data for a single XmlFragment object.
*</P>
*
* Typical method of extrach XML Fragment Linkage(s) from an element.
* Example:
* @code
    XmlFragmentListPtr linkageFragments = XmlFragmentList::ExtractFromElement (eeh);
    if (linkageFragments.IsValid ())
        {
        for each (XmlFragmentPtr fragment in (*linkageFragments))
            {
            printf ("AppID=%d, AppType=%d, SchemaURN=%S\n", fragment->GetAppID (), fragment->GetAppType (), fragment->GetSchemaURN ());
            }
        }

* @endcode
* @beginGroup
*/
//=======================================================================================
//! Class used to process list of XML Fragments.
//! @see     XmlFragment
//  @bsiclass                                                    Bill.Steinbock  08/09
//=======================================================================================
struct XmlFragmentList : public RefCountedBase
{
//__PUBLISH_SECTION_END__
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    bvector<XmlFragmentPtr> m_fragments;

public:
    DGNPLATFORM_EXPORT XmlFragmentList (XmlFragmentPtr& xmlFragment);
    DGNPLATFORM_EXPORT XmlFragmentList ();
    DGNPLATFORM_EXPORT ~XmlFragmentList ();

    DGNPLATFORM_EXPORT XmlFragmentList (UShort appID, UShort appType, bool isCompressed=false, WCharCP pSchemaURN=NULL, WCharCP pText=NULL);

    //! Extract an bvector of fragments contained in the XML Fragment list.
    DGNPLATFORM_EXPORT bvector<XmlFragmentPtr>& GetVectorOfXmlFragments();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    typedef bvector<XmlFragmentPtr>::const_iterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    //! Returns the beginning of an iteration over the XMLFragments within this XmlFragmentList.
    DGNPLATFORM_EXPORT bvector<XmlFragmentPtr>::iterator begin ();

    //! Returns the end of the  an iteration over the XMLFragments within this XmlFragmentList.
    DGNPLATFORM_EXPORT bvector<XmlFragmentPtr>::iterator end ();

    //! Constructs an XML Fragment List containing a single XML Fragment.
    //! @param[in] xmlFragment       XML fragment to be used as the fist entry in the list.
    //! @return XmlFragmentListPtr, use IsValid() test to see if the method was successful.
    DGNPLATFORM_EXPORT static XmlFragmentListPtr ConstructFromFragment (XmlFragmentPtr& xmlFragment);

                       static XmlFragmentListPtr ConstructFromFragment (XmlFragmentPtr const& xmlFragment) {return ConstructFromFragment(const_cast<XmlFragmentPtr&>(xmlFragment));} // WIP_NONPORT - don't pass non-const reference to temporary object

    //! Constructs an XML Fragment List containing a single XML Fragment.
    //! @param[in] pText       XML text to set (or NULL).
    //! @param[in] pSchemaURN  Schema URN to set (or NULL).
    //! @param[in] appID       The ID of the "app" to which this XmlFragment belongs.
    //!                        The app identifier is a Bentley assigned application identifier.
    //!                        (typically an application's LinkageID).
    //! @param[in] appType     The type of this XmlFragment within the "app" to which it belongs.
    //!                        The app type is an application assigned value which uniquely identifies the
    //!                        XML Fragment within the application's app.
    //! @return XmlFragmentListPtr, use IsValid() test to see if the method was successful.
    DGNPLATFORM_EXPORT static XmlFragmentListPtr  Construct (WChar* pText, WChar* pSchemaURN, UShort appID, UShort appType);

    //! Constructs an XML Fragment List and populate it with an XML Fragment created from the stream buffer data.
    //! @param[in] pBuffer      A byte buffer containing the XML fragment data
    //! @param[in] ulBufferSize The size of data in buffer
    //! @param[in] appId        The ID of the "app" to which this XmlFragment belongs
    //! @param[in] appType      The type of this XmlFragment within the "app" to which it belongs
    DGNPLATFORM_EXPORT static XmlFragmentListPtr ConstructFromBuffer (byte* pBuffer, UInt32 ulBufferSize, UShort appId=0, UShort appType=0);

    //! Appends an XML Fragment List to an XML Fragment List.
    //! @param [in] xmlFragment  XML Fragment to append.
    //! @return normally SUCCESS; MDLERR_BADARG if xmlFragment is invalid
    DGNPLATFORM_EXPORT StatusInt Append  (XmlFragmentPtr& xmlFragment);

    //! Prepend an XML Fragment List to the head of an XML Fragment List.
    //! @param [in] xmlFragment  XML Fragment to prepend.
    //! @return normally SUCCESS; MDLERR_BADARG if xmlFragment is invalid
    DGNPLATFORM_EXPORT StatusInt Prepend (XmlFragmentPtr& xmlFragment);

    //! Remove an XML Fragment from an XML Fragment List.
    //! @param [in] index  Index of fragment in list.
    //! @return normally SUCCESS; MDLERR_BADARG if indexOfFragment is invalid
    DGNPLATFORM_EXPORT StatusInt Remove (size_t index);

    //! Get the number of XML Fragments in a XML Fragment List.
    DGNPLATFORM_EXPORT size_t GetCount();

    //! Get specified XML Fragment from an XML Fragment List.
    //! @param [in] index  Index of fragment in list.
    //! @return a valid XmlFragmentPtr if index is valid. If not XmlFragment.IsValid() will return false.
    DGNPLATFORM_EXPORT XmlFragmentPtr GetFragmentAtIndex (size_t index);

    //! Constructs an XML Fragment List by containing all XML Fragments from all linkages matching the specifiec AppId and AppType.
    //! @param[in] eh           Element that contains XML Fragment linkages
    //! @param[in] pAppID       The AppID of the linkage to find. May be NULL if pAppType is also NULL, which means find any XML Fragment linkage.
    //! @param[in] pAppType     The AppType of the linkage to find. If NULL only the pAppID test must be satisfied for a linkage to be found.
    DGNPLATFORM_EXPORT static XmlFragmentListPtr ExtractFromElement (ElementHandleCR eh, const UShort* pAppID=NULL, const UShort* pAppType=NULL);

    //! Attach all XML Fragments in the list as linkages to specified element.
    //! @param[in] eeh        Element to receive linkages
    //! @see     ExtractFromElement
    DGNPLATFORM_EXPORT StatusInt AttachToElement (EditElementHandleR eeh);
};

/** @endGroup */
END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
