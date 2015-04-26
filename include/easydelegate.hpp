/**
 *  @file easydelegate.hpp
 *  @version 2.0
 *  @date 4/26/2015
 *  @author <a href="http://dx.no-ip.org">Robert MacGregor</a>
 *  @brief Portable delegate system that should work on any C++11 compliant compiler.
 *  @details EasyDelegate is a library that exposes an easy and flexible delegate system with
 *  the use of C++11's variatic templates.
 *
 *  @example example.cpp
 *  @copyright This software is licensed under the MIT license. Refer to LICENSE.txt for more
 *	information.
 */

#ifndef _INCLUDE_EASYDELEGATE_HPP_
#define _INCLUDE_EASYDELEGATE_HPP_

#include <stdexcept>        // Standard exception types
#include <assert.h>         // assert(expr)
#include <type_traits>      // std::remove_reference<type>

#include <set>              // std::set<type>
#include <tuple>            // std::tuple<...>
#include <unordered_set>    // std::unordered_set<type>

// Define __forceinline if we're on GCC
#if defined(__GNUC__) || defined(__GNUG__)
    #define __forceinline __attribute__((always_inline))
#endif

// If we're going to inline stuff, force it
#ifdef EASYDELEGATE_FORCE_INLINE
    #define INLINE __forceinline
#else
    #define INLINE
#endif

//! Namespace containing all EasyDelegate functionality.
namespace EasyDelegate
{
    using namespace std;

    // Taken from the chosen answer of
    // http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
    template<int ...> struct seq {};
    template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
    template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

    // Taken from the chosen answer of
    // http://stackoverflow.com/questions/12742877/remove-reference-from-stdtuple-members
    template <typename... typenames>
    using NoReferenceTuple = tuple<typename remove_reference<typenames>::type...>;

    /**
     *  @brief A general base exception type for all exceptions that is thrown
     *  by the EasyDelegate library.
     */
    class DelegateException : public exception { };

    /**
     *  @brief An exception type that is thrown by the EasyDelegate library
     *  when a member delegate type has attempted to make a call using a
     *  NULL 'this' pointer.
     */
    class InvalidThisPointerException : public DelegateException
    {
        // Public Methods
        public:
            virtual const char *what() const throw()
            {
                return "Attempted to call member method again a NULL 'this' pointer";
            }
    };

    /**
     *  @brief An exception type that is thrown by the EasyDelegate library when
     *  any delegate type has attempted to make a call against a NULL method
     *  pointer.
     */
    class InvalidMethodPointerException : public DelegateException
    {
        // Public Methods
        public:
            virtual const char *what() const throw()
            {
                return "Attempted to perform a call against a NULL method pointer";
            }
    };

    /**
     *  @brief A type that can represent any delegate type, but it cannot be invoked
     *  without casting to a delegate type that knows the proper function signature.
     *  @details This delegate type is useful in the event you wish to pass
     *  around delegates regardless of the type in a system where the function
     *  signature is resolved via other means for type casting and invocation.
     *
     *  This class type has two booleans readily available that can be used to help
     *  distinguish the actual type at runtime: mIsMemberDelegate and mIsCachedDelegate.
     */
    class GenericDelegate
    {
        // Public Members
        public:
            //! A boolean representing whether or not this delegate is a member delegate.
            const bool mIsMemberDelegate;
            //! A boolean representing whether or not this delegate is a cached delegate.
            const bool mIsCachedDelegate;

        // Protected Methods
        protected:
            /**
             *  @brief Constructor accepting a boolean.
             *  @param isMemberDelegate A boolean representing whether or not this delegate
             *  is an instance of MemberDelegate.
             */
            GenericDelegate(const bool &isMemberDelegate, const bool &isCachedDelegate) noexcept :
            mIsMemberDelegate(isMemberDelegate), mIsCachedDelegate(isCachedDelegate) { }
    };

    /**
     *  @brief Forward declaration for DelegateBase.
     *  @details This is done because the real DelegateBase declaration
     *  at the bottom of this file utilizes typedefs against the StaticDelegate
     *  and MemberDelegate types but at the same time, MemberDelegate and StaticDelegate
     *  derive from DelegateBase to allow storage of both types in the DelegateSet type.
     */
    template <typename returnType, typename... parameters>
    class DelegateBase;

