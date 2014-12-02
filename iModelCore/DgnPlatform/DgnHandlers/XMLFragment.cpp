/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/XMLFragment.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

 /*----------------------------------------------------------------------+
|                                                                       |
|   XML Linkage Types                                                   |
|                                                                       |                                                                       |
+----------------------------------------------------------------------*/
#define LINKAGETYPE_XML             (1<<0)   /* XmlFragment stored on element */

#define TFStream_encoding_BinaryCrossPlatform  1    /* not zipped */
#define TFStream_encoding_gZip                 2    /* zipped     */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Local Definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
typedef StatusInt     (*PFXMLFragmentExtractAttachmentCallback)
(
XmlFragmentList     * pXMLFragmentList,
void                * callbackArg
);

typedef StatusInt     (*PFXMLFragmentDeleteAttachmentCallback)
(
XmlFragmentList     * pXMLFragmentList,
void                * callbackArg
);


/*  Structure used in calls to extractXMLFragmentFromElementByAppIDAndType
    from within mdlXMLFragmentList_extractFromElement AND
                mdlXMLFragmentList_extractFromElementByAppIDAndType */
typedef struct xmlextractattachmentcallbackparams   //needs work: move to xmlclass.c
    {
    XmlFragmentList*                          pXMLFragmentList;
    const UInt16                            * pAppID;
    const UInt16                            * pAppType;
    bool                                      justCheckForPresence;
    bool                                      fragmentsPresent;
    bool                                      (*linkFunc)(LinkageHeader *, XmlFragmentPtr, void *);
    void *                                    pUserParams;
    } XMLExtractAttachmentCallBackParams;


