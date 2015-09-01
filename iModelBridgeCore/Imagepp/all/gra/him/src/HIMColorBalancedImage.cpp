//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMColorBalancedImage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first

#include <Imagepp/all/h/HIMColorBalancedImage.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HIMColorBalancedImageIterator.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeGray.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HRABlitter.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRPHistogram.h>

HPM_REGISTER_CLASS(HIMColorBalancedImage, HRAImageView)


/** ---------------------------------------------------------------------------
    Default constructor. For persistence only
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImage::HIMColorBalancedImage()
    :   HRAImageView(HFCPtr<HRARaster>())
    {
    }


/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImage::HIMColorBalancedImage(const HFCPtr<HRARaster>& pi_pSource,
                                             bool                    pi_ApplyGlobalBalance,
                                             bool                    pi_ApplyPositionalBalance)
    :   HRAImageView(pi_pSource)
    {
    if (pi_pSource->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        m_ColorMode = COLORMODE_RGB;
        }
    else if (pi_pSource->GetPixelType()->IsCompatibleWith(HRPPixelTypeGray::CLASS_ID))
        {
        // V8Gray8 or V8GrayWhite8

        m_ColorMode = COLORMODE_GRAY;
        }
    else
        {
        // Pixeltype not supported
        HASSERT(false);
        }

    InitObject();

    m_ApplyGlobal     = pi_ApplyGlobalBalance;
    m_ApplyPositional = pi_ApplyPositionalBalance;

    m_SamplingQuality = SAMPLING_NORMAL;
    }


/** ---------------------------------------------------------------------------
    Copy constructor
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImage::HIMColorBalancedImage(const HIMColorBalancedImage& pi_rImage)
    :   HRAImageView(pi_rImage)
    {
    // Not implemented
    HASSERT(false);
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImage::~HIMColorBalancedImage()
    {
    ClearNeighbors();
    }


/** ---------------------------------------------------------------------------
    Return a copy of self.
    ---------------------------------------------------------------------------
*/
HRARaster* HIMColorBalancedImage::Clone (HPMObjectStore* pi_pStore,
                                         HPMPool*        pi_pLog) const
    {
    return new HIMColorBalancedImage(*this);
    }
/** ---------------------------------------------------------------------------
    Return a copy of self.
    ---------------------------------------------------------------------------
*/
HPMPersistentObject* HIMColorBalancedImage::Clone () const
    {
    return new HIMColorBalancedImage(*this);
    }


/** ---------------------------------------------------------------------------
    Create a new iterator.
    ---------------------------------------------------------------------------
*/
HRARasterIterator* HIMColorBalancedImage::CreateIterator (const HRAIteratorOptions& pi_rOptions) const
    {
    if (m_ApplyGlobal || m_ApplyPositional)
        {
        HRARasterIterator*  pIterator;

        // Return an iterator on the reference
        ((HIMColorBalancedImage*)this)->IncrementRef(); // Wrap temporary HFCPtr creation with an artificial addref
        pIterator = new HIMColorBalancedImageIterator(
            HFCPtr<HIMColorBalancedImage>((HIMColorBalancedImage*)this), pi_rOptions);
        ((HIMColorBalancedImage*)this)->DecrementRef();

        return pIterator;
        }
    else
        return GetSource()->CreateIterator(pi_rOptions);
    }


