
// Generated from D:/bsw/git-native-master/src/imodel-native/iModelCore/ECDb/Scripts//../ECDb/ECSql/Antlr4/Grammer/ECSqlLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4/antlr4-runtime.h"




class  ECSqlLexer : public antlr4::Lexer {
public:
  enum {
    SCOL = 1, DOT = 2, OPEN_PAR = 3, CLOSE_PAR = 4, COMMA = 5, ASSIGN = 6,
    STAR = 7, PLUS = 8, MINUS = 9, TILDE = 10, PIPE2 = 11, DIV = 12, MOD = 13,
    LT2 = 14, GT2 = 15, AMP = 16, PIPE = 17, LT = 18, LT_EQ = 19, GT = 20,
    GT_EQ = 21, EQ = 22, NOT_EQ1 = 23, NOT_EQ2 = 24, DOLLAR = 25, ARROW = 26,
    ABORT_ = 27, ACTION_ = 28, ADD_ = 29, AFTER_ = 30, ALL_ = 31, ALTER_ = 32,
    ANALYZE_ = 33, AND_ = 34, AS_ = 35, ASC_ = 36, ATTACH_ = 37, AUTOINCREMENT_ = 38,
    BEFORE_ = 39, BEGIN_ = 40, BETWEEN_ = 41, BY_ = 42, CASCADE_ = 43, CASE_ = 44,
    CAST_ = 45, CHECK_ = 46, COLLATE_ = 47, COLUMN_ = 48, COMMIT_ = 49,
    CONFLICT_ = 50, CONSTRAINT_ = 51, CREATE_ = 52, CROSS_ = 53, CURRENT_DATE_ = 54,
    CURRENT_TIME_ = 55, CURRENT_TIMESTAMP_ = 56, DATABASE_ = 57, DEFAULT_ = 58,
    DEFERRABLE_ = 59, DEFERRED_ = 60, DELETE_ = 61, DESC_ = 62, DETACH_ = 63,
    DISTINCT_ = 64, DROP_ = 65, EACH_ = 66, ELSE_ = 67, END_ = 68, ESCAPE_ = 69,
    EXCEPT_ = 70, EXCLUSIVE_ = 71, EXISTS_ = 72, EXPLAIN_ = 73, FAIL_ = 74,
    FOR_ = 75, FOREIGN_ = 76, FROM_ = 77, FULL_ = 78, GLOB_ = 79, GROUP_ = 80,
    HAVING_ = 81, IF_ = 82, IGNORE_ = 83, IMMEDIATE_ = 84, IN_ = 85, INDEX_ = 86,
    INDEXED_ = 87, INITIALLY_ = 88, INNER_ = 89, INSERT_ = 90, INSTEAD_ = 91,
    INTERSECT_ = 92, INTO_ = 93, IS_ = 94, ISNULL_ = 95, JOIN_ = 96, KEY_ = 97,
    LEFT_ = 98, LIKE_ = 99, LIMIT_ = 100, MATCH_ = 101, NATURAL_ = 102,
    NO_ = 103, NOT_ = 104, NOTNULL_ = 105, NULL_ = 106, OF_ = 107, OFFSET_ = 108,
    ON_ = 109, OR_ = 110, ORDER_ = 111, OUTER_ = 112, PLAN_ = 113, PRAGMA_ = 114,
    PRIMARY_ = 115, QUERY_ = 116, RAISE_ = 117, RECURSIVE_ = 118, REFERENCES_ = 119,
    REGEXP_ = 120, REINDEX_ = 121, RELEASE_ = 122, RENAME_ = 123, REPLACE_ = 124,
    RESTRICT_ = 125, RETURNING_ = 126, RIGHT_ = 127, ROLLBACK_ = 128, ROW_ = 129,
    ROWS_ = 130, SAVEPOINT_ = 131, SELECT_ = 132, SET_ = 133, TABLE_ = 134,
    TEMP_ = 135, TEMPORARY_ = 136, THEN_ = 137, TO_ = 138, TRANSACTION_ = 139,
    TRIGGER_ = 140, UNION_ = 141, UNIQUE_ = 142, UPDATE_ = 143, USING_ = 144,
    VACUUM_ = 145, VALUES_ = 146, VIEW_ = 147, VIRTUAL_ = 148, WHEN_ = 149,
    WHERE_ = 150, WITH_ = 151, WITHOUT_ = 152, FIRST_VALUE_ = 153, OVER_ = 154,
    PARTITION_ = 155, RANGE_ = 156, PRECEDING_ = 157, UNBOUNDED_ = 158,
    CURRENT_ = 159, FOLLOWING_ = 160, CUME_DIST_ = 161, DENSE_RANK_ = 162,
    LAG_ = 163, LAST_VALUE_ = 164, LEAD_ = 165, NTH_VALUE_ = 166, NTILE_ = 167,
    PERCENT_RANK_ = 168, RANK_ = 169, ROW_NUMBER_ = 170, GENERATED_ = 171,
    ALWAYS_ = 172, STORED_ = 173, TRUE_ = 174, FALSE_ = 175, WINDOW_ = 176,
    NULLS_ = 177, FIRST_ = 178, LAST_ = 179, FILTER_ = 180, GROUPS_ = 181,
    EXCLUDE_ = 182, TIES_ = 183, OTHERS_ = 184, DO_ = 185, NOTHING_ = 186,
    IDENTIFIER = 187, NUMERIC_LITERAL = 188, BIND_PARAMETER = 189, STRING_LITERAL = 190,
    BLOB_LITERAL = 191, SINGLE_LINE_COMMENT = 192, MULTILINE_COMMENT = 193,
    SPACES = 194, UNEXPECTED_CHAR = 195
  };

  explicit ECSqlLexer(antlr4::CharStream *input);

  ~ECSqlLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

