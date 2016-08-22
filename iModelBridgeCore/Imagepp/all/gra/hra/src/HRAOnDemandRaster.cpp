//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAOnDemandRaster.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HPSObjectStore.h>
//#include <all/gra/hps/src/HPSUtility.h>
#include <Imagepp/all/h/HRAOnDemandRaster.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>

#include <Imagepp/all/h/HRFUtility.h>

#include "../../hps/src/HPSParserScope.h"

#include <BeXml/BeXml.h>

#define ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS 10

void ImagePP::GetDoubleFormatting(Utf8Char* format, size_t maxNbChars)
    {
    size_t nbCharWritten = BeStringUtilities::Snprintf(format, maxNbChars, "%s.%df", "%", ODMO_XML_SERIALIZATION_SIGNIFICANT_DIGITS);
    assert(nbCharWritten < maxNbChars);
    }


//-----------------------------------------------------------------------------
// Default Constructor.
//-----------------------------------------------------------------------------
HRAOnDemandRaster::HRAOnDemandRaster(HPMPool*                       pi_pMemPool,
                                     bool                           pi_IsOpaque,
                                     const HFCPtr<HVEShape>&        pi_rpEffectiveShape,
                                     const Utf8String&                 pi_rRepresentativePSS,
                                     const HFCPtr<HPSWorldCluster>& pi_rpHPSWorldCluster,
                                     HGF2DWorldIdentificator        pi_CurrentWorldId,
                                     const HFCPtr<HFCURL>&          pi_rpPSSUrl, 
                                     bool                           pi_hasLookAhead,
                                     bool                           pi_isDataChangingWithResolution,                                               
                                     bool                           pi_hasUnlimitedRasterSource)
    {
    m_pMemPool                     = pi_pMemPool;
    m_IsOpaque                     = pi_IsOpaque;
    m_pEffectiveShape              = pi_rpEffectiveShape;   
    m_RepresentativePSS            = pi_rRepresentativePSS;         
    m_pHPSWorldCluster             = pi_rpHPSWorldCluster;
    m_CurrentWorldId               = pi_CurrentWorldId;
    m_pPSSUrl                      = pi_rpPSSUrl;
    m_hasLookAhead                 = pi_hasLookAhead;   
    m_hasUnlimitedRasterSource     = pi_hasUnlimitedRasterSource;  
    m_isDataChangingWithResolution = pi_isDataChangingWithResolution;        
    m_hasLastLoadFailed            = false;
    }


//-----------------------------------------------------------------------------
// Serializing Constructor.
//-----------------------------------------------------------------------------                           
HRAOnDemandRaster::HRAOnDemandRaster(BeXmlNode*                         pi_pOnDemandRasterXMLNode,                                     
                                     HPMPool*                           pi_pMemPool,
                                     const HFCPtr<HGF2DCoordSys>&       pi_rpCacheCoordSys,
                                     const HFCPtr<HPSWorldCluster>&     pi_rpHPSWorldCluster,
                                     HGF2DWorldIdentificator            pi_CurrentWorldId,
                                     const HFCPtr<HFCURL>&              pi_rpPSSUrl)
