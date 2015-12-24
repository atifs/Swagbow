/*
	Created: 4/28/2014
	Author: Cyrus
	Purpose: Helper functions that go well with std::string
*/

#ifndef __STREXTENDED_H
#define __STREXTENDED_H

#include <string>
#include <vector>

namespace strextended {	

	//checks if haystack starts with needle
	bool startswith(std::string needle, std::string &haystack);

	//split3 struct
	struct split3 {
		std::string before, middle, after;
	};

	//returns the first case of a string found splitted into 3 strings, before, middle, after
	split3 splatter(std::string find_prefix, std::string find_postfix, std::string &str);

	//splits a string by delim
	std::vector<std::string> splitter(std::string delim, std::string &str);

	//removes all delim from a string
	std::string removechar(char delim, std::string &str);

	//splits string every n characters
	std::vector<std::string> ripn(unsigned int n, std::string &str); 

	//finds search in raw and replaces it with replace
	std::string replacestr(std::string &raw, const std::string search, const std::string replace);

	//checks if a char is alphanumeric
	inline bool c_isalnum(char c);

	//checks if string is alphanumeric
	bool isalnum(const std::string &str);

	//gets n occurence of delim, returns -1 if nothing found
	int find(int occur, char c, const char *str, unsigned int len);

	//convert a string to html mirrored about the horizontal axis
	std::string upsidedown(const std::string &str);

	//reverse a string
	std::string reverse(const std::string &str);
};

#endif