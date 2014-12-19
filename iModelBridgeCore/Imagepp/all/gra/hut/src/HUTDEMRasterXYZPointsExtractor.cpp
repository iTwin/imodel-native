//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTDEMRasterXYZPointsExtractor.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGSBlitter.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSToolbox.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HUTDEMRasterXYZPointsIterator.h>
#include <Imagepp/all/h/HUTDEMRasterXYZPointsExtractor.h>
#include <Imagepp/all/h/HRFException.h>


USING_NAMESPACE_IMAGEPP

HFCPtr<HGFHMRStdWorldCluster>
HUTDEMRasterXYZPointsExtractor::m_spWorldCluster = new HGFHMRStdWorldCluster();

/** ---------------------------------------------------------------------------
    Public
    Default constructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsExtractor::HUTDEMRasterXYZPointsExtractor(const WString&         pi_rDEMRasterFileName,
                                                               const HFCPtr<HPMPool>& pi_rpMemPool)
// Open the file
    :   m_pRasterFile(HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(pi_rDEMRasterFileName),
                                                                    true)),

    //Ouput the horizontal coordinate (i.e. : x-y) of the 3D point to the HMR coordinate system for now.
    m_pXYCoordSys(m_spWorldCluster->GetWorldReference(HGF2DWorld_HMRWORLD).GetPtr())
    {
    HFCPtr<HRPPixelType> pPixelType(m_pRasterFile->
                                    GetPageDescriptor(0)->
                                    GetResolutionDescriptor(0)->
                                    GetPixelType());

    if ((pPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID) == false) &&
        (pPixelType->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == false)  &&
        (pPixelType->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == false))
        {
        throw HRFException(HRF_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION,
                           pi_rDEMRasterFileName);
        }

    LoadRasterFile(m_pRasterFile, pi_rpMemPool, m_pRaster);    

    ((HFCPtr<HRAStoredRaster>&)m_pRaster)->GetRasterSize(&m_WidthInPixels, 
                                                         &m_HeightInPixels);   
}

/** ---------------------------------------------------------------------------
    Public
    Default constructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsExtractor::HUTDEMRasterXYZPointsExtractor (const WString&         pi_rDEMRasterFileName, 
                                                                const HFCPtr<HPMPool>& pi_rpMemPool,
                                                                bool                   pi_legacyPixelTypeSupportOnly)
    :   m_pRasterFile(HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(pi_rDEMRasterFileName), 
                                                                    TRUE)),

        //Ouput the horizontal coordinate (i.e. : x-y) of the 3D point to the HMR coordinate system for now.
        m_pXYCoordSys(m_spWorldCluster->GetWorldReference(HGF2DWorld_HMRWORLD).GetPtr())
    {     
    HFCPtr<HRPPixelType> pPixelType(m_pRasterFile->
                                    GetPageDescriptor(0)->
                                    GetResolutionDescriptor(0)->
                                    GetPixelType()); 

    // NTERAY: This test should normally not be required. We should be the most permissive possible.
    //         Also have a look at HRPDEMFilter::SetFor which has a much more generic way of 
    //         performing this test.
    if (pi_legacyPixelTypeSupportOnly)
        {
        if ((pPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID) == false) &&
            (pPixelType->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == false)  &&
            (pPixelType->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == false))
            {
            throw HRFException(HRF_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION, 
                               pi_rDEMRasterFileName);
            }
        }
    else
        {
        if ((pPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) == false) &&
            (pPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID) == false) &&
            (pPixelType->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == false)  &&
            (pPixelType->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == false))
            {
            throw HRFException(HRF_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION, 
                               pi_rDEMRasterFileName);
            }
        }

    LoadRasterFile(m_pRasterFile, pi_rpMemPool, m_pRaster);

    ((HFCPtr<HRAStoredRaster>&)m_pRaster)->GetRasterSize(&m_WidthInPixels,
                                                         &m_HeightInPixels);
    }

/** ---------------------------------------------------------------------------
    Public
    Destructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsExtractor::~HUTDEMRasterXYZPointsExtractor()
    {
    }

/** ---------------------------------------------------------------------------
    Public
    CreateXYZPointsIterator
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator* HUTDEMRasterXYZPointsExtractor::CreateXYZPointsIterator(const WString& pi_rDestCoordSysKeyName,
        double  pi_ScaleFactor)
    {
    return new HUTDEMRasterXYZPointsIterator(this, pi_rDestCoordSysKeyName, pi_ScaleFactor);
    }

/** ---------------------------------------------------------------------------
    Public
    CreateXYZPointsIteratorWithNoDataValueRemoval
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator* HUTDEMRasterXYZPointsExtractor::CreateXYZPointsIteratorWithNoDataValueRemoval   (const WString& pi_rDestCoordSysKeyName,
        double  pi_ScaleFactor)
    {
    return HUTDEMRasterXYZPointsIterator::CreateFor(*this, pi_rDestCoordSysKeyName, pi_ScaleFactor);
    }

/** ---------------------------------------------------------------------------
    Public
    GetDimensionInPixels
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsExtractor::GetDimensionInPixels(uint64_t* po_pWidthInPixels,
                                                          uint64_t* po_pHeightInPixels) const
    {
    HPRECONDITION((po_pWidthInPixels != 0) && (po_pHeightInPixels != 0));

    *po_pWidthInPixels  = m_WidthInPixels;
    *po_pHeightInPixels = m_HeightInPixels;
    }

/** ---------------------------------------------------------------------------
    Public
    GetNumberOfPoints
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsExtractor::GetNumberOfPoints(uint64_t* po_pNumberPoints) const
    {
    HPRECONDITION(po_pNumberPoints != 0);

    *po_pNumberPoints = m_WidthInPixels * m_HeightInPixels;
    }

/** ---------------------------------------------------------------------------
    Public
    GetNoDataValues
    ---------------------------------------------------------------------------
*/
const double* HUTDEMRasterXYZPointsExtractor::GetNoDataValue() const
    {
    HPRECONDITION(0 != m_pRasterFile->GetPageDescriptor(0));
    const HRFPageDescriptor& pageDescriptor = *m_pRasterFile->GetPageDescriptor(0);

    // Extract no data value
    HASSERT(0 != pageDescriptor.GetResolutionDescriptor(0));
    const HRPPixelType& pixelType(*pageDescriptor.GetResolutionDescriptor(0)->GetPixelType());

    HASSERT((pixelType.IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) == true) ||
            (pixelType.IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID) == true) ||
            (pixelType.IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) == true) ||
            (pixelType.IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID) == true));

    return pixelType.GetChannelOrg().GetChannelPtr(0)->GetNoDataValue();
    }



