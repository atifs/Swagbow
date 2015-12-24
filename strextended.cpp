/*
	Created: 4/28/2014
	Author: Cyrus
	Purpose: Helper functions that go well with std::string
*/

#include "strextended.h"

const static char normal_upsidedownstrings[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '.', '?', '!', '"', '\'', '`', '(', ')', '[', ']', '{', '}', '<', '>', '&', '_', };
const static std::string upsidedownstrings[] = { "&#592;", "q", "&#596;", "p", "&#477;", "&#607;", "&#387;", "&#613;", "&#7433;", "&#638;", "&#670;", "l", "&#623;", "u", "o", "d", "b", "&#633;", "s", "&#647;", "n", "&#652;", "&#653;", "x", "&#654;", "z", "&#8704;", "B", "&#390;", "D", "&#398;", "&#8498;", "&#1508;", "H", "I", "&#383;", "K", "&#741;", "W", "N", "O", "&#1280;", "Q", "R", "S", "&#9524;", "&#8745;", "&#923;", "M", "X", "&#8516;", "Z", "0", "&#406;", "&#4357;", "&#400;", "&#12579;", "&#987;", "9", "&#12581;", "8", "6", "'", "&#729;", "&#191;", "&#161;", ",,", ",", ",", ")", "(", "]", "[", "}", "{", ">", "<", "&#8523;", "&#8254;",};

bool strextended::startswith(std::string needle, std::string &haystack) {
	std::string start = haystack.substr(0, needle.length());
	return start == needle;
}

strextended::split3 strextended::splatter(std::string find_prefix, std::string find_postfix, std::string &str) {
	strextended::split3 split3 = {"", "", ""};
	int begin = str.find(find_prefix);
	if( begin == std::string::npos ) {
		//Nothing found
		split3.middle = str;
		return split3;
	}

	int end = str.find(find_postfix, begin + 1);
	if( end == std::string::npos ) {
		//Nothing found
		split3.middle = str;
		return split3;
	}

	end += find_postfix.length() - 1;

	//Something was found
	int extract_size = end - begin + 1;
	split3.before = str.substr(0, begin);
	split3.middle = str.substr(begin, extract_size);
	split3.after = str.substr(end + 1, str.length());

	return split3;
}

std::vector<std::string> strextended::splitter(std::string delim, std::string& str) {
	std::vector<std::string> list;
	int n, last = 0;
	
	//Get all elements: arg(delim), and (delim)arg(delim)
	while( (n = str.find(delim, last)) != std::string::npos ) {
		n += delim.length() - 1;
		list.emplace_back( str.substr(last, n - last) );
		last = n + 1;
	}

	//Required to get the last element, i.e. ...(delim)last\0
	if( last != 0 ) 
		list.emplace_back( str.substr(last, str.length()) );
	else
		list.emplace_back(str);	//Only one arg

	return list;
}

std::string strextended::removechar(char delim, std::string& str) {
	std::string s;
	for( unsigned int i = 0; i < str.length(); i++ ) {
		//We only want characters that AREN'T delim
		if( str[i] != delim ) {
			s += str[i];
		}
	}

	return s;
}

std::vector<std::string> strextended::ripn(unsigned int n, std::string &str) {
	std::vector<std::string> list;
	unsigned int offset = 0;

	while( offset < str.length() ) {
		std::string segment = str.substr(offset, n);
		list.emplace_back(segment);
		offset += n;
	}

	return list;
}

std::string strextended::replacestr(std::string &raw, const std::string search, const std::string replace) {
	size_t pos = 0;
    while ((pos = raw.find(search, pos)) != std::string::npos) {
         raw.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return raw;
}

inline bool strextended::c_isalnum(char c) {
	bool is_alnum = 
		(c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
		
		return is_alnum;
}

bool strextended::isalnum(const std::string &str) {	
	for( unsigned int i = 0; i < str.length(); i++ ) {
		if( !c_isalnum(str[i]) )
			return false;
	}

	return true;
}

int strextended::find(int occur, char c, const char *str, unsigned int len) {	
	int s_occur = 0;

	for( unsigned int i = 0; i < len; i++ ) {
		if( str[i] == c) 
			++s_occur;
		if( occur == s_occur )
			return i;
	}
	
	return -1;
}

std::string strextended::upsidedown(const std::string &str) {
	std::string flipped;	

	//For each character in str
	for( unsigned int i = 0; i < str.length(); i++ ) {
		bool found = false;
		//Try to match it to another char
		for( unsigned int j = 0; j < sizeof(normal_upsidedownstrings); j++ ) {
			if( str[i] == normal_upsidedownstrings[j] ) {
				//We found a match!
				flipped += upsidedownstrings[j];
				found = true;
				break; //Break out of the match finding loop
			}
		}

		if( !found )
			flipped += str[i];
	}

	return flipped;
}

std::string strextended::reverse(const std::string &str) {
	std::string rev;
	unsigned int i;

	for( i = str.length() - 1; i > 0; i-- )
		rev += str[i];

	rev += str[0];
	return rev;
}