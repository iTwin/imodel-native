/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/ITiledTriangulatorValidator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/DgnPlatform.h>
#include <ScalableMesh/IScalableMesh.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH

//BEGIN_GEODTMAPP_NAMESPACE

struct ITiledTriangulatorValidator;
typedef RefCountedPtr<ITiledTriangulatorValidator> ITiledTriangulatorValidatorPtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
/*struct ITiledTriangulatorValidator : public RefCountedBase
    {
        private:
        protected:

        virtual int _CompareMemDTMwithTileDTM(const DTMPtr& tileDtmPtr) = 0;

        virtual int _GetLastTileStat(unsigned __int64& nbComparedTriangles, unsigned __int64& nbWrongTriangles) const = 0;

        virtual int _GetTotalStat(unsigned __int64& totalNbComparedTiles,
                                  unsigned __int64& totalNbWrongTiles,
                                  unsigned __int64& totalNbComparedTriangles,
                                  unsigned __int64& totalNbWrongTriangles) const = 0;

        virtual int _SetOuputInActiveModel(bool outputIncorrectTriangles) = 0;

    public:

        int CompareMemDTMwithTileDTM(const DTMPtr& tileDtmPtr);

        int GetLastTileStat(unsigned __int64& nbComparedTriangles, unsigned __int64& nbWrongTriangles) const;

        int GetTotalStat(unsigned __int64& totalNbComparedTiles,
                                   unsigned __int64& totalNbWrongTiles,
                                   unsigned __int64& totalNbComparedTriangles,
                                   unsigned __int64& totalNbWrongTriangles) const;

        int SetOuputInActiveModel(bool outputIncorrectTriangles);

        static ITiledTriangulatorValidatorPtr CreateFor (RefCountedPtr<BcDTM> memDtmPtr);
    };*/

//END_GEODTMAPP_NAMESPACE
//#endif