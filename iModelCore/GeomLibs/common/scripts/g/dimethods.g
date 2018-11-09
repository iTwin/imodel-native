!
! gema patterns to pluck out instance method headers and generate
!    stub methods that are to be included in a wrapper class that has the
!    struct as an instance variable and needs to delegate the method call.
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}

! DOMAIN: Default
! PATTERN: <Comment block> + Public + func type and name + arglist
! ACTION:  The pattern followed by ;

\/\* METHOD(<I>,<I>,none) \*\/=


\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
public @FixTypes{$5} $3$7\n\
@OutputObjectArglist{@FixTypes{($8)}}\
@NewLine{x}\{\
@NewLine{x}__DECLARE_LOCALS_FOR_DELEGATED_METHOD__\
@NewLine{x}__GET_LOCALS_FOR_DELEGATED_METHOD__\
@NewLine{x}@DeclareAndAssignDelegatedMethodResult{$5}__DELEGATE_STRUCTURE_NAME__.$3\ @OutputInstanceArgIdentifiers{($8)}\;\
@NewLine{x}@AnnounceChanges{($8)}\
@NewLine{x}@ReturnDelegatedMethodResult{$5}\
@NewLine{x}\}\
\n\/\/ END_NONEDITABLE_CODE\n


! Same as above, but match on pointer type?
\/\*\SMETHOD(<I>,<I>,<I>)\S\*\/\S\
<COMMENT>\W\N\
Public\S<I>\S\*\W<I><OPTIONALCOMMENT>\W(<arglist>)=\
\n\/\/ BEGIN_NONEDITABLE_CODE\n\
@CommentToJava{$4}\n\
public @FixTypes{$5} \*$3$7\n\
@OutputObjectArglist{@FixTypes{($8)}}\
@NewLine{x}\{\
@NewLine{x}__DECLARE_LOCALS_FOR_DELEGATED_METHOD__\
@NewLine{x}__GET_LOCALS_FOR_DELEGATED_METHOD__\
@NewLine{x}@DeclareAndAssignDelegatedMethodResult{$5 \*}__DELEGATE_STRUCTURE_NAME__.$3\ @OutputInstanceArgIdentifiers{($8)}\;\
@NewLine{x}@AnnounceChanges{($8)}\
@NewLine{x}@ReturnDelegatedMethodResult{$5 \*}\
@NewLine{x}\}\
\n\/\/ END_NONEDITABLE_CODE\n


! PATTERN: Everything else in the file
! ACTION:  swallow
?=

! Usage:  @DeclareAndAssignDelegatedMethodResult{__returnType__}
!   where __returnType__ is the return type as stripped from the C function signature
DeclareAndAssignDelegatedMethodResult:<I>\W\*=$1 \*pResultPointer \=\
DeclareAndAssignDelegatedMethodResult:void=
DeclareAndAssignDelegatedMethodResult:<I>=@FixTypes{$1} resultValue \=\

! Usage:  @ReturnDelegatedMethodResult{__returnType__}
!   where __returnType__ is the return type as stripped from the C function signature
ReturnDelegatedMethodResult:<I>\W\*=return pResultPointer\;
ReturnDelegatedMethodResult:void=
ReturnDelegatedMethodResult:<I>=return resultValue\;

AnnounceChanges:(\Wconst*)=
AnnounceChanges:*=__ANNOUNCE_CHANGES_BY_DELEGATED_METHOD__


! (additional patterns in domains.g)
FixTypes:Dpoint3d=DPoint3d

! NewLine function -- Turn eacg character into 4 blanks on a new line, with tabs for 8
NewLine:????=\n\t\t
NewLine:???=\n\t\ \ \ \
NewLine:??=\n\t
NewLine:?=\n\ \ \ \
