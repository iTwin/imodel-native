/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
            data->GetValues().clear();

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
            double*     pData = const_cast<double*> (channelData->GetValues().data());
            size_t      dataSize = channelData->GetValues().size();

            switch (channel->GetDataType())
                {
                case PolyfaceAuxChannel::DataType::Vector:
                case PolyfaceAuxChannel::DataType::Covector:
                case PolyfaceAuxChannel::DataType::Point:
                    {
                    for (size_t i=0; i<dataSize; i += 3)
                        {
                        DPoint3dP    point = reinterpret_cast <DPoint3dP> (pData + i);

                        switch(channel->GetDataType())
                            {
                            case PolyfaceAuxChannel::DataType::Vector:
                                rMatrix.Multiply(*point);
                                break;

                            case PolyfaceAuxChannel::DataType::Covector:
                                inverseRMatrix.MultiplyTranspose(*point);
                                break;

                            case PolyfaceAuxChannel::DataType::Point:
                                transform.Multiply(*point);
                                break;
                            }
                        }
                    break;
                    }
                case PolyfaceAuxChannel::DataType::Distance:
                    {
                    double transformScale = static_cast<double> (pow (fabs (determinant), 1.0 / 3.0) * (determinant >= 0.0 ? 1.0 : -1.0));

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
void PolyfaceAuxData::Channels::Init(ChannelsCR input)
    {
    for (auto& channel : input)
        this->push_back(channel->CloneWithoutData());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channels::AppendDataByIndex(ChannelsCR input, size_t index)
    {
    if (empty())
        Init(input);

    for (size_t i=0; i<this->size(); i++)
        this->at(i)->AppendDataByIndex(*input.at(i), index);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::Channels::AppendInterpolatedData(ChannelsCR input, size_t index, size_t nextIndex, double t)
    {
    if (empty())
        Init(input);

    for (size_t i=0; i<this->size(); i++)
        this->at(i)->AppendInterpolatedData(*input.at(i), index, nextIndex, t);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
PolyfaceAuxChannelCPtr PolyfaceAuxData::GetChannel(Utf8CP name) const
    {
    for (auto& channel : m_channels)
        if (channel->GetName().Equals(name))
            return channel;

    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      03/2018
+--------------------------------------------------------------------------------------*/
void PolyfaceAuxData::AppendDataByIndex(ChannelsCR input, size_t index)
    {
    m_indices.push_back((int32_t) m_channels.GetValueCount()+1);
    m_channels.AppendDataByIndex(input, index);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