/** ---------------------------------------------------------------------------
    Public
    Get2DCoordMinMaxValues
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsExtractor::Get2DCoordMinMaxValues(double* po_pXMin, double* po_pXMax,
                                                            double* po_pYMin, double* po_pYMax) const
    {
    HPRECONDITION((po_pXMin != 0) && (po_pXMax != 0) && (po_pYMin != 0) && (po_pYMax != 0));

    HFCPtr<HRFPageDescriptor> pPageDescriptor(m_pRasterFile->GetPageDescriptor(0));

    HFCPtr<HGF2DCoordSys>     pCoordSys(m_spWorldCluster->
                                        GetCoordSysReference(m_pRasterFile->
                                                             GetWorldIdentificator()));

    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    if (pPageDescriptor->HasTransfoModel() != 0)
        {
        pTransfoModel = pPageDescriptor->GetTransfoModel();
        }
    else
        {
        pTransfoModel = new HGF2DIdentity();
        }

    pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*(pCoordSys->
                                                                GetTransfoModelTo(m_pXYCoordSys)));

    *po_pXMin = DBL_MAX;
    *po_pXMax = (-DBL_MAX);
    *po_pYMin = DBL_MAX;
    *po_pYMax = (-DBL_MAX);

    double LogicCoordinateX;
    double LogicCoordinateY;

    for (uint64_t PhysicalCoordinateX = 0; PhysicalCoordinateX < m_WidthInPixels;)
        {
        for (uint64_t PhysicalCoordinateY = 0; PhysicalCoordinateY < m_HeightInPixels;)
            {
            pTransfoModel->ConvertDirect((double)PhysicalCoordinateX,
                                         (double)PhysicalCoordinateY,
                                         &LogicCoordinateX,
                                         &LogicCoordinateY);

            *po_pXMin = min(*po_pXMin, LogicCoordinateX);
            *po_pXMax = max(*po_pXMax, LogicCoordinateX);
            *po_pYMin = min(*po_pYMin, LogicCoordinateY);
            *po_pYMax = max(*po_pYMax, LogicCoordinateY);

            PhysicalCoordinateY += m_HeightInPixels - 1;
            }

        PhysicalCoordinateX += m_WidthInPixels - 1;
        }
    }

/** ---------------------------------------------------------------------------
    Public
    Get2DCoordMinMaxValues
    ---------------------------------------------------------------------------
*/
bool HUTDEMRasterXYZPointsExtractor::GetZCoordMinMaxValues(double* po_pZMin, double* po_pZMax) const
    {
    bool   IsMinMaxFoundInFile = false;

    HRFAttributeMinSampleValue const* pMinSampleValueTag = m_pRasterFile->GetPageDescriptor(0)->FindTagCP<HRFAttributeMinSampleValue>();

    if (pMinSampleValueTag != 0)
        {
        double                               FactorToMeterForZ = GetFactorToMeterForZ();

        HRFAttributeMaxSampleValue const* pMaxSampleValueTag = m_pRasterFile->GetPageDescriptor(0)->FindTagCP<HRFAttributeMaxSampleValue>();

        //If the minimum value is available the maximum value should also be available.
        HASSERT(pMaxSampleValueTag != 0);

        *po_pZMin = pMinSampleValueTag->GetData()[0] * FactorToMeterForZ;
        *po_pZMax = pMaxSampleValueTag->GetData()[0] * FactorToMeterForZ;

        IsMinMaxFoundInFile = true;
        }

    return IsMinMaxFoundInFile;
    }

