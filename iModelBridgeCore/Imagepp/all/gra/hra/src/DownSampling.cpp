/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hra/src/DownSampling.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>
#include <ImagePPInternal/gra/DownSampling.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPChannelType.h>
#include <Imagepp/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HGSTypes.h>


//////////////////////////////////////////////////////////////////////////
// Supported type definitions for down sampling
template <HRPChannelType::DataType DataType_T, uint32_t NbBytes_T> struct Type_T {};
template <> struct Type_T<HRPChannelType::VOID_CH, 8>   { typedef uint8_t _MyType; typedef uint32_t _SumType; };
template <> struct Type_T<HRPChannelType::VOID_CH, 16>  { typedef uint16_t _MyType; typedef uint32_t _SumType; };
template <> struct Type_T<HRPChannelType::VOID_CH, 32>  { typedef uint32_t _MyType; typedef uint64_t _SumType; };
template <> struct Type_T<HRPChannelType::INT_CH, 8>    { typedef uint8_t _MyType; typedef uint32_t _SumType; };
template <> struct Type_T<HRPChannelType::INT_CH, 16>   { typedef uint16_t _MyType; typedef uint32_t _SumType; };
template <> struct Type_T<HRPChannelType::INT_CH, 32>   { typedef uint32_t _MyType; typedef uint64_t _SumType; };
template <> struct Type_T<HRPChannelType::SINT_CH, 8>   { typedef int8_t _MyType; typedef int32_t _SumType; };
template <> struct Type_T<HRPChannelType::SINT_CH, 16>  { typedef int16_t _MyType; typedef int32_t _SumType; };
template <> struct Type_T<HRPChannelType::SINT_CH, 32>  { typedef int32_t _MyType; typedef int64_t _SumType; };
template <> struct Type_T<HRPChannelType::FLOAT_CH, 32> { typedef float _MyType; typedef double _SumType; };

// Supported tile size definitions for down sampling
template <uint32_t TileSizeX_T, uint32_t TileSizeY_T> struct TileSizeXY_T {};
template <> struct TileSizeXY_T<256, 256> { enum { TileSizeX = 256, TileSizeY = 256 }; };
template <> struct TileSizeXY_T<512, 512> { enum { TileSizeX = 512, TileSizeY = 512 }; };
template <> struct TileSizeXY_T<1024, 1024> { enum { TileSizeX = 1024, TileSizeY = 1024 }; };
template <> struct TileSizeXY_T<2048, 2048> { enum { TileSizeX = 2048, TileSizeY = 2048 }; };

