/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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


BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SqlSetQuantifier
    {
    NotSpecified,
    Distinct,
    All,
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class BinarySqlOperator
    {
    Plus,
    Minus,
    Divide,
    Multiply,
    Modulo,
    ShiftLeft,
    ShiftRight,
    BitwiseOr,
    BitwiseAnd,
    BitwiseXOr,
    Concat
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class BooleanSqlOperator
    {
    EqualTo,
    NotEqualTo,
    LessThan,
    LessThanOrEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    Is,
    IsNot,
    In,
    NotIn,
    Between,
    NotBetween,
    Like,
    NotLike,
    Or,
    And,
    Match,
    NotMatch
    };


//WIP_ECSQL: PropertyPath below should be replaced by ECSqlPropertyPath, PropertyNamePath, and ECSqlPropertyPathBuilder
//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct PropertyPath final
    {
    public:
        struct Location final
            {
            friend struct PropertyPath;

            private:
                static const int NOT_ARRAY = -1;

                Utf8String m_name;
                ECN::ECPropertyCP m_property = nullptr;
                int m_arrayIndex = NOT_ARRAY;

                void SetProperty(ECN::ECPropertyCR property);
                void ClearResolvedProperty() { m_property = nullptr; }

            public:
                Location(Utf8StringCR name, int arrayIndex) : m_name(name), m_arrayIndex(arrayIndex) { BeAssert(!name.empty()); }

                Utf8StringCR GetName() const { return m_name; }
                ECN::ECPropertyCP GetProperty() const { return m_property; }
                bool HasArrayIndex() const { return m_arrayIndex >= 0; }
                int GetArrayIndex() const { return m_arrayIndex; }

                bool IsResolved() const { return m_property != nullptr; }
                Utf8String ToString(bool includeArrayIndexes) const;
            };

    private:
        std::vector<Location> m_path;
        ClassMap const* m_classMap = nullptr;

        void Reset();
        void Clear();

    public:
        PropertyPath() {}
        ~PropertyPath() {}

        PropertyPath(PropertyPath const& rhs) : m_path(rhs.m_path), m_classMap(rhs.m_classMap) {}
        PropertyPath& operator=(PropertyPath const& rhs);
        PropertyPath(PropertyPath&& rhs) : m_path(std::move(rhs.m_path)), m_classMap(std::move(rhs.m_classMap)) {}
        PropertyPath& operator=(PropertyPath&& rhs);

        Location const& operator[] (size_t index) const { return m_path[index]; }
        size_t Size() const { return m_path.size(); }
        bool IsEmpty() const { return m_path.empty(); }
        ClassMap const* GetClassMap() const { return m_classMap; }

        void Push(Utf8StringCR propertyName, int arrayIndex = Location::NOT_ARRAY) { m_path.push_back(Location(propertyName, arrayIndex)); }

        void Pop();
        void Remove(size_t index) { m_path.erase(m_path.begin() + index); }
        BentleyStatus Resolve(ClassMap const& classMap, Utf8String* errorMessage = nullptr);
        bool IsResolved() const;

        Utf8String ToString(bool escape = false, bool includeArrayIndexes = true) const;
    };


//forward declarations
struct ECSqlParseContext;
struct ParameterExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct Exp
    {
    public:
        enum class Type
            {
            AllOrAny,
            Assignment,
            AssignmentList,
            BetweenRangeValue,
            BinaryBoolean,
            BinaryValue,
            BooleanFactor,
            Cast,
            ClassName,
            CrossJoin,
            DateTimeConstantValue,
            Delete,
            DerivedProperty,
            ECRelationshipJoin,
            EnumValue,
            FromClause,
            FunctionCall,
            GroupBy,
            Having,
            Insert,
            JoinCondition,
            LikeRhsValue,
            LimitOffset,
            LiteralValue,
            MemberFunctionCall,
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
            SingleSelect,
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
            };

        struct Collection final
            {
            friend struct Exp;

            public:
                template <typename TExp>
                struct const_iterator final : std::iterator<std::forward_iterator_tag, TExp>
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
                        const_iterator(const_iterator&& rhs) : m_iterator(std::move(rhs.m_iterator)) {}

                        const_iterator& operator= (const_iterator&& rhs)
                            {
                            if (this != &rhs)
                                m_iterator = std::move(rhs.m_iterator);

                            return *this;
                            }

                        TExp operator* () const { return m_iterator->get(); }

                        const_iterator& operator++ ()
                            {
                            m_iterator++;
                            return *this;
                            }
                        bool operator== (const_iterator const& rhs) const { return m_iterator == rhs.m_iterator; }
                        bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }
                    };


            private:
                std::vector<std::unique_ptr<Exp>> m_collection;

                //not copyable
                Collection(Collection const&) = delete;
                Collection& operator=(Collection const&) = delete;

            public:
                Collection() {}
                ~Collection() {}

                size_t size() const { return m_collection.size(); }
                bool empty() const { return m_collection.empty(); }
                Exp const* operator[] (size_t i) const { return m_collection[i].get(); }
                Exp* operator[] (size_t i) { return m_collection[i].get(); }

                template <typename TExp>
                TExp const* Get(size_t i) const
                    {
                    Exp const* child = this->operator[] (i);
                    BeAssert(child == nullptr || dynamic_cast<TExp const*> (child) != nullptr);
                    return static_cast<TExp const*> (child);
                    }


                template <typename TExp>
                bool Replace(Exp const& replacee, std::vector<std::unique_ptr<TExp>>& replaceWith)
                    {
                    std::vector<std::unique_ptr<Exp>> copiedCollection = std::move(m_collection);
                    BeAssert(m_collection.empty());

                    bool found = false;
                    for (std::unique_ptr<Exp>& exp : copiedCollection)
                        {
                        if (exp.get() != &replacee)
                            {
                            m_collection.push_back(std::move(exp));
                            continue;
                            }

                        // found a matching expr to be replaced
                        for (std::unique_ptr<TExp>& replacementExp : replaceWith)
                            {
                            m_collection.push_back(std::move(replacementExp));
                            }

                        found = true;
                        }

                    return found;
                    }

                const_iterator<Exp const*> begin() const { return const_iterator<Exp const*>(m_collection.begin()); }
                const_iterator<Exp*> begin() { return const_iterator<Exp*>(m_collection.begin()); }
                const_iterator<Exp const*> end() const { return const_iterator<Exp const*>(m_collection.end()); }
                const_iterator<Exp*> end() { return const_iterator<Exp*>(m_collection.end()); }
            };

        enum class FinalizeParseStatus { Completed, NotCompleted, Error };

        //=======================================================================================
        //! @bsiclass                                               Krischan.Eberle      03/2017
        //+===============+===============+===============+===============+===============+======
        struct ECSqlRenderContext final
            {
            public:
                enum class Mode
                    {
                    Default = 0,
                    GenerateNameForUnnamedParameter = 1
                    };

                struct ParameterNameInfo final
                    {
                    Utf8String m_name;
                    bool m_isSystemGeneratedName = false;

                    ParameterNameInfo() {}
                    ParameterNameInfo(Utf8StringCR name, bool isSystemGeneratedName) : m_name(name), m_isSystemGeneratedName(isSystemGeneratedName) {}
                    };

            private:
                Mode m_mode;
                bmap<int, ParameterNameInfo> m_parameterIndexNameMap;

                Utf8String m_ecsqlBuilder;

                //not copyable
                ECSqlRenderContext(ECSqlRenderContext const&) = delete;
                ECSqlRenderContext& operator=(ECSqlRenderContext const&) = delete;

            public:
                explicit ECSqlRenderContext(Mode mode = Mode::Default) : m_mode(mode) {}

                Mode GetMode() const { return m_mode; }
                bmap<int, ParameterNameInfo> const& GetParameterIndexNameMap() const { BeAssert(m_mode == Mode::GenerateNameForUnnamedParameter); return m_parameterIndexNameMap; }
                ECSqlRenderContext& AppendToECSql(Utf8StringCR str) { m_ecsqlBuilder.append(str); return *this; }
                ECSqlRenderContext& AppendToECSql(Utf8CP str) { m_ecsqlBuilder.append(str);  return *this; }
                ECSqlRenderContext& AppendToECSql(Exp const& exp) { exp.ToECSql(*this);  return *this; }

                void AddParameterIndexNameMapping(int index, Utf8StringCR name, bool isSystemName)
                    {
                    auto it = m_parameterIndexNameMap.find(index);
                    BeAssert(it == m_parameterIndexNameMap.end() || (it->second.m_name.Equals(name) && it->second.m_isSystemGeneratedName == isSystemName));

                    if (it == m_parameterIndexNameMap.end())
                        m_parameterIndexNameMap.insert(bpair<int, ParameterNameInfo>(index, ParameterNameInfo(name, isSystemName)));
                    }

                Utf8StringCR GetECSql() const { return m_ecsqlBuilder; }
                void ResetECSqlBuilder() { m_ecsqlBuilder.clear(); }
            };

    protected:
        enum class FinalizeParseMode { BeforeFinalizingChildren, AfterFinalizingChildren };

    public:
        static Utf8CP const ASTERISK_TOKEN;

    protected:
        static const int UNSET_CHILDINDEX = -1;

    private:
        Type m_type;
        Exp* m_parent = nullptr;
        mutable Exp::Collection m_children;
        bool m_isComplete = false;

        //not copyable
        Exp(Exp const&) = delete;
        Exp& operator=(Exp const&) = delete;

        virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) { return FinalizeParseStatus::Completed; }
        virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const { return false; }
        virtual void _ToECSql(ECSqlRenderContext&) const = 0;
        virtual Utf8String _ToString() const = 0;

        void Find(std::vector<Exp const*>& expList, Type candidateType, bool recursive) const;

    protected:
        explicit Exp(Type type) : m_type(type) {}

        void SetIsComplete() { m_isComplete = true; }

        template <typename TExp>
        TExp const* GetChild(size_t index) const { return m_children.Get<TExp>(index); }
        template <typename TExp>
        TExp* GetChildP(size_t index) const
            {
            Exp* child = m_children[index];
            BeAssert(child == nullptr || dynamic_cast<TExp*> (child) != nullptr);
            return static_cast<TExp*> (child);
            }

        size_t AddChild(std::unique_ptr<Exp> child);

    public:
        virtual ~Exp() {}

        template <typename TExp>
        TExp const* GetAsCP() const 
            { 
            BeAssert(dynamic_cast<TExp const*> (this) != nullptr);
            return static_cast<TExp const*> (this); 
            }

        template <typename TExp>
        TExp const& GetAs() const { return *GetAsCP<TExp>(); }

        BentleyStatus FinalizeParsing(ECSqlParseContext&);
        bool TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const;

        bool IsComplete() const { return m_isComplete; }

        Type GetType() const { return m_type; }
        bool IsParameterExp() const { return GetType() == Type::Parameter; }
        Exp const* GetParent() const { return m_parent; }
        Collection const& GetChildren() const { return m_children; }
        Collection& GetChildrenR() { return m_children; }
        size_t GetChildrenCount() const { return m_children.size(); }

        //! Converts this expression into an ECSQL snippet.
        //! The child expressions are considered in this conversion.
        //! @return ECSQL snippet representing this expression graph
        Utf8String ToECSql() const { ECSqlRenderContext ctx; ToECSql(ctx); return ctx.GetECSql(); }
        void ToECSql(ECSqlRenderContext& ctx) const { _ToECSql(ctx); }

        //! Returns a string description of this expression without recursion into its child expressions.
        //! @return string description of this expression
        Utf8String ToString() const { return _ToString(); }

        static bool IsAsteriskToken(Utf8StringCR token) { return token.Equals(ASTERISK_TOKEN); }

        Exp const* FindParent(Exp::Type) const;
        bool Contains(Type candidateType) const;
        std::vector<Exp const*> Find(Type candidateType, bool recursive) const;
    };

typedef Exp const* ExpCP;
typedef Exp const& ExpCR;


END_BENTLEY_SQLITE_EC_NAMESPACE