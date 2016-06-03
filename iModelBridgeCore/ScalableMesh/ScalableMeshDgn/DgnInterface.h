#include "ElementStats.h"
#include <list>

int testFunction();

typedef void* DgnFileHandle;
typedef void* DgnModelHandle;
typedef int LevelId;
typedef void* DgnElementHandle;

enum class ElementType : uint16_t
    {
    RASTER_ELEM_TYPE,
    STM_ELEM_TYPE,
    CIVIL_ELEM_TYPE,
    POD_ELEM_TYPE
    };

struct RefToModelAndElement
    {
    ElementType type;
    DgnModelHandle targetModel;
    DgnElementHandle targetElement;
    };

DgnFileHandle OpenDGNFile  (const wchar_t*      pName,
                            int&        status);

DgnModelHandle FindDGNModel(const DgnFileHandle&     dgnFile,
                            int                  modelID,
                            int&               status);

DgnModelHandle FindDGNModel(const DgnFileHandle&    dgnFile,
                            const wchar_t*          modelName,
                            int&              status);

int                      FindLevel(const DgnModelHandle&                model,
                                   LevelId                                 levelID);

int                      FindLevel(const DgnModelHandle&              model,
                                   const wchar_t*                          levelName,
                                   LevelId&                      levelID);

void ComputeCounts(ElementStats& stats, DgnModelHandle    modelRefP, LevelId        levelID);

void ImportSourceRef(std::list<RefToModelAndElement>& refs, DgnModelHandle    modelRefP, LevelId        levelID);