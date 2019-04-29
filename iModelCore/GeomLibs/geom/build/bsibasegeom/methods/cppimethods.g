!
! gema patterns to pluck out instance method headers
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}


<ControlLine>BEGIN_CONSTRUCTOR <u>\/\/\#END_CONSTRUCTOR=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@OutputConstructorToHFile{$0}\
\/\/ END_NONEDITABLE_CODE\n


! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\* METHOD(<I>,<I>,none) \*\/=

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


\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public GEOMDLLIMPEXP <I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppIMethodDecl>\n\
@CommentToJava{$4}\n\
$5 $3$7\n\
(\n@RemoveInstanceArg{$8}\n) @ConstQualifierFromFirstArg{$8}\;\n\
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

RemoveInstanceArg:\W<I>CP <I>\W,<u>=$3
RemoveInstanceArg:\W<I>P <I>\W,<u>=$3

RemoveInstanceArg:\W<I>CP <I>\W=
RemoveInstanceArg:\W<I>P <I>\W=

RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\W<I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W=
RemoveInstanceArg:\W<I>\W\*\W<I>\W=


ConstQualifierFromFirstArg:\W\ICP <u>=\ const
ConstQualifierFromFirstArg:\Wconst <u>=\ const
ConstQualifierFromFirstArg:<u>=



! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d

!
ControlLine:\n\W\/\/\#=$0@terminate
ControlLine:=@fail

ReadToNextControlLine:<ControlLine>=@end
ReadToNextControlLine:?=?

OutputConstructorToHFile:\/\/\#BEGIN_CONSTRUCTOR <u>\/\/\#SIGNATURE <u>\/\/\#<u>=\
$1\
$2\;\n
