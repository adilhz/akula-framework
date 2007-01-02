/***************************************************************************
 *   Copyright (C) 2006 by Krasimir Marinov   *
 *   krasimir.vanev@gmail.com    *
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

#ifndef OS_EVENT_DEMULTIPLEXER_H
#define OS_EVENT_DEMULTIPLEXER_H

//We want to avoid virtual function invokation! It kills the performance.
//Using the engines methodology.
//Here is a brief description:
/*
When are virtual functions bad? 
       - When the amount of code inside the virtual function is small (e.g. fewer than 25 flops); and 
       - When the virtual function is used frequently (e.g. inside a loop). You can determine this with the help
       of a profiler. 
Virtual functions are okay (and highly recommended!) if the function being called is big (does a lot of 
computations), or if it isn't called very often. 
Unfortunately, in scientific codes some of the most useful places for virtual function calls are inside loops, 
and involve small routines. A classic example is: 
      class Matrix {
         public: virtual double operator()(int i, int j) = 0;
      };
      class SymmetricMatrix : public Matrix {
         public: virtual double operator()(int i, int j);
      };
      class UpperTriangularMatrix : public Matrix {
         public: virtual double operator()(int i, int j);
      };
The virtual function dispatch to operator() to read elements from the matrix will ruin the performance of 
any matrix algorithm. 
1.3.2: Solution A: simple engines
The "engines" term was coined by the POOMA team at LANL. The design pattern folks probably have a 
name for this too (if you know it, please tell me). 
The idea is to replace dynamic polymorphism (virtual functions) with static polymorphism (template 
parameters). Here's a skeleton of the Matrix example using this technique: 

    class Symmetric {
     // Encapsulates storage info for symmetric matrices
    };
    class UpperTriangular {
     // Storage format info for upper tri matrices
    };
    template<class T_engine>
    class Matrix {
    private:
       T_engine engine;
    };
    // Example routine which takes any matrix structure
    template<class T_engine>
    double sum(Matrix<T_engine>& A);
    // Example use ...
       Matrix<Symmetric> A;
       sum(A);

In this approach, the variable aspects of matrix structures are hidden inside the engines Symmetric and 
UpperTriangular. The Matrix class takes an engine as a template parameter, and contains an instance 
of that engine. 
The notation is different than what users are used to: Matrix<Symmetric> rather than 
SymmetricMatrix. This is probably not a big deal. One could provide typedefs to hide this quirk. Also, 
in Matrix most interesting operations have to be delegated to the engines again, not a big deal. 
The biggest headache is that all Matrix subtypes must have the same member functions. For example, if we 
want Matrix<Symmetric> to have a method isSymmetricPositiveDefinite(), then a typical 
implementation would be 

    template<class T_engine>
    class Matrix {
    public:
        // Delegate to the engine
        bool isSymmetricPositiveDefinite()
        { return engine.isSymmetricPositiveDefinite(); }
    private:
        T_engine engine;
    };
    class Symmetric {
    public:
        bool isSymmetricPositiveDefinite()
        {
             ...
        }
    };

But it doesn't make sense for Matrix<UpperTriangular> to have an 
isSymmetricPositiveDefinite() method, since upper triangular matrices cannot be symmetric! 
What you find with this approach is that the base type (in this example, Matrix) ends up having the union 
of all methods provided by the subtypes. So you end up having lots of methods in each engine which just 
throw exceptions: 
       class UpperTriangular {
       public:
           bool isSymmetricPositiveDefinite()
           {
                throw makeError("Method isSymmetricPositiveDefinite() is "
                       "not defined for UpperTriangular matrices");
                return false;
           }
       };
Alternate (but not recommended) approach: just throw your users to the wolves by omitting these methods. 
Then users must cope with mysterious template instantiation errors which result when they try to use missing 
methods. 
Fortunately, there is a better solution.
 1.3.3: Solution B: the Barton and Nackman Trick
This trick is often called the "Barton and Nackman Trick", because they used it in their excellent book 
Scientific and Engineering C++. Geoff Furnish coined the term "Curiously Recursive Template Pattern", 
which is a good description. 
*/

/*We are using the first desribed method because all of our engines should provide the same interface!
* So this way we force the new engines implementors to use the given inteface.
*/
namespace reactor
{

template<class T_engine>
class OsEventDemultiplexer
{
 private:
    T_engine engine;

 public:
    // Delegating to the engine
    void add_fd(int iFD, unsigned long ulFlag)
    {
        engine.add_fd(iFD, ulFlag);
    }
    
    void remove_fd(int iFD, unsigned long ulFlag)
    {
        engine.remove_fd(iFD, ulFlag);
    }

    /* VERY IMPORTANT!!!
    *May be here we'll have problems when implementing epoll(). How will the ready events structure 
    * be passed to the app demultiplexer? May be we have to pass not only int (ready fds) by watch_fds() but
    * a structure containg data for the used underlying mechanism, for example:
    * struct SResult {
    *     int iReadyFds;
    *     void* pInfo; //epoll_event*
    *     EOSReactorType type; // type epoll
    * };
    * Thus we will easy have mechanism for communication from os to app reactor's implementation
    *
    * For poll and select implementations we'll have:
    * struct SResult pollResult = {n, NULL, poll};
    * For epoll:
    * struct SResult epollResult = {n, pevents, epoll};
    */
    int watch_fds(void)
    {
        return engine.watch_fds();
    }
    
    bool check_fd(int iFD, unsigned long ulFlag)
    {
        return engine.check_fd(iFD, ulFlag);
    }

    void deactivate_fd(int iFD)
    {
        engine.deactivate_fd(iFD);
    }
        
    void reactivate_fd(int iFD)
    {
        engine.reactivate_fd(iFD);
    }
};

}//namespace reactor

#endif /*OS_EVENT_DEMULTIPLEXER_H*/

