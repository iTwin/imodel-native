/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Cpp_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeThread.h>
#include <vector>
#include <memory>
#include <complex>
#include <functional>
#include <random>
#include <regex>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include "shared_mutex.h"

using namespace std::placeholders; //for _1, _2, _3...

#define REPORT_MISSING_FEATURE(F) void test_##F () {BeTest::Log ("TestRunner", BeTest::PRIORITY_ERROR, "C++11 test - This platform does not support: " #F);}

#if (BENTLEY_CPLUSPLUS <= 199711L)
    #pragma message ("***")
    #pragma message ("*** Cpp_Test.cpp skipped -- this toolchain supports no C++11 features.")
    #pragma message ("***")
#else

#if defined (__clang__)
    #define IS_CLANG    1
#elif defined (__GNUC__)
    #define IS_GCC      1
#elif defined (_MSC_VER)
    // See http://msdn.microsoft.com/en-us/library/vstudio/hh567368.aspx for which features are supported
    #if (_MSC_VER >= 2000)
        #error Unkown compiler
    #elif (_MSC_VER >= 1900)
        #define IS_VC14 1
    #elif (_MSC_VER >= 1800)
        #define IS_VC12 1
    #else
        #error Unkown compiler
    #endif
#else
    #error Unkown compiler
#endif

    struct Object
        {
        float first;
        int second;
        };

//
// Note: some of these "tests" are just attempts to use a given feature. In many cases, we are just checking that the compiler will accept the code.
//      Some of these tests are copied from http://www.stroustrup.com/C++11FAQ.html
//      Some are copied or adapted from examples on http://www.cplusplus.com

//  -------------------------------------------------------------------------------------
//  std::chrono::high_resolution_clock
//  -------------------------------------------------------------------------------------
// Sam: We’ve had too many problems with it to waste any more time.
// static void test_high_resolution_clock()
//     {
//     std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
//     BeThreadUtilities::BeSleep(1);
//     std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
//     ASSERT_GT( t2 , t1 );
//     std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
//     ASSERT_GE( time_span , std::chrono::milliseconds(1) );
//     }

//  --------------------------------------------------------------------------------------
//  hhinnant shared_mutex http://home.roadrunner.com/~hinnant/
//  --------------------------------------------------------------------------------------
static int                  g_rw_value;
static ting::shared_mutex   g_rw_value_mutex;
static bool                 g_rw_stop;
static int                  g_rw_timesRead;
static std::mutex           g_rw_timesRead_mutex;

static void test_shared_mutex_reader()
    {
    while (!g_rw_stop)
        {
        //  Get shared access to value
        ting::shared_lock<ting::shared_mutex> lockvalue (g_rw_value_mutex);  // calls g_rw_value_mutex.lock_shared(), which blocks until current writer is out and then gets shared access

        // *** NEEDS WORK: I have no way of knowing if both readers have entered concurrently.

        ASSERT_NE( g_rw_value, 1 ) << L"Reader should never see writer's work in progress";

        // Indicate that I got into g_rw_value_mutex
        std::lock_guard<std::mutex> lock(g_rw_timesRead_mutex);
        g_rw_timesRead++;

//        std::hash<std::thread::id> hasher;
//        size_t hashedThreadId = hasher (std::this_thread::get_id());
//        BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, Utf8PrintfString("%llx test_shared_mutex_reader.", hashedThreadId).c_str());
        }
    }