    template <typename classType, typename returnType, typename... parameters>
    class MemberDelegate;

    template <typename returnType, typename... parameters>
    class CachedStaticDelegate;

    /**
     *  @brief A delegate of a static function.
     *  @details The function that is to be made into a delegate ae template.
     */
    template <typename returnType, typename... parameters>
    class StaticDelegate : public DelegateBase<returnType, parameters...>
    {
        public:
            //! Helper typedef referring to a static function pointer that is compatible with this delegate.
            typedef returnType(*StaticDelegateFuncPtr)(parameters...);
            //! Helper typedef referring to a member function pointer.
            template <typename classType>
            using MemberDelegateFuncPtr = returnType(classType::*)(parameters...);

            /**
             *  @brief Constructor accepting a static function.
             *  @param procAddress The static function that this delegate
             *  is intended to invoke.
             */
            StaticDelegate(const StaticDelegateFuncPtr procAddress) : DelegateBase<returnType, parameters...>(false), mProcAddress(procAddress) { }

            /**
             *  @brief Standard copy constructor.
             *  @param other An instance of a StaticDelegate with the same function signature to copy.
             */
            StaticDelegate(const StaticDelegate<returnType, parameters...>& other) : DelegateBase<returnType, parameters...>(false),
            mProcAddress(other.mProcAddress) { }

            /**
             *  @brief Invoke the StaticDelegate.
             *  @param params Anything; It depends on the function signature specified in the template.
             *  @return Anything; It depends on the function signature specified in the template.
             *  @throw EasyDelegate::InvalidMethodPointerException Thrown when the static function that this
             *  delegate is supposed to be invoking is NULL.
             *  @throw std::exception Potentially thrown by the function called by
             *  this StaticDelegate.
             *  @note The call will not throw an exception in any of the std::runtime_error cases but rather
             *  assert if assertions are enabled.
             */
            returnType invoke(parameters... params)
            {
                assert(mProcAddress);

                if (!mProcAddress)
                    throw InvalidMethodPointerException();

                return ((StaticDelegateFuncPtr)this->mProcAddress)(params...);
            }

            /**
             *  @brief Returns whether or not this StaticDelegate calls against the given this pointer.
             *  @param thisPointer A pointer referring to the object of interest.
             *  @return A boolean representing whether or not this StaticDelegate calls a member function
             *  against the given this pointer.
             *  @note Always returns false because StaticDelegates don't use a this pointer.
             */
            const bool has_thispointer(const void *thisPointer) const noexcept { return false; }

            /**
             *  @brief Returns whether or not this StaticDelegate calls the given static proc address.
             *  @param procAddress A pointer to the static function to be checked against.
             *  @return A boolean representing whether or not this StaticDelegate calls the given proc address.
             */
            const bool has_procaddress(const StaticDelegate::StaticDelegateFuncPtr procAddress) const noexcept { return mProcAddress == procAddress; };

            // Comparison against a member delegate type
            bool operator ==(const StaticDelegate<returnType, parameters...> &other) const noexcept { return other.mProcAddress == mProcAddress; }

            template <typename otherClass, typename otherReturn, typename... otherParams>
            bool operator ==(const MemberDelegate<otherClass, otherReturn, otherParams...> &other) const noexcept { return false;  }

            template <typename otherReturn, typename... otherParams>
            bool operator ==(const StaticDelegate<otherReturn, otherParams...> &other) const noexcept { return false; }

        private:
            //! Internal pointer to proc address to be called.
            const StaticDelegateFuncPtr mProcAddress;
    };

    /**
     *  @brief A delegate of a class member method.
     *  @details The method that is to be made into a delegate must have
     *  the class type, return type and argument information specified in the template.
     */
    template <typename classType, typename returnType, typename... parameters>
    class MemberDelegate : public DelegateBase<returnType, parameters...>
    {
        public:
            //! Helper typedef referring to a member function pointer.
            typedef returnType(classType::*MemberDelegateFuncPtr)(parameters...);
            //! Helper typedef referring to a static function pointer.
            typedef returnType(*StaticDelegateFuncPtr)(parameters...);

