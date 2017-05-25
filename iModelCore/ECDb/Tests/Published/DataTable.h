/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DataTable.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestFrameworkHelper.h"
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct DataTable final : NonCopyableClass
    {
    struct Value final
        {
        enum class Type : int
            {
            Integer = 1,
            Float = 2,
            Text = 3,
            Blob = 4,
            Null = 5,
            };

        private:
            Type m_type;
            size_t m_size;
            union
                {
                int64_t m_i64;
                double m_flt;
                void* m_buff;
                };

            void Copy(const void* blob, size_t n, Type type);
            void CopyObject(Value const& rhs);
            void MoveObject(Value & rhs);

        public:
            Value() :m_i64(0), m_size(0), m_type(Type::Null) {}
            Value(Value const& rhs);
            Value(Value && rhs);
            Value& operator =(Value const& rhs);
            Value& operator =(Value && rhs);
            bool Equals(Value const& rhs) const;
            bool EqualsI(Value const& rhs) const;
            bool operator == (Value const& rhs) const;
            bool operator != (Value const& rhs) const;
            bool IsNull() const { return m_type == Type::Null; }
            bool IsBlob() const { return m_type == Type::Blob; }
            bool IsText() const { return m_type == Type::Text; }
            bool IsInteger() const { return m_type == Type::Integer; }
            bool IsFloat() const { return m_type == Type::Float; }
            void SetNull();
            void SetValue(int64_t i);
            void SetValue(double i);
            void SetValue(Utf8CP str);
            void SetValue(const void* blob, size_t n);
            int64_t GetInteger() const { BeAssert(IsInteger()); return m_i64; }
            double GetFloat() const { BeAssert(IsFloat()); return m_flt; }
            void* GetBlob() const { BeAssert(IsBlob()); return m_buff; }
            Utf8CP GetText() const { BeAssert(IsText()); return (Utf8CP) m_buff; }
            size_t GetSize() const { return m_size; }
            Type GetType() const { return m_type; }
            Utf8String ToString() const;
        };

    struct Column final : NonCopyableClass
        {
        friend struct DataTable;
        private:
            Value::Type m_type;
            Utf8String m_name;
            DataTable const& m_table;
            size_t m_index;
        protected:
            Column(DataTable const& table, size_t index, Utf8CP name)
                :m_table(table), m_name(name), m_type(Value::Type::Blob), m_index(index)
                {}
            Column(DataTable const& table, size_t index, Utf8CP name, Value::Type type)
                :m_table(table), m_name(name), m_type(type), m_index(index)
                {}

        public:
            DataTable const& GetTable() const { return m_table; }
            Utf8CP GetName() const { return m_name.c_str(); }
            Value::Type GetType() const { return m_type; }
            size_t GetIndex() const { return m_index; }
        };

    struct Row final : NonCopyableClass
        {
        friend struct DataTable;
        private:
            size_t m_index;
            std::vector<Value> m_values;
            DataTable const & m_table;

        protected:
            Row(DataTable const& table, size_t index);
        public:
            Value& GetValueR(size_t columnIndex);
            Value const& GetValue(size_t columnIndex) const;
            Value& GetValueR(Column const& column);
            Value const& GetValue(Column const& column) const;
            void SetValue(Column const& column, Value&& value);
            void SetValue(Column const& column, Value const& value);
            Utf8String ToString() const;
        };

    private:
        std::vector<std::unique_ptr<Column>> m_columns;
        std::vector<std::unique_ptr<Row>> m_rows;
        bool m_lock;
        Utf8String m_name;

    public:
        DataTable() :m_lock(false) {}
        explicit DataTable(Utf8CP name) :m_lock(false), m_name(name) {}
        void LockDefinition() { m_lock = true; }
        void SetName(Utf8CP name) { m_name = name; }
        Utf8CP GetName() const { return m_name.c_str(); }
        bool IsDefinitionLocked() const { return m_lock; }
        size_t GetRowCount() const { return m_rows.size(); }
        size_t GetColumnCount() const { return m_columns.size(); }
        Column const& GetColumn(size_t index) const { return *m_columns[index]; }
        Row const& GetRow(size_t index) const { return *m_rows[index]; }
        Row & GetRowR(size_t index) const { return *m_rows[index]; }
        void ClearRows() { return m_rows.clear(); }
        void Reset() { ClearRows(); m_columns.clear(); m_lock = false; }
        Column const& AddColumn(Utf8CP name, Value::Type type);
        Column const& AddColumn(Utf8CP name);
        Row& NewRow();
    };

//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct CSVAdaptor
    {
    private:
        static Utf8Char PreviousChar(Utf8CP current, Utf8CP bof);
        static Utf8Char NextChar(Utf8CP current);
        static BentleyStatus ParseField(DataTable& table, Utf8CP begin, Utf8CP end, int line, int field);
        static BentleyStatus ParseLine(DataTable& table, Utf8CP begin, Utf8CP end, int line);
    public:
        static void Fill(Utf8StringR toCSVBuffer, DataTable const& fromDataTable);
        static BentleyStatus Fill(DataTable& toDataTable, Utf8CP fromCSVBuffer);
    };

//=======================================================================================
// @bsiclass                                      Affan.Khan                       05/17
//+===============+===============+===============+===============+===============+======
struct SqlAdaptor final
    {
    public:
        static void Fill(DataTable& table, Statement& stmt, Utf8CP tableName);
        static BentleyStatus Fill(DbCR db, DataTable const& table);
    };

END_ECDBUNITTESTS_NAMESPACE