/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JoinExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlJoinType
    {
    None,
    LeftOuterJoin,
    RightOuterJoin,
    FullOuterJoin,
    InnerJoin,
    CrossJoin,
    NaturalJoin,
    JoinUsingRelationship
    };

//=======================================================================================
//! For a JOIN USING clause the values of this enum specify which end of the relationship
//! the joined class refers to.
//! The metaphor of a direction is used to express this. The forward direction means that 
//! the JOIN expression in the ECSQL statement goes from the ECRelationship's source constraint
//! to the target constraint, whereas the backward direction goes from target to source constraint.
//! 
//! @remarks In most cases the direction can be implied from the ECSQL 
//! statement directly. But there are case where this is ambiguous, e.g. for joins between 
//! the same class.
//! @see ECN::ECRelationshipClass::GetSource, ECN::ECRelationshipClass::GetTarget
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle      04/2013
//+===============+===============+===============+===============+===============+======
enum class JoinDirection
    {
    Implied = 0, //!< The direction can be implied from the ECSQL statement.
    Forward = 1, //!< JOIN expression goes from source to target constraint of the ECN::ECRelationshipClass.
    Backward = 2 //!< JOIN expression goes from target to source constraint of the ECN::ECRelationshipClass.
    };

struct FromExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct JoinExp : ClassRefExp
    {
    private:
        ECSqlJoinType  m_joinType;
        size_t m_nFromClassRefIndex;
        size_t m_nToClassRefIndex;

    protected:
        JoinExp(Type type, ECSqlJoinType joinType, std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to)
            : ClassRefExp(type), m_joinType(joinType)
            {
            m_nFromClassRefIndex = AddChild(move(from));
            m_nToClassRefIndex = AddChild(move(to));
            }

    public:
        virtual ~JoinExp() {}

        ECSqlJoinType GetJoinType() const { return m_joinType; }
        ClassRefExp const& GetFromClassRef() const { return *GetChild<ClassRefExp>(m_nFromClassRefIndex); }
        ClassRefExp const& GetToClassRef() const { return *GetChild<ClassRefExp>(m_nToClassRefIndex); }

        FromExp const* FindFromExpression() const;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct JoinSpecExp : Exp
    {
    protected:
        explicit JoinSpecExp(Type type) : Exp(type) {}

    public:
        virtual ~JoinSpecExp() {}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct CrossJoinExp final : JoinExp
    {
    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override { ctx.AppendToECSql(GetFromClassRef()).AppendToECSql(" CROSS JOIN ").AppendToECSql(GetToClassRef()); }
        Utf8String _ToString() const override { return "CrossJoin"; }
    public:
        CrossJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to)
            :JoinExp(Type::CrossJoin, ECSqlJoinType::CrossJoin, std::move(from), std::move(to))
            {}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct NaturalJoinExp final : JoinExp
    {
    private:
        ECSqlJoinType m_appliedJoinType;

        void _ToECSql(ECSqlRenderContext& ctx) const override;
        Utf8String _ToString() const override;

    public:
        NaturalJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType appliedJoinType)
            :JoinExp(Type::NaturalJoin, ECSqlJoinType::NaturalJoin, std::move(from), std::move(to)), m_appliedJoinType(appliedJoinType)
            {}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct QualifiedJoinExp final : JoinExp
    {
    private:
        size_t m_nJoinSpecIndex;
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        Utf8String _ToString() const override { return "QualifiedJoin"; }

    public:
        QualifiedJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType joinType, std::unique_ptr<JoinSpecExp> joinSpecExp);

        JoinSpecExp const* GetJoinSpec() const { return GetChild<JoinSpecExp>(m_nJoinSpecIndex); }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct ECRelationshipJoinExp final : JoinExp
    {
    public:
        enum class ClassLocation
            {
            NotResolved = 0,
            ExistInSource = 1,
            ExistInTarget = 2,
            ExistInBoth = ExistInSource | ExistInTarget
            };

        struct ResolvedEndPoint
            {
            friend ECRelationshipJoinExp;
            private:
                ClassNameExp const*    m_classRef;
                ClassLocation           m_location;
            protected:
                ResolvedEndPoint() :m_classRef(nullptr), m_location(ClassLocation::NotResolved){}
                void SetClassRef(ClassNameExp const* classRef)
                    {
                    m_classRef = classRef;
                    }
                void SetLocation(ClassLocation location, bool append)
                    {
                    if (append)
                        m_location = static_cast<ClassLocation>((int) m_location | (int) location);
                    else
                        m_location = location;
                    }
            public:
                ClassNameExp const* GetClassNameRef() const { return m_classRef; }
                ClassLocation       GetLocation() const { return m_location; }
            };
    private:
        JoinDirection           m_direction;
        size_t                  m_relationshipClassNameExpIndex;
        ResolvedEndPoint        m_resolvedFrom;
        ResolvedEndPoint        m_resolvedTo;

        void _ToECSql(ECSqlRenderContext& ctx) const override;
        Utf8String _ToString() const override;
        BentleyStatus ResolveRelationshipEnds(ECSqlParseContext&);
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;

    public:
        ECRelationshipJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, std::unique_ptr<ClassRefExp> relationship, JoinDirection direction)
            : JoinExp(Type::ECRelationshipJoin, ECSqlJoinType::JoinUsingRelationship, std::move(from), std::move(to)), m_direction(direction)
            {
            m_relationshipClassNameExpIndex = AddChild(move(relationship));
            }

        ClassNameExp const& GetRelationshipClassNameExp() const { return *GetChild<ClassNameExp>(m_relationshipClassNameExpIndex); }

        ResolvedEndPoint const&  GetResolvedFromEndPoint() const { return m_resolvedFrom; }
        ResolvedEndPoint const&  GetResolvedToEndPoint() const { return m_resolvedTo; }

        JoinDirection GetDirection() const { return m_direction; }
    };


//************* JoinSpecExp subclasses **********************
//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct NamedPropertiesJoinExp final : JoinSpecExp
    {
    private:
        std::vector<Utf8String> m_properties;

        void _ToECSql(ECSqlRenderContext& ctx) const override
            {
            ctx.AppendToECSql("USING (");
            bool isFirstProp = true;
            for (Utf8StringCR property : m_properties)
                {
                if (!isFirstProp)
                    ctx.AppendToECSql(", ");

                ctx.AppendToECSql(property);
                isFirstProp = false;
                }

            ctx.AppendToECSql(")");
            }

        Utf8String _ToString() const override
            {
            Utf8String str("NamedPropertiesJoin [Properties: ");
            bool isFirstItem = true;
            for (auto const& propertyName : m_properties)
                {
                if (!isFirstItem)
                    str.append(", ");

                str.append(propertyName.c_str());
                isFirstItem = false;
                }

            str.append("]");
            return str;
            }

    public:
        NamedPropertiesJoinExp() : JoinSpecExp(Type::NamedPropertiesJoin) {}
        void Append(Utf8StringCR propertyName) { m_properties.push_back(propertyName); }
    };

struct BooleanExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct JoinConditionExp final : JoinSpecExp
    {
    private:
        void _ToECSql(ECSqlRenderContext& ctx) const override;
        Utf8String _ToString() const override { return "JoinCondition"; }

    public:
        explicit JoinConditionExp(std::unique_ptr<BooleanExp> searchCondition);
        BooleanExp const* GetSearchCondition() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