static void test_shared_mutex_unconditional_writer()
    {
    for (int i=0; i<5; ++i)
        {
        BeThreadUtilities::BeSleep(1);   // should permit reads to run, and they should be able to acquire g_rw_value_mutex

        //  Get exclusive access to value
        std::unique_lock<ting::shared_mutex> lockvalue (g_rw_value_mutex); // calls g_rw_value_mutex.lock(), which blocks until all others are out and then gets write access
        g_rw_value = 1; // indicates that I'm doing something
        BeThreadUtilities::BeSleep(1);   // should permit reads to run ... but they should not be able to acquire g_rw_value_mutex
        g_rw_value = 0; // indicates that my work is done

//        std::hash<std::thread::id> hasher;
//        size_t hashedThreadId = hasher (std::this_thread::get_id());
//        BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, Utf8PrintfString("%llx test_shared_mutex_unconditional_writer.", hashedThreadId).c_str());
        }
    }

    /*
static void test_shared_mutex_maybe_writer()
    {
    if (g_rw_value_mutex.try_lock()) // if there is no other readers or writer, gets write access. otherwise, it fails. Does not block.
        {
        }
    // another approach is to use std::unique_lock and pass in a timeout
    }
    */

static void test_shared_mutex()
    {
    g_rw_value = 0;
    g_rw_timesRead = 0;

    std::thread w1(test_shared_mutex_unconditional_writer);
    std::thread r1(test_shared_mutex_reader);
    std::thread r2(test_shared_mutex_reader);
    //  Wait for the writer to finish.
    w1.join();  // !!! "...the programmer must ensure that the destructor [on a thread] is never executed while the thread is still joinable." (http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/2008/n2802.html)
    // Tell the readers to stop looping
    g_rw_stop = true;   // C++11 guarantees atomic assignment (I think!)
    //  Wait for readers to exit
    r1.join();
    r2.join();

    if (g_rw_timesRead < 2)
        {
        BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, "test_shared_mutex - Readers never got into the shared mutex, so this test is worthless.");
        }
    }

//  --------------------------------------------------------------------------------------
//  std::thread, std::mutex
//      example adapted from http://en.cppreference.com/w/cpp/thread/lock_guard
//  --------------------------------------------------------------------------------------
int         g_thr_i = 0;
std::mutex  g_thr_i_mutex;  // protects g_thr_i
 
void test_thread_safe_increment()
    {
    std::lock_guard<std::mutex> lock(g_thr_i_mutex);
    ++g_thr_i;
    }
 
static void test_thread()
    {
    std::thread t1(test_thread_safe_increment);
    std::thread t2(test_thread_safe_increment);
 
    t1.join();
    t2.join();

    ASSERT_EQ( g_thr_i, 2 );
    }

static void test_thread_sleep_for()
    {
    uint64_t t0 = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    BeThreadUtilities::BeSleep(100);
    ASSERT_TRUE ((BeTimeUtilities::GetCurrentTimeAsUnixMillis() - t0) >= 99);
    }

//  --------------------------------------------------------------------------------------
//  std::condition_variable
//  --------------------------------------------------------------------------------------
std::mutex              g_cv_m;
std::condition_variable g_cv_cv;
std::string             g_cv_data;
bool                    g_cv_ready = false;
bool                    g_cv_processed = false;
 
static void test_condition_variable_worker_thread()
    {
    // Wait until main() sends data
        {
        std::unique_lock<std::mutex> lk(g_cv_m);
        g_cv_cv.wait(lk, []{return g_cv_ready;});
        }
 
    BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, "Worker thread is processing data");
    g_cv_data += " after processing";
 
    // Send data back to main()
        {
        std::lock_guard<std::mutex> lk(g_cv_m);
        g_cv_processed = true;
        BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, "Worker thread signals data processing completed");
        }
    g_cv_cv.notify_one();
    }
 
static void test_condition_variable()
    {
    std::thread worker(test_condition_variable_worker_thread);
 
    g_cv_data = "Example data";
    // send data to the worker thread
        {
        std::lock_guard<std::mutex> lk(g_cv_m);
        g_cv_ready = true;
        BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, "main() signals data ready for processing");
        }
    g_cv_cv.notify_one();
 
    // wait for the worker
        {
        std::unique_lock<std::mutex> lk(g_cv_m);
        g_cv_cv.wait(lk, []{return g_cv_processed;});
        }
    BeTest::Log ("TestRunner", BeTest::PRIORITY_INFO, std::string("Back in main(), data = ").append(g_cv_data).c_str());
 
    worker.join();

    /* should log this:
main() signals data ready for processing
Worker thread is processing data
Worker thread signals data processing completed
Back in main(), data = Example data after processing
    */
    }

