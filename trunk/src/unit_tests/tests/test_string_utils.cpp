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

#include "TUT/tut.h"
#include "akula/utils/utils.h"
#include <iostream>
#include <vector>
#include <string>

namespace tut
{
    //empty data structure since we don't need any shared object (any state) for the tests
    struct string_utils_data
    {
        int result;
        int* presult;
        string_utils_data() : result(0), presult(&result) {}
    };

    typedef test_group<string_utils_data> factory;
    typedef factory::object object;

    factory test_string_utils("test_string_utils");

    /**Tests CTrimmer functionality*/
    template<>
    template<>
    void object::test<1>()
    {
        std::string s1("   \t\n\rSTRING");
        std::string s2("STRING   \t\r\n");
        std::string s3(" \t\r\nSTRING   \t\r\n");

        utils::CTrimmer<>::trimLeft(s1);
        ensure("CTrimmer<>::trimLeft()", s1.compare("STRING") == 0);

        utils::CTrimmer<>::trimRight(s2);
        ensure("CTrimmer<>::trimRight()", s2.compare("STRING") == 0);

        utils::CTrimmer<>::trim(s3);
        ensure("CTrimmer<>::trim()", s3.compare("STRING") == 0);
    }

    /**Tests CTokenizer*/
    template<>
    template<>
    void object::test<2>()
    {
        std::vector<std::string> result;


        std::string str1("NAME:VALUE");
        utils::CTokenizer<utils::CIsColon>::tokenize(result, str1);
        ensure("str1: tokenized elements number", result.size() == 2);
        ensure("str1: first token", result[0].compare("NAME") == 0);
        ensure("str1: second token", result[1].compare("VALUE") == 0);

        char* buf1 = "NAME:VALUE";
        utils::CTokenizer<utils::CIsColon>::tokenize(result, buf1, (buf1 + ::strlen(buf1) -1));
        ensure("buf1: tokenized elements number", result.size() == 2);
        ensure("buf1: first token", result[0].compare("NAME") == 0);
        ensure("buf1: second token", result[1].compare("VALUE") == 0);

        char* buf2 = "0   \t\r\n1        \t2";
        utils::CTokenizer<>::tokenize(result, buf2, (buf2 + ::strlen(buf2) -1));
        ensure("buf2: tokenized elements number", result.size() == 3);
        ensure("buf2: first token", result[0].compare("0") == 0);
        ensure("buf2: second token", result[1].compare("1") == 0);
        ensure("buf2: third token", result[2].compare("2") == 0);

        std::string str2 = "0   \t\r\n1        \t2";
        utils::CTokenizer<>::tokenize(result, str2);
        ensure("str2: tokenized elements number", result.size() == 3);
        ensure("str2: first token", result[0].compare("0") == 0);
        ensure("str2: second token", result[1].compare("1") == 0);
        ensure("str2: third token", result[2].compare("2") == 0);

        char* buf3 = "                       \t\r\n              ";
        utils::CTokenizer<>::tokenize(result, buf3, buf3 + ::strlen(buf3) -1);
        ensure("buf3: tokenized elements number", result.size() == 0);

        std::string str3 = "                       \t\r\n              ";
        utils::CTokenizer<>::tokenize(result, str3);
        ensure("str3: tokenized elements number", result.size() == 0);
    }

    template<>
    template<>
    void object::test<3>()
    {
        std::string sResult;
        std::string sValue("10");
        int iResult;
        int iValue = 10;

        utils::to_string(iValue, sResult, std::dec);
        ensure("to_string(std::dec)", sResult.compare("10") == 0);

        utils::to_string(iValue, sResult, std::hex);
        ensure("to_string(std::hex)", sResult.compare("a") == 0);

        utils::from_string(iResult, sValue, std::dec);
        ensure("from_string(std::dec)", iResult == 10);

        utils::from_string(iResult, sValue, std::hex);
        ensure("from_string(std::hex)", iResult == 16);
    }
}
