/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/MXModelFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel/Formats/Formats.h>
#include "msl_array.h"
class MXTriangle;
#include <Bentley/bmap.h>
#include <Bentley/bvector.h>
#include <Bentley\BeFile.h>

#pragma warning( disable: 4275 4251 )
#ifdef MXMODEL_EXPORTS
#define DLL  __declspec( dllexport )
#else
#define DLL
#endif

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

#ifndef MX_PI
#define MX_PI 3.14159265358979
#endif
class MXStringDimData;
enum ErrorStatus
    {
    eOk = 0,
    eCantOpenFile,
    eDuplicateName,
    eOpenedForWrite,
    eNotOpened,
    eKeyNotFound,
    eInvalidId,
    eNotImplemented,
    eReadOnly,
    eInvalidData,
    eNotInModelFile,
    eInvalidNumber,
    eInvalidName,
    eLeftHandedTriangle
    };
enum eDiscontinuity
    {
    eNone = 0,
    eStart = 1,
    eEnd = 2,
    eBearing = 3
    };
enum eCoordSystems
    {
    eCAD = -3,
    eWorld = -2,
    eNoSystem = -1,
    eXY = 0,
    eYX,
    eDane,
    eCzech1,
    eCzech2,
    eJapanese
    };

class ModelObjectId
    {
    public:
        short modelFileNumber;
        long objectNumber;

        ModelObjectId()
            {
            modelFileNumber = 0;
            objectNumber = 0;
            }
        bool IsNull(void) const
            {
            return modelFileNumber == 0 && objectNumber == 0;
            }
    };

const int RecordSize = 0x7e0;

const long stringTableType = 0xffc53d2c;

struct ModelTableFileRecord
    {
    char modelName[28];
    char modelType[4];
    long stringTablePos;
    long date; // ????
    };

struct StringTableFileRecord
    {
    char stringName[4];
    char subReference[4];
    long type;
    long numPoints;
    long minx, miny, maxx, maxy;
    long recordNumber;
    long offset;
    };

struct ElementContourString
    {
    double x;
    double y;
    };

#ifdef MXALLSTRINGTYPES
struct Element3dString
    {
    double x;
    double y;
    double z;
    };

struct ElementSurveyStationString
    {
    double x;
    double y;
    double z;
    char stationName[8];
    };
struct ElementSurveyObservationString
    {
    double x;
    double y;
    double z;
    long observation;
    long pad;
    };

struct ElementSectionString
    {
    double x;
    double y;
    double z;
    double offset;
    long label;
    long pad;
    };

struct ElementInterfaceString
    {
    double x;
    double y;
    double z;
    double offset;
    double bearing;
    };

struct ElementMasterString
    {
    double x;
    double y;
    double z;
    double chainage;
    double bearing;
    double radius;
    };

struct ElementGeometryString
    {
    double x; //HorizontalTPXcoordinate;
    double y; //HorizontalTPYcoordinate;
    double z; //HorizontalTPLevel;
    union
        {
        struct
            {
            double horizontalTPChainage;
            double horizontalTPBearing;
            double horizontalTPRadius;
            double horizontalTPVerticalGradient;
            double horizontalTPverticalMValue;
            } htps;
        struct
            {
            double nu1; //horizontalTPChainage;
            double nu2; //horizontalTPBearing;
            double horizontalTPRadius;
            double horizontalTPVerticalGradient;
            double horizontalTPverticalMValue;
            } hips;
        struct
            {
            double horizontalTPChainage;
            double horizontalTPBearing;
            double horizontalTPRadius;
            } hcen;
        struct
            {
            double chainage;
            double bearing;
            double radius;
            double verticalGradient;
            double designSpeed;
            } supe;
        struct
            {
            double chainage;
            double bearing;
            double radius;
            double verticalGradient;
            double verticalMValue;
            } vtps;
        struct
            {
            double nu1; //horizontalTPChainage;
            double verticalTPBearing;
            double verticalTPHorizontalRadius;
            double verticalTPVerticalGradient;
            double verticalTPverticalMValue;
            } vips;
        struct
            {
            double verticalmidordinatechainage;
            double verticalmidordinatebearing;
            double verticalmidordinateradius;
            double verticalmidordinateverticalgradient;
            double verticalmidordinateverticalmvalue;
            } vmos;
        struct
            {
            double verticalflatpointchainage;
            double verticalflatpointbearing;
            double verticalflatpointradius;
            double verticalflatpointverticalgradient;
            double verticalflatpointverticalmvalue;
            } vfps;
        };
    char horizontalTangentPointCode[8];
    char verticalTangentPointCode[8];
    char horizontalElementName[8];
    char verticalElementName[8];
    };

struct ElementCadastreString
    {
    double x;
    double y;
    double z;
    double symbolReferenceBearing;
    char surveyPointNum[8];
    char featureCode[4];
    long pad1;
    char cadastralPointNumber[16];
    };

struct ElementAreaString
    {
    double x;
    double y;
    double z;
    double chainage;
    double areaBetweenSections;
    double accumulatedArea;
    double cutAreaBetweenSections;
    double accumulatedCutArea;
    double fillAreaBetweenSections;
    double accumulatedFillArea;
    };

struct ElementEndAreaString
    {
    double x;
    double y;
    double z;
    double chainage;
    double sectionalAreaOfCut;
    double sectionalAreaOfFill;
    double d7;
    double d8;
    double d9;
    double d10;
    };

struct ElementVolumeString
    {
    double x;
    double y;
    double z;
    double chainage;
    double fillVolume;
    double cutVolume;
    double d7;
    double d8;
    double d9;
    double d10;
    };

struct ElementVisibilityString
    {
    double x;
    double y;
    double eyeX;
    double eyeY;
    double targetX;
    double targetY;
    double chainage;
    double achievedVisibilityDistance;
    double requiredVisibilityDistance;
    double visibilityDeficiency;
    };
#endif

struct ElementTextString
    {
    double x;
    double y;
    double height;
    double angle;
    char text[56];
    };

struct ElementTriangleStringDoubles
    {
    double pts[3][3];
    double a[8];
    };

struct ElementTriangleStringInts
    {
    int pad1[3];
    int adjTri[3];
    int pts[3];
    int pad2[3];
    int label[3];
    int groupCode;
    };

struct ElementTriangleStringRecord
    {
    ElementTriangleStringDoubles doubles[10];
    int pad;                                                        // Pad of 1 in here for some reason.
    ElementTriangleStringInts ints[9];    // Note this is 10 but the groupCode of the 10 is the nextRecord Field.
    };

struct ElementTriangleString
    {
    ElementTriangleStringDoubles doubles;
    ElementTriangleStringInts ints;
    };

