!! header and footer
\B=@read{copyright\.txt}@read{\.\\typescript\\topOfFile\.txt}
\E=@read{\.\\typescript\\endOfFile\.txt}

ApplyTagToIdentifiers:<I>\W\==\ \ \ \ tag$1\ \=
ApplyTagToIdentifiers:?=?

!! make enum names distinct from classes
enum VariantGeometryUnion\{<u>\}\;=enum VariantGeometryUnion\{@ApplyTagToIdentifiers{$1}\}
VariantGeometryUnion\.<I>=VariantGeometryUnion\.tag$1

!! add missing semicolons after name and inputName methods that end in "null"
\Lname(<T>null\N=name($1null\;
\LinputName(<T>null\N=inputName($1null\;

\}\;=\}
\:<I>=\: $1

!! The generated code uses "var" instead of more specific "const" and "let".
!! fortunately there are only 2 names in play.
!!    make "offset" a const, and "i" in a loop a let:
\Ivar offset\I=const offset
(var i=(let i

?=?