/** ---------------------------------------------------------------------------
    Initialization
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::InitObject()
    {
    m_pLeftDeltas = 0;
    m_pRightDeltas = 0;
    m_pTopDeltas = 0;
    m_pBottomDeltas = 0;

    m_HasLeftNeighbor = false;
    m_HasRightNeighbor = false;
    m_HasTopNeighbor = false;
    m_HasBottomNeighbor = false;
    }


/** ---------------------------------------------------------------------------
    Set the left neighbor
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetLeftNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor)
    {
    m_pLeftNeighbor = pi_rpNeighbor;

    m_HasLeftNeighbor = true;

    ComputeLeftHistograms();
    }


/** ---------------------------------------------------------------------------
    Compute histograms for the left side
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeLeftHistograms()
    {
    // Compute overlap
    HFCPtr<HVEShape> pOverlap(new HVEShape(*GetEffectiveShape()));
    pOverlap->Intersect(*m_pLeftNeighbor->GetEffectiveShape());

    // Compute both histograms and dispersions

    m_pLeftHistogram = GetHistogramInfoFor(this, pOverlap);
    ComputeDispersion(m_pLeftHistogram, m_Left);

    m_pLeftNeighborHistogram = GetHistogramInfoFor(m_pLeftNeighbor, pOverlap);
    ComputeDispersion(m_pLeftNeighborHistogram, m_LeftNeighbor);
    }


/** ---------------------------------------------------------------------------
    Set the right neighbor
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetRightNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor)
    {
    m_pRightNeighbor = pi_rpNeighbor;

    m_HasRightNeighbor = true;

    ComputeRightHistograms();
    }


/** ---------------------------------------------------------------------------
    Compute histograms for the right side
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeRightHistograms()
    {
    // Compute overlap

    HFCPtr<HVEShape> pOverlap(new HVEShape(*GetEffectiveShape()));
    pOverlap->Intersect(*m_pRightNeighbor->GetEffectiveShape());

    // Compute both histograms and dispersions

    m_pRightHistogram = GetHistogramInfoFor(this, pOverlap);
    ComputeDispersion(m_pRightHistogram, m_Right);

    m_pRightNeighborHistogram = GetHistogramInfoFor(m_pRightNeighbor, pOverlap);
    ComputeDispersion(m_pRightNeighborHistogram, m_RightNeighbor);
    }


/** ---------------------------------------------------------------------------
    Set the top neighbor
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetTopNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor)
    {
    m_pTopNeighbor = pi_rpNeighbor;

    m_HasTopNeighbor = true;

    ComputeTopHistograms();
    }


/** ---------------------------------------------------------------------------
    Compute histograms on the top side
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeTopHistograms()
    {
    // Compute overlap

    HFCPtr<HVEShape> pOverlap(new HVEShape(*GetEffectiveShape()));
    pOverlap->Intersect(*m_pTopNeighbor->GetEffectiveShape());

    // Compute both histograms and dispersions

    m_pTopHistogram = GetHistogramInfoFor(this, pOverlap);
    ComputeDispersion(m_pTopHistogram, m_Top);

    m_pTopNeighborHistogram = GetHistogramInfoFor(m_pTopNeighbor, pOverlap);
    ComputeDispersion(m_pTopNeighborHistogram, m_TopNeighbor);
    }


/** ---------------------------------------------------------------------------
    Set the bottom neighbor
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetBottomNeighbor(const HFCPtr<HIMColorBalancedImage>& pi_rpNeighbor)
    {
    m_pBottomNeighbor = pi_rpNeighbor;

    m_HasBottomNeighbor = true;

    ComputeBottomHistograms();
    }


/** ---------------------------------------------------------------------------
    Compute histograms on the bottom side
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeBottomHistograms()
    {
    // Compute overlap

    HFCPtr<HVEShape> pOverlap(new HVEShape(*GetEffectiveShape()));
    pOverlap->Intersect(*m_pBottomNeighbor->GetEffectiveShape());

    // Compute both histograms and dispersions

    m_pBottomHistogram = GetHistogramInfoFor(this, pOverlap);
    ComputeDispersion(m_pBottomHistogram, m_Bottom);

    m_pBottomNeighborHistogram = GetHistogramInfoFor(m_pBottomNeighbor, pOverlap);
    ComputeDispersion(m_pBottomNeighborHistogram, m_BottomNeighbor);
    }


//-----------------------------------------------------------------------------
// ClearNeighbors
//-----------------------------------------------------------------------------
void HIMColorBalancedImage::ClearNeighbors()
    {
    delete m_pLeftDeltas;
    m_pLeftDeltas = 0;
    delete m_pRightDeltas;
    m_pRightDeltas = 0;
    delete m_pTopDeltas;
    m_pTopDeltas = 0;
    delete m_pBottomDeltas;
    m_pBottomDeltas = 0;

    m_pLeftHistogram = 0;
    m_pLeftNeighborHistogram = 0;
    m_pRightHistogram = 0;
    m_pRightNeighborHistogram = 0;
    m_pTopHistogram = 0;
    m_pTopNeighborHistogram = 0;
    m_pBottomHistogram = 0;
    m_pBottomNeighborHistogram = 0;

    m_pLeftNeighbor = 0;
    m_pRightNeighbor = 0;
    m_pTopNeighbor = 0;
    m_pBottomNeighbor = 0;

    m_HasLeftNeighbor = false;
    m_HasRightNeighbor = false;
    m_HasTopNeighbor = false;
    m_HasBottomNeighbor = false;

    m_pApplicationShape = 0;

    m_MyGlobal = RGBDispersion();
    }


/** ---------------------------------------------------------------------------
    Set the region inside which the positional balance will be graduated.
    Outside this region, the positional balance will be constant, applied at
    100% depending on the side.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetApplicationShape(const HFCPtr<HVEShape>& pi_rpShape)
    {
    m_pApplicationShape = new HVEShape(*pi_rpShape);
    }


/** ---------------------------------------------------------------------------
    Retrieve the graduated region.
    ---------------------------------------------------------------------------
*/
const HFCPtr<HVEShape>& HIMColorBalancedImage::GetApplicationShape() const
    {
    HASSERT(m_pApplicationShape != 0);

    return m_pApplicationShape;
    }


