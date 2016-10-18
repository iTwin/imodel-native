/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceImportConfig.h $
|    $RCSfile: IScalableMeshSourceImportConfig.h,v $
|   $Revision: 1.17 $
|       $Date: 2011/12/01 18:51:38 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Import/Definitions.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct ImportSequence;
struct ContentConfig;
struct ImportConfig;

struct DataType;
struct ScalableMeshData;

namespace Internal
    {
    class Config;
    }
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
struct GCS;
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*__PUBLISH_SECTION_END__*/
struct EditListener;
/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceImportConfig : private Import::Unassignable
    {
    friend class Import::Internal::Config;
private:
    struct                                      Impl;
    std::auto_ptr<Impl>                         m_implP;

public:

/*__PUBLISH_SECTION_END__*/
    explicit                                    SourceImportConfig         (const Import::ImportSequence&               defaultSequence);

                                                SourceImportConfig         (const SourceImportConfig&                   rhs);

                                                ~SourceImportConfig        ();

    void                                        SetInternalContentConfig   (const Import::ContentConfig&                config);
    void                                        SetInternalSequence        (const Import::ImportSequence&               sequence);
    void                                        SetInternalConfig          (const Import::ImportConfig*                 config);

    void                                        RegisterEditListener       (EditListener&                               listener);
    void                                        UnregisterEditListener     (const EditListener&                         listener);

/*__PUBLISH_SECTION_START__*/

    /*
     * NTERAY: Exported in order to be able to continue development in Descartes. Use with care and only on last resort.
     */
    BENTLEY_SM_EXPORT const Import::ContentConfig&    GetContentConfig           () const;
    BENTLEY_SM_EXPORT const Import::ImportSequence&   GetSequence                () const;
    BENTLEY_SM_EXPORT Import::ImportConfig*     GetConfig                  () const;

    /*
     * NTERAY: Exported in order to be able to continue development in Descartes. Use with care and only on last resort.
     */
    BENTLEY_SM_EXPORT void                            SetContentConfig           (const Import::ContentConfig&                config);
    BENTLEY_SM_EXPORT void                            SetSequence                (const Import::ImportSequence&               sequence);
    BENTLEY_SM_EXPORT void                            SetConfig                  (const Import::ImportConfig*                 config);


    /*
     * Default is to import all layers. Use this only to import only specific layers.
     */
    BENTLEY_SM_EXPORT void                            AddImportedLayer           (uint32_t                                        layerID);



    BENTLEY_SM_EXPORT void                            SetReplacementType         (const Import::DataType&                     type);



    BENTLEY_SM_EXPORT const GeoCoords::GCS&           GetReplacementGCS          ();
    BENTLEY_SM_EXPORT void                            SetReplacementGCS          (const GeoCoords::GCS&                       gcs);


    BENTLEY_SM_EXPORT void                            SetReplacementGCS          (const GeoCoords::GCS&                       gcs,
                                                                            bool                                        prependToExistingLocalTransform,
                                                                            bool                                        preserveExistingIfGeoreferenced,
                                                                            bool                                        preserveExistingIfLocalCS);
    BENTLEY_SM_EXPORT void                            SetReplacementSMData       (const Import::ScalableMeshData& data);
    BENTLEY_SM_EXPORT const Import::ScalableMeshData& GetReplacementSMData       () const;


    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
