#include <Bentley/BeTimeUtilities.h>
#include "RandomECInstanceGenerator.h"


USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void IPropertyValueGenerator::Reset()
{
    _Reset();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus IPropertyValueGenerator::NextPrimitiveValue(ECValueR value, PrimitiveType valueType)
    {
    BentleyStatus r = BentleyStatus::ERROR;
    switch(valueType)
        {
        case PrimitiveType::PRIMITIVETYPE_Binary:
            r = _NextBinary(value); break;
        case PrimitiveType::PRIMITIVETYPE_Boolean:
            r = _NextBoolean(value); break;
        case PrimitiveType::PRIMITIVETYPE_DateTime:
            r = _NextDateTime(value); break;
        case PrimitiveType::PRIMITIVETYPE_Double:
            r = _NextDouble(value); break;
        case PrimitiveType::PRIMITIVETYPE_IGeometry:
            r = _NextIGeometry(value); break;
        case PrimitiveType::PRIMITIVETYPE_Integer:
            r = _NextInteger(value); break;
        case PrimitiveType::PRIMITIVETYPE_Long:
            r = _NextLong(value); break;
        case PrimitiveType::PRIMITIVETYPE_Point2D:
            r = _NextPoint2d(value); break;
        case PrimitiveType::PRIMITIVETYPE_Point3D:
            r = _NextPoint3d(value); break;
        case PrimitiveType::PRIMITIVETYPE_String:
            r = _NextString(value); break;
        }
                
    if (r != BentleyStatus::SUCCESS)
        return r;

    PrimitiveType setType = value.GetPrimitiveType();
    BeAssert(setType ==  valueType);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextInteger  (ECValueR value)
    {
    m_currentInt32 += m_incrementInt32;
    return value.SetInteger(m_currentInt32);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextDouble   (ECValueR value)
    {
    m_currentDouble += m_incrementDouble ;
    return value.SetDouble(m_currentDouble);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextLong     (ECValueR value)
    {
    m_currentLong += m_incrementLong;
    return value.SetLong(m_currentInt32);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextString   (ECValueR value)
    {
    Utf8String strValue;
    m_currentInt32 += m_incrementInt32;
    strValue.Sprintf("%s-%05d",m_stringPrefix.c_str(),m_currentInt32);
    return value.SetUtf8CP(strValue.c_str(), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextPoint2d  (ECValueR value)
    {
    m_currentPoint2d.Add(m_incrementPoint2d) ;
    return value.SetPoint2D(m_currentPoint2d);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextPoint3d  (ECValueR value)
    {
    m_currentPoint3d.Add(m_incrementPoint3d) ;
    return value.SetPoint3D(m_currentPoint3d);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextBoolean  (ECValueR value)
    {
    m_currentBoolean = !m_currentBoolean;
    return value.SetBoolean(m_currentBoolean);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextDateTime (ECValueR value)
    {
    m_currentDateTimeJulian += m_incrementDateTimeJulian;
    DateTime dateTime;
    DateTime::FromJulianDay(dateTime, m_currentDateTimeJulian,m_seedDateTime.GetInfo());
    return value.SetDateTime(dateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextBinary   (ECValueR value)
    {
    Utf8String strValue, finalValue;
    m_currentInt32 += m_incrementInt32;
                    
    while(finalValue.size() < m_binaryLength)
        {
        strValue.Sprintf("B%04d",m_currentInt32);
        finalValue += strValue;
        }
    auto bytes = (Byte*)finalValue.c_str();
    return value.SetBinary(bytes, m_binaryLength, true);                 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus DefaultPropertyValueGenerator::_NextIGeometry(ECValueR value)
    {
    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::Init()
    {
    SetSeedBoolean(false);
    DateTime seed = DateTime(DateTime::Kind::Unspecified, 2000, 01, 01, 0, 0);
    if (GetProperty() && (
        (GetProperty()->GetIsPrimitive() && GetProperty()->GetAsPrimitiveProperty()->GetType() ==PRIMITIVETYPE_DateTime) || 
        (GetProperty()->GetIsArray() && GetProperty()->GetAsArrayProperty()->GetKind() == ARRAYKIND_Primitive && GetProperty()->GetAsArrayProperty()->GetPrimitiveElementType() == PRIMITIVETYPE_DateTime)))
        {
        DateTimeInfo info;
        if (StandardCustomAttributeHelper::GetDateTimeInfo(info, *GetProperty()) == BentleyStatus::SUCCESS)        
            seed = DateTime (info.GetInfo(true).GetKind(), seed.GetYear(), seed.GetMonth(), seed.GetDay(), seed.GetHour(),  seed.GetMinute());
        }

    SetSeedDateTime(seed, 432/*hectoseconds = 12hrs*/ );
    SetSeedDouble(10000,0.5);
    SetSeedInt32(10000,1);
    SetSeedLong(10000,1);
    SetSeedPoint2D(DPoint2d::From(1,1),DPoint2d::From(0.2,0.4));
    SetSeedPoint3D(DPoint3d::From(1,1,1),DPoint3d::From(0.2, 0.4, 0.8));
    SetStringPrefix("StRiNg");
    SetBinaryLength(18);              
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
DefaultPropertyValueGenerator::DefaultPropertyValueGenerator(ECN::ECPropertyCP templateProperty)
    :m_property(templateProperty)
    {
    Init();
    Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedInt32(int32_t seed, int32_t increment)
    {
    m_seedInt32 = seed;
    m_incrementInt32 = increment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedLong(int64_t seed, int64_t increment)
    {
    m_seedLong = seed;
    m_incrementLong = increment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedDouble(double seed, double increment)
    {
    m_seedDouble = seed;
    m_incrementDouble = increment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedPoint2D(DPoint2dCR seed, DPoint2d increment)
    {
    m_seedPoint2d = seed;
    m_incrementPoint2d = increment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedPoint3D(DPoint3dCR seed, DPoint3d increment)
    {
    m_seedPoint3d = seed;
    m_incrementPoint3d = increment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetBinaryLength(size_t length)
    {
    m_binaryLength = length;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetStringPrefix(Utf8CP prefix)
    {
    m_stringPrefix = prefix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedDateTime(DateTime seed, uint64_t julianHectoSecondsIncrement)
    {
    m_seedDateTime = seed;
    m_incrementDateTimeJulian = julianHectoSecondsIncrement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::SetSeedBoolean(bool seed)
    {
    m_seedBoolean = seed;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void DefaultPropertyValueGenerator::_Reset()
    {
    m_currentInt32 = m_seedInt32;
    m_currentLong = m_seedLong;
    m_currentDouble = m_seedDouble;
    m_currentPoint2d = m_seedPoint2d;
    m_currentPoint3d = m_seedPoint3d;
    m_currentBoolean = m_seedBoolean;
    m_seedDateTime.ToJulianDay(m_currentDateTimeJulian);
    }

RandomPropertyValueGenerator::RandomPropertyValueGenerator()
    {
    //srand (1);
    m_binaryLength = 18;
    } 

BentleyStatus RandomPropertyValueGenerator::_NextBinary(ECN::ECValueR value)
    {
    int randomNumber = rand ();
    Utf8String strValue, finalValue;

    while(finalValue.size() < m_binaryLength)
        {
        strValue.Sprintf("B%04d",randomNumber);
        finalValue += strValue;
        }
    auto bytes = (Byte*)finalValue.c_str();
    return value.SetBinary(bytes, m_binaryLength, true);                 
    }

BentleyStatus RandomPropertyValueGenerator::_NextBoolean(ECN::ECValueR value)
    {
    int randomNumber = rand ();
    return value.SetBoolean(randomNumber % 2 != 0); 
    }

BentleyStatus RandomPropertyValueGenerator::_NextDateTime(ECN::ECValueR value)
    {
    DateTime utcTime = DateTime::GetCurrentTimeUtc ();
    return value.SetDateTime(utcTime); 
    }

BentleyStatus RandomPropertyValueGenerator::_NextDouble(ECN::ECValueR value)
    {
    return value.SetDouble((double) rand () / RAND_MAX);
    }

BentleyStatus RandomPropertyValueGenerator::_NextIGeometry(ECN::ECValueR value)
    {
    IGeometryPtr line = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    return value.SetIGeometry (*line);
    }

BentleyStatus RandomPropertyValueGenerator::_NextInteger(ECN::ECValueR value)
    {
    return value.SetInteger(rand() % 1001); 
    }

BentleyStatus RandomPropertyValueGenerator::_NextLong(ECN::ECValueR value)
    {
    const int32_t intMax = std::numeric_limits<int32_t>::max ();
    const int64_t longValue = static_cast<int64_t> (intMax) + rand();
    return value.SetLong(longValue); 
    }

BentleyStatus RandomPropertyValueGenerator::_NextPoint2d(ECN::ECValueR value)
    {
    double randomNumber = (double) rand () / RAND_MAX;
    DPoint2d point2d;
    point2d.x=randomNumber * 1.0;
    point2d.y=randomNumber * 1.8;
    return value.SetPoint2D(point2d);
    }

BentleyStatus RandomPropertyValueGenerator::_NextPoint3d(ECN::ECValueR value)
    {
    double randomNumber = (double) rand () / RAND_MAX;
    DPoint3d point3d;
    point3d.x=randomNumber * 1.0;
    point3d.y=randomNumber * 1.8;
    point3d.z=randomNumber * 2.9;
    return value.SetPoint3D(point3d);
    }

BentleyStatus RandomPropertyValueGenerator::_NextString(ECN::ECValueR value)
    {
    int randomNumber = rand () % 1001;

    Utf8String text;
    text.Sprintf ("Sample-%d", randomNumber);
    return value.SetUtf8CP(text.c_str (), true); 
    }

void RandomPropertyValueGenerator::_Reset()
    {
    srand ((uint32_t)BeTimeUtilities::QueryMillisecondsCounter ());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void RandomECInstanceGenerator::Param::AddSchema(ECSchemaCR ecSchema, bool recursively)
    {
    if (recursively)
        {
        for (auto& refSchema : ecSchema.GetReferencedSchemas())
            {
            AddSchema (*refSchema.second, recursively);
            }
        }
    for (auto ecClass : ecSchema.GetClasses())
        {
        GetClassList().push_back(ecClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void RandomECInstanceGenerator::ClassValueGenerator::Init (ECClassCR ecClass, PropertyValueGeneratorFactory* factory)
    {
    BeAssert(factory != nullptr);
    m_generators.clear();
    auto enabler = ecClass.GetDefaultStandaloneEnabler();
    bvector<uint32_t> indices;
    enabler->GetPropertyIndices(indices, 0);
    for (auto property : ecClass.GetProperties(true))
        {
        if (property->IsCalculated())
            continue;

        if (property->GetIsPrimitive() || (property->GetIsArray() && property->GetAsArrayProperty()->GetKind()==ARRAYKIND_Primitive))
            m_generators[property] = factory->CreateInstance(property);        
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
IPropertyValueGenerator* RandomECInstanceGenerator::ClassValueGenerator::GetGenerator(ECPropertyCP property)
    {
    if (m_generators.find(property) == m_generators.end())
        return nullptr;

    return m_generators [property].get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
std::shared_ptr<RandomECInstanceGenerator::ClassValueGenerator> RandomECInstanceGenerator::GetClassValueGenerator(ECClassCR ecClass)
    {
    auto itor = m_generators.find(&ecClass);
    if (itor != m_generators.end())
        return itor->second;

    std::shared_ptr<ClassValueGenerator> newGen = std::make_shared<ClassValueGenerator>();
    newGen->Init(ecClass, GetParameters().GetValueGeneratorFactory());
    /*if (ecClass.GetIsStruct())*/
    m_generators[&ecClass] = newGen;

    return newGen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void RandomECInstanceGenerator::InitRandom(unsigned int seed)       
    {
    srand(seed);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
float RandomECInstanceGenerator::GetNextRandom()
    {
    return (float)rand()/RAND_MAX;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
size_t RandomECInstanceGenerator::GetNextRandom(size_t start, size_t end)
    {
        return (size_t)(start + (GetNextRandom()* (end - start)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
int RandomECInstanceGenerator::GetNextRandom(RandomECInstanceGenerator::Param::RangeValue& range)
    {       
    return (int)(range.GetMin() + (GetNextRandom()* range.Delta()));
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
IECInstancePtr RandomECInstanceGenerator::CreateInstance(ECClassCR ecClass)
    {
    auto ecInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
    if (ecInstance.IsValid())
        SetInstanceData(*ecInstance);

    return ecInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void RandomECInstanceGenerator::GetConstraintClasses(std::set<ECClassCP>& constraintClasses, ECRelationshipConstraintCR constraint)
    {
    for(auto constraintClass: constraint.GetClasses())
        {
        if (constraintClasses.find(constraintClass) != constraintClasses.end())
            continue;
        if (constraintClass->GetName().Equals("AnyClass"))
            {
            for(auto key : m_instances)
                {
                if (constraintClasses.find(constraintClass) != constraintClasses.end())
                    continue;
                constraintClasses.insert(key.first);                
                }
            return;
            }
        else
            {
            constraintClasses.insert (constraintClass);
            if (constraint.GetIsPolymorphic())
                GetFlatListOfDerivedClasses(constraintClasses, *constraintClass);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
void RandomECInstanceGenerator::GetFlatListOfDerivedClasses(std::set<ECClassCP>& derivedClasses, ECClassCR baseClass)
    {
    for(auto derivedClass : baseClass.GetDerivedClasses())
        {
        if (derivedClasses.find(derivedClass) != derivedClasses.end())
            continue;

        derivedClasses.insert(derivedClass);
            
        if (!derivedClasses.empty())
            GetFlatListOfDerivedClasses(derivedClasses, *derivedClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
IECInstancePtr RandomECInstanceGenerator::GetRandomInstance( std::vector<IECInstancePtr>& inputList, bool returnAndRemoveFromList)
    {
    auto nPos = GetNextRandom (0, inputList.size());
    auto selectedInstanceItor = (inputList.begin() + nPos);
    auto selectedInstance = *selectedInstanceItor;
    if (returnAndRemoveFromList)
        inputList.erase (selectedInstanceItor);

    return selectedInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::GenerateInstances(ECClassCR ecClass) 
    {
    BeAssert(ecClass.GetRelationshipClassCP() == nullptr);
    int noOfInstance = 0;
    if (ecClass.GetIsStruct())
        noOfInstance = GetNextRandom(GetParameters().GetNumberOfStructInstancesToGeneratePerClass());
    else
        noOfInstance = GetNextRandom(GetParameters().GetNumberOfRegularInstancesToGeneratePerClass());

//    noOfInstance = 100;
    for (int i = 0; i < noOfInstance ; i++) 
        {
        auto newECInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance();
        BeAssert(newECInstance.IsValid());
        auto r = SetInstanceData(*newECInstance);
        BeAssert(r == BentleyStatus::SUCCESS);
        m_instances[&ecClass].push_back(newECInstance);
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
bool HasAnyClass(std::set<ECClassCP>& classes)
    {
    for (auto& ecclass : classes)
        {
        if (ecclass->GetName() == "AnyClass")
            return true;
        }
    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::GenerateRelationshipInstances(ECRelationshipClassCR ecRelationshipClass)
{
    auto& sourceConstraint = ecRelationshipClass.GetSource();
    auto& targetConstraint = ecRelationshipClass.GetTarget();

    std::set<ECClassCP> sourceClasses, targetClasses;
    GetConstraintClasses(sourceClasses, sourceConstraint);
    GetConstraintClasses(targetClasses, targetConstraint);

    if (sourceClasses.empty() || targetClasses.empty())
        return BentleyStatus::SUCCESS; //not a valid relationship class

    std::vector<IECInstancePtr> sourceInstances;
    if (HasAnyClass(sourceClasses))
    {
        for (auto& inst : m_instances)
            sourceInstances.insert(sourceInstances.end(), inst.second.begin(), inst.second.end());
    }
    else
    for (auto sourceClass : sourceClasses)
    {
        auto& si = m_instances[sourceClass];
        sourceInstances.insert(sourceInstances.end(), si.begin(), si.end());
    }

    std::vector<IECInstancePtr> targetInstances;
    if (HasAnyClass(targetClasses))
    {
        for (auto& inst : m_instances)
            targetInstances.insert(targetInstances.end(), inst.second.begin(), inst.second.end());
    }
    else
    for (auto targetClass : targetClasses)
    {
        auto& ti = m_instances[targetClass];
        targetInstances.insert(targetInstances.end(), ti.begin(), ti.end());
    }

    if (sourceInstances.empty() || targetInstances.empty())
        return BentleyStatus::ERROR;

    auto sUL = sourceConstraint.GetCardinality().GetUpperLimit();
    auto tUL = targetConstraint.GetCardinality().GetUpperLimit();

    enum class Cardinality
    {
        S1_T1,
        Sm_Tm,
        S1_Tm,
        Sm_T1
    } cardinality;

    if (sUL > 1 && tUL > 1)
        cardinality = Cardinality::Sm_Tm;
    else if (sUL > 1 && tUL <= 1)
        cardinality = Cardinality::Sm_T1;
    else if (sUL <= 1 && tUL > 1)
        cardinality = Cardinality::S1_Tm;
    else if (sUL <= 1 && tUL <= 1)
        cardinality = Cardinality::S1_T1;
    else
    {
        BeAssert(false); return ERROR;
    }

    auto noOfInstancesToGenerate = GetNextRandom(GetParameters().GetNumberOfRelationshipInstancesToGeneratePerClass());
    IECInstancePtr sourceInstance, targetInstance;
    bool hasData = true;
    //noOfInstancesToGenerate = 100;
    for (int nPos = 0; nPos < noOfInstancesToGenerate && hasData; nPos++)
    {
        switch (cardinality)
        {
        case Cardinality::S1_T1:
            if (hasData = (!sourceInstances.empty() && !targetInstances.empty()))
            {
                sourceInstance = GetRandomInstance(sourceInstances, true);
                targetInstance = GetRandomInstance(targetInstances, true);
            }
            break;
        case Cardinality::S1_Tm:
            if (hasData = (!targetInstances.empty()))
            {
                sourceInstance = GetRandomInstance(sourceInstances, false);
                targetInstance = GetRandomInstance(targetInstances, true);
            }
            break;
        case Cardinality::Sm_T1:
            if (hasData = (!sourceInstances.empty()))
            {
                sourceInstance = GetRandomInstance(sourceInstances, true);
                targetInstance = GetRandomInstance(targetInstances, false);
            }
            break;
        case Cardinality::Sm_Tm:
            sourceInstance = GetRandomInstance(sourceInstances, false);
            targetInstance = GetRandomInstance(targetInstances, false);
            break;
        };

        if (!hasData)
            break;
        //Before we add this, we need to check that source and target are unique for the cardinality
        bool addInstance = true;
        Utf8String newSrcClassName = sourceInstance->GetClass().GetName();
        Utf8String newSrcId = sourceInstance->GetInstanceId();
        Utf8String newTarClassName = targetInstance->GetClass().GetName();
        Utf8String newTarId = targetInstance->GetInstanceId();
        for (auto& existingEntry : m_relationshipInstances)
        {
            //auto ecClass = existingEntry.first;
            auto const& instanceList = existingEntry.second;
            FOR_EACH(IECInstancePtr existingInstance, instanceList)
            {
                IECRelationshipInstancePtr relInstance = dynamic_cast<IECRelationshipInstance*>(existingInstance.get());
                if (cardinality == Cardinality::S1_T1 || cardinality == Cardinality::S1_Tm) // target should be unique
                {
                    Utf8String exstTarClassName = relInstance->GetTarget()->GetClass().GetName();
                    Utf8String existingTarInstId = relInstance->GetTarget()->GetInstanceId();
                    if (exstTarClassName.Equals(newTarClassName) && existingTarInstId.Equals(newTarId))
                    {
                        addInstance = false;
                        nPos--;
                    }
                }
                if (cardinality == Cardinality::S1_T1 || cardinality == Cardinality::Sm_T1) // source should be unique
                {
                    Utf8String exstSrcClassName = relInstance->GetSource()->GetClass().GetName();
                    Utf8String existingSrcInstId = relInstance->GetSource()->GetInstanceId();
                    if (exstSrcClassName.Equals(newSrcClassName) && existingSrcInstId.Equals(newSrcId))
                    {
                        addInstance = false;
                        nPos--;
                    }
                }
            }
        }
        if (addInstance)
        {
            auto enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(ecRelationshipClass);
            auto ecRelationshipInstance = enabler->CreateRelationshipInstance();
            if (ecRelationshipInstance.IsValid())
                SetInstanceData(*ecRelationshipInstance);

            ecRelationshipInstance->SetSource(sourceInstance.get());
            ecRelationshipInstance->SetTarget(targetInstance.get());
            m_relationshipInstances[&ecRelationshipClass].push_back(ecRelationshipInstance);
        }
    }

    return BentleyStatus::SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::SetPrimitiveValue (ECValueR value, PrimitiveType primitiveType, IPropertyValueGenerator* gen)
    {
    BeAssert(gen != nullptr);
    return gen->NextPrimitiveValue(value, primitiveType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus RandomECInstanceGenerator::CopyStruct(IECInstanceR target, IECInstanceCR structValue, Utf8CP propertyName)
    {
    return CopyStruct(target, *ECValuesCollection::Create(structValue), propertyName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus RandomECInstanceGenerator::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, Utf8CP baseAccessPath)
    { 
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;

    for(auto& propertyValue : collection)
        {
        Utf8String targetVAS;
        if (baseAccessPath)
            {
            targetVAS.append(baseAccessPath);
            targetVAS.append(".");
            }

        targetVAS.append(propertyValue.GetValueAccessor().GetManagedAccessString());
        ECValueAccessor accessor;
        status = ECValueAccessor::PopulateValueAccessor(accessor, source, targetVAS.c_str());
        if (status == ECOBJECTS_STATUS_ArrayIndexDoesNotExist)
            {
            status = source.AddArrayElements(accessor.GetAccessString(), 1);
            }
        else if (status != ECOBJECTS_STATUS_Success)
            {
            return status;
            }

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct (source, *propertyValue.GetChildValues(), baseAccessPath);
            if (status != ECOBJECTS_STATUS_Success && status != ECOBJECTS_STATUS_PropertyValueMatchesNoChange)
                {
                BeAssert(false && "CopyStruct(): Fail to copy child values");
                return status;
                }
            continue;
            }

        //set primitive value
        status = source.SetValueUsingAccessor(accessor, propertyValue.GetValue());
        if (status != ECOBJECTS_STATUS_Success && status != ECOBJECTS_STATUS_PropertyValueMatchesNoChange)
            {
            BeAssert(false && "CopyStruct(): Fail to set primitive value");
            return status;
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::SetStructValue (IECInstanceR generatedECInstance, ECClassCR structType, ECPropertyCP ecProperty)
    {

    auto structInstance = structType.GetDefaultStandaloneEnabler()->CreateInstance();
    BeAssert(structInstance.IsValid());

    auto r = SetInstanceData (*structInstance);
    BeAssert(r == BentleyStatus::SUCCESS);
    
    auto status = CopyStruct(generatedECInstance, *structInstance, ecProperty->GetName().c_str());
    if ( status != ECOBJECTS_STATUS_Success && status != ECOBJECTS_STATUS_PropertyValueMatchesNoChange)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::SetInstanceData(IECInstanceR generatedECInstance)
    {
    ECValue value;
    auto gen = GetClassValueGenerator(generatedECInstance.GetClass());
    for (auto ecProperty: generatedECInstance.GetClass().GetProperties(true))
        {
        bool leaveValueNull = (GetNextRandom() <  GetParameters().GetProbabilityOfNullValuesPerClass());
        if (leaveValueNull) 
            continue;
        
        if (ecProperty->IsCalculated()) continue;
        if (ecProperty->GetIsStruct())
            {
            auto r = SetStructValue (generatedECInstance, ecProperty->GetAsStructProperty()->GetType(), ecProperty);
            BeAssert(r == BentleyStatus::SUCCESS);
            }
        else if (ecProperty->GetIsPrimitive())
            {
            SetPrimitiveValue (value, ecProperty->GetAsPrimitiveProperty()->GetType(), gen->GetGenerator(ecProperty));
            generatedECInstance.SetValue (ecProperty->GetName().c_str(), value);
            }
        else if (ecProperty->GetIsArray())
            {
            auto arrayProperty  = ecProperty->GetAsArrayProperty();
            if(arrayProperty->GetKind() == ARRAYKIND_Primitive && arrayProperty->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            auto maxArrayEntries = (uint32_t)GetNextRandom(GetParameters().GetNumberOfArrayElementsToGenerate());

            if( (arrayProperty->GetMaxOccurs() - arrayProperty->GetMinOccurs()) < maxArrayEntries)
                continue;

            generatedECInstance.AddArrayElements (ecProperty->GetName().c_str (), maxArrayEntries);
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {

                for ( uint32_t i=0 ; i < maxArrayEntries; i++ )
                    {
                    auto structInstance = arrayProperty->GetStructElementType()->GetDefaultStandaloneEnabler()->CreateInstance();
                    BeAssert(structInstance.IsValid());

                    auto r = SetInstanceData (*structInstance);
                    BeAssert(r == BentleyStatus::SUCCESS);
                        value.SetStruct(structInstance.get());

                    auto status = generatedECInstance.SetValue (ecProperty->GetName().c_str (), value, i);
                    if (status != ECOBJECTS_STATUS_Success)
                        {
                        BeAssert(false);
                        return BentleyStatus::ERROR;
                        }
                    }
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Primitive )
                {
                for ( uint32_t i=0 ; i < maxArrayEntries; i++ )
                    {
                    SetPrimitiveValue (value, arrayProperty->GetPrimitiveElementType(), gen->GetGenerator(ecProperty));
                    auto status = generatedECInstance.SetValue (ecProperty->GetName().c_str (), value, i);
                    if (status != ECOBJECTS_STATUS_Success)
                        {
                        BeAssert(false);
                        return BentleyStatus::ERROR;
                        }
                    
                    }
                }
            }
        }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
RandomECInstanceGenerator::Param& RandomECInstanceGenerator::GetParameters()
    {
    return m_param;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::Generate(bool includeRelationships)
{
    std::vector<ECClassCP> structs;
    std::vector<ECClassCP> classes;
    std::vector<ECRelationshipClassCP> relationships;

    this->m_instances.clear();
    this->m_relationshipInstances.clear();
    this->m_generators.clear();

    for (auto ecClass : GetParameters().GetClassList())
    {
        if (ecClass->GetIsCustomAttributeClass())
            continue;

        if (GetParameters().GenerateInstancesForEmptyClasses())
        if (ecClass->GetDefaultStandaloneEnabler()->GetClassLayout().GetPropertyCount() == 0)
            continue;

        if (!ecClass->GetIsDomainClass())
        if (!GetParameters().GenerateInstancesForNoneDomainClasses())
            continue;

        if (ecClass->GetIsStruct())
            structs.push_back(ecClass);
        else if (auto relationshipClass = ecClass->GetRelationshipClassCP())
            relationships.push_back(relationshipClass);
        else
            classes.push_back(ecClass);
    }

    for (auto ecStruct : structs)
    {
        BeAssert(GenerateInstances(*ecStruct) == BentleyStatus::SUCCESS);
    }

    for (auto ecClass : classes)
    {
        BeAssert(GenerateInstances(*ecClass) == BentleyStatus::SUCCESS);
    }

    if (includeRelationships)
    {
        for (auto ecRelationshipClass : relationships)
        {
            BeAssert(GenerateRelationshipInstances(*ecRelationshipClass) == BentleyStatus::SUCCESS);
        }
    }
    return BentleyStatus::SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Majd Uddin     10/2014
//---------------------------------------------------------------------------------------
BentleyStatus RandomECInstanceGenerator::GenerateRelationships()
{
    std::vector<ECClassCP> structs;
    std::vector<ECClassCP> classes;
    std::vector<ECRelationshipClassCP> relationships;

    //this->m_instances.clear(); // We want other instances to check for cardinality
    this->m_relationshipInstances.clear();
    this->m_generators.clear();

    for (auto ecClass : GetParameters().GetClassList())
    {
        if (ecClass->GetIsCustomAttributeClass())
            continue;

        if (GetParameters().GenerateInstancesForEmptyClasses())
        if (ecClass->GetDefaultStandaloneEnabler()->GetClassLayout().GetPropertyCount() == 0)
            continue;

        if (!ecClass->GetIsDomainClass())
        if (!GetParameters().GenerateInstancesForNoneDomainClasses())
            continue;

        if (ecClass->GetIsStruct())
            structs.push_back(ecClass);
        else if (auto relationshipClass = ecClass->GetRelationshipClassCP())
            relationships.push_back(relationshipClass);
        else
            classes.push_back(ecClass);
    }

    for (auto ecRelationshipClass : relationships)
    {
        BeAssert(GenerateRelationshipInstances(*ecRelationshipClass) == BentleyStatus::SUCCESS);
    }

    return BentleyStatus::SUCCESS;
}
