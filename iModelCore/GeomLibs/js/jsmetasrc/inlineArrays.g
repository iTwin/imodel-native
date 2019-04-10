@define{@read{@getenv{JSMETASRCPATH}\/BalancedText.g}}
! At any place in the file, replace [_X_] [_Y_] [_Z_] by x y z:


\n<SignificantSpace>FOR_XY\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XY{$1\:FOR_XY($2)\{$3\}}}
\n<SignificantSpace>FOR_XYZ\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYZ{$1\:FOR_XYZ($2)\{$3\}}}
\n<SignificantSpace>FOR_XYz\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYz{$1\:FOR_XYz($2)\{$3\}}}
\n<SignificantSpace>FOR_XYZW\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYZW{$1\:FOR_XYZW($2)\{$3\}}}
\n<SignificantSpace>FOR_XYzW\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYzW{$1\:FOR_XYzW($2)\{$3\}}}

\n<SignificantSpace>FOR_XYZ\W\[<u>\](\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYZ{$1\:FOR_XYZ\[$2\]($3)\{$4\}}}
\n<SignificantSpace>FOR_XYz\W\[<u>\](\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_FOR_XYz{$1\:FOR_XYz\[$2\]($3)\{$4\}}}

\n<SignificantSpace>SELECT_XYZ\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_SELECT_XYZ{$1\:SELECT_XYZ($2)\{$3\}}}
\n<SignificantSpace>SELECT_XYZW\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_SELECT_XYZW{$1\:SELECT_XYZW($2)\{$3\}}}

\n<SignificantSpace>SELECT_XYz\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_SELECT_XYz{$1\:SELECT_XYz($2)\{$3\}}}
\n<SignificantSpace>SELECT_XYzW\W(\W<I>\W)\W\{<BalancedText>\}=@{\n@Expand_SELECT_XYzW{$1\:SELECT_XYzW($2)\{$3\}}}



SUM_XYZ\W(\W<I>\W)\W(<BalancedText>)=(@{@Expand_SUM_XYZ{$1\:$2}})
SUM_XYz\W(\W<I>\W)\W(<BalancedText>)=(@{@Expand_SUM_XYz{$1\:$2}})

PERMUTING_SUM\W(\W<I>\W,\W<I>\W,\W<I>\W)\W(<BalancedText>)=\
@{\
\n          @RebuildSubscripts{$1 x @RebuildSubscripts{$2 y @RebuildSubscripts{$3 z $4}}}\
\n        - @RebuildSubscripts{$1 x @RebuildSubscripts{$2 z @RebuildSubscripts{$3 y $4}}}\
\n        + @RebuildSubscripts{$1 y @RebuildSubscripts{$2 z @RebuildSubscripts{$3 x $4}}}\
\n        - @RebuildSubscripts{$1 y @RebuildSubscripts{$2 x @RebuildSubscripts{$3 z $4}}}\
\n        + @RebuildSubscripts{$1 z @RebuildSubscripts{$2 x @RebuildSubscripts{$3 y $4}}}\
\n        - @RebuildSubscripts{$1 z @RebuildSubscripts{$2 y @RebuildSubscripts{$3 x $4}}}\
}


\n<SignificantSpace>FOR_XYZ\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}= @{\n$1FOR_XYZ($2)\{FOR_XYZ($3)\{$4\}\}}
\n<SignificantSpace>FOR_XYz\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}= @{\n$1FOR_XYz($2)\{FOR_XYz($3)\{$4\}\}}
\n<SignificantSpace>FOR_XYZW\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}= @{\n$1FOR_XYZ($2)\{FOR_XYZW($3)\{$4\}\}}
\n<SignificantSpace>FOR_XYzW\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}= @{\n$1FOR_XYz($2)\{FOR_XYzW($3)\{$4\}\}}

