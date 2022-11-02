// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE
#pragma once

# include "value.h"
# include <Bentley/WString.h>
# include <Bentley/bmap.h>
# include <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE
namespace Json 
{
   class Value;

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
    */
   class JSON_API StyledWriter
   {
   public:
      StyledWriter();

   public: // overridden from Writer
      /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
       * \param root Value to serialize.
       * \return String containing the JSON document that represents the root value.
       */
      Utf8String write( const Value &root );

   private:
      void writeValue( const Value &value );
      void writeArrayValue( const Value &value );
      bool isMultineArray( const Value &value );
      void pushValue( const Utf8String &value );
      void writeIndent();
      void writeWithIndent( const Utf8String &value );
      void indent();
      void unindent();
      static Utf8String normalizeEOL( const Utf8String &text );

      typedef bvector<Utf8String> ChildValues;

      ChildValues childValues_;
      Utf8String document_;
      Utf8String indentString_;
      int rightMargin_;
      int indentSize_;
      bool addChildValues_;
   };

   Utf8String JSON_API valueToString(Int value);
   Utf8String JSON_API valueToString(UInt value);
   Utf8String JSON_API valueToString(LargestInt value);
   Utf8String JSON_API valueToString(LargestUInt value);
   Utf8String JSON_API valueToString(double value);
   Utf8String JSON_API valueToString(bool value);
   Utf8String JSON_API valueToQuotedString(const char *value);

} // namespace Json

END_BENTLEY_NAMESPACE

