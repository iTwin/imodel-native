// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE
#pragma once

#ifndef CPPTL_JSON_H_INCLUDED
# define CPPTL_JSON_H_INCLUDED

#if !defined (JSON_IS_AMALGAMATION)
# include "forwards.h"
#endif // if !defined (JSON_IS_AMALGAMATION)

#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <cmath>

BEGIN_BENTLEY_NAMESPACE

typedef Utf8String Utf8StringAlias;

/** \brief JsonCpp JSON (JavaScript Object Notation) library.
 */
namespace Json {

   /** \brief Type of the value held by a Value object.
    */
   enum ValueType : unsigned char
   {
      nullValue = 0, ///< 'null' value
      intValue,      ///< signed integer value
      uintValue,     ///< unsigned integer value
      realValue,     ///< double value
      stringValue,   ///< UTF-8 string value
      booleanValue,  ///< bool value
      arrayValue,    ///< array value (ordered list)
      objectValue    ///< object value (collection of name/value pairs).
   };

   /** \brief Lightweight wrapper to tag static string.
    *
    * Value constructor and objectValue member assignement takes advantage of the
    * StaticString and avoid the cost of string duplication when storing the
    * string or the member name.
    *
    * Example of usage:
    * \code
    * Json::Value aValue( StaticString("some text") );
    * Json::Value object;
    * static const StaticString code("code");
    * object[code] = 1234;
    * \endcode
    */
   class StaticString
   {
   public:
      explicit StaticString( const char *czstring ) : str_(czstring) {}
      operator const char *() const {return str_;}
      const char *c_str() const {return str_;}

   private:
      const char *str_;
   };

   /** \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
    *
    * This class is a discriminated union wrapper that can represents a:
    * - signed integer [range: Value::minInt - Value::maxInt]
    * - unsigned integer (range: 0 - Value::maxUInt)
    * - double
    * - UTF-8 string
    * - boolean
    * - 'null'
    * - an ordered list of Value
    * - collection of name/value pairs (javascript object)
    *
    * The type of the held value is represented by a #ValueType and 
    * can be obtained using type().
    *
    * values of an #objectValue or #arrayValue can be accessed using operator[]() methods. 
    * Non const methods will automatically create the a #nullValue element 
    * if it does not exist. 
    * The sequence of an #arrayValue will be automatically resize and initialized 
    * with #nullValue. resize() can be used to enlarge or truncate an #arrayValue.
    *
    * The get() methods can be used to obtanis default value in the case the required element
    * does not exist.
    *
    * It is possible to iterate over the list of a #objectValue values using 
    * the getMemberNames() method.
    */
   class JSON_API Value 
   {
      friend class ValueIteratorBase;
   public:
      typedef bvector<Utf8StringAlias> Members;
      typedef ValueIterator iterator;
      typedef ValueConstIterator const_iterator;
      typedef Json::UInt UInt;
      typedef Json::Int Int;
      typedef Json::UInt32 UInt32; // BeJsonCpp
      typedef Json::UInt64 UInt64;
      typedef Json::Int64 Int64;
      typedef Json::LargestInt LargestInt;
      typedef Json::LargestUInt LargestUInt;
      typedef Json::ArrayIndex ArrayIndex;

      static const Value null;
      /// Minimum signed integer value that can be stored in a Json::Value.
      static const LargestInt minLargestInt;
      /// Maximum signed integer value that can be stored in a Json::Value.
      static const LargestInt maxLargestInt;
      /// Maximum unsigned integer value that can be stored in a Json::Value.
      static const LargestUInt maxLargestUInt;

      /// Minimum signed int value that can be stored in a Json::Value.
      static const Int minInt;
      /// Maximum signed int value that can be stored in a Json::Value.
      static const Int maxInt;
      /// Maximum unsigned int value that can be stored in a Json::Value.
      static const UInt maxUInt;

      /// Minimum signed 64 bits int value that can be stored in a Json::Value.
      static const Int64 minInt64;
      /// Maximum signed 64 bits int value that can be stored in a Json::Value.
      static const Int64 maxInt64;
      /// Maximum unsigned 64 bits int value that can be stored in a Json::Value.
      static const UInt64 maxUInt64;

