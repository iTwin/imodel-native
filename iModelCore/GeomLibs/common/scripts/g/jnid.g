! gema patterns to create headers of methods implemented in native code to be
! called via JNI

@define{@read{@getenv{GEMADIR}g\\domains.g}}

\/\*\SJNIMETHOD(<T>)\S\*\/\S\
\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\* BEGIN_NONEDITABLE_CODE \*\/\N\
@CommentToJava{$5}\Npublic native @JavaTypes{$6} @Java_MethodName{($1)}$8\N\
@OutputInstanceArglist{@StripConst{@JavaTypes{($9)}}}\;\
\n\/\* END_NONEDITABLE_CODE \*\/\n


?=

! Argument of JNIMETHOD() may have two fields if overloaded
Java_MethodName:(<I>,*)=$1
Java_MethodName:(<I>)=$1
Java_MethodName:<T>=$0


! Instance arglist.
!   First arg must be a pointer to the instance; nuke it:
OutputInstanceArglist:( <I> \*<I>\W,=(
OutputInstanceArglist:( <I> \*<I>=(
!   Any pointer on the struct side comes in as an object:
OutputInstanceArglist:<I> \*\W<I>=\n@JavaTypes{$1}\t$2
OutputInstanceArglist:<I>\W<I>=\n$1\t$2
!  Copy along the commas and closing parentheses verbatim:
OutputInstanceArglist:,=,
OutputInstanceArglist:)=\n)
OutputInstanceArglist:?=


! (additional patterns in domains.g)
CommentToJava:\IDPoint3d\I=DPoint
CommentToJava:\IDMatrix3d\I=RotMatrix
CommentToJava:\IDTransform3d\I=Transform
CommentToJava:\Ip<K><i>=o$1$2
