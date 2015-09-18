/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JoinExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

struct FromExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct JoinExp : ClassRefExp
    {
    DEFINE_EXPR_TYPE(Join) 
private:
    ECSqlJoinType  m_joinType;
    size_t m_nFromClassRefIndex;
    size_t m_nToClassRefIndex;
public:
    JoinExp (ECSqlJoinType joinType, std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to)
        : ClassRefExp (), m_joinType(joinType)
        {
        m_nFromClassRefIndex = AddChild (move (from));
        m_nToClassRefIndex = AddChild (move (to));
        }
    virtual ~JoinExp () {}

    ECSqlJoinType GetJoinType() const {return m_joinType;}
    ClassRefExp const& GetFromClassRef() const { return *GetChild<ClassRefExp> (m_nFromClassRefIndex); }
    ClassRefExp const& GetToClassRef() const { return *GetChild<ClassRefExp> (m_nToClassRefIndex);}

    FromExp const* FindFromExpression() const;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct JoinSpecExp : Exp
    {
DEFINE_EXPR_TYPE(JoinSpec) 
public:
    JoinSpecExp () : Exp () {}
    virtual ~JoinSpecExp () {}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct CrossJoinExp: JoinExp
    {
DEFINE_EXPR_TYPE(CrossJoin) 
private:
    virtual Utf8String _ToECSql() const override { return GetFromClassRef().ToECSql() + " CROSS JOIN " + GetToClassRef().ToECSql(); }
    virtual Utf8String _ToString() const override { return "CrossJoin"; }
public:
    CrossJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to)
        :JoinExp(ECSqlJoinType::CrossJoin, std::move (from), std::move (to))
        {}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct NaturalJoinExp: JoinExp
    {
DEFINE_EXPR_TYPE(NaturalJoin) 
private:
    ECSqlJoinType m_appliedJoinType;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override;

public:
    NaturalJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType appliedJoinType)
        :JoinExp(ECSqlJoinType::NaturalJoin, std::move(from), std::move(to)), m_appliedJoinType(appliedJoinType)
        {}

    ECSqlJoinType GetAppliedJoinType() const {return m_appliedJoinType;}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct QualifiedJoinExp: JoinExp
    {    
    DEFINE_EXPR_TYPE(QualifiedJoin) 

private:
    size_t m_nJoinSpecIndex;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "QualifiedJoin"; }

public:
    QualifiedJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType joinType, std::unique_ptr<JoinSpecExp> joinSpecExp);

    JoinSpecExp const* GetJoinSpec() const { return GetChild<JoinSpecExp> (m_nJoinSpecIndex);}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct RelationshipJoinExp: JoinExp
    {    
DEFINE_EXPR_TYPE(RelationshipJoin) 
public:
    enum class ClassLocation
        {
        NotResolved=0,
        ExistInSource = 1,
        ExistInTarget = 2,
        ExistInBoth = ExistInSource | ExistInTarget
        }
    ;
    struct ResolvedEndPoint
        {
        friend RelationshipJoinExp;
    private:
        ClassNameExp const*    m_classRef;
        ClassLocation           m_location;
        bool                   m_anyClass;
    protected:
        ResolvedEndPoint() :m_classRef (nullptr), m_location (ClassLocation::NotResolved), m_anyClass (false) {}
        void SetClassRef(ClassNameExp const* classRef)
            {
            m_classRef = classRef;
            }
        void SetLocation(ClassLocation location, bool append)
            {
            if (append)
                m_location = static_cast<ClassLocation>(static_cast<int>(m_location) | static_cast<int>(location));
            else
                m_location = location;
            }
        void SetAnyClass (bool isAnyClass)
            {
            m_anyClass = isAnyClass;
            }
    public:
        ClassNameExp const* GetClassNameRef() const { return m_classRef;}
        ClassLocation       GetLocation() const { return m_location;}
        bool                IsAnyClass () const { return m_anyClass; }
        };
private:
    JoinDirection           m_direction;
    size_t                  m_relationshipClassIndex;
    ResolvedEndPoint        m_resolvedFrom;
    ResolvedEndPoint        m_resolvedTo;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override;
    BentleyStatus ResolveRelationshipEnds (ECSqlParseContext&);
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext&, FinalizeParseMode mode) override;

public:
    RelationshipJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, std::unique_ptr<ClassRefExp> relationship, JoinDirection direction)
        : JoinExp(ECSqlJoinType::JoinUsingRelationship, move (from), move (to)), m_direction(direction)
        {
        m_relationshipClassIndex = AddChild (move (relationship));
        }

    ClassNameExp const& GetRelationshipClass() const { return *GetChild<ClassNameExp> (m_relationshipClassIndex);}

    ResolvedEndPoint const&  GetResolvedFromEndPoint() const { return m_resolvedFrom;}
    ResolvedEndPoint const&  GetResolvedToEndPoint() const { return m_resolvedTo;}

    JoinDirection GetDirection() const { return m_direction;}
    };


//************* JoinSpecExp subclasses **********************
//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct NamedPropertiesJoinExp : JoinSpecExp
    {
    DEFINE_EXPR_TYPE(NamedPropertiesJoin) 
private:
    std::vector<Utf8String> m_properties;

    virtual Utf8String _ToECSql() const override
        {
        Utf8String tmp = "USING (";
        auto end = &m_properties.at(m_properties.size() - 1);
        for (auto& property : m_properties)
            {
            tmp.append(property);
            if (&property != end)
                tmp.append(", ");
            }
        tmp.append(")");
        return tmp;
        }

    virtual Utf8String _ToString () const override
        {
        Utf8String str ("NamedPropertiesJoin [Properties: ");
        bool isFirstItem = true;
        for (auto const& propertyName : m_properties)
            {
            if (!isFirstItem)
                str.append (", ");

            str.append (propertyName.c_str ());
            isFirstItem = false;
            }

        str.append ("]");
        return str;
        }

public:
    NamedPropertiesJoinExp() : JoinSpecExp () {}

    void Append(Utf8StringCR propertyName) { m_properties.push_back(propertyName); }
    };

struct BooleanExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct JoinConditionExp: JoinSpecExp
    {
    DEFINE_EXPR_TYPE(JoinCondition)
private:
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "JoinCondition"; }

public:
    explicit JoinConditionExp(std::unique_ptr<BooleanExp> searchCondition);

    BooleanExp const* GetSearchCondition() const {return GetChild<BooleanExp> (0);}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