struct ModelFileRecord
    {
    union
        {
        ModelTableFileRecord models[50];
        StringTableFileRecord strings[50];
        ElementTriangleStringRecord triangle;
        char data[2000];
        };
    long nextRecord;
    long lastRecord;
    long pad[2];
    };

class MXRunTimeObject;

typedef MXRunTimeObject* (*ClassMakeFunction)(void);
typedef bool (*IsTypeFunction)(const StringTableFileRecord* str, const long modelType); //const int type, const long name);

class MXRunTimeClass
    {
    public:
        char* _className;
        MXRunTimeClass* _parent;
        IsTypeFunction _isType;
        ClassMakeFunction _makeFunction;

        MXRunTimeObject* create(void);
        int isDerivedFrom(const MXRunTimeClass* classPtr);
        MXRunTimeClass()
            {
            _className = NULL;
            }
        virtual ~MXRunTimeClass()
            {
            if(_className)
                free(_className);
            }
        const char* name(void) const
            {
            return _className;
            }
    };

class DLL ClassList
    {
    private:
        ArrayPtrClass<MXRunTimeClass> _classes;
    public:
        MXRunTimeClass* at(const char* const className);
        MXRunTimeClass* getType(const StringTableFileRecord* str, const long modelType); //const int type, const long name);
        MXRunTimeClass* newMXRunTimeClass(const char* const className, const char* const parentClass, IsTypeFunction isType, ClassMakeFunction makeFunction);
        void deleteClass(MXRunTimeClass* classPtr);
        ClassList();
        static ClassList& ClassList::Instance()
            {
            if(!cl_classList) cl_classList = new ClassList();
            return *cl_classList;
            };
        static ClassList* cl_classList;
    };


DLL MXRunTimeClass* newMXRunTimeClass(const char* const className, const char* const parentClass, IsTypeFunction isType = NULL, ClassMakeFunction makeFunction = NULL);

class DLL MXRunTimeObject
    {
    public:
        virtual MXRunTimeClass* isA() const;
        int isKindOf(MXRunTimeClass* desc) const;
    };


#define MXRT_DECLARE_MEMBERS(CLASS_NAME) \
    virtual MXRunTimeClass* isA() const; \
    static MXRunTimeClass* gpDesc; \
    static MXRunTimeClass* desc(); \
    static bool isType(const StringTableFileRecord* str, const long modelType); /*const int type, const long Name);*/ \
    static CLASS_NAME* cast(const MXRunTimeObject* inPtr) \
    { return ((inPtr == NULL) || !inPtr->isKindOf(CLASS_NAME::desc())) \
    ? NULL : (CLASS_NAME*)inPtr; }; \
    static void rtInit()