            /**
             *  @brief Constructor accepting a this pointer and a member function.
             *  @param thisPtr A pointer to the object instance to be considered 'this'
             *  during invocation.
             *  @param procAddress A pointer to the member function to be invoked upon
             *  'this'.
             *  @note The MemberDelegate invocation code has no way of knowing if the 'this'
             *  pointer at any time has been deallocated. The MemberDelegate will cause undefined
             *  behavior and/or segfault upon invocation in that case.
             */
            MemberDelegate(const classType *thisPtr, const MemberDelegateFuncPtr procAddress) : mThisPtr(thisPtr),
            mProcAddress(procAddress), DelegateBase<returnType, parameters...>(true) { }

            /**
             *  @brief Standard copy constructor.
             */
            MemberDelegate(const MemberDelegate<classType, returnType, parameters...>& other) : mThisPtr(other.mThisPtr),
            mProcAddress(other.mProcAddress), DelegateBase<returnType, parameters...>(true) { }

            /**
             *  @brief Invoke the MemberDelegate.
             *  @param params Anything; It depends on the function signature specified in the template.
             *  @return Anything; It depends on the function signature specified in the template.
             *  @throw EasyDelegate::InvalidMethodPointerException Thrown when the static function that this
             *  delegate is supposed to be invoking is NULL.
             *  @throw EasyDelegate::InvalidThisPointerException Thrown when the MemberDelegate's this pointer
             *  is NULL.
             *  @throw std::exception Potentially thrown by the function called by
             *  this MemberDelegate.
             *  @note The call will not throw an exception in any of the std::runtime_error cases but rather
             *  assert if assertions are enabled.
             */
            returnType invoke(parameters... params)
            {
                assert(mThisPtr);
                assert(mProcAddress);

                if (!mThisPtr)
                    throw InvalidThisPointerException();

                if (!mProcAddress)
                    throw InvalidMethodPointerException();

                classType *thisPtr = (classType*)this->mThisPtr;
                return (thisPtr->*mProcAddress)(params...);
            }

            /**
             *  @brief Returns whether or not this MemberDelegate calls against the given this pointer.
             *  @param thisPointer A pointer referring to the object of interest.
             *  @return A boolean representing whether or not this MemberDelegate calls a member function
             *  against the given this pointer.
             */
            const bool has_thispointer(const void *thisPointer) const noexcept { return mThisPtr == thisPointer; }

            /**
             *  @brief Returns whether or not this MemberDelegate calls the given member proc address.
             *  @param procAddress A pointer to a member function to be checked against.
             *  @return A boolean representing whether or not this MemberDelegate calls the given member proc address.
             */
            const bool has_procaddress(const MemberDelegate::MemberDelegateFuncPtr procAddress) const noexcept { return mProcAddress == procAddress; }

            /**
             *  @brief Returns whether or not this MemeberDelegate calls the given static proc address.
             *  @param procAddress A pointer to the static function to be checked against.
             *  @return A boolean representing whether or not this delegate calls the given proc address.
             *  @note Always returns false because a static function cannot be invoked by a MemberDelegate.
             */
            const bool has_procaddress(const MemberDelegate::StaticDelegateFuncPtr funcPtr) const noexcept { return false; }

            // Comparison against a member delegate type
            bool operator ==(const MemberDelegate<classType, returnType, parameters...> &other) const noexcept { return other.mProcAddress == mProcAddress; }

            template <typename otherClass, typename otherReturn, typename... otherParams>
            bool operator ==(const MemberDelegate<otherClass, otherReturn, otherParams...> &other) const noexcept { return false;  }

            template <typename otherReturn, typename... otherParams>
            bool operator ==(const StaticDelegate<otherReturn, otherParams...> &other) const noexcept { return false; }

        private:
            //! An internal pointer to this object.
            const classType *mThisPtr;
            //! An internal pointer to the proc address to be called.
            const MemberDelegateFuncPtr mProcAddress;
    };

