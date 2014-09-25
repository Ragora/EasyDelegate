/**
 *  @file ex_delegateset.cpp
 */

#include <iostream>

#include <easydelegate.hpp>

unsigned int myStaticMethod(char *str, float flt, double dbl)
{
    std::cout << "myStaticMethod: " << str << "," << flt << "," << dbl << std::endl;
    return 5;
}

class MyCustomClass
{
    public:
        unsigned int myMemberMethod(char *str, float flt, double dbl)
        {
            std::cout << "MyCustomClass::myMemberMethod: " << str << "," << flt << "," << dbl << std::endl;
            return 2;
        }
};

int main(int argc, char *argv[])
{
    // You can typedef the actual delegate type to some type that's specific to your project
    typedef EasyDelegate::DelegateSet<unsigned int, char*, float, double> MyEventType;

    MyEventType myDelegateSet;
    MyCustomClass *myCustomClassInstance = new MyCustomClass();

    // Register both our static function and our member function
    myDelegateSet.push_back(new MyEventType::StaticDelegateType(myStaticMethod));
    myDelegateSet.push_back(new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod));

    // This form works too.
    myDelegateSet += new MyEventType::StaticDelegateType(myStaticMethod);

    // Call the set via .invoke(), ignoring return values
    std::cout << "------------- CALLING VIA .invoke() ---------------" << std::endl;
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Call the set via .invoke(), collecting returns into an std::vector
    std::cout << "------------- CALLING VIA .invoke(), Getting Returns ---------------" << std::endl;
    std::vector<MyEventType::ReturnType> myReturnValues;
    myDelegateSet.invoke(myReturnValues, "Foo", 3.14, 3.14159);

    for (std::vector<MyEventType::ReturnType>::iterator it = myReturnValues.begin(); it != myReturnValues.end(); it++)
        std::cout << *it << std::endl;

    // Iterate on our own, calling invoke() for each delegate
    std::cout << "------- CUSTOM ITERATION --------" << std::endl;
    for (MyEventType::iterator it = myDelegateSet.begin(); it != myDelegateSet.end(); it++)
        std::cout << (*it)->invoke("Foo", 3.14, 3.14159) << std::endl;

    // Remove a static listener function by address
    std::cout << "-------------- REMOVING STATIC LISTENERS -----------------" << std::endl;
    myDelegateSet.remove_delegate_procaddress(myStaticMethod);
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a member listener function by address
    std::cout << "-------------- REMOVING MEMBER LISTENERS -----------------" << std::endl;
    myDelegateSet.push_back(new MyEventType::StaticDelegateType(myStaticMethod));
    myDelegateSet.remove_delegate_procaddress(&MyCustomClass::myMemberMethod);

    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a member listener function by this pointer
    std::cout << "-------------- REMOVING MEMBER LISTENERS VIA THIS -----------------" << std::endl;
    myDelegateSet.push_back(new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod));
    myDelegateSet.remove_delegate_this(myCustomClassInstance);

    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a delegate by it's address
    std::cout << "-------------- REMOVING DELEGATE VIA ADDRESS -----------------" << std::endl;
    MyEventType::MemberDelegateType<MyCustomClass> *delegateToRemove = new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod);
    myDelegateSet.push_back(delegateToRemove);

    myDelegateSet.remove_delegate(delegateToRemove, false);
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // delegateToRemove Still Exists
    std::cout << "---------- Removed Delegate is still usable ------------" << std::endl;
    delegateToRemove->invoke("Foo", 3.14, 3.14159);

    // Create a cached delegate with the removed member delegate above
    std::cout << "---------- CACHED DELEGATES ---------------" << std::endl;
    typedef EasyDelegate::CachedDelegate<unsigned int, char*, float, double> MyCachedDelegateType;

    MyCachedDelegateType *cachedDelegate = new MyCachedDelegateType(delegateToRemove, "Cached", 3.14, 3.14159);
    cachedDelegate->dispatch(); // Call this at any point in your application to have a deferred function call

    // Cleanup
    delete cachedDelegate; // Also deletes delegateToRemove
    delete myCustomClassInstance;

    return 0;
}
