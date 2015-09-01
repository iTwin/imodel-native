//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSObjectStore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPSObjectStore
//---------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HGF2DWorldCluster.h"
#include "HPMObjectStore.h"
#include "HPSNode.h"
#include "HPSParser.h"
#include "HPSWorldCluster.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
class HPSStoreDescriptor;
class HFCURL;
class HFCBinStream;
class HRARaster;

class HPSObjectStore : public HPMObjectStore
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(HPSObjectStoreId, HPMObjectStore)

    IMAGEPP_EXPORT   HPSObjectStore(HPMPool*                         pi_pLog,
                            HFCPtr<HFCURL>                   pi_pURL,
                            HFCPtr<HFCURL>&                  pi_pCacheFileForPSSURL,
                            const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster = HFCPtr<HGF2DWorldCluster>(0),
                            const HFCPtr<HFCURL>             pi_pSessionURL = HFCPtr<HFCURL>(),
                            const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster = HFCPtr<HPSWorldCluster>(0),
                            const HGF2DWorldIdentificator*   pi_pCurrentWorldId = 0);

    IMAGEPP_EXPORT   HPSObjectStore(HPMPool*                         pi_pLog,
                            HFCPtr<HFCURL>                   pi_pURL,
                            const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster = HFCPtr<HGF2DWorldCluster>(0),
                            const HFCPtr<HFCURL>             pi_pSessionURL = HFCPtr<HFCURL>(),
                            const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster = HFCPtr<HPSWorldCluster>(0),
                            const HGF2DWorldIdentificator*   pi_pCurrentWorldId = 0);
    virtual ~HPSObjectStore();


    // From HPMObjectStore
    virtual void            Save(HPMPersistentObject* pi_pObj) override;
    virtual bool            IsReadOnly() const override;
    virtual void            ForceReadOnly(bool pi_ReadOnly) override;
    
    // Added methods

        IMAGEPP_EXPORT /*IppImaging_Needs*/ HFCPtr<HGF2DWorldCluster> GetWorldCluster() const; 
        IMAGEPP_EXPORT /*IppImaging_Needs*/ HGF2DWorldIdentificator   GetWorldID() const;
        IMAGEPP_EXPORT uint32_t          CountPages() const;
        IMAGEPP_EXPORT const PageStatementNode* GetPageStatementNode(HPMObjectID pi_PageID) const;

    IMAGEPP_EXPORT HFCPtr<HRARaster> LoadRaster(uint32_t pi_pageID);

    IMAGEPP_EXPORT static void GetURLsFromChildrenNode(const HPANode* pi_pParentNode, ListOfRelatedURLs& po_rRelatedURLs); 

private:

    HPSObjectStore(const HPSObjectStore&);
    HPSObjectStore& operator=(const HPSObjectStore&);

    void Construct(HPMPool*                         pi_pLog,
                   HFCPtr<HFCURL>                   pi_pURL,
                   HFCPtr<HFCURL>&                  pi_pCacheFileForPSSURL,
                   const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                   HFCPtr<HFCURL>                   pi_pSessionURL,
                   const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster,
                   const HGF2DWorldIdentificator*   pi_pCurrentWorldId);

    HPSParser                       m_Parser;
    HFCBinStream*                   m_pBinStream;
    HPSNode*                        m_pNode;
    const HFCPtr<HGF2DWorldCluster> m_pWorldCluster;
    HFCPtr<HRFRasterFile>           m_pCacheFileForPSS;
    HFCPtr<HFCURL>                  m_pURL;
    };

END_IMAGEPP_NAMESPACE