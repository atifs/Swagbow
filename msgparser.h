/*
	Created: 4/28/2014
	Author: Cyrus
	Purpose: Parse chatango packets
		Order:
			Intercept packet
			Convert to MsgPacket
			Convert to Segments
			Parse commands in segments
			Convert non-commands to text/rainbow, and convert <> etc into respective HTML tags
			Convert all data to raw data
			Send new packet

*/

#ifndef __MSGPARSER_H
#define __MSGPARSER_H

#include <string>
#include <vector>
#include <regex>

namespace Chatango {	

	//Used to identify the function, in form of $(FUNC@ARGS_CSV)
	const static std::tr1::regex FuncRegex("\\$\\((.+?)@(.+?)\\)\\$");

	//Used for function indexes
	enum {
		INVALIDFUNC = -1,
		MATHFUNC = 0, 
		SPOOFURLFUNC,
		RAWFUNC,
		BLOCKFUNC,
		UNBLOCKFUNC,
		MODEFUNC,
		STATUSFUNC,
		GETSTATUSFUNC,
		NPOSFUNC,	
		BLOCKLISTFUNC,	
		LATEXFUNC,
	};

	//Function names, with respect to indexes
	const static std::string FuncNames[] = {
		"MATH",				//For mathematics, i.e. parse 4+10/2
		"SPOOFURL",			//Spoof a url, i.e. bing->google (only for groups)
		"RAW",				//Display raw data (will not be parsed)
		"BLOCK",			//Ignore a user, hook recv (only for groups)
		"UNBLOCK",			//Don't ignore a user (only for groups)
		"MODE",				//Sets a mode.  I.e., NORMAL, SWAG (swag is only for groups)
		"STATUS",			//ON or OFF
		"GETSTATUS",		//Displays MODE and STATUS
		"NPOS",				//Defines how often rainbow should occur [1..]
		"BLOCKLIST",		//Shows a list of blocked users in the group	
		"LATEX",			//Show latex math expressions
	};

	//Identifiers for Packet type
	const static int CHATROOM = 0;
	const static int PM = 1;
	const static int PM_CONNECT = 2;
	const static int INVALID = 3;	

	//Used to store the message packet
	struct MsgPacket {

		MsgPacket(char *data);	//Converts a raw packet to a MsgPacket structure
		~MsgPacket();			//Cleanup

		int type;				//Chatroom or PM	
		std::string prefix;		//What comes before the body
		std::string body;		//The user message
		std::string postfix;	//What comes after the body

	};	

	//Contains a msg segment and a boolean used to check if it's parsed or not
	struct MsgSegment {
		bool parsed;			//Is the data parsed?
		std::string msg;		//The data
	};

	//A vector for MsgSegment used to store parsed data
	typedef std::vector<MsgSegment> MsgList;

	//Converts a packet into segments, call must be followed by setParsed;
	MsgList createSegment(std::string body);

	//Call this after createSegment to set parsed in MsgSegment
	void setParsed(MsgList &list);

	//Contains information about functions
	struct funcInfo {
		int funcType;			//enum value identifying function		
		std::vector<std::string> args;

		//initializes structure
		funcInfo(std::string &funcstr);
	};

	//converts special html such as &amp; to respective chars for groups
	std::string groupHTMLToRaw(std::string &html);

	//converts chars to respective HTML for groups
	std::string groupRawToHTML(std::string &raw);

	//converts special html such as &amp; to respective chars for groups
	std::string pmHTMLToRaw(std::string &html);

	//converts chars to respective HTML for groups
	std::string pmRawToHTML(std::string &raw);

	//used to add emote/link support
	bool find_first(std::string &subject, int &position, int &length);
	std::vector<MsgSegment> str_to_seg(std::string &str);
	std::vector<MsgSegment> linkify(std::vector<MsgSegment> &list);
}

#endif