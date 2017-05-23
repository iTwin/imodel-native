using IndexECPlugin.Source.Helpers;
using NUnit.Framework;


namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class TableDescriptorTests
        {
        private TableDescriptor emptyTableDescriptor1;
        private TableDescriptor emptyTableDescriptor2;
        private TableDescriptor emptyTableDescriptor3;
        private TableDescriptor mainTableDescriptor;
        private TableDescriptor otherTableDescriptor;

        [SetUp]
        public void setUp ()
            {
            emptyTableDescriptor1 = new TableDescriptor("emptyTableDescriptor", "alias");
            emptyTableDescriptor2 = new TableDescriptor("emptyTableDescriptor", "different alias");
            emptyTableDescriptor3 = new TableDescriptor("another emptyTableDescriptor", "alias");

            mainTableDescriptor = new TableDescriptor("a table descriptor", "an alias");
            mainTableDescriptor.SetTableJoined(emptyTableDescriptor1, "a first table key", "a table key");

            otherTableDescriptor = new TableDescriptor("a table descriptor", "an alias");
            }

        [Test]
        public void IsEqualToEqualTest ()
            {
            //test for null first table
            Assert.That(emptyTableDescriptor1.IsEqualTo(emptyTableDescriptor2), Is.True);

            otherTableDescriptor.SetTableJoined(emptyTableDescriptor1, "a first table key", "a table key");
            //test for non-null first table
            Assert.That(mainTableDescriptor.IsEqualTo(otherTableDescriptor), Is.True);
            }

        [Test]
        public void IsEqualToNotEqualTest ()
            {
            // different names
            Assert.That(mainTableDescriptor.IsEqualTo(emptyTableDescriptor1), Is.False);

            // different table key
            otherTableDescriptor.SetTableJoined(emptyTableDescriptor1, "a different first table key", "a table key");
            Assert.That(mainTableDescriptor.IsEqualTo(otherTableDescriptor), Is.False);

            // different first table key
            otherTableDescriptor.SetTableJoined(emptyTableDescriptor1, "a first table key", "a different table key");
            Assert.That(mainTableDescriptor.IsEqualTo(otherTableDescriptor), Is.False);

            // one null first table and one non-null first table
            otherTableDescriptor.SetTableJoined(null, "a first table key", "a table key");
            Assert.That(mainTableDescriptor.IsEqualTo(otherTableDescriptor), Is.False);

            // two different non-null first tables
            otherTableDescriptor.SetTableJoined(emptyTableDescriptor3, "a first table key", "a table key");
            Assert.That(mainTableDescriptor.IsEqualTo(otherTableDescriptor), Is.False);

            }
        }
    }
