﻿using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    static internal class ECToSQLMap
        {
        static public string ECRelationalOperatorToSQL (RelationalOperator op)
            {
            switch ( op )
                {
                //case RelationalOperator.CONTAINS:

                //    break;
                //case RelationalOperator.CONTAINSWHOLEWORD:

                //    break;
                //case RelationalOperator.ENDSWITH:
                //    break;
                case RelationalOperator.EQ:
                    return "=";
                case RelationalOperator.GT:
                    return ">";
                case RelationalOperator.GTEQ:
                    return ">=";
                case RelationalOperator.IN:
                    return "IN";
                case RelationalOperator.LIKE:
                    return "LIKE";
                case RelationalOperator.LT:
                    return "<";
                case RelationalOperator.LTEQ:
                    return "<=";
                //case RelationalOperator.MATCHESREGEX:
                //    break;
                case RelationalOperator.NE:
                    return "<>";
                case RelationalOperator.NOTIN:
                    return "NOT IN";
                case RelationalOperator.NOTLIKE:
                    return "NOT LIKE";
                case RelationalOperator.ISNULL:
                    return "IS NULL";
                case RelationalOperator.ISNOTNULL:
                    return "IS NOT NULL";
                //case RelationalOperator.STARTSWITH:
                //    break;
                //case RelationalOperator.X:
                //    break;
                default:
                    throw new NotImplementedException(String.Format("The relational operator {0} is not implemented yet!", op));
                }
            }

        static internal DbType ECTypeToSQL (IECType ecType)
            {
            if ( ECTypeHelper.IsString(ecType) )
                {
                //instancePropertyValue.StringValue = reader.GetString(i);
                return DbType.String;
                }
            if ( ECTypeHelper.IsDouble(ecType) )
                {
                //instancePropertyValue.DoubleValue = reader.GetDouble(i);
                return DbType.Double;
                }
            if ( ECTypeHelper.IsBoolean(ecType) )
                {
                //instancePropertyValue.NativeValue = reader.GetBoolean(i);
                return DbType.Boolean;
                }
            if ( ECTypeHelper.IsInteger(ecType) )
                {
                //instancePropertyValue.IntValue = reader.GetInt32(i);
                return DbType.Int32;
                }
            if ( ECTypeHelper.IsLong(ecType) )
                {
                //instancePropertyValue.NativeValue = reader.GetInt64(i);
                return DbType.Int64;
                }
            if ( ECTypeHelper.IsDateTime(ecType) )
                {
                //instancePropertyValue.NativeValue = reader.GetDateTime(i);
                return DbType.DateTime;
                }
            throw new ProgrammerException(String.Format("The ECType {0} is not bound to any SQL type. Please modify the ECSchema", ecType.Name));
            }

        static internal void SQLReaderToECProperty (IECPropertyValue instancePropertyValue, DbDataReader reader, int i)
            {
            if ( ECTypeHelper.IsString(instancePropertyValue.Type) )
                {
                instancePropertyValue.StringValue = reader.GetString(i);
                return;
                }
            if ( ECTypeHelper.IsDouble(instancePropertyValue.Type) )
                {
                instancePropertyValue.DoubleValue = reader.GetDouble(i);
                return;
                }
            if ( ECTypeHelper.IsBoolean(instancePropertyValue.Type) )
                {
                instancePropertyValue.NativeValue = reader.GetBoolean(i);
                return;
                }
            if ( ECTypeHelper.IsInteger(instancePropertyValue.Type) )
                {
                instancePropertyValue.IntValue = reader.GetInt32(i);
                return;
                }
            if ( ECTypeHelper.IsLong(instancePropertyValue.Type) )
                {
                instancePropertyValue.NativeValue = reader.GetInt64(i);
                return;
                }
            if ( ECTypeHelper.IsDateTime(instancePropertyValue.Type) )
                {
                instancePropertyValue.NativeValue = reader.GetDateTime(i);
                return;
                }
            throw new ProgrammerException(String.Format("The ECType {0} is not bound to any SQL type. Please modify the ECSchema", instancePropertyValue.Type.Name));

            }
        }
    }
