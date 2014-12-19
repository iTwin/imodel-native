//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPDEMFilter.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>


#include <Imagepp/all/h/HRPDEMFilter.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSEditor.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

// Conflict with the use of std::numeric_limits<T>::max()
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// [a, b, c]
// [d, e, f]
// [g, h, i]
#define A_Val pSrcLinePre[Column-1]
#define B_Val pSrcLinePre[Column]
#define C_Val pSrcLinePre[Column+1]
#define D_Val pSrcLine[Column-1]
#define E_Val pSrcLine[Column]
#define F_Val pSrcLine[Column+1]
#define G_Val pSrcLineNext[Column-1]
#define H_Val pSrcLineNext[Column]
#define I_Val pSrcLineNext[Column+1]

#define GetRed(rgba)    (rgba & 0xFF)
#define GetGreen(rgba)  ((rgba >> 8) & 0xFF)
#define GetBlue(rgba)   ((rgba >> 16) & 0xFF)
#define GetAlpha(rgba)  ((rgba >> 24) & 0xFF)
#define MakeRGBA(red, green, blue, alpha) (((alpha) << 24) | ((blue) << 16) | ((green) << 8) | (red))

/*---------------------------------------------------------------------------------**//**
* Class epsilon_value
+---------------+---------------+---------------+---------------+---------------+------*/
template <class _type> struct epsilon_value
    {
    static double epsilon() {
        return 0.00001;
        }
    };

/*---------------------------------------------------------------------------------**//**
* Class min_value
+---------------+---------------+---------------+---------------+---------------+------*/
template <class _type> struct min_value
    {
    static _type min() {
        return std::numeric_limits<_type>::min();
        }
    };

/*---------------------------------------------------------------------------------**//**
* Class min_value (Template specialization)
+---------------+---------------+---------------+---------------+---------------+------*/
template<> struct min_value<double>
    {
    static double min() {
        return -std::numeric_limits<double>::max();
        }
    };