/** ---------------------------------------------------------------------------
    Compute dispersion information for an histogram.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeDispersion(HFCPtr<HRPHistogram>& pi_rpHistogram,
                                              RGBDispersion&        pi_rDispersion)
    {
    uint32_t TotalPixels = 0;
    uint32_t CurrentEntryCount;
    double ITimesCurrentEntryCount;

    pi_rDispersion.MeanRed = 0;
    pi_rDispersion.StddevRed = 0;
    double StddevInternalRed = 0;

    HASSERT(pi_rpHistogram->GetEntryFrequenciesSize() >= 256);

    int i;
    for (i = 0 ; i < 256 ; i++)
        {
        CurrentEntryCount = pi_rpHistogram->GetEntryCount(i);
        TotalPixels += CurrentEntryCount;

        ITimesCurrentEntryCount = i * CurrentEntryCount;
        pi_rDispersion.MeanRed += ITimesCurrentEntryCount;
        StddevInternalRed += i * ITimesCurrentEntryCount;
        }

    HASSERT(TotalPixels != 0);
    double TotalPixelsInverse = 1.0 / TotalPixels;

    pi_rDispersion.MeanRed *= TotalPixelsInverse;
    pi_rDispersion.StddevRed = sqrt((StddevInternalRed * TotalPixelsInverse - pi_rDispersion.MeanRed * pi_rDispersion.MeanRed) * TotalPixels / (TotalPixels - 1));
    if (pi_rDispersion.StddevRed <= HGLOBAL_EPSILON)
        pi_rDispersion.StddevRed = 1.0;

    if (pi_rpHistogram->GetEntryFrequenciesSize() >= 768)
        {
        pi_rDispersion.MeanGreen = 0;
        pi_rDispersion.StddevGreen = 0;
        double StddevInternalGreen = 0;
        for (i = 0 ; i < 256 ; i++)
            {
            CurrentEntryCount = pi_rpHistogram->GetEntryCount(i+256);
            ITimesCurrentEntryCount = i * CurrentEntryCount;
            pi_rDispersion.MeanGreen += ITimesCurrentEntryCount;
            StddevInternalGreen += i * ITimesCurrentEntryCount;
            }
        pi_rDispersion.MeanGreen *= TotalPixelsInverse;
        pi_rDispersion.StddevGreen = sqrt((StddevInternalGreen * TotalPixelsInverse - pi_rDispersion.MeanGreen * pi_rDispersion.MeanGreen) * TotalPixels / (TotalPixels - 1));
        if (pi_rDispersion.StddevGreen <= HGLOBAL_EPSILON)
            pi_rDispersion.StddevGreen = 1.0;

        pi_rDispersion.MeanBlue = 0;
        pi_rDispersion.StddevBlue = 0;
        double StddevInternalBlue = 0;
        for (i = 0 ; i < 256 ; i++)
            {
            CurrentEntryCount = pi_rpHistogram->GetEntryCount(i+512);
            ITimesCurrentEntryCount = i * CurrentEntryCount;
            pi_rDispersion.MeanBlue += ITimesCurrentEntryCount;
            StddevInternalBlue += i * ITimesCurrentEntryCount;
            }
        pi_rDispersion.MeanBlue *= TotalPixelsInverse;
        pi_rDispersion.StddevBlue = sqrt((StddevInternalBlue * TotalPixelsInverse - pi_rDispersion.MeanBlue * pi_rDispersion.MeanBlue) * TotalPixels / (TotalPixels - 1));
        if (pi_rDispersion.StddevBlue <= HGLOBAL_EPSILON)
            pi_rDispersion.StddevBlue = 1.0;
        }
    else
        {
        // COLORMODE_GRAY

        pi_rDispersion.MeanGreen = pi_rDispersion.MeanRed;
        pi_rDispersion.StddevGreen = pi_rDispersion.StddevRed;

        pi_rDispersion.MeanBlue = pi_rDispersion.MeanRed;
        pi_rDispersion.StddevBlue = pi_rDispersion.StddevRed;
        }
    }


/** ---------------------------------------------------------------------------
    Receive the global dispersion information. All neighbors must be set
    before calling this method. The global map, prime dispersions and
    deltas are all computed here.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::SetGlobalDispersion(const RGBDispersion& pi_Dispersion)
    {
    //
    // Compute values for global balancing
    //
    m_AllGlobal = pi_Dispersion;

    size_t NumberOfNeighbors = GetNumberOfNeighbors();

    if (NumberOfNeighbors > 0)
        {
        ComputeGlobalMap();

        //
        // Compute prime parameters and deltas
        //

        if (m_HasLeftNeighbor)
            {
            // Compute two primes (us plus neighbor)
            ComputePrimeBasedOnHistogram(m_pLeftHistogram, GetLocalDispersionForGlobalAlgorithm(), m_LeftPrime);
            ComputePrimeBasedOnHistogram(m_pLeftNeighborHistogram, m_pLeftNeighbor->GetLocalDispersionForGlobalAlgorithm(), m_LeftNeighborPrime);

            // Precompute the deltas for this side
            if (m_ApplyGlobal)
                m_pLeftDeltas = ComputeDeltas(m_LeftPrime, m_LeftNeighborPrime);
            else
                m_pLeftDeltas = ComputeDeltas(m_Left, m_LeftNeighbor);
            }

        if (m_HasRightNeighbor)
            {
            // Compute two primes (us plus neighbor)
            ComputePrimeBasedOnHistogram(m_pRightHistogram, GetLocalDispersionForGlobalAlgorithm(), m_RightPrime);
            ComputePrimeBasedOnHistogram(m_pRightNeighborHistogram, m_pRightNeighbor->GetLocalDispersionForGlobalAlgorithm(), m_RightNeighborPrime);

            // Precompute the deltas for this side
            if (m_ApplyGlobal)
                m_pRightDeltas = ComputeDeltas(m_RightPrime, m_RightNeighborPrime);
            else
                m_pRightDeltas = ComputeDeltas(m_Right, m_RightNeighbor);
            }

        if (m_HasTopNeighbor)
            {
            // Compute two primes (us plus neighbor)
            ComputePrimeBasedOnHistogram(m_pTopHistogram, GetLocalDispersionForGlobalAlgorithm(), m_TopPrime);
            ComputePrimeBasedOnHistogram(m_pTopNeighborHistogram, m_pTopNeighbor->GetLocalDispersionForGlobalAlgorithm(), m_TopNeighborPrime);

            // Precompute the deltas for this side
            if (m_ApplyGlobal)
                m_pTopDeltas = ComputeDeltas(m_TopPrime, m_TopNeighborPrime);
            else
                m_pTopDeltas = ComputeDeltas(m_Top, m_TopNeighbor);
            }

        if (m_HasBottomNeighbor)
            {
            // Compute two primes (us plus neighbor)
            ComputePrimeBasedOnHistogram(m_pBottomHistogram, GetLocalDispersionForGlobalAlgorithm(), m_BottomPrime);
            ComputePrimeBasedOnHistogram(m_pBottomNeighborHistogram, m_pBottomNeighbor->GetLocalDispersionForGlobalAlgorithm(), m_BottomNeighborPrime);

            // Precompute the deltas for this side
            if (m_ApplyGlobal)
                m_pBottomDeltas = ComputeDeltas(m_BottomPrime, m_BottomNeighborPrime);
            else
                m_pBottomDeltas = ComputeDeltas(m_Bottom, m_BottomNeighbor);
            }
        }
    }


/** ---------------------------------------------------------------------------
    Compute the prime dispersion starting from the base histogram.
    @note The global dispersions must be computed prior to calling this method.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputePrimeBasedOnHistogram(HFCPtr<HRPHistogram>& pi_rpHistogram,
                                                         const RGBDispersion&  pi_rLocalDispersionForGlobal,
                                                         RGBDispersion&        po_rDispersionPrime)
    {
    HPRECONDITION(!m_AllGlobal.IsEmpty());
    HPRECONDITION(!pi_rLocalDispersionForGlobal.IsEmpty());

    HFCPtr<HRPHistogram> pHistogramPrime(new HRPHistogram(pi_rpHistogram->GetEntryFrequenciesSize()));
    HASSERT(pHistogramPrime->GetEntryFrequenciesSize() >= 256);

    // Red or gray channel
    double PrecomputedFactorRed = m_AllGlobal.StddevRed / pi_rLocalDispersionForGlobal.StddevRed;

    // Transform each 256 possible value for each channel, and make a new histogram
    // using the entry counts of the original one
    uint32_t NewValue;
    int i;
    for (i = 0 ; i < 256 ; i++)
        {
        NewValue = (uint32_t) MIN( MAX(((((double)i) - pi_rLocalDispersionForGlobal.MeanRed) * PrecomputedFactorRed + m_AllGlobal.MeanRed),
                                     0.0),
                                 255.0);
        pHistogramPrime->IncrementEntryCount(NewValue, pi_rpHistogram->GetEntryCount(i));
        }

    if (pHistogramPrime->GetEntryFrequenciesSize() >= 768)
        {
        // Compute the two other channels

        double PrecomputedFactorGreen = m_AllGlobal.StddevGreen / pi_rLocalDispersionForGlobal.StddevGreen;
        double PrecomputedFactorBlue  = m_AllGlobal.StddevBlue / pi_rLocalDispersionForGlobal.StddevBlue;

        for (i = 0 ; i < 256 ; i++)
            {
            NewValue = (uint32_t) MIN( MAX(((((double)i) - pi_rLocalDispersionForGlobal.MeanGreen) * PrecomputedFactorGreen + m_AllGlobal.MeanGreen),
                                         0.0),
                                     255.0);
            pHistogramPrime->IncrementEntryCount(256+NewValue, pi_rpHistogram->GetEntryCount(256+i));
            }
        for (i = 0 ; i < 256 ; i++)
            {
            NewValue = (uint32_t) MIN( MAX(((((double)i) - pi_rLocalDispersionForGlobal.MeanBlue) * PrecomputedFactorBlue + m_AllGlobal.MeanBlue),
                                         0.0),
                                     255.0);
            pHistogramPrime->IncrementEntryCount(512+NewValue, pi_rpHistogram->GetEntryCount(512+i));
            }
        }

    ComputeDispersion(pHistogramPrime, po_rDispersionPrime);
    }


/** ---------------------------------------------------------------------------
    Precompute the deltas for each side
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImage::Deltas* HIMColorBalancedImage::ComputeDeltas(const RGBDispersion& pi_rDispersion,
                                                                    const RGBDispersion& pi_rNeighborDispersion)
    {
    Deltas* pDeltas = 0;

    if (m_ColorMode == COLORMODE_RGB)
        {
        pDeltas = new RGBDeltas;

        double PrecomputedFactorRed = pi_rNeighborDispersion.StddevRed / pi_rDispersion.StddevRed;
        double PrecomputedFactorGreen = pi_rNeighborDispersion.StddevGreen / pi_rDispersion.StddevGreen;
        double PrecomputedFactorBlue = pi_rNeighborDispersion.StddevBlue / pi_rDispersion.StddevBlue;

        for (int i = 0 ; i < 256 ; ++i)
            {
            ((RGBDeltas*)pDeltas)->Red[i]   = ((((double)i) - pi_rDispersion.MeanRed) *
                                               PrecomputedFactorRed + pi_rNeighborDispersion.MeanRed - ((double)i)) * 0.5;
            ((RGBDeltas*)pDeltas)->Green[i] = ((((double)i) - pi_rDispersion.MeanGreen) *
                                               PrecomputedFactorGreen + pi_rNeighborDispersion.MeanGreen - ((double)i)) * 0.5;
            ((RGBDeltas*)pDeltas)->Blue[i]  = ((((double)i) - pi_rDispersion.MeanBlue) *
                                               PrecomputedFactorBlue + pi_rNeighborDispersion.MeanBlue - ((double)i)) * 0.5;
            }
        }
    else
        {
        // COLORMODE_GRAY

        pDeltas = new Deltas;

        // Can use any color, they are equal
        double PrecomputedFactorGray = pi_rNeighborDispersion.StddevRed / pi_rDispersion.StddevRed;

        for (int i = 0 ; i < 256 ; ++i)
            {
            pDeltas->Gray[i] = ((((double)i) - pi_rDispersion.MeanRed) *
                                PrecomputedFactorGray + pi_rNeighborDispersion.MeanRed - ((double)i)) * 0.5;
            }
        }

    return pDeltas;
    }


/** ---------------------------------------------------------------------------
    Precompute the global balance map.
    @note The global dispersions must be computed prior to calling this method.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ComputeGlobalMap()
    {
    HPRECONDITION(!m_AllGlobal.IsEmpty());

    const RGBDispersion& MyGlobal = GetLocalDispersionForGlobalAlgorithm();

    HPRECONDITION(!MyGlobal.IsEmpty());

    if (m_ColorMode == COLORMODE_RGB)
        {
        double PrecomputedFactorRed = m_AllGlobal.StddevRed / MyGlobal.StddevRed;
        double PrecomputedFactorGreen = m_AllGlobal.StddevGreen / MyGlobal.StddevGreen;
        double PrecomputedFactorBlue = m_AllGlobal.StddevBlue / MyGlobal.StddevBlue;

        for (int i = 0 ; i < 256 ; ++i)
            {
            m_GlobalMap.Red[i]   = (Byte) MIN( MAX(((((double)i) - MyGlobal.MeanRed) * PrecomputedFactorRed + m_AllGlobal.MeanRed),
                                                     0.0),
                                                 255.0);
            m_GlobalMap.Green[i] = (Byte) MIN( MAX(((((double)i) - MyGlobal.MeanGreen) * PrecomputedFactorGreen + m_AllGlobal.MeanGreen),
                                                     0.0),
                                                 255.0);
            m_GlobalMap.Blue[i]  = (Byte) MIN( MAX(((((double)i) - MyGlobal.MeanBlue) * PrecomputedFactorBlue + m_AllGlobal.MeanBlue),
                                                     0.0),
                                                 255.0);
            }
        }
    else
        {
        // COLORMODE_GRAY

        // Can use any color, they are equal
        double PrecomputedFactorGray = m_AllGlobal.StddevRed / MyGlobal.StddevRed;

        for (int i = 0 ; i < 256 ; ++i)
            {
            m_GlobalMap.Gray[i]   = (Byte) MIN( MAX(((((double)i) - MyGlobal.MeanRed) * PrecomputedFactorGray + m_AllGlobal.MeanRed),
                                                      0.0),
                                                  255.0);
            }
        }
    }


/** ---------------------------------------------------------------------------
    Compute the histogram for an image portion.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRPHistogram> HIMColorBalancedImage::GetHistogramInfoFor(const HFCPtr<HIMColorBalancedImage>& pi_rpImage,
                                                                const HFCPtr<HVEShape>&  pi_rpShape) const
    {
    HRASamplingOptions  SamplingOptions;
    SamplingOptions.SetRegionToScan(pi_rpShape);
    SamplingOptions.SetPixelsToScan(100);
    SamplingOptions.SetTilesToScan(100);

    // 50-100 -> 1:1 res.
    // < 50   -> first subres.
    // < 25   -> second subres.
    // < 12.5 -> third subres.
    // ...
    // Second subres seems very close. Third seems highly acceptable. Using
    // "1" to specify lowest resolution seems a bit too far off.
    switch(m_SamplingQuality)
        {
        case SAMPLING_FAST:
            SamplingOptions.SetPyramidImageSize(1);
            break;
        case SAMPLING_NORMAL:
            SamplingOptions.SetPyramidImageSize(20);
            break;
        case SAMPLING_HIGH_QUALITY:
            SamplingOptions.SetPyramidImageSize(100);
            break;
        default:
            HASSERT(false);
        }

    HRAHistogramOptions HistogramOptions(new HRPHistogram(pi_rpImage->GetColorMode() == COLORMODE_GRAY ? 256 : 768), 0);
    HistogramOptions.SetSamplingOptions(SamplingOptions);

    pi_rpImage->GetSource()->ComputeHistogram(&HistogramOptions);

    return HistogramOptions.GetHistogram();
    }


/** ---------------------------------------------------------------------------
    Compute the means of the dispersions for our 4 (potential) sides. All the
    useful sides must be set prior to calling this method.
    @note The mean is computed once and will be kept until ClearParameters()
    is called.

    @see ClearParameters()
    ---------------------------------------------------------------------------
*/
const HIMColorBalancedImage::RGBDispersion& HIMColorBalancedImage::GetLocalDispersionForGlobalAlgorithm()
    {
    if (m_MyGlobal.IsEmpty())
        {
        int32_t NumberOfNeighbors = (int32_t)GetNumberOfNeighbors();

        if (NumberOfNeighbors > 0)
            {
            // Accumulate sides
            if (m_HasLeftNeighbor)
                {
                m_MyGlobal += m_Left;
                }
            if (m_HasRightNeighbor)
                {
                m_MyGlobal += m_Right;
                }
            if (m_HasTopNeighbor)
                {
                m_MyGlobal += m_Top;
                }
            if (m_HasBottomNeighbor)
                {
                m_MyGlobal += m_Bottom;
                }

            // Do the means
            m_MyGlobal /= NumberOfNeighbors;
            }
        }

    return m_MyGlobal;
    }


