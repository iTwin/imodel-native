//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPDEMFilter.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRPDEMFilter.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>
#include <ImagePP/all/h/HRAImageOp.h>


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
            lastIdx = MIN(lastIdx, maxIdx);    // Fix possible rounding error
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
inline double ComputeSlopeRad(double const& pi_rRateX, double const& pi_rRateY, uint16_t const& pi_rZFactor = 1)
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
inline double ComputeZenithRad(uint16_t const& pi_AltitudeAngle)
    {
    // Altitude                                 >> The altitude of the illumination source is specified in degrees above horizontal.
    // Zenith_deg = 90 - Altitude               >> Change altitude to zenith angle
    // Zenith_rad = Zenith_deg * PI / 180.0     >> Convert to radians
    return (90 - pi_AltitudeAngle) * PI / 180.0;
    }

//-----------------------------------------------------------------------------
// ComputeAzimuthRad
//-----------------------------------------------------------------------------
inline double ComputeAzimuthRad(uint16_t const& pi_AzimuthDeg)
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
HRPDEMFilter::HillShadingSettings::HillShadingSettings(uint16_t pi_AltitudeAngle, uint16_t pi_AzimuthDegree)
    :m_HillShadingState(false),
     m_AltitudeAngle(pi_AltitudeAngle),
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
uint16_t HRPDEMFilter::HillShadingSettings::GetAltitudeAngle() const
    {
    return m_AltitudeAngle;
    }

//-----------------------------------------------------------------------------
// SetAltitudeAngle
//-----------------------------------------------------------------------------
void        HRPDEMFilter::HillShadingSettings::SetAltitudeAngle(uint16_t pi_AltitudeAngle)
    {
    m_AltitudeAngle = pi_AltitudeAngle;
    }

//-----------------------------------------------------------------------------
// GetAzimuthDegree
//-----------------------------------------------------------------------------
uint16_t HRPDEMFilter::HillShadingSettings::GetAzimuthDegree() const
    {
    return m_AzimuthDegree;
    }

//-----------------------------------------------------------------------------
// SetAzimuthDegree
//-----------------------------------------------------------------------------
void        HRPDEMFilter::HillShadingSettings::SetAzimuthDegree(uint16_t pi_AltitudeAngle)
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
uint16_t HRPDEMFilter::GetVerticalExaggeration() const
    {
    return m_VerticalExaggeration;
    }

//-----------------------------------------------------------------------------
// SetVerticalExaggeration
//-----------------------------------------------------------------------------
void HRPDEMFilter::SetVerticalExaggeration(uint16_t pi_VerticalExaggeration)
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


/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------DEMFilterProcessor_T--------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
template <class Src_T, class RangeValue_T, bool HasNoDataValue_T>
//&&Backlog: Could we make dest template? Current code use UInt32 == RGBA.
//         we probably could make it more versatile and template the associated value of a range. The ranges are already converted 
//         to the input type so we would simply need to template the destination color.
struct DEMFilterProcessor_T : public HRAImageOpDEMFilter::DEMFilterProcessor
{
public:
    typedef RangeMap<RangeValue_T, uint32_t> UpperRangeValuesT;       // <RangeValue, RGBAColor>
    