    /**
     *  @brief A set of delegate instances that provides helper methods to invoke all
     *  contained delegates.
     *  @details The DelegateSet type can be described as a sink for specific event types.
     *  Typical usage of this behavior would involve creating a typedef of a DelegateSet type
     *  for a specific method signature which then has its own specialized typedefs to facilitate
     *  the creation of StaticDelegate and MemberDelegate types that are compatible with instances
     *  of this new specialized DelegateSet type.
     *  @example ex_events.cpp
     */
    template <typename returnType, typename... parameters>
    class DelegateSet : public set<DelegateBase<returnType, parameters...> *>
    {
        public:
            //! Helper typedef to construct the function pointer signature from the template.
            typedef returnType (*delegateFuncPtr)(parameters...);

        public:
            //! Helper typedef for when building static delegates for this set.
            typedef StaticDelegate<returnType, parameters...> StaticDelegateType;
            //! Helper typedef for when building member delegates for this set.
            template <typename classType>
            using MemberDelegateType = MemberDelegate<classType, returnType, parameters...>;
            //! Helper typedef for when wanting the return type of this set.
            typedef returnType ReturnType;
            //! Helper typedef to construct the StaticDelegate signature from the template.
            typedef DelegateBase<returnType, parameters...> DelegateBaseType;

            //! Helper typedef referring to a static function pointer.
            typedef returnType(*StaticDelegateFuncPtr)(parameters...);
            //! Helper typedef referring to a member function pointer.
            template <typename classType>
            using MemberDelegateFuncPtr = returnType(classType::*)(parameters...);

            void invoke(parameters... params) const
            {
                for (typename set<DelegateSet::DelegateBaseType *>::const_iterator it = this->begin(); it != this->end(); it++)
                    (*it)->invoke(params...);
            }

            /**
             *  @brief Invoke all delegates in the set, storing return values in out.
             *  @param out The std::vector that all return values will be sequentially written to.
             *  @param args All other arguments that will be used as parameters to each delegate.
             *  @throw std::runtime_error Thrown when assertions are disabled and either a stored MemberDelegate
             *  or StaticDelegate type have a NULL function to call.
             *  @throw std::runtime_error Thrown when assertions are disabled and a stored MemberDelegate is
             *  attempting to call against a NULL this pointer.
             *  @throw std::exception Any exception can be potentially thrown by the functions each delegate calls.
             *  @note The call will not throw an exception in any of the std::runtime_error cases but rather
             *  assert if assertions are enabled.
             */
            void invoke(set<returnType> &out, parameters... params) const
            {
                for (typename set<DelegateSet::DelegateBaseType *>::const_iterator it = this->begin(); it != this->end(); it++)
                    out.insert(out.end(), (*it)->invoke(params...));
            }

            /**
             *  @brief Pushes a delegate instance to the end of the set.
             *  @param delegateInstance The delegate instance to the pushed onto the set.
             */
            void push_back(DelegateSet::DelegateBaseType *delegateInstance)
            {
                this->insert(this->end(), delegateInstance);
            }

            //! Alternate to push_back that looks a bit prettier in source.
            void operator +=(DelegateSet::DelegateBaseType *delegateInstance)
            {
                this->push_back(delegateInstance);
            }

            /**
             *  @brief Removes all delegates from the set that have the given static proc address
             *  for it's method.
             *  @param procAddress The static proc address to check against.
             *  @param deleteInstances A boolean representing whether or not all matches should be deleted when removed. The default
             *  for this parameter is true for better memory management.
             *  @param out A pointer to an std::vector templated for the DelegateBase type that this DelegateSet contains. This vector
             *  will then be populated with a list of removed delegates. If deleteInstances is false, this list will never be populated.
             *  @warning If deleteInstances is false and there is no out specified, you will be leaking memory if there is no other
             *  delegate tracking mechanism implemented by your project.
             */
            template <typename className>
            void remove_delegate_procaddress(DelegateSet::MemberDelegateFuncPtr<className> procAddress, bool deleteInstances=true, unordered_set<DelegateSet::DelegateBaseType *>* out=NULL)
            {
                for (typename set<DelegateSet::DelegateBaseType *>::const_iterator it = this->begin(); it != this->end(); it++)
                {
                    DelegateSet::DelegateBaseType *current = *it;

                    if (current->has_procaddress(procAddress))
                    {
                        if (deleteInstances)
                            delete current;
                        else if (out)
                            out->insert(out->end(), current);

                        this->erase(current);
                    }
                }
            }