/*---------------------------------------------------------------------------------**//**
* Class RangeMap
+---------------+---------------+---------------+---------------+---------------+------*/
template <class _keyType, class _valueType> class RangeMap
    {
public:
    /*---------------------------------------------------------------------------------**//**
    * Types
    +---------------+---------------+---------------+---------------+---------------+------*/
    typedef std::pair<_keyType, _valueType> value_type;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    RangeMap()
        : m_minKey(std::numeric_limits<_keyType>::max()), m_maxKey(min_value<_keyType>::min()),
          m_size(0), m_factor(1.0), m_lowerBounded(false), m_upperBounded(false)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~RangeMap()
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    void insert(_keyType const& key, _valueType const& value)
        {
        m_keyValueMap.insert(value_type(key, value));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    void insert(value_type const& value)
        {
        m_keyValueMap.insert(value);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetLowerDefaultValue(_valueType value)
        {
        m_lowerDefaultValue = value;
        m_lowerBounded = true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUpperDefaultValue(_valueType value)
        {
        m_upperDefaultValue = value;
        m_upperBounded = true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
#ifdef __HMR_DEBUG
    // Called in debug for validation purposes only.
    _valueType const& lower_bound_from_hashing(_keyType const& key) const
#else
    // Directly called in release.
    _valueType const& lower_bound(_keyType const& key) const
#endif
        {
        HASSERT (!m_entries.empty());

        // test upper and lower limit first
        if (key >= m_maxKey)
            {
            if (key > m_maxKey && m_upperBounded)
                return m_upperDefaultValue;

            return m_keyValueMap.rbegin()->second;
            }
        else if (key <= m_minKey)
            {
            if (key < m_minKey && m_lowerBounded)
                return m_lowerDefaultValue;

            return m_keyValueMap.begin()->second;
            }

        // The key is in range
        // compute corresponding index in internal vector
        size_t const idx ((size_t)((key - m_minKey) * m_factor));

        // find the corresponding entry to the key
        HASSERT (idx <= m_entries.size());
        HASSERT (m_entries[idx].size() > 0);
        for (typename item_type::const_iterator itr(m_entries[idx].begin()); itr != m_entries[idx].end(); ++itr)
            {
            if (key <= itr->first)
                return (*itr).second;
            }

        HASSERT (false); // we should never get here
        return m_entries.rbegin()->rbegin()->second;
        }

#ifdef __HMR_DEBUG
    // This method is used in debug to validate the acuracy of hashing.
    // It compares the value returned by RangeMap::lower_bound_from_hashing and std::map::lower_bound.
    _valueType const& lower_bound(_keyType const& key) const
        {
        // get value from hashing
        _valueType const& value_hash(lower_bound_from_hashing(key));
        _valueType value;
        if (key >= m_maxKey)
            {
            if (key > m_maxKey && m_upperBounded)
                value = m_upperDefaultValue;
            else
                value = m_keyValueMap.rbegin()->second;
            }
        else if (key <= m_minKey)
            {
            if (key < m_minKey && m_lowerBounded)
                value = m_lowerDefaultValue;
            else
                value = m_keyValueMap.begin()->second;
            }
        else
            {
            // the key is in range
            typename key_value_map::const_iterator itr = m_keyValueMap.lower_bound(key);
            HASSERT (itr != m_keyValueMap.end());
            value = itr->second;
            }

        HASSERT (0 == memcmp(&value_hash, &value, sizeof (value)));
        return value_hash;
        }
#endif

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    void update_hash()
        {
        if (m_keyValueMap.empty())
            return;

        m_entries.clear();

        if (m_size == 0)
            m_size = m_keyValueMap.size() * 8;

        m_entries.resize(m_size);

        m_minKey = m_keyValueMap.begin()->first;
        m_maxKey = m_keyValueMap.rbegin()->first;

        if (m_maxKey - m_minKey)
            m_factor = (double)m_size / (m_maxKey - m_minKey);

        size_t maxIdx = m_entries.size()-1;
        typename key_value_map::const_iterator itr_from (m_keyValueMap.begin());
        typename key_value_map::const_iterator itr_to (m_keyValueMap.begin());
        itr_to++;
        for (; itr_to != m_keyValueMap.end(); ++itr_from, ++itr_to)
            {
            // compute index
            size_t firstIdx = (size_t)((itr_from->first - m_minKey) * m_factor);
            size_t lastIdx =  (size_t)((itr_to->first - m_minKey) * m_factor);
            lastIdx = hmin(lastIdx, maxIdx);    // Fix possible rounding error
            size_t idx = firstIdx;

            do
                {
                value_type value (itr_to->first, itr_to->second);

                // insert the range
                m_entries[idx].push_back(value);

                // sort entries
                sort(m_entries[idx].begin(), m_entries[idx].end(), RangeMap<_keyType, _valueType>::range_pred);

                idx++;
                }
            while (idx <= lastIdx);
            }
        }

private:
    /*---------------------------------------------------------------------------------**//**
    * Types
    +---------------+---------------+---------------+---------------+---------------+------*/
    typedef std::map<_keyType, _valueType>  key_value_map;
    typedef std::vector<value_type>         item_type;
    typedef std::vector<item_type>          array_type;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    StephanePoulin  06/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool range_pred (value_type const& elem1, value_type const& elem2)
        {
        return elem1.first < elem2.first;
        }

    /*---------------------------------------------------------------------------------**//**
    * Members
    +---------------+---------------+---------------+---------------+---------------+------*/
    _keyType        m_minKey;
    _keyType        m_maxKey;
    size_t          m_size;
    double          m_factor;
    key_value_map   m_keyValueMap;
    bool            m_lowerBounded;
    _valueType      m_lowerDefaultValue;
    bool            m_upperBounded;
    _valueType      m_upperDefaultValue;
    array_type      m_entries;
    };


// Use this if we need to test noData float value with an epsilon
// template<class _Ty>
// class IsNoData
// {
// public:
//     static bool Is(const _Ty& value, const _Ty& noDataValue)
//     {
//         return value == noDataValue;
//     }
// };
//
// template<> class IsNoData<float>
// {
// public:
//     static bool Is(const float& value, const float& noDataValue)
//     {
//         return HDOUBLE_EQUAL(value, noDataValue, FLT_EPSILON);
//     }
// };

// inline double fast_atan(double x)
//     {
//     if(fabs(x) <= 1)
//         return x/(1+ 0.28*x*x);
//
//     if(x >= 0)
//         return  PI/2 - x/(x*x + 0.28);
//
//     return -(PI/2 + x/(x*x + 0.28));
//     }

//-----------------------------------------------------------------------------
// ComputeSlopeRad
//-----------------------------------------------------------------------------
inline double ComputeSlopeRad(double const& pi_rRateX, double const& pi_rRateY, unsigned short const& pi_rZFactor = 1)
    {
    // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
    // slope_rad = ATAN(rise_run)
    double rise_run = sqrt((pi_rRateX*pi_rRateX) + (pi_rRateY*pi_rRateY));
    return atan(pi_rZFactor * rise_run);
    }

//-----------------------------------------------------------------------------
// ComputeAspectRad
//-----------------------------------------------------------------------------
inline double ComputeAspectRad(double const& pi_rRateX, double const& pi_rRateY)
    {
    // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
    // if Aspect_rad < 0 then
    //    Aspect_rad = 2*PI + Aspect_rad;
    double Aspect_rad = atan2(pi_rRateY, -pi_rRateX);
    if(Aspect_rad < 0)
        return 2*PI + Aspect_rad;

    return Aspect_rad;
    }

//-----------------------------------------------------------------------------
// ComputeCompassDirectionDeg
//-----------------------------------------------------------------------------
inline double ComputeCompassDirectionDeg(double const& pi_rRateX, double const& pi_rRateY)
    {
    // Aspect_deg = 57.29578 * atan2( (dz/dx), -(dz/dx) )
    // if Aspect_deg < 0 then
    //    direction = 90 - Aspect_deg
    // else if Aspect_deg > 90 then
    //    direction = 360 - Aspect_deg + 90
    // else
    //    direction = 90 - Aspect_deg
    double Aspect_deg = 57.29578 * atan2(pi_rRateX, -pi_rRateY);

    HASSERT(Aspect_deg >= -181. && Aspect_deg <= 181.);

    if(Aspect_deg > 90.0)
        return 360 - Aspect_deg + 90;

    return 90 - Aspect_deg;
    }

//-----------------------------------------------------------------------------
// ComputeZenithRad
//-----------------------------------------------------------------------------
inline double ComputeZenithRad(unsigned short const& pi_AltitudeAngle)
    {
    // Altitude                                 >> The altitude of the illumination source is specified in degrees above horizontal.
    // Zenith_deg = 90 - Altitude               >> Change altitude to zenith angle
    // Zenith_rad = Zenith_deg * PI / 180.0     >> Convert to radians
    return (90 - pi_AltitudeAngle) * PI / 180.0;
    }

//-----------------------------------------------------------------------------
// ComputeAzimuthRad
//-----------------------------------------------------------------------------
inline double ComputeAzimuthRad(unsigned short const& pi_AzimuthDeg)
    {
    // Azimuth                                  >> The direction of the illumination in degrees(compass direction).
    // Azimuth_math = 360.0 - m_Azimuth + 90    >> Change azimuth to mathematic unit (right angle)
    // if Azimuth_math >= 360 then
    //     Azimuth_math = Azimuth_math - 360.0
    // Azimuth_rad = Azimuth_rad * PI / 180.0   >> Convert to radians
    double Azimuth_rad = 360.0 - pi_AzimuthDeg + 90;
    if(Azimuth_rad >= 360.0)
        Azimuth_rad = Azimuth_rad - 360.0;

    return Azimuth_rad * PI / 180.0;
    }


//-----------------------------------------------------------------------------
// DEMFilterImplementation
//-----------------------------------------------------------------------------
class DEMFilterImplementation
    {
public:
    DEMFilterImplementation(double const& pi_UnitSizeX, double const& pi_UnitSizeY, const HRPDEMFilter::HillShadingSettings& pi_HillShadingSettings, unsigned short const& pi_rVerticalExaggeration, Byte const* pi_pDefaultRGBA)
        :m_UnitSizeX(pi_UnitSizeX),
         m_UnitSizeY(pi_UnitSizeY),
         m_HillShadingSettings(pi_HillShadingSettings),
         m_VerticalExaggeration(pi_rVerticalExaggeration),
         m_pOrientationTransfo(new HGF2DIdentity),
         m_00(1.0),
         m_01(0.0),
         m_10(0.0),
         m_11(1.0)
        {
        m_DefaultRGBA = *(uint32_t*)pi_pDefaultRGBA;
        }
    virtual ~DEMFilterImplementation()  {}

    virtual void SetNoDataValue(double const& pi_rNoDataValue) = 0;

    void         SetOrientationTransfo(HFCPtr<HGF2DTransfoModel> pi_pOrientationTransfo)
        {
        HPRECONDITION(pi_pOrientationTransfo != 0);
        HASSERT (pi_pOrientationTransfo->CanBeRepresentedByAMatrix());

        HFCMatrix<3,3> orientationMatrix(pi_pOrientationTransfo->GetMatrix());
        HASSERT(orientationMatrix[0][2] == 0.0);    // No translation assumed.
        HASSERT(orientationMatrix[1][2] == 0.0);    // No translation assumed.

        m_pOrientationTransfo = pi_pOrientationTransfo;

        m_00 = orientationMatrix[0][0];
        m_01 = orientationMatrix[0][1];
        m_10 = orientationMatrix[1][0];
        m_11 = orientationMatrix[1][1];
        }

    void ApplyFilter(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                     HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY)
        {
        if(m_HillShadingSettings.GetHillShadingState())
            ProcessPixelsWithShading(pi_pDestination, pi_pSource, pi_rNeighbourhood, pi_ScalingX, pi_ScalingY);
        else
            ProcessPixels(pi_pDestination, pi_pSource, pi_rNeighbourhood, pi_ScalingX, pi_ScalingY);
        }

    const HRPDEMFilter::HillShadingSettings& GetHillShadingSettings() const {return m_HillShadingSettings;}

protected:

    virtual void ProcessPixelsWithShading(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                          HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) = 0;

    virtual void ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                               HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) = 0;

    double     m_UnitSizeX;                // Size of a pixels. In pixel elevation unit (usually, meters or feet)
    double     m_UnitSizeY;                // Size of a pixels. In pixel elevation unit (usually, meters or feet)
    uint32_t   m_DefaultRGBA;              // Default color
    HRPDEMFilter::HillShadingSettings m_HillShadingSettings;
    unsigned short m_VerticalExaggeration;     // HillShading option.
    HFCPtr<HGF2DTransfoModel>   m_pOrientationTransfo;

    double      m_00;
    double      m_01;
    double      m_10;
    double      m_11;
    };

//-----------------------------------------------------------------------------
// VoidFilterImpl
//-----------------------------------------------------------------------------
class VoidFilterImpl : public DEMFilterImplementation
    {
public:
    VoidFilterImpl(Byte const* pi_pDefaultRGB)
        :DEMFilterImplementation(1.0, 1.0, HRPDEMFilter::HillShadingSettings(), 1, pi_pDefaultRGB)   // Do not care from most of the base member.
        {
        }

    virtual void ProcessPixelsWithShading(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                          HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        ProcessPixels(pi_pDestination, pi_pSource, pi_rNeighbourhood, pi_ScalingX, pi_ScalingY);
        }

    virtual void ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                               HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        // Clear with default color.
        HFCPtr<HGSEditor> pEditor = new HGSEditor();
        pEditor->SetFor(new HGSSurface(pi_pDestination.GetPtr()));
        pEditor->Clear(&m_DefaultRGBA);
        }

    virtual void SetNoDataValue(double const& pi_rNoDataValue) {
        /*do not care*/
        };
    };


//-----------------------------------------------------------------------------
// FilterImplTyped
//-----------------------------------------------------------------------------
template<class _SrcT, class _RangeT>
class FilterImplTyped : public DEMFilterImplementation
    {
public:
    FilterImplTyped(double const& pi_UnitSizeX, double const& pi_UnitSizeY, const HRPDEMFilter::HillShadingSettings& pi_HillShadingSetting, unsigned short const& pi_rVerticalExaggeration, bool pi_ClipToEndValues, Byte const* pi_pDefaultRGB, HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues)
        :DEMFilterImplementation(pi_UnitSizeX, pi_UnitSizeY, pi_HillShadingSetting, pi_rVerticalExaggeration, pi_pDefaultRGB),
         m_HasNoDataValue(false)
        {
        for(HRPDEMFilter::UpperRangeValues::const_iterator itr(pi_rUpperRangeValues.begin()); itr != pi_rUpperRangeValues.end(); ++itr)
            {
            HRPDEMFilter::RangeInfo const& rangeInfo = itr->second;

            if(rangeInfo.m_IsOn)
                {
                m_UpperRangeValues.insert(typename UpperRangeValuesT::value_type((_RangeT)itr->first, MakeRGBA(rangeInfo.m_rgb[0], rangeInfo.m_rgb[1], rangeInfo.m_rgb[2], 0xFF)));
                }
            else
                {
                m_UpperRangeValues.insert(typename UpperRangeValuesT::value_type((_RangeT)itr->first, m_DefaultRGBA));
                }
            }

        if(pi_ClipToEndValues)
            {
            // Everything that goes beyond the max or below the min value will be set to the default color.
            m_UpperRangeValues.SetUpperDefaultValue(m_DefaultRGBA);
            m_UpperRangeValues.SetLowerDefaultValue(m_DefaultRGBA);
            }

        m_UpperRangeValues.update_hash();
        }

    virtual void SetNoDataValue(double const& pi_rNoDataValue) override
        {
        m_NoDataValue = (_SrcT)pi_rNoDataValue;
        m_HasNoDataValue = true;
        }

protected:
    typedef RangeMap<_RangeT, uint32_t> UpperRangeValuesT;       // <RangeValue, RGBAColor>

    bool                m_HasNoDataValue;
    _SrcT               m_NoDataValue;          // Pixel value that indicate that no elevation information is available at this coordinate.
    UpperRangeValuesT   m_UpperRangeValues;     // Adapted version of the upper range values table.
    };

//-----------------------------------------------------------------------------
// ElevationFilterImpl
//-----------------------------------------------------------------------------
template<class _SrcT>
class ElevationFilterImpl : public FilterImplTyped<_SrcT, _SrcT>
    {
public:
    ElevationFilterImpl(double const& pi_UnitSizeX, double const& pi_UnitSizeY, const HRPDEMFilter::HillShadingSettings& pi_HillShadingSetting, unsigned short const& pi_rVerticalExaggeration, bool pi_ClipToEndValues, Byte const* pi_pDefaultRGB, HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues)
        : FilterImplTyped<_SrcT, _SrcT>(pi_UnitSizeX, pi_UnitSizeY, pi_HillShadingSetting, pi_rVerticalExaggeration, pi_ClipToEndValues, pi_pDefaultRGB, pi_rUpperRangeValues)
        {
        }

    virtual void ProcessPixelsWithShading(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                          HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_pDestination->GetWidth() + (pi_rNeighbourhood.GetWidth()-1) == pi_pSource->GetWidth());
        HPRECONDITION(pi_pDestination->GetHeight() + (pi_rNeighbourhood.GetHeight()-1) == pi_pSource->GetHeight());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(FilterImplTyped<_SrcT, _SrcT>::m_HillShadingSettings.GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(FilterImplTyped<_SrcT, _SrcT>::m_HillShadingSettings.GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * FilterImplTyped<_SrcT, _SrcT>::m_UnitSizeX* pi_ScalingX);
        double RateMultiplierY = 1 / (8.0 * FilterImplTyped<_SrcT, _SrcT>::m_UnitSizeY* pi_ScalingY);

        for(uint32_t Line=pi_rNeighbourhood.GetYOrigin(); Line < Heigh + pi_rNeighbourhood.GetYOrigin(); ++Line)
            {
            _SrcT const* pSrcLinePre  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line-1)));
            _SrcT const* pSrcLine     =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));
            _SrcT const* pSrcLineNext =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line+1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*(Line-1)));

            if(FilterImplTyped<_SrcT, _SrcT>::m_HasNoDataValue)
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    if(E_Val == FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, _SrcT>::m_DefaultRGBA;
                        continue;
                        }

                    _SrcT A = A_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? A_Val : E_Val;
                    _SrcT B = B_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? B_Val : E_Val;
                    _SrcT C = C_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? C_Val : E_Val;
                    _SrcT D = D_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? D_Val : E_Val;
                    _SrcT F = F_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? F_Val : E_Val;
                    _SrcT G = G_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? G_Val : E_Val;
                    _SrcT H = H_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? H_Val : E_Val;
                    _SrcT I = I_Val != FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue ? I_Val : E_Val;

                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                    double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, _SrcT>::m_00 * tempRateX + FilterImplTyped<_SrcT, _SrcT>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, _SrcT>::m_10 * tempRateX + FilterImplTyped<_SrcT, _SrcT>::m_11 * tempRateY;

                    double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, _SrcT>::m_VerticalExaggeration);
                    double Aspect_rad = ComputeAspectRad(RateX, RateY);

                    double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                    if(hillShade > 0)
                        {
                        uint32_t const RGBAColor = FilterImplTyped<_SrcT, _SrcT>::m_UpperRangeValues.lower_bound(E_Val);
                        pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                            (Byte)(GetGreen(RGBAColor)*hillShade),
                                                            (Byte)(GetBlue(RGBAColor)*hillShade),
                                                            GetAlpha(RGBAColor));
                        }
                    else    // 100% shaded
                        {
                        pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                        }
                    }
                }
            else
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C_Val + 2*F_Val + I_Val) - (A_Val + 2*D_Val + G_Val)) * RateMultiplierX;
                    double tempRateY = ((G_Val + 2*H_Val + I_Val) - (A_Val + 2*B_Val + C_Val)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, _SrcT>::m_00 * tempRateX + FilterImplTyped<_SrcT, _SrcT>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, _SrcT>::m_10 * tempRateX + FilterImplTyped<_SrcT, _SrcT>::m_11 * tempRateY;

                    double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, _SrcT>::m_VerticalExaggeration);
                    double Aspect_rad = ComputeAspectRad(RateX, RateY);

                    double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                    if(hillShade > 0)
                        {
                        uint32_t const RGBAColor = FilterImplTyped<_SrcT, _SrcT>::m_UpperRangeValues.lower_bound(E_Val);
                        pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                            (Byte)(GetGreen(RGBAColor)*hillShade),
                                                            (Byte)(GetBlue(RGBAColor)*hillShade),
                                                            GetAlpha(RGBAColor));
                        }
                    else    // 100% shaded
                        {
                        pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                        }
                    }
                }
            }
        }

    virtual void ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                               HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_rNeighbourhood.IsUnity());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        for(uint32_t Line=0; Line < Heigh; ++Line)
            {
            _SrcT const* pSrcLine  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));

            uint32_t*  pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*Line));

            // NO data tag.
            if(FilterImplTyped<_SrcT, _SrcT>::m_HasNoDataValue)
                {
                for(uint32_t Column=0; Column < Width; ++Column)
                    {
                    // NO data tag.
                    if(pSrcLine[Column] == FilterImplTyped<_SrcT, _SrcT>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column] = FilterImplTyped<_SrcT, _SrcT>::m_DefaultRGBA;
                        }
                    else
                        {
                        pDstLineBuffer[Column] = FilterImplTyped<_SrcT, _SrcT>::m_UpperRangeValues.lower_bound(E_Val);
                        }
                    }
                }
            else
                {
                for(uint32_t Column=0; Column < Width; ++Column)
                    {
                    pDstLineBuffer[Column] = FilterImplTyped<_SrcT, _SrcT>::m_UpperRangeValues.lower_bound(E_Val);
                    }
                }
            }
        }
    };