\n<SignificantSpace>FOR_OFFDIAGONALS\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}=\
@{\
\n$1@RebuildSubscripts{$2 x @RebuildSubscripts{$3 y $4}}3DONLY(\
\n$1@RebuildSubscripts{$2 x @RebuildSubscripts{$3 z $4}})\
\n$1@RebuildSubscripts{$2 y @RebuildSubscripts{$3 x $4}}3DONLY(\
\n$1@RebuildSubscripts{$2 y @RebuildSubscripts{$3 z $4}}\
\n$1@RebuildSubscripts{$2 z @RebuildSubscripts{$3 x $4}}\
\n$1@RebuildSubscripts{$2 z @RebuildSubscripts{$3 y $4}})\
TRANSFORMONLY(\n$1@RebuildSubscripts{$2 x @RebuildSubscripts{$3 w $4}})\
TRANSFORMONLY(\n$1@RebuildSubscripts{$2 y @RebuildSubscripts{$3 w $4}})3DONLY(\
TRANSFORMONLY(\n$1@RebuildSubscripts{$2 z @RebuildSubscripts{$3 w $4}}))\
}

\n<SignificantSpace>FOR_LOWEROFFDIAGONALS\W(\W<I>\W,<I>\W)\W\{<BalancedText>\}=\
@{\
\n$1@RebuildSubscripts{$2 y @RebuildSubscripts{$3 x $4}}3DONLY(\
\n$1@RebuildSubscripts{$2 z @RebuildSubscripts{$3 x $4}}\
\n$1@RebuildSubscripts{$2 z @RebuildSubscripts{$3 y $4}})\
}




SignificantSpace:/[     ]*/=$0@terminate
SignificantSpace:=@fail

BalancedText:\{#\}=$0
BalancedText:(#)=$0
BalancedText:?=?

! This is FOR_XY.. matched all over again with leading blanks as significant first arg.
Expand_FOR_XY:<u>\:FOR_XY\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$2 x $3}\n\
$1@RebuildSubscripts{$2 y $3}

Expand_FOR_XYZ:<u>\:FOR_XYZ\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$2 x $3}\n\
$1@RebuildSubscripts{$2 y $3}\n\
$1@RebuildSubscripts{$2 z $3}

Expand_FOR_XYz:<u>\:FOR_XYz\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$2 x $3}\n\
$1@RebuildSubscripts{$2 y $3}3DONLY(\n\
$1@RebuildSubscripts{$2 z $3})

!! Expand with separator
Expand_FOR_XYZ:<u>\:FOR_XYZ\W\[<u>\]\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$3 x $4}$2\
@RebuildSubscripts{$3 y $4}$2\
@RebuildSubscripts{$3 z $4}

Expand_FOR_XYz:<u>\:FOR_XYz\W\[<u>\]\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$3 x $4}$2\
@RebuildSubscripts{$3 y $4}3DONLY($2\
@RebuildSubscripts{$3 z $4})


!! 
Expand_FOR_XYZW:<u>\:FOR_XYZW\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$2 x $3}\n\
$1@RebuildSubscripts{$2 y $3}\n\
$1@RebuildSubscripts{$2 z $3}TRANSFORMONLY(\n\
$1@RebuildSubscripts{$2 w $3})

Expand_FOR_XYzW:<u>\:FOR_XYzW\W(\W<I>\W)\W\{<BalancedText>\}=\
$1@RebuildSubscripts{$2 x $3}\n\
$1@RebuildSubscripts{$2 y $3}3DONLY(\n\
$1@RebuildSubscripts{$2 z $3})TRANSFORMONLY(\n\
$1@RebuildSubscripts{$2 w $3})


Expand_SELECT_XYZ:<u>\:SELECT_XYZ\W(\W<I>\W)\W\{<BalancedText>\}=\
$1if ($2 == 0)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 x $3}\n\
$1\ \ \ \ \}\n\
$1else if ($2 == 1)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 y $3}\n\
$1\ \ \ \ \}\n\
$1else\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 z $3}\n\
$1\ \ \ \ \}\n

Expand_SELECT_XYZW:<u>\:SELECT_XYZW\W(\W<I>\W)\W\{<BalancedText>\}=\
$1if ($2 == 0)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 x $3}\n\
$1\ \ \ \ \}\n\
$1else if ($2 == 1)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 y $3}\n\
$1\ \ \ \ \}\n\
$1else if ($2 == 2)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 z $3}\n\
$1\ \ \ \ \}\n\
$1else\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 w $3}\n\
$1\ \ \ \ \}\n


Expand_SELECT_XYz:<u>\:SELECT_XYz\W(\W<I>\W)\W\{<BalancedText>\}=\
$1if ($2 == 0)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 x $3}\n\
$1\ \ \ \ \}\n\
$1else3DONLY( if ($2 == 1))\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 y $3}\n\
$1\ \ \ \ \}3DONLY(\n\
$1else\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 z $3}\n\
$1\ \ \ \ \})\n

Expand_SELECT_XYzW:<u>\:SELECT_XYzW\W(\W<I>\W)\W\{<BalancedText>\}=\
$1if ($2 == 0)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 x $3}\n\
$1\ \ \ \ \}\n\
$1else if ($2 == 1)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 y $3}\n\
$1\ \ \ \ \}\n\
3DONLY($1else if ($2 == 2)\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 z $3}\n\
$1\ \ \ \ \}\n\
)$1else\n\
$1\ \ \ \ \{\n\
$1\ \ \ \ @RebuildSubscripts{$2 w $3}\n\
$1\ \ \ \ \}\n




! This is SUM_XYZ matched all over again with leading blanks as significant first arg.
Expand_SUM_XYZ:<I>\:<u>=\
@RebuildSubscripts{$1 x $2} + @RebuildSubscripts{$1 y $2} + @RebuildSubscripts{$1 z $2}
Expand_SUM_XYz:<I>\:<u>=\
@RebuildSubscripts{$1 x $2} + @RebuildSubscripts{$1 y $2}3DONLY( + @RebuildSubscripts{$1 z $2})


RebuildSubscripts:<I> <I> <u>=@RebuildDots{$1 $2 @RebuildBrackets{$1 $2 $3}}
RebuildSubscripts:?=?

RebuildBrackets:<I> <I> <u>=@subst{\[$1\]=$2;$3}

RebuildDots:<I> <I> <u>=@subst{.$1=.$2;$3}


\<expandFile\><u>\<\/expandFile\>=@{@read{@{$1}}}
\<def name\=<I>\W\><u>\<\/def\>\W=@set{$1;$2}@cmps{$1;SEPARATOR;\n\/\/$0;;\n\/\/$0}
\<defExpansion name\=<I>\W\><u>\<\/defExpansion\>\W=@set{$1;@{$2}}
\$(\W<I>\W)=@{@var{$1;(Unable to expand $1)}}

\<FixTrailingComma\><u>\<\/FixTrailingComma\>=@FixTrailingComma{@{$1}}

FixTrailingComma:,<S>)=$1)
FixTrailingComma:?=?


MULTISUB\W(<I>\;<u>\;<u>)=\
@push{_MULTISUB_BODY_;$3}\
@push{_MULTISUB_ID_;$1}\
@ExecuteMultiSub{@{$2}}\
@pop{_MULTISUB_ID_}\
@pop{_MULTISUB_BODY_}

FOREACH\W<I> in <u> expand <BalancedText> END_FOREACH=\
@push{_MULTISUB_BODY_;$3}\
@push{_MULTISUB_ID_;$1}\
@ExecuteMultiSub{@{$2}}\
@pop{_MULTISUB_ID_}\
@pop{_MULTISUB_BODY_}

ExecuteMultiSub:<I>=@subst{\\I@var{_MULTISUB_ID_}\\I=$1;@var{_MULTISUB_BODY_}}
ExecuteMultiSub:(<BalancedText>)=@{$1}
ExecuteMultiSub:?=?
