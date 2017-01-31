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

! THREE-ARG METHOD(class,cmethod,imethod)
!   is deprecated.  Use TWO-ARG INSTANCE_METHOD and CLASS_METHOD
!
\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppIMethodDecl>\n\
@CommentToJava{$4}\n\
$5 $3$7\n\
(\n@RemoveInstanceArg{$8}\n) @ConstQualifierFromFirstArg{$8}\;\n\
\/\/\</cppIMethodDecl>\n\
\/\/ END_NONEDITABLE_CODE\n

\/\*\SINSTANCE_METHOD(<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppIMethodDecl>\n\
@CommentToJava{$3}\n\
$4 $2$6\n\
(\n@RemoveInstanceArg{$7}\n) @ConstQualifierFromFirstArg{$7}\;\n\
\/\/\</cppIMethodDecl>\n\
\/\/ END_NONEDITABLE_CODE\n


\/\*\SCLASS_METHOD(<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppCMethodDecl>\n\
@CommentToJava{$3}\n\
static $4 $2$6\n\
(\n$7\n)\;\n\
\/\/\</cppCMethodDecl>\n\
\/\/ END_NONEDITABLE_CODE\n

RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\W<I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W=
RemoveInstanceArg:\W<I>\W\*\W<I>\W=

ConstQualifierFromFirstArg:\Wconst <u>=\ const
ConstQualifierFromFirstArg:<u>=



! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d
