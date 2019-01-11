/*
    PTR_TYPE<char> p1;                       
    ASSERT_TRUE( p1.get() == nullptr );             
    p1.reset (new char[10]);                        
    ASSERT_TRUE( p1.get() != nullptr );             
    strcpy (p1.get(), "abcd");                      
    ASSERT_TRUE( 0==strcmp (p1.get(), "abcd") );    
    ASSERT_TRUE( *p1 == 'a' );                      
    *p1 = 'b';                                      
    ASSERT_TRUE( 0==strcmp (p1.get(), "bbcd") );    
    ASSERT_TRUE( *p1 == 'b' );                      
                                                    
    PTR_TYPE<char> p2 (std::move(p1));       
    ASSERT_TRUE( p1.get() == nullptr );             
    ASSERT_TRUE( p2.get() != nullptr );             
    ASSERT_TRUE( 0==strcmp (p2.get(), "bbcd") );    
                                                    
    p2.reset();                                     
    ASSERT_TRUE( p2.get() == nullptr );             
 */  
    PTR_TYPE<MyStruct> ps1 (new MyStruct(1));
    PTR_TYPE<MyStruct> ps2 (new MyStruct(2));
    ASSERT_TRUE( ps1->GetValue() == 1 );
    ASSERT_TRUE( ps2->GetValue() == 2 );
    ps1 = std::move (ps2);
    ASSERT_TRUE( ps2.get() == nullptr );
    ASSERT_TRUE( ps1.get() != nullptr );
    ASSERT_TRUE( ps1->GetValue() == 2 );

    ASSERT_TRUE( MyBase::s_instanceCount == 1 );
    ps1.reset();
    ASSERT_TRUE( MyBase::s_instanceCount == 0 );

    PTR_TYPE<MyBase> pb1;
    PTR_TYPE<MyStruct> ps22 (new MyStruct(22));
    pb1 = std::move (ps22);

    pb1.reset();
    ASSERT_TRUE( MyBase::s_instanceCount == 0 );