    typedef Src_T                          SourcePixelT; 
    typedef RangeValue_T                   RangeValueT;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    DEMFilterProcessor_T(HRAImageOpDEMFilter const& imgOpDEMFilter, HGF2DTransfoModel const& orientation, HRPChannelType const& channelType)
        :m_DEMImageOPRef(imgOpDEMFilter),
        m_00(1.0),
        m_01(0.0),
        m_10(0.0),
        m_11(1.0)
        {
        HPRECONDITION (orientation.CanBeRepresentedByAMatrix());

        memcpy(&m_defaultRGBA, imgOpDEMFilter.GetDefaultRGBA(), sizeof(m_defaultRGBA));

        if (HasNoDataValue_T)
            m_noDataValue = (SourcePixelT)*channelType.GetNoDataValue();
                    
        HFCMatrix<3,3> orientationMatrix(orientation.GetMatrix());
        HASSERT(orientationMatrix[0][2] == 0.0);    // No translation assumed.
        HASSERT(orientationMatrix[1][2] == 0.0);    // No translation assumed.
        m_00 = orientationMatrix[0][0];
        m_01 = orientationMatrix[0][1];
        m_10 = orientationMatrix[1][0];
        m_11 = orientationMatrix[1][1];
              
        // Build range value in RangeValueT
        HRPDEMFilter::UpperRangeValues const&  upperRangeValues = imgOpDEMFilter.GetUpperRangeValues();
        for(HRPDEMFilter::UpperRangeValues::const_iterator itr(upperRangeValues.begin()); itr != upperRangeValues.end(); ++itr)
            {
            HRPDEMFilter::RangeInfo const& rangeInfo = itr->second;

            if(rangeInfo.m_IsOn)
                {
                m_upperRangeValues.insert(typename UpperRangeValuesT::value_type((RangeValue_T)itr->first, MakeRGBA(rangeInfo.m_rgb[0], rangeInfo.m_rgb[1], rangeInfo.m_rgb[2], 0xFF)));
                }
            else
                {
                m_upperRangeValues.insert(typename UpperRangeValuesT::value_type((RangeValue_T)itr->first, m_defaultRGBA));
                }
            }

        if(imgOpDEMFilter.GetClipToEndValue())
            {
            // Everything that goes beyond the max or below the min value will be set to the default color.
            m_upperRangeValues.SetUpperDefaultValue(m_defaultRGBA);
            m_upperRangeValues.SetLowerDefaultValue(m_defaultRGBA);
            }

        m_upperRangeValues.update_hash();
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~DEMFilterProcessor_T(){}

protected:


    HRPDEMFilter::HillShadingSettings const& GetHillShadingSettings() const {return m_DEMImageOPRef.GetHillShadingSettings();}
    double GetUnitSizeX() const {return m_DEMImageOPRef.GetUnitSizeX();}
    double GetUnitSizeY() const {return m_DEMImageOPRef.GetUnitSizeY();}
    uint16_t GetVerticalExaggeration() const {return m_DEMImageOPRef.GetVerticalExaggeration();}
    HRPPixelNeighbourhood const& GetNeighbourhood() const {return m_DEMImageOPRef.GetNeighbourhood();}
    

    HRAImageOpDEMFilter const& m_DEMImageOPRef;     // A ref to do filter.
    
    double              m_00;       // Transform pieces use when shading.
    double              m_01;
    double              m_10;
    double              m_11;
    uint32_t            m_defaultRGBA;
    UpperRangeValuesT   m_upperRangeValues;         
    SourcePixelT        m_noDataValue;              // valid only when HasNoDataValue_T=true

};

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------ElevationFilterProcessorT---------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
template <class Src_T, bool HasNoDataValue_T>
struct ElevationFilterProcessorT : public DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>
    {
    typedef Src_T                          SourcePixelT;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElevationFilterProcessorT(HRAImageOpDEMFilter const& imgOpDEMFilter, HGF2DTransfoModel const& orientation, HRPChannelType const& channelType)
        :DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>(imgOpDEMFilter, orientation, channelType)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~ElevationFilterProcessorT(){}

    virtual void _ProcessPixelsWithShading(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
        HPRECONDITION(outData.GetWidth() + (DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetNeighbourhood().GetWidth() - 1) == inputData.GetWidth());
        HPRECONDITION(outData.GetHeight() + (DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetNeighbourhood().GetHeight() - 1) == inputData.GetHeight());
#ifndef DISABLE_BUFFER_GETDATA

        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetHillShadingSettings().GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetHillShadingSettings().GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetUnitSizeX() * scalingX);
        double RateMultiplierY = 1 / (8.0 * DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetUnitSizeY() * scalingY);

        HRPPixelNeighbourhood const& neighbourhood = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetNeighbourhood();

        uint16_t verticalExaggeration = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::GetVerticalExaggeration();