//  ---------------------------------------------------------------------
//  std::regex
//---------------------------------------------------------------------------------------
static void test_regex()
    {
    char const* str = "2012-12-05T23:44:59.500Z";

    // This regex is from DateTimeStringConverter::FromIso8601()
    char const* regexPattern = "([+-]?[\\d]{4})" //Group 1: Year (Group 0 is entire match)
        "-?" //date component delimiter '-' is optional
        "(1[0-2]|0[1-9])" //Group 2: Month
        "-?" //date component delimiter '-' is optional
        "(3[0-1]|0[1-9]|[1-2][\\d])" //Group 3: Day
        "(?:"                          //Non-capture group: Entire time component
        "T"
        "(2[0-3]|[0-1][\\d])" //Group 4: Hour
        ":?" //time component delimiter ':' is optional
        "([0-5][\\d])" //Group 5: Minute
        "(?:" //non-capture group: Second component is optional
        ":?" //time component delimiter ':' is optional
        "([0-5][\\d])" //Group 6: Second
        "(\\.[\\d]+)?" //Group 7: Second fraction
        ")?"
        "(Z|[+-](?:2[0-3]|[0-1][\\d]):?[0-5][\\d])?" //Group 8: time zone (time zone delimiter ':' is optional)
        ")?";
    
    bool hasMatches = false;

    try {
        const std::regex isoRegex (regexPattern);
        std::match_results<char const*> matches;
        hasMatches = std::regex_search (str, matches, isoRegex);
        }
    catch (std::regex_error& e)
        {
#define CASE_STR(ID)   case ID : str = #ID; break;
        char const* str;
        switch (e.code())
            {
            CASE_STR (std::regex_constants::error_collate	)
            CASE_STR (std::regex_constants::error_ctype	    )
            CASE_STR (std::regex_constants::error_escape	)
            CASE_STR (std::regex_constants::error_backref	)
            CASE_STR (std::regex_constants::error_brack	    )
            CASE_STR (std::regex_constants::error_paren	    )
            CASE_STR (std::regex_constants::error_brace	    )
            CASE_STR (std::regex_constants::error_badbrace	)
            CASE_STR (std::regex_constants::error_range	    )
            CASE_STR (std::regex_constants::error_space	    )
            CASE_STR (std::regex_constants::error_badrepeat	)
            CASE_STR (std::regex_constants::error_complexity)
            CASE_STR (std::regex_constants::error_stack	    )
            default:
                str = "unknown error";
            }

        FAIL() << str;
        }

    ASSERT_TRUE (hasMatches);
    }

//  ---------------------------------------------------------------------
//  std::bind

//  (from http://en.cppreference.com/w/cpp/utility/functional/)

void f(int n1, int n2, int n3, const int& n4, int n5)
    {
    printf ("%d %d %d %d %d\n", n1, n2, n3, n4, n5);
    }
 
int g(int n1)
    {
    return n1;
    }
 
struct Foo 
    {
    void print_sum(int n1, int n2)
        {
        printf ("%d\n", n1+n2);
        }
    int data;// = 10;
    Foo() : data(10) {;}
    };
 

void test_std_bind ()
    {
    // demonstrates argument reordering and pass-by-reference
    int n = 7;
    auto f1 = std::bind(f, _2, _1, 42, std::cref(n), n);
    n = 10;
    f1(1, 2, 1001); // 1 is bound by _2, 2 is bound by _1, 1001 is unused
 
    // nested bind subexpressions share the placeholders
    auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5);
    f2(10, 11, 12);
 
    // bind to a member function
    Foo foo;
    auto f3 = std::bind(&Foo::print_sum, foo, 95, _1);
    f3(5);
 
    // bind to member data
    auto f4 = std::bind(&Foo::data, _1);
    printf ("%d\n", f4(foo));

    /* Output:
2 1 42 10 7
12 12 12 4 5
1 5 0 2 0 8 2 2 10 8
100
10
    */
    }

