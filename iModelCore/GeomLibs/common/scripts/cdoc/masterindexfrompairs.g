! Input file: sorted triples of form
!   <keyword for index > <target description for index> <full link>
! Output file: HTML tags
@set{priorKeyword;}
\N<I> <I> <T>\n=@cmps{@var{priorKeyword};$1;@EmitKeyword{$1};;@EmitKeyword{$1}}\
        @set{priorKeyword;$1}

EmitKeyword:<I>=\
        <LI><a href=\"index.html\#$1\" target="indexFrame">$1</a><br></LI>\n

?=
\B=<UL>
\E=</UL>