/** ---------------------------------------------------------------------------
    Public
    GetDEMRasterCoordSys
    ---------------------------------------------------------------------------
*/
IRasterBaseGcsPtr HUTDEMRasterXYZPointsExtractor::GetDEMRasterCoordSys() const
    {
    return m_pRasterFile->GetPageDescriptor(0)->GetGeocoding();
    }

#if (0)

/** ---------------------------------------------------------------------------
    Public    
    GetGeoTiffKeys
    ---------------------------------------------------------------------------
*/
const HCPGeoTiffKeys* HUTDEMRasterXYZPointsExtractor::GetGeoTiffKeys () const
{
    return static_cast<HCPGeoTiffKeys*>(m_pRasterFile->
                                               GetPageDescriptor(0)->
                                               GetMetaDataContainer(HMDMetaDataContainer::HMD_GEOCODING_INFO).GetPtr());
}

#endif
/** ---------------------------------------------------------------------------
    Public    
    GetXYCoordSysPtr
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HUTDEMRasterXYZPointsExtractor::GetXYCoordSysPtr () const
    {
    return m_pXYCoordSys;
    }



/** ---------------------------------------------------------------------------
    Public    
    GetPhysicalLowerLeftPixelCoordSysPtr
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HUTDEMRasterXYZPointsExtractor::GetPhysicalLowerLeftPixelCoordSysPtr () const
    {
    const HRAStoredRaster& storedRaster = static_cast<const HRAStoredRaster&>(*m_pRaster);

    const HFCPtr<HRFResolutionDescriptor>& pResDesc(m_pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0));


    const HRFScanlineOrientation physRasterSLO = pResDesc->GetScanlineOrientation();

    HGF2DStretch physPixelToLowerLeftPixel;
    if(physRasterSLO.IsUpper())
        {
        physPixelToLowerLeftPixel.AddTranslation(HGF2DDisplacement(0, (static_cast<double>(pResDesc->GetHeight()) * -1)));
        physPixelToLowerLeftPixel.AddAnisotropicScaling(1.0, -1.0);
        }
    else if(physRasterSLO.IsRight())
        {
        physPixelToLowerLeftPixel.AddTranslation(HGF2DDisplacement((static_cast<double>(pResDesc->GetWidth()) * -1), 0));
        physPixelToLowerLeftPixel.AddAnisotropicScaling(-1.0, 1.0);
        }

    return new HGF2DCoordSys(physPixelToLowerLeftPixel, storedRaster.GetPhysicalCoordSys());
    }

/** ---------------------------------------------------------------------------
    Public    
    GetDEMRasterCoordSys
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HUTDEMRasterXYZPointsExtractor::GetAppliedSourceToTargetXYTransform () const
{
    return GetPhysicalLowerLeftPixelCoordSysPtr()->GetTransfoModelTo(GetXYCoordSysPtr());
}

/** ---------------------------------------------------------------------------
    Private
    GetRasterFile
    ---------------------------------------------------------------------------
*/
const HRFRasterFile& HUTDEMRasterXYZPointsExtractor::GetRasterFile () const
    {
    HPRECONDITION(0 != m_pRasterFile);
    return *m_pRasterFile;
    }


