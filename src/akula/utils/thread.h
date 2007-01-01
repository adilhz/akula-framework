/***************************************************************************
 *   Copyright (C) 2006 by Krasimir Marinov   *
 *   krasimir.vanev@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <akula/dbg/dbg.h>
#include <cassert>
#include <cerrno>
#include <functional>    /*std::unary_function<>*/

namespace utils
{

    /**A do-nothing functor used as default CThread template argument.*/
    struct SDoNothing : public std::unary_function<void*, void*>
    {
        void* operator()(void*)
        {
            return NULL;
        }
    };

    /**
    * The thread class used to start threads with the functors passed as its template parameter.
    * The functor should have method with this signature: <code>void* operator()(void*);</code>;
    * For example see the <code>SDoNothing</code> functor, used as default template prameter.
    * Example for using this class can be found in the unit tests code <file>test_thread.cpp</file>
    *
    * We have default template parameter to be able to use the static methods in the following way:
    * - CThread<>::self();
    * - CThread<>::exit(); and so...
    */
    template<class T = SDoNothing>
    class CThread
    {
     public:
        /**Thread ID type*/
        typedef pthread_t ThreadId_t;

     protected:
        /** The thread functor instance*/
        T m_pThreadObj;

        /** Thread arguments*/
        void* m_pThreadArg;

        /** The started thread (ID)*/
        ThreadId_t m_thread;

        /** Specifies if the thread is in detached mode
        * In detached mode you cannot join the thread to get its exit status and the thread object is
        * destroyed right after thread funvtion execution (its exit point)
        * In joinable mode you cannot join the thread from another thread to obtain its execution status.
        * Joining the joinable threads destroys it (its object presentation).
        */
        bool m_bDetached;

        /**
        * We are using the strategy "every allocation is initialization" to be able to destroy the resource
        * when the object leaves the current scope. 
        * In the particular case this object ensures destroying the thread attributes object
        */
        struct SThreadAttrInitializer
        {
            /** The thread attributes object*/
            pthread_attr_t attr;

            SThreadAttrInitializer() //initialization
            {
                ::pthread_attr_init(&attr);
            }

            ~SThreadAttrInitializer() //resource deallocation
            {
                ::pthread_attr_destroy(&attr);
            }
        };


     public:
        /** Get the thread ID*/
       static ThreadId_t self() {return ::pthread_self();}

        /**
         * The pthread_exit() function terminates the calling thread. All thread-specific data bindings are released. 
         * If the calling thread is not detached, then the thread's ID and the exit status specified by status are retained
         * until the thread is waited for (blocked). Otherwise, status is ignored and the thread's ID can be reclaimed immediately.
         *
         * The pointer status must not point to the object that is local to the calling thread, since that object disappears when the thread 
         * terminates.
        */
       static void exit(void* status = NULL) {::pthread_exit(status);}

        /**
        * We need static method instance-creator because we have to ensure the object will be created in the heap.
        * the thread existance and this object, representing it, are in direct relation, i.e. the object exists till the thread function is
        * running, after that the object is automatically destroyed. So we doesn't allow the user create instances in  the stack.
        * 
        * @param pArg The thread arguments (Defaults to NULL)
        * @param bDetached Specifies the thread status - detached / joinable (Defaults to false, i.e. joinable)
        * @return Pointer to the thread object
        */
        static CThread* getInstance(void* pArg = NULL, bool bDetached = false)
        {
            return new CThread(pArg, bDetached);
        }

        /**
        * Starts the thread
        * @return <code>true</code> if the thread attributes are set and the thread is started, otherwise <code>false</code>
        * In case of failure the object is automatically destroyed
        */
        bool run()
        {
            // thread attributes
            SThreadAttrInitializer attrInitializer;

            //set detach state
            if(::pthread_attr_setdetachstate(&(attrInitializer.attr), isDetached() ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED) != 0)
            {
                dbg::error() << "Couldn't set detach state: " << ::strerror(errno) << "\n";
                delete this;
                return false;
            }

            if(::pthread_create(&m_thread, NULL, threadRunner, reinterpret_cast<void*>(this)) != 0)
            {
                dbg::error() << "Error starting thread: " << ::strerror(errno) << std::endl;
                delete this;
                return false;
            }

            return true;
        }

        /**
        * Join the thread
        * @param status The status returned from thread execution
        * @return <code>false</code> in case of failure
        */
        bool join(void** status = NULL)
        {
            if(m_thread == 0 || isDetached() || m_thread == self())
                return false;

            if(0 == ::pthread_join(m_thread, status))
            {
                delete this;
                return true;
            }

           return false;
        }

        /**
        * Returns the type of the thread (joinable/detached)
        */
        bool isDetached() const {return m_bDetached;}

        /**
        * Returns the thread arguments
        */
        void* getArg() {return m_pThreadArg;}

     protected:

        /**
        * Thread ctor
        * @param pArg The thread arguments
        * @param bDetached Thread type (detached/joinable)
        */
        CThread(void* pArg, bool bDetached)
            : m_pThreadObj()
            , m_pThreadArg(pArg)
            , m_thread(0)
            , m_bDetached(bDetached)
        {
        }

        /**
        * The object is destroyed when the thread exits, in case of detached, or being joined by other thread in case of joinable
        * So as we stated before the object representing the thread exists till the thread function works
        */
        virtual ~CThread() {}

        /**The thread proxy function, used to execute the thread method through the thread system call*/
        static void* threadRunner(void *pArg)
        {

            // Here, the template has been implicitly specialized by its context.
            // It is within the specialization region of the class scope.  Thus it does not need the template arguments.
            // For a class definition, the specialization region is the class block.
            CThread* pThread = reinterpret_cast<CThread*>(pArg);
            assert(pThread != NULL);

            void* pResult = pThread->m_pThreadObj(pThread->getArg());

            void* pRetVal = NULL;
            if(pThread->isDetached())
            {
                exit(0);
                delete pThread;
            }
            else
            {
                pRetVal = pResult;
                exit(pRetVal); 
            }

            return pRetVal;
        }
    };

}//namespace utils

#endif //THREAD_H

