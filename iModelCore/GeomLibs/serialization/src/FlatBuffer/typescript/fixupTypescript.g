!! make enum names distinct from classes
enum VariantGeometryUnion\{<u>\}\;=enum VariantGeometryUnion\{@ApplyTagToIdentifiers{$1}\}
!!  \L\N<I>\:=\n    public $1\: 
!!  \L\N<I>\W\(\W<I>\:=\n    public $1($2\: 
!!  \L\Nstatic <I>\W\(\W<I>\:=\n   public static $1($2\: 
!!  \L\N<I>\W\(\W<I>\?\:=\n    public $1($2\?\: 
!!  \L\N<I>()\:=\n    public $1()\: 
!!  \L\n  geometry\<T extends=\n    public geometry\<T extends
VariantGeometryUnion\.<I>=VariantGeometryUnion\.tag$1

\}\;=\}
\:<I>=\: $1
!! The generated code uses "var" instead of more specific "const" and "let".
!! fortunately there are only 2 names in play.
!!    make "offset" a const, and "i" in a loop a let:
\Ivar offset\I=const offset
(var i=(let i
?=?


ApplyTagToIdentifiers:<I>\W\==\ \ \ \ tag$1\ \=
ApplyTagToIdentifiers:?=?

