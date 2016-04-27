/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/ImportConfig.h $
|    $RCSfile: ImportConfig.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:58:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/GeoCoords/GCS.h>
BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
#if 0
struct IImportConfigVisitor;

struct ImportConfigComponent;
struct ImportConfigComponentBase;
#endif
struct ContentDescriptor;
struct LayerDescriptor;
struct ExtractionConfig;
struct FilteringConfig;
struct CustomFilteringSequence;
//struct GCS;

#if 0
/*---------------------------------------------------------------------------------**//**
* @description  Object used to store the configuration specifying how to import dtm
*               data. 
*               
* NOTE : - Not designed to be used as a base class.
*        - This is a copy on write implementation, so there is no cost copying instances
*          of this object.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfig
    {
private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;

    ImplPtr                             m_pImpl;

public:
    typedef ImportConfigComponent       value_type;
    typedef const value_type&           const_reference;
    typedef value_type&                 reference;

    typedef const std::type_info*       ComponentClassID;

    IMPORT_DLLE explicit                ImportConfig                   ();
    IMPORT_DLLE                         ~ImportConfig                  ();

    IMPORT_DLLE                         ImportConfig                   (const ImportConfig&                     rhs);
    IMPORT_DLLE ImportConfig&           operator=                      (const ImportConfig&                     rhs);

    IMPORT_DLLE void                    push_back                      (const ImportConfigComponent&            config);
    IMPORT_DLLE void                    push_back                      (const ImportConfigComponentBase&        config);

    IMPORT_DLLE void                    Accept                         (IImportConfigVisitor&                   visitor) const;

    IMPORT_DLLE void                    RemoveAllOfType                (ComponentClassID                        classID);
    };

#endif

struct ImportConfig : RefCountedBase
    {
    protected:
        virtual void _SetAreAttachmentsImported(bool importAttachments) = 0;
        virtual void _SetDefaultTargetLayer(uint32_t layer) = 0;
        virtual void _SetDefaultTargetType(const DataTypeFamily* typeP) = 0;

        virtual void _SetDefaultSourceGCS(const GCS* gcsP) = 0;
        virtual void _SetDefaultTargetGCS(const GCS* gcsP) = 0;

        virtual void _SetDefaultTargetSMData(const ScalableMeshData* smDataP) = 0;

        virtual void _SetExtractionConfig(const ExtractionConfig* extractionConfigP) = 0;
        virtual void _SetFilteringConfig(const FilteringConfig* filteringConfigP) = 0;

        virtual void _SetSourceFilters(const CustomFilteringSequence& sourceFilters) = 0;
        virtual void _SetTargetFilters(const CustomFilteringSequence& targetFilters) = 0;

         explicit                ImportConfig();
         virtual                        ~ImportConfig();

    public:
        IMPORT_DLLE void SetAreAttachmentsImported(bool importAttachments)
            {
            return _SetAreAttachmentsImported(importAttachments);
            }
        IMPORT_DLLE void SetDefaultTargetLayer(uint32_t layer)
            {
            return _SetDefaultTargetLayer(layer);
            }
        IMPORT_DLLE void SetDefaultTargetType(const DataTypeFamily* typeP)
            {
            return _SetDefaultTargetType(typeP);
            }

        IMPORT_DLLE void SetDefaultSourceGCS(const GCS* gcsP)
            {
            return _SetDefaultSourceGCS(gcsP);
            }
        IMPORT_DLLE void SetDefaultTargetGCS(const GCS* gcsP)
            {
            return _SetDefaultTargetGCS(gcsP);
            }

        IMPORT_DLLE void SetDefaultTargetSMData(const ScalableMeshData* smDataP)
            {
            return _SetDefaultTargetSMData(smDataP);
            }

        IMPORT_DLLE void SetExtractionConfig(const ExtractionConfig* extractionConfigP)
            {
            return _SetExtractionConfig(extractionConfigP);
            }
        IMPORT_DLLE void SetFilteringConfig(const FilteringConfig* filteringConfigP)
            {
            return _SetFilteringConfig(filteringConfigP);
            }

        IMPORT_DLLE void SetSourceFilters(const CustomFilteringSequence& sourceFilters)
            {
            return _SetSourceFilters(sourceFilters);
            }
        IMPORT_DLLE void SetTargetFilters(const CustomFilteringSequence& targetFilters)
            {
            return _SetTargetFilters(targetFilters);
            }

        IMPORT_DLLE RefCountedPtr<ImportConfig> Create();
    };

#if 0
/*---------------------------------------------------------------------------------**//**
* @description  
*                        
* @see ImportConfig
* @see ImportConfigComponentBase
*
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfigComponent
    {
private:
    typedef const std::type_info*       ClassID;
    typedef SharedPtrTypeTrait<const ImportConfigComponentBase>::type       
                                        BaseCPtr;
    BaseCPtr                            m_basePtr;
    ClassID                             m_classID;
public:
    explicit                            ImportConfigComponent          (const ImportConfigComponentBase&        config);

    IMPORT_DLLE                         ~ImportConfigComponent         ();

    IMPORT_DLLE                         ImportConfigComponent          (const ImportConfigComponent&            rhs);    
    IMPORT_DLLE ImportConfigComponent&  operator=                      (const ImportConfigComponent&            rhs);  

    ClassID                             GetClassID                     () const { return m_classID; }

    IMPORT_DLLE void                    Accept                         (IImportConfigVisitor&                   visitor) const;

    };

#endif

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