            /**
             *  @brief Removes all delegates from the set that have the given static proc address
             *  for it's method.
             *  @param procAddress The static proc address to check against.
             *  @param deleteInstances A boolean representing whether or not all matches should be deleted when removed. The default
             *  for this parameter is true for better memory management.
             *  @param out A pointer to an std::vector templated for the DelegateBase type that this DelegateSet contains. This vector
             *  will then be populated with a list of removed delegates. If deleteInstances is false, this list will never be populated.
             *  @warning If deleteInstances is false and there is no out specified, you will be leaking memory if there is no other
             *  delegate tracking mechanism implemented by your project.
             */
            void remove_delegate_procaddress(DelegateSet::StaticDelegateFuncPtr procAddress, const bool &deleteInstances=true, unordered_set<DelegateSet::DelegateBaseType *>* out=NULL)
            {
                for (typename set<DelegateSet::DelegateBaseType *>::const_iterator it = this->begin(); it != this->end(); it++)
                {
                    DelegateSet::DelegateBaseType *current = *it;

                    if (current->has_procaddress(procAddress))
                    {
                        if (deleteInstances)
                            delete current;
                        else if (out)
                            out->insert(out->end(), current);

                        this->erase(current);
                    }
                }
            }

            /**
             *  @brief Removes a all MemberDelegate types from the set that have a given 'this' pointer address
             *  to call against.
             *  @param thisPtr The address of the object to check against.
             *  @param deleteInstances A boolean representing whether or not all matches should be deleted when removed. The default
             *  for this parameter is true for better memory management.
             *  @param out A pointer to an std::vector templated for the DelegateBase type that this DelegateSet contains. This vector
             *  will then be populated with a list of removed delegates. If deleteInstances is false, this list will never be populated.
             *  @warning If deleteInstances is false and there is no out specified, you will be leaking memory if there is no other
             *  delegate tracking mechanism implemented by your project.
             */
            void remove_delegate_this(const void *thisPtr, const bool &deleteInstances=true, unordered_set<DelegateSet::DelegateBaseType *>* out=NULL)
            {
                for (typename set<DelegateSet::DelegateBaseType *>::iterator it = this->begin(); it != this->end(); it++)
                {
                    DelegateSet::DelegateBaseType *current = *it;

                    if (current->mIsMemberDelegate && current->has_thispointer(thisPtr))
                    {
                        if (deleteInstances)
                            delete current;
                        else if (out)
                            out->insert(out->end(), current);

                        this->erase(current);
                    }
                }
            }

            /**
             *  @brief Removes a given delegate by it's address.
             *  @param instance The delegate to attempt to remove from this set.
             *  @param deleteInstance A boolean representing whether or not the target delegate should
             *  be deleted.
             *  @return A pointer to the delegate that was removed. This is NULL if none were removed or
             *  if deleteInstance is true.
             */
            DelegateSet::DelegateBaseType *remove_delegate(DelegateSet::DelegateBaseType *instance, const bool &deleteInstance=true)
            {
                for (typename set<DelegateSet::DelegateBaseType *>::iterator it = this->begin(); it != this->end(); it++)
                {
                    DelegateSet::DelegateBaseType *current = *it;

                    if (current == instance)
                    {
                        if (deleteInstance)
                            delete current;

                        this->erase(current);
                        return current;
                    }
                }

                return NULL;
            }
    };

