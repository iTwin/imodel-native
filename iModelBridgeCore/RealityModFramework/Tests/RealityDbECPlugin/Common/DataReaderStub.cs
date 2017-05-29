using System;
using System.Data;

namespace IndexECPlugin.Tests.Common
    {
    internal class DataReaderStub : IDataReader
    {
        private int _currentIndex;

        public bool CanRead
            {
            get;
            set;
            }

        public byte[] ByteBuffer
            {
            get;
            set;
            }

        public void Close ()
            {
            throw new NotImplementedException();
            }

        public void Dispose ()
            {
            
            }

        public bool Read ()
            {
            return CanRead;
            }

        public long GetBytes (int column, long fieldOffset, byte[] buffer, int bufferOffset, int length)
        {
            int bytesLeft = ByteBuffer.Length - _currentIndex;
            if ( length > bytesLeft )
                {
                length = bytesLeft;
                }

            for ( int i = 0; i < length; i++ )
                {
                buffer[bufferOffset + i] = ByteBuffer[fieldOffset + i];
                }
            _currentIndex += length;

            return length;
            }

        public int Depth
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public int FieldCount
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public bool IsClosed
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public object this[int index]
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public object this[string name]
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public int RecordsAffected
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public bool GetBoolean (int column)
            {
            throw new NotImplementedException();
            }

        public byte GetByte (int column)
            {
            throw new NotImplementedException();
            }

        public char GetChar (int column)
            {
            throw new NotImplementedException();
            }

        public long GetChars (int column, long fieldOffset, char[] buffer, int bufferOffset, int length)
            {
            throw new NotImplementedException();
            }

        public IDataReader GetData (int column)
            {
            throw new NotImplementedException();
            }

        public string GetDataTypeName (int field)
            {
            throw new NotImplementedException();
            }

        public DateTime GetDateTime (int field)
            {
            throw new NotImplementedException();
            }

        public Decimal GetDecimal (int field)
            {
            throw new NotImplementedException();
            }

        public Double GetDouble (int field)
            {
            throw new NotImplementedException();
            }

        public Type GetFieldType (int field)
            {
            throw new NotImplementedException();
            }

        public float GetFloat (int field)
            {
            throw new NotImplementedException();
            }

        public Guid GetGuid (int field)
            {
            throw new NotImplementedException();
            }

        public short GetInt16 (int field)
            {
            throw new NotImplementedException();
            }

        public int GetInt32 (int field)
            {
            throw new NotImplementedException();
            }

        public long GetInt64 (int field)
            {
            throw new NotImplementedException();
            }

        public string GetName (int field)
            {
            throw new NotImplementedException();
            }

        public int GetOrdinal (string name)
            {
            throw new NotImplementedException();
            }

        public DataTable GetSchemaTable ()
            {
            throw new NotImplementedException();
            }

        public string GetString (int field)
            {
            throw new NotImplementedException();
            }

        public object GetValue (int field)
            {
            throw new NotImplementedException();
            }

        public int GetValues (object[] values)
            {
            throw new NotImplementedException();
            }

        public bool IsDBNull (int field)
            {
            throw new NotImplementedException();
            }

        public bool NextResult ()
            {
            throw new NotImplementedException();
            }
        }
    }
