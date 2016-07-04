/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceDescription.h $
|    $RCSfile: IScalableMeshSourceDescription.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/01/06 16:30:17 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/DataType.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct ContentDescriptor;
struct ILayerDescriptor;
struct ContentConfig;
struct ImportSequence;
struct Source;


typedef SharedPtrTypeTrait<Source>::type    SourcePtr; 
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct IDTMSource;
struct SourceDescriptor;
struct SourceImportConfig;

typedef RefCountedPtr<SourceDescriptor>     SourceDescriptorCPtr; // TDORAY: Make const as it becomes available.


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceLayerDescriptor
    {
private:
    friend struct                           SourceLayerDescriptorHolder;

    uint32_t                                    m_id;
    RefCountedPtr<const Import::ILayerDescriptor>          m_descriptorP;
    const void*                             m_implP;

    explicit                                SourceLayerDescriptor      (uint32_t                            layer,
                                                                        const Import::ILayerDescriptor&  descriptor);   

                                            SourceLayerDescriptor      (const SourceLayerDescriptor&    rhs);
    SourceLayerDescriptor&                  operator=                  (const SourceLayerDescriptor&    rhs);
public:
    BENTLEY_SM_EXPORT uint32_t                        GetID                      () const;
    BENTLEY_SM_EXPORT const WChar*              GetName                    () const;

    BENTLEY_SM_EXPORT const Import::ILayerDescriptor&
                                            GetDescriptor              () const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceDescriptor : private Import::Uncopyable, public RefCountedBase
    {
    /* 
     * Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
     *          is required.
     */          
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_ERROR_EMPTY_SLOT, // TDORAY: Was not removed as should have been. Do remove when next occasion presents itself.
        S_ERROR_NOT_SUPPORTED,
        S_ERROR_NOT_FOUND,
        S_QTY,
        };

private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_implP;

    explicit                                SourceDescriptor           (Impl*                       implP);

    static SourceDescriptorCPtr             CreateFor                  (const Import::Source&       source,
                                                                        const SourceImportConfig&   config);

public:

    // TDORAY: Couldn't this be provided as part of the layer selection?
    struct IncompleteType
        {
    private:
        uint32_t                                m_layerID;
        Import::DataType                    m_type;
    public:
        explicit                            IncompleteType             (uint32_t                        layer,
                                                                        const Import::DataType&     type);

        BENTLEY_SM_EXPORT uint32_t                    GetLayerID                 () const;
        BENTLEY_SM_EXPORT const Import::DataType& GetType                    () const;

        };


    typedef const IncompleteType*           IncompleteTypeCIter;
    typedef const SourceLayerDescriptor*    LayerCIter;

    BENTLEY_SM_EXPORT                             ~SourceDescriptor          ();


    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source);
    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);

    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source);   
    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEY_SM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);


    BENTLEY_SM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source);
    BENTLEY_SM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEY_SM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);

    BENTLEY_SM_EXPORT static Import::SourcePtr    ConfigureSource            (const Import::SourcePtr&        sourcePtr,
                                                                        const SourceImportConfig&       config);

    BENTLEY_SM_EXPORT bool                        HoldsIncompleteTypes       () const;
    BENTLEY_SM_EXPORT bool                        IsPod                      () const;

    BENTLEY_SM_EXPORT IncompleteTypeCIter         IncompleteTypesBegin       () const;
    BENTLEY_SM_EXPORT IncompleteTypeCIter         IncompleteTypesEnd         () const;


    BENTLEY_SM_EXPORT uint32_t                        GetLayerSelectionSize      () const;

    BENTLEY_SM_EXPORT LayerCIter                  LayerSelectionBegin        () const;
    BENTLEY_SM_EXPORT LayerCIter                  LayerSelectionEnd          () const;


    BENTLEY_SM_EXPORT uint32_t                        GetLayerCount              () const;
    BENTLEY_SM_EXPORT const Import::ContentDescriptor&
                                            GetDescriptor              () const;
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
