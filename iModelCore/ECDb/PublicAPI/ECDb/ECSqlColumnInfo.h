/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/DateTime.h>

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
struct ECSqlPropertyPath final
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
        struct Entry final : RefCountedBase
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
                ECN::ECPropertyCP m_property = nullptr;
                int m_arrayIndex = -1;

                explicit Entry(ECN::ECPropertyCR ecProperty) : m_property(&ecProperty) {}
                explicit Entry(int arrayIndex) : m_arrayIndex(arrayIndex) {}

            public:
#if !defined (DOCUMENTATION_GENERATOR)
                static RefCountedPtr<Entry> Create(ECN::ECPropertyCR ecProperty);
                static RefCountedPtr<Entry> Create(int arrayIndex);
#endif

                ~Entry() {}

                //! Gets the kind of this entry.
                //! @return Entry kind.
                Kind GetKind() const { return m_property != nullptr ? Kind::Property : Kind::ArrayIndex; }

                //! Gets the property path entry's ECProperty, if the entry holds a property,
                //! i.e. Entry::GetKind is Kind::Property
                //! @return Property path entry's ECProperty or nullptr if the entry is an array index
                ECN::ECPropertyCP GetProperty() const { return m_property; }

                //! Gets the array index if this entry represents an array element,
                //! i.e. Entry::GetKind is Kind::ArrayIndex.
                //! @return Array index or negative number if entry is a property
                int GetArrayIndex() const { return m_arrayIndex; }

#if !defined (DOCUMENTATION_GENERATOR)
                //! Sets the array index in this entry, if this entry is of the Kind::ArrayIndex
                //! kind.
                //! @param[in] newArrayIndex Array index to set
                //! @return ::SUCCESS in case of success, ::ERROR if the entry was no
                //! array index entry.
                BentleyStatus SetArrayIndex(int newArrayIndex);
#endif
            };

        typedef RefCountedPtr<Entry> EntryPtr;
        typedef Entry const& EntryCR;
        typedef Entry const* EntryCP;

        //=======================================================================================
        //! Iterator over an ECSqlPropertyPath.
        //! The iterator iterates from the top-level entry to the leaf entry
        // @bsiclass                                                 Krischan.Eberle    06/2013
        //+===============+===============+===============+===============+===============+======
        struct const_iterator : std::iterator<std::forward_iterator_tag, EntryCP>
            {
            private:
                bvector<EntryPtr>::const_iterator m_innerIterator;
            public:
#if !defined (DOCUMENTATION_GENERATOR)
                explicit const_iterator(bvector<EntryPtr>::const_iterator);
#endif

                ~const_iterator() {}

                EntryCP operator*() const { return m_innerIterator->get(); }
                const_iterator& operator++() { m_innerIterator++; return *this; }
                bool operator==(const_iterator const& rhs) const { return m_innerIterator == rhs.m_innerIterator; }
                bool operator!=(const_iterator const& rhs) const { return !(*this == rhs); }
            };


    private:
        bvector<EntryPtr> m_entryList;

        EntryPtr const& GetEntry(size_t index) const { return m_entryList.at(index); }

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        ECSqlPropertyPath();

#endif
        ~ECSqlPropertyPath() {}

        //! Gets the number of entries in the property path
        //! @return Number of entries in property path
        size_t Size() const { return m_entryList.size(); }

        //! Gets the property path entry at the specified @p index
        //! @remarks The 0-based index is counted from top-level property to leaf property, i.e. the index represents
        //!          the nesting level within the property path. So index 0 points to the 
        //!          top-level property (root property), and the greatest
        //!          index (ECSqlPropertyPath::Size - 1) points to the leaf property.
        //!          Callers must make sure that @p index is not out of bounds. Otherwise the behavior is undefined.
        //! @param[in] index Index of entry to be returned
        //! @return Property path entry
        EntryCR At(size_t index) const { return *GetEntry(index); }

        //! Gets the leaf entry of the path, i.e the entry to which this path points to.
        //! @return Property path's leaf entry
        EntryCR GetLeafEntry() const { BeAssert(Size() > 0); return At(Size() - 1); }


        //! Gets an iterator pointing to the beginning of this property path.
        //! @return Iterator pointing to the beginning of this property path.
        ECDB_EXPORT const_iterator begin() const;
        //! Gets an iterator pointing to the end of this property path.
        //! @return Iterator pointing to the end of this property path.
        ECDB_EXPORT const_iterator end() const;

        //! Gets the property path formatted as a property access string.
        //! @param[in] option Specify one of the format option
        //! @return Property path as string.
        ECDB_EXPORT Utf8String ToString(FormatOptions option = FormatOptions::Default) const;