   private:
      class CZString 
      {
      public:
         static char* Malloc(unsigned int size) {return (char*) bentleyAllocator_malloc(size);}
         static void Free(char* str) {bentleyAllocator_free((void*) str);}
        
         static char* Duplicate(const char* value, unsigned int length=0)
          {
          if (length == 0) length = (unsigned int)strlen(value);
          char *newString = Malloc(length+1);
          memcpy(newString, value, length);
          newString[length] = 0;
          return newString;
          }
        
         enum DuplicationPolicy {noDuplication = 0, duplicate, duplicateOnCopy};
         CZString() : cstr_(nullptr), index_(0) {}
         CZString(ArrayIndex index) : cstr_(nullptr), index_(index) {}
         CZString(const char *cstr, DuplicationPolicy policy) : cstr_(policy==duplicate ? Duplicate(cstr) : cstr), index_(policy){}
         CZString(CZString const& other) : cstr_(other.index_ != noDuplication && other.cstr_ != nullptr ?  Duplicate(other.cstr_) : other.cstr_), 
                                           index_(other.cstr_ ? (other.index_ == noDuplication ? noDuplication : duplicate) : other.index_){}
         ~CZString() {if (cstr_ && index_ == duplicate) Free((char*)cstr_);}
         CZString& operator=(CZString const& other) {CZString temp(other); swap(temp); return *this;}
         bool operator<(CZString const& other) const {if (cstr_) return strcmp(cstr_, other.cstr_) < 0; return index_ < other.index_;}
         bool operator==(CZString const& other) const {if (cstr_) return strcmp(cstr_, other.cstr_) == 0; return index_ == other.index_;}
         ArrayIndex index() const {return index_;}
         const char *c_str() const {return cstr_;}
         bool isStaticString() const {return index_ == noDuplication;}
      private:
         void swap(CZString& other) {std::swap(cstr_, other.cstr_); std::swap(index_, other.index_);}
         const char *cstr_;
         ArrayIndex index_;
      };

   public:
      typedef bmap<CZString, Value> ObjectValues;

   public:
      /** \brief Create a default Value of the given type.

        This is a very useful constructor.
        To create an empty array, pass arrayValue.
        To create an empty object, pass objectValue.
        Another Value can then be set to this one by assignment.
    This is useful since clear() and resize() will not alter types.

        Examples:
    \code
    Json::Value null_value; // null
    Json::Value arr_value(Json::arrayValue); // []
    Json::Value obj_value(Json::objectValue); // {}
    \endcode
      */
      Value(ValueType type=ValueType::nullValue) : type_(type), allocated_(0)
        {
        switch (type)
           {
           case nullValue:
              break;
           case intValue:
           case uintValue:
              value_.int_ = 0;
              break;
           case realValue:
              value_.real_ = 0.0;
              break;
           case stringValue:
              value_.string_ = 0;
              break;
           case arrayValue:
           case objectValue:
              value_.map_ = new ObjectValues();
              break;
           case booleanValue:
              value_.bool_ = false;
              break;
           }
        }

