

\-\>=.
\Ivirtual\I=override 
)\Woverride\I=)
\Iinitonly\I=readonly
internal\:=internal 

\/\/\<def name\=<I>\W\><u>\<\/def\>\W=@set{$1;$2}@cmps{$1;SEPARATOR;\n$0;;\n$0}
\<def name\=<I>\W\><u>\<\/def\>\W=@set{$1;$2}@cmps{$1;SEPARATOR;\n\/\/$0;;\n\/\/$0}

Object\W\^\W<I>=Object $1


\IIS_TYPE\W(<u>,<u>)=($1 is $2)
array\W\<<u>\>\W\^\W<I>=@{$1}\[\] $2

!! constructor
public:\W<I>\W(=public $1 (

!! swallow operator overloads 
\IOPERATOR\I<u>\IENDOPERATOR\I=

static const=static readonly
ref class <I>=class $1

CPPOnly\W(<u>)=
CSOnly\W(<u>)=$1\S

ARGLIST:\W(\W)\W=()
ARGLIST:\W(\W<u>\W)\W=\n(\n$1\n)

\IJS_OFF\I<u>\IJS_ON\I=
!! method name presentation (visibility,resultType,UniqueName,ExtensionForNonOverloadedEnvironment)
CLASSMETHOD\W(\W<I>\W,\W<Balanced>,\W<I>,\W<I>\W)\W(<Balanced>)=$1 static $3$4 @ARGLIST{(@{$5})}@RESULTTYPE{$2}
INSTANCEMETHOD\W(\W<I>\W,\W<I>,\W<I>\W,\W<I>\W)\W(<Balanced>)=$1 $3$4 @ARGLIST{(@{$5})}@RESULTTYPE{$2}


\IINSTANCEOF\W(\W<I>\W,\W<I>\W)=$1 instanceOf $2

!! method name presentation (visibility,resultType,UniqueName)
CLASSMETHOD\W(\W<I>\W,\W<Balanced>,\W<I>)\W(<Balanced>)=$1 static $3 @ARGLIST{(@{$4})}@RESULTTYPE{$2}
INSTANCEMETHOD\W(\W<I>\W,\W<I>\W,\W<Balanced>)\W(<Balanced>)=$1 $3 @ARGLIST{(@{$4})}@RESULTTYPE{$2}


RESULTTYPE:\Ivoid\I=
RESULTTYPE:<u>=\ \:\ $0

!!\WENDMETHOD=\;
ENDMETHOD=

NEW_BY_ALL_FIELDS=${ThisType}.From

ARGTYPE:<u>=\n\/\* $1 \*\/\ 
ARGSIG:<u>\;\W<I>=\n$2 \: $1
\Ilong double\I=double
public value class <I> abstract sealed=public struct $1
public\: value class <I> abstract sealed=public struct $1
public\:=
private\:=
value class=struct
\WINPUT_BYVALUE\W(<u>,\W<I>\W,\W\"<u>\"\W)=@ARGSIG{$1\; $2}
\WINPUT_STRUCT\W(<u>,\W<I>\W,\W\"<u>\"\W)=@ARGSIG{$1\; $2}
\WOUTPUT_VALUE\W(<u>,\W<I>\W,\W\"<u>\"\W)=@ARGSIG{$1\; $2}
\WPARAM_BYREF\W(<u>,\W<I>\W,\W\"<u>\"\W)=@ARGSIG{$1\; $2}
\WPARAM_BYREF\W(\WOUTPARAM\W<I>\W,\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@ARGSIG{$1\; $2}

READ_WRITE_PROPERTY_FOR_FIELD\W(<u>,\W<I>\W,<u>,\W\"<u>\"\W)=@{\
\/\/\/ \<summary\> $4\<\/summary\>\n\
public $1 $2\n\
\ \ \ \ \{\n\
\ \ \ \ get \{ return $3\;\}\n\
\ \ \ \ set \{ $3 = value\;\}\n\
\ \ \ \ \}\n\
}

READ_PROPERTY\W(<I>,\W<I>\W,<u>,\W\"<u>\"\W)=@{\
\/\/\/ \<summary\> $4\<\/summary\>\n\
public $1 $2\n\
\ \ \ \ \{\n\
\ \ \ \ get \{ return $3\;\}\n\
\ \ \ \ \}\n\
}


OUTP\W(\W<I>\W)=out $1
\IOUT\I=out
IScalarFunction\W\^\W=IScalarFunction\ 
IWeightedAccumulator\W\^\W=IWeightedAccumulator\ 
BezierArray\W\^\W=BezierArray\ 

for each=foreach
ARRAY_COUNT\W(\W<I>\W)=$1.Count
\n\/\/cs=\n
\IREFP\I=

\Inullptr\I=null

\Igcnew <I>=new $1
GCNEW=new

<I>\W\^\W<I>\W\==$1 $2 \=
\IREF\I=ref
property <I>=$1
<I> get ()=get
void set (\W<I> value\W)=set
\\\n= 
!! old stuff ...
\#ifdef CSHARP_SOURCE<u>\#endif=

\#ifdef NEEDS_WORK<u>\#endif=

\#ifdef SupportManagedArrays<u>\#endif=


!! current escape to csharp..
\#ifdef CSHARP<BalancedIfBlocks>\#else<BalancedIfBlocks>\#endif=@{$1}

BalancedIfBlocks:\#if#\#endif=$0
BalancedIfBlocks:?=?


\#ifdef <I>\W\n=\#if $1\n


\n\[Serializable\]=\n\[System.SerializableAttribute\]
\n\[StructLayoutAttribute<u>\]=\n\[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)\]
\n[NameValueAttribute<u>\]=
\[Description<u>\]=
\L\[System<u>\]=
ARRAY_INIT_ASSIGNER=
ENUMERABLE_PARAM\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@ARGTYPE{Array of $1}$2
ARRAY_PARAM\W(\W<I>\W,\W<I>\W,\W\"<u>\"\W)=@ARGTYPE{Enumerablef of $1}$2

DECLARE_AND_ALLOCATE_FIXED_ARRAY\W(\W<I>\W,\W<I>\W,\W<u>\W)=$1 \[\] $2 = new $1 \[$3\]\;
FIXED_ARRAY_PARAM\W(<u>,<u>)=$1\[\] $2
DECLARE_FIXED_ARRAY_VAR(<u>,<u>)=$1 []  $2 = new $1[]

CONDITIONAL_CLEAR_ARRAY\W(<u>)=if (null \!\= $1) $1.Clear ()

CONDITIONAL_ADD_TO_ARRAY\W(<u>,<u>)=if (null \!\= $1)$1.Add ($2)

\IASSIGN_TO_THIS\W(<u>)\;=this = $1\;
STRINGPTR=System.String 
TAGGED_STRUCT_ATTRIBUTES=
\<<I>\> \^=\<$1\>

STATIC=static
\*this=this
BIG_POSITIVE=System.Double.MaxValue
BIG_NEGATIVE=(-System.Double.MaxValue)

\INEW <I>=new $1
!! Protect constructors from "new" swizzle ...
\n\WDVector2d\W(=$0
\n\WDVector3d\W(=$0
\n\WDPoint2d\W(=$0
\n\WDPoint3d\W(=$0
\n\WDPoint4d\W(=$0
\n\WDEllipse3d\W(=$0
\n\WDPlane3d\W(=$0
\n\WDSegment3d\W(=$0
\n\WDSegment1d\W(=$0
\n\WDRay3d\W(=$0
\n\WDTransform3d\W(=$0
\n\WDRange3d\W(=$0
\n\WDRange2d\W(=$0
\n\WDMatrix3d\W(=$0
\n\W\IAngle\W(=$0

public\:\W<I>\W(=public $1 (

operator <I>=$0
!! prefix constructor invocations..  someday clean up the source and invoke with NEW
\IDVector2d\W(=new DVector2d (
\IDVector3d\W(=new DVector3d (
\IDPoint2d\W(=new DPoint2d (
\IDPoint3d\W(=new DPoint3d (
\IDPoint4d\W(=new DPoint4d (
\IDPlane3d\W(=new DPlane3d (
\IDSegment1d\W(=new DSegment1d(
\IDEllipse3d\W(=new DEllipse3d (
\IDSegment3d\W(=new DSegment3d (
\IDRay3d\W(=new DRay3d (
\IDMatrix3d\W(=new DMatrix3d (
\IDRange3d\W(=new DRange3d(
\IDRange2d\W(=new DRange2d(
\IDTransform3d\W(=new DTransform3d (
\IAngle\W(=new Angle (
\:\:<I>=.$1

\n\/\/InsertParamTags=


Math\:\:Abs=Math.abs
Math\:\:Sqrt=Math.sqrt
Math\:\:Cos=Math.cos
Math\:\:Sin=Math.sin
Math\:\:Tan=Math.tan


double <I>=var $1

\#pragma warning (disable\:4100)=\/\/$0
?=?

Balanced:(#)=$0
Balanced:\{#\}=$0
Balanced:?=?