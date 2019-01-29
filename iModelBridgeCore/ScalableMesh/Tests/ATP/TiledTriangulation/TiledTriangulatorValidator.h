/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/TiledTriangulatorValidator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*#pragma once
#include "ITiledTriangulatorValidator.h"
#include "MrDTMFace.h"
#include "TiledTriangulatorUtil.h"
#include <TerrainModel/TerrainModel.h>
#include <ImagePP/all/h/HGF3DExtent.h>
#include <Geom/dpoint3d.h>

typedef HGF3DExtent<double> YProtFeatureExtentType;

USING_NAMESPACE_BENTLEY_SCALABLEMESH

//BEGIN_GEODTMAPP_NAMESPACE

class TiledTriangulatorValidator : public ITiledTriangulatorValidator
    {
    private:

        RefCountedPtr<BcDTM>      m_memDtmPtr;
        vector<FaceWithProperties> m_memDtmFaces;
        vector<vector<size_t>>     m_memDtmIndexedFaces;

        double m_indexRangeX;
        double m_indexRangeY;
        double m_indexStepX;
        double m_indexStepY;


        bool   m_outputIncorrectTriangles;

        //Stats
        unsigned __int64 m_totalNbComparedTiles;
        unsigned __int64 m_totalNbWrongTiles;

        unsigned __int64 m_totalNbComparedTriangles;
        unsigned __int64 m_totalNbWrongTriangles;

        unsigned __int64 m_nbComparedTrianglesLastTile;
        unsigned __int64 m_nbWrongTrianglesLastTile;

    protected:

        virtual int _CompareMemDTMwithTileDTM(const DTMPtr& tileDtmPtr) override;

        virtual int _GetLastTileStat(unsigned __int64& nbComparedTriangles, unsigned __int64& nbWrongTriangles) const override;

        virtual int _GetTotalStat(unsigned __int64& totalNbComparedTiles,
                                  unsigned __int64& totalNbWrongTiles,
                                  unsigned __int64& totalNbComparedTriangles,
                                  unsigned __int64& totalNbWrongTriangles) const override;

        virtual int _SetOuputInActiveModel(bool outputIncorrectTriangles) override;

    public:

        typedef RefCountedPtr<TiledTriangulatorValidator> Ptr;

        TiledTriangulatorValidator(RefCountedPtr<BcDTM> memDtmPtr);
        ~TiledTriangulatorValidator();
    };