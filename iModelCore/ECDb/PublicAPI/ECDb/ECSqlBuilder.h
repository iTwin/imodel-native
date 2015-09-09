/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECSqlBuilder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! For a JOIN USING clause the values of this enum specify which end of the relationship
//! the joined class refers to.
//! The metaphor of a direction is used to express this. The forward direction means that 
//! the JOIN expression in the ECSQL statement goes from the ECRelationship's source constraint
//! to the target constraint, whereas the reverse direction goes from target to source constraint.
//! 
//! @remarks In most cases the direction can be implied from the ECSQL 
//! statement directly. But there are case where this is ambiguous, e.g. for joins between 
//! the same class.
//! @see ECSqlSelectBuilder::Using, ECN::ECRelationshipClass::GetSource, ECN::ECRelationshipClass::GetTarget
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle      04/2013
//+===============+===============+===============+===============+===============+======
enum class JoinDirection
    {
    Implied = 0, //!< The direction can be implied from the ECSQL statement.
    Forward = 1, //!< JOIN expression goes from source to target constraint of the ECN::ECRelationshipClass.
    Reverse = 2 //!< JOIN expression goes from target to source constraint of the ECN::ECRelationshipClass.
    };

#if !defined (DOCUMENTATION_GENERATOR)
//**************** ClassClause **************************************************

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2013
//+===============+===============+===============+===============+===============+======
struct ClassClause
    {
private:
    ECN::ECClassCP m_class;
    Utf8String m_alias;
    bool m_isPolymorphic;

public:
    ClassClause ();
    ClassClause (ECN::ECClassCR ecClass, Utf8CP alias, bool isPolymorphic);
    ClassClause (ClassClause const& rhs);
    ~ClassClause () {};
    ClassClause& operator= (ClassClause const& rhs);
    ClassClause (ClassClause&& rhs);
    ClassClause& operator= (ClassClause&& rhs);

    bool operator== (ClassClause const& rhs) const;
    bool operator!= (ClassClause const& rhs) const;

    bool IsNull () const;
    ECN::ECClassCR GetClass () const {return *m_class;}
    Utf8StringCR GetAlias () const;
    bool GetIsPolymorphic () const {return m_isPolymorphic;}

    Utf8String ToString () const;
    Utf8String ToString (bool ignoreIsPolymorphic) const;

    //! Builds the ECSQL snippet from the specified ECClass.
    //! @remarks Only does the pure conversion from the class to
    //! the direct ECSQL snippet without applyingr polymorphism or aliases
    //! @param[in] ecClass ECClass to create the ECSQL snippet for
    //! @return ECSQL class snippet
    static Utf8String ToString (ECN::ECClassCR ecClass);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      01/2013
//+===============+===============+===============+===============+===============+======
struct SelectClause
    {
private:
    bool m_isNull;
    bool m_selectAll;
    //unsupported for now -> always false
    bool m_selectDistinct;
    Utf8String m_clause;

public:
    SelectClause ();
    explicit SelectClause (bool selectAll);
    explicit SelectClause (Utf8CP selectClause);
    ~SelectClause ();
    SelectClause (SelectClause const& rhs);
    SelectClause& operator= (SelectClause const& rhs);
    SelectClause (SelectClause&& rhs);
    SelectClause& operator= (SelectClause&& rhs);

    bool operator== (SelectClause const& rhs) const;
    bool operator!= (SelectClause const& rhs) const;

    bool IsNull () const;
    bool IsSelectAll () const;
    Utf8StringCR GetClause () const;

    Utf8String ToString () const;
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct JoinUsingClause
    {
private:
    ClassClause m_relationshipClassClause;
    JoinDirection m_joinDirection;

public:
    JoinUsingClause ();
    JoinUsingClause (ECN::ECRelationshipClassCR relationshipClass, Utf8CP alias = nullptr, JoinDirection joinDirection = JoinDirection::Implied);
    JoinUsingClause (JoinUsingClause const& rhs);
    ~JoinUsingClause ();
    JoinUsingClause& operator= (JoinUsingClause const& rhs);
    JoinUsingClause (JoinUsingClause&& rhs);
    JoinUsingClause& operator= (JoinUsingClause&& rhs);
    bool operator== (JoinUsingClause const& rhs) const;
    bool operator!= (JoinUsingClause const& rhs) const;

    bool IsNull () const;
    ECN::ECRelationshipClassCR GetRelationshipClass () const;
    JoinDirection GetJoinDirection () const;

    Utf8String ToString () const;
    };


//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman      08/2012
//+===============+===============+===============+===============+===============+======
struct WhereClause
    {
public:
    enum Type
        {
        WHERE_TYPE_None,
        WHERE_TYPE_Relationship,
        WHERE_TYPE_Expression
        };

private:
        Type m_type;
        Utf8String m_expression;
        Utf8String m_sourceClassName;
        Utf8String m_sourceECInstanceIdExpression;
        Utf8String m_targetClassName;
        Utf8String m_targetECInstanceIdExpression;
public:
        WhereClause ();
        explicit WhereClause (Utf8CP expression);
        WhereClause (Utf8CP sourceClassName, Utf8CP sourceECInstanceIdExpression, Utf8CP targetClassName, Utf8CP targetECInstanceIdExpression);
        WhereClause (WhereClause const& rhs);
        ~WhereClause ();
        WhereClause& operator= (WhereClause const& rhs);
        WhereClause (WhereClause&& rhs);
        WhereClause& operator= (WhereClause&& rhs);

        bool operator== (WhereClause const& rhs) const;
        bool operator!= (WhereClause const& rhs) const;

        bool IsNull () const;

        Type GetType () const;
        bool GetRelationshipEnd (Utf8CP& className, Utf8CP& ecInstanceIdExpression, ECN::ECRelationshipEnd relationshipEnd) const;
        Utf8StringCR GetExpression () const;

        Utf8String ToString () const;
    };
      
//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman      12/2012
//+===============+===============+===============+===============+===============+======
struct LimitClause 
    {
private:
    bool m_isNull;
    Utf8String m_limitClause;
    Utf8String m_offsetClause;
 
public:
    LimitClause ();
    explicit LimitClause (Utf8CP limitClause);
    LimitClause (Utf8CP limitClause, Utf8CP offsetClause);
    ~LimitClause ();
    LimitClause (LimitClause const& rhs);
    LimitClause& operator= (LimitClause const& rhs);
    LimitClause (LimitClause&& rhs);
    LimitClause& operator= (LimitClause&& rhs);

    bool operator== (LimitClause const& rhs) const;
    bool operator!= (LimitClause const& rhs) const;

    bool IsNull () const;

    Utf8StringCR GetLimitClause () const;
    bool HasOffsetClause () const;
    Utf8StringCR GetOffsetClause () const;

    Utf8String ToString () const;
    };

#endif // DOCUMENTATION_GENERATOR

//**************** ECSqlBuilder **************************************************
struct ECSqlBuilder;
typedef ECSqlBuilder& ECSqlBuilderR;
typedef ECSqlBuilder const& ECSqlBuilderCR;
typedef ECSqlBuilder* ECSqlBuilderP;
typedef ECSqlBuilder const* ECSqlBuilderCP;

//=======================================================================================
//! An ECSqlBuilder is used to conveniently build an @ref ECSQLOverview statement.
//! @see ECSqlSelectBuilder, ECSqlInsertBuilder, ECSqlUpdateBuilder, ECSqlDeleteBuilder, ECSqlStatement, @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle     01/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBuilder 
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    //=======================================================================================    
    //! Statement types that can be built with an ECSqlBuilder
    // @bsiclass                                                 Krischan.Eberle      01/2013
    //=======================================================================================    
    enum class StatementType
        {
        Select = 0,
        Insert,
        Update,
        Delete
        };
#endif

public:
    //! The name of the ECInstanceId system property used to reference the primary key of an ECClass in an ECSQL statement.
    //! E.g. <c>SELECT name FROM myschema.Foo WHERE <b>ECInstanceId</b> = 123</c>
    //! @see @ref ECInstanceIdInECDb
    ECDB_EXPORT static Utf8CP const ECINSTANCEID_SYSTEMPROPERTY;

private:
    StatementType m_statementType;

    //! Compares this builder with the specified one
    //! @param[in] rhs Builder to compare with
    //! @return true if both builders are equal. false otherwise
    virtual bool _Equals (ECSqlBuilderCR rhs) const = 0;

    //! Clones this builder.
    //! @remarks The caller is responsible for freeing the clone.
    //! @return Clone of this builder. The caller is responsible for freeing the clone.
    virtual ECSqlBuilderCP _Clone () const = 0;

    //! Creates a text representation of the content of this builder.
    //! @return text representation of this builder
    virtual Utf8String _ToString () const = 0;

//__PUBLISH_SECTION_END__
protected:
    //! Initializes a new instance of the ECSqlBuilder class.
    //! @remarks Derived classes *must* call this constructor in
    //!          order to ensure that the type is correctly set in the instance.
    //! @param[in] type statement type.
    explicit ECSqlBuilder (StatementType type);

    ECSqlBuilder (ECSqlBuilderCR rhs);
    ECSqlBuilderR operator= (ECSqlBuilderCR rhs);
    ECSqlBuilder (ECSqlBuilder&& rhs);
    ECSqlBuilderR operator= (ECSqlBuilder&& rhs);

//__PUBLISH_SECTION_START__

 public:
    virtual ~ECSqlBuilder () {}

    //! Compares this builder with the specified one for equality
    //! @param[in] rhs builder to compare with
    //! @return true if both builders are equal. false otherwise.
    ECDB_EXPORT bool operator== (ECSqlBuilderCR rhs) const;

    //! Compares this builder with the specified one for inequality
    //! @param[in] rhs builder to compare with
    //! @return true if the builders are not equal. false otherwise.
    ECDB_EXPORT bool operator!= (ECSqlBuilderCR rhs) const;

    //! Creates a text representation of the content of this builder.
    //! @return text representation of this builder
    ECDB_EXPORT Utf8String ToString () const;

    //! Creates the ECSQL snippet for the specified @p ecClass.
    //! @remarks this is a convenience methods for clients that build
    //! the ECSQL string manually, but want to leverage the logic of
    //! converting an ECClass to ECSQL (considering the schema prefix etc.)
    //! 
    //! @param[in] ecClass ECClass to create the ECSQL snippet for
    //! @return ECSQL class snippet
    ECDB_EXPORT static Utf8String ToECSqlSnippet (ECN::ECClassCR ecClass);

//__PUBLISH_SECTION_END__

    //! Clones this builder.
    //! @remarks The caller is responsible for freeing the clone.
    //! @return Clone of this builder. The caller is responsible for freeing the clone.
    ECSqlBuilderCP Clone () const;

    //! Gets the statement type of this builder.
    //! @return Statement type
    StatementType GetType () const;

//__PUBLISH_SECTION_START__
    };


//**************** ECSqlSelectBuilder **************************************************

struct ECSqlSelectBuilder;
typedef ECSqlSelectBuilder& ECSqlSelectBuilderR;
typedef ECSqlSelectBuilder const& ECSqlSelectBuilderCR;

//=======================================================================================
//! The ECSqlSelectBuilder is used to build ECSQL Select statements.
//! 
//! The API supports a [fluent interface style API] (http://en.wikipedia.org/wiki/Fluent_interface)
//! It allows you to write code that resembles a text ECSQL statement.
//! @see @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      08/2012
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlSelectBuilder : public ECSqlBuilder
    {
private:
    SelectClause m_select;
    ClassClause m_from;
    ClassClause m_join;
    JoinUsingClause m_using;
    WhereClause m_where;
    Utf8String m_orderBy;
    LimitClause m_limit;

    virtual bool _Equals (ECSqlBuilderCR rhs) const override;
    virtual ECSqlBuilderCP _Clone () const override;

    virtual Utf8String _ToString () const override;

public:
    ECDB_EXPORT ECSqlSelectBuilder();
    ECDB_EXPORT ECSqlSelectBuilder (ECSqlSelectBuilderCR rhs);
    ECDB_EXPORT ECSqlSelectBuilderR operator= (ECSqlSelectBuilderCR rhs);
    ECDB_EXPORT ECSqlSelectBuilder (ECSqlSelectBuilder&& rhs);
    ECDB_EXPORT ECSqlSelectBuilderR operator= (ECSqlSelectBuilder&& rhs);
    ECDB_EXPORT ~ECSqlSelectBuilder ();

    //! Specifies the FROM clause
    //! @remarks If @p isPolymorphic is true, @p fromClass is interpreted polymorphically, i.e. subclasses will be included.
    //! The equivalent ECSQL string is <c>SELECT ... FROM myschema.Foo</c>. If @p isPolymorphic is false,
    //! no subclasses will be included. The equivalent ECSQL string is <c>SELECT ... FROM <b>ONLY</b> myschema.Foo</c>.
    //! @param[in] fromClass class being the target class in the FROM clause
    //! @param[in] fromAlias Alias for @p fromClass. Pass nullptr if not needed
    //! @param[in] isPolymorphic true, if the class in the FROM clause is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR From (ECN::ECClassCR fromClass, Utf8CP fromAlias = nullptr, bool isPolymorphic = true);
    //! Specifies the FROM clause
    //! @remarks If @p isPolymorphic is true, @p fromClass is interpreted polymorphically, i.e. subclasses will be included.
    //! The equivalent ECSQL string is <c>SELECT ... FROM myschema.Foo</c>. If @p isPolymorphic is false,
    //! no subclasses will be included. The equivalent ECSQL string is <c>SELECT ... FROM <b>ONLY</b> myschema.Foo</c>.
    //! @param[in] fromClass class being the target class in the FROM clause
    //! @param[in] isPolymorphic true, if the class in the FROM clause is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR From (ECN::ECClassCR fromClass, bool isPolymorphic);
    //! Specifies the ECSQL select clause
    //! @param selectClause ECSQL select clause
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Select (Utf8CP selectClause);
    //! Specifies the SELECT * clause
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR SelectAll ();

    //! Specifies a JOIN clause that is to be combined with a @ref ECSqlSelectBuilder::Using "USING" clause.
    //!
    //! The equivalent ECSQL clause is <c>SELECT .. FROM myschema.Foo <b>JOIN</b> myschema.Goo USING myschema.FooHasGoo</c>
    //! @param[in] joinClass Class to join to
    //! @param[in] joinClassAlias Alias for join class
    //! @param[in] isPolymorphic true, if the join class is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Join (ECN::ECClassCR joinClass, Utf8CP joinClassAlias = nullptr, bool isPolymorphic = true);
    //! Specifies a JOIN clause that is to be combined with a @ref ECSqlSelectBuilder::Using "USING" clause.
    //!
    //! The equivalent ECSQL clause is <c>SELECT .. FROM myschema.Foo <b>JOIN</b> myschema.Goo USING myschema.FooHasGoo</c>
    //! @param[in] joinClass Class to join to
    //! @param[in] isPolymorphic true, if the join class is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Join (ECN::ECClassCR joinClass, bool isPolymorphic);

    //! Specifies a USING clause that follows a @ref ECSqlSelectBuilder::Join "JOIN" clause
    //!
    //! The equivalent ECSQL clause is <c>SELECT .. FROM myschema.Foo JOIN myschema.Goo <b>USING</b> myschema.FooHasGoo</c>
    //! @param[in] relationshipClass ECRelationship defining the join condition between FROM class and JOIN class
    //! @param[in] relationshipClassAlias Alias for relationship class
    //! @param[in] joinDirection Direction of the JOIN expression in the ECSQL. In most of the
    //!            cases the direction can be implied. So only pass a value other than JoinDirection::Implied if the 
    //!            direction is ambiguous. An example for that are joins between the same class.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Using (ECN::ECRelationshipClassCR relationshipClass, Utf8CP relationshipClassAlias = nullptr, JoinDirection joinDirection = JoinDirection::Implied);

    //! Specifies a USING clause that follows a @ref ECSqlSelectBuilder::Join "JOIN" clause
    //!
    //! The equivalent ECSQL clause is <c>SELECT .. FROM myschema.Foo JOIN myschema.Goo <b>USING</b> myschema.FooHasGoo</c>
    //! @param[in] relationshipClass ECRelationship defining the join condition between FROM class and JOIN class
    //! @param[in] joinDirection Direction of the JOIN expression in the ECSQL. In most of the
    //!            cases the direction can be implied. So only pass a value other than JoinDirection::Implied if the 
    //!            direction is ambiguous. An example for that are joins between the same class.
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Using (ECN::ECRelationshipClassCR relationshipClass, JoinDirection joinDirection);

    //! Specifies the WHERE clause
    //! @param[in] whereClause WHERE clause. Properties can be qualified by using the ECClass name, e.g. MyClass.Foo < 3
    //! @remarks Use ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY when you need to specify an expression that involves 
    //! the system property @b %ECInstanceId 
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Where (Utf8CP whereClause);
    //! Specifies a WHERE clause for looking up an ECRelationship instance by its source key
    //! @param[in] sourceClass Source end class of relationship
    //! @param[in] sourceECInstanceId ECInstanceId of source end class of relationship
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR WhereSourceEndIs (Utf8CP sourceClass, Utf8CP sourceECInstanceId);
    //! Specifies a WHERE clause for looking up an ECRelationship instance by its target key
    //! @param[in] targetClass Target end class of relationship
    //! @param[in] targetECInstanceId ECInstanceId of target end class of relationship
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR WhereTargetEndIs (Utf8CP targetClass, Utf8CP targetECInstanceId);
    //! Specifies a WHERE clause for looking up an ECRelationship instance by its source and target key
    //! @param[in] sourceClass Source end class of relationship
    //! @param[in] sourceECInstanceId ECInstanceId of source end class of relationship
    //! @param[in] targetClass Target end class of relationship
    //! @param[in] targetECInstanceId ECInstanceId of target end class of relationship
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR WhereRelationshipEndsAre (Utf8CP sourceClass, Utf8CP sourceECInstanceId, Utf8CP targetClass, Utf8CP targetECInstanceId);
    
    //! Specifies ORDER BY clause
    //! @param[in] orderByClause ORDER BY clause
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR OrderBy (Utf8CP orderByClause);
    //! Specifies the LIMIT clause
    //! @param[in] limitClause LIMIT clause
    //! @param[in] offsetClause OFFSET clause (optional)
    //! @return this builder instance
    ECDB_EXPORT ECSqlSelectBuilderR Limit (Utf8CP limitClause, Utf8CP offsetClause = nullptr);

//__PUBLISH_SECTION_END__
    SelectClause const& GetSelectClause () const {return m_select;}
    ClassClause const& GetFromClause () const {return m_from;}
    ClassClause const& GetJoinClause () const { return m_join; }
    JoinUsingClause const& GetUsingClause () const { return m_using; }
    WhereClause const& GetWhereClause () const { return m_where; }
    Utf8StringCR GetOrderByClause () const {return m_orderBy;}
    LimitClause const& GetLimitClause () const {return m_limit;}
//__PUBLISH_SECTION_START__
    };


//**************** ECSqlInsertBuilder **************************************************

struct ECSqlInsertBuilder;
typedef ECSqlInsertBuilder& ECSqlInsertBuilderR;
typedef ECSqlInsertBuilder const& ECSqlInsertBuilderCR;

//=======================================================================================
//! The ECSqlInsertBuilder is used to build ECSQL Insert statements.
//! 
//! The API supports a [fluent interface style API] (http://en.wikipedia.org/wiki/Fluent_interface)
//! It allows you to write code that resembles a text ECSQL statement.
//! @see @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle      01/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlInsertBuilder : public ECSqlBuilder
    {
private:
    ClassClause m_targetClass;
    //target properties
    bvector<Utf8String> m_targetPropertiesClause;
    bvector<Utf8String> m_valuesClause;

    virtual bool _Equals (ECSqlBuilderCR rhs) const override;
    virtual ECSqlBuilderCP _Clone () const override;
    virtual Utf8String _ToString () const override;

public:
    ECDB_EXPORT ECSqlInsertBuilder ();
    ECDB_EXPORT ECSqlInsertBuilder (ECSqlInsertBuilderCR rhs);
    ECDB_EXPORT ECSqlInsertBuilderR operator= (ECSqlInsertBuilderCR rhs);
    ECDB_EXPORT ECSqlInsertBuilder (ECSqlInsertBuilder&& rhs);
    ECDB_EXPORT ECSqlInsertBuilderR operator= (ECSqlInsertBuilder&& rhs);
    ECDB_EXPORT ~ECSqlInsertBuilder ();

    //! Specifies the INSERT INTO clause which indicates the class the statement performs upon
    //! @param[in] targetClass Target class for the Insert statement
    //! @return this builder instance
    ECDB_EXPORT ECSqlInsertBuilderR InsertInto (ECN::ECClassCR targetClass);
    
    //! Adds an item to the VALUES clause along with the property into which it is to be inserted.
    //! 
    //! Example: The code
    //! 
    //!     ECSqlInsertBuilder builder;
    //!     builder.InsertInto (myFooClass).AddValue ("MyProp1", "1").AddValue ("MyProp2", "'hello'");
    //! 
    //! is equivalent to the ECSQL statement <c>INSERT INTO myschema.myFooClass (MyProp1, MyProp2) VALUES (1, 'hello')</c>
    //! @note The primary key of the row to insert is the @ref ECInstanceIdInECDb "ECInstanceId". It is automatically issued by ECDb. 
    //! Trying to insert a value into it via the %ECInstanceId system property therefore fails.
    //! @param[in] targetProperty Property into which @p targetValue is to be inserted
    //! @param[in] targetValue Value to insert
    //! @return this builder instance
    ECDB_EXPORT ECSqlInsertBuilderR AddValue (Utf8CP targetProperty, Utf8CP targetValue);

    //! A convenience method that will create the entire ECSql for INSERTing a given ECClass with all of its properties as well
    //! all of the inherited properties.  The VALUES will be named parameters in the format of :&lt;PropertyName&gt;
    //! @param[in] targetClass Target class for the Insert statement
    //! @return The ECSql string to be used with an ECSqlStatement
    ECDB_EXPORT static Utf8String BuildECSqlForClass(ECN::ECClassCR targetClass);

    //! A convenience method that will create the entire ECSql for INSERTing a given ECClass.  This overload takes a list
    //! of parameters to be bound, instead of using all of the properties from an ECClass.
    //! The VALUES will be named parameters in the format of :&lt;ParameterName&gt;
    //! @param[in] targetClass Target class for the Insert statement
    //! @param[in] insertParams List of parameters to use for the INSERT statement
    //! @return The ECSql string to be used with an ECSqlStatement
    ECDB_EXPORT static Utf8String BuildECSqlForClass(ECN::ECClassCR targetClass, bvector<Utf8String>& insertParams);

//__PUBLISH_SECTION_END__
    ClassClause const& GetTargetClass () const;
    bvector<Utf8String> const& GetTargetPropertiesClause () const;
    bvector<Utf8String> const& GetValuesClause () const;
    bool TryFindECInstanceIdProperty (int& propertyIndex) const;
//__PUBLISH_SECTION_START__
    };

//**************** ECSqlUpdateBuilder **************************************************
struct ECSqlUpdateBuilder;
typedef ECSqlUpdateBuilder& ECSqlUpdateBuilderR;
typedef ECSqlUpdateBuilder const& ECSqlUpdateBuilderCR;

//=======================================================================================
//! The ECSqlUpdateBuilder is used to build ECSQL Update statements.
//! 
//! The API supports a [fluent interface style API] (http://en.wikipedia.org/wiki/Fluent_interface)
//! It allows you to write code that resembles a text ECSQL statement.
//! @see @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle      01/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlUpdateBuilder : public ECSqlBuilder
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    typedef bpair<Utf8String, Utf8String> SetClauseItem;
    typedef bvector<SetClauseItem > SetClause;
#endif

private:
    ClassClause m_targetClass;
    SetClause m_setClause;
    WhereClause m_where;

    virtual bool _Equals (ECSqlBuilderCR rhs) const override;
    virtual ECSqlBuilderCP _Clone () const override;
    virtual Utf8String _ToString () const override;

public:
    ECDB_EXPORT ECSqlUpdateBuilder ();
    ECDB_EXPORT ECSqlUpdateBuilder (ECSqlUpdateBuilderCR rhs);
    ECDB_EXPORT ECSqlUpdateBuilderR operator= (ECSqlUpdateBuilder&& rhs);
    ECDB_EXPORT ECSqlUpdateBuilder (ECSqlUpdateBuilder&& rhs);
    ECDB_EXPORT ECSqlUpdateBuilderR operator= (ECSqlUpdateBuilderCR rhs);
    ECDB_EXPORT ~ECSqlUpdateBuilder ();

    //! Specifies the UPDATE clause which indicates the class the statement performs upon.
    //! @remarks If \p isPolymorphic is true, \p targetClass is interpreted polymorphically, i.e. subclasses will be included.
    //! The equivalent ECSQL string is <c>UPDATE myschema.Foo SET ... </c>. If \p isPolymorphic is false,
    //! no subclasses will be included. The equivalent ECSQL string is <c>UPDATE <b>ONLY</b> myschema.Foo SET ...</c>.
    //! @param[in] targetClass target ECClass of the UPDATE statement
    //! @param[in] isPolymorphic true, if the class in the FROM clause is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlUpdateBuilderR Update (ECN::ECClassCR targetClass, bool isPolymorphic = true);
    
    //! Adds an expression to the SET clause.
    //! Example: The code
    //! @code
    //! ECSqlUpdateBuilder builder;
    //! builder.Update (myFooClass).AddSet ("MyProp1", "1").AddSet ("MyProp2", "'hello'");
    //! @endcode
    //! is equivalent to the ECSQL statement
    //! @code UPDATE ONLY myschema.Foo MyProp1 = 1, MyProp2 = 'hello' @endcode
    //! @note The primary key of a row is the @ref ECInstanceIdInECDb "ECInstanceId". It is managed by ECDb. Trying to 
    //! update it via the %ECInstanceId system property therefore fails.
    //! @param[in] propertyName Name of ECProperty making up the LHS of the set clause expression.
    //! @param[in] rhsExpression RHS of the set clause expression
    ECDB_EXPORT ECSqlUpdateBuilderR AddSet (Utf8CP propertyName, Utf8CP rhsExpression);

    //! Specifies the WHERE clause
    //! @param[in] whereClause WHERE clause.
    //! @remarks Use ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY when you need to specify an expression that involves 
    //! the system property @b %ECInstanceId 
    //! @return this builder instance
    ECDB_EXPORT ECSqlUpdateBuilderR Where (Utf8CP whereClause);

//__PUBLISH_SECTION_END__
    ClassClause const& GetTargetClass () const;
    SetClause const& GetSetClause () const;
    WhereClause const& GetWhereClause () const;
//__PUBLISH_SECTION_START__
    };

