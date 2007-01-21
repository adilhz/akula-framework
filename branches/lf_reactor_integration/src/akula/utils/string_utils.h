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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <vector>

namespace utils
{

/*
 * String tokenizer class and tokenize functors
 * Thanks to http://www.codeproject.com/cpp/stringtok.asp
 */

/* --------------Common predicates(functors ) ------------------*/

// white-space character
class CIsSpace : public std::unary_function<char, bool>
{
 public:
    inline bool operator()(char c) const
    {
        // isspace<char> returns true if c is a white-space character 
        // (0x09-0x0D or 0x20)
        return std::isspace(c);
    }
};

// comma
class CIsComma : public std::unary_function<char, bool>
{
 public:
    inline bool operator()(char c) const
    {
        return (',' == c);
    }
};

// is colon
class CIsColon : public std::unary_function<char, bool>
{
 public:
    inline bool operator()(char c) const
    {
        return (':' == c);
    }
};

//defined string
class CIsFromString : public std::unary_function<char, bool>
{
 public:
    //Constructor specifying the separators
    CIsFromString(const std::string& str) : m_str(str) {}
    inline bool operator()(char c) const
    {
        std::string::size_type pos = m_str.find(c);
        if(pos != std::string::npos)
            return true;
        else
            return false;
    }

  private:
    std::string m_str;
};

/*-----------------------------The tokenizer class------------------------------*/
/*
How to use

The following code snippet is showing some simple usage examples, one for each one of the implemented predicates:

//Test CIsSpace() predicate
cout << "Test CIsSpace() predicate:" << endl;
//The Results Vector
vector<string> oResult;
//Call Tokeniker
CTokenizer<>::Tokenize(oResult, " wqd \t hgwh \t sdhw \r\n kwqo \r\n  dk ");
//Display Results
for(int i=0; i<oResult.size(); i++)
  cout << oResult[i] << endl;

//Test CIsComma() predicate
cout << "Test CIsComma() predicate:" << endl;
//The Results Vector
vector<string> oResult;
//Call Tokeniker
CTokenizer<CIsComma>::Tokenize(oResult, "wqd,hgwh,sdhw,kwqo,dk", CIsComma());
//Display Results
for(int i=0; i<oResult.size(); i++)
  cout << oResult[i] << endl;

//Test CIsFromString predicate
cout << "Test CIsFromString() predicate:" << endl;
//The Results Vector
vector<string> oResult;
//Call Tokeniker
CTokenizer<CIsFromString>::Tokenize(oResult, ":wqd,;hgwh,:,sdhw,:;kwqo;dk,", 
                                          CIsFromString(",;:"));
//Display Results
cout << "Display strings:" << endl;
for(int i=0; i<oResult.size(); i++)
  cout << oResult[i] << endl;
*/

template <class Predicate=CIsSpace>
class CTokenizer
{
 public:
    //The predicate should evaluate to true when applied to a separator.
    static void tokenize(std::vector<std::string>& result,
                                    const std::string& str,
                                    const Predicate& predicate=Predicate())
    {
        //First clear the results vector
        result.clear();
        std::string::const_iterator it = str.begin();
        std::string::const_iterator itTokenEnd = str.begin();

        while(it != str.end())
        {
            //Eat separators
            while(predicate(*it))
            {
                it++;
            }
        
            //Find next token
            itTokenEnd = std::find_if(it, str.end(), predicate);
        
            //Append token to result
            if(it < itTokenEnd)
            {
                result.push_back(std::string(it, itTokenEnd));
            }
        
            it = itTokenEnd;
        }
    }

    /**
    * Tokenizes the buffer [pchStart, pchEnd]
    */
    static void tokenize(std::vector<std::string>& result, 
                                    const char* pchStart, const char* pchEnd,
                                    const Predicate& predicate=Predicate())
    {
        //First clear the results vector
        result.clear();
        const char* it = pchStart;
        const char* itTokenEnd = pchStart;

        while(it != pchEnd)
        {
            //Eat separators
            while(predicate(*it) && it < pchEnd)
                it++;
        
            //Find next token
            //itTokenEnd = std::find_if(it, pchEnd, predicate);
            for(itTokenEnd = it; itTokenEnd < pchEnd && !predicate(*itTokenEnd); itTokenEnd++)
                ;
        
            //Append token to result
            if(it < itTokenEnd)
            {
                result.push_back(std::string(it, 
                                                    predicate(*itTokenEnd) ? itTokenEnd - it : itTokenEnd -it + 1));
            }
            else if(it == itTokenEnd && !predicate(*it)) //this is the last character from the string
            {
                result.push_back(std::string(it, 1));
            }
        
            it = itTokenEnd;
        }
    }
};

/*-----------------------CTrimmer--------------------------------------------*/

template<class Predicate = CIsSpace>
class CTrimmer
{
 public:
    static void trimLeft(std::string& s, const Predicate& predicate = Predicate())
    {
        std::string::iterator it = std::find_if(s.begin(), s.end(),
                                                            std::unary_negate<Predicate>(predicate));

        s.erase(s.begin(), it);
    }

    static void trimRight(std::string& s, const Predicate& predicate = Predicate())
    {
        std::string::reverse_iterator rit = std::find_if(s.rbegin(), s.rend(),
                                                            std::unary_negate<Predicate>(predicate));

        s.erase(rit.base(), s.end());
    }

    static void trim(std::string& s, const Predicate& predicate = Predicate())
    {
        trimLeft(s, predicate);
        trimRight(s, predicate);
    }
};

/*
 * Converting from/to string
 */
template<class T>
inline
bool
to_string(const T& val, std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::ostringstream oss;
  if(!(oss << f << val).fail())
  {
    s = oss.str();
    return true;        
  }

  return false;
}

// the third parameter of from_string() should be
// one of std::hex, std::dec or std::oct</span>
template <class T>
inline
bool
from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

}//namespace utils

#endif /*STRING_UTILS_H*/