        for(uint32_t Line=neighbourhood.GetYOrigin(); Line < Heigh + neighbourhood.GetYOrigin(); ++Line)
            {
            SourcePixelT const* pSrcLinePre = (SourcePixelT const*)(pSrcBuffer + (inPitch*(Line - 1)));
            SourcePixelT const* pSrcLine     = (SourcePixelT const*)(pSrcBuffer + (inPitch*Line));
            SourcePixelT const* pSrcLineNext = (SourcePixelT const*)(pSrcBuffer + (inPitch*(Line+1)));
            uint32_t*     pDstLineBuffer       = (uint32_t*)(pDstBuffer + (outPitch*(Line-1)));

            for(uint32_t Column=neighbourhood.GetXOrigin(); Column < Width + neighbourhood.GetXOrigin(); ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && E_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_defaultRGBA;
                    continue;
                    }

                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                SourcePixelT A = (HasNoDataValue_T && A_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : A_Val;
                SourcePixelT B = (HasNoDataValue_T && B_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : B_Val;
                SourcePixelT C = (HasNoDataValue_T && C_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : C_Val;
                SourcePixelT D = (HasNoDataValue_T && D_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : D_Val;
                SourcePixelT F = (HasNoDataValue_T && F_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : F_Val;
                SourcePixelT G = (HasNoDataValue_T && G_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : G_Val;
                SourcePixelT H = (HasNoDataValue_T && H_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : H_Val;
                SourcePixelT I = (HasNoDataValue_T && I_Val == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue) ? E_Val : I_Val;

                // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                double RateX, RateY;

                RateX = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_00 * tempRateX + DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_01 * tempRateY;
                RateY = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_10 * tempRateX + DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_11 * tempRateY;

                double Slope_rad  = ComputeSlopeRad(RateX, RateY, verticalExaggeration);
                double Aspect_rad = ComputeAspectRad(RateX, RateY);

                double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                if(hillShade > 0)
                    {
                    uint32_t const RGBAColor = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_upperRangeValues.lower_bound(E_Val);
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
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        for(uint32_t Line=0; Line < Heigh; ++Line)
            {
            SourcePixelT const* pSrcLine  =  (SourcePixelT const*)(pSrcBuffer + (inPitch*Line));

            uint32_t*  pDstLineBuffer = (uint32_t*)(pDstBuffer + (outPitch*Line));  // RGBA

            for(uint32_t Column=0; Column < Width; ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && pSrcLine[Column] == DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column] = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_defaultRGBA;
                    }
                else
                    {
                    pDstLineBuffer[Column] = DEMFilterProcessor_T<Src_T, Src_T, HasNoDataValue_T>::m_upperRangeValues.lower_bound(E_Val);
                    }
                }
            }
#endif
        }
    };

//-----------------------------------------------------------------------------
// SlopePercentFilterProcessorT
//-----------------------------------------------------------------------------
template<class _SrcT, bool HasNoDataValue_T>
class SlopePercentFilterProcessorT : public DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>
    {
    typedef _SrcT SourcePixelT;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    SlopePercentFilterProcessorT(HRAImageOpDEMFilter const& imgOpDEMFilter, HGF2DTransfoModel const& orientation, HRPChannelType const& channelType)
        :DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>(imgOpDEMFilter, orientation, channelType)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~SlopePercentFilterProcessorT(){}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ProcessPixelsWithShading(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        HPRECONDITION(outData.GetWidth() + (DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood().GetWidth() - 1) == inputData.GetWidth());
        HPRECONDITION(outData.GetHeight() + (DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood().GetHeight() - 1) == inputData.GetHeight());

        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetHillShadingSettings().GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetHillShadingSettings().GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeX() * scalingX);
        double RateMultiplierY = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeY() * scalingY);

        HRPPixelNeighbourhood const& neighbourhood = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood();

        uint16_t verticalExaggeration = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetVerticalExaggeration();

        for(uint32_t Line=neighbourhood.GetYOrigin(); Line < Heigh + neighbourhood.GetYOrigin(); ++Line)
            {
            SourcePixelT const* pSrcLinePre  = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line-1)));
            SourcePixelT const* pSrcLine     = (SourcePixelT*)(pSrcBuffer + (inPitch*Line));
            SourcePixelT const* pSrcLineNext = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line+1)));
            uint32_t*     pDstLineBuffer       = (uint32_t*)(pDstBuffer + (outPitch*(Line-1)));