//**************** ECSqlDeleteBuilder **************************************************
struct ECSqlDeleteBuilder;
typedef ECSqlDeleteBuilder& ECSqlDeleteBuilderR;
typedef ECSqlDeleteBuilder const& ECSqlDeleteBuilderCR;

//=======================================================================================
//! The ECSqlDeleteBuilder is used to build ECSQL Delete statements.
//! 
//! The API supports a [fluent interface style API] (http://en.wikipedia.org/wiki/Fluent_interface)
//! It allows you to write code that resembles a text ECSQL statement.
//! @see @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle      01/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlDeleteBuilder : public ECSqlBuilder
    {
private:
    ClassClause m_targetClass;
    WhereClause m_where;

    virtual bool _Equals (ECSqlBuilderCR rhs) const override;
    virtual ECSqlBuilderCP _Clone () const override;
    virtual Utf8String _ToString () const override;

public:
    ECDB_EXPORT ECSqlDeleteBuilder ();
    ECDB_EXPORT ECSqlDeleteBuilder (ECSqlDeleteBuilderCR rhs);
    ECDB_EXPORT ECSqlDeleteBuilderR operator= (ECSqlDeleteBuilderCR rhs);
    ECDB_EXPORT ECSqlDeleteBuilder (ECSqlDeleteBuilder&& rhs);
    ECDB_EXPORT ECSqlDeleteBuilderR operator= (ECSqlDeleteBuilder&& rhs);
    ECDB_EXPORT ~ECSqlDeleteBuilder ();

    //! Specifies the DELETE FROM clause which indicates the class the statement performs upon.
    //! @remarks If @p isPolymorphic is true, @p targetClass is interpreted polymorphically, i.e. subclasses will be included.
    //! The equivalent ECSQL string is <c>DELETE FROM myschema.Foo</c>. If @p isPolymorphic is false,
    //! no subclasses will be included. The equivalent ECSQL string is <c>DELETE FROM <b>ONLY</b> myschema.Foo</c>.
    //! @param[in] targetClass target ECClass of the UPDATE statement
    //! @param[in] isPolymorphic true, if the class in the FROM clause is to be used polymorphic. false, otherwise.
    //! @return this builder instance
    ECDB_EXPORT ECSqlDeleteBuilderR DeleteFrom (ECN::ECClassCR targetClass, bool isPolymorphic = true);

    //! Specifies the WHERE clause
    //! @param[in] whereClause WHERE clause.
    //! @remarks Use ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY when you need to specify an expression that involves 
    //! the system property @b %ECInstanceId 
    //! @return this builder instance
    ECDB_EXPORT ECSqlDeleteBuilderR Where (Utf8CP whereClause);

//__PUBLISH_SECTION_END__
    ClassClause const& GetTargetClass () const;
    WhereClause const& GetWhereClause () const;
//__PUBLISH_SECTION_START__
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
