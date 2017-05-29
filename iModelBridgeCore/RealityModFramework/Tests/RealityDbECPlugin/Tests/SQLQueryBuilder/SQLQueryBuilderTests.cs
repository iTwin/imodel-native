using System;
using System.Collections.Generic;
using System.Data;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class SQLQueryBuilderTests
        {
        private TestableSQLQueryBuilder sqlQueryBuilder;
        private TableDescriptor tableDescriptor;
        private TableDescriptor dummyTableDescriptor;

        private const string tableName = "table name";
        private const string columnName = "column name";

        private const string polygon = "awesome polygon";
        private const int polygonSRID = 5;
        private const string tableHint = "table hint";

        private const string minX = "minX";
        private const string maxX = "maxX";
        private const string minY = "minY";
        private const string maxY = "maxY";

        private readonly BBox box = new BBox
        {
            maxX = 5, minX = 0, maxY = 9, minY = 3
        };

        [SetUp]
        public void SetUp ()
            {
            sqlQueryBuilder = new TestableSQLQueryBuilder();
            tableDescriptor = new TableDescriptor("table name", "table alias");
            dummyTableDescriptor = new TableDescriptor("", "");
            }

        [Test]
        public void AddSelectClauseTest ()
            {
            const string nonPropertyDataColumnName = "npdcn";
            const string otherColumnName = "other column";
            MockRepository mocks = new MockRepository();
            IECProperty ECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty ECPropertyStub2 = mocks.Stub<IECProperty>();

            sqlQueryBuilder.AddSelectClause(tableDescriptor, columnName, ColumnCategory.spatialInstanceData, ECPropertyStub);
            sqlQueryBuilder.AddSelectClause(tableDescriptor, nonPropertyDataColumnName, ColumnCategory.nonPropertyData, ECPropertyStub);
            sqlQueryBuilder.AddSelectClause(tableDescriptor, otherColumnName, ColumnCategory.streamData, ECPropertyStub2);

            Assert.That(sqlQueryBuilder.SqlSelectClause.Contains(tableDescriptor.Alias + "." + columnName + ".STAsText()"));
            Assert.That(sqlQueryBuilder.SqlSelectClause.Contains(tableDescriptor.Alias + "." + columnName + ".STSrid"));
            Assert.That(sqlQueryBuilder.DataReadingHelper.getInstanceDataColumn(ECPropertyStub), Is.EqualTo(0));

            Assert.That(sqlQueryBuilder.SqlSelectClause.Contains(tableDescriptor.Alias + "." + nonPropertyDataColumnName));
            Assert.That(sqlQueryBuilder.DataReadingHelper.getNonPropertyDataColumn(nonPropertyDataColumnName), Is.EqualTo(2));

            Assert.That(sqlQueryBuilder.SqlSelectClause.Contains(tableDescriptor.Alias + "." + otherColumnName));
            Assert.That(sqlQueryBuilder.DataReadingHelper.getStreamDataColumn(), Is.EqualTo(3));
            }

        [Test]
        public void SpecifyFromClauseTest ()
            {
            sqlQueryBuilder.SpecifyFromClause(tableDescriptor);

            Assert.That(sqlQueryBuilder.SqlFromClause.IsEqualTo(tableDescriptor));
            }

        [Test]
        public void SpecifyFromClauseExceptionTest ()
            {
            tableDescriptor.SetTableJoined(dummyTableDescriptor, null, null);
            Assert.That(() => sqlQueryBuilder.SpecifyFromClause(tableDescriptor), Throws.TypeOf<ProgrammerException>());

            tableDescriptor.SetTableJoined(null, "FirstTableKey", null);
            Assert.That(() => sqlQueryBuilder.SpecifyFromClause(tableDescriptor), Throws.TypeOf<ProgrammerException>());

            tableDescriptor.SetTableJoined(null, null, "TableKey");
            Assert.That(() => sqlQueryBuilder.SpecifyFromClause(tableDescriptor), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddLeftJoinClauseTest ()
            {
            TableDescriptor similarTable;
            tableDescriptor.SetTableJoined(dummyTableDescriptor, "FirstTableKey", "TableKey");

            Assert.That(sqlQueryBuilder.AddLeftJoinClause(tableDescriptor, out similarTable), Is.True);
            Assert.That(sqlQueryBuilder.SqlLeftJoinClause.Contains(tableDescriptor));
            Assert.That(similarTable == null);

            Assert.That(sqlQueryBuilder.AddLeftJoinClause(tableDescriptor, out similarTable), Is.False);
            Assert.That(similarTable == tableDescriptor);
            }

        [Test]
        public void AddLeftJoinClauseExceptionTest ()
            {
            TableDescriptor similarTable;

            tableDescriptor.SetTableJoined(dummyTableDescriptor, "FirstTableKey", null);
            Assert.That(() => sqlQueryBuilder.AddLeftJoinClause(tableDescriptor, out similarTable), Throws.TypeOf<ProgrammerException>());

            tableDescriptor.SetTableJoined(dummyTableDescriptor, null, "TableKey");
            Assert.That(() => sqlQueryBuilder.AddLeftJoinClause(tableDescriptor, out similarTable), Throws.TypeOf<ProgrammerException>());

            tableDescriptor.SetTableJoined(null, "FirstTableKey", "TableKey");
            Assert.That(() => sqlQueryBuilder.AddLeftJoinClause(tableDescriptor, out similarTable), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddOperatorToWhereClauseTest ()
            {
            sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo("AND "));

            sqlQueryBuilder.AddOperatorToWhereClause(LogicalOperator.OR);
            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo("AND OR "));
            }

        [Test]
        public void StartOfInnerWhereClauseTest ()
            {
            sqlQueryBuilder.StartOfInnerWhereClause();
            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(" ( "));
            }

        [Test]
        public void EndOfInnerWhereClauseTest ()
            {
            sqlQueryBuilder.EndOfInnerWhereClause();
            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(" ) "));
            }

        [Test]
        public void AddWhereClauseNullTableNameIsNullOperatorTest ()
            {
            sqlQueryBuilder.AddWhereClause(null, columnName, RelationalOperator.ISNULL, "", DbType.Date);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(columnName + " IS NULL "));
            }

        [Test]
        public void AddWhereClauseEmptyTableNameIsNotNullOperatorTest ()
            {
            sqlQueryBuilder.AddWhereClause("", columnName, RelationalOperator.ISNOTNULL, "", DbType.Object);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(columnName + " IS NOT NULL "));
            }

        [Test]
        public void AddWhereClauseInOperatorTest ()
            {
            sqlQueryBuilder.AddWhereClause(tableName, columnName, RelationalOperator.IN, "MicroStation,ContextCapture,ConceptStation", DbType.String);
            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(tableName + "." + columnName + " IN (@param0,@param1,@param2) "));
            Assert.That(sqlQueryBuilder.paramNameValueMap["@param0"].Equals(new Tuple<object, DbType>("MicroStation", DbType.String)));
            Assert.That(sqlQueryBuilder.paramNameValueMap["@param1"].Equals(new Tuple<object, DbType>("ContextCapture", DbType.String)));
            Assert.That(sqlQueryBuilder.paramNameValueMap["@param2"].Equals(new Tuple<object, DbType>("ConceptStation", DbType.String)));
            }

        [Test]
        public void AddWhereClauseLikeOperatorTest ()
            {
            const string rightSideString = "%.3mx";

            sqlQueryBuilder.AddWhereClause(tableName, columnName, RelationalOperator.LIKE, rightSideString, DbType.String);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(tableName + "." + columnName + " LIKE@param0 "));
            Assert.That(sqlQueryBuilder.paramNameValueMap["@param0"].Equals(new Tuple<object, DbType>(rightSideString, DbType.String)));
            }

        [Test]
        public void AddOrderByClauseTest ()
            {
            sqlQueryBuilder.AddOrderByClause(tableDescriptor, columnName, true);
            sqlQueryBuilder.AddOrderByClause(tableDescriptor, columnName, false);

            Assert.That(sqlQueryBuilder.SqlOrderByClause.Contains(tableDescriptor.Alias + "." + columnName + " ASC "));
            Assert.That(sqlQueryBuilder.SqlOrderByClause.Contains(tableDescriptor.Alias + "." + columnName + " DESC "));
            }

        [Test]
        public void OrderByListIsEmptyTest ()
            {
            bool orderClauseIsInitiallyEmpty = sqlQueryBuilder.OrderByListIsEmpty();

            sqlQueryBuilder.AddOrderByClause(tableDescriptor, "column name", true);

            Assert.That(orderClauseIsInitiallyEmpty, Is.True);
            Assert.That(sqlQueryBuilder.OrderByListIsEmpty(), Is.False);
            }

        [Test]
        public void AddSpatialIntersectsWhereClauseTest ()
            {
            sqlQueryBuilder.AddSpatialIntersectsWhereClause(tableName, columnName, polygon, polygonSRID, tableHint);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(tableName + "." + columnName + @".STIntersects(geometry::STGeomFromText('" +
                                                                   polygon + @"'," + polygonSRID + @")) = 1"));
            Assert.That(sqlQueryBuilder.TableHint, Is.EqualTo(tableHint));
            }

        [Test]
        public void AddSpatialIntersectsWhereClauseNullTableNameTest ()
            {
            sqlQueryBuilder.AddSpatialIntersectsWhereClause(null, columnName, polygon, polygonSRID, tableHint);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(columnName + @".STIntersects(geometry::STGeomFromText('" + polygon + @"'," +
                                                                   polygonSRID + @")) = 1"));
            Assert.That(sqlQueryBuilder.TableHint, Is.EqualTo(tableHint));
            }

        [Test]
        public void AddSpatialIntersectsWhereClauseEmptyTableNameTest ()
            {
            sqlQueryBuilder.AddSpatialIntersectsWhereClause("", columnName, polygon, polygonSRID, tableHint);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(columnName + @".STIntersects(geometry::STGeomFromText('" + polygon + @"'," +
                                                                   polygonSRID + @")) = 1"));
            Assert.That(sqlQueryBuilder.TableHint, Is.EqualTo(tableHint));
            }

        [Test]
        public void AddSpatialIntersectsWhereClauseExceptionTest ()
            {
            sqlQueryBuilder.AddSpatialIntersectsWhereClause("table", "column", "", 0, "first table hint");
            Assert.That(() => sqlQueryBuilder.AddSpatialIntersectsWhereClause("table", "column", "", 0, "second table hint"),
                Throws.TypeOf<NotImplementedException>());
            }

        [Test]
        public void AddBBoxIntersectsWhereClauseTest ()
            {
            const string tn = tableName + ".";

            string expectedString = "(" + tn + minX + " < " + box.maxX + " AND " + tn + maxX + " > " + box.minX + " AND " + tn + minY + " < " +
                                    box.maxY + " AND " + tn + maxY + " > " + box.minY + ")";

            sqlQueryBuilder.AddBBoxIntersectsWhereClause(tableName, minX, maxX, minY, maxY, box);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(expectedString));
            }

        [Test]
        public void AddBBoxIntersectsWhereClauseNullTableNameTest ()
            {
            string expectedString = "(" + minX + " < " + box.maxX + " AND " + maxX + " > " + box.minX + " AND " + minY + " < " + box.maxY + " AND " +
                                    maxY + " > " + box.minY + ")";

            sqlQueryBuilder.AddBBoxIntersectsWhereClause(null, minX, maxX, minY, maxY, box);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(expectedString));
            }

        [Test]
        public void AddBBoxIntersectsWhereClauseEmptyTableNameTest ()
            {
            string expectedString = "(" + minX + " < " + box.maxX + " AND " + maxX + " > " + box.minX + " AND " + minY + " < " + box.maxY + " AND " +
                                    maxY + " > " + box.minY + ")";

            sqlQueryBuilder.AddBBoxIntersectsWhereClause("", minX, maxX, minY, maxY, box);

            Assert.That(sqlQueryBuilder.SqlWhereClause, Is.EqualTo(expectedString));
            }

        [Test]
        public void BuildCountQueryTest ()
            {
            TableDescriptor otherTable = new TableDescriptor("other table name", "other table alias");
            TableDescriptor similarTable;
            otherTable.SetTableJoined(tableDescriptor, "first table key", "table key");
            sqlQueryBuilder.SpecifyFromClause(tableDescriptor);
            sqlQueryBuilder.AddLeftJoinClause(otherTable, out similarTable);
            sqlQueryBuilder.AddWhereClause("", columnName, RelationalOperator.ISNULL, "", DbType.Date);

            string expectedQuery = "SELECT COUNT(*) FROM " + tableDescriptor.Name + " " + tableDescriptor.Alias + " LEFT JOIN " + otherTable.Name +
                                   " " + otherTable.Alias + " ON " + otherTable.FirstTable.Alias + "." + otherTable.FirstTableKey + " = " +
                                   otherTable.Alias + "." + otherTable.TableKey + "  WHERE " + columnName + " IS NULL " + " ";

            string query = sqlQueryBuilder.BuildCountQuery();

            Assert.That(query, Is.EqualTo(expectedQuery));
            }

        private class TestableSQLQueryBuilder : SQLQueryBuilder
            {
            public List<string> SqlSelectClause
                {
                get
                    {
                    return m_sqlSelectClause;
                    }
                }

            public TableDescriptor SqlFromClause
                {
                get
                    {
                    return m_sqlFromClause;
                    }
                }

            public List<TableDescriptor> SqlLeftJoinClause
                {
                get
                    {
                    return m_sqlLeftJoinClause;
                    }
                }

            public string TableHint
                {
                get
                    {
                    return m_tableHint;
                    }
                }

            public string SqlWhereClause
                {
                get
                    {
                    return m_sqlWhereClause;
                    }
                }

            public List<string> SqlOrderByClause
                {
                get
                    {
                    return m_sqlOrderByClause;
                    }
                }

            public DataReadingHelper DataReadingHelper
                {
                get
                    {
                    return m_dataReadingHelper;
                    }
                }

            public override string BuildQuery (out DataReadingHelper dataReadingHelper)
                {
                throw new NotImplementedException();
                }
            }
        }
    }