      Value(Int value) : type_(intValue) {value_.int_ = value;}
      Value(UInt value) : type_(uintValue) {value_.uint_ = value;}

#if defined (__APPLE__)
#if !defined (__LP64__)
      Value( UInt32 value ); // BeJsonCpp
#else
      Value(long value);
      Value(unsigned long value);
#endif
#endif
      Value(Int64 value) : type_(intValue) {value_.int_ = value;}
      Value(UInt64 value) : type_(uintValue) {value_.uint_ = value;}
      Value(double value) : type_(realValue) {if (std::isnan(value)) {BeAssert (false); type_ = nullValue; return;} value_.real_ = value;}
      Value(const char *value) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(value ? value : "" );}
      Value(const char *beginValue, const char *endValue) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(beginValue, (unsigned int)(endValue - beginValue));}

      /** \brief Constructs a value from a static string.

       * Like other value string constructor but do not duplicate the string for
       * internal storage. The given string must remain alive after the call to this
       * constructor.
       * Example of usage:
       * \code
       * Json::Value aValue( StaticString("some text") );
       * \endcode
       */
      Value(StaticString const& value) : type_(stringValue), allocated_(false) {value_.string_ = const_cast<char *>(value.c_str());}
      Value(Utf8StringCR value) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(value.c_str(), (unsigned int)value.length());}
      Value(bool value) : type_(booleanValue) {value_.bool_ = value;}
      Value(Value const& other) : type_( other.type_ )
        {
        switch (type_)
           {
           case nullValue:
           case intValue:
           case uintValue:
           case realValue:
           case booleanValue:
              value_ = other.value_;
              break;
           case stringValue:
              if (other.value_.string_)
              {
                 value_.string_ = CZString::Duplicate(other.value_.string_);
                 allocated_ = 1;
              }
              else
                 value_.string_ = nullptr;
              break;
           case arrayValue:
           case objectValue:
              value_.map_ = new ObjectValues( *other.value_.map_ );
              break;
           }
        }

      Value(Value &&other) : type_(ValueType::nullValue), allocated_(0) {swap(other);}
      ~Value()
        {
        switch ( type_ )
           {
           case nullValue:
           case intValue:
           case uintValue:
           case realValue:
           case booleanValue:
              break;
           case stringValue:
              if (allocated_) CZString::Free(value_.string_);
              break;
           case arrayValue:
           case objectValue:
              delete value_.map_;
              break;
           }
        }


      Value& operator= (Value const& other) {Value temp(other); swap(temp); return *this;}
      Value& operator= (Value&& other) {swap(other); return *this;}

      /// Swap values.
      void swap(Value& other)
        {
        std::swap(type_, other.type_);
        std::swap(value_, other.value_);
        std::swap(allocated_, other.allocated_);
        }

      ValueType type() const {return type_;}

      bool operator <( const Value &other ) const;
      bool operator <=( const Value &other ) const {return !(other < *this);}
      bool operator >=( const Value &other ) const {return !(*this < other);}
      bool operator >( const Value &other ) const {return other < *this;}
      bool operator ==( const Value &other ) const;
      bool operator !=( const Value &other ) const;

      int compare( const Value &other ) const {if (*this < other) return -1; if (*this > other) return 1; return 0;}

      const char *asCString() const {BeAssert(type_ == stringValue); return value_.string_;}
      Utf8String asString() const
        {
        switch (type_)
           {
           case stringValue:
              return value_.string_;
           case booleanValue:
              return value_.bool_ ? "true" : "false";
           case intValue:
           case uintValue:
           case realValue:
           case arrayValue:
           case objectValue:
              BeAssert(false);
           }
        return Utf8String();
        }

      Int asInt() const;
      UInt asUInt() const;
      Int64 asInt64() const;
      UInt64 asUInt64() const;
      LargestInt asLargestInt() const {return asInt64();}
      LargestUInt asLargestUInt() const {return asUInt64();}
      float asFloat() const;
      double asDouble() const;
      bool asBool() const;

      bool isNull() const {return type_ == nullValue;}
      bool isBool() const {return type_ == booleanValue;}
      bool isInt() const {return type_ == intValue;}
      bool isUInt() const {return type_ == uintValue;}
      bool isIntegral() const {return type_ == intValue || type_ == uintValue || type_ == booleanValue;}
      bool isDouble() const {return type_ == realValue;}
      bool isNumeric() const {return isIntegral() || isDouble();}
      bool isString() const {return type_ == stringValue;}
      bool isArray() const {return type_ == nullValue || type_ == arrayValue;}
      bool isObject() const {return type_ == nullValue || type_ == objectValue;}
      bool isConvertibleTo( ValueType other ) const;

      /// Number of values in array or object
      ArrayIndex size() const;

      /// \brief Return true if empty array, empty object, or null;
      /// otherwise, false.
      bool empty() const {if (isNull() || isArray() || isObject()) return size() == 0u; else return false;}

      /// Return isNull()
      bool operator!() const {return isNull();}

      /// Remove all object members and array elements.
      /// \pre type() is arrayValue, objectValue, or nullValue
      /// \post type() is unchanged
      void clear();

      /// Resize the array to size elements. 
      /// New elements are initialized to null.
      /// May only be called on nullValue or arrayValue.
      /// \pre type() is arrayValue or nullValue
      /// \post type() is arrayValue
      void resize( ArrayIndex size );

      /// Access an array element (zero based index ).
      /// If the array contains less than index element, then null value are inserted
      /// in the array so that its size is index+1.
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      Value &operator[]( ArrayIndex index );

      /// Access an array element (zero based index ).
      /// If the array contains less than index element, then null value are inserted
      /// in the array so that its size is index+1.
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      Value &operator[]( int index ) {BeAssert(index >= 0); return (*this)[ArrayIndex(index)];}

      /// Access an array element (zero based index )
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      const Value &operator[]( ArrayIndex index ) const;

      /// Access an array element (zero based index )
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      const Value &operator[]( int index ) const  {BeAssert(index >= 0); return (*this)[ArrayIndex(index)];}

      /// If the array contains at least index+1 elements, returns the element value, 
      /// otherwise returns defaultValue.
      Value get( ArrayIndex index, 
                 const Value &defaultValue ) const;
      /// Return true if index < size().
      bool isValidIndex( ArrayIndex index ) const;
      /// \brief Append value to array at the end.
      ///
      /// Equivalent to jsonvalue[jsonvalue.size()] = value;
      Value &append( const Value &value );

      /// Access an object value by name, create a null member if it does not exist.
      Value &operator[]( const char *key ) {return resolveReference(key, false);}

      /// Access an object value by name, returns null if there is no member with that name.
      const Value &operator[]( const char *key ) const;

      /// Access an object value by name, create a null member if it does not exist.
      Value &operator[]( const Utf8StringAlias &key ) {return (*this)[key.c_str()];}

      /// Access an object value by name, returns null if there is no member with that name.
      const Value &operator[]( const Utf8StringAlias &key ) const {return (*this)[key.c_str()];}

      /** \brief Access an object value by name, create a null member if it does not exist.

       * If the object as no entry for that name, then the member name used to store
       * the new entry is not duplicated.
       * Example of use:
       * \code
       * Json::Value object;
       * static const StaticString code("code");
       * object[code] = 1234;
       * \endcode
       */
      Value &operator[]( const StaticString &key ) {return resolveReference(key, true);}

      /// Return the member named key if it exist, defaultValue otherwise.
      Value get( const char *key, 
                 const Value &defaultValue ) const;
      /// Return the member named key if it exist, defaultValue otherwise.
      Value get( const Utf8StringAlias &key,
                 const Value &defaultValue ) const;
      /// \brief Remove and return the named member.  
      ///
      /// Do nothing if it did not exist.
      /// \return the removed Value, or null.
      /// \pre type() is objectValue or nullValue
      /// \post type() is unchanged
      Value removeMember( const char* key );

      /// Same as removeMember(const char*)
      Value removeMember(Utf8StringCR key) {return removeMember(key.c_str());}

      /** \brief Remove the indexed array element.

          O(n) expensive operations.
          Update 'removed' iff removed.
          \return true iff removed (no exceptions)
          */
      bool removeIndex(ArrayIndex i, Value* removed);
      bool removeIndex(ArrayIndex i) {Value removed; return removeIndex(i, &removed);}

      /// Return true if the object has a member named key.
      bool isMember(const char *key) const {const Value *value = &((*this)[key]); return value != &null;}

      /// Return true if the object has a member named key.
      bool isMember(Utf8StringCR key) const {return isMember(key.c_str());}

      /// \brief Return a list of the member names.
      ///
      /// If null, return an empty list.
      /// \pre type() is objectValue or nullValue
      /// \post if type() was nullValue, it remains nullValue
      Members getMemberNames() const;

      Utf8StringAlias toStyledString() const;

      const_iterator begin() const;
      const_iterator end() const;

      iterator begin();
      iterator end();

   private:
      Value &resolveReference(const char *key, bool isStatic);

      union ValueHolder
      {
         LargestInt int_;
         LargestUInt uint_;
         double real_;
         bool bool_;
         char *string_;
         ObjectValues *map_;
      } value_;
      ValueType type_;
      bool allocated_;
   };


   /** \brief Experimental and untested: represents an element of the "path" to access a node.
    */
   class PathArgument
   {
   public:
      friend class Path;

      PathArgument();
      PathArgument( ArrayIndex index );
      PathArgument( const char *key );
      PathArgument( const Utf8StringAlias &key );

   private:
      enum Kind
      {
         kindNone = 0,
         kindIndex,
         kindKey
      };
      Utf8StringAlias key_;
      ArrayIndex index_;
      Kind kind_;
   };

   /** \brief Experimental and untested: represents a "path" to access a node.
    *
    * Syntax:
    * - "." => root node
    * - ".[n]" => elements at index 'n' of root node (an array value)
    * - ".name" => member named 'name' of root node (an object value)
    * - ".name1.name2.name3"
    * - ".[0][1][2].name1[3]"
    * - ".%" => member name is provided as parameter
    * - ".[%]" => index is provied as parameter
    */
   class Path
   {
   public:
      Path( const Utf8StringAlias &path,
            const PathArgument &a1 = PathArgument(),
            const PathArgument &a2 = PathArgument(),
            const PathArgument &a3 = PathArgument(),
            const PathArgument &a4 = PathArgument(),
            const PathArgument &a5 = PathArgument() );

      const Value &resolve( const Value &root ) const;
      Value resolve( const Value &root, 
                     const Value &defaultValue ) const;
      /// Creates the "path" to access the specified node and returns a reference on the node.
      Value &make( Value &root ) const;

   private:
      typedef bvector<const PathArgument *> InArgs;
      typedef bvector<PathArgument> Args;

      void makePath( const Utf8StringAlias &path,
                     const InArgs &in );
      void addPathInArg( const Utf8StringAlias &path, 
                         const InArgs &in, 
                         InArgs::const_iterator &itInArg, 
                         PathArgument::Kind kind );
      void invalidPath( const Utf8StringAlias &path, 
                        int location );

      Args args_;
   };


   /** \brief base class for Value iterators.
    *
    */
   class ValueIteratorBase
   {
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef ValueIteratorBase SelfType;

      ValueIteratorBase();
      explicit ValueIteratorBase( const Value::ObjectValues::iterator &current );

      bool operator ==( const SelfType &other ) const
      {
         return isEqual( other );
      }

      bool operator !=( const SelfType &other ) const
      {
         return !isEqual( other );
      }

      difference_type operator -( const SelfType &other ) const
      {
         return computeDistance( other );
      }

      /// Return either the index or the member name of the referenced value as a Value.
      Value key() const;

      /// Return the index of the referenced Value. -1 if it is not an arrayValue.
      UInt index() const;

      /// Return the member name of the referenced Value. "" if it is not an objectValue.
      const char *memberName() const;

   protected:
      Value &deref() const;

      void increment();

      void decrement();

      difference_type computeDistance( const SelfType &other ) const;

      bool isEqual( const SelfType &other ) const;

      void copy( const SelfType &other );

   private:
      Value::ObjectValues::iterator current_;
      // Indicates that iterator is for a null value.
      bool isNull_;
   };

   /** \brief const iterator for object and array value.
    *
    */
   class ValueConstIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef const Value &reference;
      typedef const Value *pointer;
      typedef ValueConstIterator SelfType;

      ValueConstIterator();
   private:
      /*! \internal Use by Value to create an iterator.
       */
      explicit ValueConstIterator( const Value::ObjectValues::iterator &current );
   public:
      SelfType &operator =( const ValueIteratorBase &other );

      SelfType operator++( int )
      {
         SelfType temp( *this );
         ++*this;
         return temp;
      }

      SelfType operator--( int )
      {
         SelfType temp( *this );
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };


   /** \brief Iterator for object and array value.
    */
   class ValueIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef Value &reference;
      typedef Value *pointer;
      typedef ValueIterator SelfType;

      ValueIterator();
      ValueIterator( const ValueConstIterator &other );
      ValueIterator( const ValueIterator &other );
   private:
      /*! \internal Use by Value to create an iterator.
       */
      explicit ValueIterator( const Value::ObjectValues::iterator &current );
   public:

      SelfType &operator =( const SelfType &other );

      SelfType operator++( int )
      {
         SelfType temp( *this );
         ++*this;
         return temp;
      }

      SelfType operator--( int )
      {
         SelfType temp( *this );
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };


} // namespace Json

typedef Json::Value& JsonValueR;
typedef Json::Value const& JsonValueCR;
typedef Json::Value const* JsonValueCP;

END_BENTLEY_NAMESPACE

#endif // CPPTL_JSON_H_INCLUDED
