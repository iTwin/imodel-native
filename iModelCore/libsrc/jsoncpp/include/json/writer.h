// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_WRITER_H_INCLUDED
# define JSON_WRITER_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
# include "value.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

#if defined (BEJSONCPP_USE_STDSTRING)
# include <string>
# include <vector>
# define Utf8String std::string
# define bmap std::map
# define bvector std::vector
#else
# include <Bentley/WString.h>
# include <Bentley/bmap.h>
# include <Bentley/bvector.h>
#endif

#if defined (BEJSONCPP_ALLOW_IOSTREAM)
# include <iostream>
#endif

namespace Json {

   class Value;

   /** \brief Abstract class for writers.
    */
   class JSON_API Writer
   {
   public:
      virtual ~Writer();

      virtual Utf8String write( const Value &root ) = 0;
   };

   /** \brief Outputs a Value in <a HREF="http://www.json.org">JSON</a> format without formatting (not human friendly).
    *
    * The JSON document is written in a single line. It is not intended for 'human' consumption,
    * but may be usefull to support feature such as RPC where bandwith is limited.
    * \sa Reader, Value
    */
   class JSON_API FastWriter : public Writer
   {
   public:
      FastWriter();
      virtual ~FastWriter(){}

      void enableYAMLCompatibility();

   public: // overridden from Writer
      virtual Utf8String write( const Value &root );
      static Utf8String ToString (const Value &root );

   private:
      void writeValue( const Value &value );

      Utf8String document_;
      bool yamlCompatiblityEnabled_;
   };

   /** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a human friendly way.
    *
    * The rules for line break and indent are as follow:
    * - Object value:
    *     - if empty then print {} without indent and line break
    *     - if not empty the print '{', line break & indent, print one value per line
    *       and then unindent and line break and print '}'.
    * - Array value:
    *     - if empty then print [] without indent and line break
    *     - if the array contains no object value, empty array or some other value types,
    *       and all the values fit on one lines, then print the array on a single line.
    *     - otherwise, it the values do not fit on one line, or the array contains
    *       object or non empty array, then print one value per line.
    *
    * If the Value have comments then they are outputed according to their #CommentPlacement.
    *
    * \sa Reader, Value, Value::setComment()
    */
   class JSON_API StyledWriter: public Writer
   {
   public:
      StyledWriter();
      virtual ~StyledWriter(){}

   public: // overridden from Writer
      /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
       * \param root Value to serialize.
       * \return String containing the JSON document that represents the root value.
       */
      virtual Utf8String write( const Value &root );

   private:
      void writeValue( const Value &value );
      void writeArrayValue( const Value &value );
      bool isMultineArray( const Value &value );
      void pushValue( const Utf8String &value );
      void writeIndent();
      void writeWithIndent( const Utf8String &value );
      void indent();
      void unindent();
      void writeCommentBeforeValue( const Value &root );
      void writeCommentAfterValueOnSameLine( const Value &root );
      bool hasCommentForValue( const Value &value );
      static Utf8String normalizeEOL( const Utf8String &text );

      typedef bvector<Utf8String> ChildValues;

      ChildValues childValues_;
      Utf8String document_;
      Utf8String indentString_;
      int rightMargin_;
      int indentSize_;
      bool addChildValues_;
   };

#if defined (BEJSONCPP_ALLOW_IOSTREAM)
   /** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a human friendly way,
        to a stream rather than to a string.
    *
    * The rules for line break and indent are as follow:
    * - Object value:
    *     - if empty then print {} without indent and line break
    *     - if not empty the print '{', line break & indent, print one value per line
    *       and then unindent and line break and print '}'.
    * - Array value:
    *     - if empty then print [] without indent and line break
    *     - if the array contains no object value, empty array or some other value types,
    *       and all the values fit on one lines, then print the array on a single line.
    *     - otherwise, it the values do not fit on one line, or the array contains
    *       object or non empty array, then print one value per line.
    *
    * If the Value have comments then they are outputed according to their #CommentPlacement.
    *
    * \param indentation Each level will be indented by this amount extra.
    * \sa Reader, Value, Value::setComment()
    */
   class JSON_API StyledStreamWriter
   {
   public:
      StyledStreamWriter( Utf8String indentation="\t" );
      ~StyledStreamWriter(){}

   public:
      /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
       * \param out Stream to write to. (Can be ostringstream, e.g.)
       * \param root Value to serialize.
       * \note There is no point in deriving from Writer, since write() should not return a value.
       */
      void write( std::ostream &out, const Value &root );

   private:
      void writeValue( const Value &value );
      void writeArrayValue( const Value &value );
      bool isMultineArray( const Value &value );
      void pushValue( const Utf8String &value );
      void writeIndent();
      void writeWithIndent( const Utf8String &value );
      void indent();
      void unindent();
      void writeCommentBeforeValue( const Value &root );
      void writeCommentAfterValueOnSameLine( const Value &root );
      bool hasCommentForValue( const Value &value );
      static Utf8String normalizeEOL( const Utf8String &text );

      typedef bvector<Utf8String> ChildValues;

      ChildValues childValues_;
      std::ostream* document_;
      Utf8String indentString_;
      int rightMargin_;
      Utf8String indentation_;
      bool addChildValues_;
   };
#endif

# if defined(JSON_HAS_INT64)
   Utf8String JSON_API valueToString( Int value );
   Utf8String JSON_API valueToString( UInt value );
# endif // if defined(JSON_HAS_INT64)
   Utf8String JSON_API valueToString( LargestInt value );
   Utf8String JSON_API valueToString( LargestUInt value );
   Utf8String JSON_API valueToString( double value );
   Utf8String JSON_API valueToString( bool value );
   Utf8String JSON_API valueToQuotedString( const char *value );

#if defined (BEJSONCPP_ALLOW_IOSTREAM)
   /// \brief Output using the StyledStreamWriter.
   /// \see Json::operator>>()
   std::ostream& operator<<( std::ostream&, const Value &root );
#endif

} // namespace Json



#endif // JSON_WRITER_H_INCLUDED
