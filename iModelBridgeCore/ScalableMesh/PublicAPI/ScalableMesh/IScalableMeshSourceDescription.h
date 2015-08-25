/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceDescription.h $
|    $RCSfile: IScalableMeshSourceDescription.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/01/06 16:30:17 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/DataType.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct ContentDescriptor;
struct LayerDescriptor;
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

    UInt                                    m_id;
    const Import::LayerDescriptor*          m_descriptorP;
    const void*                             m_implP;

    explicit                                SourceLayerDescriptor      (UInt                            layer,
                                                                        const Import::LayerDescriptor&  descriptor);   

                                            SourceLayerDescriptor      (const SourceLayerDescriptor&    rhs);
    SourceLayerDescriptor&                  operator=                  (const SourceLayerDescriptor&    rhs);
public:
    BENTLEYSTM_EXPORT UInt                        GetID                      () const;
    BENTLEYSTM_EXPORT const WChar*              GetName                    () const;

    BENTLEYSTM_EXPORT const Import::LayerDescriptor&
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
        UInt                                m_layerID;
        Import::DataType                    m_type;
    public:
        explicit                            IncompleteType             (UInt                        layer,
                                                                        const Import::DataType&     type);

        BENTLEYSTM_EXPORT UInt                    GetLayerID                 () const;
        BENTLEYSTM_EXPORT const Import::DataType& GetType                    () const;

        };


    typedef const IncompleteType*           IncompleteTypeCIter;
    typedef const SourceLayerDescriptor*    LayerCIter;

    BENTLEYSTM_EXPORT                             ~SourceDescriptor          ();


    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source);
    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateOriginalFor          (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);

    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source);   
    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEYSTM_EXPORT static SourceDescriptorCPtr CreateFor                  (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);


    BENTLEYSTM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source);
    BENTLEYSTM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source,
                                                                        Status&                         status);
    BENTLEYSTM_EXPORT static Import::SourcePtr    CreateOriginalSourceFor    (const IDTMSource&               source,
                                                                        Status&                         status,
                                                                        StatusInt&                      statusEx);

    BENTLEYSTM_EXPORT static Import::SourcePtr    ConfigureSource            (const Import::SourcePtr&        sourcePtr,
                                                                        const SourceImportConfig&       config);

    BENTLEYSTM_EXPORT bool                        HoldsIncompleteTypes       () const;
    BENTLEYSTM_EXPORT bool                        IsPod                      () const;

    BENTLEYSTM_EXPORT IncompleteTypeCIter         IncompleteTypesBegin       () const;
    BENTLEYSTM_EXPORT IncompleteTypeCIter         IncompleteTypesEnd         () const;


    BENTLEYSTM_EXPORT UInt                        GetLayerSelectionSize      () const;

    BENTLEYSTM_EXPORT LayerCIter                  LayerSelectionBegin        () const;
    BENTLEYSTM_EXPORT LayerCIter                  LayerSelectionEnd          () const;


    BENTLEYSTM_EXPORT UInt                        GetLayerCount              () const;
    BENTLEYSTM_EXPORT const Import::ContentDescriptor&
                                            GetDescriptor              () const;
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