//-----------------------------------------------------------------------------
// SlopePercentFilterImpl
//-----------------------------------------------------------------------------
template<class _SrcT>
class SlopePercentFilterImpl : public FilterImplTyped<_SrcT, double>
    {
public:
    SlopePercentFilterImpl(double const& pi_UnitSizeX, double const& pi_UnitSizeY, const HRPDEMFilter::HillShadingSettings& pi_HillShadingSetting, unsigned short const& pi_rVerticalExaggeration, bool pi_ClipToEndValues, Byte const* pi_pDefaultRGB, HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues)
        :FilterImplTyped<_SrcT, double>(pi_UnitSizeX, pi_UnitSizeY, pi_HillShadingSetting, pi_rVerticalExaggeration, pi_ClipToEndValues, pi_pDefaultRGB, pi_rUpperRangeValues)
        {
        }

    virtual void ProcessPixelsWithShading(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                          HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_pDestination->GetWidth() + (pi_rNeighbourhood.GetWidth()-1) == pi_pSource->GetWidth());
        HPRECONDITION(pi_pDestination->GetHeight() + (pi_rNeighbourhood.GetHeight()-1) == pi_pSource->GetHeight());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(FilterImplTyped<_SrcT, double>::m_HillShadingSettings.GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(FilterImplTyped<_SrcT, double>::m_HillShadingSettings.GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeX* pi_ScalingX);
        double RateMultiplierY = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeY* pi_ScalingY);

        for(uint32_t Line=pi_rNeighbourhood.GetYOrigin(); Line < Heigh + pi_rNeighbourhood.GetYOrigin(); ++Line)
            {
            _SrcT const* pSrcLinePre  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line-1)));
            _SrcT const* pSrcLine     =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));
            _SrcT const* pSrcLineNext =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line+1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*(Line-1)));

            if(FilterImplTyped<_SrcT, double>::m_HasNoDataValue)
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    if(E_Val == FilterImplTyped<_SrcT, double>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        continue;
                        }

                    _SrcT A = A_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? A_Val : E_Val;
                    _SrcT B = B_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? B_Val : E_Val;
                    _SrcT C = C_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? C_Val : E_Val;
                    _SrcT D = D_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? D_Val : E_Val;
                    _SrcT F = F_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? F_Val : E_Val;
                    _SrcT G = G_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? G_Val : E_Val;
                    _SrcT H = H_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? H_Val : E_Val;
                    _SrcT I = I_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? I_Val : E_Val;

                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                    double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, double>::m_VerticalExaggeration);
                    double Aspect_rad = ComputeAspectRad(RateX, RateY);

                    double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                    if(hillShade > 0) //&&MM test speed with test of hillShade > 1/255
                        {
                        // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                        double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                        uint32_t const RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(slopePercent);
                        pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                            (Byte)(GetGreen(RGBAColor)*hillShade),
                                                            (Byte)(GetBlue(RGBAColor)*hillShade),
                                                            GetAlpha(RGBAColor));
                        }
                    else        // 100% shaded
                        {
                        pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                        }
                    }
                }
            else
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C_Val + 2*F_Val + I_Val) - (A_Val + 2*D_Val + G_Val)) * RateMultiplierX;
                    double tempRateY = ((G_Val + 2*H_Val + I_Val) - (A_Val + 2*B_Val + C_Val)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, double>::m_VerticalExaggeration);
                    double Aspect_rad = ComputeAspectRad(RateX, RateY);

                    double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                    if(hillShade > 0)
                        {
                        // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                        double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                        uint32_t const RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(slopePercent);
                        pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                            (Byte)(GetGreen(RGBAColor)*hillShade),
                                                            (Byte)(GetBlue(RGBAColor)*hillShade),
                                                            GetAlpha(RGBAColor));
                        }
                    else    // 100% shaded
                        {
                        pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                        }
                    }
                }
            }
        }

    virtual void ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                               HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_pDestination->GetWidth() + (pi_rNeighbourhood.GetWidth()-1) == pi_pSource->GetWidth());
        HPRECONDITION(pi_pDestination->GetHeight() + (pi_rNeighbourhood.GetHeight()-1) == pi_pSource->GetHeight());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        // Pre-computed variables
        double RateMultiplierX = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeX* pi_ScalingX);
        double RateMultiplierY = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeY* pi_ScalingY);

        for(uint32_t Line=pi_rNeighbourhood.GetYOrigin(); Line < Heigh + pi_rNeighbourhood.GetYOrigin(); ++Line)
            {
            _SrcT const* pSrcLinePre  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line-1)));
            _SrcT const* pSrcLine     =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));
            _SrcT const* pSrcLineNext =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line+1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*(Line-1)));

            if(FilterImplTyped<_SrcT, double>::m_HasNoDataValue)
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    if(E_Val == FilterImplTyped<_SrcT, double>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        continue;
                        }

                    _SrcT A = A_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? A_Val : E_Val;
                    _SrcT B = B_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? B_Val : E_Val;
                    _SrcT C = C_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? C_Val : E_Val;
                    _SrcT D = D_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? D_Val : E_Val;
                    _SrcT F = F_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? F_Val : E_Val;
                    _SrcT G = G_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? G_Val : E_Val;
                    _SrcT H = H_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? H_Val : E_Val;
                    _SrcT I = I_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? I_Val : E_Val;

                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                    double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                    double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                    pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(slopePercent);
                    }
                }
            else
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double RateX = ((C_Val + 2*F_Val + I_Val) - (A_Val + 2*D_Val + G_Val)) * RateMultiplierX;
                    double RateY = ((G_Val + 2*H_Val + I_Val) - (A_Val + 2*B_Val + C_Val)) * RateMultiplierY;
                    // *** Does not require to apply orientation transform.

                    // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                    double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                    pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(slopePercent);
                    }
                }
            }
        }
    };

