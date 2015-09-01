//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAOnDemandRaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes any graphical object of raster type (graphics that
// are composed of pixels).  Abstract class.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HGF2DWorldCluster.h"
#include "HPSObjectStore.h"
#include "HPSWorldCluster.h"
#include "HRARaster.h"
#include "HRFRasterFile.h"
#include "HVEShape.h"

BEGIN_BENTLEY_NAMESPACE

    struct BeXmlNode;

END_BENTLEY_NAMESPACE

#define DOUBLE_VALUE_BUFFER_LENGTH 1024
#define DOUBLE_FORMATTING_BUFFER_LENGTH 24

BEGIN_IMAGEPP_NAMESPACE
void GetDoubleFormatting(WChar* format, size_t maxNbChars);

class HRAOnDemandRaster : public HFCShareableObject<HRAOnDemandRaster>
    {
public:

    // Primary methods
    IMAGEPP_EXPORT              HRAOnDemandRaster(HPMPool*                          pi_pMemPool,
                                          bool                              pi_IsOpaque,
                                          const HFCPtr<HVEShape>&           pi_rpEffectiveShape,
                                          const WString&                    pi_rRepresentativePSS,
                                          const HFCPtr<HPSWorldCluster>&    pi_rpHPSWorldCluster,
                                          HGF2DWorldIdentificator           pi_CurrentWorldId,
                                          const HFCPtr<HFCURL>&             pi_rpPSSUrl, 
                                          bool                              pi_hasLookAhead, 
                                          bool                              pi_isDataChangingWithResolution, 
                                          bool                              pi_hasUnlimitedRasterSource);                             
        
                        HRAOnDemandRaster(BeXmlNode*                        pi_pOnDemandRasterXMLNode, 
					                      HPMPool*                          pi_pMemPool,
                                          const HFCPtr<HGF2DCoordSys>&      pi_rpCacheCoordSys,
					                      const HFCPtr<HPSWorldCluster>&    pi_rpHPSWorldCluster,
                                          HGF2DWorldIdentificator           pi_CurrentWorldId,
                                          const HFCPtr<HFCURL>&             pi_rpPSSUrl);                      
                                                                           
    virtual             ~HRAOnDemandRaster();
                
    //Mosaic Indexing Methods
    HGF2DExtent         GetExtent() const;
    HFCPtr<HVEShape>    GetEffectiveShape () const;
    bool               IsOpaque() const;

    HFCPtr<HRARaster>   LoadRaster() const;

    //Cache Methods
        void                GetSerializationXMLNode(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSysForSerialization,                                            
                                                    BeXmlNode*                   pio_pOnDemandRasterXMLParentNode) const;        

        IMAGEPP_EXPORT bool         GetSourceFileURLs(ListOfRelatedURLs& po_rRelatedURLs);

    //Miscellaneous Methods
        WString             GetRepresentativePSS() const;

        bool                HasLookAhead() const;

        bool                HasUnlimitedRasterSource() const;
               
        bool                IsDataChangingWithResolution() const;

        bool                HasLastLoadFailed() const;
     

protected:

    void                GetHPSObjectStore(HFCPtr<HPSObjectStore>& po_rpHPSObjectStore) const;

    void                GetURLsFromChildrenNode(const HPANode*     pi_pParentNode,
                                                ListOfRelatedURLs& po_rRelatedURLs) const;

private:

    // Primary Methods
    HRAOnDemandRaster(const HRAOnDemandRaster& pi_rObj);
    HRAOnDemandRaster&  operator=(const HRAOnDemandRaster& pi_rObj);

    // Shaping and extent of the raster
    HFCPtr<HVEShape> m_pEffectiveShape;        
    bool             m_IsOpaque;
    HPMPool*         m_pMemPool;
    WString          m_RepresentativePSS;
    HFCPtr<HFCURL>   m_pPSSUrl;
    bool             m_hasLookAhead;
    bool             m_hasUnlimitedRasterSource;
    bool             m_isDataChangingWithResolution;
    mutable bool     m_hasLastLoadFailed;

    //World related information
    mutable HFCPtr<HPSWorldCluster> m_pHPSWorldCluster;
    HGF2DWorldIdentificator         m_CurrentWorldId;                      
    };
END_IMAGEPP_NAMESPACE

#include "HRAOnDemandRaster.hpp"
