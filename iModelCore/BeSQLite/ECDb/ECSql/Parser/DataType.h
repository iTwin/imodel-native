#pragma once



namespace com {
    namespace  sun {
        namespace  star {
            namespace sdbc {

                struct  DataType
                    {
                    const static long BIT = -7;

                    const static long TINYINT = -6;

                    const static long SMALLINT = 5;

                    const static long INTEGER = 4;

                    const static long BIGINT = -5;

                    const static long FLOAT = 6;

                    const static long REAL = 7;

                    const static long DOUBLE = 8;

                    const static long NUMERIC = 2;

                    const static long DECIMAL = 3;

                    const static long CHAR = 1;

                    const static long VARCHAR = 12;

                    const static long LONGVARCHAR = -1;

                    const static long DATE = 91;

                    const static long TIME = 92;

                    const static long TIMESTAMP = 93;

                    const static long BINARY = -2;

                    const static long VARBINARY = -3;

                    const static long LONGVARBINARY = -4;

                    const static long SQLNULL = 0;


                    /** indicates that the SQL type is database-specific and
                             gets mapped to an object that can be accessed via
                             the method
                             <member scope="com::sun::star::sdbc">XRow::getObject()</member>
                             .
                             */
                    const static long OTHER = 1111;


                    /** indicates a type which is represented by an object which implements
                             this type.
                             */
                    const static long OBJECT = 2000;


                    /** describes a type based on a built-in type.
                                 It is a user-defined data type (UDT).
                                 */
                    const static long DISTINCT = 2001;


                    /** indicates a type consisting of attributes that may be any type.
                                 It is a user-defined data type (UDT).
                                 */
                    const static long STRUCT = 2002;


                    /** indicates a type representing an SQL ARRAY.
                     */
                    const static long ARRAY = 2003;


                    /** indicates a type representing an SQL Binary Large Object.
                     */
                    const static long BLOB = 2004;


                    /** indicates a type representing an SQL Character Large Object.
                     */
                    const static long CLOB = 2005;


                    /** indicates a type representing an SQL REF, a referencing type.
                     */
                    const static long REF = 2006;

                    /** identifies the generic SQL type
                     * <code>BOOLEAN</code>.
                     *
                     * @since OOo 2.0
                     */
                    const static long BOOLEAN = 16;
                    };

                }
            }
        }
    }