/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/Exp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "../ECDbInternalTypes.h"
#include <stack>
#include "Parser/SqlNode.h"
#include "Parser/SqlParse.h"
#include "../ECDbSystemSchemaHelper.h"
#include "../ClassMap.h"

#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//WIP_ECSQL: PropertyPath below should be replaced by ECSqlPropertyPath, PropertyNamePath, and ECSqlPropertyPathBuilder
typedef bvector<bpair<Utf8String, int>> PropertyNamePath;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct PropertyPath
    {
public:
    //=======================================================================================
    //! @bsiclass                                             Krischan.Eberle      07/2013
    //+===============+===============+===============+===============+===============+======
    enum class Status
        {
        Success = SUCCESS,
        PropertyPathIsEmpty,
        InvalidPropertyPath,
        FailedToResolvePropertyName,
        ArrayIndexNotSupported
        };

    struct Location
        {
    friend struct PropertyPath;
    private:
        Utf8String      m_propertyName;
        ECPropertyCP    m_property;
        int             m_index;
    protected:
        void SetProperty(ECPropertyCR property)  
            {
            WString propertyName (GetPropertyName().c_str(), BentleyCharEncoding::Utf8);
            BeAssert(propertyName == property.GetName());
            m_property = &property;
            }
        void ClearResolvedProperty() { m_property = nullptr; }

    public: 
        const static int NOT_ARRAY =-1;

        Location(Utf8StringCR name, int index) : m_property(nullptr), m_index(index), m_propertyName(name) { BeAssert(m_propertyName.empty()); }
            
        bool HasArrayIndex() const { return GetArrayIndex() != NOT_ARRAY;}
        ECPropertyCP GetProperty() const { return m_property;}
        Utf8StringCR GetPropertyName() const {return m_propertyName;}

        bool IsResolved() const { return m_property != nullptr;}
        int GetArrayIndex() const { return m_index;} 
        Utf8String ToString(bool includeArrayIndexes) const;
        };

private:
    std::vector<std::unique_ptr<Location>> m_path;
    IClassMap const* m_classMap;

    //non-copyable
    PropertyPath (PropertyPath const& rhs);
    PropertyPath& operator= (PropertyPath const& rhs);

    Status ResetResolutionInfo(Status status)
        {
        m_classMap = nullptr;
        for (auto& loc : m_path)
            loc->ClearResolvedProperty();

        return status;
        }

    Status ResolvePath(IClassMap const& classMap)
        {
        if (Empty())
            return Status::PropertyPathIsEmpty;

        auto cursorClass = &classMap.GetClass();
        for (size_t i = 0; i < Size(); i++)
            {
            auto& element = At(i);
            auto& propertyName = element.GetPropertyName();
            if (element.GetArrayIndex() >= 0)
                return ResetResolutionInfo(Status::ArrayIndexNotSupported);

            ECPropertyCP property = cursorClass->GetPropertyP(propertyName.c_str(), true);

            if (property == nullptr && i == 0)
                {
                auto propertyMap = classMap.GetPropertyMap(WString(propertyName.c_str(), true).c_str());
                if (propertyMap)
                    property = &propertyMap->GetProperty();
                }

            if (property == nullptr)
                return ResetResolutionInfo(Status::FailedToResolvePropertyName);

            element.SetProperty(*property);

            if (property->GetIsPrimitive())
                {
                if (&element != &Top() || element.HasArrayIndex())
                    return ResetResolutionInfo(Status::InvalidPropertyPath);
                }
            else if (property->GetIsStruct())
                {
                if (element.HasArrayIndex())
                    return ResetResolutionInfo(Status::InvalidPropertyPath);
                cursorClass = &property->GetAsStructProperty()->GetType();
                }
            else if (property->GetIsArray())
                {
                auto arrayProperty = property->GetAsArrayProperty();
                if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
                    {
                    if (&element != &Top())
                        return ResetResolutionInfo(Status::InvalidPropertyPath);
                    }
                if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                    {
                    cursorClass = arrayProperty->GetStructElementType();
                    }
                }
            if (!element.IsResolved())
                return ResetResolutionInfo(Status::FailedToResolvePropertyName);
            }
        
        BeAssert(IsResolved() && "Must be resolved by now");
        m_classMap = &classMap;
        return Status::Success;
        }

public:
    PropertyPath () : m_classMap (nullptr) {}
    ~PropertyPath () {}

    PropertyPath (PropertyPath&& rhs) : m_path (std::move (rhs.m_path)), m_classMap (std::move (rhs.m_classMap)) {}

    PropertyPath& operator= (PropertyPath&& rhs)
        {
        if (this != &rhs)
            {
            m_path = std::move (rhs.m_path);
            m_classMap = std::move (rhs.m_classMap);
            }

        return *this;
        }

    IClassMap const* GetClassMap() const { return m_classMap; }

    void Push(Utf8StringCR propertyName, int arrayIndex = Location::NOT_ARRAY) { m_path.push_back(std::unique_ptr<Location>(new Location(propertyName, arrayIndex))); }
    void Pop()
        {
        m_path.pop_back();
        if (Empty())
            Clear();
        }
    void Remove(size_t index) { m_path.erase(m_path.begin() + index); }
    Location& Top() const { return *m_path.at(m_path.size() - 1); }
    Location& Bottom() const { return *m_path.at(0); }
    Location& At(size_t index) const { return *m_path[index]; }
    size_t Size() const { return m_path.size(); }
    bool Empty() const  { return m_path.empty(); }
    bool IsResolved() const
        {
        for(auto& entry : m_path)
            if (!entry->IsResolved())
                return false;
        return true;
        }
    Status Resolve(IClassMap const& classMap) { return ResolvePath(classMap); }

    void Clear() 
        {
        m_path.clear();
        m_classMap = nullptr;
        }

    Utf8String ToString(bool includeArrayIndexes = true) const
        {
        Utf8String tmp;
        for(size_t i =0; i < Size(); i++)
            {
            tmp.append (At(i).ToString(includeArrayIndexes));
            if (i != (Size() - 1))
                tmp.append (".");            
            }
        return tmp;
        }
    
    ECSqlStatus TryGetQualifiedPath(Utf8StringR qualifiedPath) const
        {
        if (m_classMap == nullptr)
            {
            BeAssert(false && "Invalid property path");
            return ECSqlStatus::ProgrammerError;
            }
        qualifiedPath = Utf8String(GetClassMap()->GetClass().GetFullName());
        qualifiedPath.append(":");
        qualifiedPath.append(ToString(false));
        return ECSqlStatus::Success;
        }

    static ECSqlStatus TryParseQualifiedPath(PropertyPath& resolvedPropertyPath, Utf8StringCR qualifedPath, ECDbCR ecdb)
        {
        resolvedPropertyPath.Clear();
        bvector<Utf8String> subParts;
        BeStringUtilities::Split(qualifedPath.c_str(), ":", NULL, subParts);
        if (subParts.size() != 3)
            {
            BeAssert(false && "Invalid qualified path");
            return ECSqlStatus::ProgrammerError;
            }

        auto& schemaName = subParts.at(0);
        auto& className = subParts.at(1);
        auto& propertyPath = subParts.at(2);
        
        bvector<Utf8String> propertyNames;
        BeStringUtilities::Split(propertyPath.c_str(), ".", NULL, propertyNames);
        for(auto& propertyName : propertyNames)
            resolvedPropertyPath.Push(propertyName);

        auto targetClass = ecdb.Schemas().GetECClass(schemaName.c_str (), className.c_str (), ResolveSchema::AutoDetect);
        if (!targetClass)
            {
            BeAssert(false && "Failed to find ECClass");
            return  ECSqlStatus::ProgrammerError;
            }

        auto targetClassMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*targetClass);
        if (targetClassMap == nullptr)
            {
            BeAssert (false && "No class map found for class.");
            return ECSqlStatus::ProgrammerError;
            }

        if (resolvedPropertyPath.Resolve(*targetClassMap) != Status::Success)
            return ECSqlStatus::ProgrammerError;

        return ECSqlStatus::Success;
        }
    };


