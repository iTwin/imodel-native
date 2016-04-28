/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Content/GCS.h $
|    $RCSfile: GCS.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/22 21:58:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/LocalTransform.h>
#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


struct GCSConfigImpl;

struct GCSConfig //: public ContentConfigComponentMixinBase<TypeConfig>
    {

    private:
        RefCountedPtr<GCSConfigImpl>
            m_pImpl;

    public:
        //    IMPORT_DLLE static ClassID          s_GetClassID                       ();
        IMPORT_DLLE                 GCSConfig();
        IMPORT_DLLE                 GCSConfig(const GCS&             gcs, uint32_t flags = 0);
        IMPORT_DLLE virtual                 ~GCSConfig();

        IMPORT_DLLE                         GCSConfig(const GCSConfig&           rhs);

        const GCS&                     GetGCS() const;
        GCSConfig&                  PrependToExistingLocalTransform(bool                        prepend);
        GCSConfig&                  PreserveExistingIfGeoreferenced(bool                        preserve);
        GCSConfig&                  PreserveExistingIfLocalCS(bool                        preserve);

        bool                                IsPrependedToExistingLocalTransform() const;
        bool                                IsExistingPreservedIfGeoreferenced() const;
        bool                                IsExistingPreservedIfLocalCS() const;

        bool                       IsSet() const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
