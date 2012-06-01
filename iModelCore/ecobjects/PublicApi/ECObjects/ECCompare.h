/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicApi/ECObjects/ECCompare.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* ECProperties can be referenced by access string or property index on the instance
* or the class. Looking up by property index is cheaper, but property indices differ
* between ECClass and ECInstance.
* Since comparing instances involves lots of looking up of properties and their custom
* attributes for a small set of ECClasses, this class is used to cache that information
* during the comparison to make lookup as inexpensive as possible.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClassCompareCache
    {
private:
    enum
        {
        PROPERTYFLAG_Strict             = 1 << 0,       // ignore property unless doing comparison of type COMPARE_Strict
        PROPERTYFLAG_Array              = 1 << 1,
        PROPERTYFLAG_Enum               = 1 << 2,       // is an integer enum/ID value like a LinestyleID or a StandardValues property - don't compare arithmetically
        };
public:
    struct PropertyInfo
        {
    private:
        EC::ECPropertyCP        m_property;
        UInt8                   m_flags;
    public:
        PropertyInfo (EC::ECPropertyCR prop);
        PropertyInfo () : m_property (NULL), m_flags (0) { }

        EC::ECPropertyCR        GetProperty() const { BeAssert (NULL != m_property); return *m_property; }
        bool                    IsArray() const     { return 0 != (PROPERTYFLAG_Array & m_flags); }
        bool                    IsStrict() const    { return 0 != (PROPERTYFLAG_Strict & m_flags); }
        bool                    IsNull() const      { return NULL == m_property; }
        bool                    IsEnum() const      { return 0 != (PROPERTYFLAG_Enum & m_flags); }
        };

    typedef bvector<PropertyInfo>                   PropertyInfoMap;    // indexed by property index as provided by ECEnabler
private:
    typedef bmap<EC::ECClassCP, PropertyInfoMap>    ClassMap;
    typedef bmap<UInt32, PropertyInfo>              PropertyInfosByIndex;

    ClassMap                    m_classMap;

    void                        PopulatePropertyMap (PropertyInfosByIndex& propertyMap, UInt32& highestPropertyIndex, EC::ECClassCR ecClass, WStringCR parentAccessString, EC::ECEnablerCR enabler);
public:
    ECClassCompareCache() { }
    PropertyInfoMap const&      GetProperties (EC::ECEnablerCR enabler);
    };