: m_pMemPool(pi_pMemPool),
  m_pHPSWorldCluster(pi_rpHPSWorldCluster),
  m_CurrentWorldId(pi_CurrentWorldId),
  m_pPSSUrl(pi_rpPSSUrl), 
  m_hasLastLoadFailed(false)
  {           
    HPRECONDITION(0 == BeStringUtilities::Stricmp (pi_pOnDemandRasterXMLNode->GetName(), "ODR"));    

    BeXmlNodeP pEffectiveShapeChildNode;
      
    pEffectiveShapeChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("ES");

    HASSERT(pEffectiveShapeChildNode != 0);
                  
    vector<double> EffectiveShapePts;
    double         EffectivePointCoord;
    BeXmlStatus    XmlStatus; 
    BeXmlNodeP     pEffectiveShapePointNode = pEffectiveShapeChildNode->GetFirstChild();    

    while (0 != pEffectiveShapePointNode) 
        {        
        HASSERT(0 == BeStringUtilities::Stricmp(pEffectiveShapePointNode->GetName(), "D"));    
        
        XmlStatus = pEffectiveShapePointNode->GetAttributeDoubleValue(EffectivePointCoord, "V"); 

        EffectiveShapePts.push_back(EffectivePointCoord);
                                              
        pEffectiveShapePointNode = pEffectiveShapePointNode->GetNextSibling();
        }
    
    HRFClipShape* pClipShape = ImportShapeFromArrayOfDouble(&(*EffectiveShapePts.begin()), 
                                                            EffectiveShapePts.size());
    HASSERT(pClipShape != 0);

    m_pEffectiveShape = pClipShape;
    m_pEffectiveShape->SetCoordSys(pi_rpCacheCoordSys);   

    BeXmlNodeP pChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("LA");

    HASSERT(pChildNode != 0);

    Utf8String NodeContent;
    
    pChildNode->GetContent(NodeContent);         

    if (true == NodeContent.Equals("1"))
        {
        m_hasLookAhead = true;
        }
    else
        {
        HASSERT(true == NodeContent.Equals("0"));
        m_hasLookAhead = false;
        }          
  
    pChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("ISO");

    HASSERT(pChildNode != 0);

    pChildNode->GetContent(NodeContent);         
               
    if (true == NodeContent.Equals("1"))
        {
        m_IsOpaque = true;
        }
    else
        {
        HASSERT(true == NodeContent.Equals("0"));
        m_IsOpaque = false;
        }      

    pChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("RPSS");

    HASSERT(pChildNode != 0);
                   
    //RepresentativePSS
    XmlStatus = pChildNode->GetContent(m_RepresentativePSS);

    HASSERT(BEXML_Success == XmlStatus);    

    //The nodes below might not be found in old cache. 
    ListOfRelatedURLs relatedURLs;

    pChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("IDCR");
        
    if (pChildNode != 0)
    {
        pChildNode->GetContent(NodeContent);     

        if (true == NodeContent.Equals("1"))
        {
            m_isDataChangingWithResolution = true;
        }
        else
        {
            HASSERT(true == NodeContent.Equals("0"));
            m_isDataChangingWithResolution = false;
        }      
    }
    else
    {           
        GetSourceFileURLs(relatedURLs);

        ListOfRelatedURLs::const_iterator relatedURLIter(relatedURLs.begin());
        ListOfRelatedURLs::const_iterator relatedURLIterEnd(relatedURLs.end());

        m_isDataChangingWithResolution = false;
    
        while (relatedURLIter != relatedURLIterEnd)
        {    
            if (HRFRasterFileFactory::GetInstance()->IsKindOfFile(HRFFileId_VirtualEarth, *relatedURLIter) || 
                HRFRasterFileFactory::GetInstance()->IsKindOfFile(HRFFileId_WMS, *relatedURLIter))
            {
                m_isDataChangingWithResolution = true;
                break;
            }    

            relatedURLIter++;
        }    
    }   

    pChildNode = pi_pOnDemandRasterXMLNode->SelectSingleNode("IUR");
        
    if (pChildNode != 0)
    {
        pChildNode->GetContent(NodeContent);     

        if (true == NodeContent.Equals("1"))
        {
            m_hasUnlimitedRasterSource = TRUE;
        }
        else
        {
            HASSERT(true == NodeContent.Equals("0"));
            m_hasUnlimitedRasterSource = FALSE;
        }      
    }
    else
    {    
        if (relatedURLs.size() == 0)
        {
            GetSourceFileURLs(relatedURLs);
        }

        m_hasUnlimitedRasterSource = false;

#if defined (__IPP_EXTERNAL_THIRD_PARTY_SUPPORTED)
        ListOfRelatedURLs::const_iterator relatedURLIter(relatedURLs.begin());
        ListOfRelatedURLs::const_iterator relatedURLIterEnd(relatedURLs.end());

        HRFRasterFileFactory* pRasterFileFactory = HRFRasterFileFactory::GetInstance();

        while (relatedURLIter != relatedURLIterEnd)
        {    
            if (pRasterFileFactory->IsKindOfFile(HRFFileId_PDF, *relatedURLIter) || 
                pRasterFileFactory->IsKindOfFile(HRFFileId_WMS, *relatedURLIter))
            {
                m_hasUnlimitedRasterSource = true;
                break;
            }    

            relatedURLIter++;
        } 
#endif           
    }     
    }


