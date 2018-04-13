/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/PolyfaceAuxData.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
//! @file PolyfaceAuxData.h  Data for auxilliary polyface data.
#include <Bentley/bvector.h>
#include <Bentley/RefCounted.h>



BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(PolyfaceAuxData);
DEFINE_REF_COUNTED_PTR(PolyfaceAuxData);

//=======================================================================================
// @bsiclass
//=======================================================================================
template <typename T_Data>  struct T_PolyfaceAuxChannel : RefCountedBase
    {
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Data);
    DEFINE_REF_COUNTED_PTR(Data);

    enum DataType
        {
        Scalar      = 0,
        Distance    = 1,
        Vector      = 2,
        Covector    = 3,
        Point       = 4, 
        };

    struct Data : RefCountedBase
        { 
        friend T_PolyfaceAuxChannel;

        private:
        T_Data                   m_input;
        bvector<T_Data>          m_values;

        public:
        T_Data                      GetInput() const                     { return m_input; }
        size_t                      GetValueCount() const                { return m_values.size(); }
        bvector<T_Data> const&      GetValues() const                    { return m_values; }
        bvector<T_Data>&            GetValues()                          { return m_values; }
        void                        AddValue(T_Data value)               { m_values.push_back(value); }

        Data(T_Data input, bvector<T_Data>&& values) : m_input(input), m_values(values) { }
        };

    private:

    DataType                m_dataType;
    Utf8String              m_name;             // Channel name (stress, temperature etc...)
    Utf8String              m_inputName;        // Input that may vary, time, force etc...
    bvector<DataPtr>        m_data;

    public:
                                T_PolyfaceAuxChannel(DataType dataType, Utf8CP name, Utf8CP inputName, bvector<DataPtr> const&& data) : m_dataType(dataType), m_name(name), m_inputName(inputName), m_data(data) { }

    DataType                    GetDataType() const         { return (DataType) m_dataType; }
    Utf8StringCR                GetName() const             { return m_name; }
    Utf8StringCR                GetInputName() const        { return m_inputName; }
    bvector<DataPtr> const&     GetData() const             { return m_data; }
    bool                        IsScalar() const            { return DataType::Scalar == m_dataType || DataType::Distance == m_dataType; }
    size_t                      GetValueCount() const       { return m_data.empty() ? 0 : m_data.front()->GetValueCount() / GetBlockSize(); }
    size_t                      GetBlockSize() const        { return m_dataType < Vector ? 1 : 3; }

    void AppendDataByIndex(T_PolyfaceAuxChannel const& input, size_t index)
        {
        for (size_t i=0; i<this->m_data.size(); i++)  
            for (size_t k = 0, blockSize = GetBlockSize(); k<blockSize; k++)
                this->m_data.at(i)->m_values.push_back(input.m_data.at(i)->m_values.at(index * blockSize + k));
        }

    void AppendInterpolatedData(T_PolyfaceAuxChannel const& input, size_t index, size_t nextIndex, double t)
        {
        for (size_t i=0; i<this->m_data.size(); i++)  
            {
            for (size_t k = 0, blockSize = GetBlockSize(); k<blockSize; k++)
                {
                auto const&     inputValues = input.m_data.at(i)->m_values;
                T_Data          value     = inputValues[blockSize * index + k],
                                valueNext = inputValues[blockSize * nextIndex + k];

                this->m_data.at(i)->m_values.push_back(value + (T_Data) t * (valueNext - value));
                }
            }
        }
    RefCountedPtr<T_PolyfaceAuxChannel> CloneWithoutData() const
        {
        bvector<DataPtr>    dataVector;

        for (auto& data : GetData())
            {
            bvector<T_Data>     values;
            dataVector.push_back (new Data(data->GetInput(), std::move(values)));
            }

        return new T_PolyfaceAuxChannel(GetDataType(), GetName().c_str(), GetInputName().c_str(), std::move(dataVector));
        }

    };

using PolyfaceAuxChannel = T_PolyfaceAuxChannel<double>;

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(PolyfaceAuxChannel);
DEFINE_REF_COUNTED_PTR(PolyfaceAuxChannel);



//=======================================================================================
// @bsiclass
//=======================================================================================
struct PolyfaceAuxData : RefCountedBase
{
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Channels);

    
    struct Channels : bvector<PolyfaceAuxChannelPtr> 
        {
        size_t                              GetValueCount() const  { return empty() ? 0 : front()->GetValueCount(); }
        GEOMDLLIMPEXP void                  AppendInterpolatedData(ChannelsCR input, size_t index, size_t iNext, double t);
        GEOMDLLIMPEXP void                  Init(ChannelsCR input);
        GEOMDLLIMPEXP void                  AppendDataByIndex(ChannelsCR input, size_t index);
        };

    private:
    bvector<int32_t>         m_indices;
    Channels                 m_channels; 

    public:

    bvector<int32_t> const&                 GetIndices() const          { return m_indices; }
    ChannelsCR                              GetChannels() const                      { return m_channels; }                  
    GEOMDLLIMPEXP PolyfaceAuxChannelCPtr    GetChannel(Utf8CP name) const;

                                            PolyfaceAuxData(bvector<int32_t>&& indices, Channels&& channels) : m_indices(indices), m_channels(channels) { }
    PolyfaceAuxDataPtr                      CreateForVisitor() const;
    void                                    AdvanceVisitorToNextFace(PolyfaceAuxDataCR parent, uint32_t i0, uint32_t numItem, uint32_t numWrap);

    GEOMDLLIMPEXP void                      Transform(TransformCR transform);

};  // T_PolyfaceAuxData





END_BENTLEY_GEOMETRY_NAMESPACE