#if !defined (DOCUMENTATION_GENERATOR)
        //! Appends a property entry to the end of this path.
        void AddEntry(ECN::ECPropertyCR ecProperty);
        //! Appends an array index entry to the end of this path.
        void AddEntry(int arrayIndex);
        //! Inserts the entries of the specified path at the beginning of this path
        void InsertEntriesAtBeginning(ECSqlPropertyPath const& pathToInsert);
        //! Sets the specified array index in the leaf entry.
        //! @param[in] newArrayIndex Array index to set
        //! @return ::SUCCESS if this entry is an array index entry. ::ERROR if this entry is no array index entry
        BentleyStatus SetLeafArrayIndex(int newArrayIndex);
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
struct ECSqlColumnInfo final
    {
    //! Represents the ECClass of the top-level ECProperty backing this column.
    //! @remarks When nesting into a struct or array this method still always returns the ECClass
    //! from the top-level property, i.e. it is an ECClass from the FROM or JOIN clause. 
    //! It does not return the ECClass of ECSqlColumnInfo::GetProperty.
    //! @e Example
    //! For the ECSQL <c>SELECT Address FROM myschema.Company</c> where Address is a property of the ECStruct @c Location,
    //! GetRootClass of the first column returns the ECN::ECClass 'Company' and not the ECStruct type 'Location'.
    struct RootClass final
        {
        private:
            ECN::ECClassCP m_class = nullptr;
            Utf8String m_tableSpace;
            Utf8String m_alias;

        public:
#if !defined (DOCUMENTATION_GENERATOR)
            RootClass();
            RootClass(ECN::ECClassCR ecClass, Utf8CP tableSpace, Utf8CP alias = nullptr);
#endif
            bool IsValid() const { return m_class != nullptr; };
            //! Gets the ECClass of the top-level ECProperty backing this column.
            //! @return Column's ECClass reference
            ECN::ECClassCR GetClass() const { BeAssert(IsValid()); return *m_class; }
            //! Gets the table space in which this root class is persisted.
            //! @remarks for classes in the primary file the table space is MAIN. For classes in attached
            //! files, the table space is the name by which the file was attached (see BentleyApi::BeSQLite::Db::AttachDb)
            //! For generated properties the table space is empty
            Utf8StringCR GetTableSpace() const { BeAssert(IsValid()); return m_tableSpace; }
            //! Gets the class alias of the root class to which the column refers to.
            //! @return Alias of root class the column refers to or an empty string if no class alias was specified in the select clause
            Utf8StringCR GetAlias() const { BeAssert(IsValid()); return m_alias; }

        };
    private:
        ECN::ECTypeDescriptor m_dataType;
        DateTime::Info m_dateTimeInfo;
        ECN::ECStructClassCP m_structType = nullptr;
        ECN::ECPropertyCP m_property = nullptr;
        bool m_isSystemProperty = false;
        bool m_isGeneratedProperty = false;
        ECSqlPropertyPath m_propertyPath;
        RootClass m_rootClass;

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        ECSqlColumnInfo();
        ECSqlColumnInfo(ECN::ECTypeDescriptor const&, DateTime::Info const&, ECN::ECStructClassCP, ECN::ECPropertyCP, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const&, RootClass const&);
#endif

        ~ECSqlColumnInfo() {}

        //!Indicates whether this ECSqlColumnInfo object is valid.
        //!Invalid ECSqlColumnInfos can be returned for invalid calls to retrieve an ECSqlColumnInfo.
        //!@see ECSqlStatement::GetColumnInfo
        //!@return true or false
        bool IsValid() const { return m_rootClass.IsValid(); }

        //! Gets the data type of the column represented by this info object.
        //! @return Column data type
        ECN::ECTypeDescriptor const& GetDataType() const { return m_dataType; }
        
        //! Gets datetime metadata if the column's data type is DateTime or DateTime array.
        //! @return Datetime metadata for DateTime or DateTime array columns
        DateTime::Info const& GetDateTimeInfo() const { BeAssert((m_dataType.IsPrimitive() || m_dataType.IsPrimitiveArray()) && m_dataType.GetPrimitiveType() == ECN::PRIMITIVETYPE_DateTime); return m_dateTimeInfo; }

        //! Gets the struct type if the column's data type is struct or struct array.
        //! @return Struct type for struct or struct array columns
        ECN::ECStructClassCP GetStructType() const { BeAssert((m_dataType.IsStruct() || m_dataType.IsStructArray()) && m_structType != nullptr); return m_structType; }

        //! Gets the ECProperty backing the specified column
        //! @note When called from an IECSqlValue that represents an array @b element, this method will return nullptr.
        //! @return ECProperty backing the column represented by this info object.
        ECN::ECPropertyCP GetProperty() const { return m_property; }

        //! Gets the enum type if the column's data type is an ECEnumeration.
        //! @return Enumeration type for ECEnumeration columns and nullptr otherwise
        ECDB_EXPORT ECN::ECEnumerationCP GetEnumType() const;

        //! Indicates whether the property returned from ECSqlColumnInfo::GetProperty is an ECSQL system property.
        //! @return true if ECSqlColumnInfo::GetProperty is an ECSQL system property. false otherwise.
        //! @see @ref ECSqlSystemProperties
        bool IsSystemProperty() const { return m_isSystemProperty; }

        //! Indicates whether the property returned from GetProperty is a generated one or whether it 
        //! directly refers to an existing ECProperty of one of the classes in the FROM or JOIN clauses.
        //!
        //! For select clause expressions that do not simply refer to an ECProperty from the FROM or JOIN clauses, but instead
        //! use a column alias or are a computed expression, a property is dynamically generated that matches the expression.
        //!
        //! If this is a generated property, calling GetProperty returns the generated ECProperty
        //! which provides metadata about the computed field. GetProperty does not return a property of the FROM or JOIN classes
        //! in this case as there is no such matching property.
        //!
        //! @e Example
        //! For the ECSQL <c>SELECT AssetID, Length * Breadth AS Area FROM myschema.Cubicle</c> the first column (@c AssetID) would 
        //! not be a generated property, but the second (@c Area) would be. 
        //! @note Using a column alias always generates a property. So in the ECSQL <c>SELECT AssetID AS 'Cubicle ID' FROM myschema.Cubicle</c>
        //! the first column would be a generated property and its name would be 'Cubicle ID'.
        //! @return true, if the property for this column is generated. false,
        //! if the property for this column refers to one of the FROM or JOIN classes.
        bool IsGeneratedProperty() const { return m_isGeneratedProperty; }

        //! Gets the ECSQL property path of the specified column.
        //! @return Property path for the column represented by this info object.
        ECSqlPropertyPathCR GetPropertyPath() const { return m_propertyPath; }

        //! Gets the ECClass of the top-level ECProperty backing this column.
        //! @remarks When nesting into a struct or array this method still always returns the ECClass
        //! from the top-level property, i.e. it is an ECClass from the FROM or JOIN clause. 
        //! It does not return the ECClass of ECSqlColumnInfo::GetProperty.
        //! @e Example
        //! For the ECSQL <c>SELECT Address FROM myschema.Company</c> where Address is a property of the ECStruct @c Location,
        //! GetRootClass of the first column returns the ECN::ECClass 'Company' and not the ECStruct type 'Location'.
        //! @return Column's ECClass reference
        RootClass const& GetRootClass() const { BeAssert(IsValid() && "Must not call GetRootClass if IsValid is false"); return m_rootClass; }
    };

typedef ECSqlColumnInfo const& ECSqlColumnInfoCR;
END_BENTLEY_SQLITE_EC_NAMESPACE
