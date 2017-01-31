!
! gema patterns to pluck out C-sharpish instance methods.
!
! /* CSVFUNC(csName) */...Public ... jmdlXXX_YYY ....
! Means....
!  Make a C# CLASS method which invokes jmdlXXX_YYY.
!  There is a crucial correspondence between the C and C# arg lists:
!  1) There must be only one returned value from the C function.  This can be
!       either the function return or a single non-const parameter.
!  1a) If the C function return value is a primitive type, the return
!                   of the c# class method is the same
!  1b) If the C function has void return, the return of the C# method is
!                   the VALUE of the (one and only) non-const arg.
!  2) All other C args (both primitives and const pointers) are value
!                   parameters to the C# method.
!
! /*CSIMETHOD(csName) */...Public ... jmdlXXX_YYY ....
! Means...
!  Make a C# INSTANCE method.
!  The first C param becomes the instance var. ("this")
!  The returns are the same type in C and C# (presumably void or a primitive)
!  Remaining params convert as follows:
!       C const pointer becomes ref
!       C nonconst pointer becomes out
!       C primitive becomes same primitive.
!
! Clear as mud?
!
@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
@define{@read{@getenv{GEMADIR}g\\convertbody.g}}

! Place "@dllName name" (probably in a comment) to get away from the default bsibasegeom lib name.
@set{dllName;bsibasegeom}
\@dllName <I> =@set{dllName;$1}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:
!     Echo the comment
!   Make an instance method which does nothing but call the static method (follows)
!   Make a static method linked to the native code.
!
\/\* METHOD(<I>,<I>,none) \*\/=

!                           $1=csName   $2=comments        #3=retType #4=cName $5=arglist
\/\*\WCSIMETHOD\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)=\
@ConvertVarNamesCToCSharp{\
\n\/\/ BEGIN_NONEDITABLE_CODE\
@ConvertComment{$2}\n\
public @ConvertReturnType{$3} @ConvertMethodName{$1}\n\
@set{LineFeed;\n\ \ \ \ }\
@RenameTypes{\
    @ConvertToValueParam{\
            @InstanceArgsFromFullArgs{(@ConvertCSArgs{$5})}}}@var{LineFeed}\
\{@var{LineFeed}\
@ReturnDirective{$3} @ConvertMethodName{$4}\
            @ArglistSigToCallArgs{@CallFirstArgThis{($5)\;}}@var{LineFeed}\
\}\n@var{LineFeed}\
@DeclareDLLLinkage{($1)$0}\
\/\/ END_NONEDITABLE_CODE\n\
}


DeclareDLLLinkage:(<I>)<u>Public <I> <I>\W(<arglist>)=\
\n@var{LineFeed}\
\[@var{LineFeed}\
DllImport(\@\"bsibasegeom.dll\",@var{LineFeed}\
EntryPoint\=\"$4\",@var{LineFeed}\
CharSet=CharSet.Ansi,@var{LineFeed}\
ExactSpelling=true,@var{LineFeed}\
CallingConvention=CallingConvention.Cdecl)@var{LineFeed}\
\]\n\
private extern static @ConvertReturnType{$3} @ConvertMethodName{$4}\n\
@RenameTypes{\
    @ConvertCArgsToRef{\
            ($5)\
}}\;\n
DeclareDLLLinkage:<u>=// BAD DLL LINKAGE INVOCATION\n$0


RenameTypes:\IRotMatrix\I=DMatrix3d
RenameTypes:\ITransform\I=DTransform3d
RenameTypes:?=?

ReturnDirective:\Wvoid\I\W=@end
ReturnDirective:<u>=return\

ConvertCSArgs:\Iconst\I\W=
ConvertCSArgs:\*=
ConvertCSArgs:\n\W=\n
ConvertCSArgs:?=?

ArglistSigToCallArgs:const\W<I>\W\*\W<I><ArgTerminator>=ref $2$3
ArglistSigToCallArgs:<I>\W\*\W<I><ArgTerminator>=out $2$3
ArglistSigToCallArgs:<I> <I><ArgTerminator>=$2$3
ArglistSigToCallArgs:\W=
ArglistSigToCallArgs:?=?

ArgTerminator:\W,\W=,\ @end
ArgTerminator:\W)\W=)@end
ArgTerminator:=@fail

ConvertCArgsToRef:const <I>\W\*<I>=ref $1 $2
ConvertCArgsToRef:<I>\W\*\W<I>=out $1 $2
ConvertCArgsToRef:<I> <I>=$1 $2
ConvertCArgsToRef:?=?

! Value params are EZ, yes?
ConvertToValueParam:?=?

CallFirstArgThis:<I>\W,<u>=this,$2
CallFirstArgThis:<I>\W)=this)
CallFirstArgThis:?=?