    /**
     *  @brief Base delegate type.
     *  @details Inheritance from this type allows for both the StaticDelegate
     *  and MemberDelegate instances to be used in the DelegateSet type as it
     *  specifies a common interface for the two types to be sharing.
     */
    template <typename returnType, typename... parameters>
    class DelegateBase : public GenericDelegate
    {
      //  friend class EasyDelegate::DelegateSet<returnType, parameters...>;
        public:
            //! Helper typedef referring to the return type of this delegate.
            typedef returnType ReturnType;
            //! Helper typedef for when building static delegates.
            typedef StaticDelegate<returnType, parameters...> StaticDelegateType;
            //! Helper typedef for when building member delegates.
            template <typename classType>
            using MemberDelegateType = MemberDelegate<classType, returnType, parameters...>;

            //! Helper typedef referring to a static function pointer.
            typedef returnType(*StaticDelegateFuncPtr)(parameters...);
            //! Helper typedef referring to a member function pointer.
            template <typename classType>
            using MemberDelegateFuncPtr = returnType(classType::*)(parameters...);

            /**
             *  @brief Constructor accepting a boolean.
             *  @param isMemberDelegate A boolean representing whether or not this delegate is a
             *  member delegate.
             */
            DelegateBase(const bool &isMemberDelegate) noexcept : GenericDelegate(isMemberDelegate, false) { }

            /**
             *  @brief Returns whether or not this delegate calls the given static proc address.
             *  @param procAddress A pointer to the static function to be checked against.
             *  @return A boolean representing whether or not this delegate calls the given proc address.
             *  @note Always returns false for MemeberDelegate types because a MemberDelegate cannot invoke static
             *  functions.
             */
            virtual const bool has_procaddress(const DelegateBase::StaticDelegateFuncPtr procAddress) const noexcept = 0;

            /**
             *  @brief Returns whether or not this delegate calls the given member proc address.
             *  @param procAddress A pointer to a member function to be checked against.
             *  @return A boolean representing whether or not this delegate calls the given proc address.
             *  @details This is not declared as a wholly virtual function because a templated function
             *  cannot be virtual.
             *  @note Always returns false for StaticDelegate types because a StaticDelegate cannot
             *  call member functions.
             */
            template <typename className>
            const bool has_procaddress(const DelegateBase::MemberDelegateFuncPtr<className> procAddress) const noexcept
            {
                // Don't try to check as a MemberDelegate if we're not actually one
                if (!mIsMemberDelegate)
                    return false;

                // This is a hack so that the proper has_proc_address function is called to get the return value
                MemberDelegate<className, returnType, parameters...> *memberDelegateObj = (MemberDelegate<className, returnType, parameters...>*)this;
                return memberDelegateObj->has_procaddress(procAddress);
            }

            /**
             *  @brief Returns whether or not this delegate calls against the given this pointer.
             *  @param thisPointer A pointer referring to the object of interest.
             *  @return A boolean representing whether or not this delegate calls a member function
             *  against the given this pointer.
             *  @note Always returns false for StaticDelegate types because they do not use a this pointer.
             */
            virtual const bool has_thispointer(const void *thisPointer) const noexcept = 0;

            /**
             *  @brief Invoke the delegate with the given arguments and return a value, if any.
             *  @param params Anything; It depends on the function signature specified in the template.
             *  @return Anything; It depends on the function signature specified in the template.
             *  @throw InvalidMethodPointerException Thrown when assertions are disabled and either a MemberDelegate
             *  or StaticDelegate type have a NULL function to call.
             *  @throw InvalidThisPointerException Thrown when assertions are disabled and this is a MemberDelegate
             *  attempting to call against a NULL this pointer.
             *  @throw std::exception Any exception can be potentially thrown by the function to be called.
             *  @note The call will not throw an exception in any of the std::runtime_error cases but rather
             *  assert if assertions are enabled.
             */
            virtual returnType invoke(parameters... params) = 0;

    };

    /**
     *  @brief The most generic of the cached delegate types. All cached delegate
     *  eventually trace back to this class in their hierarchy, therefore it is possible
     *  to cast them to this type and use the EasyDelegate::GenericCachedDelegate::generic_dispatch
     *  method.
     */
    class GenericCachedDelegate
    {
        public:
            /**
             *  @brief Invoke the cached delegate and ignore the return value.
             */
            virtual void generic_dispatch(void) = 0;
    };

