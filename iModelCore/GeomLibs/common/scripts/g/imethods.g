!
! gema patterns to pluck out instance method headers
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\* METHOD(<I>,<I>,none) \*\/=


\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
jmdl_import (dll = "@var{dllName}", name = $6, immediate_call)\n\
public native @FixTypes{$5} $3$7\n\
@OutputArglist{@FixTypes{($8)}}\;\
\n\/\/ END_NONEDITABLE_CODE\n

\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S\*\W<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
jmdl_import (dll = "@var{dllName}", name = $6, immediate_call)\n\
public native @FixTypes{$5} \*$3$7\n\
@OutputArglist{@FixTypes{($8)}}\;\
\n\/\/ END_NONEDITABLE_CODE\n


! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d
