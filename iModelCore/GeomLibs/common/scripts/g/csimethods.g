!
! gema patterns to pluck out C-sharpish instance methods.
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
@define{@read{@getenv{GEMADIR}g\\convertbody.g}}

! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\* METHOD(<I>,<I>,none) \*\/=

!                           $1=csName   $2=comments        #3=retType #4=cName $5=arglist $6=body
\/\*\WCSIMETHOD\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)\W\{<FunctionBody>\}=\
\n\/\/ BEGIN_NONEDITABLE_CODE\
@ConvertComment{$2}\n\
public @ConvertReturnType{$3} @ConvertMethodName{$1} \/\/ $4\n\
@RenameTypes{\
    @ConvertToRef{\
            @InstanceArgsFromFullArgs{(\
                    @ConvertArgs{$5}\
            )\
        }\
}}\n\
\ \ \ \ \{\
@RenameTypes{\
    @PassByRef{\
            @ConvertBody{\
                    @subst{@InstanceNamePatternFromOriginalArgs{$5};@ResolveConditionalCSharp{$6}}\
                    }\
            }\
    }\
\}\n\
\/\/ END_NONEDITABLE_CODE\n

FunctionBody:\{#\}=$0
Functionbody:?=?

RenameTypes:\IRotMatrix\I=DMatrix3d
RenameTypes:\ITransform\I=DTransform3d
RenameTypes:?=?

ResolveConditionalCSharp:\#ifdef <I>\W\n=\#if ($1)\n
ResolveConditionalCSharp:\#if defined (\W<I>\W)\W\n=\#if ($1)\n
ResolveConditionalCSharp:\#elif defined (\W<I>\W)\W\n=\#elif ($1)\n
ResolveConditionalCSharp:?=?

ConvertArgs:\Iconst\I\W=
ConvertArgs:\*=
ConvertArgs:\n\W=\n
ConvertArgs:?=?

! Find (we hope) all pointers passed to functions, annotate as a call by ref.
PassByRef:(\W<PointerVarName>\W\P,=(ref $1
PassByRef:(\W<PointerVarName>\W\P)=(ref $1
PassByRef:,\W<PointerVarName>\W\P)=, ref $1
PassByRef:,\W<PointerVarName>\W\P,=, ref $1

PointerVarName:<PointerVarName0>\W\[\W<I>\W\]=$0
PointerVarName:<PointerVarName0>=$0
PointerVarName:=@fail

PointerVarName0:\Ip/[A-Z]/<I>=$0
PointerVarName0:\Ip/[A-Z]/\I=$0
PointerVarName0:\Ithis\I=$0
PointerVarName0:=@fail


ConvertBody:RotMatrix <I> \= \*<I>\;=RotMatrix $1 \= new RotMatrix(ref $2)\;
ConvertBody:Transform <I> \= \*<I>\;=Transform $1 \= new Transform(ref $2)\;
ConvertBody:DPoint3d <I> \= \*<I>\;=DPoint3d $1 \= new DPoint3d(ref $2)\;
ConvertBody:DPlane3d <I> \= \*<I>\;=DPlane3d $1 \= new DPlane3d(ref $2)\;
! Let explicit csharp stuff go through..
ConvertBody:DPoint3d \[\]=$0

ConvertBody:\IDPoint3d <u>\;=DPoint3d @InitializeList{DPoint3d(0.0, 0.0, 0.0)\;$1\;}
ConvertBody:\IDPoint4d <u>\;=DPoint4d @InitializeList{DPoint4d()\;$1\;}
ConvertBody:\IRotMatrix <u>\;=RotMatrix @InitializeList{RotMatrix(1.0)\;$1\;}
ConvertBody:\ITransform <u>\;=Transform @InitializeList{Transform(1.0)\;$1\;}

ConvertBody:\IDMatrix3d <u>\;=RotMatrix @InitializeList{RotMatrix(1.0)\;$1\;}
ConvertBody:\IDTransform3d <u>\;=Transform @InitializeList{Transform(1.0)\;$1\;}


ConvertBody:\&\&=$0
ConvertBody:\&<I>=ref $1
ConvertBody:\-\>=\.

ConvertBody:this.<I> \= \*<I>\;=$1 \= $2\;
ConvertBody:\*<I> \= \*<I>\;=$1 \= $2\;
ConvertBody:\*<I> \= <I>\;=$1 \= $2\;
ConvertBody:<I> \= \*<I>\;=$1 \= $2\;

ConvertBody:?=?


ConvertToRef:\IDPlane3d\I=ref DPlane3d
ConvertToRef:\IDPoint4d\I=ref DPoint4d
ConvertToRef:\IDPoint3d\I=ref DPoint3d
ConvertToRef:\IDPoint2d\I=ref DPoint3d
ConvertToRef:\IRotMatrix\I=ref RotMatrix
ConvertToRef:\ITransform\I=ref Transform
ConvertToRef:\IDEllipse3d\I=ref DEllipse3d
ConvertToRef:?=?

InitializeList:<u>\;\W<I>\W,<u>=$2 \= new $1,\n\t\t@InitializeList{$1\;$3\;}
InitializeList:<u>\;\W<I>\W\;=$2 \= new $1\;





InstanceArgsFromFullArgs:\W(\W<I> <I>\W)\W=(\n)
InstanceArgsFromFullArgs:\W(\W<I> <I>\W,<u>=($3
InstanceArgsFromFullArgs:\W(\W<I> <I>\W)\W=(\n)
InstanceArgsFromFullArgs:\W(\W<I> <I>\W,<u>=($3
InstanceArgsFromFullArgs:<u>=\/\/Translation error InstanceArgsFromFullArgs\[$0\]\n

InstanceNamePatternFromOriginalArgs:\W<I>\W\*\W<I>=$2\=this@end
InstanceNamePatternFromOriginalArgs:\Wconst <I>\W\*\W<I>=$2\=this@end
InstanceNamePatternFromOriginalArgs:?=?


ConvertComment:\/\*\WMETHOD\W(<I>,<I>,<I>)\W\*\/\n=

! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d