/** ---------------------------------------------------------------------------
    Private
    GetRaster
    ---------------------------------------------------------------------------
*/
const HRARaster& HUTDEMRasterXYZPointsExtractor::GetRaster () const
    {
    HPRECONDITION(0 != m_pRaster);
    return *m_pRaster;
    }


/** ---------------------------------------------------------------------------
    Private
    LoadRasterFile
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsExtractor::LoadRasterFile(HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                                    const HFCPtr<HPMPool>& pi_rpMemPool,
                                                    HFCPtr<HRARaster>&     po_rpRaster) const
    {
    HPRECONDITION(pi_rpRasterFile != 0);
    HPRECONDITION(pi_rpRasterFile->CountPages() == 1);
    HPRECONDITION(po_rpRaster == 0);

    HFCPtr<HGF2DCoordSys> pLogical;

    HFCPtr<HRARaster> pRaster;

    pLogical = m_spWorldCluster->GetCoordSysReference(pi_rpRasterFile->GetPageWorldIdentificator(0));
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(pi_rpMemPool, pi_rpRasterFile, 0, pLogical);

    // Get the raster from the store
    pRaster = pStore->LoadRaster();
    HASSERT(pRaster != NULL);

    po_rpRaster = pRaster;
    }

/** ---------------------------------------------------------------------------
    Private
    GetFactorToMeterForZ
    ---------------------------------------------------------------------------
*/
double HUTDEMRasterXYZPointsExtractor::GetFactorToMeterForZ() const
    {
    double FactorToMeterForZ = 1.0;

    IRasterBaseGcsPtr baseGCS = m_pRasterFile->GetPageDescriptor(0)->GetGeocoding();

    if (baseGCS != 0) // Validity is not required
        FactorToMeterForZ = baseGCS->GetVerticalUnits();

    return FactorToMeterForZ;
    }

/** ---------------------------------------------------------------------------
    Private
    GetCoordSystP
    ---------------------------------------------------------------------------
*/
const HFCPtr<HGF2DCoordSys>& HUTDEMRasterXYZPointsExtractor::GetXYCoordSystP() const
    {
    return m_pXYCoordSys;
    }