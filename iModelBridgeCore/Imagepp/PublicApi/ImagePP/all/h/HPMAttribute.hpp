//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMAttribute.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> inline HPMAttribute_T<T, ID>::HPMAttribute_T(const T& pi_rData)
    : HPMGenericAttribute(), m_Data(pi_rData)
    {
    }

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
    return (HPMAttributesID)HPMAttribute_T<T, ID>::ATTRIBUTE_ID;
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
template<class T> inline WString ExpressAsString(T const& data)
    {
    wostringstream result;
    result << data;

    return result.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> inline WString ExpressAsString(std::vector<T> const& pi_rVector)
    {
    WString result;
    for(typename vector<T>::size_type index = 0; index < pi_rVector.size()-1; ++index)
        result += ExpressAsString<T>(pi_rVector[index]) + L", "; 

    result += ExpressAsString<T>(pi_rVector[pi_rVector.size()-1]); 

    return result;
    }

/*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  08/2012
// +---------------+---------------+---------------+---------------+---------------+------*/
inline WString ExpressAsString(std::string const& data)
    {
    return WString(data.c_str(),false);
    }

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  08/2012
// +---------------+---------------+---------------+---------------+---------------+------*/
inline WString ExpressAsString(WString const& data)
    {
    return data;
    }

//-----------------------------------------------------------------------------
//operator<<
//-----------------------------------------------------------------------------
inline WString ExpressAsString(const HFCMatrix<4,4, double>& pi_rMatrix)
   {
   WString result;
   for(unsigned short i = 0 ; i < 4 ; ++i)
       {
       for(unsigned short j = 0; j < 3 ; ++j)
           ExpressAsString(pi_rMatrix[i][j]) + L", ";
           
       result += ExpressAsString(pi_rMatrix[i][3]) + L"\n";
       }

   return  result;
   }

//-----------------------------------------------------------------------------
// Get the data as a string
//-----------------------------------------------------------------------------
template<class T, HPMAttributesID ID> WString HPMAttribute_T<T, ID>::GetDataAsString() const
    {
    return ExpressAsString(m_Data);
    }

END_IMAGEPP_NAMESPACE