//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HRAOnDemandRaster::~HRAOnDemandRaster()
    {
    }

//-----------------------------------------------------------------------------
// Check if the raster is opaque
//-----------------------------------------------------------------------------
bool HRAOnDemandRaster::IsOpaque() const
    {
    return m_IsOpaque;
    }

//-----------------------------------------------------------------------------
// Load the raster in memory
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAOnDemandRaster::LoadRaster() const
    {
    HFCPtr<HRARaster> pRaster;

    //Once a raster loading failed don't try to load it again.
    if (m_hasLastLoadFailed == false)
        {
        try
            {
            HFCPtr<HPSObjectStore> pPSSObjectStore;

            GetHPSObjectStore(pPSSObjectStore);

            // Get the raster from the store
            pRaster = pPSSObjectStore->LoadRaster(0);

            HASSERT(pRaster != NULL);
            //The two shapes define the same area
            HASSERT((m_pEffectiveShape->GetShapePtr()->CalculateSpatialPositionOf(*(pRaster->GetEffectiveShape()->GetShapePtr())) == HVE2DShape::S_ON) &&
                    (pRaster->GetEffectiveShape()->GetShapePtr()->CalculateSpatialPositionOf(*(m_pEffectiveShape->GetShapePtr())) == HVE2DShape::S_ON));                                    
            }
        catch (HFCException& )
            {
            m_hasLastLoadFailed = true;
            }
        } 

    return pRaster;
    }

//-----------------------------------------------------------------------------
// Get the HPS object store for loading the raster
//-----------------------------------------------------------------------------
void HRAOnDemandRaster::GetHPSObjectStore(HFCPtr<HPSObjectStore>& po_rpHPSObjectStore) const
    {
    Utf8String representativePSS_UTF8(m_RepresentativePSS);

    size_t NbBytesInPSS = representativePSS_UTF8.size();
    
    HFCPtr<HFCBuffer> pBuffer(new HFCBuffer(NbBytesInPSS));
    pBuffer->AddData((const Byte*)representativePSS_UTF8.c_str(), NbBytesInPSS);
    
    HFCPtr<HFCURL> pURLMemFile = new HFCURLMemFile(HFCURLMemFile::s_SchemeName() + "://Raster", pBuffer);

    po_rpHPSObjectStore = new HPSObjectStore(m_pMemPool,
                                             pURLMemFile,
                                             0,
                                             m_pPSSUrl,
                                             m_pHPSWorldCluster,
                                             &m_CurrentWorldId);
    }

//-----------------------------------------------------------------------------
// Get the list of URLs representing the source file composing this raster
//-----------------------------------------------------------------------------
bool HRAOnDemandRaster::GetSourceFileURLs(ListOfRelatedURLs& po_rRelatedURLs)
    {
    bool areSourceFileURLsAvailable;

    try 
    {
        HFCPtr<HPSObjectStore> pPSSObjectStore;

        GetHPSObjectStore(pPSSObjectStore);

        HASSERT(pPSSObjectStore->CountPages() == 1);

        const HPANode* pNode = (const HPANode*)pPSSObjectStore->GetPageStatementNode(0);

        GetURLsFromChildrenNode(pNode, po_rRelatedURLs);    

        areSourceFileURLsAvailable = true;
    }
    catch (HFCException& )
    {
        areSourceFileURLsAvailable = false;
    }

    return areSourceFileURLsAvailable;
}


//-----------------------------------------------------------------------------
// Get the URL of any children node which is an ImageFileExpressionNode
//-----------------------------------------------------------------------------
void HRAOnDemandRaster::GetURLsFromChildrenNode(const HPANode*     pi_pParentNode,
                                                ListOfRelatedURLs& po_rRelatedURLs) const
{   
    HPSObjectStore::GetURLsFromChildrenNode(pi_pParentNode, po_rRelatedURLs); 
}

