using IndexECPlugin.Source.Helpers;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class TableAliasCreatorTests
        {
        [Test]
        public void GetNewTableAliasTest ()
            {
            TableAliasCreator tableAliasCreator = new TableAliasCreator();
            for (int i = 0; i < 10; i++)
                {
                Assert.That(tableAliasCreator.GetNewTableAlias(), Is.EqualTo("tab" + i.ToString()));
                }
            }
        }
    }
