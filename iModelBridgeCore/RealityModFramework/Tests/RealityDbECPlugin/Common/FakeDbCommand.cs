using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;

namespace IndexECPlugin.Tests.Common
    {
    internal delegate int DbCommandDelegate ();

    class FakeDbCommand : IDbCommand
        {
        private readonly FakeDataParameterCollection _parameterCollection = new FakeDataParameterCollection();
        private readonly List<IDbDataParameter> _dbDataParametersToCreate = new List<IDbDataParameter>();
        private DbCommandDelegate _executeNonQueryBehavior = () => 0;

        public List<IDbDataParameter> DbDataParametersToCreate
            {
            get
                {
                return _dbDataParametersToCreate;
                }
            }

        public void SetExecuteNonQueryBehavior (DbCommandDelegate behavior)
            {
            _executeNonQueryBehavior = behavior;
            }

        public string CommandText
            {
            get;
            set;
            }

        public int CommandTimeout
            {
            get;
            set;
            }

        public CommandType CommandType
            {
            get;
            set;
            }

        public IDbConnection Connection
            {
            get;
            set;
            }

        public IDataParameterCollection Parameters
            {
            get
                {
                return _parameterCollection;
                }
            }

        public IDbTransaction Transaction
            {
            get;
            set;
            }

        public UpdateRowSource UpdatedRowSource
            {
            get;
            set;
            }

        public void Cancel ()
            {
            throw new NotImplementedException();
            }

        public IDbDataParameter CreateParameter ()
            {
            IDbDataParameter parameter = _dbDataParametersToCreate.First();
            _dbDataParametersToCreate.RemoveAt(0);
            return parameter;
            }

        public void Dispose ()
            {

            }

        public int ExecuteNonQuery ()
            {
            return _executeNonQueryBehavior();
            }

        public IDataReader ExecuteReader ()
            {
            throw new NotImplementedException();
            }

        public IDataReader ExecuteReader (CommandBehavior commandBehavior)
            {
            throw new NotImplementedException();
            }

        public object ExecuteScalar ()
            {
            throw new NotImplementedException();
            }

        public void Prepare ()
            {
            throw new NotImplementedException();
            }
        }
    }
