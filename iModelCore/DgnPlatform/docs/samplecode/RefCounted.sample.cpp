/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/RefCounted.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ RefCounted_Basics.sampleCode
#include <Bentley/RefCounted.h>         // Include types that implement the reference-counting pattern
#include <Bentley/NonCopyableClass.h>   // Include base class to make a class non-copyable.

struct Foo;                             // Forward declaration
typedef RefCountedPtr<Foo> FooPtr;      // A RefCountedPtr templated on Foo
typedef Foo const& FooCR;
typedef Foo const* FooCP;

//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct Foo : RefCountedBase // Declare the Foo class. Inheriting from RefCountedBase makes it easy to implement the RefCounted pattern
{
private:
    Foo() {} // Good: Force use of static Create method by making constructor private
    static BentleyStatus ValidateInput(int);

public:
    static FooPtr Create(int input); // Good: Provide a static "factory" method to construct a new Foo instance
    void DoSomething();
};


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct MyUtilities : NonCopyableClass // Inheriting from NonCopyableClass indicates that MyUtilities should not be instantiated because it just has static helper methods.
{
    static void DoSomethingGoodWithConstReference(FooCR foo);   // Good: if the input will not be changed
    static void DoSomethingGoodWithConstPointer(FooCP foo);     // Good: if the input will not be changed and nullptr is valid for default behavior
    static void DoSomethingBadWithPtr(FooPtr foo);              // Bad: don't pass RefCountedPtr by value
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
FooPtr Foo::Create(int input)
    {
    if (BentleyStatus::SUCCESS != ValidateInput(input))
        {
        // can return nullptr to indicate error condition with Create
        return nullptr;
        }

    // malloc as usual, RefCounterPtr will call free when reference count goes to zero
    return new Foo();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
BentleyStatus sampleRefCountedPtrUsage(int input)
    {
    // "localFooPtr" is the only object with an owning interest in the created Foo object (reference count of 1 if Create is successful)
    FooPtr localFooPtr = Foo::Create(input);    

    // check to make sure Create was successful
    if (!localFooPtr.IsValid())
        return BentleyStatus::ERROR;

    // Note: 'localFooPtr' will maintain its owning interest throughout the scope of the "sampleRefCountedPtrUsage" method regardless of the code in any of the methods called below

    // -> operator overloading makes using "localFooPtr" easy
    localFooPtr->DoSomething();                                         

    // * operator overloading makes passing a Foo reference easy
    MyUtilities::DoSomethingGoodWithConstReference(*localFooPtr);

    // Can "get" underlying pointer
    MyUtilities::DoSomethingGoodWithConstPointer(localFooPtr.get());

    // Will increment refcount to 2 when parameter is copied and decrement when the stack is unwound. This is unnecessary overhead and makes debugging lifecyle issues harder.
    MyUtilities::DoSomethingBadWithPtr(localFooPtr);                    

    // When "localFooPtr" goes out of scope, its destructor will decrement the reference count (to zero) and its memory will be freed
    return BentleyStatus::SUCCESS;
    }

//__PUBLISH_EXTRACT_END__
BentleyStatus Foo::ValidateInput(int) {return BentleyStatus::SUCCESS;}
void Foo::DoSomething() {}
void MyUtilities::DoSomethingGoodWithConstReference(FooCR) {}
void MyUtilities::DoSomethingGoodWithConstPointer(FooCP) {}
void MyUtilities::DoSomethingBadWithPtr(FooPtr) {}