void test_random ()
    {
    // common use case: binding a RNG with a distribution
    std::default_random_engine e;
    std::uniform_int_distribution<> d(0, 10);
    std::function<int()> rnd = std::bind(d, e);
    for(int n=0; n<10; ++n)
        printf ("%d ", rnd());
    printf ("\n");
    }

//  ---------------------------------------------------------------------
//  std::shared_ptr
void test_shared_ptr ()
    {
	std::shared_ptr<int> p1(new int);	// count is 1
    ASSERT_TRUE( p1.use_count() == 1 );
    if (true)
	    {
		std::shared_ptr<int> p2(p1);	// count is 2
        ASSERT_TRUE( p1.use_count() == 2 );
        ASSERT_TRUE( p2.use_count() == 2 );
        if (true)
		    {
			std::shared_ptr<int> p3(p1);	// count is 3
            ASSERT_TRUE( *p3 == *p1 );
            ASSERT_TRUE( p1.use_count() == 3 );
            ASSERT_TRUE( p2.use_count() == 3 );
            ASSERT_TRUE( p3.use_count() == 3 );
		    }	// count goes back down to 2
        ASSERT_TRUE( p1.use_count() == 2 );
        ASSERT_TRUE( p2.use_count() == 2 );
	    } // count goes back down to 1
    ASSERT_TRUE( p1.use_count() == 1 );

    std::shared_ptr<int> spi (new int);
    *spi = 1;
    ASSERT_TRUE (*spi == 1);

    if (true)
        {
        ASSERT_TRUE (spi.unique ());
        std::shared_ptr<int> spi2 = spi;
        ASSERT_FALSE (spi.unique ());
        }

    if (true)
        {
        std::shared_ptr<int> spi = std::make_shared<int> (123);
        ASSERT_TRUE (*spi == 123);
        ASSERT_TRUE (spi.unique ());
        }
    }

void test_shared_ptr_in_collections ()
    {
    bpair<std::shared_ptr<int>, int> pair1 (std::make_shared<int> (1), 1);
    bpair<int, std::shared_ptr<int>> pair2 (1, std::make_shared<int> (1));

    bpair<int, std::shared_ptr<int>> pair3;
    pair3.first = 1;
    pair3.second = std::make_shared<int> (1);

    bvector<bpair<int, std::shared_ptr<int>>> vector1;
    vector1.push_back (pair2);
    vector1.push_back (pair3);

    bvector<std::shared_ptr<int>> vector2;
    vector2.push_back (std::make_shared<int> (1));
    vector2.push_back (pair1.first);
    }

//  ---------------------------------------------------------------------
// enum class
enum class EnumClassColor;  // forward declare enum class
extern void somefunction (int a, EnumClassColor b);

enum class StrongTypeEnumeration 
    {
    Val1,
    Val2,
    Val3 = 100,
    Val4 // = 101
    };

enum TraditionalEnumAlert { green, yellow, election, red }; // traditional enum
enum class EnumClassColor { red, blue }; // scoped and strongly typed enum, no export of enumerator names into enclosing scope, no implicit conversion to int

void test_enumClass ()
    {
    EnumClassColor c = EnumClassColor::blue;
    ASSERT_TRUE( c == EnumClassColor::blue );
    int a3 = TraditionalEnumAlert::red;     // ok in C++11, error in C++98
    ASSERT_TRUE( a3 == 3 );
    //int a4 = blue;                        // error: blue not in scope
    //int a5 = EnumClassColor::blue;        // error: not Color->int conversion

    StrongTypeEnumeration e = StrongTypeEnumeration::Val4;
    ASSERT_TRUE( e == StrongTypeEnumeration::Val4 );
    //ASSERT_TRUE( e == 101 );        //should not compile!
    }

