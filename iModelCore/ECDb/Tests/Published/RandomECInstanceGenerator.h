//#pragma once

#include <Logging/BentleyLogging.h>
#include <Bentley/Bentley.h>
#include <BeSQLite/BeSQLite.h>
#include <ECDb/ECDbApi.h>
#include <Logging/BentleyLogging.h>
#include <Geom/GeomApi.h>
#include <ECObjects/ECObjectsAPI.h>
#include <set>


struct IPropertyValueGenerator
{
protected:
    virtual BentleyStatus _NextInteger(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextDouble(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextLong(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextString(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextPoint2d(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextPoint3d(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextBoolean(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextDateTime(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextBinary(ECN::ECValueR value) = 0;
    virtual BentleyStatus _NextIGeometry(ECN::ECValueR value) = 0;
    virtual void _Reset() = 0;
public:
    BentleyStatus NextPrimitiveValue(ECN::ECValueR value, ECN::PrimitiveType valueType);
    void Reset();
};

struct DefaultPropertyValueGenerator : IPropertyValueGenerator
{

private:
    int32_t     m_seedInt32, m_currentInt32, m_incrementInt32;
    int64_t     m_seedLong, m_currentLong, m_incrementLong;
    double      m_seedDouble, m_currentDouble, m_incrementDouble;
    DateTime    m_seedDateTime;
    uint64_t    m_currentDateTimeJulian, m_incrementDateTimeJulian;
    DPoint2d    m_seedPoint2d, m_currentPoint2d, m_incrementPoint2d;
    DPoint3d    m_seedPoint3d, m_currentPoint3d, m_incrementPoint3d;
    Utf8String  m_stringPrefix;
    bool        m_seedBoolean, m_currentBoolean;
    size_t      m_binaryLength;
    ECN::ECPropertyCP m_property;
protected:
    virtual BentleyStatus _NextInteger(ECN::ECValueR value) override;
    virtual BentleyStatus _NextDouble(ECN::ECValueR value) override;
    virtual BentleyStatus _NextLong(ECN::ECValueR value) override;
    virtual BentleyStatus _NextString(ECN::ECValueR value) override;
    virtual BentleyStatus _NextPoint2d(ECN::ECValueR value) override;
    virtual BentleyStatus _NextPoint3d(ECN::ECValueR value) override;
    virtual BentleyStatus _NextBoolean(ECN::ECValueR value) override;
    virtual BentleyStatus _NextDateTime(ECN::ECValueR value) override;
    virtual BentleyStatus _NextBinary(ECN::ECValueR value) override;
    virtual BentleyStatus _NextIGeometry(ECN::ECValueR value) override;
    virtual void _Reset() override;

    void Init();
public:

    DefaultPropertyValueGenerator(ECN::ECPropertyCP templateProperty);
    void SetSeedInt32(int32_t seed, int32_t increment);
    void SetSeedLong(int64_t seed, int64_t increment);
    void SetSeedDouble(double seed, double increment);
    void SetSeedPoint2D(DPoint2dCR seed, DPoint2d increment);
    void SetSeedPoint3D(DPoint3dCR seed, DPoint3d increment);
    void SetBinaryLength(size_t length);
    void SetStringPrefix(Utf8CP prefix);
    void SetSeedDateTime(DateTime seed, uint64_t julianHectoSecondsIncrement);
    void SetSeedBoolean(bool seed);
    ECN::ECPropertyCP GetProperty() const { return m_property; }
};

struct RandomPropertyValueGenerator : IPropertyValueGenerator
{
private:
    size_t      m_binaryLength;

protected:
    virtual BentleyStatus _NextInteger(ECN::ECValueR value) override;
    virtual BentleyStatus _NextDouble(ECN::ECValueR value) override;
    virtual BentleyStatus _NextLong(ECN::ECValueR value) override;
    virtual BentleyStatus _NextString(ECN::ECValueR value) override;
    virtual BentleyStatus _NextPoint2d(ECN::ECValueR value) override;
    virtual BentleyStatus _NextPoint3d(ECN::ECValueR value) override;
    virtual BentleyStatus _NextBoolean(ECN::ECValueR value) override;
    virtual BentleyStatus _NextDateTime(ECN::ECValueR value) override;
    virtual BentleyStatus _NextBinary(ECN::ECValueR value) override;
    virtual BentleyStatus _NextIGeometry(ECN::ECValueR value) override;
    virtual void _Reset() override;

public:
    RandomPropertyValueGenerator();
};

struct PropertyValueGeneratorFactory
{
private:
    virtual std::unique_ptr<IPropertyValueGenerator> _CreateInstance(ECN::ECPropertyCP templateProperty)
    {
        return std::unique_ptr<IPropertyValueGenerator>(new DefaultPropertyValueGenerator(templateProperty));
        //        return std::unique_ptr<IPropertyValueGenerator>(new RandomPropertyValueGenerator());
    }

public:

    std::unique_ptr<IPropertyValueGenerator> CreateInstance(ECN::ECPropertyCP templateProperty)
    {
        return _CreateInstance(templateProperty);
    }
};

struct RandomECInstanceGenerator
{
public:

    struct Param
    {
        struct RangeValue
        {
        private:
            int m_min;
            int m_max;
        public:
            explicit RangeValue(int min = 0, int max = 0) : m_min(min), m_max(max){}
            int GetMin() const { return m_min; }
            int GetMax() const { return m_max; }
            int Delta()const { return GetMax() - GetMin(); }
            bool BetweenI(int n) { return n >= GetMin() && n <= GetMax(); }
            bool BetweenE(int n) { return n > GetMin() && n < GetMax(); }
            bool IsValid() const { return GetMin() <= GetMax(); }
        };
    private:
        float                   m_probabilityOfNullValuesPerClass;  //percentage of properties in a given class to have a value            
        RangeValue              m_rRegularInstancesPerClass;
        RangeValue              m_rRelationshipInstancesPerClass;
        RangeValue              m_rStructInstancesPerClass;
        RangeValue              m_rArrayElements;
        bool                    m_bGenerateInstanceOfEmptyClasses;
        bool                    m_bGenerateInstanceForNoneDomainClasses;
        std::vector<ECN::ECClassCP>  m_classes;
        std::unique_ptr<PropertyValueGeneratorFactory> m_valueGeneratorFactory;

        Param(const Param&){}
        Param& operator=(const Param&) { return *this; }

    public:
        Param()
        {
            m_probabilityOfNullValuesPerClass = 0.2f;
            m_rRegularInstancesPerClass = RangeValue(3, 3);
            m_rRelationshipInstancesPerClass = RangeValue(6, 6);
            m_rStructInstancesPerClass = RangeValue(3, 3);
            m_rArrayElements = RangeValue(3, 3);
            m_bGenerateInstanceOfEmptyClasses = true;
            m_bGenerateInstanceForNoneDomainClasses = true;
            m_valueGeneratorFactory = std::unique_ptr<PropertyValueGeneratorFactory>(new PropertyValueGeneratorFactory());
        }

        bool GenerateInstancesForEmptyClasses() const  { return m_bGenerateInstanceOfEmptyClasses; }
        bool GenerateInstancesForNoneDomainClasses() const  { return m_bGenerateInstanceForNoneDomainClasses; }

        float GetProbabilityOfNullValuesPerClass() const { return m_probabilityOfNullValuesPerClass; }
        RangeValue& GetNumberOfRegularInstancesToGeneratePerClass() { return m_rRegularInstancesPerClass; }
        RangeValue& GetNumberOfRelationshipInstancesToGeneratePerClass() { return m_rRelationshipInstancesPerClass; }
        RangeValue& GetNumberOfStructInstancesToGeneratePerClass() { return m_rStructInstancesPerClass; }
        RangeValue& GetNumberOfArrayElementsToGenerate() { return m_rArrayElements; }
        PropertyValueGeneratorFactory* GetValueGeneratorFactory() { return m_valueGeneratorFactory.get(); }
        std::vector<ECN::ECClassCP>&  GetClassList() { return m_classes; }
        void SetClassList(std::vector<ECN::ECClassCP> const& newClassList) { m_classes = newClassList; }
        void AddSchema(ECN::ECSchemaCR ecSchema, bool recursively);
    };

    struct ClassValueGenerator
    {
    private:
        std::map<ECN::ECPropertyCP, std::unique_ptr<IPropertyValueGenerator>> m_generators;
    public:
        void Init(ECN::ECClassCR ecClass, PropertyValueGeneratorFactory* factory);
        IPropertyValueGenerator* GetGenerator(ECN::ECPropertyCP property);
    };

private:
    Param m_param;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> m_instances;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> m_relationshipInstances;
    bmap<ECN::ECClassCP, std::shared_ptr<ClassValueGenerator>> m_generators;

    //return value generator for a class
    std::shared_ptr<ClassValueGenerator> GetClassValueGenerator(ECN::ECClassCR ecClass);

    //random numbers
    void InitRandom(unsigned int seed);
    float GetNextRandom();
    size_t GetNextRandom(size_t start, size_t end);
    int GetNextRandom(Param::RangeValue& range);

    ECN::IECInstancePtr CreateInstance(ECN::ECClassCR ecClass);
    //regular class
    BentleyStatus GenerateInstances(ECN::ECClassCR ecClass);
    //relationship
    void GetConstraintClasses(std::set<ECN::ECClassCP>& constraintClasses, ECN::ECRelationshipConstraintCR constraint);
    void GetFlatListOfDerivedClasses(std::set<ECN::ECClassCP>& derivedClasses, ECN::ECClassCR baseClass);
    ECN::IECInstancePtr GetRandomInstance(std::vector<ECN::IECInstancePtr>& inputList, bool returnAndRemoveFromList);
    BentleyStatus GenerateRelationshipInstances(ECN::ECRelationshipClassCR ecRelationshipClass);

    //Date generations
    BentleyStatus SetPrimitiveValue(ECN::ECValueR value, ECN::PrimitiveType primitiveType, IPropertyValueGenerator* gen);
    BentleyStatus SetStructValue(ECN::IECInstanceR generatedECInstance, ECN::ECClassCR structType, ECN::ECPropertyCP ecProperty);
    BentleyStatus SetInstanceData(ECN::IECInstanceR generatedECInstance);
    ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR target, ECN::IECInstanceCR structValue, Utf8CP propertyName);
    ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, Utf8CP baseAccessPath);

public:
    explicit RandomECInstanceGenerator(std::vector<ECN::ECClassCP> const& classList)
    {
        m_param.SetClassList(classList);
        InitRandom(1);
    }

    Param& GetParameters();
    BentleyStatus Generate(bool includeRelationships);
    BentleyStatus GenerateRelationships();
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const& GetGeneratedInstances() const { return m_instances; }
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const& GetGeneratedRelationshipInstances() const { return m_relationshipInstances; }
};