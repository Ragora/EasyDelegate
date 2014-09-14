/**
 *  @file easydelegate.hpp
 *  @version 0.1
 *  @date 9/13/2014
 *  @copyright This software is licensed under the Lesser General Public License version 3.
 *  Please refer to GPL.txt and LGPL.txt for more information.
 *  @brief Portable delegate system that should work on any C++11 compliant compiler.
 *  @details EasyDelegate is a library that exposes an easy and flexible delegate system with
 *  the use of C++11's variatic templates.
 *  @example ex_delegateset.cpp
 */

#include <stdexcept>
#include <assert.h>
#include <vector>

//! Namespace containing all EasyDelegate functionality.
namespace EasyDelegate
{
    /**
     *  @brief Base delegate type.
     *  @details Inheritance from this type allows for both the StaticDelegate
     *  and MemberDelegate instances to be used in the DelegateSet type.
     */
    template <typename returnType, typename... arguments>
    class DelegateBase
    {
        public:
            /**
             *  @brief Invoke the delegate.
             */
            virtual returnType invoke(arguments... args) = 0;
    };

    /**
     *  @brief A delegate of a static function.
     *  @details The function that is to be made into a delegate must have
     *  it's return type and argument information specified in the template.
     */
    template <typename returnType, typename... arguments>
    class StaticDelegate : public DelegateBase<returnType, arguments...>
    {
        public:
            //! Helper typedef referring to a function pointer that is compatible with this delegate.
            typedef returnType(*staticDelegateFuncPtr)(arguments...);

            /**
             *  @brief Constructor accepting a static function.
             *  @param newDelegateFunc The static function that this delegate
             *  is intended to invoke.
             */
            StaticDelegate(staticDelegateFuncPtr newDelegateFunc)
            {
                mStaticDelegateFunc = newDelegateFunc;
            }

            /**
             *  @brief Invoke the StaticDelegate.
             *  @throw std::runtime_error Thrown when the static function that this
             *  delegate is supposed to be invoking is NULL.
             */
            returnType invoke(arguments... args)
            {
                assert(mStaticDelegateFunc);

                if (!mStaticDelegateFunc)
                    throw std::runtime_error("Bad StaticDelegate Function");

                return mStaticDelegateFunc(args...);
            }

        private:
            staticDelegateFuncPtr mStaticDelegateFunc;
    };

    template <typename classType, typename returnType, typename... arguments>
    class MemberDelegate : public DelegateBase<returnType, arguments...>
    {
        public:
            typedef returnType(classType::*memberDelegateFuncPtr)(arguments...);

            MemberDelegate(classType *thisPtr, memberDelegateFuncPtr newDelegateFunc)
            {
                mThisPtr = thisPtr;
                mMemberDelegateFunc = newDelegateFunc;
            }

            returnType invoke(arguments... args)
            {
                assert(mThisPtr);
                assert(mMemberDelegateFunc);

                if (!mThisPtr)
                    throw std::runtime_error("Invalid MemberDelegate This Pointer");

                if (!mMemberDelegateFunc)
                    throw std::runtime_error("Invalid MemberDelegate Function");

                return (mThisPtr->*mMemberDelegateFunc)(args...);
            }

        private:
            classType *mThisPtr;
            memberDelegateFuncPtr mMemberDelegateFunc;
    };

    /**
     *  @brief A set of delegates with manipulation similar to std::vector.
     *  @note Both MemberDelegate and StaticDelegate instances may be used with
     *  this.
     */
    template <typename returnType, typename... arguments>
    class DelegateSet
    {
        private:
            //! Helper typedef to construct the function pointer signature from the template.
            typedef returnType(*delegateFuncPtr)(arguments...);
            //! Helper typedef to construct the StaticDelegate signature from the template.
            typedef DelegateBase<returnType, arguments...>* delegateType;

            //! Internal vector storing the StaticDelegate instances.
            std::vector<delegateType> mDelegateVector;

        public:

            /**
             *  @brief Pushes a delegate instance to the end of the set.
             *  @param delegateInstance The delegate instance to the pushed onto the set.
             */
            void push_back(delegateType delegateInstance)
            {
                mDelegateVector.push_back(delegateInstance);
            }

            /**
             *  @brief Inserts a given function as a delegate at it.
             *  @param it The iterator representing the position to insert at.
             *  @param in The static function to insert as a delegate.
             */
            void insert(typename std::vector<delegateType>::iterator it, delegateFuncPtr in)
            {
                mDelegateVector.insert(it, delegateType(in));
            }

            /**
             *  @brief Inserts a given StaticDelegate instance at it.
             *  @param it The iterator representing the position to insert at.
             *  @param in the StaticDelegate instance to insert.
             */
            void insert(typename std::vector<delegateType>::iterator it, delegateType in)
            {
                mDelegateVector.insert(it, in);
            }

            /**
             *  @brief Invoke all delegates in the set, storing return values in out.
             *  @param out The std::vector that all return values will be sequentially written to.
             */
            void invoke(std::vector<returnType> &out, arguments... args)
            {
                for (typename std::vector<delegateType>::iterator it = mDelegateVector.begin(); it != mDelegateVector.end(); it++)
                    out.push_back((*it)->invoke(args...));
            }

            /**
             *  @brief Invoke all delegates in the set, ignoring any return values.
             */
            void invoke(arguments... args)
            {
                for (typename std::vector<delegateType>::iterator it = mDelegateVector.begin(); it != mDelegateVector.end(); it++)
                    (*it)->invoke(args...);
            }

            /**
             *  @brief Get an iterator to the beginning of the set.
             *  @return An std::iterator referencing the beginning of the set.
             */
            typename std::vector<delegateType>::iterator begin(void)
            {
                return mDelegateVector.begin();
            }

            /**
             *  @brief Get an iterator to the end of the set.
             *  @return An std::const_iterator referencing the end of the set.
             */
            typename std::vector<delegateType>::const_iterator end(void)
            {
                return mDelegateVector.end();
            }

            //! Helper typedef to get an std::iterator.
            typedef typename std::vector<delegateType>::iterator iterator;
            //! Helper typedef to get an std::const_iterator.
            typedef typename std::vector<delegateType>::const_iterator const_iterator;
    };
} // End Namespace EasyDelegate