//forward declarations
struct ECSqlParseContext;
struct ParameterExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct Exp : NonCopyableClass
    {
public:
    enum class Type
        {
        AllOrAny,
        Assignment,
        AssignmentList,
        BetweenRangeValue,
        BinaryValue,
        BinaryBoolean,
        Boolean,
        BooleanFactor,
        Cast,
        ClassName,
        ClassRef,
        Computed,
        ConstantValue,
        CrossJoin,
        DateTimeConstantValue,
        Delete,
        DerivedProperty,
        FoldFunctionCall,
        FromClause,
        FunctionCall,
        ECClassIdFunction,
        GroupBy,
        Having,
        Insert,
        Join,
        JoinCondition,
        JoinSpec,
        LikeRhsValue,
        LimitOffset,
        NamedPropertiesJoin,
        NaturalJoin,
        NonJoinQuery,
        OrderBy,
        OrderBySpec,
        Parameter,
        Predicate,
        PropertyName,
        PropertyNameList,
        QualifiedJoin,
        Query,
        RangeClassRef,
        RelationshipJoin,
        RowValueConstructorList,
        Select,
        Selection,
        SetFunctionCall,
        Subquery,
        SubqueryRef,
        SubqueryTest,
        SubqueryValue,
        UnaryPredicate,
        UnaryValue,
        Update,
        Value,
        ValueBinary,
        ValueExpList,
        Where,        
        SingleSelect,
        };

    struct Collection : NonCopyableClass
        {
        friend struct Exp;

    public:
        template <class TExp>
        struct const_iterator : std::iterator<std::forward_iterator_tag, TExp>
            {
        private:
            std::vector<std::unique_ptr<Exp>>::const_iterator m_iterator;

        public:
            explicit const_iterator (std::vector<std::unique_ptr<Exp>>::const_iterator const& iterator)
                : m_iterator (iterator)
                {}
            ~const_iterator () {}
            //copyable
            const_iterator (const_iterator const& rhs)
                : m_iterator (rhs.m_iterator)
                {}
            const_iterator& operator= (const_iterator const& rhs)
                {
                if (this != &rhs)
                    m_iterator = rhs.m_iterator;

                return *this;
                }
            //moveable
            const_iterator (const_iterator&& rhs)
                : m_iterator (std::move (rhs.m_iterator))
                {}

            const_iterator& operator= (const_iterator&& rhs)
                {
                if (this != &rhs)
                    m_iterator = std::move (rhs.m_iterator);

                return *this;
                }

            TExp operator* () const
                {
                return m_iterator->get ();
                }

            const_iterator& operator++ ()
                {
                m_iterator++;
                return *this;
                }
            bool operator== (const_iterator const& rhs) const
                {
                return m_iterator == rhs.m_iterator;
                }

            bool operator!= (const_iterator const& rhs) const
                {
                return !(*this == rhs);
                }
            };


private:
    std::vector<std::unique_ptr<Exp>> m_collection;
public:
    Collection () {}
    ~Collection () {}

    size_t size () const;
    bool empty () const;
    Exp const* operator[] (size_t i) const;
    Exp* operator[] (size_t i);
    
    template <class TExp>
    TExp const* Get (size_t i) const
        {
        auto child = this->operator[] (i);
        BeAssert (child == nullptr || dynamic_cast<TExp const*> (child) != nullptr);
        return static_cast<TExp const*> (child);
        }

    bool Replace (Exp const& replacee, std::vector<std::unique_ptr<Exp>>& replaceWith);

    const_iterator<Exp*> begin ();
    const_iterator<Exp const*> begin () const;
    const_iterator<Exp const*> end () const;
    const_iterator<Exp*> end ();


    };


    //=======================================================================================
    //! @bsiclass                                                Krischan.Eberle      12/2013
    //+===============+===============+===============+===============+===============+======
    struct SystemPropertyExpIndexMap : NonCopyableClass
        {
    private:
        bmap<ECSqlSystemProperty, size_t> m_indexMap;

    public:
        SystemPropertyExpIndexMap() {}

        void AddIndex(ECSqlSystemProperty systemPropertyExp, size_t index);
        bool IsUnset(ECSqlSystemProperty systemPropertyExp) const;
        int GetIndex(ECSqlSystemProperty systemPropertyExp) const;
        };

    enum class FinalizeParseStatus {Completed, NotCompleted, Error};

protected:
    enum class FinalizeParseMode {BeforeFinalizingChildren, AfterFinalizingChildren};

public:
    static Utf8CP const ASTERISK_TOKEN;

protected:
    static const int UNSET_CHILDINDEX = -1;

private:
    Exp* m_parent;
    mutable Exp::Collection m_children;
    bool m_isComplete;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) { return FinalizeParseStatus::Completed; }
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const { return false; }
    virtual Utf8String _ToECSql() const = 0;
    virtual Utf8String _ToString () const = 0;

