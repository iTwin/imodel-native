/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hps/src/HPSPssFileCreator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPSPssFileCreator.h>
#include <Imagepp/all/h/HPSPssFile.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HPSPssFileCreator::HPSPssFileCreator ()
    {
    m_pWorldCluster = new HGFHMRStdWorldCluster();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HPSPssFileCreator::~HPSPssFileCreator()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HSTATUS HPSPssFileCreator::CreateFileW(WString const& pi_rPssFileName)
    {
    if (m_imageList.empty())
        return H_ERROR;

    HPSPssFile pssFile;
    int32_t id (0);

    // Make mosaic
    HFCPtr<PSSMosaicToken> pMosaicToken(new PSSMosaicToken());

    for (ImageList::iterator itr (m_imageList.begin()); itr != m_imageList.end(); ++itr, ++id)
        {
        uint32_t PageNumber(itr->m_pageNumber);



        // open raster file
        HFCPtr<HRFRasterFile> pRasterFile (GetRasterFile(itr->m_imageName));
        if (pRasterFile == NULL)
            continue;

        // Image Name
        HFCPtr<PSSIMFToken> pImfToken = new PSSIMFToken (itr->m_imageName, PageNumber);


        HFCPtr<HGF2DTransfoModel> pReprojectionModel;

        pReprojectionModel = HCPGeoTiffKeys::GetTransfoModelForReprojection(pRasterFile,
                                                            PageNumber,
                                                            GetDstProjectionCP(),
                                                            (HFCPtr<HGF2DWorldCluster>&)
                                                            m_pWorldCluster,
                                                            itr->m_projection.get());
        // Georeference
        if (pReprojectionModel != 0)
            {
            // Create georeference token
            HFCPtr<HPSPssToken> pGeorefToken;
            if (pReprojectionModel->CanBeRepresentedByAMatrix())
                pGeorefToken = new PSSMatrixToken(pReprojectionModel->GetMatrix());
            else
                pGeorefToken = new PSSLocalProjectiveGridToken(reinterpret_cast<HFCPtr<HGF2DLocalProjectiveGrid>&>(pReprojectionModel));

            // Create Transformed Image token
            HFCPtr<HPSPssToken> pToken (new PSSTFIGeorefToken(pImfToken, pGeorefToken, pRasterFile->GetWorldIdentificator()));

            HFCPtr<PSSImageIdToken> pImageIdToken = new PSSImageIdToken(pToken, id);
            pssFile.AddToken(reinterpret_cast<HFCPtr<HPSPssToken>&> (pImageIdToken));
            pMosaicToken->AddToken(pImageIdToken);
            }
        }

    if (pssFile.CountToken())
        {
        // Select HMR world
        HFCPtr<HPSPssToken> pSelectWorldToken = new PSSSelectWorldToken(HGF2DWorld_HMRWORLD);
        pssFile.AddToken(pSelectWorldToken);

        // Add mosaic
        pssFile.AddToken(reinterpret_cast<HFCPtr<HPSPssToken>&>(pMosaicToken));

        // Add page
        HFCPtr<HPSPssToken> pPageToken(new PSSPageToken(pMosaicToken));
        pssFile.AddToken(pPageToken);
        return pssFile.CreatePSSFile(pi_rPssFileName);
        }

    return H_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void HPSPssFileCreator::AddImage(WString const& pi_rImageName, uint32_t pi_pageNumber, GeoCoordinates::BaseGCSP pi_projection)
    {
    ImageDef imageDef;
    imageDef.m_imageName = pi_rImageName;
    imageDef.m_pageNumber = pi_pageNumber;
    imageDef.m_projection = pi_projection;
    m_imageList.push_back(imageDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void HPSPssFileCreator::SetDstProjection(GeoCoordinates::BaseGCSP pi_projection)
    {
    m_dstProjection = pi_projection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCSCP HPSPssFileCreator::GetDstProjectionCP() const
    {
    return m_dstProjection.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRFRasterFile> HPSPssFileCreator::GetRasterFile(WString const& rImageName) const
    {
    // Create an URL
    HFCPtr<HFCURL> pURL(HFCURL::Instanciate(rImageName));
    if (pURL == 0)
        {
        pURL = HFCURL::Instanciate(WString(L"file://") + rImageName);

        HASSERT(pURL != 0);
        }

    // Open the raster file
    HFCPtr<HRFRasterFile> pFile(HRFRasterFileFactory::GetInstance()->OpenFile(pURL, true));
    if (pFile == 0)
        throw HFCFileNotFoundException(rImageName);
    if (!pFile->IsCompatibleWith(HRFFileId_InternetImaging/*HRFInternetImagingFile::CLASS_ID*/))
        {
        // improve the file
        pFile = GenericImprove(pFile, HRFiTiffCacheFileCreator::GetInstance());
        }

    return (pFile);
    }