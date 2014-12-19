//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSObjectStore.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HPSStoreDescriptor;
class HFCURL;
class HFCBinStream;
class HRARaster;


//MST : Should be moved in HPSUtility.h on BEIJING in HPSUtility.h should be moved in imagepp\all\h
_HDLLg void HPSGetURLsFromChildrenNode(const HPANode*     pi_pParentNode,
                                       ListOfRelatedURLs& po_rRelatedURLs); 

class HPSObjectStore : public HPMObjectStore
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(1532, HPMObjectStore)

    _HDLLg   HPSObjectStore(HPMPool*                         pi_pLog,
                            HFCPtr<HFCURL>                   pi_pURL,
                            HFCPtr<HFCURL>&                  pi_pCacheFileForPSSURL,
                            const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster = HFCPtr<HGF2DWorldCluster>(0),
                            const HFCPtr<HFCURL>             pi_pSessionURL = HFCPtr<HFCURL>(),
                            const HFCPtr<HPSWorldCluster>&   pi_rpHPSWorldCluster = HFCPtr<HPSWorldCluster>(0),
                            const HGF2DWorldIdentificator*   pi_pCurrentWorldId = 0);

    _HDLLg   HPSObjectStore(HPMPool*                         pi_pLog,
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

        _HDLLg /*IppImaging_Needs*/ HFCPtr<HGF2DWorldCluster> GetWorldCluster() const; 
        _HDLLg /*IppImaging_Needs*/ HGF2DWorldIdentificator   GetWorldID() const;
        _HDLLg uint32_t          CountPages() const;
        _HDLLg const PageStatementNode* GetPageStatementNode(HPMObjectID pi_PageID) const;

    _HDLLg HFCPtr<HRARaster> LoadRaster(uint32_t pi_pageID);

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

