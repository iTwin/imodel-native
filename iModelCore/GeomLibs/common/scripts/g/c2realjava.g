!
! gema patterns that transform C functions (headers and bodies) into
! pure java methods (both static and instance).
!
! NOTE: what to do with functions that return pointer?

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\funcnames.g}}

! Ignore non-exported methods
\/\* METHOD(<I>,none,none) \*\/=

! static methods ...
\/\*\SJAVASTATIC(<I>)\S\*\/\S\
\/\*\SMETHOD(*)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic static @FixTypes{$4} $1$6\N\
@OutputStaticArglist{@FixTypes{($7)}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{@ArrowToDot{$8}}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n


! instance methods ...
\/\*\SJAVAMETHOD(<I>)\S\*\/\S\
\/\*\SMETHOD(*)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)\S\{<balancedBraces>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$3}\Npublic @FixTypes{$4} $1$6\N\
@OutputNoFrillsArglist{@FixTypes{(@StripStars{$7})}}\n\ \ \ \ \{@OutputBody{@FixTypes{@RenameFunctions{@ArrowToDot{$8}}}}\}\
\n\/\/ END_NONEDITABLE_CODE\n


! PATTERN: Everything else in the file
! ACTION:  swallow
?=


ArrowToDot:\-\>=.
ArrowToDot:?=?


StripStars:\*=
StripStars:\Iconst\W=
StripStars:?=?


! (additional patterns in domains.g)
CommentToJava:\IDPoint3d\I=DPoint
CommentToJava:\IDMatrix3d\I=RotMatrix
CommentToJava:\IDTransform3d\I=Transform


! Wipe first arg (& comment) only
! Easier than mjava - no const, no stars
OutputNoFrillsArglist:(\W<I>\W<I>,<OPTIONALCOMMENT>\W\N<T>\N)=($4\N)
OutputNoFrillsArglist:(\W<I>\W<I><OPTIONALCOMMENT>\W\N)=(\n)
OutputNoFrillsArglist:(\Wvoid\W)=(\n)
OutputNoFrillsArglist:<T>=\n\/\* \?\? NOTMATCHED [$1] \?\?\*\/\n


! (additional pattern in domains.g)
NativeInstanceCall:*=$1.


! (additional patterns in domains.g)
OutputBody:\IbsiTrig_smallAngle\W(\W)\I=sm_smallAngle
OutputBody:\IpInstance\W.=
OutputBody:\IpInstance\W\-\>=
OutputBody:\&\&=$0
OutputBody:\&=! Never ever any reason to take an address in java.  BUT... beware of bitops!!


! Real java needs its pointers to be initialized.
! Need a real parser here!!
! Handles only up to three variable declarations sharing a single type.
! (additional patterns in domains.g)
FixTypes:DPoint3d\W<I>\;=DPoint $1 \= new DPoint ()\;
FixTypes:DPoint3d\W<I>\W,\W<I>\W\;=DPoint $1 \= new DPoint ()\;\n\tDPoint $2 \= new DPoint ()\;
FixTypes:DPoint3d\W<I>\W,\W<I>\W,\W<I>\W\;=DPoint $1 \= new DPoint ()\;\n\
    \tDPoint $2 \= new DPoint ()\;\n\tDPoint $3 \= new DPoint ()\;
FixTypes:\IDPoint3d\I=DPoint
FixTypes:\IDpoint3d\I=DPoint
