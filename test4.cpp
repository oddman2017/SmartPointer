//  smart pointer test program  ----------------------------------------------//

#include "smart_ptr.h"
using namespace smart_ptr;

#if defined(_MSC_VER)

# pragma warning(disable: 4786)  // identifier truncated in debug info
# pragma warning(disable: 4710)  // function not inlined
# pragma warning(disable: 4711)  // function selected for automatic inline expansion
# pragma warning(disable: 4514)  // unreferenced inline removed

#if (_MSC_VER >= 1310)
# pragma warning(disable: 4675)  // resolved overload found with Koenig lookup
#endif

#endif

#ifdef __BORLANDC__
# pragma warn -8092 // template argument passed to 'find' is not an iterator
#endif

#include <iostream>
#include <set>
#include <string.h>
#include <assert.h>

#define ASSERT assert

template<class T>
void ck( const T* v1, T v2 ) { ASSERT( *v1 == v2 ); }

namespace {
    int UDT_use_count;  // independent of pointer maintained counts
}

//  user defined type  -------------------------------------------------------//

class UDT {
    long value_;
public:
    explicit UDT( long value=0 ) : value_(value) { ++UDT_use_count; }
    ~UDT() {
        --UDT_use_count;
        std::cout << "UDT with value " << value_ << " being destroyed\n";
    }
    long value() const { return value_; }
    void value( long v ) { value_ = v;; }
};  // UDT

//  This isn't a very systematic test; it just hits some of the basics.

void test()
{
    ASSERT( UDT_use_count == 0 );  // reality check

    //  test strong_ptr with a built-in type
    int * ip = new int(500);
    strong_ptr<int> cp ( ip );
    ASSERT( ip == cp.get() );
    ASSERT( cp.use_count() == 1 );

    *cp = 54321;
    ASSERT( *cp == 54321 );
    ASSERT( *ip == 54321 );
    ck( static_cast<int*>(cp.get()), 54321 );
    ck( static_cast<int*>(ip), *cp );

    strong_ptr<int> cp2 ( cp );
    ASSERT( ip == cp2.get() );
    ASSERT( cp.use_count() == 2 );
    ASSERT( cp2.use_count() == 2 );

    ASSERT( *cp == 54321 );
    ASSERT( *cp2 == 54321 );
    ck( static_cast<int*>(cp2.get()), 54321 );
    ck( static_cast<int*>(ip), *cp2 );

    strong_ptr<int> cp3 ( cp );
    ASSERT( cp.use_count() == 3 );
    ASSERT( cp2.use_count() == 3 );
    ASSERT( cp3.use_count() == 3 );
    cp.reset();
    ASSERT( cp2.use_count() == 2 );
    ASSERT( cp3.use_count() == 2 );
    cp.reset( new int(430) );
    *cp =  98765;
    ASSERT( *cp == 98765 );
    *cp3 = 87654;
    ASSERT( *cp3 == 87654 );
    ASSERT( *cp2 == 87654 );
    cp.swap( cp3 );
    ASSERT( *cp == 87654 );
    ASSERT( *cp2 == 87654 );
    ASSERT( *cp3 == 98765 );
    cp.swap( cp3 );
    ASSERT( *cp == 98765 );
    ASSERT( *cp2 == 87654 );
    ASSERT( *cp3 == 87654 );
    cp2 = cp2;
    ASSERT( cp2.use_count() == 2 );
    ASSERT( *cp2 == 87654 );
    cp = cp2;
    ASSERT( cp2.use_count() == 3 );
    ASSERT( *cp2 == 87654 );
    ASSERT( cp.use_count() == 3 );
    ASSERT( *cp == 87654 );

    strong_ptr<int> cp4;
    std::swap( cp2, cp4 );
    ASSERT( cp4.use_count() == 3 );
    ASSERT( *cp4 == 87654 );
    ASSERT( cp2.get() == 0 );

    std::set< strong_ptr<int> > scp;
    scp.insert(cp4);
    ASSERT( scp.find(cp4) != scp.end() );
    ASSERT( scp.find(cp4) == scp.find( strong_ptr<int>(cp4) ) );

    ASSERT( UDT_use_count == 0 );  // reality check

    //  test strong_ptr with a user defined type
    UDT * up = new UDT;
    strong_ptr<UDT> sup ( up );
    ASSERT( up == sup.get() );
    ASSERT( sup.use_count() == 1 );

    sup->value( 54321 ) ;
    ASSERT( sup->value() == 54321 );
    ASSERT( up->value() == 54321 );

    strong_ptr<UDT> sup2;
    sup2 = sup;
    ASSERT( sup2->value() == 54321 );
    ASSERT( sup.use_count() == 2 );
    ASSERT( sup2.use_count() == 2 );
    sup2 = sup2;
    ASSERT( sup2->value() == 54321 );
    ASSERT( sup.use_count() == 2 );
    ASSERT( sup2.use_count() == 2 );

    weak_ptr<UDT> wpUdt(sup2);
    ASSERT( sup2.use_count() == 2 );
    strong_ptr<UDT> spRecover(wpUdt.lock());
    ASSERT( sup2.use_count() == 3 );

    spRecover->value(6666);

    int dummy = 786;
    {
        strong_ptr<UDT> sp5 = make_strong_ptr<UDT>::generate(dummy);
        std::cout << sp5->value() << std::endl;
    }

    dummy = 341;
    strong_ptr<int> sp6 = make_strong_ptr<int>::generate(dummy);
    std::cout << *sp6 << std::endl;

    std::cout << "OK\n";

    new char[12345]; // deliberate memory leak to verify leaks detected
}

void test2(void)
{
    strong_ptr<int> p1(new int(5));
    weak_ptr<int> wp1 = p1; //p1 owns the memory.

    {
        strong_ptr<int> p2 = wp1.lock(); //Now p1 and p2 own the memory.
        if (p2) { //Always check to see if the memory still exists
            //Do something with p2
            std::cout << *p2 << std::endl;
        }
    } //p2 is destroyed. Memory is owned by p1.

    p1.reset(); //Memory is deleted.

    strong_ptr<int> p3 = wp1.lock(); //Memory is gone, so we get an empty strong_ptr.
    if (p3) {
        //Will not execute this.
        std::cout << *p3 << std::endl;
    } else {
        std::cout << "Oops, the object is destroyed." << std::endl;
    }
}

void test4(void)
{
    // test the auto array.
    strong_array<int> sa1(new int[8]);
    sa1[0] = 30;
    sa1[4] = 7;
    strong_array<UDT> sa2(new UDT[8]);
}


#ifndef CDECL
#if defined(WIN32)
#define CDECL           _cdecl
#else
#define CDECL 
#endif // defined(WIN32)
#endif // !CDECL

int CDECL main()
{
    test();
    test2();
    test4();
    return 0;
}
