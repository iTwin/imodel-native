/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
//! @file PolyfaceAuxData.h  Data for auxiliary polyface data.
#include <Bentley/bvector.h>
#include <Bentley/RefCounted.h>



BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(PolyfaceAuxData);
DEFINE_REF_COUNTED_PTR(PolyfaceAuxData);

//=======================================================================================
// @bsiclass
//! Represents a single channel of auxilliary polyface data.  A channel would represent
//! either scalar data with a single value per vertex data such as temperature or stress 
//! or vector data such as displacement with 3 values per vertex.
//! Multiple instances of the channel data may be included included at varying input
//! values (such as time for an animation).
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

        //! Constructor for a single instance of channel data.
        //! @param [in] input The value of the input parameter.
        //! @param [in] values The input values at each polyface vertex. A single value
        //! per vertex for scalar data and 3 values per vertex for vector data (typically displacement).
        //! the value data is consumed so should be passed with std::move().
        Data(T_Data input, bvector<T_Data>&& values) : m_input(input), m_values(values) { }
        };

    private:

    DataType                m_dataType;
    Utf8String              m_name;             // Channel name (stress, temperature etc...)
    Utf8String              m_inputName;        // Input that may vary, time, force etc...
    bvector<DataPtr>        m_data;

    public:
        //! Constructor for a auxiliary polyface channel.
        //! @param [in] dataType the type of data
        //! @param [in] name string representing the channel name.
        //! @param [in] inputName string representing the channel input name.
        //! @param [in] data channel data.  This is consumed and should be passed with std::move().
                                T_PolyfaceAuxChannel(DataType dataType, Utf8CP name, Utf8CP inputName, bvector<DataPtr> const&& data) : m_dataType(dataType), m_name(name), m_inputName(inputName), m_data(data) { }

    DataType                    GetDataType() const         { return (DataType) m_dataType; }                                                   //! Return the channel data type.
    Utf8StringCR                GetName() const             { return m_name; }                                                                  //! Return the data name.
    Utf8StringCR                GetInputName() const        { return m_inputName; }                                                             //! Return the input name.
    bvector<DataPtr> const&     GetData() const             { return m_data; }                                                                  //! Return reference to the data.
    bool                        IsScalar() const            { return DataType::Scalar == m_dataType || DataType::Distance == m_dataType; }      //! Return true if scalar data (1 value per vertex).
    size_t                      GetValueCount() const       { return m_data.empty() ? 0 : m_data.front()->GetValueCount() / GetBlockSize(); }   //! Return the number of values.
    size_t                      GetBlockSize() const        { return m_dataType < Vector ? 1 : 3; }                                             //! Return the number of values per vertex.

        //! Append data from channel input at index.
    void AppendDataByIndex(T_PolyfaceAuxChannel const& input, size_t index)
        {
        for (size_t i=0; i<this->m_data.size(); i++)  
            for (size_t k = 0, blockSize = GetBlockSize(); k<blockSize; k++)
                this->m_data.at(i)->m_values.push_back(input.m_data.at(i)->m_values.at(index * blockSize + k));
        }
        //! Append data from channel input interpolated between index and nextIndex at value t.
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
        //! Return a cloned channel without data.
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
    DRange1d   GetValueRange() const
        {
        DRange1d        range = DRange1d::NullRange();

        for (auto& data : GetData())
            for (auto value : data->GetValues())
                range.Extend((double) value);
        
        return range;
        }
    };

using PolyfaceAuxChannel = T_PolyfaceAuxChannel<double>;

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(PolyfaceAuxChannel);
DEFINE_REF_COUNTED_PTR(PolyfaceAuxChannel);



//=======================================================================================
// @bsiclass
//! PolyfaceAuxData represents one or more channel of auxiliary, per vertex data for a 
//! polyface.  Each channel contains a scalar or vector for each vertex that may represent 
//! analysis or measured results such as temperature, stress or displacement.
//! PolyfaceAuxData may be added to an existing polyface with PolyfaceVectors::SetAuxData.
//! it is up to the caller to ensure that the Auxiliary data matches the polyface. 
//! i.e. same number of faces, vertices etc.
//! Each PolyfaceAuxData contains a single index array that represents the indices for 
//! all channels.
//=======================================================================================
struct PolyfaceAuxData : RefCountedBase
{
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Channels);

    
    //! PolyfaceAuxData::Channels represents an array of auxiliary channels for a polyface.
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

    bvector<int32_t> const&                 GetIndices() const          { return m_indices; }          //! Return the indices for all channels of the PolyfaceAuxData.
    ChannelsCR                              GetChannels() const         { return m_channels; }         //! Return constant reference to the channels.
    ChannelsR                               GetChannels()               { return m_channels; }         //! return refernce to the channels.         
    GEOMDLLIMPEXP PolyfaceAuxChannelCPtr    GetChannel(Utf8CP name) const;

                                            PolyfaceAuxData() { }                                      //! Constructor for empty PolyfaceAuxData.
                                            //! Constructor for PolyfaceAuxData.
                                            //! @param [in] indices index array for all channels.  This is consumed and should be passed with std::move().
                                            //! @param [in] channels The channel array.  This is consumed and should be passed with std::move().
                                            PolyfaceAuxData(bvector<int32_t>&& indices, Channels&& channels) : m_indices(indices), m_channels(channels) { }
    PolyfaceAuxDataPtr                      CreateForVisitor() const;                                  //! Create a new PolyfaceAuxData appropriate for use by a PolyfaceVisitor.
    void                                    AdvanceVisitorToNextFace(PolyfaceAuxDataCR parent, uint32_t i0, uint32_t numItem, uint32_t numWrap);  //! Advance PolyfaceAuxData for visitor to next face.

    GEOMDLLIMPEXP void                      Transform(TransformCR transform);                          //! Transform all channels.  This will apply scale to distance channels and transform vector channels appropriately. 
    GEOMDLLIMPEXP void                      AppendDataByIndex(ChannelsCR input, size_t index);
                  void                      AddIndexTerminator() { m_indices.push_back(0); }


};  // T_PolyfaceAuxData





END_BENTLEY_GEOMETRY_NAMESPACE
