//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// HPMAttribute inline methods 
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Test if object refers to same attribute as parameter
//-----------------------------------------------------------------------------
inline bool HPMGenericAttribute::SameAttributeAs(HPMGenericAttribute const& pi_attribute)
    {
    return pi_attribute.GetID() == GetID();
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline HPMAttribute_T<T, ID>::HPMAttribute_T()
    : HPMGenericAttribute()
    {}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline HPMAttribute_T<T, ID>::HPMAttribute_T(const T& pi_rData)
    : HPMGenericAttribute(), m_Data(pi_rData)
    {}

//-----------------------------------------------------------------------------
// Retrieve the data
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline const T& HPMAttribute_T<T, ID>::GetData() const
    {
    return m_Data;
    }

//-----------------------------------------------------------------------------
// Set the data
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline void HPMAttribute_T<T, ID>::SetData(const T& pi_rData)
    {
    m_Data = pi_rData;
    }

//-----------------------------------------------------------------------------
// Get the ID                                          
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline HPMAttributesID HPMAttribute_T<T, ID>::GetID() const
    {
    return (HPMAttributesID) HPMAttribute_T<T, ID>::ATTRIBUTE_ID;
    }

//-----------------------------------------------------------------------------
// Clone
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> HPMGenericAttribute* HPMAttribute_T<T, ID>::Clone() const
    {
    return new HPMAttribute_T<T, ID>(m_Data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> inline Utf8String ExpressAsString(T const& data)
    {
    stringstream result;
    result << data;

    return result.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<> inline Utf8String ExpressAsString<unsigned char>(unsigned char const& data)
    {
    stringstream result;
    // Need to cast to integer to avoid a string conversion in ostream.
    result << (uint32_t) data;

    return result.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> inline Utf8String ExpressAsString(std::vector<T> const& pi_rVector)
    {
    Utf8String result;
    for (typename vector<T>::size_type index = 0; index < pi_rVector.size() - 1; ++index)
        result += ExpressAsString<T>(pi_rVector[index]) + ", ";

    result += ExpressAsString<T>(pi_rVector[pi_rVector.size() - 1]);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  08/2012
// +---------------+---------------+---------------+---------------+---------------+------*/
inline Utf8String ExpressAsString(std::string const& data)
    {
    return Utf8String(data.c_str());
    }

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  08/2012
// +---------------+---------------+---------------+---------------+---------------+------*/
inline Utf8String ExpressAsString(Utf8String const& data)
    {
    return data;
    }

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  08/2012
// +---------------+---------------+---------------+---------------+---------------+------*/
inline Utf8String ExpressAsString(WString const& data)
    {
    return Utf8String(data);
    }

//-----------------------------------------------------------------------------
//operator<<
//-----------------------------------------------------------------------------
inline Utf8String ExpressAsString(const HFCMatrix<4, 4, double>& pi_rMatrix)
    {
    Utf8String result;
    for (uint16_t i = 0; i < 4; ++i)
        {
        for (uint16_t j = 0; j < 3; ++j)
            ExpressAsString(pi_rMatrix[i][j]) + ", ";

        result += ExpressAsString(pi_rMatrix[i][3]) + "\n";
        }

    return  result;
    }

//-----------------------------------------------------------------------------
// Get the data as a string
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> Utf8String HPMAttribute_T<T, ID>::GetDataAsString() const
    {
    return ExpressAsString(m_Data);
    }

END_IMAGEPP_NAMESPACE