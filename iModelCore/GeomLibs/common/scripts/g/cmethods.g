!
! gema patterns to pluck out class method headers
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\* METHOD(<I>,none,<I>) \*\/=

\#ifdef CMETHODS<T>\#endif=$1
\#if defined CMETHODS<T>\#endif=$1

\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=@FixTypes{\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
jmdl_import (dll = "@var{dllName}", name = $6, immediate_call)\n\
public static native $5 $2$7\n\
@OutputStaticArglist{($8)}\;\
\n\/\/ END_NONEDITABLE_CODE\n\
}

\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S\*\W<I><OPTIONALCOMMENT>\W(<arglist>)=@FixTypes{\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
jmdl_import (dll = "@var{dllName}", name = $6, immediate_call)\n\
public static native $5 \*$2$7\n\
@OutputStaticArglist{($8)}\;\
\n\/\/ END_NONEDITABLE_CODE\n\
}


! PATTERN: Everything else in the file
! ACTION:  swallow
?=


! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d
