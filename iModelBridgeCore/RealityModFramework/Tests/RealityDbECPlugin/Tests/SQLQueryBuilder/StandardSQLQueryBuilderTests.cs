using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {

    [TestFixture]
    class StandardSQLQueryBuilderTests
        {
        private const string tableName = "SpaceMcTable";
        private const string tableAlias = "t";
        private const string nonPropertyDataColumnName = "npdcn";
        private const string otherColumnName = "SpatialColumn";
        private const string polygonWKT = "Insert WKT polygon here";
        private const int polygonSRID = 1234;
        private const string tableHint = "Super helpful hint";
        private const string joinedTableName = "JoinedTable";
        private const string joinedTableAlias = "j";
        private const string firstTableKey = "1";
        private const string joinedTableKey = "42";

        private StandardSQLQueryBuilder standardSQLQueryBuilder;
        private TableDescriptor tableDescriptor;
        private TableDescriptor joinedTableDescriptor;
        private DataReadingHelper dataReadingHelper;

        private MockRepository mocks;
        private IECProperty ECPropertyStub;

        [SetUp]
        public void SetUp ()
            {
            standardSQLQueryBuilder = new StandardSQLQueryBuilder();
            tableDescriptor = new TableDescriptor(tableName, tableAlias);
            joinedTableDescriptor = new TableDescriptor(joinedTableName, joinedTableAlias);
            joinedTableDescriptor.SetTableJoined(tableDescriptor, firstTableKey, joinedTableKey);

            mocks = new MockRepository();
            ECPropertyStub = mocks.Stub<IECProperty>();
            }

        [Test]
        public void BuildQueryMinimalistTest ()
            {
            standardSQLQueryBuilder.AddSelectClause(tableDescriptor, nonPropertyDataColumnName, ColumnCategory.nonPropertyData, ECPropertyStub);
            standardSQLQueryBuilder.AddSelectClause(tableDescriptor, otherColumnName, ColumnCategory.streamData, ECPropertyStub);
            standardSQLQueryBuilder.SpecifyFromClause(tableDescriptor);

            Assert.That(standardSQLQueryBuilder.BuildQuery(out dataReadingHelper), Is.EqualTo("SELECT TOP 1000 " + tableAlias + "." +
                nonPropertyDataColumnName + ", " + tableAlias + "." + otherColumnName + " FROM " + tableName + " " + tableAlias + "  ;"));

            ValidateDataReadingHelper();
            }

        [Test]
        public void BuildQueryTest ()
            {
            TableDescriptor dummy;

            standardSQLQueryBuilder.AddSelectClause(tableDescriptor, nonPropertyDataColumnName, ColumnCategory.nonPropertyData, ECPropertyStub);
            standardSQLQueryBuilder.AddSelectClause(tableDescriptor, otherColumnName, ColumnCategory.streamData, ECPropertyStub);
            standardSQLQueryBuilder.SpecifyFromClause(tableDescriptor);
            standardSQLQueryBuilder.AddLeftJoinClause(joinedTableDescriptor, out dummy);
            standardSQLQueryBuilder.AddSpatialIntersectsWhereClause(tableName, otherColumnName, polygonWKT, polygonSRID, tableHint);
            standardSQLQueryBuilder.AddOrderByClause(tableDescriptor, otherColumnName, true);
            standardSQLQueryBuilder.AddOrderByClause(tableDescriptor, nonPropertyDataColumnName, false);

            string expectedQuery = "SELECT TOP 1000 " + tableAlias + "." + nonPropertyDataColumnName + ", " + tableAlias + "." + otherColumnName +
                                   " FROM " + tableName + " " + tableAlias + "  WITH(INDEX(" + tableHint + ")) LEFT JOIN " + joinedTableName + " " +
                                   joinedTableAlias + " ON " + tableAlias + "." + firstTableKey + " = " + joinedTableAlias + "." + joinedTableKey +
                                   "  WHERE " + tableName + "." + otherColumnName + @".STIntersects(geometry::STGeomFromText('" + polygonWKT + @"'," +
                                   polygonSRID + @")) = 1 ORDER BY " + tableAlias + "." + otherColumnName + " ASC , " + tableAlias + "." +
                                   nonPropertyDataColumnName + " DESC ;";

            Assert.That(standardSQLQueryBuilder.BuildQuery(out dataReadingHelper), Is.EqualTo(expectedQuery));

            ValidateDataReadingHelper();
            }

        private void ValidateDataReadingHelper ()
            {
            Assert.That(dataReadingHelper.getNonPropertyDataColumn(nonPropertyDataColumnName), Is.EqualTo(0));
            Assert.That(dataReadingHelper.getStreamDataColumn(), Is.EqualTo(1));
            }
        }
    }