/*---------------------------------------------------------------------------------**//**
* To simplify passing the cache around.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClassCompareCacheHolder
    {
private:
    ECClassCompareCache*        m_cache;
    bool                        m_ownsCache;
public:
    ECClassCompareCacheHolder (ECClassCompareCache* cache) : m_cache (cache ? cache : new ECClassCompareCache()), m_ownsCache (NULL == cache) { }
    ~ECClassCompareCacheHolder() { if (m_ownsCache) delete m_cache; }

    ECClassCompareCache*        GetCache()  { return m_cache; }
    };

/*---------------------------------------------------------------------------------**//**
* Represents a difference in property value between baseline and comparand instances.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECPropertyDiff
    {
private:
    UInt32                      m_propertyIndex;
    EC::IECInstanceCR           m_baselineInstance;
    EC::IECInstanceCR           m_comparandInstance;
    double                      m_diffScore;
public:
    ECPropertyDiff (EC::IECInstanceCR baseline, EC::IECInstanceCR comparand, UInt32 propertyIndex, double score)
        : m_propertyIndex(propertyIndex), m_baselineInstance(baseline), m_comparandInstance(comparand), m_diffScore(score) { }

    // Get the value of the property in the baseline or comparand instance.
    // Return false if the value could not be retrieved.
    // If the property is a complex (struct) property, this method returns false.
    // For arrays, returns the ArrayInfo containing the element type and count.
    ECOBJECTS_EXPORT bool       GetBaselineValue (EC::ECValueR v) const;
    ECOBJECTS_EXPORT bool       GetComparandValue (EC::ECValueR v) const;

    UInt32                      GetPropertyIndex() const                    { return m_propertyIndex; }
    ECOBJECTS_EXPORT WCharCP    GetAccessString() const;
    EC::IECInstanceCR           GetBaselineInstance() const                 { return m_baselineInstance; }
    EC::IECInstanceCR           GetComparandInstance() const                { return m_comparandInstance; }
    // Return floating point value in [0,1] representing 'degree of difference' between the two instances.
    // This is primarily useful internally for doing fuzzy matching.
    double                      GetDiffScore() const                        { return m_diffScore; }
    };

/*---------------------------------------------------------------------------------**//**
* Iterator over the ECPropertyDiffs for a ECInstanceDiff.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceDiffIterator
    {
    friend struct ECInstanceDiff;
private:
    typedef bpair<UInt32, double>  PropDiff;        // propertyindex, diffscore

    EC::IECInstancePtr              m_baseline;
    EC::IECInstancePtr              m_comparand;
    bvector<PropDiff>::iterator     m_end;
    bvector<PropDiff>::iterator     m_current;

    ECInstanceDiffIterator (EC::IECInstancePtr baseline, EC::IECInstancePtr comparand, bvector<PropDiff>& propDiffs, bool isEnd)
        : m_baseline(baseline), m_comparand(comparand), m_end(propDiffs.end()),
          m_current(isEnd || baseline.IsNull() || comparand.IsNull() ? propDiffs.end() : propDiffs.begin()) { }

    bool IsEnd() const      { return m_end == m_current; }
public:
    ECPropertyDiff operator*() const
        {
        BeAssert (!IsEnd());
        return ECPropertyDiff (*m_baseline, *m_comparand, m_current->first, m_current->second);
        }

    bool operator== (ECInstanceDiffIterator const& other) { return m_current == other.m_current; }
    bool operator!= (ECInstanceDiffIterator const& other) { return !(*this == other); }

    ECInstanceDiffIterator& operator++()
        {
        BeAssert (!IsEnd());
        ++m_current;
        return *this;
        }
    };

/*---------------------------------------------------------------------------------**//**
* A collection of property value differences between baseline and comparand.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceDiff
    {
    friend struct ECInstanceComparer;
protected:
    typedef bpair<UInt32, double>  PropDiff;            // propertyindex, diffscore

    EC::IECInstancePtr              m_baselineInstance;
    EC::IECInstancePtr              m_comparandInstance;
    bvector<PropDiff>               m_diffs;
    double                          m_score;

    ECInstanceDiff (EC::IECInstanceR baseline, EC::IECInstanceR comparand, double score)
        : m_baselineInstance (&baseline), m_comparandInstance (&comparand), m_score (score) { }

    void                        SetBaseline (EC::IECInstanceR instance)            { m_baselineInstance = &instance; }
    void                        SetComparand (EC::IECInstanceR instance)           { m_comparandInstance = &instance; }
    void                        AddDiff (UInt32 propIdx, double score)             { m_diffs.push_back (PropDiff (propIdx, score)); }
    void                        SetScore (double score)                            { m_score = score; }
public:
    typedef ECInstanceDiffIterator   const_iterator;

    ECInstanceDiff() : m_score(0.0) { }

    virtual UInt32              Count()                 { return (UInt32)m_diffs.size(); }
    double                      GetDiffScore() const    { return m_score; }
    WCharCP                     GetClassName() const    { return m_baselineInstance->GetEnabler().GetClass().GetFullName(); }
    virtual const_iterator      begin()                 { return ECInstanceDiffIterator (m_baselineInstance, m_comparandInstance, m_diffs, false); }
    virtual const_iterator      end()                   { return ECInstanceDiffIterator (m_baselineInstance, m_comparandInstance, m_diffs, true); }
    };

/*---------------------------------------------------------------------------------**//**
* Compares two instances of the same class.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
#define ECCOMPARE_DEFAULT_TOLERANCE 1.0e-6
struct ECInstanceComparer
    {
    enum CompareType
        {
        COMPARE_Strict,         // compare all properties for equality
        COMPARE_UserLevel       // compare only properties not marked with custom attribute 'IgnoreForTesting' ###TODO: attribute name
        };
private:
    EC::IECInstanceR                m_lhs;
    EC::IECInstanceR                m_rhs;
    double                          m_tolerance;            // proportional, for comparing double/point properties
    double                          m_score;
    ECClassCompareCacheHolder       m_cacheHolder;
    CompareType                     m_compareType;

    bool                            CompareProperties (double& score, UInt32& numProperties, ECInstanceDiff* diffs, bool requireEquality);
    bool                            CompareArrays (double& score, UInt32 propIdx, UInt32 nElems, bool requireEquality, bool isStructArray);
    double                          ScoreValues (EC::ECValueCR lhs, EC::ECValueCR rhs) const;
    double                          ScorePrimitives (EC::ECValueCR lhs, EC::ECValueCR rhs, bool compareArithmetically) const;
    double                          ScoreDoubles (double a, double b) const;   
    double                          ScoreIntegers (Int64 a, Int64 b) const;
public:
    ECInstanceComparer (EC::IECInstanceR lhs, EC::IECInstanceR rhs, CompareType compType, ECClassCompareCache* cache = NULL) 
        : m_lhs(lhs), m_rhs(rhs), m_tolerance(ECCOMPARE_DEFAULT_TOLERANCE), m_score(0.0), m_cacheHolder(cache), m_compareType(compType) { }
    ECInstanceComparer (EC::IECInstanceR lhs, EC::IECInstanceR rhs, CompareType compType, double tolerance, ECClassCompareCache* cache = NULL) 
        : m_lhs(lhs), m_rhs(rhs), m_tolerance(tolerance), m_score(0.0), m_cacheHolder(cache), m_compareType(compType) { }

    // Returns true if comparison determined instances are equivalent.
    ECOBJECTS_EXPORT bool   CheckEquality();

    // Calculate the degree of difference between the two instances, optionally populating diffs.
    ECOBJECTS_EXPORT double Compare (ECInstanceDiff* diffs);

    // Return the overall diff score for the instances; only meaningful if iteration completed.
    double                  GetScore() const { return m_score; }

    // Return a default tolerance for comparing floating-point values
    static double           GetDefaultTolerance() { return ECCOMPARE_DEFAULT_TOLERANCE; }
    };

END_BENTLEY_EC_NAMESPACE

