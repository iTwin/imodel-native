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

struct ContentDescriptor;
struct LayerDescriptor;
struct ExtractionConfig;
struct FilteringConfig;
struct CustomFilteringSequence;
//struct GCS;



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
        virtual const CustomFilteringSequence& _GetTargetFilters(void) const = 0;

        virtual void _SetClipShape(const HFCPtr<HVEClipShape>& clipShapePtr) = 0;

        virtual const HVEClipShape* _GetClipShape() const = 0;

        virtual bool                    _HasDefaultTargetGCS() const = 0;
        virtual const GCS&              _GetDefaultTargetGCS() const = 0;

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
        IMPORT_DLLE void SetClipShape(const HFCPtr<HVEClipShape>& clipShapePtr)
            {
            return _SetClipShape(clipShapePtr);
            }

        IMPORT_DLLE const HVEClipShape* GetClipShape() const
            {
            return _GetClipShape();
            }

        IMPORT_DLLE const CustomFilteringSequence& GetTargetFilters() const
            {
            return _GetTargetFilters();
            }

        IMPORT_DLLE bool                    HasDefaultTargetGCS() const
            {
            return _HasDefaultTargetGCS();
            }
        IMPORT_DLLE const GCS&              GetDefaultTargetGCS() const
            {
            return _GetDefaultTargetGCS();
            }

        IMPORT_DLLE RefCountedPtr<ImportConfig> Create();
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
