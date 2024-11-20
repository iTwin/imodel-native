set self_path=%~dp0
set parse_dir=%self_path%\..\ECDb\ECSql\Antlr4
antlr4 -Dlanguage=Cpp -visitor -listener %parse_dir%/Grammer/ECSqlLexer.g4 %parse_dir%/Grammer/ECSqlParser.g4 -o %parse_dir%