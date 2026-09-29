!! header and footer
\B=@read{\.\\fixup\\copyright\.txt}@read{\.\\fixup\\topOfFileTypescript\.txt}
\E=@read{\.\\fixup\\endOfFileTypescript\.txt}

ApplyTagToIdentifiers:<I>\W\==\ \ \ \ tag$1\ \=
ApplyTagToIdentifiers:?=?

ApplyTagToVariantGeometryUnion:VariantGeometryUnion\.<I>=VariantGeometryUnion\.tag$1
ApplyTagToVariantGeometryUnion:?=?

!! make enum names distinct from classes
enum VariantGeometryUnion\{<u>\}\;=enum VariantGeometryUnion \{@ApplyTagToIdentifiers{$1}\}
VariantGeometryUnion\.<I>=VariantGeometryUnion\.tag$1

!! avoid changes to comment blocks. Just recognizing them prevents other actions on them.
\/\*\*<T>\*\/=$0

NameOrInputName:\Iname\I=name@end;\IinputName\I=inputName@end;=@fail

ColonSpaceBar:<I>|<I>=$1 | $2
ColonSpaceBar:<I>|<I>|<I>=$1 | $2 | $3
ColonSpaceBar:\:<s><I>=\: $2
ColonSpaceBar:\:<s><I>|<I>=\: $2 | $3
ColonSpaceBar:\:<s><I>|<I>|<I>=\: $2 | $3 | $4
ColonSpaceBar:?=?

!! add missing semicolons to special cases
\L<NameOrInputName>(<t>)<T>\{\N=$1(@ColonSpaceBar{$2})@ColonSpaceBar{$3}\{
\L<NameOrInputName>(<t>)<T>/[;]*/\N=$1(@ColonSpaceBar{$2})@ColonSpaceBar{$3}\;

!! remove unnecessary semicolons
\}\;=\}

!! add missing spaces
\:<s><G>=\: @ColonSpaceBar{@ApplyTagToVariantGeometryUnion{$2}}
<A1>\==$1 \=
<A1>\{=$1 \{

!! The generated code uses "var" instead of more specific "const" and "let".
!! fortunately there are only 2 names in play.
!!    make "offset" a const, and "i" in a loop a let:
\Ivar offset\I=const offset
(var i=(let i

?=?