//  ---------------------------------------------------------------------
// rvalue references
template<class T> 
void testswap(T& a, T& b)	// "perfect swap" (almost)
{
    T tmp = std::move(a);	// could invalidate a
    a = std::move(b);		// could invalidate b
    b = std::move(tmp);		// could invalidate tmp
}

void test_rvalueReferences ()
    {
    int i=1, j=2;
    testswap (i, j);
    printf ("i=%d, j=%d\n", i, j);
    }

//  ---------------------------------------------------------------------
// constexpr
#if IS_VC12
    REPORT_MISSING_FEATURE(Constexpr)
#else
    constexpr int get_five() {return 5;}
    int some_value[get_five() + 7];
    void test_Constexpr() {;}
#endif

//  ---------------------------------------------------------------------
// initializer lists
Object scalar = {0.43f, 10};
Object anArray[] = {{13.4f, 3}, {43.28f, 29}, {5.934f, 17}};
std::vector<int> vv = {1,2,3};

struct SequenceClass 
    {
    SequenceClass(std::initializer_list<int> list)
        {
        BeAssert( std::find (list.begin(), list.end(), 1) != list.end() );
        }
    };

void function_name(std::initializer_list<float> list)
    {
    ASSERT_TRUE( std::find (list.begin(), list.end(), 1.0f) != list.end() );
    } 

void test_initiailzerLists ()
    {
    SequenceClass some_var = {1, 4, 5, 6};
    function_name ({1.0f, -3.45f, -0.4f});

    Object o = {0.43f, 10};
    Object oArray[] = {{13.4f, 3}, {43.28f, 29}, {5.934f, 17}};
    std::vector<int> vec = {1,2,3};

    UNUSED_VARIABLE(o);
    UNUSED_VARIABLE(oArray);
    UNUSED_VARIABLE(vec);
    }

//  ---------------------------------------------------------------------
// uniform initialization (part I)
struct YourBasicStruct
    {    
    int x;    
    double y; 
    };

struct AClassWithCtor 
    {
    private:
    int     m_x;
    double  m_y;
    public:
    AClassWithCtor(int x, double y) : m_x(x), m_y(y) {;} 
    int     GetX() {return m_x;}
    double  GetY() {return m_y;}
    };

void test_uniformInitializationSyntax ()
    {
    YourBasicStruct b {1,1.1};
    ASSERT_TRUE( b.x == 1 && b.y == 1.1 );
    AClassWithCtor  c {1,1.1};
    ASSERT_TRUE( c.GetX()==1 && c.GetY()==1.1 );
    }

//  ---------------------------------------------------------------------
// Constructor improvements: delegation
struct SomeType  
    {
    int number;
    SomeType(int new_number) : number(new_number) {}
    SomeType() : SomeType(42) {}
    };
void test_delegatingConstructors(){};

//  ---------------------------------------------------------------------
// Constructor improvements: inheriting
#if IS_VC12
    REPORT_MISSING_FEATURE(inheritingConstructors)
#else
    struct BaseClass 
        {
        BaseClass(int value){}
        };
 
    struct DerivedClass : BaseClass 
        {
        using BaseClass::BaseClass;
        };

    static DerivedClass dd (1);
    void test_inheritingConstructors(){};
#endif

//  ---------------------------------------------------------------------
// Constructor improvements: default member initialization
struct SomeClassWithDefaultMemberInitialization 
    {
    SomeClassWithDefaultMemberInitialization() {}
    SomeClassWithDefaultMemberInitialization(int v) : value2(v) {}  // constructor takes precedence over default
    int value = 5;
    int value2= 2;
    };

void test_defaultMemberInitialization ()
    {
    SomeClassWithDefaultMemberInitialization sdmi;
    ASSERT_TRUE( sdmi.value == 5 );
    }

//  ---------------------------------------------------------------------
// nullptr
void* p = nullptr;

