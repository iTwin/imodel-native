/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/Exp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
                ECN::ECPropertyCP m_property;
                int m_arrayIndex;

                void SetProperty(ECN::ECPropertyCR property);
                void ClearResolvedProperty() { m_property = nullptr; }

            public:
                Location(Utf8StringCR name, int arrayIndex) : m_propertyName(name), m_property(nullptr), m_arrayIndex(arrayIndex) { BeAssert(!name.empty()); }

                Utf8CP GetPropertyName() const { return m_propertyName.c_str(); }
                ECN::ECPropertyCP GetProperty() const { return m_property; }
                bool HasArrayIndex() const { return GetArrayIndex() != NOT_ARRAY; }
                int GetArrayIndex() const { return m_arrayIndex; }

                bool IsResolved() const { return m_property != nullptr; }
                Utf8String ToString(bool includeArrayIndexes) const;
            };

    private:
        std::vector<Location> m_path;
        ClassMap const* m_classMap;

        void Reset();
        void Clear();

    public:
        PropertyPath() : m_classMap(nullptr) {}
        ~PropertyPath() {}

        PropertyPath(PropertyPath const& rhs) : m_path(rhs.m_path), m_classMap(rhs.m_classMap) {}
        PropertyPath& operator=(PropertyPath const& rhs);
        PropertyPath(PropertyPath&& rhs) : m_path(std::move(rhs.m_path)), m_classMap(std::move(rhs.m_classMap)) {}
        PropertyPath& operator=(PropertyPath&& rhs);

        Location const& operator[] (size_t index) const { return m_path[index]; }
        size_t Size() const { return m_path.size(); }
        bool IsEmpty() const { return m_path.empty(); }
        ClassMap const* GetClassMap() const { return m_classMap; }

        void Push(Utf8StringCR propertyName);
        void Push(Utf8StringCR propertyName, size_t arrayIndex);
        void Pop();
        void Remove(size_t index) { m_path.erase(m_path.begin() + index); }

        BentleyStatus Resolve(ClassMap const& classMap, Utf8String* errorMessage = nullptr);
        bool IsResolved() const;

        BentleyStatus TryGetQualifiedPath(Utf8StringR qualifiedPath) const;
        static BentleyStatus TryParseQualifiedPath(PropertyPath& resolvedPropertyPath, Utf8StringCR qualifiedPath, ECDbCR ecdb);

        Utf8String ToString(bool escape = false, bool includeArrayIndexes = true) const;
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
            BooleanFactor,
            Cast,
            ClassName,
            CrossJoin,
            DateTimeConstantValue,
            Delete,
            DerivedProperty,
            FromClause,
            FunctionCall,
            ECRelationshipJoin,
            GroupBy,
            Having,
            Insert,
            JoinCondition,
            LikeRhsValue,
            LimitOffset,
            LiteralValue,
            NamedPropertiesJoin,
            NaturalJoin,
            NonJoinQuery,
            Option,
            Options,
            OrderBy,
            OrderBySpec,
            Parameter,
            Predicate,
            PropertyName,
            PropertyNameList,
            QualifiedJoin,
            RowValueConstructorList,
            Select,
            Selection,
            Subquery,
            SubqueryRef,
            SubqueryTest,
            SubqueryValue,
            UnaryPredicate,
            UnaryValue,
            Update,
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
                        explicit const_iterator(std::vector<std::unique_ptr<Exp>>::const_iterator const& iterator)
                            : m_iterator(iterator)
                            {}
                        ~const_iterator() {}
                        //copyable
                        const_iterator(const_iterator const& rhs)
                            : m_iterator(rhs.m_iterator)
                            {}
                        const_iterator& operator= (const_iterator const& rhs)
                            {
                            if (this != &rhs)
                                m_iterator = rhs.m_iterator;

                            return *this;
                            }
                        //moveable
                        const_iterator(const_iterator&& rhs)
                            : m_iterator(std::move(rhs.m_iterator))
                            {}

                        const_iterator& operator= (const_iterator&& rhs)
                            {
                            if (this != &rhs)
                                m_iterator = std::move(rhs.m_iterator);

                            return *this;
                            }

                        TExp operator* () const
                            {
                            return m_iterator->get();
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
                Collection() {}
                ~Collection() {}

                size_t size() const;
                bool empty() const;
                Exp const* operator[] (size_t i) const;
                Exp* operator[] (size_t i);

                template <class TExp>
                TExp const* Get(size_t i) const
                    {
                    Exp const* child = this->operator[] (i);
                    BeAssert(child == nullptr || dynamic_cast<TExp const*> (child) != nullptr);
                    return static_cast<TExp const*> (child);
                    }

                bool Replace(Exp const& replacee, std::vector<std::unique_ptr<Exp>>& replaceWith);

                const_iterator<Exp*> begin();
                const_iterator<Exp const*> begin() const;
                const_iterator<Exp const*> end() const;
                const_iterator<Exp*> end();


            };

        enum class FinalizeParseStatus { Completed, NotCompleted, Error };

    protected:
        enum class FinalizeParseMode { BeforeFinalizingChildren, AfterFinalizingChildren };

    public:
        static Utf8CP const ASTERISK_TOKEN;

    protected:
        static const int UNSET_CHILDINDEX = -1;

    private:
        Type m_type;
        Exp* m_parent;
        mutable Exp::Collection m_children;
        bool m_isComplete;

        virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode);
        virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const;
        virtual Utf8String _ToECSql() const = 0;
        virtual Utf8String _ToString() const = 0;

    protected:
        explicit Exp(Type type) : m_type(type), m_parent(nullptr), m_isComplete(false) {}

        void SetIsComplete() { m_isComplete = true; }

        template <class TExp>
        TExp const* GetChild(size_t index) const { return GetChildren().Get<TExp>(index); }

        template <class TExp>
        TExp* GetChildP(size_t index) const
            {
            Exp* child = GetChildrenR()[index];
            BeAssert(child == nullptr || dynamic_cast<TExp*> (child) != nullptr);
            return static_cast<TExp*> (child);
            }

        size_t AddChild(std::unique_ptr<Exp> child);
        Collection& GetChildrenR() const;
        void FindRecusive(std::vector<Exp const*>& expList, Type ofType) const;
        void FindInDirectDecedentOnly(std::vector<Exp const*>& expList, Type ofType) const;

    public:
        virtual ~Exp() {}

        BentleyStatus FinalizeParsing(ECSqlParseContext&);
        bool TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const;

        bool IsComplete() const { return m_isComplete; }

        Type GetType() const { return m_type; }
        bool IsParameterExp() const { return GetType() == Type::Parameter; }
        Exp const* GetParent() const { return m_parent; }
        Collection const& GetChildren() const;
        size_t GetChildrenCount() const { return m_children.size(); }

        //! Converts this expression into an ECSQL snippet.
        //! The child expressions are considered in this conversion.
        //! @return ECSQL snippet representing this expression graph
        Utf8String ToECSql() const;

        //! Returns a string description of this expression without recursion into its child expressions.
        //! @return string description of this expression
        Utf8String ToString() const;

        static bool IsAsteriskToken(Utf8CP token) { return strcmp(token, ASTERISK_TOKEN) == 0; }
        Exp const* FindParent(Exp::Type type) const;
        std::vector<Exp const*>  Find(Type ofType, bool recusive) const;
        std::set<DbTable const*> GetReferencedTables() const;
    };

typedef Exp* ExpP;
typedef Exp const* ExpCP;
typedef Exp const& ExpCR;


struct RangeClassRefExp;
struct RangeClasssInfo
    {
    typedef std::vector<RangeClasssInfo> List;
    enum Scope
        {
        Nil,
        Local,
        Inherited
        };
    private:
        RangeClassRefExp const* m_exp;
        Scope m_scope;

    public:
        RangeClasssInfo() :m_exp(nullptr), m_scope(Scope::Nil) {}
        RangeClasssInfo(RangeClassRefExp const& exp, Scope scope) :m_exp(&exp), m_scope(scope) {}
        ~RangeClasssInfo() {}
        Scope GetScope() const { return m_scope; }
        bool IsLocal() const { return m_scope == Scope::Local; }
        bool IsInherited() const { return m_scope == Scope::Inherited; }
        RangeClassRefExp const& GetExp() const { return *m_exp; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE