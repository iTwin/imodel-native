using Bentley.Exceptions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
{
    internal class PagedSQLQueryBuilder : SQLQueryBuilder
    {
        private int m_colNumber;
        private int m_lowerBound;
        private int m_upperBound;

        private List<string> m_aliasList;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="lowerBound">Lower bound (included) of the instance rows returned</param>
        /// <param name="upperBound">Upper bound (included) of the instance rows returned</param>
        public PagedSQLQueryBuilder(int lowerBound, int upperBound) : base()
        {
            m_colNumber = 0;

            //m_lowerBound = skip + 1;

            //m_upperBound = m_lowerBound + top - 1;

            m_lowerBound = lowerBound;
            m_upperBound = upperBound;

            m_aliasList = new List<string>();
        }

        override public string BuildQuery()
        {
            string innerRequestAlias = "Results";

            if (m_sqlOrderByClause.Count == 0)
            {
                throw new UserFriendlyException("Please include an OrderBy statement in order for the paging to work.");
            }

            string innerOrderByClause = "ORDER BY ";
            innerOrderByClause += String.Join(", ", m_sqlOrderByClause.ToArray());

            string innerSelectString = "SELECT ROW_NUMBER() OVER ( " + innerOrderByClause + " ) AS RowNum";
            foreach (string selectedColumn in m_sqlSelectClause)
            {
                string alias = GetNewColAlias();
                m_aliasList.Add(alias);
                innerSelectString += ", " + selectedColumn + " AS " + alias;
            }
            innerSelectString += " ";

            string innerFromStr = "FROM " + m_sqlFromClause.Name + " " + m_sqlFromClause.Alias + " ";

            //string innerLeftJoinClause = String.Join(" ", m_sqlLeftJoinClause.ToArray()) + " ";
            string innerLeftJoinClause = "";
            foreach (var table in m_sqlLeftJoinClause)
            {
                innerLeftJoinClause += "LEFT JOIN " + AddBrackets(table.Name) + " " + table.Alias
                    + " ON " + AddBrackets(table.FirstTable.Alias) + "." + AddBrackets(table.FirstTableKey) + " = " + AddBrackets(table.Alias) + "." + AddBrackets(table.TableKey) + " ";
            }


            string innerWhereClause = "";
            if (!String.IsNullOrWhiteSpace(m_sqlWhereClause))
            {
                innerWhereClause = "WHERE " + m_sqlWhereClause + " ";
            }

            string completeInnerString = innerSelectString + innerFromStr + innerLeftJoinClause + innerWhereClause;

            string outerSelect = "";
            foreach(string alias in m_aliasList)
            {
                if(outerSelect == "")
                {
                    outerSelect = "SELECT ";
                }
                else
                {
                    outerSelect += ", ";
                }

                outerSelect += innerRequestAlias + "." + alias;
            }
            outerSelect += " ";

            string outerFrom = "FROM ( " + completeInnerString + " ) AS " + innerRequestAlias + " ";

            string outerWhere = "WHERE RowNum >= " + m_lowerBound + " AND RowNum <= " + m_upperBound + " ORDER BY RowNum";

            return outerSelect + outerFrom + outerWhere;

            //string completeRequest = "";

            //int upperBound = m_itemsByPage * m_pageNumber;
            //int lowerBound = upperBound - m_itemsByPage + 1;

            //foreach (string alias in m_aliasList)
            //{
            //    if (completeRequest == "")
            //    {
            //        completeRequest = "SELECT ";
            //    }
            //    else
            //    {
            //        completeRequest += ", ";
            //    }

            //    completeRequest += innerRequestAlias + "." + alias + " ";
            //}
            //completeRequest += "FROM ( " + innerRequest + " ) AS " + innerRequestAlias
            //                + String.Format(" WHERE RowNum >= {0} AND RowNum <= {1}", lowerBound, upperBound)
            //                + " ORDER BY RowNum";

            //return completeRequest;
        }

        //Generates a new unique nolumn
        private string GetNewColAlias()
        {
            return String.Format("col{0}", m_colNumber++);
        }
    }
}