//  ---------------------------------------------------------------------
// override
struct Base             {virtual void Function (int x) {;}};
struct Derived  : Base  {virtual void Function (int x) override {;}};
//struct Derived2 : Base  {virtual void Function (double x) override {;}};  //Should not compile!
void test_override ()
    {
    }

//  ---------------------------------------------------------------------
// final
struct NoSubclassing final
    {
    int member;
    };
void test_final() {;}

//  ---------------------------------------------------------------------
//  explicit conversion operator
struct MyString
    {
    std::string str;
    MyString (char const* s) : str(s) {;}
    explicit operator const char *() const {return str.c_str();}
    };

void test_explicitConversionOperator ()
    {
    MyString mystring ("abc");
    //ASSERT_TRUE( 0==strcmp("abc", mystring) );   //should not compile!
    ASSERT_TRUE( 0==strcmp("abc", (char const*)mystring) );
    }

//  ---------------------------------------------------------------------
// Unrestricted unions
#if IS_VC12
    REPORT_MISSING_FEATURE(unrestrictedUnions)
#else
    #include <new> // Required for placement 'new'.

    struct Point 
        {
        Point() {}
        Point(int x, int y): x_(x), y_(y) {}
        int x_, y_;
        };

    union U 
        {
        int     z;
        double  w;
        Point   p; // Illegal in C++03; legal in C++11.
        //
        // Due to the Point member, a constructor definition is now required.
        //
        U() {new(&p) Point();} 
        };

    void test_unrestrictedUnions ()
        {
        Point pnt;
        }
#endif

//  ---------------------------------------------------------------------
// Control and query object alignment
#if IS_GCC || IS_CLANG || IS_VC12 || IS_VC14
    REPORT_MISSING_FEATURE(alignas)
#else
    struct AlignAsTest
        {
                        char    m_byte;
        alignas(char)   double  m_double;
        };

    void test_alignas ()
        {
        AlignAsTest aat;
        ASSERT_TRUE( (char*)&aat.m_double == &aat.m_byte+1 );
        }
#endif

//  ---------------------------------------------------------------------
// double literals
#if IS_GCC || IS_VC12 || IS_VC14
    REPORT_MISSING_FEATURE(doubleLiterals)
#else
    std::complex<long double> operator "" i(long double d)   // imaginary literal
    {
        return {0,d};	// complex is a literal type
    }

    void test_doubleLiterals ()
        {
            /* WIP
        auto z = 2 + 1i;
            printf ("%lf,%lf\n", z.real(), z.imag());
             */
        }
#endif

//  ---------------------------------------------------------------------
// u8 string literals
#if IS_VC12
    REPORT_MISSING_FEATURE(u8StringLiterals)
#else
    void test_u8StringLiterals ()
        {
        Utf8CP  utf8    = u8"This is a Unicode Character: \u2018.";
        /*Utf16CP*/char16_t const* utf16   = u"This is a bigger Unicode Character: \u2018.";
        /*WCharCP*/char32_t const* utf32   = U"This is a Unicode Character: \u2018.";
        // *** TBD: tests
        
	printf ("%s\n", utf8);
	if (utf16 == NULL)
		printf ("why?");
	if (utf32 == NULL)
		printf ("why?");
	}
#endif

//  ---------------------------------------------------------------------
// raw string literals
void test_rawStringLiterals ()
    {
    char const* c1 = R"(The String Data \ Stuff " )";
    printf ("%s\n", c1);
    char const* c2 = R"delimiter(The String Data \ Stuff " )delimiter";
    printf ("%s\n", c2);
    }

//  ---------------------------------------------------------------------
// auto
void test_auto ()
    {
    int i;
    int* p = &i;

    auto p2 = p;
    ASSERT_TRUE( p2 == p );

    decltype(p) p3 = p;
    ASSERT_TRUE( p3 == p );
    }