    /**
     *  @brief
     */
    template <typename returnType>
    class GenericTypedCachedDelegate : public GenericCachedDelegate
    {
        public:
            /**
             *  @brief Invoke the cached delegate and return the result to the
             *  calling method.
             *  @return The value of the called method's return.
             */
            virtual returnType dispatch(void) = 0;

            /**
             *  @brief Invoke the cached delegate and ignore the return value.
             */
            virtual void generic_dispatch(void) = 0;
    };

    /**
     *  @brief
     */
    template <typename classType, typename returnType, typename... parameters>
    class CachedMemberDelegate : public GenericTypedCachedDelegate<returnType>
    {
        // Public Methods
        public:
            //! Helper typedef referring to a member function pointer.
            typedef returnType(classType::*MemberDelegateFuncPtr)(parameters...);

            /**
             *  @brief Constructor accepting a this pointer and a member function.
             *  @param thisPtr A pointer to the object instance to be considered 'this'
             *  during invocation.
             *  @param procAddress A pointer to the member function to be invoked upon
             *  'this'.
             *  @note The MemberDelegate invocation code has no way of knowing if the 'this'
             *  pointer at any time has been deallocated. The MemberDelegate will cause undefined
             *  behavior and/or segfault upon invocation in that case.
             */
            CachedMemberDelegate(const MemberDelegateFuncPtr procAddress, classType *thisPtr, parameters... params) :
            mThisPtr(thisPtr), mParameters(params...), mProcAddress(procAddress)
            {

            }

            /**
             *  @brief Dispatches the CachedDelegate.
             *  @details This is equivalent to the invoke() method on all other delegate
             *  types except the parameters were cached at creation. Said cached parameters
             *  will be passed in automatically upon calling this, so it is completely safe
             *  to store.
             *  @return Anything; it depends on the function signature defined in the template.
             */
            returnType dispatch(void)
            {
                return performCachedCall(typename gens<sizeof...(parameters)>::type());
            }

            /**
             *  @brief Dispatches the CachedDelegate, ignoring the return value.
             *  @details This behaves exactly as the dispatch method above except it does not
             *  care about the return of the called function. This method is also callable on
             *  the CachedDelegateBase type, unlike the normal dispatch method.
             */
            void generic_dispatch(void) { dispatch(); }

            /**
             *  @brief Returns whether or not this delegate calls the given member proc address.
             *  @param procAddress A pointer to a member function to be checked against.
             *  @return A boolean representing whether or not this delegate calls the given proc address.
             *  @details This is not declared as a wholly virtual function because a templated function
             *  cannot be virtual.
             *  @note Always returns false for StaticDelegate types because a StaticDelegate cannot
             *  call member functions.
             */
            const bool has_procaddress(const MemberDelegateFuncPtr funcPtr) const noexcept { return funcPtr == mProcAddress; }

            /**
             *  @brief Returns whether or not this delegate calls against the given this pointer.
             *  @param thisPointer A pointer referring to the object of interest.
             *  @return A boolean representing whether or not this delegate calls a member function
             *  against the given this pointer.
             *  @note Always returns false for StaticDelegate types because they do not use a this pointer.
             */
            virtual const bool has_thispointer(const void *thisPointer) const noexcept { return thisPointer == mThisPtr; }

            bool operator ==(const CachedMemberDelegate<classType, returnType, parameters...> &other) const noexcept { return other.mProcAddress == mProcAddress; }

            template <typename otherClass, typename otherReturn, typename... otherParams>
            bool operator ==(const CachedMemberDelegate<otherClass, otherReturn, otherParams...> &other) const noexcept { return false;  }

            template <typename otherReturn, typename... otherParams>
            bool operator ==(const CachedStaticDelegate<otherReturn, otherParams...> &other) const noexcept { return false; }

        // Private Methods
        private:
            //! Internal templated method to invoke the stored delegate instance.
            template<int ...S>
            INLINE returnType performCachedCall(seq<S...>)
            {
                assert(mThisPtr);

                if (!mThisPtr)
                    throw InvalidThisPointerException();

                return (mThisPtr->*mProcAddress)(get<S>(mParameters) ...);
            }

        // Public Members
        public:
            //! An internal pointer to this object.
            classType *mThisPtr;

        // Private Members
        private:
            //! An internal pointer to the proc address to be called.
            const MemberDelegateFuncPtr mProcAddress;

            //! Internal std::tuple that is utilized to cache the parameter list.
            const NoReferenceTuple<parameters...> mParameters;
    };

    /**
     *  @brief
     */
    template <typename returnType, typename... parameters>
    class CachedStaticDelegate : public GenericTypedCachedDelegate<returnType>
    {
        // Public Methods
        public:
            //! Helper typedef referring to a static function pointer.
            typedef returnType(*StaticDelegateFuncPtr)(parameters...);

            /**
             *  @brief Constructor accepting a this pointer and a member function.
             *  @param thisPtr A pointer to the object instance to be considered 'this'
             *  during invocation.
             *  @param procAddress A pointer to the member function to be invoked upon
             *  'this'.
             *  @note The MemberDelegate invocation code has no way of knowing if the 'this'
             *  pointer at any time has been deallocated. The MemberDelegate will cause undefined
             *  behavior and/or segfault upon invocation in that case.
             */
            CachedStaticDelegate(const StaticDelegateFuncPtr procAddress, parameters... params) :
            mParameters(params...), mProcAddress(procAddress) { }

            /**
             *  @brief Dispatches the CachedDelegate.
             *  @details This is equivalent to the invoke() method on all other delegate
             *  types except the parameters were cached at creation. Said cached parameters
             *  will be passed in automatically upon calling this, so it is completely safe
             *  to store.
             *  @return Anything; it depends on the function signature defined in the template.
             */
            INLINE returnType dispatch(void)
            {
                return performCachedCall(typename gens<sizeof...(parameters)>::type());
            }

            /**
             *  @brief Dispatches the CachedDelegate, ignoring the return value.
             *  @details This behaves exactly as the dispatch method above except it does not
             *  care about the return of the called function. This method is also callable on
             *  the CachedDelegateBase type, unlike the normal dispatch method.
             */
            void generic_dispatch(void) { dispatch(); }

            /**
             *  @brief Returns whether or not this delegate calls the given member proc address.
             *  @param procAddress A pointer to a member function to be checked against.
             *  @return A boolean representing whether or not this delegate calls the given proc address.
             *  @details This is not declared as a wholly virtual function because a templated function
             *  cannot be virtual.
             *  @note Always returns false for StaticDelegate types because a StaticDelegate cannot
             *  call member functions.
             */
            const bool has_procaddress(const StaticDelegateFuncPtr funcPtr) const noexcept { return false; }


            /**
             *  @brief Returns whether or not this delegate calls against the given this pointer.
             *  @param thisPointer A pointer referring to the object of interest.
             *  @return A boolean representing whether or not this delegate calls a member function
             *  against the given this pointer.
             *  @note Always returns false for StaticDelegate types because they do not use a this pointer.
             */
            virtual const bool has_thispointer(const void *thisPointer) const noexcept { return true; }

            bool operator ==(const CachedStaticDelegate<returnType, parameters...> &other) const noexcept { return other.mProcAddress == mProcAddress; }

            template <typename otherClass, typename otherReturn, typename... otherParams>
            bool operator ==(const CachedMemberDelegate<otherClass, otherReturn, otherParams...> &other) const noexcept { return false;  }

            template <typename otherReturn, typename... otherParams>
            bool operator ==(const CachedStaticDelegate<otherReturn, otherParams...> &other) const noexcept { return false; }

        // Private Methods
        private:
            //! Internal templated method to invoke the stored delegate instance.
            template<int ...S>
            INLINE constexpr returnType performCachedCall(seq<S...>)
            {
                return (mProcAddress)(get<S>(mParameters) ...);
            }

            //! An internal pointer to the proc address to be called.
            const StaticDelegateFuncPtr mProcAddress;

            //! Internal std::tuple that is utilized to cache the parameter list.
            const NoReferenceTuple<parameters...> mParameters;
    };
} // End Namespace EasyDelegate
#endif // _INCLUDE_EASYDELEGATE_HPP_
