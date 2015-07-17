using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
{
    internal class StandardSQLQueryBuilder : SQLQueryBuilder
    {
        public StandardSQLQueryBuilder() : base()
        {

        }

        override public string BuildQuery()
        {
            string completeSelectStr = "SELECT ";
            completeSelectStr += String.Join(", ", m_sqlSelectClause.ToArray());

            completeSelectStr += " ";


            string completeFromStr = "FROM " + m_sqlFromClause.Name + " " + m_sqlFromClause.Alias + " ";

            //string completeLeftJoinClause = String.Join(" ", m_sqlLeftJoinClause.ToArray());
            string completeLeftJoinClause = "";
            foreach (var table in m_sqlLeftJoinClause)
            {
                completeLeftJoinClause += "LEFT JOIN " + AddBrackets(table.Name) + " " + table.Alias 
                    + " ON " + AddBrackets(table.FirstTable.Alias) + "." + AddBrackets(table.FirstTableKey) + " = " + AddBrackets(table.Alias) + "." + AddBrackets(table.TableKey) + " ";
            }


            completeLeftJoinClause += " ";

            string completeWhereClause = "";
            if (!String.IsNullOrWhiteSpace(m_sqlWhereClause))
            {
                completeWhereClause = "WHERE " + m_sqlWhereClause + " ";
            }

            string completeOrderByClause = "";
            if (! (m_sqlOrderByClause.Count == 0))
            {
                completeOrderByClause = "ORDER BY ";
                completeOrderByClause += String.Join(", ", m_sqlOrderByClause.ToArray());
            }

            return completeSelectStr + completeFromStr + completeLeftJoinClause + completeWhereClause + completeOrderByClause + ";";
        }
    }
}