/*-----------------------------------------------------------------------------
  Get the XML node representating the serialization of an 
  HRAOnDemandRaster object.

  GetSerializationXMLNode

  XML Schema - The node names are kept int16_t since many on-demand raster could be 
  in a on-demand mosaic.
  
  <ODR>             <!--OnDemandRaster-->                           
      <ES>          <!--EffectiveShape-->            
      <D V=/>
      </ES>         
      <LA></LA>     <!--LookAhead-->                           
      <ISO></ISO>   <!--IsOpaque-->       
      <RPSS></RPSS> <!--RepresentativePSS-->       
      <IDCR></IDCR> <!--IsDataChagingWithResolution-->       
      <IUR></IUR>   <!--IsUnlimitedRaster-->       
  </ODR>
/*-----------------------------------------------------------------------------*/
void HRAOnDemandRaster::GetSerializationXMLNode(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSysForSerialization,
                                                BeXmlNodeP                   pio_pOnDemandRasterXMLParentNode) const
    {
    HPRECONDITION(m_pEffectiveShape != 0);
    HPRECONDITION(pio_pOnDemandRasterXMLParentNode != 0);

    BeXmlNodeP pODRNode = pio_pOnDemandRasterXMLParentNode->AddEmptyElement("ODR");
  
    //Shape
    m_pEffectiveShape->ChangeCoordSys(pi_rpCoordSysForSerialization);

    HRFClipShape      ClipShape(*m_pEffectiveShape, HRFCoordinateType::PHYSICAL);
    HAutoPtr<double>  pSerializedShapePts;
    size_t            serializedShapeNbPts;                                    

    pSerializedShapePts = ExportClipShapeToArrayOfDouble(ClipShape, &serializedShapeNbPts);
    
    BeXmlNodeP pEffectiveShapeXMLNode = pODRNode->AddEmptyElement("ES");    
        
    string serializationString;
    
    size_t shapePointInd = 0; 

    Utf8Char doubleValueBuffer[DOUBLE_VALUE_BUFFER_LENGTH];
    Utf8Char format[DOUBLE_FORMATTING_BUFFER_LENGTH];

    GetDoubleFormatting(format, DOUBLE_FORMATTING_BUFFER_LENGTH);
    
    while (shapePointInd  < serializedShapeNbPts)
        {              
        BeXmlNodeP pDoubleXMLNode = pEffectiveShapeXMLNode->AddEmptyElement("D");    
         
        BeStringUtilities::Snprintf(doubleValueBuffer, DOUBLE_VALUE_BUFFER_LENGTH, format, pSerializedShapePts[shapePointInd]);
        pDoubleXMLNode->AddAttributeStringValue("V", doubleValueBuffer);       

        shapePointInd++;
        }                            
    
    //LookAhead                 
    BeXmlNodeP pLookAheadXMLNode = pODRNode->AddElementStringValue("LA", m_hasLookAhead ? "1" : "0");    
    HASSERT(pLookAheadXMLNode != 0);
    
    //Is Opaque
    BeXmlNodeP pIsOpaqueXMLNode = pODRNode->AddElementStringValue("ISO", m_IsOpaque ? "1" : "0");    
    HASSERT(pIsOpaqueXMLNode != 0);
           
    //RepresentativePSS
    BeXmlNodeP pRepresentativePssXMLNode = pODRNode->AddElementStringValue("RPSS", m_RepresentativePSS.c_str());    
    HASSERT(pRepresentativePssXMLNode != 0);
      
    //Is Data Changing With Resolution (e.g. : WMS, Virtual Earth).
    BeXmlNodeP pIsDataChangingWithResXMLNode = pODRNode->AddElementStringValue("IDCR", m_isDataChangingWithResolution ? "1" : "0");    
    HASSERT(pIsDataChangingWithResXMLNode != 0);
    
    //Has Unlimited Raster (e.g. : WMS, PDF).
    BeXmlNodeP pHasUnlimitedRasterSourceXMLNode = pODRNode->AddElementStringValue("IUR", m_hasUnlimitedRasterSource ? "1" : "0");    
    HASSERT(pHasUnlimitedRasterSourceXMLNode != 0);    
    }