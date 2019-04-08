! gema patterns to transform native functions into JNI-callable methods

@define{@read{@getenv{GEMADIR}g\\domains.g}}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\*\SJNIMETHOD(<T>)\S\*\/\S\
\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\* BEGIN_NONEDITABLE_CODE \*\/\N\
@JNI_FunctionName{@JavaTypes{$6},$7,@JNI_ClassFromArgList{$9},@JNI_ImpName{($1)}}\
$8\N(\n\
JNIEnv\t\*pJniEnv,\
@JNI_ArgList{@StripConst{@JavaTypes{$9}}}\n)\n\t\{\
@JNI_DeclareLocals{@StripConst{$9}}\
@JNI_DeclareResult{@SimpleCTypeOf{$6}}\
\n@JNI_AccessLocals{@StripConst{$9}}\
\n\t@JNI_AssignResult{@SimpleCTypeOf{$6}}@JNI_CalledFunctionName{$7} (@JNI_NativeArgs{@StripConst{$9}}\n\t\t)\;\
@JNI_CopyLocalsToObjects{$9}\
\n\t@JNI_ReturnResult{@SimpleCTypeOf{$6}}\
\n\t\}\
\n\/\* END_NONEDITABLE_CODE \*\/\n


?=


! Argument of JNIMETHOD() may have two fields if overloaded; concat if so
JNI_ImpName:(<I>,<I>)=$1$2
JNI_ImpName:(<I>)=$1
JNI_ImpName:<T>=$0


! The class of the instance is the identifier before the first star
! in the arg list.  Got it?
JNI_ClassFromArgList:<I> \*<I>=@JavaTypes{$1}@terminate
JNI_ClassFromArgList:?=


! Usage: @JNI_FunctionName{resultType,nativeName,className,methodName}
JNI_FunctionName:<I>,<I>,<I>,<I>=\
JNIEXPORT @JNI_ReturnType{$1} JNICALL Java_com_bentley_dgn_$3_$4


JNI_DeclareResult:void=
JNI_DeclareResult:<I>=\n\t$1 result\;


JNI_ArgList:<I> \*<I>=\njobject\t$2
JNI_ArgList:<I> <I>=\nj$1\t$2
JNI_ArgList:,=,
JNI_ArgList:?=


JNI_ReturnType:boolean=jboolean
JNI_ReturnType:double=jdouble
JNI_ReturnType:int=jint
JNI_ReturnType:*=$0


! Declare pointers for matrix data, structs for points, guess for others:
JNI_DeclareLocals:DPoint3d \*\W<I>=\n\tDPoint3d local_$1\;
JNI_DeclareLocals:DMatrix3d \*\W<I>=\n\tRotMatrix \*p$1\;
JNI_DeclareLocals:DTransform3d \*\W<I>=\n\tTransform \*p$1\;
JNI_DeclareLocals:<I> \*\W<I>=\n\t$0\;
JNI_DeclareLocals:?=


JNI_AccessLocals:DPoint3d \*\W<I>=\n\textractDPoint3d (pJniEnv, \&local_$1, $1)\;
JNI_AccessLocals:DMatrix3d \*\W<I>=\n\tgetRotMatrix (pJniEnv, \&p$1, $1)\;
JNI_AccessLocals:DTransform3d \*\W<I>=\n\tgetTransform (pJniEnv, \&p$1, $1)\;
JNI_AccessLocals:<I> \*\W<I>=\n\tget$1 (pJniEnv, \&p$1, $1)
JNI_AccessLocals:?=


! Objects that are COPIED to locals must be copied back.
! For instance, DPoint:
JNI_CopyLocalsToObjects:const <I> \*\W<I>=     ! Any const struct is ok
JNI_CopyLocalsToObjects:DPoint3d \*\W<I>=\n\tsetDPoint (pJniEnv, @StripConst{$1}, \&local_@StripConst{$1})\;
JNI_CopyLocalsToObjects:?=


JNI_NativeArgs:DPoint3d \*\W<I>=\n\t\t\&local_$1
JNI_NativeArgs:,=,
JNI_NativeArgs:<I> \*\W<I>=\n\t\tp$2
JNI_NativeArgs:<I>\W<I>=\n\t\t$2
JNI_NativeArgs:?=


JNI_AssignResult:void=
JNI_AssignResult:<I>=result \=\
JNI_AssignResult:*=\/\* Unknown return type $0 \*\/\n


! JNI_ReturnResult(CResultType) -- generate  "return X" where X may be the result variable
!   or empty string according to the CResultType
JNI_ReturnResult:void=return\;
JNI_ReturnResult:<I>=return result\;
JNI_ReturnResult:*=\/\* Unknown return type $0 \*\/return\;


JNI_CalledFunctionName:DMatrix3d=RotMatrix
JNI_CalledFunctionName:DTransform3d=Transform
JNI_CalledFunctionName:?=?