protected:
    Exp () : m_parent (nullptr), m_isComplete (false) {}

    void SetIsComplete () { m_isComplete = true; }

    template <class TExp>
    TExp const* GetChild (size_t index) const { return GetChildren ().Get<TExp> (index); }

    template <class TExp>
    TExp* GetChildP (size_t index) const
        {
        auto child = GetChildrenR ()[index];
        BeAssert (child == nullptr || dynamic_cast<TExp*> (child) != nullptr);
        return static_cast<TExp*> (child);
        }

    size_t AddChild (std::unique_ptr<Exp> child);
    Collection& GetChildrenR () const;

public:
    virtual ~Exp () {}

    ECSqlStatus FinalizeParsing (ECSqlParseContext&);
    bool TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const;

    bool IsComplete () const {return m_isComplete;}

    virtual Type GetType() const = 0;
    bool IsParameterExp() const { return GetType() == Type::Parameter; }
    Exp const* GetParent() const { return m_parent; }
    Collection const& GetChildren () const;
    size_t GetChildrenCount() const { return m_children.size();}

    //! Converts this expression into an ECSQL snippet.
    //! The child expressions are considered in this conversion.
    //! @return ECSQL snippet representing this expression graph
    Utf8String ToECSql() const;
    
    //! Returns a string description of this expression without recursion into its child expressions.
    //! @return string description of this expression
    Utf8String ToString () const;

    static bool IsAsteriskToken (Utf8CP token) { return strcmp (token, ASTERISK_TOKEN) == 0; }
    Exp const* FindParent (Exp::Type type) const
        {
        Exp const* p = this;
        do
            {
            p = p->GetParent ();        
            } while (p != nullptr && p->GetType () != type);

        return p;
        }
    };

typedef Exp* ExpP;
typedef Exp const* ExpCP;
typedef Exp const& ExpCR;


#define DEFINE_EXPR_TYPE(X) public: virtual Type GetType () const override { return Type::X;} 

END_BENTLEY_SQLITE_EC_NAMESPACE