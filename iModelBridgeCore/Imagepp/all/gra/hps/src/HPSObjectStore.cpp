//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSObjectStore.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class HPSObjectStore
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HIMOnDemandMosaic.H>
#include <Imagepp/all/h/HPSObjectStore.h>
#include <Imagepp/all/h/HPSParser.h>
#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRFException.h>

#include "HPSInternalNodes.h"
#include "HPSParserScope.h"

//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
/*static*/ void HPSObjectStore::GetURLsFromChildrenNode(const HPANode* pi_pParentNode, ListOfRelatedURLs& po_rRelatedURLs)
{
    HPANodeList::const_iterator NodeIter    = pi_pParentNode->GetSubNodes().begin();
    HPANodeList::const_iterator NodeIterEnd = pi_pParentNode->GetSubNodes().end();
    
    while (NodeIter != NodeIterEnd)
    {  
        //ImageFileExpressionNode found, stop the recursivity
        if (dynamic_cast<ImageFileExpressionNode*>((*NodeIter).GetPtr()) != 0)
        {            
            HFCPtr<HFCURL> pFileURL;

            ((ImageFileExpressionNode*)(*NodeIter).GetPtr())->GetFileURL(pFileURL);

            po_rRelatedURLs.push_back(pFileURL);                                    
        }        
        else
        if (dynamic_cast<VariableRefNode*>((*NodeIter).GetPtr()) != 0)
        {
            const HPANode* pNode = ((HFCPtr<VariableRefNode>&)(*NodeIter))->
                                                    GetVariableTokenNode()->
                                                    GetExpressionNode();

            GetURLsFromChildrenNode(pNode, po_rRelatedURLs);                        
        }
        else
        {
            GetURLsFromChildrenNode((*NodeIter).GetPtr(), po_rRelatedURLs);                        
        }   

        NodeIter++;
    }  
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPSObjectStore::HPSObjectStore(HPMPool*                         pi_pLog,
                               HFCPtr<HFCURL>                   pi_pURL,
                               HFCPtr<HFCURL>&                  pi_pCacheFileForPSSURL,
                               const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                               HFCPtr<HFCURL>                   pi_pSessionURL,
                               const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster,
                               const HGF2DWorldIdentificator*   pi_pCurrentWorldId)
    : HPMObjectStore(pi_pLog),
      m_Parser(),
      m_pBinStream(0),
      m_pWorldCluster(pi_rpWorldCluster),
      m_pURL(pi_pURL)
    {
    Construct(pi_pLog,
              pi_pURL,
              pi_pCacheFileForPSSURL,
              pi_rpWorldCluster,
              pi_pSessionURL,
              pi_rpHPSWorldCluster,
              pi_pCurrentWorldId);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPSObjectStore::HPSObjectStore(HPMPool*                         pi_pLog,
                               HFCPtr<HFCURL>                   pi_pURL,
                               const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                               HFCPtr<HFCURL>                   pi_pSessionURL,
                               const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster,
                               const HGF2DWorldIdentificator*   pi_pCurrentWorldId)
    : HPMObjectStore(pi_pLog),
      m_Parser(),
      m_pBinStream(0),
      m_pWorldCluster(pi_rpWorldCluster),
      m_pURL(pi_pURL)
    {
    try
        {
        try
            {
            if (HRFiTiffCacheFileCreator::GetInstance()->HasCacheFor(pi_pURL))
                {              
                HFCPtr<HFCURL>               pCacheURL(HRFiTiffCacheFileCreator::GetInstance()->GetCacheURLFor(pi_pURL));

                if (HRFcTiffCreator::GetInstance()->IsKindOfFile(pCacheURL))
                    {
                    m_pCacheFileForPSS = HRFcTiffCreator::GetInstance()->Create(pCacheURL);

                    HASSERT(m_pCacheFileForPSS != 0);

                    //The cache file is invalid if no information about the on-demand rasters are found.
                    if (!(m_pCacheFileForPSS->GetPageDescriptor(0)->HasTag<HRFAttributeOnDemandRastersInfo>())) 
                        {
                        m_pCacheFileForPSS = 0;
                        }
                    }
                }
            }
        catch (HFCException&)
            {
            //If an exception occurs, assume that there is no cache file available.
            }

        //If no cache file was found, parse the PSS.
        if (m_pCacheFileForPSS == 0)
            {
            m_pBinStream = HFCBinStream::Instanciate(pi_pURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 0, true);

            m_pNode = (HPSNode*)(m_Parser.Parse(m_pBinStream,
                                                pi_pLog,
                                                pi_pSessionURL,
                                                pi_rpHPSWorldCluster,
                                                pi_pCurrentWorldId));
            HASSERT(m_pNode != 0);
            HASSERT(m_pNode->GetGrammarObject() == &m_Parser.PictureScript);
            
            if (pi_rpWorldCluster != 0)
                {
                // Link the PSS world cluster to the received one.
                HFCPtr<HGF2DCoordSys> pPSSUnknown(GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                HFCPtr<HGF2DCoordSys> pAppUnknown(pi_rpWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                pPSSUnknown->SetReference(HGF2DIdentity(), pAppUnknown);
                }
            }
        else
            {
            m_pNode = 0;
            }
        }
    catch(...)
        {
        if (m_pBinStream != 0)
            delete m_pBinStream;
        throw;
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPSObjectStore::Construct(HPMPool*                         pi_pLog,
                               HFCPtr<HFCURL>                   pi_pURL,
                               HFCPtr<HFCURL>&                  pi_pCacheFileForPSSURL,
                               const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                               HFCPtr<HFCURL>                   pi_pSessionURL,
                               const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster,
                               const HGF2DWorldIdentificator*   pi_pCurrentWorldId)
    {
    try
        {
        try
            {
            HFCPtr<HFCURL> pCacheURL(pi_pCacheFileForPSSURL);

            if ((pCacheURL == 0) && HRFiTiffCacheFileCreator::GetInstance()->HasCacheFor(pi_pURL))
                {
                pCacheURL = HRFiTiffCacheFileCreator::GetInstance()->GetCacheURLFor(pi_pURL);
                }

            if ((pCacheURL != 0) &&
                (HRFcTiffCreator::GetInstance()->IsKindOfFile(pCacheURL) == true))
                {
                

                m_pCacheFileForPSS = HRFcTiffCreator::GetInstance()->Create(pCacheURL);

                HASSERT(m_pCacheFileForPSS != 0);

                //The cache file is invalid if no information about the on-demand rasters are found.
                if (!(m_pCacheFileForPSS->GetPageDescriptor(0)->HasTag<HRFAttributeOnDemandRastersInfo>()))
                    {
                    m_pCacheFileForPSS = 0;
                    }
                }
            }
        catch (HFCException&)
            {
            //If an exception occurs, assume that there is no cache file available.
            }

        //If no cache file was found, parse the PSS.
        if (m_pCacheFileForPSS == 0)
            {
            m_pBinStream = HFCBinStream::Instanciate(pi_pURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 0, true);

            m_pNode = (HPSNode*)(m_Parser.Parse(m_pBinStream,
                                                pi_pLog,
                                                pi_pSessionURL,
                                                pi_rpHPSWorldCluster,
                                                pi_pCurrentWorldId));
            HASSERT(m_pNode != 0);
            HASSERT(m_pNode->GetGrammarObject() == &m_Parser.PictureScript);

            if (pi_rpWorldCluster != 0)
                {
                // Link the PSS world cluster to the received one.
                HFCPtr<HGF2DCoordSys> pPSSUnknown(GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                HFCPtr<HGF2DCoordSys> pAppUnknown(pi_rpWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
                pPSSUnknown->SetReference(HGF2DIdentity(), pAppUnknown);
                }
            }
        else
            {
            m_pNode = 0;
            }
        }
    catch(...)
        {
        if (m_pBinStream != 0)
            delete m_pBinStream;
        throw;
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPSObjectStore::~HPSObjectStore()
    {
    delete m_pBinStream;

    //IMPORTANT : Don't call delete on m_pNode because it triggers a recursive
    //            deletion than can lead to some stack overflow. Delete the
    //            node one at a time instead starting from the leaf of the node
    //            tree to its root.
    HPANode*        pNode = m_pNode;
    HFCPtr<HPANode> pTempNode;

    while (pNode != 0)
        {
        //Check for last child
        if (pNode->GetSubNodes().size() > 0)
            {
            pNode = pNode->GetSubNodes()[pNode->GetSubNodes().size() - 1];
            }
        else //Node has no child, delete the node
            {
            pTempNode = pNode;

            //Remove the node from its parent
            if (pTempNode->GetOwner() != 0)
                {
                ((HPANodeList&)pTempNode->GetOwner()->GetSubNodes()).pop_back();
                }

            pNode = pTempNode->GetOwner();

            //Delete node
            pTempNode = 0;
            }
        }
    }

//---------------------------------------------------------------------------
// Object ID is page number
//---------------------------------------------------------------------------
HFCPtr<HRARaster> HPSObjectStore::LoadRaster(uint32_t pi_PageID)
    {
    HPRECONDITION(pi_PageID < CountPages());

    HFCPtr<HRARaster> pRaster;

    if (m_pCacheFileForPSS != 0)
        {                            
        HRFDownSamplingMethod::DownSamplingMethod cacheFileDownsamplingMethod;
        HFCPtr<HGF2DCoordSys>        pLogical;                    
        HFCPtr<HRSObjectStore>       pStore;
        HFCPtr<HRARaster>                         pSubResForPSS;
        HRFAttributeOnDemandRastersInfo const*    pODRastersInfoAttr;
        HAutoPtr<HIMOnDemandMosaic>               pMosaic;
        
        pLogical = m_pWorldCluster->GetCoordSysReference(m_pCacheFileForPSS->GetWorldIdentificator());
                
        HASSERT((m_pCacheFileForPSS->GetPageDescriptor(0) != 0) && (m_pCacheFileForPSS->GetPageDescriptor(0)->CountResolutions() > 0));
        
        cacheFileDownsamplingMethod = m_pCacheFileForPSS->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetDownSamplingMethod().m_DownSamplingMethod;

#ifdef __HMR_DEBUG

        //Only support cTIFF cache file whose decimation method are the same for all resolutions.
        for (unsigned short resolutionInd = 1; resolutionInd < m_pCacheFileForPSS->GetPageDescriptor(0)->CountResolutions(); resolutionInd++)
            {
            HASSERT(m_pCacheFileForPSS->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetDownSamplingMethod().m_DownSamplingMethod == 
                    m_pCacheFileForPSS->GetPageDescriptor(0)->GetResolutionDescriptor(resolutionInd)->GetDownSamplingMethod().m_DownSamplingMethod);
            }

#endif

        pStore = new HRSObjectStore(GetPool(),
                                    m_pCacheFileForPSS,
                                    0,
                                    pLogical);

        // Get the raster from the store
        pSubResForPSS = pStore->LoadRaster();

        HASSERT(pSubResForPSS->IsCompatibleWith(HRAPyramidRaster::CLASS_ID) == true);

        pLogical = m_pWorldCluster->GetCoordSysReference(1);        

        pMosaic = new HIMOnDemandMosaic(pLogical, m_pCacheFileForPSS->GetURL(), (HFCPtr<HRAPyramidRaster>&)pSubResForPSS, cacheFileDownsamplingMethod);   

        pODRastersInfoAttr = m_pCacheFileForPSS->GetPageDescriptor(0)->FindTagCP<HRFAttributeOnDemandRastersInfo>();

        if (pMosaic->Add(pODRastersInfoAttr->GetData(), GetPool(), m_pWorldCluster, m_pURL) == false)
            {
            throw HRFInvalidSisterFileException(m_pCacheFileForPSS->GetURL()->GetURL());
            }                

        pRaster = pMosaic.release();
        }
    else
        {   
        HASSERT(m_pNode != 0);
        pRaster = m_pNode->GetPage(pi_PageID);
        }
        
    return pRaster;
    }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPSObjectStore::Save(HPMPersistentObject* pi_pObj)
    {
    HPRECONDITION(false);  // Not supported, file is always read-only.
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPSObjectStore::IsReadOnly() const
    {
    return true;
    }

//----------------------------------------------------------------------------
// Make the database writeable or read-only
//----------------------------------------------------------------------------
void HPSObjectStore::ForceReadOnly(bool pi_ReadOnly)
    {
    // Nothing to do since store is always read-only
    }


HFCPtr<HGF2DWorldCluster> HPSObjectStore::GetWorldCluster() const
    {
    HFCPtr<HGF2DWorldCluster> pWorldCluster;

    //TBD - The world cluster of an PSS is the HPSWorldCluster which can be modified by statements
    //      in the PSS file. So we may need to save those statements in the cache file to reconstruct
    //      the cluster correctly??
    if (m_pCacheFileForPSS != 0)
        {
        //Currently returns the application world cluster, which is the one use for creating
        //the raster.
        pWorldCluster = m_pWorldCluster;
        }
    else
        {
        pWorldCluster = m_pNode->GetWorldCluster();
        }

    return pWorldCluster;
    }

HGF2DWorldIdentificator HPSObjectStore::GetWorldID() const
    {
    HGF2DWorldIdentificator WorldId;

    //TDB The world identificator of m_pNode correspond to the world id of the currently selected
    //    world in the session, which likely means, after the PSS parsing is done,
    //    the last selected world in the PSS. Maybe it can be unknown??
    if (m_pCacheFileForPSS != 0)
        {
        WorldId = m_pCacheFileForPSS->GetWorldIdentificator();
        }
    else
        {
        WorldId = m_pNode->GetWorldID();
        }

    return WorldId;
    }

uint32_t HPSObjectStore::CountPages() const
    {
    uint32_t NbPages;

    if (m_pCacheFileForPSS == 0)
        {
        HASSERT(m_pNode != 0);
        NbPages = m_pNode->CountPages();
        }
    else
        {
        NbPages = m_pCacheFileForPSS->CountPages();
        //Cached multipage PSS is not yet supported.
        HASSERT(NbPages == 1);
        }

    return NbPages;
    }

const PageStatementNode* HPSObjectStore::GetPageStatementNode(HPMObjectID pi_PageID) const
    {
    return m_pNode->GetPageStatementNode(pi_PageID);
    }