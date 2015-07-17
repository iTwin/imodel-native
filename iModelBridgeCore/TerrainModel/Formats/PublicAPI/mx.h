/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/mx.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <Bentley/WString.h>
/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/
#include <list>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>


///////// bcdtmMX/////////
BENTLEYDTM_Private        int bcdtmFormatMX_clipUsingIslandFeatureIdDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureId dtmFeatureId) ;
BENTLEYDTM_Private        int bcdtmFormatMX_getMxTriangleNumberDtmObject(BC_DTM_OBJ *dtmP,DTM_MX_TRG_INDEX *trgIndexP,long trgPnt1,long trgPnt2,long trgPnt3,long *trgNumP ) ;
BENTLEYDTM_Private        int bcdtmFormatMX_insertRectangleAroundTinDtmObject(BC_DTM_OBJ *dtmP,double xdec,double xinc,double ydec,double yinc,DTMFeatureId *islandFeatureIdP) ;
BENTLEYDTM_Private        int bcdtmFormatMX_loadMxTrianglesFromDtmObject(BC_DTM_OBJ *dtmP,int (*loadFunctionP)() );

BENTLEYDTMFORMATS_EXPORT  int bcdtmExport_MXTriangulationFromDtmObject(BC_DTM_OBJ* dtmP, void* triPtrP, void* pointsPtrP);
BENTLEYDTMFORMATS_EXPORT  int bcdtmImport_MXTriangulationToDtmObject(BC_DTM_OBJ* dtmP, void* triPtrP, void* pointsPtrP);

TERRAINMODEL_TYPEDEFS (MXFilImporter)
ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, MXFilImporter);

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<MXFilImporter> MXFilImporterPtr;

/*__PUBLISH_SECTION_END__*/
class MXModelFile;
class ModelTableRecord;
class StringTableRecord;
/*__PUBLISH_SECTION_START__*/
struct MXFilImporter : TerrainImporter
    {
    /*__PUBLISH_SECTION_END__*/
    private: WString m_filename;
    private: mutable TerrainInfoList m_surfaces;

    private: MXFilImporter (WCharCP filename);

    private: void DoImport (bmap <WString, Bentley::TerrainModel::BcDTMPtr>& nameDtms, bool importAll) const;
    private: BcDTMPtr ImportStringModel (ModelTableRecord* modelTableRecord) const;
    private: BcDTMPtr ImportTriangulation (StringTableRecord* stringTableRecord, WCharCP name) const;

    public: virtual const TerrainInfoList& _GetTerrains () const override;
    public: virtual ImportedTerrain _ImportTerrain (WCharCP name) const override;
    public: virtual ImportedTerrainList _ImportTerrains () const override;
    public: virtual ImportedTerrainList _ImportTerrains (bvector<WString>& names) const override;

    /*__PUBLISH_SECTION_START__*/
    public: BENTLEYDTMFORMATS_EXPORT static bool IsFileSupported (WCharCP filename);
    public: BENTLEYDTMFORMATS_EXPORT static MXFilImporterPtr Create (WCharCP filename);

    };

END_BENTLEY_TERRAINMODEL_NAMESPACE