InstanceArgsFromFullArgs:\W(\W<I> <I>\W)\W=(\n)
InstanceArgsFromFullArgs:\W(\W<I> <I>\W,<u>=($3
InstanceArgsFromFullArgs:\W(\W<I> <I>\W)\W=(\n)
InstanceArgsFromFullArgs:\W(\W<I> <I>\W,<u>=($3
InstanceArgsFromFullArgs:<u>=\/\/Translation error InstanceArgsFromFullArgs\[$0\]\n

ConvertComment:\/\*\WMETHOD\W(<I>,<I>,<I>)\W\*\/\n=
ConvertComment:\@instance =\@param

! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d


! first arg...
GetNonConstTypeFromArgList:\W<I>\W\*\W<I><u>=$1@end
! second arg...
GetNonConstTypeFromArgList:\Wconst <I>\W\*\W<I>\W,\W<I>\W\*\W<I><u>=$3@end
! failed!!
GetNonConstTypeFromArgList:<u>=<No Returnable Struct In Arg List>


! CONST args remains ...
ValueFuncArgsFromFullArgs:\n\Wconst <I>\W\*\W<I>\W=\nconst $1 \*$2
! Var arg (and trailing comma, if any) goes away ...
ValueFuncArgsFromFullArgs:\n\W<I>\W\*\W<I>\W\,=
ValueFuncArgsFromFullArgs:\n\W\I<I>\W\*\W<I>=
! Primitive value stays ...
ValueFuncArgsFromFullArgs:\n\W<I> <I>=$0
ValueFuncArgsFromFullArgs:?=?

! Not the const art
DeclareLocalResult:\Iconst <I>\W\*\W<I>\W=
DeclareLocalResult:\I<I>\W\*\W<I>\W\,=$1 $2 \= new $1()\;
DeclareLocalResult:\I<I>\W\*\W<I>=$1 $2 \= new $1()\;
DeclareLocalResult:\I<I> <I>\I=
DeclareLocalResult:?=

! Not the const art
ReturnTheLocalResult:\Iconst <I>\W\*\W<I>\W=
ReturnTheLocalResult:\I<I>\W\*\W<I>\W\,=return $2\;
ReturnTheLocalResult:\I<I>\W\*\W<I>=return $2\;
ReturnTheLocalResult:\I<I> <I>\I=
ReturnTheLocalResult:?=

!@ConvertComment{$2}\n

ConvertVarNamesCToCSharp:\Ip<K><I>=$1$2
ConvertVarNamesCToCSharp:\Ip<K>\I=$1
ConvertVarNamesCToCSharp:?=?

!                           $1=csName   $2=comments         #3 = returnType #4=cName $5=arglist
! ... arglist must have a single non-const.
! ... the non-const will be treated as "OUT"
! ... the non-const will not appear in the arg list
! ... the method is a class (static) method!!!
\/\*\WCSVFUNC\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)=\
@set{ResultIsParameter;@cmps{$3;void;TRUE;FALSE;TRUE}}\
\n\/\/ BEGIN_NONEDITABLE_CODE\
@RenameTypes{\
@ConvertVarNamesCToCSharp{\
@ConvertComment{$2}\n\
public static \
@cmps{@var{ResultIsParameter};TRUE;;@ConvertReturnType{$3};}\
@cmps{@var{ResultIsParameter};FALSE;;@ConvertReturnType{@GetNonConstTypeFromArgList{$5}};}\
\ @ConvertMethodName{$1}\n\
@set{LineFeed;\n\ \ \ \ }\
@RenameTypes{\
    @ConvertToValueParam{\
            @ConvertCSArgs{(@ValueFuncArgsFromFullArgs{$5}\N)}}}@var{LineFeed}\
\{@var{LineFeed}\
@cmps{@var{ResultIsParameter};FALSE;;@DeclareLocalResult{$5}@var{LineFeed}\
@ConvertMethodName{$4} @ArglistSigToCallArgs{($5)\;}@var{LineFeed}\
@ReturnTheLocalResult{$5}@var{LineFeed}\
;}\
\
@cmps{@var{ResultIsParameter};TRUE;;\
return @ConvertMethodName{$4} @ArglistSigToCallArgs{($5)\;}@var{LineFeed}\
;}\
\}\n\
}\
}\
@DeclareDLLLinkage{($1)$0}\
\/\/ END_NONEDITABLE_CODE\n


!                           $1=csName   $2=comments         #3 = returnType #4=cName $5=arglist
\/\*\WCSDECLARECLASSMETHOD\W(\W<I>\W)\W\*\/<u>\n\WPublic <I> <I>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\
@DeclareDLLLinkage{($1)$0}\
\/\/ END_NONEDITABLE_CODE\n