//-----------------------------------------------------------------------------
// AspectFilterImpl
//-----------------------------------------------------------------------------
template<class _SrcT>
class AspectFilterImpl : public FilterImplTyped<_SrcT, double>
    {
public:
    AspectFilterImpl(double const& pi_UnitSizeX, double const& pi_UnitSizeY, const HRPDEMFilter::HillShadingSettings& pi_HillShadingSetting, unsigned short const& pi_rVerticalExaggeration, bool pi_ClipToEndValues, Byte const* pi_pDefaultRGB, HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues)
        :FilterImplTyped<_SrcT, double>(pi_UnitSizeX, pi_UnitSizeY, pi_HillShadingSetting, pi_rVerticalExaggeration, pi_ClipToEndValues, pi_pDefaultRGB, pi_rUpperRangeValues)
        {
        }

    virtual void ProcessPixelsWithShading(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                          HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_pDestination->GetWidth() + (pi_rNeighbourhood.GetWidth()-1) == pi_pSource->GetWidth());
        HPRECONDITION(pi_pDestination->GetHeight() + (pi_rNeighbourhood.GetHeight()-1) == pi_pSource->GetHeight());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(FilterImplTyped<_SrcT, double>::m_HillShadingSettings.GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(FilterImplTyped<_SrcT, double>::m_HillShadingSettings.GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeX* pi_ScalingX);
        double RateMultiplierY = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeY* pi_ScalingY);

        for(uint32_t Line=pi_rNeighbourhood.GetYOrigin(); Line < Heigh + pi_rNeighbourhood.GetYOrigin(); ++Line)
            {
            _SrcT const* pSrcLinePre  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line-1)));
            _SrcT const* pSrcLine     =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));
            _SrcT const* pSrcLineNext =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line+1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*(Line-1)));

            if(FilterImplTyped<_SrcT, double>::m_HasNoDataValue)
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    if(E_Val == FilterImplTyped<_SrcT, double>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        continue;
                        }

                    _SrcT A = A_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? A_Val : E_Val;
                    _SrcT B = B_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? B_Val : E_Val;
                    _SrcT C = C_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? C_Val : E_Val;
                    _SrcT D = D_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? D_Val : E_Val;
                    _SrcT F = F_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? F_Val : E_Val;
                    _SrcT G = G_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? G_Val : E_Val;
                    _SrcT H = H_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? H_Val : E_Val;
                    _SrcT I = I_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? I_Val : E_Val;

                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                    double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    // Flat areas are set to the default color
                    if(RateX != 0.0 && RateY != 0.0)
                        {
                        double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, double>::m_VerticalExaggeration);
                        double Aspect_rad = ComputeAspectRad(RateX, RateY);

                        double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                        if(hillShade > 0)
                            {
                            uint32_t RGBAColor;

                            // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                            double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                            // Convert to compass direction.
                            if(Aspect_deg > 90)
                                RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(360 - Aspect_deg + 90);
                            else
                                RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(90 - Aspect_deg);

                            pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                                (Byte)(GetGreen(RGBAColor)*hillShade),
                                                                (Byte)(GetBlue(RGBAColor)*hillShade),
                                                                GetAlpha(RGBAColor));
                            }
                        else    // 100% shaded
                            {
                            pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                            }
                        }
                    else
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        }
                    }
                }
            else
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C_Val + 2*F_Val + I_Val) - (A_Val + 2*D_Val + G_Val)) * RateMultiplierX;
                    double tempRateY = ((G_Val + 2*H_Val + I_Val) - (A_Val + 2*B_Val + C_Val)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    // Flat areas are set to the default color
                    if(RateX != 0.0 && RateY != 0.0)
                        {
                        double Slope_rad  = ComputeSlopeRad(RateX, RateY, FilterImplTyped<_SrcT, double>::m_VerticalExaggeration);
                        double Aspect_rad = ComputeAspectRad(RateX, RateY);

                        double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                        if(hillShade > 0)
                            {
                            uint32_t RGBAColor;

                            // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                            double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                            // Convert to compass direction.
                            if(Aspect_deg > 90)
                                RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(360 - Aspect_deg + 90);
                            else
                                RGBAColor = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(90 - Aspect_deg);

                            pDstLineBuffer[Column-1] = MakeRGBA((Byte)(GetRed(RGBAColor)*hillShade),
                                                                (Byte)(GetGreen(RGBAColor)*hillShade),
                                                                (Byte)(GetBlue(RGBAColor)*hillShade),
                                                                GetAlpha(RGBAColor));
                            }
                        else    // 100% shaded
                            {
                            pDstLineBuffer[Column-1] = 0xFF000000;  // Opaque black.
                            }
                        }
                    else
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        }
                    }
                }
            }
        }

    virtual void ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                               HRPPixelNeighbourhood const& pi_rNeighbourhood, double pi_ScalingX, double pi_ScalingY) override
        {
        HPRECONDITION(pi_pDestination->GetWidth() + (pi_rNeighbourhood.GetWidth()-1) == pi_pSource->GetWidth());
        HPRECONDITION(pi_pDestination->GetHeight() + (pi_rNeighbourhood.GetHeight()-1) == pi_pSource->GetHeight());

        Byte const* pSrcBuffer = (Byte*)pi_pSource->GetPacket()->GetBufferAddress();
        Byte*       pDstBuffer = (Byte*)pi_pDestination->GetPacket()->GetBufferAddress();

        uint32_t Width = pi_pDestination->GetWidth();
        uint32_t Heigh = pi_pDestination->GetHeight();

        // Pre-computed variables
        double RateMultiplierX = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeX* pi_ScalingX);
        double RateMultiplierY = 1 / (8.0 * FilterImplTyped<_SrcT, double>::m_UnitSizeY* pi_ScalingY);

        for(uint32_t Line=pi_rNeighbourhood.GetYOrigin(); Line < Heigh + pi_rNeighbourhood.GetYOrigin(); ++Line)
            {
            _SrcT const* pSrcLinePre  =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line-1)));
            _SrcT const* pSrcLine     =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*Line));
            _SrcT const* pSrcLineNext =  (_SrcT*)(pSrcBuffer + (pi_pSource->GetBytesPerRow()*(Line+1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (pi_pDestination->GetBytesPerRow()*(Line-1)));

            if(FilterImplTyped<_SrcT, double>::m_HasNoDataValue)
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    if(E_Val == FilterImplTyped<_SrcT, double>::m_NoDataValue)
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        continue;
                        }

                    _SrcT A = A_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? A_Val : E_Val;
                    _SrcT B = B_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? B_Val : E_Val;
                    _SrcT C = C_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? C_Val : E_Val;
                    _SrcT D = D_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? D_Val : E_Val;
                    _SrcT F = F_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? F_Val : E_Val;
                    _SrcT G = G_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? G_Val : E_Val;
                    _SrcT H = H_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? H_Val : E_Val;
                    _SrcT I = I_Val != FilterImplTyped<_SrcT, double>::m_NoDataValue ? I_Val : E_Val;

                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                    double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    // Flat areas are set to the default color
                    if(RateX != 0.0 && RateY != 0.0)
                        {
                        // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                        double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                        // Convert to compass direction.
                        if(Aspect_deg > 90)
                            pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(360 - Aspect_deg + 90);
                        else
                            pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(90 - Aspect_deg);
                        }
                    else
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        }
                    }
                }
            else
                {
                for(uint32_t Column=pi_rNeighbourhood.GetXOrigin(); Column < Width + pi_rNeighbourhood.GetXOrigin(); ++Column)
                    {
                    // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                    // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                    double tempRateX = ((C_Val + 2*F_Val + I_Val) - (A_Val + 2*D_Val + G_Val)) * RateMultiplierX;
                    double tempRateY = ((G_Val + 2*H_Val + I_Val) - (A_Val + 2*B_Val + C_Val)) * RateMultiplierY;
                    double RateX, RateY;

                    RateX = FilterImplTyped<_SrcT, double>::m_00 * tempRateX + FilterImplTyped<_SrcT, double>::m_01 * tempRateY;
                    RateY = FilterImplTyped<_SrcT, double>::m_10 * tempRateX + FilterImplTyped<_SrcT, double>::m_11 * tempRateY;

                    // Flat areas set to the default color
                    if(RateX != 0.0 && RateY != 0.0)
                        {
                        // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                        double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                        // Convert to compass direction.
                        if(Aspect_deg > 90)
                            pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(360 - Aspect_deg + 90);
                        else
                            pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_UpperRangeValues.lower_bound(90 - Aspect_deg);
                        }
                    else
                        {
                        pDstLineBuffer[Column-1] = FilterImplTyped<_SrcT, double>::m_DefaultRGBA;
                        }
                    }
                }
            }
        }
    };

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPDEMFilter::HillShadingSettings::HillShadingSettings()
    :m_HillShadingState(false)
    {
    m_AltitudeAngle = 45;
    m_AzimuthDegree = 315;
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPDEMFilter::HillShadingSettings::HillShadingSettings(unsigned short pi_AltitudeAngle, unsigned short pi_AzimuthDegree)
    :m_AltitudeAngle(pi_AltitudeAngle),
     m_AzimuthDegree(pi_AzimuthDegree)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRPDEMFilter::HillShadingSettings::GetHillShadingState() const
    {
    return m_HillShadingState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRPDEMFilter::HillShadingSettings::SetHillShadingState(bool state)
    {
    m_HillShadingState = state;
    }


//-----------------------------------------------------------------------------
// GetAltitudeAngle
//-----------------------------------------------------------------------------
unsigned short HRPDEMFilter::HillShadingSettings::GetAltitudeAngle() const
    {
    return m_AltitudeAngle;
    }

//-----------------------------------------------------------------------------
// SetAltitudeAngle
//-----------------------------------------------------------------------------
void        HRPDEMFilter::HillShadingSettings::SetAltitudeAngle(unsigned short pi_AltitudeAngle)
    {
    m_AltitudeAngle = pi_AltitudeAngle;
    }

//-----------------------------------------------------------------------------
// GetAzimuthDegree
//-----------------------------------------------------------------------------
unsigned short HRPDEMFilter::HillShadingSettings::GetAzimuthDegree() const
    {
    return m_AzimuthDegree;
    }

//-----------------------------------------------------------------------------
// SetAzimuthDegree
//-----------------------------------------------------------------------------
void        HRPDEMFilter::HillShadingSettings::SetAzimuthDegree(unsigned short pi_AltitudeAngle)
    {
    m_AzimuthDegree = pi_AltitudeAngle;
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPDEMFilter::HRPDEMFilter(const HillShadingSettings& settings, Style pi_Style)
    :m_HillShadingSettings(settings),
     m_ClipToEndValues(false),
     m_VerticalExaggeration(1),
     m_Style(pi_Style)
    {
    m_DefaultRGBAColor[0] = m_DefaultRGBAColor[1] = m_DefaultRGBAColor[2] = 0;
    m_DefaultRGBAColor[3] = 0x00;
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPDEMFilter::HRPDEMFilter(HRPDEMFilter const& object)
    :m_HillShadingSettings(object.m_HillShadingSettings),
     m_ClipToEndValues(object.m_ClipToEndValues),
     m_VerticalExaggeration(object.m_VerticalExaggeration),
     m_Style(object.m_Style),
     m_RangeValues(object.m_RangeValues)
    {
    m_DefaultRGBAColor[0] = object.m_DefaultRGBAColor[0];
    m_DefaultRGBAColor[1] = object.m_DefaultRGBAColor[1];
    m_DefaultRGBAColor[2] = object.m_DefaultRGBAColor[2];
    m_DefaultRGBAColor[3] = object.m_DefaultRGBAColor[3];
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPDEMFilter::~HRPDEMFilter()
    {
    }

//-----------------------------------------------------------------------------
// GetCutOffMinimum
//-----------------------------------------------------------------------------
bool HRPDEMFilter::GetClipToEndValues() const
    {
    return m_ClipToEndValues;
    }

//-----------------------------------------------------------------------------
// SetCutOffMinimum
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetClipToEndValues(bool pi_ClipToEndValues)
    {
    m_ClipToEndValues = pi_ClipToEndValues;
    }


//-----------------------------------------------------------------------------
// GetHillShading
//-----------------------------------------------------------------------------
const HRPDEMFilter::HillShadingSettings& HRPDEMFilter::GetHillShadingSettings() const
    {
    return m_HillShadingSettings;
    }

//-----------------------------------------------------------------------------
// SetHillShading
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetHillShadingSettings(const HRPDEMFilter::HillShadingSettings& pi_HillShadingSettings)
    {
    m_HillShadingSettings = pi_HillShadingSettings;
    }


//-----------------------------------------------------------------------------
// GetVerticalExaggeration
//-----------------------------------------------------------------------------
unsigned short HRPDEMFilter::GetVerticalExaggeration() const
    {
    return m_VerticalExaggeration;
    }

//-----------------------------------------------------------------------------
// SetVerticalExaggeration
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetVerticalExaggeration(unsigned short pi_VerticalExaggeration)
    {
    m_VerticalExaggeration = pi_VerticalExaggeration;
    }

//-----------------------------------------------------------------------------
// GetOutputPixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType>        HRPDEMFilter::GetOutputPixelType() const
    {
    return new HRPPixelTypeV32R8G8B8A8();
    }

//-----------------------------------------------------------------------------
// GetNeighbourhood
//-----------------------------------------------------------------------------
HRPPixelNeighbourhood HRPDEMFilter::GetNeighbourhood() const
    {
    if(m_HillShadingSettings.GetHillShadingState())
        return HRPPixelNeighbourhood(3,3,1,1);

    HRPPixelNeighbourhood PixelNeighBourhood;

    switch(m_Style)
        {
        case Style_SlopePercent:
        case Style_Aspect:
            PixelNeighBourhood = HRPPixelNeighbourhood(3,3,1,1);
            break;
        case Style_Elevation:
        case Style_Unknown:
        default:
            PixelNeighBourhood = HRPPixelNeighbourhood(1,1,0,0);
            break;
        }

    return PixelNeighBourhood;
    }

//-----------------------------------------------------------------------------
// GetStyle
//-----------------------------------------------------------------------------
HRPDEMFilter::Style HRPDEMFilter::GetStyle() const
    {
    return m_Style;
    }

//-----------------------------------------------------------------------------
// SetStyle
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetStyle(HRPDEMFilter::Style pi_Style)
    {
    m_Style = pi_Style;
    }

//-----------------------------------------------------------------------------
// GetDefaultColor
//-----------------------------------------------------------------------------
const Byte* HRPDEMFilter::GetDefaultColor() const
    {
    return m_DefaultRGBAColor;
    }

//-----------------------------------------------------------------------------
// SetDefaultColor
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetDefaultColor(Byte pi_Red, Byte pi_Green, Byte pi_Blue, Byte pi_Alpha)
    {
    m_DefaultRGBAColor[0] = pi_Red;
    m_DefaultRGBAColor[1] = pi_Green;
    m_DefaultRGBAColor[2] = pi_Blue;
    m_DefaultRGBAColor[3] = pi_Alpha;
    }

//-----------------------------------------------------------------------------
// GetUpperRangeValues
//-----------------------------------------------------------------------------
HRPDEMFilter::UpperRangeValues const& HRPDEMFilter::GetUpperRangeValues() const
    {
    return m_RangeValues;
    }

//-----------------------------------------------------------------------------
// SetUpperRangeValues
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetUpperRangeValues(HRPDEMFilter::UpperRangeValues const& pi_rUpperRangeValues)
    {
    m_RangeValues = pi_rUpperRangeValues;
    }

//-----------------------------------------------------------------------------
// ProcessPixels
//-----------------------------------------------------------------------------
void HRPDEMFilter::ProcessPixels(HFCPtr<HGSMemorySurfaceDescriptor>& pi_pDestination, HFCPtr<HGSMemorySurfaceDescriptor>& pi_pSource,
                                 double pi_ScaleFactorX, double pi_ScaleFactorY)
    {
    HPRECONDITION(m_pFilterImpl != 0);

    m_pFilterImpl->ApplyFilter(pi_pDestination, pi_pSource, GetNeighbourhood(), pi_ScaleFactorX, pi_ScaleFactorY);
    }

//-----------------------------------------------------------------------------
// SetFor
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetFor(HFCPtr<HRPPixelType> const& pi_pPixelType, double pi_PixelSizeX, double pi_PixelSizeY, HFCPtr<HGF2DTransfoModel> pi_pOrientationTransfo)
    {
    m_pFilterImpl = 0;  // Clear any previously allocated implementation

    // For now, we only support single channel pixelType.
    if(pi_pPixelType->GetChannelOrg().CountChannels() == 1)
        {
        const HRPChannelType* pChannelType = pi_pPixelType->GetChannelOrg().GetChannelPtr(0);

        switch(pChannelType->GetSize())
            {
            case 8:
                switch(pChannelType->GetDataType())
                    {
                    case HRPChannelType::INT_CH:
                    case HRPChannelType::VOID_CH:
                        m_pFilterImpl = CreateFilterImplementation<uint8_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    case HRPChannelType::SINT_CH:
                        m_pFilterImpl = CreateFilterImplementation<int8_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    default:
                        break;
                    }
                break;
            case 16:
                switch(pChannelType->GetDataType())
                    {
                    case HRPChannelType::INT_CH:
                    case HRPChannelType::VOID_CH:
                        m_pFilterImpl = CreateFilterImplementation<uint16_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    case HRPChannelType::SINT_CH:
                        m_pFilterImpl = CreateFilterImplementation<int16_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    default:
                        break;
                    }
                break;
            case 32:
                switch(pChannelType->GetDataType())
                    {
                    case HRPChannelType::INT_CH:
                    case HRPChannelType::VOID_CH:
                        m_pFilterImpl = CreateFilterImplementation<uint32_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    case HRPChannelType::SINT_CH:
                        m_pFilterImpl = CreateFilterImplementation<int32_t>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    case HRPChannelType::FLOAT_CH:
                        m_pFilterImpl = CreateFilterImplementation<float>(pChannelType, pi_PixelSizeX, pi_PixelSizeY);
                        break;
                    default:
                        break;
                    }
                break;
            default:
                break;
            }
        }

    if(m_pFilterImpl == 0)
        {
        HASSERT(!"DEM Filter: Unsupported pixelType");
        m_pFilterImpl = new VoidFilterImpl(m_DefaultRGBAColor);
        }

    m_pFilterImpl->SetOrientationTransfo(pi_pOrientationTransfo);
    }

//-----------------------------------------------------------------------------
// SetFor
//-----------------------------------------------------------------------------
template<class T>
DEMFilterImplementation* HRPDEMFilter::CreateFilterImplementation(const HRPChannelType* pi_pChannelType, double pi_PixelSizeX, double pi_PixelSizeY) const
    {
    DEMFilterImplementation* pFilterImpl = 0;
    switch(m_Style)
        {
        case Style_Elevation:
            {
            pFilterImpl = new ElevationFilterImpl<T> (pi_PixelSizeX, pi_PixelSizeY, m_HillShadingSettings, m_VerticalExaggeration, m_ClipToEndValues, m_DefaultRGBAColor, m_RangeValues);
            break;
            }
        case Style_SlopePercent:
            {
            pFilterImpl = new SlopePercentFilterImpl<T> (pi_PixelSizeX, pi_PixelSizeY, m_HillShadingSettings, m_VerticalExaggeration, m_ClipToEndValues, m_DefaultRGBAColor, m_RangeValues);
            break;
            }
        case Style_Aspect:
            {
            pFilterImpl = new AspectFilterImpl<T> (pi_PixelSizeX, pi_PixelSizeY, m_HillShadingSettings, m_VerticalExaggeration, m_ClipToEndValues, m_DefaultRGBAColor, m_RangeValues);
            break;
            }
        case Style_Unknown:
        default:
            {
            HASSERT(!"BAD DEM Filter style");
            pFilterImpl = new VoidFilterImpl(m_DefaultRGBAColor);
            break;
            }
        }

    if(pi_pChannelType->GetNoDataValue())
        pFilterImpl->SetNoDataValue(*pi_pChannelType->GetNoDataValue());

    return pFilterImpl;
    }