/*---------------------------------------------------------------------------------**//**
* AverageOptimizedDownSamplerCreator_T
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct AverageOptimizedDownSamplerCreator_T
    {
    typedef OptimizedDownSampler* _ReturnType;
    static _ReturnType Create() { return new AverageOptimizedDownSampler_T <TileSize_T::TileSizeX, TileSize_T::TileSizeY, ChannelCount_T, typename ChannelType_T::_MyType>; }
    };

/*---------------------------------------------------------------------------------**//**
* NearestDownSamplerCreator_T
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct NearestDownSamplerCreator_T
    {
    typedef OptimizedDownSampler* _ReturnType;
    static _ReturnType Create() { return new NearestDownSampler_T <TileSize_T::TileSizeX, TileSize_T::TileSizeY, ChannelCount_T, typename ChannelType_T::_MyType>; }
    };

/*---------------------------------------------------------------------------------**//**
* AverageGenericDownSamplerCreator_T
* TileSize_T is ignored
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct AverageGenericDownSamplerCreator_T
    {
    typedef GenericDownSampler* _ReturnType;
    static _ReturnType Create() { return new AverageGenericDownSampler_T <ChannelCount_T, typename ChannelType_T::_MyType>; }
    };

/*---------------------------------------------------------------------------------**//**
* NearestGenericDownSamplerCreator_T
* TileSize_T is ignored
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct NearestGenericDownSamplerCreator_T
    {
    typedef GenericDownSampler* _ReturnType;
    static _ReturnType Create() { return new NearestGenericDownSampler_T <ChannelCount_T, typename ChannelType_T::_MyType>; }
    };
/*---------------------------------------------------------------------------------**//**
* AverageGenericStretcherNCreator_T
* TileSize_T is ignored
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct AverageGenericStretcherNCreator_T
    {
    typedef Stretch_1N_FunctionP _ReturnType;
    static _ReturnType Create()  { return &AverageStretch_N <ChannelCount_T, typename ChannelType_T::_MyType>; }
    };
/*---------------------------------------------------------------------------------**//**
* NearestGenericStretcherNCreator_T
* TileSize_T is ignored
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T> struct NearestGenericStretcherNCreator_T
    {
    typedef Stretch_1N_FunctionP _ReturnType;
    static _ReturnType Create()  { return &NearestStretch_N <ChannelCount_T, typename ChannelType_T::_MyType>; }
    };

/*---------------------------------------------------------------------------------**//**
* ChannelTypeResolver_T
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template 
<
uint32_t ChannelCount_T, 
typename TileSize_T, 
template < typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T > class Creator_T,
typename ReturnType_T
>
struct ChannelTypeResolver_T
    {
    enum { ChannelCount = ChannelCount_T };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static auto Create(HRPPixelType const& pixelType) -> ReturnType_T
        {
        if (!pixelType.GetChannelOrg().HaveSameSize())
            return NULL;

        HRPChannelType const* pChannelType = pixelType.GetChannelOrg().GetChannelPtr(0);
        switch (pChannelType->GetDataType())
            {
            case HRPChannelType::VOID_CH:
                if (pChannelType->GetSize() == 8)
                    return Creator_T<Type_T<HRPChannelType::VOID_CH, 8 >, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 16)                                                                           
                    return Creator_T<Type_T<HRPChannelType::VOID_CH, 16>, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 32)                                                                           
                    return Creator_T<Type_T<HRPChannelType::VOID_CH, 32>, ChannelCount_T, TileSize_T>::Create();
            case HRPChannelType::INT_CH:                                                                                     
                if (pChannelType->GetSize() == 8)                                                                            
                    return Creator_T<Type_T<HRPChannelType::INT_CH, 8 >, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 16)                                                                           
                    return Creator_T<Type_T<HRPChannelType::INT_CH, 16>, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 32)                                                                           
                    return Creator_T<Type_T<HRPChannelType::INT_CH, 32>, ChannelCount_T, TileSize_T>::Create();
            case HRPChannelType::SINT_CH:                                                                                    
                if (pChannelType->GetSize() == 8)                                                                            
                    return Creator_T<Type_T<HRPChannelType::SINT_CH, 8 >, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 16)                                                                           
                    return Creator_T<Type_T<HRPChannelType::SINT_CH, 16>, ChannelCount_T, TileSize_T>::Create();
                if (pChannelType->GetSize() == 32)                                                                           
                    return Creator_T<Type_T<HRPChannelType::SINT_CH, 32>, ChannelCount_T, TileSize_T>::Create();
            case HRPChannelType::FLOAT_CH:
                if (pChannelType->GetSize() == 32)
                    return Creator_T<Type_T<HRPChannelType::FLOAT_CH, 32>, ChannelCount_T, TileSize_T>::Create();
            }

        return NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* ChannelCountResolver_T
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template 
<
typename TileSize_T, 
template < typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T > class Creator_T,
typename ReturnType_T
> struct ChannelCountResolver_T
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static auto Create(HRPPixelType const& pixelType) -> ReturnType_T
        {
        uint32_t channelCount = (pixelType.CountIndexBits()) ? 1 : pixelType.GetChannelOrg().CountChannels();

        switch (channelCount)
            {
            case 1:
                return ChannelTypeResolver_T<1, TileSize_T, Creator_T, ReturnType_T>::Create(pixelType);
            case 2:
                return ChannelTypeResolver_T<2, TileSize_T, Creator_T, ReturnType_T>::Create(pixelType);
            case 3:
                return ChannelTypeResolver_T<3, TileSize_T, Creator_T, ReturnType_T>::Create(pixelType);
            case 4:
                return ChannelTypeResolver_T<4, TileSize_T, Creator_T, ReturnType_T>::Create(pixelType);
            }

        return NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* DownSamplerCreator
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template 
<
template < typename ChannelType_T, uint32_t ChannelCount_T, typename TileSize_T > class Creator_T,
typename ReturnType_T
>
struct TileSizeResolver_T
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 08/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static auto Create(uint64_t tileSizeX, uint64_t tileSizeY, HRPPixelType const& pixelType) -> ReturnType_T
        {
        if (tileSizeX == 256 && tileSizeY == 256)
            return ChannelCountResolver_T<TileSizeXY_T<256, 256>, Creator_T, ReturnType_T>::Create(pixelType);
        if (tileSizeX == 512 && tileSizeY == 512)
            return ChannelCountResolver_T<TileSizeXY_T<512, 512>, Creator_T, ReturnType_T>::Create(pixelType);
        if (tileSizeX == 1024 && tileSizeY == 1024)
            return ChannelCountResolver_T<TileSizeXY_T<1024, 1024>, Creator_T, ReturnType_T>::Create(pixelType);
        if (tileSizeX == 2048 && tileSizeY == 2048)
            return ChannelCountResolver_T<TileSizeXY_T<2048, 2048>, Creator_T, ReturnType_T>::Create(pixelType);

        return NULL;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
OptimizedDownSampler* CreateOptimizedDownSampler(uint64_t tileSizeX, uint64_t tileSizeY, HRPPixelType const& pixelType, HGSResampling const& method)
    {
    if (method.GetResamplingMethod() == HGSResampling::NEAREST_NEIGHBOUR || pixelType.CountIndexBits() || pixelType.GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        return TileSizeResolver_T<NearestDownSamplerCreator_T, OptimizedDownSampler*>::Create(tileSizeX, tileSizeY, pixelType);
    else
        return TileSizeResolver_T<AverageOptimizedDownSamplerCreator_T, OptimizedDownSampler*>::Create(tileSizeX, tileSizeY, pixelType);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GenericDownSampler* CreateGenericDownSampler(HRPPixelType const& pixelType, HGSResampling const& method)
    {
    // Generic down sampler ignore tile size. Providing tile size of 256 but is ignored.
    if (method.GetResamplingMethod() == HGSResampling::NEAREST_NEIGHBOUR || pixelType.CountIndexBits() || pixelType.GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        return ChannelCountResolver_T<TileSizeXY_T<256, 256>, NearestGenericDownSamplerCreator_T, GenericDownSampler*>::Create(pixelType);
    else
        return ChannelCountResolver_T<TileSizeXY_T<256, 256>, AverageGenericDownSamplerCreator_T, GenericDownSampler*>::Create(pixelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Stretch_1N_FunctionP CreateGenericStretcherN(HRPPixelType const& pixelType, HGSResampling const& method)
    {
    // Generic down sampler ignore tile size. Providing tile size of 256 but is ignored.
    if (method.GetResamplingMethod() == HGSResampling::NEAREST_NEIGHBOUR || pixelType.CountIndexBits() || pixelType.GetChannelOrg().GetChannelPtr(0)->GetNoDataValue() != NULL)
        return ChannelCountResolver_T<TileSizeXY_T<256, 256>, NearestGenericStretcherNCreator_T, Stretch_1N_FunctionP>::Create(pixelType);
    else
        return ChannelCountResolver_T<TileSizeXY_T<256, 256>, AverageGenericStretcherNCreator_T, Stretch_1N_FunctionP>::Create(pixelType);
    }
