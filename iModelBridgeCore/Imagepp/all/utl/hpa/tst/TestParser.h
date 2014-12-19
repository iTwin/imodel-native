//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/tst/TestParser.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

// Les normes sont volontairement non respectées.  Expérimental.

#include <HPAParser.h>

class TestParser : public HPAParser
    {
public:

    TestParser();
    virtual ~TestParser();

protected:

private:

    // Disabled methods

    TestParser(const TestParser&);
    TestParser& operator=(const TestParser&);

public:

    // Rules

    HPARule Program;
    HPARule StatementList;
    HPARule Statement;
    HPARule BooleanExpr;
    HPARule Expression;

    // Tokens

    HPAToken Identifier_tk;
    HPAToken Number_tk;
    HPAToken String_tk;
    HPAToken IF_tk;
    HPAToken THEN_tk;
    HPAToken ELSE_tk;
    HPAToken WHILE_tk;
    HPAToken DO_tk;
    HPAToken BEGIN_tk;
    HPAToken END_tk;
    HPAToken PRINT_tk;
    HPAToken EQ_tk;       // =
    HPAToken NOT_tk;      // !
    HPAToken GT_tk;       // >
    HPAToken LT_tk;       // <
    HPAToken PLUS_tk;     // +
    HPAToken MINUS_tk;    // -
    HPAToken MUL_tk;      // *
    HPAToken DIV_tk;      // /
    HPAToken LP_tk;       // (
    HPAToken RP_tk;       // )

    };

