//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/tst/PTRTest/main.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

struct Dummy : public HFCShareableObject
    {
    Dummy(long pi_data) : data(pi_data) { }
    long data;
    };

void Func1(char* txt, HFCPtr<Dummy> ptr)
    {
    printf("\n%s, value %d, Ref count %d", txt, ptr->data, ptr->DebugGetRefCount());
    }

int main(int argc, char** argv)
    {
    HFCPtr<Dummy> ptr(new Dummy(123));
    Func1("ptr", ptr);
    HFCPtr<Dummy> ptr2(ptr);
    Func1("ptr after ptr2 creation", ptr);
        {
        HFCPtr<Dummy> ptr3;
        Func1("ptr2", ptr2);
        Func1("ptr3 after ptr3=ptr2", ptr3 = ptr2);
        }
    Func1("ptr2 after killing ptr3", ptr2);
    ptr2 = new Dummy(234);
    Func1("ptr after ptr2 = new object", ptr);
    Func1("ptr2", ptr2);
    getchar();
    return 0;
    }