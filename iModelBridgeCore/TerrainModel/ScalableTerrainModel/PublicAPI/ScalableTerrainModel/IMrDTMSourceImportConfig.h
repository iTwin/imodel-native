/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMSourceImportConfig.h $
|    $RCSfile: IMrDTMSourceImportConfig.h,v $
|   $Revision: 1.17 $
|       $Date: 2011/12/01 18:51:38 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
#include <ScalableTerrainModel/Import/Definitions.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct ImportSequence;
struct ContentConfig;
struct ImportConfig;

struct DataType;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
struct GCS;
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*__PUBLISH_SECTION_END__*/
struct EditListener;
/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceImportConfig : private Import::Unassignable
    {
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
    void                                        SetInternalConfig          (const Import::ImportConfig&                 config);

    void                                        RegisterEditListener       (EditListener&                               listener);
    void                                        UnregisterEditListener     (const EditListener&                         listener);

/*__PUBLISH_SECTION_START__*/

    /*
     * NTERAY: Exported in order to be able to continue development in Descartes. Use with care and only on last resort.
     */
    BENTLEYSTM_EXPORT const Import::ContentConfig&    GetContentConfig           () const;
    BENTLEYSTM_EXPORT const Import::ImportSequence&   GetSequence                () const;
    BENTLEYSTM_EXPORT const Import::ImportConfig&     GetConfig                  () const;

    /*
     * NTERAY: Exported in order to be able to continue development in Descartes. Use with care and only on last resort.
     */
    BENTLEYSTM_EXPORT void                            SetContentConfig           (const Import::ContentConfig&                config);
    BENTLEYSTM_EXPORT void                            SetSequence                (const Import::ImportSequence&               sequence);
    BENTLEYSTM_EXPORT void                            SetConfig                  (const Import::ImportConfig&                 config);


    /*
     * Default is to import all layers. Use this only to import only specific layers.
     */
    BENTLEYSTM_EXPORT void                            AddImportedLayer           (UInt                                        layerID);



    BENTLEYSTM_EXPORT void                            SetReplacementType         (const Import::DataType&                     type);



    BENTLEYSTM_EXPORT const GeoCoords::GCS&           GetReplacementGCS          ();
    BENTLEYSTM_EXPORT void                            SetReplacementGCS          (const GeoCoords::GCS&                       gcs);


    BENTLEYSTM_EXPORT void                            SetReplacementGCS          (const GeoCoords::GCS&                       gcs,
                                                                            bool                                        prependToExistingLocalTransform,
                                                                            bool                                        preserveExistingIfGeoreferenced,
                                                                            bool                                        preserveExistingIfLocalCS);

    };


END_BENTLEY_MRDTM_NAMESPACE