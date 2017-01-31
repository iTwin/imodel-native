MAP define*ENDMAP=@OandJ{\n\#define $1\n}

MAP jmdl<I>\=none.<I> ENDMAP=
MAP jmdl<I>\=<I>.none ENDMAP=

MAP jmdl<I>\=<I>.<I> ENDMAP=\
@OandJ{\#define jmdl$1 $2.$3\n}

MAP <I>\=<I>.<I> ENDMAP=\
\n\n\
\#define $1 $2.$3\n

?=


OandJ:*=$0@JtoO{$0}

JtoO:jmdl=omdl
JtoO:?=?