//  ---------------------------------------------------------------------
// range-based for loop
void test_rangeBasedFor ()
    {
    Object anArray[] = {{13.4f, 3}, {43.28f, 29}, {5.934f, 17}};
    size_t i=0;
    for (auto o : anArray)
        {
        ASSERT_TRUE( o.first == anArray[i].first );
        ++i;
        }
    ASSERT_TRUE(i==_countof(anArray));

    bvector<int> bv;
    bv.push_back(1);
    bv.push_back(2);
    i=0;
    for (auto v : bv)
        {
        ASSERT_TRUE( v == bv[i] );
        ++i;
        }
    ASSERT_TRUE(i==bv.size());
    }

//  ---------------------------------------------------------------------
// suffix return type syntax
template<class T, class U>
auto mul(T x, U y) -> decltype(x*y)
{
    return x*y;
}
        
struct SuffixTestList {
    struct Link { /* ... */ };
    Link* erase(Link* p);	// remove p and return the link before p
    // ...
};

auto SuffixTestList::erase(Link* p) -> Link* { return NULL;/* ... */ }        

void test_suffix_return_type_syntax ()
    {
    ASSERT_TRUE (mul (2,4) == 8);
    ASSERT_TRUE (mul (2.0,3.0) == 6.0);
    }

//  ---------------------------------------------------------------------
//  lambda
void test_lambda ()
    {
    bvector<int> bv;
    bv.push_back(1);
    bv.push_back(2);
    bv.push_back(3);
    bvector<int>::iterator found = std::find_if (bv.begin(), bv.end(), 
        [](int v) { return v==2; }
        );
    ASSERT_TRUE( found != bv.end() && *found==2 );
    }

void CallStdFunction (std::function<int (double)> func, double arg, int expectedResult)
    {
    int actualResult = func (arg);
    ASSERT_EQ (expectedResult, actualResult);
    }

void test_stdfunction ()
    {
    auto signFunc = [] (double arg) 
        {
        return arg >= 0 ? 1 : -1;
        };

    CallStdFunction (signFunc, 2.5, 1);
    CallStdFunction (signFunc, -2.5, -1);
    }

//  ---------------------------------------------------------------------
// std::unique_ptr (supercedes auto_ptr)
void test_unique_ptr()
    {
    std::unique_ptr<int> upi (new int);
    *upi = 1;
    ASSERT_TRUE( *upi == 1 );
    //std::unique_ptr<int> upi2 = upi;
    std::unique_ptr<int> upi2 = std::move(upi);
    ASSERT_TRUE( *upi2 == 1 );
    ASSERT_TRUE( upi.get() == NULL );
    }

void test_unique_ptr_in_collections ()
    {
    bpair<int, std::unique_ptr<int>> pair1;
    pair1.first = 1;
    pair1.second = std::unique_ptr<int> (new int (1));

    int val = *pair1.second;
    ASSERT_EQ (1, val);

    // Does not compile
    //std::unique_ptr<int> upi (new int (2));
    //bpair<int, std::unique_ptr<int>> pair2 (1, std::move(upi));

    // Does not compile
    //bvector<std::unique_ptr<int>> vector1;
    //vector1.push_back (std::move(std::unique_ptr<int> (new int (1))));

    //bvector<bpair<int, std::unique_ptr<int>>> vector2;
    //vector2.push_back (pair1);
    //bpair<int, std::unique_ptr<int>> pair2 (1, std::move (std::unique_ptr<int> (new int (1))));
    }


//  ---------------------------------------------------------------------
// Right angle bracket
void test_right_angle_bracket()
    {
    bvector<bvector<int>> okrab;
    ASSERT_TRUE( okrab.empty() );
    }

//  ---------------------------------------------------------------------
// long long
void test_long_long ()
    {
    long long x = 9223372036854775807LL;
    int64_t   y = 9223372036854775807LL;
    ASSERT_TRUE (x == y);
    }

