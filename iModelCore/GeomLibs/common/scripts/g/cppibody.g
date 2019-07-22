!
! gema patterns to pluck out instance method headers
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}

\/\*\SMETHOD(<I>,<I>,none)=

ExtractArgNames:<I>\W,=$1,
ExtractArgNames:<I>\W\;=$1
ExtractArgNames:<I>\W)=$1
ExtractArgNames:?=

RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\W<I>\W\*\W<I>\W,<u>=$3
RemoveInstanceArg:\Wconst <I>\W\*\W<I>\W=
RemoveInstanceArg:\W<I>\W\*\W<I>\W=

AddTHISAsFirstArg:\W(\W)=(this)
AddTHISAsFirstArg:\W(\W<I><u>=(this, $1$2
AddTHISAsFirstArg:?=?

ConstQualifierFromFirstArg:\Wconst <u>=\ const
ConstQualifierFromFirstArg:<u>=


ReturnDirective:void=
ReturnDirective:<u>=return

ClassNameFromFirstArg:\Wconst\W<I><u>=$1
ClassNameFromFirstArg:\W<I><u>=$1

! THREE-ARG form is deprecated.  Use INSTANCE_METHOD or CLASS_METHOD
\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppIMethodImpl>\n\
@CommentToJava{$4}\n\
$5 @ClassNameFromFirstArg{$8}\:\:$3$7\n\
(\n@RemoveInstanceArg{$8}\n)@ConstQualifierFromFirstArg{$8}\n\
\ \ \ \ \{\n\
\ \ \ \ @ReturnDirective{$5} $6 @AddTHISAsFirstArg{(\
\t\t@ExtractArgNames{@RemoveInstanceArg{$8}\;})}\;\n\
\ \ \ \ \}\n\
\/\/\<\/cppIMethodImpl>\n\
\/\/ END_NONEDITABLE_CODE\n



\/\*\SINSTANCE_METHOD(<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppIMethodImpl>\n\
@CommentToJava{$3}\n\
$4 $1\:\:$2$6\n\
(\n@RemoveInstanceArg{$7}\n)@ConstQualifierFromFirstArg{$7}\n\
\ \ \ \ \{\n\
\ \ \ \ @ReturnDirective{$4} $5 @AddTHISAsFirstArg{(\
\t\t@ExtractArgNames{@RemoveInstanceArg{$7}\;})}\;\n\
\ \ \ \ \}\n\
\/\/\<\/cppIMethodImpl>\n\
\/\/ END_NONEDITABLE_CODE\n



\/\*\SCLASS_METHOD(<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
\/\/\<cppCMethodImpl>\n\
@CommentToJava{$3}\n\
$4 $1\:\:$2$6\n\
(\n$7\n)\n\
\ \ \ \ \{\n\
\ \ \ \ @ReturnDirective{$4} $5 (\
\t\t@ExtractArgNames{$7\;})\;\n\
\ \ \ \ \}\n\
\/\/\<\/cppCMethodImpl>\n\
\/\/ END_NONEDITABLE_CODE\n

! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d