#define MXRT_DEFINE_MEMBERS(CLASS_NAME) \
    MXRunTimeClass* CLASS_NAME::desc() \
    { \
    if (CLASS_NAME::gpDesc != NULL) \
    return CLASS_NAME::gpDesc; \
    return CLASS_NAME::gpDesc \
    = ClassList::Instance().at(#CLASS_NAME); \
        } \
        MXRunTimeClass* CLASS_NAME::isA() const { return CLASS_NAME::desc(); } \
        MXRunTimeClass* CLASS_NAME::gpDesc = NULL

#define MXRTObject_DEFINE_MEMBERS(CLASS_NAME,PARENT_CLASS) \
    MXRT_DEFINE_MEMBERS(CLASS_NAME); \
    void CLASS_NAME::rtInit() { \
    if (CLASS_NAME::gpDesc != NULL) \
    return; \
    CLASS_NAME::gpDesc = newMXRunTimeClass(#CLASS_NAME, #PARENT_CLASS); \
        }

#define MXRT_STRING_DEFINE_MEMBERS(CLASS_NAME,PARENT_CLASS) \
    MXRT_DEFINE_MEMBERS(CLASS_NAME); \
    static MXRunTimeObject * make##CLASS_NAME() { return new CLASS_NAME(); } \
    void CLASS_NAME::rtInit() { \
    if (CLASS_NAME::gpDesc != NULL) \
    return; \
    CLASS_NAME::gpDesc = newMXRunTimeClass(#CLASS_NAME, #PARENT_CLASS, &CLASS_NAME::isType, &make##CLASS_NAME); \
        }

class ModelFilePos
    {
    private:
        long _pos;

    public:
        long getRecordNumber(void) const
            {
            return _pos / 500;
            }
        void setRecordNumber(const long newRecordNumber)
            {
            _pos = (_pos % 500) + (500 * newRecordNumber);
            }

        long getRecordPos(void) const
            {
            return (_pos % 500) + 1;
            }
        void setRecordPos(const short newPos)
            {
            _pos = ((_pos / 500) * 500) + (newPos - 1);
            }
        ModelFilePos(const long recordNumber, const short pos)
            {
            _pos = (recordNumber * 500) + pos - 1;
            }
        ModelFilePos()
            {
            _pos = 0;
            }
        inline ModelFilePos& operator+=(const int num)
            {
            _pos += num;
            return *this;
            }

        inline ModelFilePos& operator=(const ModelFilePos& other)
            {
            _pos = other._pos;
            return *this;
            }
        bool operator <  (const ModelFilePos& other) const
            {
            return _pos < other._pos;
            }
        bool operator >  (const ModelFilePos& other) const
            {
            return _pos > other._pos;
            }
        bool operator >= (const ModelFilePos& other) const
            {
            return _pos >= other._pos;
            }
        bool operator <= (const ModelFilePos& other) const
            {
            return _pos <= other._pos;
            }
        bool operator == (const ModelFilePos& other) const
            {
            return _pos == other._pos;
            }
        bool operator != (const ModelFilePos& other) const
            {
            return _pos != other._pos;
            }

        long operator- (const ModelFilePos& other) const
            {
            return _pos - other._pos;
            }

        void set(const long recordNumber, const short pos)
            {
            _pos = (recordNumber * 500) + pos - 1;
            }
    };

class MXModelFile;

class MXTriangleString;
class DLL ModelObject : public MXRunTimeObject
    {
    friend MXTriangleString;
    public:
        enum OpenStatus
            {
            Notify,
            Read,
            Write
            };
    private:
        int opened;
        int erased;
        int modified;
        int _delayWrite;
        bool _new;
        bool _fileNeedsUpdating;
    protected:
        ModelFilePos _pos;
        MXModelFile* _modelFile;
        ModelObjectId _id;
#ifdef _MANAGED
        ObjectNotify* _notifyObj;
#endif
        virtual void detach();
    public:
#ifdef _MANAGED
#ifdef MXMODEL_EXPORTS
        ObjectNotify* getObjectNotify()
            {
            return _notifyObj;
            }
#endif
#endif
        void setModelFilePos(const ModelFilePos& pos)
            {
            _pos = pos;
            }
        ModelFilePos getModelFilePos() const
            {
            return _pos;
            }
        void setModelFile(MXModelFile* modelFile);
        MXModelFile* getModelFile()
            {
            return _modelFile;
            }
        ModelObject();
        virtual ~ModelObject();

        virtual ErrorStatus open(OpenStatus mode);
        virtual ErrorStatus close();
        virtual ErrorStatus erase(int erase = 1);
        ErrorStatus upgradeOpen();
        ErrorStatus downgradeOpen();
        virtual void setDelayWrite(bool val);
        bool isDelayWrite(void)
            {
            return _delayWrite != 0;
            }

        void assertReadEnabled() const
            {
            if(opened == 0)
                assert(0);
            }
        void assertWriteEnabled();
        int isInModelFile() const
            {
            return _modelFile != 0;
            }
        int isErased() const
            {
            return erased;
            }
        int isModified() const
            {
            return modified;
            }
        int isNewRecord() const
            {
            return _pos.getRecordNumber() == 0 && !erased;
            }
        int isOpenedForWrite() const
            {
            return opened < 0;
            }
        int isOpened() const
            {
            return opened != 0;
            }
        ModelObjectId objectId() const
            {
            return _id;
            }
        bool isNew() const
            {
            return _new;
            }
        void clearNew()
            {
            _new= false;
            }
        void UpdateHeader()
            {
            UpdateData();
            }
    protected:
        virtual void UpdateData();
        virtual void UpdateFile();
        void Release();
        void setNeedsUpdating()
            {
            _fileNeedsUpdating = true;
            }
        void resetNeedsUpdating()
            {
            _fileNeedsUpdating = false;
            }
        bool NeedsUpdating() const
            {
            return _fileNeedsUpdating;
            }
    };

ErrorStatus DLL MXOpenModelObject(const ModelObjectId& id, ModelObject*& object, ModelObject::OpenStatus status);

struct MFUsedSpace
    {
    ModelFilePos start;
    ModelFilePos end;
    };
#ifdef FREESPACEMAPPER
class DLL ModelFileFreeSpace
    {
    ArrayPtrClass<MFUsedSpace> _usedSpace;

    int findPos(ModelFilePos& start);
    public:
        bool addBlock(ModelFilePos& start, ModelFilePos& end);
        void removeBlock(ModelFilePos& start, ModelFilePos& end);
        bool getFreeData(long size, ModelFilePos& pos, bool startNewRecord) const;
        void report(void) const;
    };
#endif

class ModelTableIterator;
class ModelTable;
class DLL MXModelFile
    {
    private:
        __int64 _pos;
        inline BeFile* fopen (WCharCP filename, char* type)
            {
            BeFile* hand = new BeFile ();

            BeFileStatus status;
            if (strcmp (type, "rb") == 0)
                status = hand->Open (filename, BeFileAccess::Read);
            else if(strcmp(type, "r+b") == 0)
                status = hand->Open (filename, BeFileAccess::ReadWrite);
            else
                status = hand->Create (filename, true);
            if(status != BeFileStatus::Success)
                hand = 0;
            _pos = 0;
            return hand;
            }

        inline void fclose (BeFile*& fp)
            {
            fp->Close ();
            delete fp;
            fp = nullptr;
            }

        inline void fseek (BeFile* fp, const uint64_t& offset, int origin)
            {
            //SEEK_CUR
            //SEEK_END
            //SEEK_SET
            fp->SetPointer (offset, BeFileSeekOrigin::Begin);
            _pos = offset;
            }

        inline uint64_t ftell(BeFile* fp)
            {
            return _pos;
            }

        inline void fread(void* data, unsigned long size, long num, BeFile* fp)
            {
            uint32_t numRead;

            fp->Read (data, &numRead, size * num);
            _pos += numRead;
            }

        inline void fwrite (void* data, unsigned long size, long num, BeFile* fp)
            {
            uint32_t numWritten;

            fp->Write (&numWritten, data, size * num);
            _pos += numWritten;
            }

        ModelTable* _modelTable;
        WString _filename;
        BeFile* _fp;

        ModelFilePos _nextDataRecord;
        int _nextRecord;
        int _maxRecord;
#ifdef NOTNEEDED // _MANAGED
        intptr_t _filePtr;
#endif
        short _modelFileNumber;
        bool _readonly;
#ifdef FREESPACEMAPPER
        ModelFileFreeSpace* _freeSpaceMapper;
        bool _noFreeSpaceMapper;
#endif
        double _modelShiftX;
        double _modelShiftY;
        bool _swapXY;
        bool _convertToWorld;
        bool _swapNeg;
        bool _doScaleFactor;
        double _scaleFactor;
        // Functions
    private:
        void initalize(void);
    public:

        MXModelFile();
        virtual ~MXModelFile();

#ifdef NOTNEEDED //_MANAGED
        void setFilePtr(intptr_t fileptr)
            {
            _filePtr = fileptr;
            }
        intptr_t getFilePtr() const
            {
            return _filePtr;
            }
#endif

        ErrorStatus UpgradeOpen();
        ErrorStatus Close();
        ErrorStatus Open(WCharCP filename, bool readonly = true, bool create = false);
        ErrorStatus getModelTable(ModelTable*& modelTable, ModelObject::OpenStatus mode);
        ErrorStatus getModelTable(ModelObjectId& modelTableId);
        bool isReadOnly(void) const
            {
            return _readonly;
            }
#ifdef FREESPACEMAPPER
        bool getNoFreeSpaceMapper(void) const
            {
            return _noFreeSpaceMapper;
            }
        void setNoFreeSpaceMapper(bool newVal)
            {
            _noFreeSpaceMapper = newVal;
            }
        bool isReusingFreeSpace(void) const
            {
            return _freeSpaceMapper != NULL;
            }
#endif
        int readRecordOffsetAndSize(void* buffer, int recordNumber, int offset, int size);
        int readRecord(ModelFileRecord* buffer, const int recordNumber);
        int readRecord2(ModelFileRecord* buffer, const int recordNumber);
        int readRecords(ModelFileRecord* buffer, const int recordNumber, int numRecords);
        int writeRecords(ModelFileRecord* buffer, int recordNumber, int numRecords);
        int writeRecord(ModelFileRecord* buffer, const int recordNumber);
        void updateNextRecord(int nextRecordNumber)
            {
            if(_nextRecord < nextRecordNumber)
                {
                _nextRecord = nextRecordNumber;
                _nextDataRecord.set(_nextRecord, 1);
                updateNextRecord();
                }
            }
        void updateNextRecord(void);
        long NextRecord()
            {
            return _nextRecord;
            }
        void getModelShift(double& x, double& y) const
            {
            x = _modelShiftX;
            y = _modelShiftY;
            }
        void setModelShift(const double& x, const double& y)
            {
            _modelShiftX = x;
            _modelShiftY = y;
            }

        double getScaleFactor() const
            {
            return _scaleFactor;
            }

        void setScaleFactor(double value)
            {
            _scaleFactor = value;
            _doScaleFactor = (fabs(value - 1.0) > 0.0001);
            }
        int addRecord(void)
            {
            ModelFilePos pos;
            getFreeData(500, pos, true);
            int recordNumber = pos.getRecordNumber(); //_nextRecord++;
            if(_nextRecord <= recordNumber)
                {
                _nextRecord = recordNumber + 1;
                updateNextRecord();
                _nextDataRecord.set(_nextRecord, 1);
                }
            return recordNumber;
            }
        int addRecord(ModelFileRecord* buffer)
            {
            int recordNumber = addRecord();
            writeRecord(buffer, recordNumber);
            return recordNumber;
            }

        int readData(void* data, const ModelFilePos& pos, const unsigned long dataSize);
        int writeData(void* data, const ModelFilePos& pos, const unsigned long dataSize);
        void getFreeData(unsigned long dataSize, ModelFilePos& pos, bool startNewRecord);
        short getModelFileNumber() const
            {
            return _modelFileNumber;
            }
        void ReleaseModelFileNumber()
            {
            _modelFileNumber = 0;
            }
#ifdef FREESPACEMAPPER
        void AddUsedRecord(const int recNum)
            {
            if(_freeSpaceMapper)
                {
                ModelFilePos l(recNum, 1);
                ModelFilePos l2(recNum + 1, 1);
                if(!_freeSpaceMapper->addBlock(l, l2))
                    {
                    delete _freeSpaceMapper;
                    _freeSpaceMapper = NULL;
                    }
                }
            }
        void AddUsedData(ModelFilePos& l, int size)
            {
            if(_freeSpaceMapper)
                {
                ModelFilePos l2 = l;
                l2 += size;
                if(!_freeSpaceMapper->addBlock(l, l2))
                    {
                    delete _freeSpaceMapper;
                    _freeSpaceMapper = NULL;
                    }
                }
            else
                {
                ModelFilePos pos = l;
                pos += size;

                if(_nextDataRecord < pos)
                    _nextDataRecord = pos;
                if(pos.getRecordNumber() >= _nextRecord)
                    {
                    _nextRecord = pos.getRecordNumber() + 1;
                    updateNextRecord();
                    }
                }
            }
        void RemoveUsedData(ModelFilePos& l, int size)
            {
            if(_freeSpaceMapper)
                {
                ModelFilePos l2 = l;
                l2 += size;
                _freeSpaceMapper->removeBlock(l, l2);
                }
            }
        void ReportModelFileMapper(void)
            {
            if(_freeSpaceMapper)
                _freeSpaceMapper->report();
            }
#endif
        ErrorStatus SetModelToWorld(eCoordSystems modelCoord, eCoordSystems outputCoord = eNoSystem, double scaleFactor = 1);
        void ConvertAngleToWorld(double& angle)
            {
            if(_swapXY)
                {
                angle = ((2 * MX_PI) - angle) + (MX_PI / 2);
                }
            if(_swapNeg)
                {
                angle -= MX_PI;
                }
            }
        void ConvertAngleToModel(double& angle)
            {
            if(_swapXY)
                {
                angle = ((2 * MX_PI) - (angle - (MX_PI / 2)));
                }
            if(_swapNeg)
                {
                angle -= MX_PI;
                }
            }
        void ConvertRadiusToWorld(double& radius)
            {
            if(_swapXY)
                {
                if(radius < 999999)
                    radius = -radius;
                }
            if(radius < 999999 && _doScaleFactor)
                radius *= _scaleFactor;
            }
        void ConvertRadiusToModel(double& radius)
            {
            if(_swapXY)
                {
                if(radius < 999999)
                    radius = -radius;
                }
            if(radius < 999999 && _doScaleFactor)
                radius /= _scaleFactor;
            }
        void ScaleUnitToWorld(double & value)
            {
            if(_doScaleFactor)
                value *= _scaleFactor;
            }
        void ScaleUnitToModel(double & value)
            {
            if(_doScaleFactor)
                value /= _scaleFactor;
            }
        eDiscontinuity GetDiscontinuity(const double& x, const double& y)
            {
            if(x < 0)
                {
                if(y < 0)
                    {
                    return eBearing;
                    }
                return eStart;
                }
            else if(y < 0)
                {
                return eEnd;
                }
            return eNone;
            }
        bool ConvertToWorld()
            {
            return _convertToWorld;
            }
        bool swapXY() const
            {
            return _swapXY;
            }
        void setSwapXY(bool value)
            {
            _swapXY = value;
            }

        eDiscontinuity ConvertXYToWorld(double& x, double& y)
            {
            eDiscontinuity ret;

            if(x < 0)
                {
                if(y < 0)
                    {
                    ret = eBearing; // Bearing
                    y = -y;
                    }
                else
                    ret = eStart;
                x = -x;
                }
            else if(y < 0)
                {
                ret = eEnd;
                y = -y;
                }
            else
                ret = eNone;

            x -= _modelShiftX;
            y -= _modelShiftY;
            if(_swapXY)
                {
                double tmp = x;
                x = y;
                y = tmp;
                }
            if(_swapNeg)
                {
                x = -x;
                y = -y;
                }
            if(_doScaleFactor)
                {
                x *= _scaleFactor;
                y *= _scaleFactor;
                }

            return ret;
            }

        eDiscontinuity ConvertXYZToWorld(double& x, double& y, double& z)
            {
            eDiscontinuity ret;

            if(x < 0)
                {
                if(y < 0)
                    {
                    ret = eBearing; // Bearing
                    y = -y;
                    }
                else
                    ret = eStart;
                x = -x;
                }
            else if(y < 0)
                {
                ret = eEnd;
                y = -y;
                }
            else
                ret = eNone;

            x -= _modelShiftX;
            y -= _modelShiftY;
            if(_swapXY)
                {
                double tmp = x;
                x = y;
                y = tmp;
                }
            if(_swapNeg)
                {
                x = -x;
                y = -y;
                }
            if(_doScaleFactor)
                {
                x *= _scaleFactor;
                y *= _scaleFactor;
                if (z >= -998)
                    z *= _scaleFactor;
                }

            return ret;
            }

        void ConvertXYZToModel(double& x, double& y, double& z, eDiscontinuity disco)
            {
            if(_swapXY)
                {
                double tmp = x;
                x = y;
                y = tmp;
                }
            x += _modelShiftX;
            y += _modelShiftY;
            switch(disco)
                {
                case eStart:
                    x = -x;
                    break;
                case eBearing:
                    x = -x;
                case eEnd:
                    y = -y;
                    break;
                }
            if(_swapNeg)
                {
                x = -x;
                y = -y;
                }
            if(_doScaleFactor)
                {
                x /= _scaleFactor;
                y /= _scaleFactor;
                if (z >= -998)
                    z /= _scaleFactor;
                }
            }
        void ConvertXYToModel(double& x, double& y, eDiscontinuity disco)
            {
            if(_swapXY)
                {
                double tmp = x;
                x = y;
                y = tmp;
                }
            x += _modelShiftX;
            y += _modelShiftY;
            switch(disco)
                {
                case eStart:
                    x = -x;
                    break;
                case eBearing:
                    x = -x;
                case eEnd:
                    y = -y;
                    break;
                }
            if(_swapNeg)
                {
                x = -x;
                y = -y;
                }
            if(_doScaleFactor)
                {
                x /= _scaleFactor;
                y /= _scaleFactor;
                }
            }
    };

class StringTable;

class DLL ModelTableRecord : public ModelObject
    {
    friend ModelTable;
    friend StringTable;
    protected:
        char _modelName[29];
        char _modelType[4];
        ModelFilePos _stringTablePos;
        long _date;
        ModelTable* _modelTable;
        StringTable* _stringTable;

    public:
        MXRT_DECLARE_MEMBERS(ModelTableRecord);
        ModelTableRecord();
        virtual ~ModelTableRecord();
        void set(const ModelTableFileRecord& modelRecord);

        const char* const modelName() const
            {
            assertReadEnabled();
            return _modelName;
            }
        ErrorStatus setModelName(const char* const modelName);
        ErrorStatus setModelType(const char* const modelType);
        const char* const modelType(void) const
            {
            assertReadEnabled();
            return _modelType;
            }
        bool secured(void) const
            {
            return _date < 0;
            }
        void setSecured(bool secured)
            {
            assertWriteEnabled();
            if((secured && _date > 0) || (!secured && _date < 0))
                _date = -_date;
            }
        int date() const
            {
            assertReadEnabled();
            return _date;
            }

        virtual ErrorStatus close();
        virtual ErrorStatus erase(int erase = 1);
        ErrorStatus getStringTable(StringTable*& stringTable, ModelObject::OpenStatus mode);
        ErrorStatus getStringTable(ModelObjectId& stringTableId);
        char* getDate(char* retString = NULL);
        static int getCurrentDate();
    protected:
        void setModelTable(ModelTable* modelTable)
            {
            assertWriteEnabled();
            _modelTable = modelTable;
            }
        virtual void UpdateData();
        virtual void UpdateFile();
        virtual void setDelayWrite(bool l)
            {
            }
    };

class ModelTableIterator;
class DLL ModelTable : public ModelObject
    {
    friend ModelTableRecord;
    friend ModelTableIterator;
    private:
        ArrayClass<ModelTableRecord*> _models;
        int getModelIndex(const char* const modelName, bool getDeleted = false) const;
    public:
        MXRT_DECLARE_MEMBERS(ModelTable);
        ModelTable(MXModelFile* modelFile);
        virtual ~ModelTable();
        ModelTableIterator* newIterator();
        static void DisposeU(ModelTableIterator *pModelTableIterator);
        ErrorStatus getModel(const char* const modelName, ModelTableRecord*& modelTableRecord, ModelObject::OpenStatus mode, bool getDeleted = false);
        ErrorStatus getModel(const char* const modelName, ModelObjectId& modelTableObjectId, bool getDeleted = false);

        ErrorStatus addModel(const char* const modelName, ModelTableRecord*& mr, const char* const modelType = NULL);
        virtual void setDelayWrite(bool l)
            {
            }
#ifdef MXACHIVE
        virtual ErrorStatus Archive(char* modelMask, unsigned long& size)
            {
            void* data = NULL;
            return Archive(modelMask, size, data);
            }
        virtual ErrorStatus Archive(char* modelMask, unsigned long& size, void*& data);
        virtual ErrorStatus Retrieve(void*& data);
#endif
    };

class DLL ModelTableIterator
    {
    private:
        ModelTable* _modelTable;
        int _index;
        bool _includeDeleted;
    public:
        ModelTableIterator(ModelTable* modelTable);
        bool includeDeleted(void) const
            {
            return _includeDeleted;
            }
        void setIncludeDeleted(const bool newValue)
            {
            _includeDeleted = newValue;
            }
        int done(void) const;
        void next(void);
        void start(void);
        ErrorStatus getRecord(ModelTableRecord*& modelTableRecord, ModelObject::OpenStatus mode);
        ErrorStatus getRecord(ModelObjectId& modelTableRecordId);
    };

class StringTable;
class DLL StringTableRecord : public ModelObject        // Proxy Just so you can get the information.
    {
    friend StringTable;
    protected:
        void* _stringData;
        char _stringName[5];
        char _subReference[5];
        long _type;
        long _numPoints;
        long _minx;
        long _miny;
        long _maxx;
        long _maxy;
        ModelFilePos _stringTableRecordDataPos;

        ModelTableRecord* _modelTableRecord;
        ModelTable* _modelTable;
        StringTable* _stringTable;
        unsigned long _originalDataSize;
        bool _loadedData;

        void resetFilePos(unsigned long newDataSize, bool startNewRecord = false)
            {
            if(newDataSize == _originalDataSize || !_modelFile)
                return;

#ifdef FREESPACEMAPPER
            if(_originalDataSize)
                _modelFile->RemoveUsedData(_stringTableRecordDataPos, _originalDataSize / 4);
#endif
            // Need to write out stringTable to TriangleString ???
            if(_originalDataSize == 0 || _originalDataSize < newDataSize)
                {
                _originalDataSize = newDataSize;
                _modelFile->getFreeData(newDataSize / 4, _stringTableRecordDataPos, startNewRecord);
                }
            else
                _originalDataSize = newDataSize;
#ifdef FREESPACEMAPPER
            _modelFile->AddUsedData(_stringTableRecordDataPos, _originalDataSize / 4);
#endif
            }

        virtual unsigned long dataSize(void) const
            {
            return recordSize() * _numPoints;
            }
        virtual void detach();
    public:
        MXRT_DECLARE_MEMBERS(StringTableRecord);
        StringTableRecord();
        virtual ~StringTableRecord();
        static void DisposeU(MXStringDimData *pMXStringDimData);
        void set(const StringTableFileRecord& data, bool reallyLoad);


        ModelFilePos StringTableRecordDataPos()
            {
            return _stringTableRecordDataPos;
            }
        void set_type(long type)
            {
            assertWriteEnabled();
            int extType = get_extType();
            _type = type + (extType * 100000000);
            }
        long get_type() const
            {
            return (_type % 100000000);
            }
        void set_extType(long type)
            {
            assertWriteEnabled();
            _type = (_type % 100000000) + type * 100000000;
            }
        long get_extType()
            {
            return (_type / 100000000);
            }

        void setModelTable(ModelTable* modelTable)
            {
            assertWriteEnabled();
            _modelTable = modelTable;
            }
        void setModelTableRecord(ModelTableRecord* modelTableRecord)
            {
            assertWriteEnabled();
            _modelTableRecord = modelTableRecord;
            }
        void setStringTable(StringTable* stringTable)
            {
            assertWriteEnabled();
            _stringTable = stringTable;
            }

        ErrorStatus getModelTableRecordId(ModelObjectId& mtrId) const
            {
            //                        assertReadEnabled();
            if(_modelTableRecord)
                mtrId = _modelTableRecord->objectId();
            else
                return eNotInModelFile;
            return eOk;
            }
        ErrorStatus GetModelTableRecord(ModelTableRecord*& mtr, ModelObject::OpenStatus openMode) const
            {
            assertReadEnabled();
            if(!_modelTableRecord)
                return eNotInModelFile;
            return MXOpenModelObject(_modelTableRecord->objectId(), (ModelObject*&)mtr, openMode);
            }
        const char* const stringName() const
            {
            assertReadEnabled();
            return _stringName;
            }
        ErrorStatus setStringName(const char* const stringName);
        const char* const subReference() const
            {
            assertReadEnabled();
            return _subReference;
            }
        void setSubReference(const char* const subReference)
            {
            assertWriteEnabled();
            strncpy(_subReference, subReference, 4);
            }
        int numPoints() const
            {
            assertReadEnabled();
            return _numPoints;
            }
        void setNumPoints(const int i)
            {
            assertWriteEnabled();
            if(!_loadedData)
                loadData();

            _numPoints = i;
            SetNumPoints(i);
            }
        int type() const
            {
            assertReadEnabled();
            return _type;
            }
        int minx() const
            {
            assertReadEnabled();
            return _minx;
            }
        int miny() const
            {
            assertReadEnabled();
            return _miny;
            }
        int maxx() const
            {
            assertReadEnabled();
            return _maxx;
            }
        int maxy() const
            {
            assertReadEnabled();
            return _maxy;
            }

        void setExt(long minx, long miny, long maxx, long maxy)
            {
            assertWriteEnabled();
            _minx = minx;
            _miny = miny;
            _maxx = maxx;
            _maxy = maxy;
            }

        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual ErrorStatus loadData()
            {
            return eNotImplemented;
            }

        virtual int recordSize(void) const;
        virtual int dataRecordSize(void) const
            {
            return recordSize();
            }
        virtual ErrorStatus setRecord(const int i, void* data);
        virtual ErrorStatus getRecord(const int i, const void*& data);
        virtual int addRecord();
        virtual ErrorStatus removeRecord(const int i);
        virtual ErrorStatus insertRecord(const int i);

        virtual MXStringDimData* getDimData(void);
        virtual ErrorStatus setDimData(const MXStringDimData* dd);
        virtual ErrorStatus close(void);

#ifdef MXARCHIVE
        virtual ErrorStatus Archive(unsigned long& size)
            {
            void* data = NULL;
            return Archive(size, data);
            }
        virtual ErrorStatus Archive(unsigned long& size, void*& data);
        virtual ErrorStatus Retrieve(void*& data);
        virtual ErrorStatus getArchiveStringHeader(void* data);
#endif
        virtual ErrorStatus get(__int64 offset, unsigned long size, void* data);
        virtual ErrorStatus put(__int64 offset, unsigned long size, void* data);
        virtual ErrorStatus erase(int erase = 1);

        virtual void* stringData(void)
            {
            if(!_loadedData)
                loadData();
            return _stringData;
            }

        bool IsDataLoaded() const
            {
            return _loadedData;
            }
        virtual void emptyData(void);
    protected:
        virtual void UpdateData();
        virtual void UpdateFile();
        virtual void SetNumPoints(const int i);
    };

class DLL MXTriangleString : public StringTableRecord
    {
    private:
        ArrayClass<ElementTriangleString> _data;

        ModelFileRecord* _records;
        int* _recordNumbers;
        int _numberRecords;
        int* _timeNumbers;
        int* _recordMapper;

        void flushRecords(bool updateFile);
        virtual unsigned long dataSize(void) const
            {
            return sizeof(ElementTriangleString) * 10 * ((_numPoints + 9) / 10);
            }
        bool _autoConvert;

    public:
        MXRT_DECLARE_MEMBERS(MXTriangleString);
        MXTriangleString()
            {
            _type = 7709;
            strcpy(_subReference, "TRIN");
            _records = NULL;
            _numberRecords = 10;
            _recordNumbers = NULL;
            _recordMapper = NULL;
            _timeNumbers = NULL;
            _recordMapper = NULL;
            _autoConvert = true;
            }
        virtual ~MXTriangleString()
            {
            if(_records)
                delete [] _records;
            if(_recordNumbers)
                delete [] _recordNumbers;
            if(_timeNumbers)
                delete [] _timeNumbers;
            if(_recordMapper)
                delete [] _recordMapper;
            }
        virtual ErrorStatus loadData(ElementTriangleString* data);
        virtual ErrorStatus loadData();
        virtual ErrorStatus loadData(ArrayClass<ElementTriangleString>& data);
        virtual ErrorStatus loadData(MXTriangle* tri);
        virtual ErrorStatus loadData2(MXTriangle* tri);
        ErrorStatus close(void);
        ErrorStatus saveData(ElementTriangleString* _data);
        ErrorStatus saveData(ArrayClass<ElementTriangleString>& _data);
        ErrorStatus saveData(MXTriangle* tri);

        ArrayClass<ElementTriangleString>& getData(void)
            {
            return getTriangleData();
            }
        virtual ArrayClass<ElementTriangleString>& getTriangleData(void)
            {
            assertReadEnabled();
            return _data;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;

        void setNumberRecords(const int number)
            {
            _numberRecords = number;
            }
        virtual ErrorStatus getTriangle(const int triangleNumber, ElementTriangleString& data);
        ErrorStatus getTriangle(const int triangleNumber, ElementTriangleStringDoubles*& doubles, ElementTriangleStringInts*& ints, bool setWrite = FALSE);
        int MXTriangleString::loadTriangle(int recordNumber);

        virtual ErrorStatus setTriangle(const int triangleNumber, const ElementTriangleString& data);

//NOTNEEDED        ErrorStatus getBoundaryString(ArrayClass<ElementContourString>& PrimaryRegion);
#ifdef MXACHIVE
        virtual ErrorStatus getArchiveStringHeader(void* data);
        virtual ErrorStatus Archive(unsigned long& size, void*& data);
        virtual ErrorStatus Retrieve(void*& data);
#endif
        virtual ErrorStatus get(__int64 offset, unsigned long size, void* data);
        virtual ErrorStatus put(__int64 offset, unsigned long size, void* data);

        bool autoConvert() const
            {
            return _autoConvert;
            }
        void setAutoConvert(const bool newVal)
            {
            _autoConvert = newVal;
            }
        virtual void emptyData(void)
            {
            _loadedData = false;
            StringTableRecord::emptyData();
            _data.empty();
            }
    protected:
        virtual void UpdateData();
        virtual void UpdateFile();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = &_data[0];
            }
        virtual MXStringDimData* getDimData(void)
            {
            return NULL;
            }
        virtual ErrorStatus setDimData(const MXStringDimData* dd)
            {
            return eInvalidData;
            }
    };

class DLL MXTextString : public StringTableRecord
    {
    private:
        ArrayClass<ElementTextString> _data;

    public:
        MXRT_DECLARE_MEMBERS(MXTextString);
        MXTextString()
            {
            _type = 1706;
            }

        virtual ErrorStatus loadData();
        virtual int dataRecordSize(void) const
            {
            return sizeof (ElementTextString);
            }


        const ArrayClass<ElementTextString>& getTextData(void)
            {
            return getData();
            }
        ErrorStatus setTextData(ArrayClass<ElementTextString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<ElementTextString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }

        virtual ErrorStatus setData(ArrayClass<ElementTextString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = (int)points.size();
            _stringData = &_data[0];
            _loadedData = true;
            return eOk;
            }

        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;

#ifdef MXARCHIVE
        virtual ErrorStatus getArchiveStringHeader(void* data);
        virtual ErrorStatus Archive(unsigned long& size)
            {
            void* data = NULL;
            return Archive(size, data);
            }
        virtual ErrorStatus Archive(unsigned long& size, void*& data);
        virtual ErrorStatus Retrieve(void*& data);
#endif
        virtual ErrorStatus get(__int64 offset, unsigned long size, void* data);
        virtual ErrorStatus put(__int64 offset, unsigned long size, void* data);
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateFile();
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = &_data[0];
            }
    };

class DLL MXContourString : public StringTableRecord
    {
    private:
        ArrayClass<ElementContourString> _data;

    public:
        MXRT_DECLARE_MEMBERS(MXContourString);
        MXContourString()
            {
            _type = 7702;
            }

        virtual ErrorStatus loadData();

        double getContourHeight(void)
            {
            assertReadEnabled();
            return ((double)(*(long*)_subReference)) / 1000.0;
            }
        void setContourHeight(const double height)
            {
            assertWriteEnabled();
            (*(long*)_subReference) = (long)(height * 1000.0);
            }

        const ArrayClass<ElementContourString>& getContourData(void)
            {
            return getData();
            }
        ErrorStatus setPoints(const ArrayClass<ElementContourString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<ElementContourString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementContourString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

#ifdef MXALLSTRINGTYPES

class DLL MXCadastreString : public StringTableRecord
    {
    private:
        ArrayClass<ElementCadastreString> _data;

    public:
        MXRT_DECLARE_MEMBERS(MXCadastreString);
        MXCadastreString()
            {
            _type = 7710;
            memcpy(_subReference, "SHEE", 4);
            }

        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementCadastreString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementCadastreString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };


class DLL MX3dString : public StringTableRecord
    {
    private:
        ArrayClass<Element3dString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MX3dString);
        MX3dString()
            {
            _type = 7703;
            }
        virtual ErrorStatus loadData();

        const ArrayClass<Element3dString>& get3dData(void)
            {
            return getData();
            }
        ErrorStatus setPoints(const ArrayClass<Element3dString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<Element3dString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<Element3dString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXSurveyStationString : public StringTableRecord
    {
    private:
        ArrayClass<ElementSurveyStationString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXSurveyStationString);
        MXSurveyStationString()
            {
            _type = 7704;
            memcpy(_subReference, "SSTA", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementSurveyStationString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementSurveyStationString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXSurveyObservationString : public StringTableRecord
    {
    private:
        ArrayClass<ElementSurveyObservationString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXSurveyObservationString);
        MXSurveyObservationString()
            {
            _type = 704;
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementSurveyObservationString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementSurveyObservationString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXSectionString : public StringTableRecord
    {
    private:
        ArrayClass<ElementSectionString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXSectionString);
        MXSectionString()
            {
            _type = 7705;
            }
        virtual ErrorStatus loadData();

        const ArrayClass<ElementSectionString>& getSectionData(void)
            {
            return getData();
            }
        ErrorStatus setSectionData(const ArrayClass<ElementSectionString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<ElementSectionString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementSectionString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXInterfaceString : public StringTableRecord
    {
    private:
        ArrayClass<ElementInterfaceString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXInterfaceString);
        MXInterfaceString()
            {
            _type = 7705;
            memcpy(_subReference, "INTC", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementInterfaceString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementInterfaceString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXGeometryString : public StringTableRecord
    {
    private:
        ArrayClass<ElementGeometryString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXGeometryString);
        MXGeometryString()
            {
            _type = 777712;
            }
        virtual ErrorStatus loadData();

        const ArrayClass<ElementGeometryString>& getGeometryData(void)
            {
            return getData();
            }
        ErrorStatus setGeometryData(const ArrayClass<ElementGeometryString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<ElementGeometryString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementGeometryString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXMasterString : public StringTableRecord
    {
    private:
        ArrayClass<ElementMasterString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXMasterString);
        MXMasterString()
            {
            _type = 7706;
            }
        virtual ErrorStatus loadData();

        const ArrayClass<ElementMasterString>& getMasterData(void)
            {
            return getData();
            }
        ErrorStatus setMasterData(const ArrayClass<ElementMasterString>& points)
            {
            return setData(points);
            }
        virtual const ArrayClass<ElementMasterString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementMasterString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };
class DLL MXAreaString : public StringTableRecord
    {
    private:
        ArrayClass<ElementAreaString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXAreaString);
        MXAreaString()
            {
            _type = 177710;
            memcpy(_subReference, "AREA", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementAreaString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementAreaString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXEndAreaString : public StringTableRecord
    {
    private:
        ArrayClass<ElementEndAreaString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXEndAreaString);
        MXEndAreaString()
            {
            _type = 177710;
            memcpy(_subReference, "ENDA", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementEndAreaString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementEndAreaString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXVisibilityString : public StringTableRecord
    {
    private:
        ArrayClass<ElementVisibilityString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXVisibilityString);
        MXVisibilityString()
            {
            _type = 177710;
            memcpy(_subReference, "VISI", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementVisibilityString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementVisibilityString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };

class DLL MXVolumeString : public StringTableRecord
    {
    private:
        ArrayClass<ElementVolumeString> _data;
    public:

        MXRT_DECLARE_MEMBERS(MXVolumeString);
        MXVolumeString()
            {
            _type = 177710;
            memcpy(_subReference, "VOLM", 4);
            }
        virtual ErrorStatus loadData();

        virtual const ArrayClass<ElementVolumeString>& getData(void)
            {
            assertReadEnabled();
            if(!_loadedData)
                loadData();
            return _data;
            }
        virtual ErrorStatus setData(const ArrayClass<ElementVolumeString>& points)
            {
            assertWriteEnabled();
            _data = points;
            _numPoints = points.size();
            _stringData = _data.getArrayPtr();
            _loadedData = true;
            return eOk;
            }
        virtual ErrorStatus copy(StringTableRecord*& clonedString) const;
        virtual void emptyData(void)
            {
            _loadedData = false;
            _stringData = NULL;
            _data.empty();
            }
    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i)
            {
            _data[i - 1];
            _stringData = _data.getArrayPtr();
            }
    };
#endif

typedef bmap<long, StringTableRecord*> StringMap;
class StringTableIterator;
class DLL StringTable : public ModelObject
    {
    friend StringTableIterator;
    friend StringTableRecord;
    private:
        bvector<int> _recordNumbers;
        //                    ArrayClass<StringTableRecord*> _strings;
        StringMap _strings;
        bool _isReallyLoaded;
        ModelTableRecord* _modelTableRecord;
        StringMap::const_iterator getStringIndex(const char* stringName) const;
        void DoLoad(bool reallyLoad);
    public:
        MXRT_DECLARE_MEMBERS(StringTable);
        StringTable(ModelFilePos& stringRecordPos, MXModelFile* modelfile, ModelTableRecord* modelTableRecord);
        virtual ~StringTable();
        StringTableIterator* newIterator();
        static void DisposeU(StringTableIterator *pStringTableIterator);
        ErrorStatus getString(const char* const stringName, StringTableRecord*& stringTableRecord, ModelObject::OpenStatus mode, bool getDeleted = false);
        ErrorStatus getString(const char* const stringName, ModelObjectId& stringTableRecordId, bool getDeleted = false);
        ErrorStatus addString(const char* const stringName, StringTableRecord* str);

#ifdef MXARCHIVE
        virtual ErrorStatus Archive(char* stringMask, unsigned long& size)
            {
            void* data = NULL;
            return Archive(stringMask, size, data);
            }
        virtual ErrorStatus Archive(char* stringMask, unsigned long& size, void*& data);
        virtual ErrorStatus Retrieve(void*& data);
        ErrorStatus CreateStringFromArchiveStringHeader(StringTableRecord*& mr, void* data);
#endif
    protected:
        virtual void UpdateData();
        virtual void UpdateFile();
    };

class DLL StringTableIterator
    {
    private:
        StringTable* _stringTable;
        StringMap::iterator _index;
        bool _includeDeleted;
    public:
        StringTableIterator(StringTable* stringTable);
        bool includeDeleted(void) const
            {
            return _includeDeleted;
            }
        void setIncludeDeleted(const bool newValue)
            {
            _includeDeleted = newValue;
            }

        int done(void) const;
        void next(void);
        void start(void);
        ErrorStatus getRecord(StringTableRecord*& stringTableRecord, ModelObject::OpenStatus mode);
        ErrorStatus getRecord(ModelObjectId& stringTableRecordId);
    };

inline void ModelObject::setModelFile(MXModelFile* modelFile)
    {
    _modelFile = modelFile;
    _id.modelFileNumber = _modelFile->getModelFileNumber();
    }

class DLL MXProxyString : public StringTableRecord
    {
    private:
        int _numPointsInData;
    public:
        MXRT_DECLARE_MEMBERS(MXProxyString);
        MXProxyString()
            {
            _numPointsInData = 0;
            }
        MXProxyString(int sci)
            {
            _numPointsInData = 0;
            _type = sci;
            }
        virtual ~MXProxyString();
        virtual ErrorStatus loadData();

    protected:
        virtual void UpdateData();

        virtual void SetNumPoints(const int i);
        virtual void emptyData();
    };

struct MFDimData
    {
    int offset;
    int size;
    };

class DLL MXStringDimData
    {
    private:
        unsigned char* _data;
        int _recSize;
        int _numPoints;
        ArrayClass<MFDimData> _dimData;
        void InitArray(const int type, const int numPoints);
    public:
        MXStringDimData();
        MXStringDimData(unsigned char* _data, const int type, const int numPoints);
        MXStringDimData(const int type, const int numPoints);
        void Init(const int type, const int numPoints);
        ~MXStringDimData()
            {
            if(_data)
                delete [] _data;
            }
        ErrorStatus MXStringDimData::getDouble(const int pointNum, const int dim, double& value) const;
        ErrorStatus MXStringDimData::getInt(const int pointNum, const int dim, int& value) const;
        ErrorStatus MXStringDimData::getText(const int pointNum, const int dim, const char*& data) const;

        ErrorStatus MXStringDimData::setDouble(const int pointNum, const int dim, double value);
        ErrorStatus MXStringDimData::setInt(const int pointNum, const int dim, int value);
        ErrorStatus MXStringDimData::setText(const int pointNum, const int dim, const char* data);

        int numDimensions(void) const
            {
            return (int)_dimData.size();
            }
        int numPoints(void) const
            {
            return _numPoints;
            }
        void setNumPoints(const int value);
        int type(void) const;
        const unsigned char* const data(void) const
            {
            return _data;
            }
        void getMinMax(long& minx, long& miny, long& maxx, long& maxy) const;
    };

class DLL MXMaskTable
    {
    public:
        MXMaskTable();
        virtual ~MXMaskTable();

        void loadMaskTable(WCharCP fileName);
        enum eStringMask
            {
            eNotInMask = 0,
            eNormal = 1,
            eAsPString = 2,
            eLinkedString = 3
            };

        eStringMask GetStringMask(StringTableRecord* string) const;
        eStringMask GetStringMask(char* stringName, char* subRef) const;

    private:
        struct maskTableEntry
            {
            long stringMask;
            long subRef;
            eStringMask type;
            };

        ArrayClass<maskTableEntry> _maskTable;
    };

#ifdef MXEVENTS
class ModelFileEvents
    {
    public:
        virtual void StringModified(StringTableRecord* str) = 0;

        virtual void StringRenaming(StringTableRecord* str, char* newName) = 0;

        virtual void StringAdded(ModelTableRecord* mtr, StringTableRecord* str) = 0;
        virtual void StringErased(ModelTableRecord* mtr, StringTableRecord* str) = 0;
        virtual void ModelAdded(ModelTableRecord* mtr) = 0;
        virtual void ModelErased(ModelTableRecord* mtr) = 0;
        virtual void ModelRenaming(ModelTableRecord* mtr, char* newName) = 0;
    };

void AddModelFileEventHandler(ModelFileEvents* mfeh);
void RemoveModelFileEventHandler(ModelFileEvents* mfeh);
#endif

END_BENTLEY_TERRAINMODEL_NAMESPACE
