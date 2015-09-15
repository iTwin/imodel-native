/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECSqlColumnInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! The property path represents a parsed property access string with typed access to the
//! items in the property path.
//!
//! The property path starts from an outermost top-level property and nests down to a given
//! property or array index.
//! An entry in a property path can either be an ECN::ECProperty or an array index. Make sure
//! to call ECSqlPropertyPath::Entry::GetKind first before accessing the content of an entry.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyPath
    {
public:
    //=======================================================================================
    //! Format Option
    // @bsiclass                                                 Affan.Khan    08/2014
    //+===============+===============+===============+===============+===============+======
    enum class FormatOptions
        {
        Simple, //!< For array property no index or descriptor is added.
        WithArrayIndex, //!< For array property add array index in square-brackets.
        WithArrayDescriptor, //!< For array property add only square-brackets without index.
        Default = WithArrayIndex, //!< Same as WithArrayIndex
        };

    //=======================================================================================
    //! Represents an entry in an ECSqlPropertyPath
    // @bsiclass                                                 Krischan.Eberle    06/2013
    //+===============+===============+===============+===============+===============+======
    struct Entry
        {
    public:
        //=======================================================================================
        //! Entry kind
        // @bsiclass                                                 Krischan.Eberle    06/2013
        //+===============+===============+===============+===============+===============+======
        enum class Kind
            {
            Property, //!< Entry is an ECN::ECProperty
            ArrayIndex //!< Entry is an array index
            };

    private:
        ECN::ECPropertyCP m_property;
        int m_arrayIndex;

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        explicit Entry (ECN::ECPropertyCR ecProperty);
        explicit Entry (int arrayIndex);
        Entry (Entry const& rhs);
        Entry& operator= (Entry const& rhs);
        Entry (Entry&& rhs);
        Entry& operator= (Entry&& rhs);
#endif

        //! Gets the kind of this entry.
        //! @return Entry kind.
        ECDB_EXPORT Kind GetKind () const;

        //! Gets the property path entry's ECProperty, if the entry holds a property,
        //! i.e. Entry::GetKind is Kind::Property
        //! @return Property path entry's ECProperty or nullptr if the entry is an array index
        ECDB_EXPORT ECN::ECPropertyCP GetProperty () const;

        //! Gets the array index if this entry represents an array element,
        //! i.e. Entry::GetKind is Kind::ArrayIndex.
        //! @return Array index or negative number if entry is a property
        ECDB_EXPORT int GetArrayIndex () const;

#if !defined (DOCUMENTATION_GENERATOR)
        //! Sets the array index in this entry, if this entry is of the Kind::ArrayIndex
        //! kind.
        //! @param[in] newArrayIndex Array index to set
        //! @return ::SUCCESS in case of success, ::ERROR if the entry was no
        //! array index entry.
        BentleyStatus SetArrayIndex (int newArrayIndex);
#endif
        };

    typedef Entry const* EntryCP;
    typedef Entry const& EntryCR;

    //=======================================================================================
    //! Iterator over an ECSqlPropertyPath.
    //! The iterator iterates from the top-level entry to the leaf entry
    // @bsiclass                                                 Krischan.Eberle    06/2013
    //+===============+===============+===============+===============+===============+======
    struct const_iterator : std::iterator<std::forward_iterator_tag, EntryCP>
        {
    #if !defined (DOCUMENTATION_GENERATOR)
    public:
        struct Impl;
    #endif

    private:
        Impl* m_pimpl;

        Impl const& GetPimpl () const;
        Impl& GetPimplR () const;
    public:
    #if !defined (DOCUMENTATION_GENERATOR)
        explicit const_iterator (Impl* impl);
    #endif
        ECDB_EXPORT ~const_iterator ();

        ECDB_EXPORT EntryCP operator* () const;
        ECDB_EXPORT const_iterator& operator++ ();
        ECDB_EXPORT bool operator== (const_iterator const& rhs) const;
        ECDB_EXPORT bool operator!= (const_iterator const& rhs) const;
        };

#if !defined (DOCUMENTATION_GENERATOR)
    struct Impl;
#endif

private:
    Impl* m_pimpl;

public:
#if !defined (DOCUMENTATION_GENERATOR)
    //! Initializes a new empty ECSqlPropertyPath object
    ECSqlPropertyPath ();
    ~ECSqlPropertyPath ();

    ECSqlPropertyPath (ECSqlPropertyPath const&);
    ECSqlPropertyPath& operator= (ECSqlPropertyPath const&);
    ECSqlPropertyPath (ECSqlPropertyPath&&);
    ECSqlPropertyPath& operator= (ECSqlPropertyPath&&);
#endif

    //! Gets the number of entries in the property path
    //! @return Number of entries in property path
    ECDB_EXPORT size_t Size () const;

    //! Gets the property path entry at the specified @p index
    //! @remarks The 0-based index is counted from top-level property to leaf property, i.e. the index represents
    //!          the nesting level within the property path. So index 0 points to the 
    //!          top-level property (root property), and the greatest
    //!          index (ECSqlPropertyPath::Size - 1) points to the leaf property.
    //!          Callers must make sure that @p index is not out of bounds. Otherwise the behavior is undefined.
    //! @param[in] index Index of entry to be returned
    //! @return Property path entry
    ECDB_EXPORT EntryCR At (size_t index) const;
    
    //! Gets the leaf entry of the path, i.e the entry to which this path points to.
    //! @return Property path's leaf entry
    ECDB_EXPORT EntryCR GetLeafEntry () const;


    //! Gets an iterator pointing to the beginning of this property path.
    //! @return Iterator pointing to the beginning of this property path.
    ECDB_EXPORT const_iterator begin () const;
    //! Gets an iterator pointing to the end of this property path.
    //! @return Iterator pointing to the end of this property path.
    ECDB_EXPORT const_iterator end () const;

    //! Gets the property path formatted as a property access string.
    //! @param[in] option Specify one of the format option
    //! @return Property path as string.
    ECDB_EXPORT Utf8String ToString (FormatOptions option = FormatOptions::Default) const;

#if !defined (DOCUMENTATION_GENERATOR)
    //! Appends a property entry to the end of this path.
    void AddEntry (ECN::ECPropertyCR ecProperty);
    //! Appends an array index entry to the end of this path.
    void AddEntry (int arrayIndex);
    //! Inserts the entries of the specified path at the beginning of this path
    void InsertEntriesAtBeginning (ECSqlPropertyPath const& pathToInsert);
    //! Sets the specified array index in the leaf entry.
    //! @param[in] newArrayIndex Array index to set
    //! @return ::SUCCESS if this entry is an array index entry. ::ERROR if this entry is no array index entry
    BentleyStatus SetLeafArrayIndex (int newArrayIndex);
#endif
    };

