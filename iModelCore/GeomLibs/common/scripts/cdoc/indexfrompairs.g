! Input file: sorted triples of form
!   <keyword for index > <target description for index> <full link>
! Output file: HTML tags
@set{priorKeyword;}
\N<I> <I> <T>\n=@cmps{@var{priorKeyword};$1;@StartKeyword{$1};;@StartKeyword{$1}}\
        @KeywordEntry{$2 $3}\
        @set{priorKeyword;$1}
StartKeyword:<I>=\
        @cmps{@var{priorKeyword};;@EndKeyword{};;@EndKeyword{}}\
        <a name=\"$1\">$1</a><br>\n\
        <UL>\n
KeywordEntry:<I> <T>=<LI><a href=\"@StripPath{$2}\#$1\" target=bodyFrame>$1</a>\n

! The output tree is at root letter on a drive, and this documentation
! is being built in a directory with absolute path ending in cdoc.  Remove absolute path prefix so the index
! is just relative.
StripPath:<I>\:\\<I>\\<I>\\<I>\\cdoc\\=
StripPath:<I>\:\\<I>\\<I>\\cdoc\\=
StripPath:?=?

EndKeyword:=</UL>
?=
\E=@EndKeyword{}