static UInt8 s_XMLLinkageConvRulesCompiled[] = 
{
0x70,0x6F,0x76,0x63,0x84,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* DataDef Conversion Rules */
static void *s_pXMLLinkageConvRules = s_XMLLinkageConvRulesCompiled;

/*----------------------------------------------------------------------+
|
| name          doesNotMatchCriteria
|
| author        CaseyMullen                             08/01
|
+----------------------------------------------------------------------*/
Public bool    doesNotMatchCriteria
(
MSXMLLinkage                       * pXMLLinkage,
XMLExtractAttachmentCallBackParams * pParams
)
    {
    //make sure this is an XML linkage
    if ( !(LINKAGETYPE_XML & pXMLLinkage->data.linkageType) )
        {
        return true;
        }

    //ensure the appID matches, if requested
    if (pParams->pAppID && *pParams->pAppID != pXMLLinkage->data.appID)
        {
        return true;
        }

    //ensure the appType matches, if requested
    if (pParams->pAppType && *pParams->pAppType != pXMLLinkage->data.appType)
        {
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|
| name          deleteXMLFragmentFromElementCallback
|
| author        CaseyMullen                             08/01
|
+----------------------------------------------------------------------*/
static bool deleteXMLFragmentFromElementCallback
(
LinkageHeader*          pLinkage,
void*                   pfuncParams
)
    {
    MSXMLLinkage* pXMLLinkage = (MSXMLLinkage*)pLinkage;
    XMLExtractAttachmentCallBackParams* pParams = (XMLExtractAttachmentCallBackParams*)pfuncParams;

    if (!pXMLLinkage)
        return false; // do not delete this one

    if (doesNotMatchCriteria (pXMLLinkage, pParams))
        return false;

    bool doDelete = true;  // do the delete

    if (pParams->linkFunc != NULL)
        {
        XmlFragmentPtr pXMLFragment = XmlFragment::ConstructFromBuffer (pXMLLinkage->data.byteBuffer, pXMLLinkage->data.numBytes, pXMLLinkage->data.appID, pXMLLinkage->data.appType);
        if (pXMLFragment.IsValid())
            {
            doDelete = (*pParams->linkFunc)(pLinkage, pXMLFragment, pParams->pUserParams);
            }
        }

    return doDelete;
    }

/*----------------------------------------------------------------------+
|
| name          deleteXMLFragmentFromElementByAppIDAndType
|
| author        CaseyMullen                             08/01
|
+----------------------------------------------------------------------*/
static StatusInt deleteXMLFragmentFromElementByAppIDAndType
(
DgnElement*      pElm,
const UInt16*   pAppID,
const UInt16*   pAppType,
bool          (*linkFunc)(LinkageHeader *, XmlFragmentPtr, void *),
void*           pUserParms
)
    {
    XMLExtractAttachmentCallBackParams params;

    memset (&params, 0, sizeof(params));

    if (NULL == pElm)
        return DGNPLATFORM_STATUS_BadArg;

    params.pAppID               = pAppID;
    params.pAppType             = pAppType;
    params.linkFunc             = linkFunc;
    params.pUserParams          = pUserParms;

    /*  Delete XML Linkages from element where XmlFragment attached directly to element.  */
    linkage_deleteFromElement (pElm, LINKAGEID_XML, s_pXMLLinkageConvRules, deleteXMLFragmentFromElementCallback, &params);
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|
| name          extractXMLFragmentFromElementCallback
|
| author        CaseyMullen                             08/01
|
+----------------------------------------------------------------------*/
static bool     extractXMLFragmentFromElementCallback
(
LinkageHeader*          pLinkage,
void*                   pfuncParams
)
    {
    MSXMLLinkage* pXMLLinkage = (MSXMLLinkage*)pLinkage;
    XMLExtractAttachmentCallBackParams* pParams = (XMLExtractAttachmentCallBackParams*)pfuncParams;

    if (!pXMLLinkage)
        return false;

    if (doesNotMatchCriteria (pXMLLinkage, pParams))
        return false;

    pParams->fragmentsPresent = true;

    if (pParams->justCheckForPresence)
        return false;

    //actually construct the XmlFragment from the buffer
    XmlFragmentPtr pXMLFragment = XmlFragment::ConstructFromBuffer (pXMLLinkage->data.byteBuffer, pXMLLinkage->data.numBytes, pXMLLinkage->data.appID, pXMLLinkage->data.appType);
    if (pXMLFragment.IsValid() && (NULL != pParams->pXMLFragmentList))
        pParams->pXMLFragmentList->Append (pXMLFragment);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    CaseyMullen     08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt extractXMLFragmentFromElementByAppIDAndType
(
XmlFragmentList*          pXMLFragmentList,
DgnElementCP               pElm,
const UInt16            * pAppID,
const UInt16            * pAppType,
bool                      justCheckForPresence,
bool                    * pFragmentsPresent
)
    {
    XMLExtractAttachmentCallBackParams params;

    memset (&params, 0, sizeof(params));

    if (NULL == pXMLFragmentList && !justCheckForPresence)
        {
        return DGNPLATFORM_STATUS_BadArg;
        }

    if (!pElm)
        {
        return DGNPLATFORM_STATUS_BadArg;
        }

    if (justCheckForPresence && !pFragmentsPresent)
        {
        return DGNPLATFORM_STATUS_BadArg;
        }

    params.pXMLFragmentList     = pXMLFragmentList;
    params.pAppID               = pAppID;
    params.pAppType             = pAppType;
    params.justCheckForPresence = justCheckForPresence;
    params.fragmentsPresent     = false;

    linkage_extractFromElement (NULL, pElm, LINKAGEID_XML, s_pXMLLinkageConvRules, extractXMLFragmentFromElementCallback, &params);

    if (pFragmentsPresent)
        {
        *pFragmentsPresent = params.fragmentsPresent;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    CaseyMullen     08/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void XMLClass_extractIDandTypeFromDGNStoreApplicationID
(
UInt16 * pId,   /* <= */
UInt16 * pType,  /* <= */
UInt32   dgnStoreApplicationID /* => */
)
    {
    UInt32   tmp = dgnStoreApplicationID;

    tmp = tmp << 16;
    tmp = tmp >> 16;

    if (pType)
        *pType = (UInt16) tmp;

    if (pId)
        *pId = (UInt16)(dgnStoreApplicationID >> 16);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          XMLClass_sizeofXMLLinkage                               |
|                                                                       |
| author        ChrisDalesandro                         04/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static UInt32  XMLClass_sizeofXMLLinkage /* <= linkage size in words */
(
UInt32          ulBufferSize            /* => size of XML data to store */
)
    {
    UInt32              ulLinkageSize;
    UInt32              ulLinkageWords;
    LinkageHeader       xmlLinkageHdr;

    /* If buffer size at or above max element size in bytes then
        return the number of words.  Don't need to build the linkage,
        since we already know this amount of data will not fit in a linkage. */
    if (ulBufferSize >= MAX_V8_ELEMENT_SIZE)
        {
        return (ulBufferSize/2);
        }

    memset (&xmlLinkageHdr, 0, sizeof (xmlLinkageHdr));

    ulLinkageSize       = ulBufferSize + sizeof (MSXMLLinkage);
    ulLinkageSize       = (ulLinkageSize + 7) & ~7;   // pad to 8-byte multiple
    if (ulLinkageSize >= MAX_V8_ELEMENT_SIZE)
        {
        return (ulBufferSize/2);
        }

    ulLinkageWords              = ulLinkageSize/2;

    if (LinkageUtil::SetWords (&xmlLinkageHdr, ulLinkageWords) != SUCCESS)
        {
        // Assume too many words, return linkageWords
        return ulLinkageWords;
        }
    else
        {
        ulLinkageWords = LinkageUtil::GetWords (&xmlLinkageHdr);
        }

    return ulLinkageWords;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          XMLClass_isSpaceForAttachments                          |
|                                                                       |
| author        ChrisDalesandro                         04/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool     XMLClass_isSpaceForAttachments
(
DgnElementCP pElm,                           /* => element to store info to */
UInt32      ulBufferSize                    /* => size of XML data to store */
)
    {
    UInt32              ulXmlWordLength;

    if (!pElm)
        {
        return false;
        }

    /*  Check size of XML linkage */
    ulXmlWordLength = XMLClass_sizeofXMLLinkage (ulBufferSize);

    /* If XML data not too large for attachment then return true */
    return (pElm->GetSizeWords() + ulXmlWordLength < MAX_V8_ELEMENT_SIZE);
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragment::XmlFragment (UShort appID, UShort appType, bool isCompressed, WCharCP pSchemaURN, WCharCP pText)
    {
    m_appID        = appID;
    m_appType      = appType;
    m_isCompressed = isCompressed;

    if (pSchemaURN)
        m_SchemaURN = pSchemaURN;
     else
        m_SchemaURN = L"";

    if (pText)
        m_Text = pText;
    else
        m_Text = L"";

    m_pStreamBuffer = NULL;
    m_ulStreamBufferSize = 0L; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
 XmlFragmentPtr XmlFragment::Construct (UShort appID, UShort appType, bool isCompressed, WCharCP pSchemaURN, WCharCP pText)
     {
     return new XmlFragment (appID, appType, isCompressed, pSchemaURN, pText);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr XmlFragment::Construct (UShort appID, UShort appType, WCharCP pText)
     {
     return new XmlFragment (appID, appType, false, L"", pText);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr XmlFragment::Construct (UShort appID, UShort appType)
     {
     return new XmlFragment (appID, appType, false, NULL, NULL);
     }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragment::~XmlFragment ()
    {
    MEMUTIL_FREE_AND_CLEAR (m_pStreamBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
 UShort XmlFragment::GetAppID ()
     {
     return m_appID;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlFragment::SetAppID (UShort appID)
     {
     m_appID = appID; // not contained in stream data
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UShort XmlFragment::GetAppType ()
    {
    return m_appType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlFragment::SetAppType (UShort appType)
    {
    m_appType = appType;  // not contained in stream data
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP XmlFragment::GetSchemaURN ()
    {
    if (m_SchemaURN.empty())
        return NULL;

    return m_SchemaURN.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlFragment::SetSchemaURN (WCharCP pSchemaURN)
    {
    FreeStreamData ();

    if (pSchemaURN)
        m_SchemaURN = pSchemaURN;
    else
        WString().swap (m_SchemaURN);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP XmlFragment::GetText ()
    {
    if (m_Text.empty())
        return NULL;
        
    return m_Text.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlFragment::SetText (WCharCP pText)
    {
    FreeStreamData ();

    if (pText)
        m_Text = pText;
    else
        WString().swap (m_Text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlFragment::IsCompressed ()
    {
    return m_isCompressed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlFragment::SetIsCompressed (bool isCompressed)
    {
    FreeStreamData ();
    m_isCompressed = isCompressed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void     XmlFragment::FreeStreamData ()
    {
    MEMUTIL_FREE_AND_CLEAR (m_pStreamBuffer);
    m_ulStreamBufferSize = 0L;
    }

/*---------------------------------------------------------------------------------**//**
*    Format of stream buffer
*    Int32               TFStream_encoding_BinaryCrossPlatform=1 or TFStream_encoding_gZip=2
*    Int32               streamSize;   Total Size of uncompressed buffer
*    ---------------------------------------------------------------
*    UInt32              total size of XmlFragment data
*    Int32               # of bytes to hold schema including trailing 0
*    byte*               schema urn text
*    Int32               # of bytes to hold XML fragment text including trailing 0
*    byte*               XML fragment text
*
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     XmlFragment::GetStreamData
(
byte**      ppBuffer,        /* <= stream buffer (DO NOT FREE) use XmlFragment::FreeStreamData () to clear from fragment */
UInt32*     pulBufferSize    /* <= size of data in buffer to write */
)
    {
    // if the stream has not already been populate then populate it now
    if (NULL == m_pStreamBuffer)
        {
        // need to generate the stream data
        DataExternalizer       compressableSink;
        DataExternalizer       linkageSink;
        int encoding = TFStream_encoding_BinaryCrossPlatform;

#if defined (WIP_NONPORT_STORED_STRINGS) // *** Stored length is probably wrong
    Why are we writing out the value of m_SchemaURN.GetMaxLocaleCharBytes and m_Text.GetMaxLocaleCharBytes? We are not storing m_SchemaURN or m_Text as multi-byte strings. We 
    are storing them as UTF-16 strings. A UTF-16 string contains 2 (or possibly 3) bytes for each Unicode character. To get the actual number of bytes that will be stored,
    you must convert these strings to UTF-16 first and then get the size of the resulting utf16 buffers.
#endif

        UInt32 totalSizeOfXMLDataBuffer = (UInt32)(sizeof(UInt32)+(2*sizeof(Int32))+m_SchemaURN.GetMaxLocaleCharBytes()+m_Text.GetMaxLocaleCharBytes());
        compressableSink.put (totalSizeOfXMLDataBuffer);
        compressableSink.put ((Int32)m_SchemaURN.GetMaxLocaleCharBytes());
        compressableSink.put (m_SchemaURN);
        compressableSink.put ((Int32)m_Text.GetMaxLocaleCharBytes());
        compressableSink.put (m_Text);

        BeAssert (compressableSink.getBytesWritten() <= totalSizeOfXMLDataBuffer);
        Int32 totalSizeOfBuffer = totalSizeOfXMLDataBuffer + (2*sizeof(Int32));

        // If caller requested compressed Xml data, then compress it (Xml data is not compressed by default)
        if (m_isCompressed)
            {
            encoding = TFStream_encoding_gZip;

            ULong compressSize = ((UInt32)((double)totalSizeOfXMLDataBuffer * 1.2)) + 8;

            DgnZLib::Zipper zip(compressSize);
            if (ZIP_SUCCESS != zip.Write(compressableSink.getBuf(), totalSizeOfXMLDataBuffer))
                return ERROR;

            if (ZIP_SUCCESS != zip.Finish())
                return ERROR;

            linkageSink.put (encoding);
            linkageSink.put (totalSizeOfBuffer);
            linkageSink.put(zip.GetResult(), (size_t)zip.GetCompressedSize());
            }
        else
            {
            linkageSink.put (encoding);
            linkageSink.put (totalSizeOfBuffer);
            linkageSink.put (compressableSink.getBuf(), compressableSink.getBytesWritten()); 
            }

        UInt32 bufSize = (UInt32)linkageSink.getBytesWritten();

        m_pStreamBuffer = (byte*) memutil_calloc (1, bufSize, 'XMLd');
        if (!m_pStreamBuffer)
            return DGNPLATFORM_STATUS_InsfMemory;

        memcpy (m_pStreamBuffer, linkageSink.getBuf(), bufSize);
        m_ulStreamBufferSize = bufSize;
        }

    if (pulBufferSize)
        *pulBufferSize = m_ulStreamBufferSize;

    if (ppBuffer)
        *ppBuffer = m_pStreamBuffer;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragment::AttachToElement (EditElementHandleR eeh)
    {
    int                 status;
    UInt32              ulXmlLinkageSize;
    MSXMLLinkage      * pstrMSXMLLinkage;

    UShort              appID   = GetAppID ();
    UShort              appType = GetAppType ();

    DgnV8ElementBlank elm;
    eeh.GetElementCP()->CopyTo (elm);

    byte*       pBuffer;
    UInt32      ulBufferSize;

    // get or construct buffer of data for linkage
    if (SUCCESS != (status = GetStreamData (&pBuffer, &ulBufferSize)))
        return status;

    if (!XMLClass_isSpaceForAttachments (&elm, ulBufferSize))
        {
        // No room on element for XML linkage
        return DGNPLATFORM_STATUS_ElemTooLarge;
        }

    ulXmlLinkageSize = (sizeof (MSXMLLinkage) + ulBufferSize + 7) & ~7;
    ScopedArray<byte>    scoped((size_t)ulXmlLinkageSize);
    pstrMSXMLLinkage = (MSXMLLinkage*)scoped.GetData();
    if (!pstrMSXMLLinkage)
        return DGNPLATFORM_STATUS_InsfMemory;

    memset (pstrMSXMLLinkage, 0, ulXmlLinkageSize);

    /* Set up xmlLinkage.header */
    pstrMSXMLLinkage->header.primaryID  = LINKAGEID_XML;
    pstrMSXMLLinkage->header.user       = true;

    // since we are passing NULL for conversion rules set size now    
    LinkageUtil::SetWords (&pstrMSXMLLinkage->header, ulXmlLinkageSize);

    /* Set up xmlLinkage.data */
    pstrMSXMLLinkage->data.linkageType  = LINKAGETYPE_XML;
    pstrMSXMLLinkage->data.appID        = appID;
    pstrMSXMLLinkage->data.appType      = appType;
    pstrMSXMLLinkage->data.numBytes     = ulBufferSize;
    memcpy (&pstrMSXMLLinkage->data.byteBuffer, pBuffer, ulBufferSize);

    status = linkage_appendToElement (&elm, &pstrMSXMLLinkage->header, &pstrMSXMLLinkage->data, s_pXMLLinkageConvRules);

    if (SUCCESS != status)
        return status;

    eeh.ReplaceElement (&elm);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragment::CreateXmlElement
(
EditElementHandleR eeh,
DgnModelP        model,
bool            setNonModelFlag
)
    {
    byte*           pBuffer=NULL;
    UInt32          ulBufferSize=0L;
    StatusInt       status;
    UInt32          concatenatedAppIds;

    if (SUCCESS != (status = GetStreamData (&pBuffer, &ulBufferSize)))
        return status;

    if (!pBuffer)
        return DGNPLATFORM_STATUS_InsfMemory;

    concatenatedAppIds = ((UInt32)m_appID << 16) + m_appType; 

    status = (StatusInt) DgnStoreHdrHandler::Create (eeh, pBuffer, ulBufferSize, XMLFRAGMENT_ID, concatenatedAppIds, *model);
    if (SUCCESS == status && setNonModelFlag)
        {
        MSElementDescrPtr edP = eeh.ExtractElementDescr ();
        edP->ElementR().SetDictionary(true);
        eeh.SetElementDescr (edP.get(), false); 
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlFragment::IsFragmentElement (ElementHandleCR eh, UInt16* pAppID, UInt16* pAppType)
    {
    UInt32 dgnStoreId = XMLFRAGMENT_ID;
    UInt32 applicationId = 0;

    // If caller passes appType then they must also pass appId
    if ((NULL == pAppID) && (NULL != pAppType))
        return false;

    // if both are defined then build the applicationId to be checked
    if ((NULL != pAppID) && (NULL != pAppType))
        applicationId = ((UInt32)*pAppID << 16) + *pAppType;

    if (!DgnStoreHdrHandler::IsDgnStoreElement (eh, dgnStoreId, applicationId))
        return false;

    // This is an XML Fragment element and it either matches the applicationId or the caller doesn't care to test app id & type, so return
    if (0 != applicationId || (NULL == pAppID) && (NULL == pAppID))
        return true;

    UInt16 appID=0;

    DgnStoreHdrHandler::GetDgnStoreIds (NULL, &applicationId, eh);

    // if pAppType is specified then pAppID  must also be defined so if we get here we only need to test the appId
    XMLClass_extractIDandTypeFromDGNStoreApplicationID (&appID, NULL, applicationId);

    return (appID == *pAppID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr  XmlFragment::ConstructFromData 
(
DataInternalizer& source, 
bool                      isCompressed, 
UInt16                    appId, 
UInt16                    appType
)
    {
    WString schemaUrn = L"";
    WString xmlText = L"";

    UInt32 xmlBufferSize;
    source.get (&xmlBufferSize);

    Int32 schemaLength;
    source.get (&schemaLength);

    if (schemaLength > 0)
        source.get (schemaUrn);

    Int32 xmlTextLength;
    source.get (&xmlTextLength);

    if (xmlTextLength > 0)
        source.get (xmlText);

    return XmlFragment::Construct (appId, appType, isCompressed, schemaUrn.c_str(), xmlText.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* The buffer is either from linkage data or and XML Fragment element.
*
* The format of the data is as follows
*    Int32               TFStream_encoding_BinaryCrossPlatform=1 or TFStream_encoding_gZip=2
*    UInt32               streamSize;   Total Size of Stream
*    ---------------------------------------------------------------
*    UInt32              total size of XmlFragment data
*    UInt32               # of bytes to hold schema including trailing 0
*    byte*               schema urn text
*    UInt32               # of bytes to hold XML fragment text including trailing 0
*    byte*               XML fragment text
*
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr XmlFragment::ConstructFromBuffer
(
byte*       pBuffer,
UInt32      ulBufferSize,
UInt16      appId,
UInt16      appType
)
    {
    XmlFragmentPtr pXMLFragment;
    DataInternalizer source (pBuffer, ulBufferSize);

    Int32 encoding;
    source.get (&encoding);

    Int32 streamSize;
    source.get (&streamSize);

    if (encoding == TFStream_encoding_gZip)
        {
        ULong    unCompressSize      = (UInt32)(streamSize * 5);    // provide for 5x expansion

        ScopedArray<byte>    scoped((size_t)unCompressSize);
        byte* pBufferUnCompressed = scoped.GetData();
        UInt32  bytesActuallyRead = 0;

        DgnZLib::UnZipper unzip;
        unzip.Init(source.getPos(), (UInt32)source.getRemainingSize());
        if (ZIP_SUCCESS == unzip.Read(pBufferUnCompressed, unCompressSize, &bytesActuallyRead))
            {
            DataInternalizer xmlFragmentData (pBufferUnCompressed, unCompressSize);
            pXMLFragment = XmlFragment::ConstructFromData (xmlFragmentData, true, appId, appType);
            }
        }
    else
        {
        pXMLFragment = XmlFragment::ConstructFromData (source, false, appId, appType);
        }

    return pXMLFragment;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr XmlFragment::ConstructFromXmlFragmentElement (ElementHandleCR eh)
    {
    byte*           pData = NULL;
    UInt32          ulDataSize=0L;
    XmlFragmentPtr  pXMLFragment;

    if (XmlFragment::IsFragmentElement (eh) && 
        SUCCESS == (int)DgnStoreHdrHandler::Extract ((void **)&pData, &ulDataSize, XMLFRAGMENT_ID, 0, eh))
        {
        UInt32 concatenatedAppIds=0L;
        UInt16 appID=0;
        UInt16 appType=0;

        DgnStoreHdrHandler::GetDgnStoreIds (NULL, &concatenatedAppIds, eh);

        XMLClass_extractIDandTypeFromDGNStoreApplicationID (&appID, &appType, concatenatedAppIds);

        pXMLFragment = XmlFragment::ConstructFromBuffer (pData, ulDataSize, appID, appType);
        }

    if (pData)
        DgnStoreHdrHandler::FreeExtractedData (pData);

    return pXMLFragment;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragment::StripFromElementByAppIDAndType
(
EditElementHandleR eeh,
UShort*         pAppID,
UShort*         pAppType
)
    {
    StatusInt status = SUCCESS;

    if (!eeh.IsValid())
        return DGNPLATFORM_STATUS_BadArg;

#ifdef IS_THIS_NEEDED
    //make it possible to pass NULL for ppXMLFragmentList without causing error in extractXMLFragmentFromElementByAppIDAndType
        status = extractXMLFragmentFromElementByAppIDAndType (ppXMLFragmentList, pElm, pAppID, pAppType, false, NULL);
#endif

    deleteXMLFragmentFromElementByAppIDAndType (eeh.GetElementP (), pAppID, pAppType, NULL, NULL);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragment::StripAllFromElement
(
EditElementHandleR eeh
)
    {
    return StripFromElementByAppIDAndType (eeh, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    MattWatkins    07.2003
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    XmlFragment::StripFromElement
(
DgnElement*      pElm,
UShort*         pAppID,
UShort*         pAppType,
bool          (*linkFunc)(LinkageHeader *, XmlFragmentPtr, void *),
void*           pUserParams
)
    {
    if (NULL == pElm)
        return DGNPLATFORM_STATUS_BadArg;

    return deleteXMLFragmentFromElementByAppIDAndType (pElm, pAppID, pAppType, linkFunc, pUserParams);
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlFragment::ElementHasXmlFragmentLinkage
(
ElementHandleCR            eh,
const UShort*           pAppID,
const UShort*           pAppType
)
    {
    bool                fragmentsPresent=false;

    if (eh.IsValid())
        {
        if (SUCCESS == extractXMLFragmentFromElementByAppIDAndType (NULL, eh.GetElementCP (), pAppID, pAppType, true, &fragmentsPresent))
            return (true == fragmentsPresent);
        }

    return false;
    }

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//  XmlFragmentList
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentList::XmlFragmentList ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentList::XmlFragmentList (UShort appID, UShort appType, bool isCompressed, WCharCP pSchemaURN, WCharCP pText)
    {
    m_fragments.push_back (XmlFragment::Construct (appID, appType, isCompressed, pSchemaURN, pText));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentList::XmlFragmentList (XmlFragmentPtr& xmlFragment)
    {
    m_fragments.push_back (xmlFragment);
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    CaseyMullen     03/01
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentListPtr XmlFragmentList::ConstructFromFragment (XmlFragmentPtr& xmlFragment)
    {
    return new XmlFragmentList (xmlFragment);
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    CaseyMullen     03/01
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentListPtr XmlFragmentList::Construct
(
WChar     * pText,
WChar     * pSchemaURN,
UShort        appID,
UShort        appType
)
    {
    return XmlFragmentList::ConstructFromFragment (XmlFragment::Construct (appID, appType, false, pSchemaURN, pText));
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentListPtr XmlFragmentList::ConstructFromBuffer
(
byte*       pBuffer,
UInt32      ulBufferSize,
UInt16      appId,
UInt16      appType
)
    {
    XmlFragmentListPtr pXMLFragmentList;

    XmlFragmentPtr pXMLFragment = XmlFragment::ConstructFromBuffer (pBuffer, ulBufferSize, appId, appType);
    if (pXMLFragment.IsValid())
        pXMLFragmentList = XmlFragmentList::ConstructFromFragment (pXMLFragment);

    return pXMLFragmentList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentList::~XmlFragmentList ()
    {
    if (!m_fragments.empty())
        m_fragments.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    CaseyMullen     03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragmentList::Append
(
XmlFragmentPtr&   pXMLFragment
)
    {
    if (!pXMLFragment.IsValid())
        return DGNPLATFORM_STATUS_BadArg;

    m_fragments.push_back (pXMLFragment);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    CaseyMullen     03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt XmlFragmentList::Prepend
(
XmlFragmentPtr&   pXMLFragment
)
    {
    if (!pXMLFragment.IsValid())
        return DGNPLATFORM_STATUS_BadArg;

    m_fragments.insert (m_fragments.begin(), pXMLFragment);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentListPtr XmlFragmentList::ExtractFromElement
(
ElementHandleCR            eh,
const UShort*           pAppID,
const UShort*           pAppType
)
    {
    StatusInt           status;
    bool                fragmentsPresent=false;
    XmlFragmentListPtr  pXMLFragmentList;

    if (eh.IsValid())
        {
        // set the XmlFragmentListPtr to point to an empty fragment list to be populated when the linkages are processed.
        XmlFragmentList* extractedFragmentList = new XmlFragmentList ();
        status = extractXMLFragmentFromElementByAppIDAndType (extractedFragmentList, eh.GetElementCP (), pAppID, pAppType, false, &fragmentsPresent);

        pXMLFragmentList = extractedFragmentList;
        }
        
    return pXMLFragmentList;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      XmlFragmentList::GetCount()
    {
    return m_fragments.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<XmlFragmentPtr>& XmlFragmentList::GetVectorOfXmlFragments()
    {
    return m_fragments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragmentList::Remove (size_t indexOfFragment)
    {
    if (indexOfFragment < m_fragments.size ())
        return DGNHANDLERS_STATUS_BadArg;

    m_fragments.erase (m_fragments.begin()+indexOfFragment);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
XmlFragmentPtr XmlFragmentList::GetFragmentAtIndex (size_t index)   
    {
    if (index < m_fragments.size())
        return m_fragments[index];

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    CaseyMullen     03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt checkForSufficientSpace
(                       /* <= SUCCESS, ERROR, or MDLERR_TOOLARGE */
DgnElementCP                  pElm,
bvector<XmlFragmentPtr>& fragmentList
)
    {
    UInt32              ulTotalBufferSize=0;
    StatusInt           status;
      
    if (0 == fragmentList.size ())
        return SUCCESS;

    /* Determine if list of XMLFragments will fit on the element */
    for (bvector<XmlFragmentPtr>::iterator it = fragmentList.begin(); it != fragmentList.end();  it++)
        {
        byte*       pBuffer=NULL;
        UInt32      ulBufferSize=0L;

        status = (*it)->GetStreamData (&pBuffer, &ulBufferSize);
        if (SUCCESS != status)
            return status;

        if (!XMLClass_isSpaceForAttachments (pElm, ulBufferSize))
            return DGNPLATFORM_STATUS_TooLarge;

        // Add on size of header info 
        ulTotalBufferSize += (sizeof(MSXMLLinkage) + ulBufferSize);
        }

    // Subtract one instance of header info size since XMLClass_isSpaceForAttachments takes it into account.
    ulTotalBufferSize -= sizeof(MSXMLLinkage);

    /* Test total size of all XMLFragments in list */
    if (!XMLClass_isSpaceForAttachments (pElm, ulTotalBufferSize))
        return DGNPLATFORM_STATUS_TooLarge;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<XmlFragmentPtr>::iterator XmlFragmentList::begin () 
    {
    return  m_fragments.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<XmlFragmentPtr>::iterator XmlFragmentList::end () 
    {
    return  m_fragments.end();
    }

/*---------------------------------------------------------------------------------**//**
* 
* @bsimethod                                                    ChrisDalesandro  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XmlFragmentList::AttachToElement
(
EditElementHandleR eeh
)
    {
    StatusInt           status;

    if (!eeh.IsValid())
        return DGNPLATFORM_STATUS_BadArg;

    if (0 == m_fragments.size ())
        return DGNPLATFORM_STATUS_NoChange;

   /* Determine if list of XMLFragments will fit on the element */
   status = checkForSufficientSpace (eeh.GetElementCP (), m_fragments);
    if (SUCCESS != status)
        return status;

    /* add each fragment specified in list */
    for (bvector<XmlFragmentPtr>::iterator it = m_fragments.begin(); it != m_fragments.end();  it++)
        {
        if (SUCCESS != (status = (*it)->AttachToElement (eeh)))
            return status;
        }

    return SUCCESS;
    }
