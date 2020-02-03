//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ImageConverter/PSSUtilities.h $
//:>    $RCSfile: PSSUtilities.h,v $
//:>   $Revision: 1.1 $
//:>       $Date: 2008/12/22 16:24:44 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Contains PSS related utility functions.
//-----------------------------------------------------------------------------

#pragma once 

#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HIMOnDemandMosaic.h>
#include <Imagepp/all/h/HPSObjectStore.h>

//-----------------------------------------------------------------------------
// GenerateOnDemandMosaicPssFile
//-----------------------------------------------------------------------------
void GenerateOnDemandMosaicPssFile(Utf8String& pi_rpFolderName,                                   
                                   Utf8String& pi_rpPSSFileName);

//-----------------------------------------------------------------------------
// ReplacePSSMosaicByOnDemandMosaic
//-----------------------------------------------------------------------------
void ReplacePSSMosaicByOnDemandMosaic(HFCPtr<HFCURL>          pi_pFileName, 
                                      HFCPtr<HPSObjectStore>& pi_pObjectStore);     

//-----------------------------------------------------------------------------
// IsValidPSSCacheFile
//-----------------------------------------------------------------------------
bool IsValidPSSCacheFile(HFCPtr<HFCURL>&            pi_rpSrcFileName, 
                          HFCPtr<HIMOnDemandMosaic>& pi_pOnDemandMosaic);

//-----------------------------------------------------------------------------
// GetRasterSizeInPixel
//-----------------------------------------------------------------------------
void GetRasterSizeInPixel(HFCPtr<HRARaster>                pi_pRaster, 
                          const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster, 
                          uint64_t&                         po_rHeight, 
                          uint64_t&                         po_rWidth);