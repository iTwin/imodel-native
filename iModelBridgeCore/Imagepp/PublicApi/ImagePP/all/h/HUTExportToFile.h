//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTExportToFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Definition of the class HUTExportToFile
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HVEShape.h"
#include "HGF2DWorldCluster.h"
#include "HRFPageDescriptor.h"
#include "HRFTypes.h"
#include "HFCURL.h"
#include "HGSTypes.h"
#include "HPMPool.h"

#include "HRASamplingOptions.h"
#include "HRACopyFromOptions.h"
#include "HRABitmap.h"


BEGIN_IMAGEPP_NAMESPACE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Forward declarations.
class HGF2DCoordSys;
class HPMPool;
class HRSObjectStore;
class HRARaster;
class HRAStoredRaster;
class HRFRasterFile;
class HRPFilter;
class HFCProgressEvaluator;

class HUTExportToFile
    {
public:
    // Constructor and destructor.
    HUTExportToFile(const HFCPtr<HFCURL>&            pi_rpSourceURL,
                    HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                    const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                    double                          pi_ScaleX,
                    double                          pi_ScaleY,
                    bool                            pi_Resample = true,
                    const HFCPtr<HRPFilter>&         pi_rpResamplingFilter = 0);

    HUTExportToFile(const HFCPtr<HRFRasterFile>&     pi_rpRasterFile,
                    HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                    const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                    double                          pi_ScaleX,
                    double                          pi_ScaleY,
                    bool                            pi_Resample = true,
                    const HFCPtr<HRPFilter>&         pi_rpResamplingFilter = 0);

    HUTExportToFile(HFCPtr<HRARaster>&               pi_rpSourceRaster,
                    HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                    const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                    double                          pi_ScaleX,
                    double                          pi_ScaleY,
                    bool                            pi_Resample = true,
                    const HFCPtr<HRPFilter>&         pi_rpResamplingFilter = 0);

    ~HUTExportToFile();

    // Methods used to customize the export.
    void        SetClipShape(const HVEShape& pi_rClipShape);
    void        SetBlendAlpha(bool pi_BlendAlpha);
    void        SetResamplingMode(const HGSResampling& pi_rResampling);
    void        SetExportSizeEstimation(bool pi_IsEnable);

    // Set number of color, if dest to palette
    //  0 --> MaxEntries
    void        SetNumberOfColorDestination(uint32_t pi_NbColors);

    void        SetUseDestinationPaletteIfIndexed(bool pi_UseDestinationPalette);

    HRASamplingOptions  GetRepresentativePaletteSamplingOptions() const;
    void                SetRepresentativePaletteSamplingOptions(const HRASamplingOptions& pi_rRepPalSamplingOptions);

    // Export method.
    void        Export();

private:

    class ExportSizeEstimator : public HFCShareableObject<ExportSizeEstimator>
        {
    public :
        ExportSizeEstimator(HFCPtr<HRFRasterFile>& pi_prDestinationFile,
                            HFCPtr<HRFRasterFile>& pi_prOriginalFile,
                            clock_t                   pi_EstimateInterval);
        virtual ~ExportSizeEstimator();

        void Estimate(HFCProgressEvaluator* pi_pProgressEvaluator);

    private :

        uint64_t EstimateCompressedFileSize();

        HFCPtr<HRFRasterFile>  m_pOriginalRasterFile;
        clock_t                m_LastCheckTime;
        clock_t                m_EstimateInterval;
        vector<uint32_t>         m_NbPixelsPerBlock;
        uint64_t               m_TotalNbOfPixelsAllRes;
        int32_t                m_NbRes;
        };


    // Members.
    bool                      m_AlreadyExported;
    bool                      m_BlendAlpha;
    HGSResampling             m_Resampling;
    HVEShape                  m_ClipShape;
    HVEShape                  m_LogicalShape;
    bool                      m_HasClipShape;
    bool                      m_SrcNeedCache;
    uint32_t                 m_NbColorsIfIndexed;
    bool                      m_UseDestinationPaletteIfIndexed;
    bool                      m_Resample;
    HFCPtr<HRPFilter>         m_pResamplingFilter;
    HRFBlockAccess            m_SrcBlockAccess;
    HFCPtr<HGF2DWorldCluster> m_pClusterWorld;      // WorldCluster
    HFCPtr<HRFRasterFile>     m_pDestinationFile;
    HFCPtr<HRAStoredRaster>   m_pDestinationRaster;
    HFCPtr<HRSObjectStore>    m_pDestinationStore;
    HFCPtr<HRFRasterFile>     m_pSourceFile;
    HFCPtr<HRARaster>         m_pSourceRaster;
    HFCPtr<HRSObjectStore>    m_pSourceStore;
    HPMPool                   m_SourcePool;
    HRASamplingOptions        m_RepPalSamplingOptions;
    bool                      m_HasRepPalSamplingOptions;
    bool                      m_EstimateExportSize;

    // Scaling members
    double                    m_ScaleFactorX;
    double                    m_ScaleFactorY;

    // Method used to compute the transformation model.
    HFCPtr<HGF2DTransfoModel> ComputeTransformation(HFCPtr<HRFPageDescriptor>       pi_pDstPageDesc,
                                                    HFCPtr<HRFResolutionDescriptor> pi_pDstResDesc);

    // Member initialisation method.
    void                      PrepareMembers();

    // Method used to clean up the object.
    void                      CleanUp();

    // Method that open and close the source file.
    void                      OpenSourceFile(const HFCPtr<HFCURL>& pi_rpSourceURL);
    void                      InitSourceFile();
    void                      CloseSourceFile();

    // Method that estimate the final size of a compress file.
    uint64_t                    EstimateCompressedFileSize(HFCPtr<HRFRasterFile>& pi_prRasterFile,
                                                            vector<uint32_t>&        pi_rBlockSizePerResInPixel,
                                                            uint64_t               pi_TotalNbOfPixelsAllRes);

    // Disable the copy constructor and assigment operator.
    HUTExportToFile(const HUTExportToFile& pi_rObj);
    HUTExportToFile&          operator=(const HUTExportToFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
