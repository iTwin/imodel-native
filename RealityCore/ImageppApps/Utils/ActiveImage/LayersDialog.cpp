/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/LayersDialog.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// LayersDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"

#include "LayersDialog.h"

#include <Imagepp/all/h/HMDLayerInfoPDF.h>
#include <Imagepp/all/h/HMDLayerInfoWMS.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>
#include <Imagepp/all/h/HMDContext.h>

// CLayersDialog dialog

//IMPLEMENT_DYNAMIC(CLayersDialog, CDialog)

CLayersDialog::CLayersDialog(HFCPtr<HRARaster>& pi_prRaster, CActiveImageView* pi_pActiveImageView)
	: CDialog(CLayersDialog::IDD, pi_pActiveImageView)
{         
    m_pRaster          = pi_prRaster;
    m_pActiveImageView = pi_pActiveImageView;
}

CLayersDialog::~CLayersDialog()
{
}

void CLayersDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_LayerList);
}


BEGIN_MESSAGE_MAP(CLayersDialog, CDialog)
    ON_BN_CLICKED(IDAPPLY, &CLayersDialog::OnBnClickedApply)
END_MESSAGE_MAP()


// CLayersDialog message handlers
BOOL CLayersDialog::OnInitDialog() 
{    
    CDialog::OnInitDialog();

    m_LayerList.SetCheckStyle( BS_AUTOCHECKBOX );
    
    HFCPtr<HMDContext>        pContext = m_pRaster->GetContext();
    HFCPtr<HMDVolatileLayers> pVolatileLayers;
    
    if (pContext != 0)
    {    
        HASSERT(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO)
                        ->IsCompatibleWith(HMDVolatileLayers::CLASS_ID));

        pVolatileLayers = static_cast<HMDVolatileLayers*>(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO).GetPtr());
    }
        
    if (pVolatileLayers != 0)
    {    
        const HMDVolatileLayerInfo* pVolatileLayerInfo;
        const HMDLayerInfo*         pLayerInfo;
       
        for (unsigned short LayerInd = 0; LayerInd < pVolatileLayers->GetNbVolatileLayers(); LayerInd++)
        {        
            pVolatileLayerInfo = pVolatileLayers->GetVolatileLayerInfo(LayerInd);
            pLayerInfo = pVolatileLayerInfo->GetLayerInfo();

            if (pLayerInfo->IsCompatibleWith(HMDLayerInfoPDF::CLASS_ID))
            {	
                WString nameW(((const HMDLayerInfoPDF*) pLayerInfo)->GetLayerName().c_str(), BentleyCharEncoding::Utf8);
                m_LayerList.AddString(nameW.c_str());
            }					
            else if (pLayerInfo->IsCompatibleWith(HMDLayerInfoWMS::CLASS_ID))
            {
                WString titleW(((const HMDLayerInfoWMS*) pLayerInfo)->GetLayerTitle().c_str(), BentleyCharEncoding::Utf8);
                m_LayerList.AddString(titleW.c_str());
            }
            
            m_LayerList.SetCheck(LayerInd, 
                                 pVolatileLayerInfo->GetVisibleState() ? BST_CHECKED : BST_UNCHECKED);
        }
    }
  
    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
}
void CLayersDialog::OnBnClickedApply()
{    
    HFCPtr<HMDContext>        pContext = m_pRaster->GetContext();
    HFCPtr<HMDVolatileLayers> pVolatileLayers;

    pVolatileLayers = static_cast<HMDVolatileLayers*>(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO).GetPtr());

    HASSERT(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO)
                    ->IsCompatibleWith(HMDVolatileLayers::CLASS_ID));

    if (pVolatileLayers != 0)
    {  
        HMDVolatileLayerInfo* pVolatileLayerInfo;    
        bool VisibleStateModification = false;

        for (unsigned short LayerInd = 0; LayerInd < pVolatileLayers->GetNbVolatileLayers(); LayerInd++)
        {        
            pVolatileLayerInfo = pVolatileLayers->GetVolatileLayerInfo(LayerInd);

            if ((m_LayerList.GetCheck(LayerInd) == 1) != pVolatileLayerInfo->GetVisibleState())
            {            
                pVolatileLayerInfo->SetVisibleState(m_LayerList.GetCheck(LayerInd) == 1);
                VisibleStateModification = true;
            }                               
        }
        
        if (VisibleStateModification == true)
        {
            m_pRaster->InvalidateRaster();        
            GetParent()->RedrawWindow();        
        }
    }
}