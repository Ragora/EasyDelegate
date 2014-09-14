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
    EasyDelegate::DelegateSet<unsigned int, char*, float, double> myDelegateSet;

    // You can typedef the actual delegate type to some type that's specific to your project
    myDelegateSet.push_back(new EasyDelegate::StaticDelegate<unsigned int, char*, float, double>(myStaticMethod));

    MyCustomClass *myCustomClassInstance = new MyCustomClass();
    myDelegateSet.push_back(new EasyDelegate::MemberDelegate<MyCustomClass, unsigned int, char*, float, double>(myCustomClassInstance, &MyCustomClass::myMemberMethod));

    // Call the set via .invoke(), ignoring return values
    std::cout << "------------- CALLING VIA .invoke() ---------------" << std::endl;
    myDelegateSet.invoke("Foo", 3.14, 3.14159);

    // Call the set via .invoke(), collecting returns into an std::vector
    std::cout << "------------- CALLING VIA .invoke(), Getting Returns ---------------" << std::endl;
    std::vector<unsigned int> myReturnValues;
    myDelegateSet.invoke(myReturnValues, "Foo", 3.14, 3.14159);

    for (std::vector<unsigned int>::iterator it = myReturnValues.begin(); it != myReturnValues.end(); it++)
        std::cout << *it << std::endl;

    // Iterate on our own, calling invoke() for each delegate
    std::cout << "------- CUSTOM ITERATION --------" << std::endl;
    for (EasyDelegate::DelegateSet<unsigned int, char*, float, double>::iterator it = myDelegateSet.begin(); it != myDelegateSet.end(); it++)
        std::cout << (*it)->invoke("Foo", 3.14, 3.14159) << std::endl;

    return 0;
}