typedef ECSqlPropertyPath const& ECSqlPropertyPathCR;

//=======================================================================================
//! Holds the metadata for a given item in the result set of an ECSQL query.
//! @remarks When nesting into the content of struct and array items of the result set,
//! each nesting level provides its own ECSqlColumnInfo.
//! See @ref ECDbCodeSamples for examples.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlColumnInfo
    {
private:
    ECN::ECTypeDescriptor m_dataType;
    ECN::ECPropertyCP m_property;
    bool m_isGeneratedProperty;
    ECSqlPropertyPath m_propertyPath;
    ECN::ECClassCP m_rootClass;
    Utf8String m_rootClassAlias;

    ECSqlColumnInfo (ECN::ECTypeDescriptor const& dataType, ECN::ECPropertyCP ecProperty, bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias);

    static ECN::ECTypeDescriptor DetermineDataType (ECN::ECPropertyCR ecProperty);

public:
#if !defined (DOCUMENTATION_GENERATOR)

    //! Initializes a new empty ECSqlColumnInfo object
    ECSqlColumnInfo ();
    static ECSqlColumnInfo CreateTopLevel (bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias);
    static ECSqlColumnInfo CreateChild (ECSqlColumnInfo const& parent, ECN::ECPropertyCR childProperty);
    static ECSqlColumnInfo CreateForArrayElement (ECSqlColumnInfo const& parent, int arrayIndex);

    ~ECSqlColumnInfo ();

    ECSqlColumnInfo (ECSqlColumnInfo const&);
    ECSqlColumnInfo& operator= (ECSqlColumnInfo const&);
    ECSqlColumnInfo (ECSqlColumnInfo&&);
    ECSqlColumnInfo& operator= (ECSqlColumnInfo&&);

    ECSqlPropertyPath& GetPropertyPathR ();
#endif
public:
    //! Gets the data type of the column represented by this info object.
    //! @return Column data type
    ECDB_EXPORT ECN::ECTypeDescriptor const& GetDataType () const;

    //! Gets the ECProperty backing the specified column
    //! @note When called from an IECSqlArrayReader for a primitive array property,
    //! this method will return nullptr, as no ECProperties exist for elements of a primitive array.
    //! @return ECProperty backing the column represented by this info object.
    ECDB_EXPORT ECN::ECPropertyCP GetProperty () const;

    //! Indicates whether the property returned from GetProperty is a generated one or whether it 
    //! directly refers to an existing ECProperty of one of the classes in the FROM or JOIN clauses.
    //!
    //! For select clause expressions that do not simply refer to an ECProperty from the FROM or JOIN clauses, but instead
    //! use a column alias or are a computed expression, a property is dynamically generated that matches the expression.
    //!
    //! If this is a generated property, calling GetProperty returns the generated ECProperty
    //! which provides metadata about the computed field. GetProperty does not return a property of the FROM or JOIN classes
    //! in this case as there is no such matching property.
    //! However, if the generated property is just an alias for the original property, the generated ECProperty carries 
    //! the custom attribute @c DefinitionMetaData from the standard ECSchema @em Bentley_Standard_CustomAttributes 
    //! that points back to the original ECProperty.
    //!
    //! @e Example
    //! For the ECSQL <c>SELECT AssetID, Length * Breadth AS Area FROM myschema.Cubicle</c> the first column (@c AssetID) would 
    //! not be a generated property, but the second (@c Area) would be. 
    //! @note Using a column alias always generates a property. So in the ECSQL <c>SELECT AssetID AS 'Cubicle ID' FROM myschema.Cubicle</c>
    //! the first column would be a generated property and its name would be 'Cubicle ID'.
    //! @return true, if the property for this column is generated. false,
    //! if the property for this column refers to one of the FROM or JOIN classes.
    ECDB_EXPORT bool IsGeneratedProperty () const;
    
    //! Gets the ECSQL property path of the specified column.
    //! @return Property path for the column represented by this info object.
    ECDB_EXPORT ECSqlPropertyPathCR GetPropertyPath () const;

    //! Gets the ECClass of the top-level ECProperty backing this column.
    //! @remarks When nesting into a struct or array this method still always returns the ECClass
    //! from the top-level property, i.e. it is an ECClass from the FROM or JOIN clause. 
    //! It does not return the ECClass of ECSqlColumnInfo::GetProperty.
    //! @e Example
    //! For the ECSQL <c>SELECT Address FROM myschema.Company</c> where Address is a property of the ECStruct @c Location,
    //! GetRootClass of the first column returns the ECN::ECClass 'Company' and not the ECStruct type 'Location'.
    //! @return Column's ECClass reference
    ECDB_EXPORT ECN::ECClassCR GetRootClass () const;
    
    //! Gets the class alias of the root class to which the column refers to.
    //! @return Alias of root class the column refers to or nullptr if no class alias was specified in the select clause
    //! @see GetRootClass
    ECDB_EXPORT Utf8CP GetRootClassAlias () const;
    };

typedef ECSqlColumnInfo const& ECSqlColumnInfoCR;
END_BENTLEY_SQLITE_EC_NAMESPACE
