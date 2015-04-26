/**
 *  @file example.cpp
 *  @brief Example coding demonstrating the usage of EasyDelegate.
 *  @date 4/25/2015
 *  @copyright This software is licensed under the MIT license. Please refer to LICENSE.txt for more
 *	information.
 */

#include <set>              // std::set
#include <unordered_set>    // std::unordered_set
#include <iostream>         // std::cout, std::endl

#include <easydelegate.hpp>

using namespace std;

unsigned int myStaticIntMethod(const char *str, const float &flt, const double &dbl)
{
    cout << "myStaticIntMethod: " << str << "," << flt << "," << dbl << endl;
    return 5;
}

void myStaticVoidMethod(const float &flt, const char *str, const double &dbl)
{
    cout << "myStaticVoidMethod: " << flt << "," << str << "," << dbl << endl;
}

class MyCustomClass
{
    public:
        unsigned int myMemberMethod(const char *str, const float &flt, const double &dbl)
        {
            cout << "MyCustomClass::myMemberMethod: " << str << "," << flt << "," << dbl << endl;
            return 2;
        }
};

int main(int argc, char *argv[])
{
    // You can typedef the actual delegate type to some type that's specific to your project
    typedef EasyDelegate::DelegateSet<unsigned int, const char *, const float &, const double &> MyEventType;
    typedef EasyDelegate::DelegateSet<void, const float &, const char *, const double &> VoidEventType;

    MyEventType myDelegateSet;
    MyCustomClass *myCustomClassInstance = new MyCustomClass();

    // Register both our static function and our member function
    myDelegateSet.insert(myDelegateSet.end(), new MyEventType::StaticDelegateType(myStaticIntMethod));
    myDelegateSet.insert(myDelegateSet.end(), new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod));


    // This form works too.
    myDelegateSet += new MyEventType::StaticDelegateType(myStaticIntMethod);

    // Call the set via .invoke(), ignoring return values
    cout << "------------- CALLING VIA .invoke() ---------------" << endl;
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Call the set via .invoke(), collecting returns into an std::set
    cout << "------------- CALLING VIA .invoke(), Getting Returns ---------------" << endl;
    set<MyEventType::ReturnType> myReturnValues;
    myDelegateSet.invoke(myReturnValues, "Foo", 3.14, 3.14159);

    for (set<MyEventType::ReturnType>::iterator it = myReturnValues.begin(); it != myReturnValues.end(); it++)
        cout << *it << endl;

    // Iterate on our own, calling invoke() for each delegate
    cout << "------- CUSTOM ITERATION --------" << endl;
    for (MyEventType::iterator it = myDelegateSet.begin(); it != myDelegateSet.end(); it++)
        cout << (*it)->invoke("Foo", 3.14, 3.14159) << endl;

    // Remove a static listener function by address
    cout << "-------------- REMOVING STATIC LISTENERS -----------------" << endl;
    myDelegateSet.remove_delegate_procaddress(myStaticIntMethod);
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a member listener function by address
    cout << "-------------- REMOVING MEMBER LISTENERS -----------------" << endl;
    myDelegateSet.push_back(new MyEventType::StaticDelegateType(myStaticIntMethod));
    myDelegateSet.remove_delegate_procaddress(&MyCustomClass::myMemberMethod);

    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a member listener function by this pointer
    std::cout << "-------------- REMOVING MEMBER LISTENERS VIA THIS -----------------" << endl;
    myDelegateSet.push_back(new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod));
    myDelegateSet.remove_delegate_this(myCustomClassInstance);

    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Remove a delegate by it's address
    cout << "-------------- REMOVING DELEGATE VIA ADDRESS -----------------" << endl;
    MyEventType::MemberDelegateType<MyCustomClass> *delegateToRemove = new MyEventType::MemberDelegateType<MyCustomClass>(myCustomClassInstance, &MyCustomClass::myMemberMethod);
    myDelegateSet.push_back(delegateToRemove);

    myDelegateSet.remove_delegate(delegateToRemove, false);
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // delegateToRemove Still Exists
    cout << "---------- Removed Delegate is still usable ------------" << endl;
    delegateToRemove->invoke("Foo", 3.14, 3.14159);

    // Create a cached delegate with the removed member delegate above
    cout << "---------- CACHED DELEGATES ---------------" << endl;
    typedef EasyDelegate::CachedMemberDelegate<MyCustomClass, unsigned int, const char*, const float&, const double&> MyCachedIntMemberDelegateType;
    typedef EasyDelegate::CachedStaticDelegate<void, const float&, const char*, const double&> MyCachedVoidStaticDelegateType;

    // Allocate our delegate types
    MyCachedIntMemberDelegateType *cachedMemberDelegate = new MyCachedIntMemberDelegateType(&MyCustomClass::myMemberMethod, myCustomClassInstance, "Cached", 3.14, 3.14159);
    MyCachedVoidStaticDelegateType *cachedStaticDelegate = new MyCachedVoidStaticDelegateType(myStaticVoidMethod, 8.15f, "Cached", 3.14f);

    // Now store these in a set
    unordered_set<EasyDelegate::GenericCachedDelegate *> delegates;

    delegates.insert(delegates.end(), cachedMemberDelegate);
    delegates.insert(delegates.end(), cachedStaticDelegate);

    // Iterate
    for (unordered_set<EasyDelegate::GenericCachedDelegate *>::iterator it = delegates.begin(); it != delegates.end(); it++)
    {
        cout << "Invoking Delegate " << endl;
        (*it)->generic_dispatch();
    }

    // Comparisons
    MyEventType::StaticDelegateType staticDelegateReference(myStaticIntMethod);
    VoidEventType::StaticDelegateType staticVoidDelegateReference(myStaticVoidMethod);
    MyEventType::MemberDelegateType<MyCustomClass> memberDelegateReference(myCustomClassInstance, &MyCustomClass::myMemberMethod);

    cout << (staticDelegateReference == staticDelegateReference) << endl;
    cout << (staticDelegateReference == memberDelegateReference) << endl;
    cout << (memberDelegateReference == memberDelegateReference) << endl;
    cout << (memberDelegateReference == staticDelegateReference) << endl;

    // Unlike comparisons
    cout << (staticDelegateReference == staticVoidDelegateReference) << endl;
    cout << (memberDelegateReference == staticVoidDelegateReference) << endl;

    // Cleanup
    delete cachedMemberDelegate;
    delete cachedStaticDelegate;
    delete myCustomClassInstance;

    return 0;
}
