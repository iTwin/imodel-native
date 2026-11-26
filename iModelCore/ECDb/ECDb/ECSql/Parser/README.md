# ECSQL parser

The ECSQL grammar is a flex lexer (`sqlflex.l`) and a bison parser (`sqlbison.y`), inherited from OpenOffice. `SqlFlex.cpp`, `SqlBison.cpp` and `SqlBison.h` are generated from them and committed.

## Changing the grammar

1. Add tokens to `sqlflex.l`:

   ```c
   KEYWORD {SQL_NEW_KEYWORD(TOKEN_NAME);}
   ```

2. Declare new tokens in `sqlbison.y`:

   ```c
   %token <pParseNode> TOKEN_NAME
   ```

   and add rules:

   ```c
   %type <pParseNode> parser_rule

   parser_rule:
       parser_logic
       ;
   ```

3. Every rule added or removed in `sqlbison.y` must be added or removed under the same name in two places:
   - the `Rule` enum of `OSQLParseNode` in `SqlNode.h`
   - `aRuleDescriptions[]` in the `OSQLParser` constructor in `SqlParse.cpp`

4. Regenerate the parser with `iModelCore/ECDb/Scripts/BuildECSqlParser.sh` (needs bison 3.8.2 or later and flex 2.6 or later; on macOS install both with Homebrew) or `BuildECSqlParser.bat` on Windows (downloads win_flex_bison).

5. Keep the generated files' machine-specific lines as committed: the include guard in `SqlBison.h` (`YY_SQLYY_SQLBISON_H_INCLUDED`) and the `#line 2` path at the top of `SqlFlex.cpp`.

6. Implement the ECSQL side in `ECSqlParser.cpp`, `ECSqlPreparer.cpp` and the related files in `iModelCore/ECDb/ECDb/ECSql/`, and add tests.

## Further reading

- [Lex and YACC primer](https://tldp.org/HOWTO/Lex-YACC-HOWTO.html)