//  ---------------------------------------------------------------------
// static_assert
void test_static_assert()
    {
    static_assert (sizeof(int) >= 4, "static assert test");
    static_assert (BENTLEY_CPLUSPLUS >= 201103L, "BENTLEY_CPLUSPLUS");
    }

//  ---------------------------------------------------------------------
// type traits
// N.B. WIP. Not all type traits tested yet. 
void test_type_traits ()
    {
    struct Base {};
    struct Sub : public Base {};
    struct AnotherClass {};

    ASSERT_TRUE ((std::is_convertible<Sub*, Base*>::value));
    ASSERT_FALSE ((std::is_convertible<Base*, Sub*>::value));
    ASSERT_TRUE ((std::is_convertible<Base*, Base*>::value));
    ASSERT_FALSE ((std::is_convertible<AnotherClass*, Base*>::value));


    ASSERT_TRUE ((std::is_base_of<Base, Sub>::value));
    ASSERT_FALSE ((std::is_base_of<Base*, Sub*>::value)); //returns always false for pointers to the classes
    ASSERT_FALSE ((std::is_base_of<Sub, Base>::value));
    ASSERT_TRUE ((std::is_base_of<Base, Base>::value));
    ASSERT_FALSE ((std::is_base_of<Base, AnotherClass>::value));
    }

//  ---------------------------------------------------------------------
// std::unordered_map
void test_unorderedmap ()
    {
    std::unordered_map<int, Utf8String> map;
    map[1] = "Hello1";
    map[2] = "Hello2";

    ASSERT_STREQ ("Hello1", map[1].c_str ());
    ASSERT_STREQ ("Hello2", map[2].c_str ());
    }


//  ---------------------------------------------------------------------
// std::numeric_limits
void test_numeric_limits ()
    {
    ASSERT_EQ ((std::numeric_limits<uint8_t>::min)(), 0);
    ASSERT_EQ ((std::numeric_limits<uint8_t>::max)(), 255);

    ASSERT_EQ ((std::numeric_limits<int8_t>::min)(), -128);
    ASSERT_EQ ((std::numeric_limits<int8_t>::max)(), 127);
    }


TEST(Cpp, LanguageFeatures)
    {
    #if defined (__clang__)
        ASSERT_TRUE( CLANG_VERSION >= 4010 ) << CLANG_VERSION;
    #elif defined (__GNUC__)
        ASSERT_TRUE( GCC_VERSION >= 4060 ) << GCC_VERSION;
    #elif defined (BENTLEY_WIN32)
        ASSERT_TRUE( _MSC_VER >= 1600 );
    #elif defined (BENTLEY_WINRT)
        ASSERT_TRUE( _MSC_VER >= 1700 );
    #else
        FAIL() << L"Unknown compiler";
    #endif

// Sam: We’ve had too many problems with it to waste any more time.
//    test_high_resolution_clock();
    test_shared_mutex ();
    test_thread ();
    test_thread_sleep_for();
    test_condition_variable ();
    test_regex ();
    test_random ();
    test_std_bind ();
    test_shared_ptr ();
    test_shared_ptr_in_collections ();
    test_suffix_return_type_syntax ();
    test_static_assert ();
    test_type_traits ();
    test_long_long ();
    test_right_angle_bracket ();
    test_auto ();
    test_rangeBasedFor ();
    test_lambda ();
    test_stdfunction ();
    test_unique_ptr ();
    test_unique_ptr_in_collections ();
    test_rvalueReferences ();
    test_initiailzerLists ();
    test_uniformInitializationSyntax ();
    test_defaultMemberInitialization ();
    test_enumClass();
    test_explicitConversionOperator ();
    test_unrestrictedUnions ();
    test_doubleLiterals ();
    test_u8StringLiterals ();
    test_rawStringLiterals ();
    test_alignas ();
    test_delegatingConstructors ();
    test_inheritingConstructors ();
    test_Constexpr ();
    test_override ();
    test_final ();
    test_unorderedmap ();
    test_numeric_limits ();
    }

#endif
