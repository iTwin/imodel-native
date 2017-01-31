!
! gema patterns that transform C functions (headers and bodies) into
! mjava methods (both static and instance).
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\funcnames.g}}

! Ignore non-exported methods
\/\* METHOD(<I>,none,none) \*\/=

! static methods ...
\/\*\SMETHOD(<I>,<I>,none)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic static @FixTypes{$4} $2$6\N\
@OutputStaticArglist{@FixTypes{($7)}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{$8}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n

\/\*\SMETHOD(<I>,<I>,none)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S\*\W<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic static @FixTypes{$4} \*$2$6\N\
@OutputStaticArglist{@FixTypes{($7)}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{$8}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n

! instance methods ...
\/\*\SMETHOD(<I>,none,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic @FixTypes{$4} $2$6\N\
@OutputArglist{@FixTypes{($7)}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{$8}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n

\/\*\SMETHOD(<I>,none,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S\*\W<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic @FixTypes{$4} \*$2$6\N\
@OutputArglist{@FixTypes{($7)}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{$8}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n

! PATTERN: Everything else in the file
! ACTION:  swallow
?=


! (additional pattern in domains.g)
NativeInstanceCall:*=$1\-\>


! (additional patterns in domains.g)
OutputBody:\IbsiTrig_smallAngle\I=Geom.smallAngle
OutputBody:\ISUCCESS\I=Geom.SUCCESS
OutputBody:\IERROR\I=Geom.ERROR


! (additional patterns in domains.g)
FixTypes:\IDpoint3d\I=DPoint3d
