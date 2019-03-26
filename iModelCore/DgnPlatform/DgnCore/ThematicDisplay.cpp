/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ThematicDisplay.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#include <DgnPlatform/RenderPrimitives.h>

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct  ThematicCookedRange 
 {
    double                              m_minimum;
    double                              m_delta;

                    ThematicCookedRange ()                                      { m_minimum = 0.0; m_delta = 1.0; }
                    ThematicCookedRange(DRange1dCR range)                       { Set(range.low, range.high); }
    void            Set (double min, double max)                                { m_minimum = min, m_delta =  (max > min) ? (max - min) : 1.0; }
    inline double   GetNormalizedValueFromRaw (double value)  const             { return (value - m_minimum) / (m_delta); }
    inline double   GetRawValueFromNormalized (double normalizedValue) const    { return m_minimum + normalizedValue * (m_delta); }

 };  // ThematicCookedRange


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint2d    computeTextureParam (double value, double margin)
    {       
    static float s_paramEpsilon = .00002; 
    double minMargin = margin, maxMargin = 1.0 - margin;
    double minLimit = -margin + s_paramEpsilon, maxlimit = 1.0 + minMargin - s_paramEpsilon;
    double colorRange = 1.0 - 2.0 * margin;
    double textureValue = margin + colorRange * value; 
                                  
    if (textureValue < minLimit)
        textureValue = minLimit;
    else if (textureValue > maxlimit)
        textureValue = maxlimit;
    else if (fabs (textureValue - minMargin) < s_paramEpsilon)
        textureValue += s_paramEpsilon;
    else if (fabs (textureValue - maxMargin) < s_paramEpsilon)
        textureValue -= s_paramEpsilon;
    
    FPoint2d    point = {.5f, static_cast<float>(textureValue) };

    return point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GradientSymbCPtr getThematicGradient(Render::Primitives::DisplayParamsCR displayParams, PolyfaceAuxChannelCR channel)
    {
    GradientSymbCPtr    gradient = displayParams.GetSurfaceMaterial().GetGradient();

    if (gradient.IsValid() && gradient->GetThematicSettings().IsValid())
        return gradient;

    auto thematicSettings  = new ThematicGradientSettings(channel.GetValueRange());

    return new GradientSymb(*thematicSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void  Render::Primitives::ThematicMeshBuilder::InitThematicDisplay(PolyfaceHeaderR mesh, Render::Primitives::DisplayParamsCR displayParams)
    {
    // TBD -- Active channel selection - for now any scalar.

    if (mesh.HasFacets() && mesh.GetAuxDataCP().IsValid())
        {
        for (auto& channel : mesh.GetAuxDataCP()->GetChannels())    
            {
            if (channel->IsScalar())
                {
                GradientSymbCPtr        gradient = getThematicGradient(displayParams, *channel);

                // Add parameters from initial entry.   Texture will not be published correctly without parameters.
                mesh.ParamIndex().clear();
                mesh.Param().clear();

                mesh.ParamIndex().SetActive(true);
                mesh.ParamIndex().resize(mesh.PointIndex().size());
                memcpy (mesh.ParamIndex().data(), mesh.GetAuxDataCP()->GetIndices().data(), mesh.PointIndex().size() * sizeof(int32_t));

                auto& values = channel->GetData().at(1)->GetValues(); 

                mesh.Param().reserve(values.size());
                mesh.Param().SetActive(true);

                ThematicCookedRange  cookedRange(gradient->GetThematicSettings()->GetRange());

                for (auto value : values)
                    mesh.Param().push_back(DPoint2d::From(computeTextureParam(cookedRange.GetNormalizedValueFromRaw((double) value), gradient->GetThematicSettings()->GetMargin())));
                 }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void   Render::Primitives::ThematicMeshBuilder::BuildMeshAuxData(MeshAuxDataR auxData, PolyfaceQueryCR mesh, Render::Primitives::DisplayParamsCR displayParams)
    {
    // TBD -- Active channel selection - for now use first scalar or displacement.
    if (!mesh.GetAuxDataCP().IsValid())
        return;

    for (auto& channel : mesh.GetAuxDataCP()->GetChannels())    
        {
        switch(channel->GetDataType())
            {
            case PolyfaceAuxChannel::Distance:
            case PolyfaceAuxChannel::Scalar:
                {
                if (auxData.m_paramChannel.IsValid())
                    break;              // More than one scalar channel -- just use first for now.

                auto&                               inDataVector = channel->GetData();
                bvector<AuxParamChannel::DataPtr>   outDataVector;
                GradientSymbCPtr                    gradient = getThematicGradient(displayParams, *channel);
                ThematicCookedRange                 cookedRange(gradient->GetThematicSettings()->GetRange());

                for (auto inData : inDataVector)
                    {
                    bvector<FPoint2d>   params;

                    for (auto& value : inData->GetValues())
                        params.push_back(computeTextureParam(cookedRange.GetNormalizedValueFromRaw(value),  gradient->GetThematicSettings()->GetMargin()));
                    
                    outDataVector.push_back(new AuxParamChannel::Data((float) inData->GetInput(), std::move(params)));
                    }
                auxData.m_paramChannel = new AuxParamChannel(std::move(outDataVector));
                break;
                }
            case PolyfaceAuxChannel::Vector:
                {
                if (auxData.m_displacementChannel.IsValid())
                    break;              // More than one scalar channel -- just use first for now.
                

                auto&                                   inDataVector = channel->GetData();
                bvector<AuxDisplacementChannel::DataPtr>      outDataVector;

                for (auto inData : inDataVector)
                    {
                    bvector<FPoint3d>   points;
                    double const*       pValue = inData->GetValues().data();

                    for (double const* pEnd = pValue + inData->GetValues().size(); pValue < pEnd; pValue += 3)
                        points.push_back(FPoint3d::From(pValue[0], pValue[1], pValue[2]));
                    
                    outDataVector.push_back(new AuxDisplacementChannel::Data((float) inData->GetInput(), std::move(points)));
                    }
                auxData.m_displacementChannel = new AuxDisplacementChannel(std::move(outDataVector));
                break;
                }
            }
            
        }
    }