            for(uint32_t Column=neighbourhood.GetXOrigin(); Column < Width + neighbourhood.GetXOrigin(); ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && E_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    continue;
                    }

                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                SourcePixelT A = (HasNoDataValue_T && A_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : A_Val;
                SourcePixelT B = (HasNoDataValue_T && B_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : B_Val;
                SourcePixelT C = (HasNoDataValue_T && C_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : C_Val;
                SourcePixelT D = (HasNoDataValue_T && D_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : D_Val;
                SourcePixelT F = (HasNoDataValue_T && F_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : F_Val;
                SourcePixelT G = (HasNoDataValue_T && G_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : G_Val;
                SourcePixelT H = (HasNoDataValue_T && H_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : H_Val;
                SourcePixelT I = (HasNoDataValue_T && I_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : I_Val;

                // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                double RateX, RateY;

                RateX = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_00 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_01 * tempRateY;
                RateY = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_10 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_11 * tempRateY;

                double Slope_rad  = ComputeSlopeRad(RateX, RateY, verticalExaggeration);
                double Aspect_rad = ComputeAspectRad(RateX, RateY);

                double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                if(hillShade > 0)
                    {
                    // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                    double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                    uint32_t const RGBAColor = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(slopePercent);
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
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        // Pre-computed variables
        double RateMultiplierX = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeX() * scalingX);
        double RateMultiplierY = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeY() * scalingY);

        HRPPixelNeighbourhood const& neighbourhood = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood();

        for(uint32_t Line=neighbourhood.GetYOrigin(); Line < Heigh + neighbourhood.GetYOrigin(); ++Line)
            {
            SourcePixelT const* pSrcLinePre  = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line-1)));
            SourcePixelT const* pSrcLine     = (SourcePixelT*)(pSrcBuffer + (inPitch*Line));
            SourcePixelT const* pSrcLineNext = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line+1)));
            uint32_t*     pDstLineBuffer       = (uint32_t*)(pDstBuffer + (outPitch*(Line-1)));

            for(uint32_t Column=neighbourhood.GetXOrigin(); Column < Width + neighbourhood.GetXOrigin(); ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && E_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    continue;
                    }

                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                SourcePixelT A = (HasNoDataValue_T && A_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : A_Val;
                SourcePixelT B = (HasNoDataValue_T && B_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : B_Val;
                SourcePixelT C = (HasNoDataValue_T && C_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : C_Val;
                SourcePixelT D = (HasNoDataValue_T && D_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : D_Val;
                SourcePixelT F = (HasNoDataValue_T && F_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : F_Val;
                SourcePixelT G = (HasNoDataValue_T && G_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : G_Val;
                SourcePixelT H = (HasNoDataValue_T && H_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : H_Val;
                SourcePixelT I = (HasNoDataValue_T && I_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : I_Val;

                // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                double RateX = ((C + 2 * F + I) - (A + 2 * D + G)) * RateMultiplierX;
                double RateY = ((G + 2 * H + I) - (A + 2 * B + C)) * RateMultiplierY;

                // *** Does not require to apply orientation transform.

                // rise_run = ( (dz/dx)^2 + (dz/dy)^2 )^0.5
                double slopePercent = sqrt((RateX*RateX) + (RateY*RateY)) * 100;

                pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(slopePercent);
                }
            }
#endif
        }
    };

//-----------------------------------------------------------------------------
// AspectFilterProcessorT
//-----------------------------------------------------------------------------
template<class _SrcT, bool HasNoDataValue_T>
class AspectFilterProcessorT : public DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>
    {
public:
    typedef _SrcT SourcePixelT;
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    AspectFilterProcessorT(HRAImageOpDEMFilter const& imgOpDEMFilter, HGF2DTransfoModel const& orientation, HRPChannelType const& channelType)
        :DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>(imgOpDEMFilter, orientation, channelType)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~AspectFilterProcessorT(){}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ProcessPixelsWithShading(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        HPRECONDITION(outData.GetWidth() + (DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood().GetWidth() - 1) == inputData.GetWidth());
        HPRECONDITION(outData.GetHeight() + (DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood().GetHeight() - 1) == inputData.GetHeight());

        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        // Pre-computed variables
        double Zenith_rad = ComputeZenithRad(DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetHillShadingSettings().GetAltitudeAngle());
        double Zenith_cos = cos(Zenith_rad);
        double Zenith_sin = sin(Zenith_rad);
        double Azimuth_rad = ComputeAzimuthRad(DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetHillShadingSettings().GetAzimuthDegree());
        double RateMultiplierX = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeX() * scalingX);
        double RateMultiplierY = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeY() * scalingY);

        HRPPixelNeighbourhood const& neighbourhood = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood();

        uint16_t verticalExaggeration = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetVerticalExaggeration();

        for(uint32_t Line=neighbourhood.GetYOrigin(); Line < Heigh + neighbourhood.GetYOrigin(); ++Line)
            {
            SourcePixelT const* pSrcLinePre  = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line-1)));
            SourcePixelT const* pSrcLine     = (SourcePixelT*)(pSrcBuffer + (inPitch*Line));
            SourcePixelT const* pSrcLineNext = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line+1)));
            uint32_t*     pDstLineBuffer       = (uint32_t*)(pDstBuffer + (outPitch*(Line-1)));

            for(uint32_t Column=neighbourhood.GetXOrigin(); Column < Width + neighbourhood.GetXOrigin(); ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && E_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    continue;
                    }

                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                SourcePixelT A = (HasNoDataValue_T && A_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : A_Val;
                SourcePixelT B = (HasNoDataValue_T && B_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : B_Val;
                SourcePixelT C = (HasNoDataValue_T && C_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : C_Val;
                SourcePixelT D = (HasNoDataValue_T && D_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : D_Val;
                SourcePixelT F = (HasNoDataValue_T && F_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : F_Val;
                SourcePixelT G = (HasNoDataValue_T && G_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : G_Val;
                SourcePixelT H = (HasNoDataValue_T && H_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : H_Val;
                SourcePixelT I = (HasNoDataValue_T && I_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : I_Val;

                // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                double RateX, RateY;

                RateX = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_00 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_01 * tempRateY;
                RateY = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_10 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_11 * tempRateY;

                // Flat areas are set to the default color
                if(RateX != 0.0 && RateY != 0.0)
                    {
                    double Slope_rad  = ComputeSlopeRad(RateX, RateY, verticalExaggeration);
                    double Aspect_rad = ComputeAspectRad(RateX, RateY);

                    double hillShade = (Zenith_cos * cos(Slope_rad)) + (Zenith_sin * sin(Slope_rad) * cos(Azimuth_rad-Aspect_rad));
                    if(hillShade > 0)
                        {
                        uint32_t RGBAColor;

                        // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                        double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                        // Convert to compass direction.
                        if(Aspect_deg > 90)
                            RGBAColor = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(360 - Aspect_deg + 90);
                        else
                            RGBAColor = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(90 - Aspect_deg);

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
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    }
                }
            }
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ProcessPixels(HRAImageSampleR outData, HRAImageSampleCR inputData, double scalingX, double scalingY) override
        {
#ifndef DISABLE_BUFFER_GETDATA
        size_t inPitch, outPitch;
        Byte const* pSrcBuffer = inputData.GetBufferCP()->GetDataCP(inPitch);
        Byte*       pDstBuffer = outData.GetBufferP()->GetDataP(outPitch);

        uint32_t Width = outData.GetWidth();
        uint32_t Heigh = outData.GetHeight();

        // Pre-computed variables
        double RateMultiplierX = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeX() * scalingX);
        double RateMultiplierY = 1 / (8.0 * DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetUnitSizeY() * scalingY);

        HRPPixelNeighbourhood const& neighbourhood = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::GetNeighbourhood();

        for(uint32_t Line=neighbourhood.GetYOrigin(); Line < Heigh + neighbourhood.GetYOrigin(); ++Line)
            {
            SourcePixelT const* pSrcLinePre = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line - 1)));
            SourcePixelT const* pSrcLine = (SourcePixelT*)(pSrcBuffer + (inPitch*Line));
            SourcePixelT const* pSrcLineNext = (SourcePixelT*)(pSrcBuffer + (inPitch*(Line + 1)));
            uint32_t*     pDstLineBuffer = (uint32_t*)(pDstBuffer + (outPitch*(Line-1)));

            for(uint32_t Column=neighbourhood.GetXOrigin(); Column < Width + neighbourhood.GetXOrigin(); ++Column)
                {
                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                if (HasNoDataValue_T && E_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue)
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    continue;
                    }

                // HasNoDataValue_T resolves to a constant, compiler will optimize it.
                SourcePixelT A = (HasNoDataValue_T && A_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : A_Val;
                SourcePixelT B = (HasNoDataValue_T && B_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : B_Val;
                SourcePixelT C = (HasNoDataValue_T && C_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : C_Val;
                SourcePixelT D = (HasNoDataValue_T && D_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : D_Val;
                SourcePixelT F = (HasNoDataValue_T && F_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : F_Val;
                SourcePixelT G = (HasNoDataValue_T && G_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : G_Val;
                SourcePixelT H = (HasNoDataValue_T && H_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : H_Val;
                SourcePixelT I = (HasNoDataValue_T && I_Val == DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_noDataValue) ? E_Val : I_Val;

                // dz/dx = ((c + 2f + i) - (a + 2d + g)) / 8 * x_cell_size)
                // dz/dy = ((g + 2h + i) - (a + 2b + c)) / 8 * y_cell_size)
                double tempRateX = ((C + 2*F + I) - (A + 2*D + G)) * RateMultiplierX;
                double tempRateY = ((G + 2*H + I) - (A + 2*B + C)) * RateMultiplierY;
                double RateX, RateY;

                RateX = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_00 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_01 * tempRateY;
                RateY = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_10 * tempRateX + DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_11 * tempRateY;

                // Flat areas are set to the default color
                if(RateX != 0.0 && RateY != 0.0)
                    {
                    // Aspect_rad = atan2( (dz/dx), -(dz/dx) )
                    double Aspect_deg = 57.29578 * atan2(RateY, -RateX);

                    // Convert to compass direction.
                    if(Aspect_deg > 90)
                        pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(360 - Aspect_deg + 90);
                    else
                        pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_upperRangeValues.lower_bound(90 - Aspect_deg);
                    }
                else
                    {
                    pDstLineBuffer[Column - 1] = DEMFilterProcessor_T<_SrcT, double, HasNoDataValue_T>::m_defaultRGBA;
                    }
                }
            }
#endif
        }
    };

/*-------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------HRAImageOpDEMFilter---------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDEMFilterPtr HRAImageOpDEMFilter::CreateDEMFilter(HRPDEMFilter::Style style, HRPDEMFilter::UpperRangeValues const& upperRangeValues,
                                                   double unitSizeX, double unitSizeY, HGF2DTransfoModel const& orientation)
    {
    return new HRAImageOpDEMFilter(style, upperRangeValues, unitSizeX, unitSizeY, orientation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDEMFilter::HRAImageOpDEMFilter(HRPDEMFilter::Style style, HRPDEMFilter::UpperRangeValues const& upperRangeValues,
                                         double unitSizeX, double unitSizeY, HGF2DTransfoModel const& orientation)
    :HRAImageOp(),
    m_style(style),
    m_defaultRGBA(0xff000000),       // Opaque black.
    m_upperRangeValues(upperRangeValues),
    m_pDEMFilterProcessor(),
    m_pOrientationTransfo(orientation.Clone()),
    m_verticalExaggeration(1),
    m_hillShadingSettings(),
    m_clipToEndValue(false),
    m_unitSizeX(unitSizeX),
    m_unitSizeY(unitSizeY)
    {
    UpdatePixelNeighbourhood();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDEMFilter::~HRAImageOpDEMFilter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* HRAImageOpDEMFilter::GetDefaultRGBA() const             {return (Byte*)&m_defaultRGBA;}
void        HRAImageOpDEMFilter::SetDefaultRGBA(Byte const* rgba)   {memcpy(&m_defaultRGBA, rgba, sizeof(m_defaultRGBA));}
uint16_t HRAImageOpDEMFilter::GetVerticalExaggeration() const                 {return m_verticalExaggeration;}
void        HRAImageOpDEMFilter::SetVerticalExaggeration(uint16_t newExaggeration) {m_verticalExaggeration=newExaggeration;}
bool        HRAImageOpDEMFilter::GetClipToEndValue() const          {return m_clipToEndValue;}
void        HRAImageOpDEMFilter::SetClipToEndValue(bool clipToEnd)  {m_clipToEndValue=clipToEnd;}
double      HRAImageOpDEMFilter::GetUnitSizeX() const               {return m_unitSizeX;}
double      HRAImageOpDEMFilter::GetUnitSizeY() const               {return m_unitSizeY;}
HRPDEMFilter::HillShadingSettings const&  HRAImageOpDEMFilter::GetHillShadingSettings() const {return m_hillShadingSettings;}
HRPDEMFilter::UpperRangeValues const& HRAImageOpDEMFilter::GetUpperRangeValues() const {return m_upperRangeValues;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpDEMFilter::UpdatePixelNeighbourhood()
    {
    if(GetHillShadingSettings().GetHillShadingState())
        {
        m_pPixelNeighbourhood = new HRPPixelNeighbourhood(3,3,1,1);
        return;
        }

    switch(m_style)
        {
        case HRPDEMFilter::Style_SlopePercent:
        case HRPDEMFilter::Style_Aspect:
            m_pPixelNeighbourhood = new HRPPixelNeighbourhood(3,3,1,1);
            break;
        case HRPDEMFilter::Style_Elevation:
            m_pPixelNeighbourhood = new HRPPixelNeighbourhood(1,1,0,0);
            break;
        case HRPDEMFilter::Style_Unknown:
        default:
            HASSERT(!"Unknown DEM filter style: HRAImageOpDEMFilter::UpdatePixelNeighbourhood");
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void        HRAImageOpDEMFilter::SetHillShadingSettings(HRPDEMFilter::HillShadingSettings const& pi_HillShading)
    {
    m_hillShadingSettings = pi_HillShading;
    UpdatePixelNeighbourhood();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::_GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    return IMAGEOP_STATUS_NoMorePixelType;      // We have no default.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::_GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch)
    {
    if(index > 0)
        return IMAGEOP_STATUS_NoMorePixelType;      

    pixelType = new HRPPixelTypeV32R8G8B8A8();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::_SetInputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pInputPixelType = NULL;
        UpdateProcessor();
        return IMAGEPP_STATUS_Success;
        }

    // For now, we only support single channel pixelType.
    if(pixelType->GetChannelOrg().CountChannels() != 1 || pixelType->CountValueBits() % 8 != 0)
        return IMAGEOP_STATUS_InvalidPixelType;

    m_pInputPixelType = pixelType;

    UpdateProcessor();
    
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::_SetOutputPixelType(HFCPtr<HRPPixelType> pixelType)
    {
    if(pixelType == NULL)
        {
        m_pOutputPixelType = NULL;
        UpdateProcessor();
        return IMAGEPP_STATUS_Success;
        }

    // DEM filter always output
    if(!pixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        return IMAGEOP_STATUS_InvalidPixelType;
     
    m_pOutputPixelType = pixelType;

    UpdateProcessor();

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::_Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params)
    {
    //&&OPTIMIZATION: need to do this once once and not for every sample we do. Maybe it could be part of ImageOpParams?
    // Compute scaling between source and destination
    double            ScaleFactorX;
    double            ScaleFactorY;
    HGF2DDisplacement  Displacement;
    params.GetTransfoModel().GetStretchParams(&ScaleFactorX, &ScaleFactorY, &Displacement);

    if (GetHillShadingSettings().GetHillShadingState())
        m_pDEMFilterProcessor->_ProcessPixelsWithShading(out, inputData, ScaleFactorX, ScaleFactorY);
    else    
        m_pDEMFilterProcessor->_ProcessPixels(out, inputData, ScaleFactorX, ScaleFactorY);

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Src_T>
HRAImageOpDEMFilter::DEMFilterProcessor* HRAImageOpDEMFilter::CreateProcessor(HRPChannelType const& srcChannelType)
    {
    DEMFilterProcessor* pProcessor = NULL;

    switch(m_style)
        {
        case HRPDEMFilter::Style_Elevation:
            if (srcChannelType.GetNoDataValue() != NULL)
                pProcessor = new ElevationFilterProcessorT<Src_T, true>(*this, *m_pOrientationTransfo, srcChannelType);
            else
                pProcessor = new ElevationFilterProcessorT<Src_T, false>(*this, *m_pOrientationTransfo, srcChannelType);            
            break;
        case HRPDEMFilter::Style_SlopePercent:
            if (srcChannelType.GetNoDataValue() != NULL)
                pProcessor = new SlopePercentFilterProcessorT<Src_T, true>(*this, *m_pOrientationTransfo, srcChannelType);
            else
                pProcessor = new SlopePercentFilterProcessorT<Src_T, false>(*this, *m_pOrientationTransfo, srcChannelType);
            break;
        case HRPDEMFilter::Style_Aspect:      
            if (srcChannelType.GetNoDataValue() != NULL)
                pProcessor = new AspectFilterProcessorT<Src_T, true>(*this, *m_pOrientationTransfo, srcChannelType);
            else
                pProcessor = new AspectFilterProcessorT<Src_T, false>(*this, *m_pOrientationTransfo, srcChannelType);
            break;
        case HRPDEMFilter::Style_Unknown:
        default:
            break;
        }

    return pProcessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageOpDEMFilter::UpdateProcessor()
    {
    // Reset processor in any case; even if inputPixelType or outputPixelType is NULL
    m_pDEMFilterProcessor.reset();

    if(GetInputPixelType() == NULL || GetOutputPixelType() == NULL)
        return IMAGEPP_STATUS_UnknownError;     // Not ready.

    const HRPChannelType* pChannelType = GetInputPixelType()->GetChannelOrg().GetChannelPtr(0);

    switch(pChannelType->GetSize())
        {
        case 8:
            switch(pChannelType->GetDataType())
                {
                case HRPChannelType::INT_CH:
                case HRPChannelType::VOID_CH:
                    m_pDEMFilterProcessor.reset(CreateProcessor<uint8_t>(*pChannelType));
                    break;
                case HRPChannelType::SINT_CH:
                    m_pDEMFilterProcessor.reset(CreateProcessor<int8_t>(*pChannelType));
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
                    m_pDEMFilterProcessor.reset(CreateProcessor<uint16_t>(*pChannelType));
                    break;
                case HRPChannelType::SINT_CH:
                    m_pDEMFilterProcessor.reset(CreateProcessor<int16_t>(*pChannelType));
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
                    m_pDEMFilterProcessor.reset(CreateProcessor<uint32_t>(*pChannelType));
                    break;
                case HRPChannelType::SINT_CH:
                    m_pDEMFilterProcessor.reset(CreateProcessor<int32_t>(*pChannelType));
                    break;
                case HRPChannelType::FLOAT_CH:
                    m_pDEMFilterProcessor.reset(CreateProcessor<float>(*pChannelType));
                    break;
                default:
                    break;
                }
            break;
        default:
            break;
        }

    return m_pDEMFilterProcessor.get() != NULL ? IMAGEPP_STATUS_Success : IMAGEPP_STATUS_UnknownError;
    }
