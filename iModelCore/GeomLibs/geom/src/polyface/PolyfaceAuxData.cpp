/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceAuxData.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2018
*
*  Creates an empty PolyfaceAuxData that will be filled in by AdvanceVisitorToNextFace
+--------------------------------------------------------------------------------------*/
PolyfaceAuxDataPtr  PolyfaceAuxData::CreateForVisitor() const
    {
    bvector<int32_t>            indices;
    PolyfaceAuxData::Channels   channels;

    channels.Init(m_channels);

    return new PolyfaceAuxData(std::move(indices), std::move(channels));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::AdvanceVisitorToNextFace(PolyfaceAuxData const& parent, uint32_t i0, uint32_t numIndex, uint32_t numWrap)
    {
    m_indices.clear();

    bvector<int32_t> const&     oneBasedIndices = parent.GetIndices();
    size_t maxIndex = oneBasedIndices.size();
    size_t numOut;

    for (auto& channel : m_channels)
        for (auto& data : channel->GetData())
            data->m_values.clear();

    for (numOut = 0; numOut < numIndex && i0 + numOut < maxIndex; numOut++)
        {
        int k1 = oneBasedIndices[i0 + numOut];
        if (k1 == 0)
            break;

        uint32_t k0 = abs (k1) - 1;
        m_indices.push_back (k0);
        m_channels.AppendDataByIndex(parent.GetChannels(), k0);
        }
    if (numOut > 0)
        {
        for (uint32_t i = 0; i < numWrap; i++)
            {
            m_indices.push_back (m_indices[i]);
            m_channels.AppendDataByIndex(m_channels, i);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Transform(TransformCR transform)
    {
    RotMatrix   rMatrix = RotMatrix::From(transform), inverseRMatrix;
    double      determinant = rMatrix.Determinant ();    

    inverseRMatrix.InverseOf(rMatrix);
    for (auto& channel : m_channels)
        {
        for (auto& channelData :  channel->GetData())
            {
            float*      pData = const_cast<float*> (channelData->GetValues().data());
            size_t      dataSize = channelData->GetValues().size();

            switch (channel->GetDataType())
                {
                case DataType::Vector:
                case DataType::Convector:
                case DataType::Point:
                    {
                    for (size_t i=0; i<dataSize; i += 3)
                        {
                        DPoint3d    point = DPoint3d::From(pData[i], pData[i+1], pData[i+2]);

                        switch(channel->GetDataType())
                            {
                            case DataType::Vector:
                                rMatrix.Multiply(point);
                                break;

                            case DataType::Convector:
                                inverseRMatrix.MultiplyTranspose(point);
                                break;

                            case DataType::Point:
                                transform.Multiply(point);
                                break;
                            }
                        pData[i]   = static_cast<float> (point.x);
                        pData[i+1] = static_cast<float> (point.y);
                        pData[i+2] = static_cast<float> (point.z);
                        }
                    break;
                    }
                case DataType::Distance:
                    {
                    float transformScale = static_cast<float> (pow (fabs (determinant), 1.0 / 3.0) * (determinant >= 0.0 ? 1.0 : -1.0));

                    for (size_t i=0; i<dataSize; i++)
                        pData[i] *= transformScale;

                    break;
                    }
                }
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channels::Init(PolyfaceAuxData::ChannelsCR input)
    {
    for (auto& channel : input)
        this->push_back(channel->CloneWithoutData());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxData::ChannelPtr  PolyfaceAuxData::Channel::CloneWithoutData() const
    {
    bvector<DataPtr>    dataVector;

    for (auto& data : GetData())
        {
        bvector<float>     values;
        dataVector.push_back (new Data(data->GetInput(), std::move(values)));
        }

    return new Channel(GetDataType(), GetName().c_str(), GetInputName().c_str(), std::move(dataVector));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channels::AppendDataByIndex(PolyfaceAuxData::ChannelsCR input, size_t index)
    {
    if (empty())
        Init(input);

    for (size_t i=0; i<this->size(); i++)
        this->at(i)->AppendDataByIndex(*input.at(i), index);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channels::AppendInterpolatedData(PolyfaceAuxData::ChannelsCR input, size_t index, size_t nextIndex, double t)
    {
    if (empty())
        Init(input);

    for (size_t i=0; i<this->size(); i++)
        this->at(i)->AppendInterpolatedData(*input.at(i), index, nextIndex, t);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channel::AppendDataByIndex(PolyfaceAuxData::ChannelCR input, size_t index)
    {
    for (size_t i=0; i<this->m_data.size(); i++)  
        for (size_t k = 0, blockSize = GetBlockSize(); k<blockSize; k++)
            this->m_data.at(i)->m_values.push_back(input.m_data.at(i)->m_values.at(index * blockSize + k));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channel::AppendInterpolatedData(PolyfaceAuxData::ChannelCR input, size_t index, size_t nextIndex, double t)
    {
    for (size_t i=0; i<this->m_data.size(); i++)  
        {
        for (size_t k = 0, blockSize = GetBlockSize(); k<blockSize; k++)
            {
            auto const&     inputValues = input.m_data.at(i)->m_values;
            float           value     = inputValues[blockSize * index + k],
                            valueNext = inputValues[blockSize * nextIndex + k];

            this->m_data.at(i)->m_values.push_back(value + (float) t * (valueNext - value));
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxData::ChannelCPtr PolyfaceAuxData::GetChannel(Utf8CP name) const
    {
    for (auto& channel : m_channels)
        if (channel->GetName().Equals(name))
            return channel;

    return nullptr;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