/** ---------------------------------------------------------------------------
    Recompute histograms for all sides with neighbors
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::RecomputeHistograms()
    {
    if (m_HasLeftNeighbor)
        {
        ComputeLeftHistograms();
        }
    if (m_HasRightNeighbor)
        {
        ComputeRightHistograms();
        }
    if (m_HasTopNeighbor)
        {
        ComputeTopHistograms();
        }
    if (m_HasBottomNeighbor)
        {
        ComputeBottomHistograms();
        }
    }

//-----------------------------------------------------------------------------
// public
// IsStoredRaster
//-----------------------------------------------------------------------------
bool HIMColorBalancedImage::IsStoredRaster () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// Draw
//-----------------------------------------------------------------------------
void HIMColorBalancedImage::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    bool DrawDone = false;

    if (GetNumberOfNeighbors() > 0)
        {
        HRADrawOptions Options(pi_Options);

        HVEShape RegionToDraw(Options.GetShape() != 0 ? *Options.GetShape() : HRARaster::GetShape());
        RegionToDraw.ChangeCoordSys(GetCoordSys());
        if (Options.GetReplacingCoordSys() != 0)
            RegionToDraw.SetCoordSys(Options.GetReplacingCoordSys());

        HFCPtr<HGSRegion> pClipRegion(pio_destSurface.GetRegion());
        if (pClipRegion != 0)
            {
            // Intersect it with the destination
            HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());
            RegionToDraw.Intersect(*pSurfaceShape);
            }
        else
            {
            // Create a rectangular clip region to stay
            // inside the destination surface.
            HVEShape DestSurfaceShape(0.0, 0.0, pio_destSurface.GetSurfaceDescriptor()->GetWidth(), pio_destSurface.GetSurfaceDescriptor()->GetHeight(), pio_rpSurface->GetSurfaceCoordSys());
            RegionToDraw.Intersect(DestSurfaceShape);
            }

        if (!RegionToDraw.IsEmpty())
            {
            // Create memory surface like destination.
            HFCPtr<HGSSurfaceDescriptor> pSourceDataDescriptor(new HGSMemorySurfaceDescriptor(pio_destSurface.GetWidth(),
                                                               pio_destSurface.GetHeight(),
                                                               GetPixelType(),
                                                               (pio_destSurface.GetWidth() * GetPixelType()->CountPixelRawDataBits() + 7) / 8));
 
            HRASurface sourceDataSurface(pSourceDataDescriptor);

            try
                {
                // place the surface so it maps the same space
                // as the original destination surface.
                HGFMappedSurface mappedSourceDataSurface(sourceDataSurface.GetSurfaceDescriptor(), pio_destSurface.GetSurfaceCoordSys());
                    
                // Editor must be destroyed to finalize edition.
                    {
                    HRAEditor editor(sourceDataSurface);
                    editor.Clear(GetPixelType()->GetDefaultRawData());
                    }

                // Fill the temp. surface with our source. Don't clip...
                Options.SetShape(0);
                GetSource()->Draw(mappedSourceDataSurface, &Options);

                // Create memory surface like destination.
                HFCPtr<HGSSurfaceDescriptor> pBalancedDescriptor(new HGSMemorySurfaceDescriptor(pio_destSurface.GetWidth(),
                                                                                                pio_destSurface.GetHeight(),
                                                                                                GetPixelType(),
                                                                                                (pio_destSurface.GetWidth() * GetPixelType()->CountPixelRawDataBits() + 7) / 8));
                HASSERT(pBalancedDescriptor != 0);

                HRASurface balancedSurface(pBalancedDescriptor);

                // place the surface so it maps the same space
                // as the original destination surface.
                HGFMappedSurface mappedBalancedSurface(balancedSurface.GetSurfaceDescriptor(), 0, 0, pio_destSurface.GetSurfaceCoordSys());

                HASSERT(sourceDataSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));
                HASSERT(balancedSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

                // create the source pixel buffer
                HRPPixelBuffer SrcPixelBuffer(*GetPixelType(),
                                                ((HFCPtr<HGSMemorySurfaceDescriptor>&)sourceDataSurface.GetSurfaceDescriptor())->GetPacket()->GetBufferAddress(),
                                                pio_destSurface.GetWidth(),
                                                pio_destSurface.GetHeight());

                // create the destination pixel buffer
                HRPPixelBuffer DstPixelBuffer(*GetPixelType(),
                                                ((HFCPtr<HGSMemorySurfaceDescriptor>&)balancedSurface.GetSurfaceDescriptor())->GetPacket()->GetBufferAddress(),
                                                pio_destSurface.GetWidth(),
                                                pio_destSurface.GetHeight());

                HFCPtr<HVEShape> pSourceTotalShape(new HVEShape(*GetSource()->GetEffectiveShape()));
                pSourceTotalShape->ChangeCoordSys(pio_destSurface.GetSurfaceCoordSys());
                RegionToDraw.ChangeCoordSys(pio_destSurface.GetSurfaceCoordSys());
                HGF2DExtent ToDrawExtent(RegionToDraw.GetExtent());
                HGF2DExtent TotalExtent(pSourceTotalShape->GetExtent());
                HFCGrid ToDrawGrid(ToDrawExtent.GetXMin(),
                                    ToDrawExtent.GetYMin(),
                                    ToDrawExtent.GetXMax(),
                                    ToDrawExtent.GetYMax());

                HASSERT(ToDrawGrid.GetXMin() <= LONG_MAX);
                HASSERT(ToDrawGrid.GetYMin() <= LONG_MAX);

                int32_t X = (int32_t)ToDrawGrid.GetXMin();
                int32_t Y = (int32_t)ToDrawGrid.GetYMin();
                int32_t SrcDisplacementX = 0;
                int32_t SrcDisplacementY = 0;

                // Balance the data
                if (m_ApplyPositional)
                    {
                    if (m_ColorMode == HIMColorBalancedImage::COLORMODE_RGB)
                        {
                        if (m_ApplyGlobal)
                            {
                            if (GetNumberOfNeighbors() == 4)
                                ApplyColorBalanceRGB4(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            else
                                ApplyColorBalanceRGB(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            }
                        else
                            {
                            if (GetNumberOfNeighbors() == 4)
                                ApplyColorBalanceRGB4PositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            else
                                ApplyColorBalanceRGBPositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            }
                        }
                    else
                        {
                        // COLORMODE_GRAY

                        if (m_ApplyGlobal)
                            {
                            if (GetNumberOfNeighbors() == 4)
                                ApplyColorBalanceGray4(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            else
                                ApplyColorBalanceGray(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            }
                        else
                            {
                            if (GetNumberOfNeighbors() == 4)
                                ApplyColorBalanceGray4PositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            else
                                ApplyColorBalanceGrayPositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                            }
                        }
                    }
                else
                    {
                    if (m_ColorMode == HIMColorBalancedImage::COLORMODE_RGB)
                        ApplyColorBalanceRGBGlobalOnly(&SrcPixelBuffer, &DstPixelBuffer, SrcDisplacementX, SrcDisplacementY);
                    else
                        ApplyColorBalanceGrayGlobalOnly(&SrcPixelBuffer, &DstPixelBuffer, SrcDisplacementX, SrcDisplacementY);
                    }

                // Draw temp. surface in destination. This is where
                // the filter really gets applied.

                HFCPtr<HGSRegion> pOldClipRegion(pio_destSurface.GetRegion());
                pio_destSurface.SetRegion(new HGSRegion(new HVEShape(RegionToDraw), pio_destSurface.GetSurfaceCoordSys()));

                HRABlitter blitter(pio_destSurface);
                if(Options.ApplyAlphaBlend())
                    blitter.SetAlphaBlend(true);

                HFCPtr<HGF2DTransfoModel> pTransfoModel = pio_destSurface.GetSurfaceCoordSys()->GetTransfoModelTo (mappedSourceDataSurface.GetSurfaceCoordSys());

                // We're assuming
                // 1. Surfaces are compatible, since the super surface is the result of a CreateCompatibleSurface
                //    on the destination.
                // 2. The model is an identity

                blitter.BlitFrom(mappedSourceDataSurface.GetImplementation(), *pTransfoModel);

                pio_destSurface.SetRegion(pOldClipRegion);                        

                DrawDone = true;
                }
            catch(HFCOutOfMemoryException& Exception)
                {
                //if HFCOutOfMemoryException do nothing
                }
            }
        }

    // Draw the source directly
    if (!DrawDone)
        {
        HWARNING(GetNumberOfNeighbors() <= 0, L"HIMColorBalancedImage::Draw out of memory. Color balancing will not be applied.");

        GetSource()->Draw(pio_destSurface, pi_Options);
        }
    }



////////////////////
// COLORMODE_RGB
////////////////////



/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceRGB(HRPPixelBuffer* pi_pSourcePixels,
                                                 HRPPixelBuffer* pi_pDestPixels,
                                                 int32_t pi_OriginX,
                                                 int32_t pi_OriginY,
                                                 const HGF2DExtent& pi_rTotalExtent,
                                                 int32_t pi_DisplacementX,
                                                 int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceRed = m_GlobalMap.Red[*pSrc++];
            Red = SourceRed;
            SourceGreen = m_GlobalMap.Green[*pSrc++];
            Green = SourceGreen;
            SourceBlue = m_GlobalMap.Blue[*pSrc++];
            Blue = SourceBlue;

            // Take account of each neighbor
            if (m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Red   += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Red[SourceRed];
                Green += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Blue[SourceBlue];
                }
            if (m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Red   += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Red[SourceRed];
                Green += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Blue[SourceBlue];
                }
            if (m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Red   += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Red[SourceRed];
                Green += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Blue[SourceBlue];
                }
            if (m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Red   += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Red[SourceRed];
                Green += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Blue[SourceBlue];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image that has 4 neighbors (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceRGB4(HRPPixelBuffer* pi_pSourcePixels,
                                                  HRPPixelBuffer* pi_pDestPixels,
                                                  int32_t pi_OriginX,
                                                  int32_t pi_OriginY,
                                                  const HGF2DExtent& pi_rTotalExtent,
                                                  int32_t pi_DisplacementX,
                                                  int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceRed = m_GlobalMap.Red[*pSrc++];
            Red = SourceRed;
            SourceGreen = m_GlobalMap.Green[*pSrc++];
            Green = SourceGreen;
            SourceBlue = m_GlobalMap.Blue[*pSrc++];
            Blue = SourceBlue;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Red += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Red[SourceRed] +
                     WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Red[SourceRed] +
                     WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Red[SourceRed] +
                     WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Red[SourceRed] ) * 0.5;
            Green += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Green[SourceGreen] +
                       WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Green[SourceGreen] +
                       WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Green[SourceGreen] +
                       WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Green[SourceGreen] ) * 0.5;
            Blue += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Blue[SourceBlue] +
                      WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Blue[SourceBlue] +
                      WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Blue[SourceBlue] +
                      WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Blue[SourceBlue] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceRGBPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                               HRPPixelBuffer* pi_pDestPixels,
                                                               int32_t pi_OriginX,
                                                               int32_t pi_OriginY,
                                                               const HGF2DExtent& pi_rTotalExtent,
                                                               int32_t pi_DisplacementX,
                                                               int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceRed = *pSrc++;
            Red = SourceRed;
            SourceGreen = *pSrc++;
            Green = SourceGreen;
            SourceBlue = *pSrc++;
            Blue = SourceBlue;

            // Take account of each neighbor
            if (m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Red   += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Red[SourceRed];
                Green += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightLeft * ((RGBDeltas*)m_pLeftDeltas)->Blue[SourceBlue];
                }
            if (m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Red   += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Red[SourceRed];
                Green += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightRight * ((RGBDeltas*)m_pRightDeltas)->Blue[SourceBlue];
                }
            if (m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Red   += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Red[SourceRed];
                Green += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightTop * ((RGBDeltas*)m_pTopDeltas)->Blue[SourceBlue];
                }
            if (m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Red   += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Red[SourceRed];
                Green += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Blue[SourceBlue];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image that has 4 neighbors (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceRGB4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                HRPPixelBuffer* pi_pDestPixels,
                                                                int32_t pi_OriginX,
                                                                int32_t pi_OriginY,
                                                                const HGF2DExtent& pi_rTotalExtent,
                                                                int32_t pi_DisplacementX,
                                                                int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceRed = *pSrc++;
            Red = SourceRed;
            SourceGreen = *pSrc++;
            Green = SourceGreen;
            SourceBlue = *pSrc++;
            Blue = SourceBlue;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Red += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Red[SourceRed] +
                     WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Red[SourceRed] +
                     WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Red[SourceRed] +
                     WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Red[SourceRed] ) * 0.5;
            Green += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Green[SourceGreen] +
                       WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Green[SourceGreen] +
                       WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Green[SourceGreen] +
                       WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Green[SourceGreen] ) * 0.5;
            Blue += ( WeightLeft *   ((RGBDeltas*)m_pLeftDeltas)->Blue[SourceBlue] +
                      WeightRight *  ((RGBDeltas*)m_pRightDeltas)->Blue[SourceBlue] +
                      WeightTop *    ((RGBDeltas*)m_pTopDeltas)->Blue[SourceBlue] +
                      WeightBottom * ((RGBDeltas*)m_pBottomDeltas)->Blue[SourceBlue] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (global)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceRGBGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                           HRPPixelBuffer* pi_pDestPixels,
                                                           int32_t pi_DisplacementX,
                                                           int32_t pi_DisplacementY) const
    {
    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    for (uint32_t Y = 0 ; Y < pi_pDestPixels->GetHeight() ; ++Y)
        {
        for (uint32_t X = 0 ; X < pi_pDestPixels->GetWidth() ; ++X)
            {
            *pDst++ = m_GlobalMap.Red[*pSrc++];
            *pDst++ = m_GlobalMap.Green[*pSrc++];
            *pDst++ = m_GlobalMap.Blue[*pSrc++];
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }



////////////////////
// COLORMODE_GRAY
////////////////////



/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceGray(HRPPixelBuffer* pi_pSourcePixels,
                                                  HRPPixelBuffer* pi_pDestPixels,
                                                  int32_t pi_OriginX,
                                                  int32_t pi_OriginY,
                                                  const HGF2DExtent& pi_rTotalExtent,
                                                  int32_t pi_DisplacementX,
                                                  int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceGray = m_GlobalMap.Gray[*pSrc++];
            Gray = SourceGray;

            // Take account of each neighbor
            if (m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Gray += NormalizedWeightLeft * m_pLeftDeltas->Gray[SourceGray];
                }
            if (m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Gray += NormalizedWeightRight * m_pRightDeltas->Gray[SourceGray];
                }
            if (m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Gray += NormalizedWeightTop * m_pTopDeltas->Gray[SourceGray];
                }
            if (m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Gray += NormalizedWeightBottom * m_pBottomDeltas->Gray[SourceGray];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image that has 4 neighbors (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceGray4(HRPPixelBuffer* pi_pSourcePixels,
                                                   HRPPixelBuffer* pi_pDestPixels,
                                                   int32_t pi_OriginX,
                                                   int32_t pi_OriginY,
                                                   const HGF2DExtent& pi_rTotalExtent,
                                                   int32_t pi_DisplacementX,
                                                   int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceGray = m_GlobalMap.Gray[*pSrc++];
            Gray = SourceGray;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Gray += ( WeightLeft * m_pLeftDeltas->Gray[SourceGray] +
                      WeightRight * m_pRightDeltas->Gray[SourceGray] +
                      WeightTop * m_pTopDeltas->Gray[SourceGray] +
                      WeightBottom * m_pBottomDeltas->Gray[SourceGray] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceGrayPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                HRPPixelBuffer* pi_pDestPixels,
                                                                int32_t pi_OriginX,
                                                                int32_t pi_OriginY,
                                                                const HGF2DExtent& pi_rTotalExtent,
                                                                int32_t pi_DisplacementX,
                                                                int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceGray = *pSrc++;
            Gray = SourceGray;

            // Take account of each neighbor
            if (m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Gray += NormalizedWeightLeft * m_pLeftDeltas->Gray[SourceGray];
                }
            if (m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Gray += NormalizedWeightRight * m_pRightDeltas->Gray[SourceGray];
                }
            if (m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Gray += NormalizedWeightTop * m_pTopDeltas->Gray[SourceGray];
                }
            if (m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Gray += NormalizedWeightBottom * m_pBottomDeltas->Gray[SourceGray];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image that has 4 neighbors (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceGray4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                 HRPPixelBuffer* pi_pDestPixels,
                                                                 int32_t pi_OriginX,
                                                                 int32_t pi_OriginY,
                                                                 const HGF2DExtent& pi_rTotalExtent,
                                                                 int32_t pi_DisplacementX,
                                                                 int32_t pi_DisplacementY) const
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceGray = *pSrc++;
            Gray = SourceGray;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Gray += ( WeightLeft * m_pLeftDeltas->Gray[SourceGray] +
                      WeightRight * m_pRightDeltas->Gray[SourceGray] +
                      WeightTop * m_pTopDeltas->Gray[SourceGray] +
                      WeightBottom * m_pBottomDeltas->Gray[SourceGray] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (global)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImage::ApplyColorBalanceGrayGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                            HRPPixelBuffer* pi_pDestPixels,
                                                            int32_t pi_DisplacementX,
                                                            int32_t pi_DisplacementY) const
    {
    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    for (uint32_t Y = 0 ; Y < pi_pDestPixels->GetHeight() ; ++Y)
        {
        for (uint32_t X = 0 ; X < pi_pDestPixels->GetWidth() ; ++X)
            {
            *pDst++ = m_GlobalMap.Gray[*pSrc++];
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }

