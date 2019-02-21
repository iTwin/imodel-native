@define{@read{@getenv{METASRCPATH}\/BalancedText.g}}

!Noisy:<u>=@err{$0}
Noisy:<u>=

\L\/\/declarations\W(<u>) with \{<Patterns>\}=$0\n\
@Noisy{DECLARATIONS $1 WITH $2\n}\
@define{$2}\n\
@out{@{@FixArgs{@ExpandDeclarations{@MapLibraryReferences{@read{@{@REROUTE{$1}}}}}}}} \n\/\/ (end declarations)\
@undefine{$2}

\L\/\/declarations\W(<T>)=\
@out{@ExpandDeclarations{@MapLibraryReferences{@FixArgs{@read{@{@REROUTE{$1}}}}}}}
?=?

\L\/\/implementations\W(<u>) with \{<Patterns>\}=$0\n\
@Noisy{IMPLEMENTATIONS $1 WITH $2\n}\
@define{$2}\n\
@Noisy{__THISTYPE__ maps to @{__THISTYPE__}\n}\
@Noisy{__PVTYPE__ maps to @{__PVTYPE__}\n}\
@Noisy{__VECTORTYPE__ maps to @{__VECTORTYPE__}\n}\
@Noisy{__POINTTYPE__ maps to @{__POINTTYPE__}\n}\
@out{@{@ExpandImplementations{@MapLibraryReferences{@FixArgs{@read{@{@REROUTE{$1}}}}}}}}\
@undefine{$2}

Description\W(\W\"<u>\"\W)=Description(\"@{@StripQuotes{$1}}\")

EmitDescription:<u>=\[Description(\"@{@StripQuotes{$1}}\")\]\n
EmitDescription:=\[Description(\"\")\]\n

StripQuotes:\"=\&quote\;
StripQuotes:?=?


! Strip off extraneous cvs stuff.
ExpandDeclarations:\L\$<I>\:<u>\$=
ExpandImplementations:\L\$<I>\:<u>\$=

FixArgs:MANAGEDARRAYP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}array \<$1\> \^$2}

!!FixArgs:VALUEP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}$1 $2}
FixArgs:VALUEP\W(\W<I>\W,\W<I>\W)=@{$1 $2}

!!FixArgs:CONSTREFP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}$1 %$2}
!!FixArgs:CONSTREFP\W(\W<I>\W,\W<I>\W)=@{$1 % $2}
FixArgs:CONSTREFP=PARAM_BYCONSTREF

FixArgs:\IREFP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}$1 REFOP $2}
FixArgs:\IREFP\W(\W<I>\W,\W<I>\W)=@{$1 REFOP $2}

FixArgs:OUTP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}[System::Runtime::InteropServices::Out] $1 REFOP $2}
FixArgs:OUTP\W(\W<I>\W,\W<I>\W)=[System::Runtime::InteropServices::Out]@{ $1 REFOP $2}

!!FixArgs:CONSTPTRP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}const $1  \*$2}
FixArgs:PTRP\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}$1  \*$2}

FixArgs:CONSTPTRP\W(\W<I>\W,\W<I>\W)=@{const $1  \*$2}
FixArgs:PTRP\W(\W<I>\W,\W<I>\W)=@{$1  \*$2}

FixArgs:OUTARRAY\W(\W<I>\W,\W<I>\W)=@{array <$1> \^ $2}
FixArgs:OUTARRAY\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@{@EmitDescription{$3}array <$1> \^ $2}


VALUE_CONSTRUCTOR=
public VALUE_CLASS=\[Serializable\]\n\[StructLayoutAttribute(LayoutKind::Sequential)\]\npublic value class
public DATACONTRACT_CLASS=\[DataContract\]\n\[StructLayoutAttribute(LayoutKind::Sequential)\]\npublic value class
\IDATA_MEMBER\I=[DataMember]
!VALUE_CLASS=value class
STRINGPTR=System\:\:String \^
OVERRIDE=
REFOP=\%


REROUTE:\/msj\/=@getenv{MSJ}
REROUTE:?=?

Patterns:(#)=$0
Patterns:\{#\}=$0
Patterns:\[#\]=$0
Patterns:\<#\>=$0
Patterns:?=?

MapLibraryReferences:\LMETHODNAME\W(<Patterns>)=@{__THISTYPE__\:\:$1}


! const const const !!!
!MapLibraryReferences:CONSTREFP\W(\W<I>\W,\W<I>\W)=$1 REFOP $2
!MapLibraryReferences:PTRP\W(\W<I>\W,\W<I>\W)=$1  \*$2
!MapLibraryReferences:CONSTPTRP\W(\W<I>\W,\W<I>\W)=const $1 \*$2


MapLibraryReferences:BEGIN_EXPANSION\W\/\*<u>\*\/=
MapLibraryReferences:\n\W\#ifdef METASRC <u>\#endif\W\n=\n
MapLibraryReferences:\IGeometry.=Geometry\:\:
MapLibraryReferences:?=?

! DECL..END_DECL is raw code...
ExpandDeclarations:\n\WDECL\I<u>\IEND_DECL\I=$1

!!ExpandDeclarations:\IPUBLIC\I<u>\IBODY\I<u>\IEND_BODY\I=@EmitSummaryXML{$0}@EmitParamXML{$1}@Declaration{$0}
ExpandDeclarations:\IPUBLIC\I<u>\IBODY\I<u>\IEND_BODY\I=@EmitSummaryXML{$0}@Declaration{$0}
ExpandDeclarations:\IPRIVATE\I<u>\IEND_BODY\I=@Declaration{$0}
ExpandDeclarations:\n\WIMPL\I<u>\IEND_IMPL\I=
ExpandDeclarations:?=?

Declaration:\LCONSTRUCTOR\W(<u>)=$1
Declaration:\n\W\IBODY\I<u>\IEND_BODY\I=\;
Declaration:\IPUBLIC\I=public:
Declaration:\IPRIVATE\I=private:
Declaration:\ISTATIC\I=static
Declaration:\IATTR\W(\[<u>\])=\[$1\]
Declaration:?=?

!EmitParamXML:<u>=\n\#ifdef abc\n$0\n\#endif\n
EmitParamXML:TRANSFORMONLY\W(<BalancedText>)=TRANSFORMONLY(@EmitParamXML{$1})
EmitParamXML:3DONLY\W(<BalancedText>)=3DONLY(@EmitParamXML{$1})
EmitParamXML:<I>\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@EmitNamedTag{param $3 $4}
EmitParamXML:?=

EmitSummaryXML:PUBLIC\W\[Description\W(\W\"<u>\"\W\)\W\]=@EmitTag{summary $1}
EmitSummaryXML:?=

EmitTag:<I> <u>=\/\/\/ \<$1\>@EmitTagBody{$2}\<\/$1\>\n
EmitTag:?=

EmitNamedTag:<I> <I> <u>=\/\/\/ \<$1 name\=\"$2\"\>@EmitTagBody{$3}\<\/$1\>\n
EmitNamedTag:?=


EmitTagBody:\\\n=\n\/\/\/\ \ \ \ \ \ \ \S
EmitTagBody:?=?


ExpandImplementations:\Ivirtual\I=
ExpandImplementations:\Ioverride\I=

ExpandImplementations:\LCONSTRUCTOR\W(<u>)=__THISTYPE__::$1
ExpandImplementations:\IATTR\W(\[<u>\])=
! Passing parameters by ref ....
ExpandImplementations:\IREFP\W(\Wthis\W)=\*this
ExpandImplementations:\IREFP\W(<Patterns>)=$1
ExpandImplementations:\IOUTP\W(\Wthis\W)=\*this
ExpandImplementations:\IOUTP\W(\W<I>\W)=$1
! Received array just needs name to go to called procedure....
ExpandImplementations:ARRAYP\W(\W<I>\W)=$1

ExpandImplementations:\n\WIMPL\I<u>\n\WEND_IMPL\I=$1


! Strip out attributes. (They should have already appeared in the declarations)
!!!!!!!!   ExpandImplementations:\[Description\W(<u>\)\W]\W\n=
! This strips only from method implementations?????
ExpandImplementations:PUBLIC\W\[Description\W(<u>\)\W]\W\n=
ExpandImplementations:PRIVATE\W\[Description\W(<u>\)\W]\W\n=
ExpandImplementations:\IDECLONLY\W(<BalancedText>)=

ExpandImplementations:<I>\W\=\Wthis\W\;=$1 \= \*this\;
ExpandImplementations:\Ithis\W\==\*this =

ExpandImplementations:\Ithis.<I>=this->$1

ExpandImplementations:\n\WBODY\I=
ExpandImplementations:\n\WEND_BODY\I=
ExpandImplementations:\n\WDECL<u>END_DECL\I=

ExpandImplementations:\IPUBLIC\I\W=
ExpandImplementations:\IPRIVATE\I\W=
ExpandImplementations:\ISTATIC\I=
ExpandImplementations:?=?
