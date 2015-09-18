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
    struct Location
        {
    friend struct PropertyPath;

    private:
        static const int NOT_ARRAY = -1;

        Utf8String m_propertyName;
        ECPropertyCP m_property;
        int m_arrayIndex;

        void SetProperty(ECPropertyCR property);
        void ClearResolvedProperty() { m_property = nullptr; }

    public: 
        Location(Utf8CP name, int arrayIndex) : m_propertyName(name), m_property(nullptr), m_arrayIndex(arrayIndex) { BeAssert(!Utf8String::IsNullOrEmpty(name)); }
            
        Utf8CP GetPropertyName() const { return m_propertyName.c_str(); }
        ECPropertyCP GetProperty() const { return m_property; }
        bool HasArrayIndex() const { return GetArrayIndex() != NOT_ARRAY; }
        int GetArrayIndex() const { return m_arrayIndex; }

        bool IsResolved() const { return m_property != nullptr;}
        Utf8String ToString(bool includeArrayIndexes) const;
        };

private:
    std::vector<Location> m_path;
    IClassMap const* m_classMap;

    void Reset();
    void Clear();

public:
    PropertyPath () : m_classMap (nullptr) {}
    ~PropertyPath () {}

    PropertyPath(PropertyPath const& rhs) : m_path(rhs.m_path), m_classMap(rhs.m_classMap) {}
    PropertyPath& operator=(PropertyPath const& rhs);
    PropertyPath(PropertyPath&& rhs) : m_path(std::move(rhs.m_path)), m_classMap(std::move(rhs.m_classMap)) {}
    PropertyPath& operator=(PropertyPath&& rhs);

    Location const& operator[] (size_t index) const { return m_path[index]; }
    size_t Size() const { return m_path.size(); }
    bool IsEmpty() const { return m_path.empty(); }
    IClassMap const* GetClassMap() const { return m_classMap; }

    void Push(Utf8CP propertyName);
    void Push(Utf8CP propertyName, size_t arrayIndex);
    void Pop();
    void Remove(size_t index) { m_path.erase(m_path.begin() + index); }

    BentleyStatus Resolve(IClassMap const& classMap, Utf8String* errorMessage = nullptr);
    bool IsResolved() const;

    BentleyStatus TryGetQualifiedPath(Utf8StringR qualifiedPath) const;
    static BentleyStatus TryParseQualifiedPath(PropertyPath& resolvedPropertyPath, Utf8StringCR qualifiedPath, ECDbCR ecdb);

    Utf8String ToString(bool includeArrayIndexes = true) const;
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

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode);
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const;
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
    bool TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const;

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