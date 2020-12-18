/*
* Copyright (C) 2015-2018, Kevin Brubeck Unhammer <unhammer@fsfe.org>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef b2070ddb5ed4e0b7_JSON_H
#define b2070ddb5ed4e0b7_JSON_H

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <locale>

#include "util.hpp"

namespace json {

inline const std::string uhex(const int i)
{
	std::stringstream ss;
	ss << "\\u";
	ss << std::hex << std::setfill('0') << std::uppercase;
	ss << std::setw(4) << static_cast<unsigned>(i);
	return ss.str();
}

inline const std::string esc(const std::u16string& str) {
	std::vector<char16_t> os;
	for (const char16_t& c : str) {
		switch(c) {
			case '"':
				os.push_back('\\');
				os.push_back('"');
				break;
			case '\\':
				os.push_back('\\');
				os.push_back('\\');
				break;
			case '\n': // Could use uhex, but looks nicer:
				os.push_back('\\');
				os.push_back('n');
				break;
			default:
				auto ci = (int)c;
				if((sizeof(c) == 1 || static_cast<unsigned>(c) < 256)
				   && (ci<=0x1f ||
				       ci==0x7f ||
				       (ci>=0x80 && ci<=0x9f) ||
				       c == '\\' ||
				       c == '"')) {
					for (const auto cc : uhex(ci)) {
						os.push_back(cc);
					}
				}
				else {
					os.push_back(c);
				}
		}
	}
	return divvun::toUtf8(std::u16string(os.begin(), os.end()));
}

inline const std::string str(const std::u16string& s)
{
	return "\""+esc(s)+"\"";
}

inline const std::string key(const std::u16string& s)
{
	return str(s)+":";
}

template<typename Container>
inline const std::string str_arr(const Container& ss)
{
	std::ostringstream os;
	for(const auto& s : ss) {
		os << str(s) << ",";
	}
	const auto& str = os.str();
	return "[" + str.substr(0, str.size() - 1) + "]";
}

inline void sanity_test() {
	std::string got = json::key(u"e\tr\"r\\s\nfoo");
	std::string want = "\"e\\u0009r\\\"r\\\\s\\nfoo\":";
	if(got != want){
		std::cerr << "Error in json::key\n GOT: "<< got << "\nWANT: " << want << std::endl;
		throw std::runtime_error("JSON sanity check failed, major regression!");
	}
}

}
#endif
