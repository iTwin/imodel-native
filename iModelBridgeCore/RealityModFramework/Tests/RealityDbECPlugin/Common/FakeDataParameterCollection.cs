using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;

namespace IndexECPlugin.Tests.Common
    {
    class FakeDataParameterCollection : IDataParameterCollection
        {
        private readonly List<IDbDataParameter> _list = new List<IDbDataParameter>();

        public int Count
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public bool IsFixedSize
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public bool IsReadOnly
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public bool IsSynchronized
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public object this[int i]
            {
            get
                {
                throw new NotImplementedException();
                }
            set
                {
                throw new NotImplementedException();
                }
            }

        public object this[string s]
            {
            get
                {
                throw new NotImplementedException();
                }
            set
                {
                throw new NotImplementedException();
                }
            }

        public object SyncRoot
            {
            get
                {
                throw new NotImplementedException();
                }
            }

        public int Add (object parameter)
            {
            IDbDataParameter dbDataParameter = parameter as IDbDataParameter;
            if ( dbDataParameter == null )
                {
                return -1;
                }
            _list.Add((IDbDataParameter) parameter);
            return _list.Count - 1;
            }

        public void Clear ()
            {
            throw new NotImplementedException();
            }

        public bool Contains (Object parameter)
            {
            return _list.Contains((IDbDataParameter)parameter);
            }

        public bool Contains (String name)
            {
            throw new NotImplementedException();
            }

        public void CopyTo (Array array, int startIndex)
            {
            throw new NotImplementedException();
            }

        public IEnumerator GetEnumerator ()
            {
            throw new NotImplementedException();
            }

        public int IndexOf (object parameter)
            {
            throw new NotImplementedException();
            }

        public int IndexOf (string name)
            {
            throw new NotImplementedException();
            }

        public void Insert (int index, object parameter)
            {
            throw new NotImplementedException();
            }

        public void Remove (object parameter)
            {
            throw new NotImplementedException();
            }

        public void RemoveAt (int index)
            {
            throw new NotImplementedException();
            }

        public void RemoveAt (string name)
            {
            throw new NotImplementedException();
            }
        }
    }
