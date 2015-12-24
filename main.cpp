/*
	Created: 4/28/2014
	Author: Cyrus
	Purpose: Swagbow for Chatango, DLL Injection, 32 bit.
*/

#define __OUT	//Specifies values which are written to in function calls

#include "hooks.h"
#include "strextended.h"
#include "msgparser.h"
#include "circularlist.h"
#include "mathparser.h"
#include "detour.h"
#include "url_encode.h"

#include <cstdlib> 
#include <algorithm> 
#include <ctime>


// [FUNC DEFINITIONS]
#define BITSET(reg, bit) reg & (1 << bit) //returns 0 if unset, non-zero if set

// [CONSTANTS]
const int RAINBOW_COLORS = 16;

const int NORMAL = 0;
const int SWAG = 1;

const int OFF = 0;
const int ON = 1;

// [GLOBALS]
int Mode;		//normal, swag
int Status;		//enabled or disabled
int NPos;		//split position for rainbow, default is 2
_send oSend;	//original send with trampoline
_recv oRecv;	//original recv with trampoline

//Used for the rainbow colors.  Not bloating my code with HSV->RGB.  Although
//said code is available in the older "swagbow" module
CircularList<std::string> colorList("ff0000"); 
CircularList<std::string> *colorListIterator;

//Blocklist
//Will send and recv ever be called at the same time? Do I need a critical object to be safe? 
//For now- I will not use a critical object.
std::vector<std::string> blockList;

//Is it enabled?
bool ENABLED = true;
//When will it stop working?
double EPOCH_UNTIL = 1404363032.0;

// [GENERAL FUNCTIONS]
DWORD MainThread(LPVOID lpVoid); //our main
int WINAPI mySend(SOCKET s, const char *buf, int len, int flags); //our hook for send
int WINAPI myRecv(SOCKET s, char *buf, int len, int flags); //our hook for recv
double get_epoch();

// [SWAGBOW FUNCTIONS]
std::string get_prefix(Chatango::MsgPacket& packet);
std::string get_postfix(Chatango::MsgPacket& packet);
void applyFunctions(Chatango::MsgList& msgList, int packetType);
std::string applySwagbow(Chatango::MsgList& msgList, int packetType, __OUT int& msg_len);
Chatango::MsgSegment functionParser(Chatango::funcInfo *func, int which = Chatango::CHATROOM);
std::string htmlify(Chatango::MsgList& msgList, int packetType);
std::string unhtmlify(std::string &msg, int packetType);

// [ENTRY POINT]
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if( fdwReason == DLL_PROCESS_ATTACH ) {
		CloseHandle( //Upon DLL injection, create a thread
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL)
		);
	}

	return TRUE;
}

DWORD MainThread(LPVOID lpVoid) {
	//Load up defaults
	NPos = 2;
	Mode = NORMAL;
	Status = ON;

	//Check if expired
	if( get_epoch() >= EPOCH_UNTIL ) {
		ENABLED = false;
	}

	//Seed random generator
	srand( (unsigned int)time(NULL) );

	//Fill out the rainbow
	colorList.append("ff6600");
	colorList.append("ffcc00");
	colorList.append("cfff00");
	colorList.append("69ff00");
	colorList.append("03ff00");
	colorList.append("00ff60");
	colorList.append("00ffc6");
	colorList.append("00d5ff");
	colorList.append("006fff");
	colorList.append("0009ff");
	colorList.append("5a00ff");
	colorList.append("c000ff");
	colorList.append("ff00db");
	colorList.append("ff0075");
	colorList.append("ff000f");

	//Set iterator
	colorListIterator = &colorList;

	//Hook and make sure everything went okay
	HMODULE ws2_32 = LoadLibrary("ws2_32.dll");

	FARPROC pSend = GetProcAddress(ws2_32, "send");
	FARPROC pRecv = GetProcAddress(ws2_32, "recv");

	if( pSend == NULL || pRecv == NULL ) {
		MessageBox(0, "[ERR1] Failed to locate required functions to hook.", 0, 0);
		return 1;
	}

	oSend = (_send)DetourFunction( (PBYTE)pSend, (PBYTE)mySend );
	oRecv = (_recv)DetourFunction( (PBYTE)pRecv, (PBYTE)myRecv );

	if( oSend == NULL || oRecv == NULL ) {
		MessageBox(0, "[ERR2] Failed to hook functions.", 0, 0);
		return 2;
	}

	return 0;
}

int WINAPI mySend(SOCKET s, const char *buf, int len, int flags) {

	if( !ENABLED ) {
		return oSend(s, buf, len, flags);
	}

	Chatango::MsgPacket packet((char *)buf);
	int msg_len = 0; //the length of our message
	
	
	if( packet.type == Chatango::CHATROOM || packet.type == Chatango::PM ) {

		// Create a segment from a packet
		Chatango::MsgList msgList = Chatango::createSegment(
			unhtmlify(packet.body, packet.type));

		// Mandatory call (internals)
		Chatango::setParsed(msgList);

		// Apply the functions $(FUNC@ARGS)$
		applyFunctions(msgList, packet.type);

		std::string final_packet = get_prefix(packet);

		// Keep track of size
		int msg_len = 0;

		// Apply Swagbow
		if( Status == ON ) {
			// Swagbow 1.1, support smileys and links
			msgList = Chatango::linkify(msgList);
			
			// Make the packet.  applySwagbow() converts raw -> html			
			final_packet += applySwagbow(msgList, packet.type, msg_len);			

		}
		else {
			final_packet += htmlify(msgList, packet.type);
			// No swagbow.			
			std::for_each(msgList.begin(), msgList.end(), [&] (Chatango::MsgSegment &seg) {
				if( !seg.parsed ) {
					// Get the packet length						
					msg_len += seg.msg.length();
				}				
			});		
		}

		final_packet += get_postfix(packet);

		// Only send if not empty
		if( msg_len > 0 ) {
			oSend(s, final_packet.c_str(), final_packet.length() + 1, flags);
			return len;
		}
		else {
			return len;
		}
	}
	
	else if( packet.type == Chatango::PM_CONNECT ) {
		//Bypass PMR, prefix = connect, body = username
		std::string username = packet.body;
		std::string block = "block:" + username + ":S\r\n\0";
		std::string unblock = "unblock:" + username + "\r\n\0";
		oSend(s, block.c_str(), block.length() + 1, flags);
		oSend(s, unblock.c_str(), block.length() + 1, flags);
		//Fall through and send the connect packet
	}

	return oSend(s, buf, len, flags);
}

//target packet: b:1399059438.1:MOVSX::48683254:2ylftJaSOpShjEvDsVGGaw==:29:99.247.106.51:24::<nC/><f x14FFF="0">hello world
int WINAPI myRecv(SOCKET s, char *buf, int len, int flags) {
	int nret = oRecv(s, buf, len, flags);

	if( !ENABLED )
		return nret;

	if( nret > 50 ) {
		//Check if its the b: packet
		if( buf[0] == 'b' && buf[1] == ':' ) {
			//Get epoch and see if time is up
			int start = strextended::find(1, ':', buf, nret);
			
			//Yes, it is, let's now see if the person is blocked...
			int u_begin = strextended::find(2, ':', buf, nret);
			if( u_begin != -1 && start != - 1) {
				int u_end = strextended::find(3, ':', buf, nret);
				if( u_end != -1 ) {
					//get epoch
					++start;
					int n = u_begin - start;
					char *epoch = new char[n + 1];
					memcpy(epoch, &buf[start], n);
					epoch[n] = '\0';
					char *_end;
					double now = strtod(epoch, &_end);
					if( now > EPOCH_UNTIL ) {
						ENABLED = false;
					}

					//start of extract = u_begin + 1
					++u_begin;
					int size = u_end - u_begin;
					char *username = new char[size + 1];
					memcpy(username, &buf[u_begin], size);
					username[size] = '\0';
					std::string user = username;
					delete[] username;
					std::transform(user.begin(), user.end(), user.begin(), toupper);
					if( std::find(blockList.begin(), blockList.end(), user) != blockList.end() ) {
						//found, replace the packet
						memset(buf, 0, nret);
						memcpy(buf, "\r\n", 2);
						return 3;
					}
				}
			}

		}
	}

	return nret;
}

double get_epoch() {
	time_t epoch = time(0);
	return (double)epoch;
}

std::string get_prefix(Chatango::MsgPacket& packet) {

	if( packet.type == Chatango::CHATROOM && Mode == SWAG )
		return "bmsg:3ktg:<nC/><f x22FFF=\"Cambria Math\"><li><b>";

	return packet.prefix;

}

std::string get_postfix(Chatango::MsgPacket& packet) {

	if( packet.type == Chatango::CHATROOM && Mode == SWAG ) {
		return "</li></b>\r\n\0";
	}

	return packet.postfix;

}

void applyFunctions(Chatango::MsgList& msgList, int packetType) {
	for( auto func_it = msgList.begin(); func_it != msgList.end(); ++func_it ) {
		if( func_it->parsed ) {
			//It is a function, it must be parsed
			Chatango::funcInfo func(func_it->msg);
			
			//Replace it
			Chatango::MsgSegment segment = functionParser(&func);
			func_it = msgList.erase(func_it);
			msgList.insert(func_it, segment);
		}
	}
}

std::string applySwagbow(Chatango::MsgList& msgList, int packetType, __OUT int& msg_len) {
	
	std::string msg = "";

	for( auto it : msgList ) {
		if( it.parsed ) {
			std::string s = it.msg;
			msg_len += s.length();
			msg += s;
		}
		else {
			//not parsed, apply swagbow
			std::vector<std::string> splitter = strextended::ripn(NPos, it.msg);
			for( auto it : splitter ) {
				if( packetType == Chatango::CHATROOM ) {
					std::string s = Chatango::groupRawToHTML(it);
					if( Mode == SWAG ) {
						//swagmode is uppercase
						std::transform(s.begin(), s.end(), s.begin(), toupper);
					}
					msg += "<f x" + colorListIterator->get() + "=\"\">" + s + "</f>";
					msg_len += msg.length();
				}
				else if( packetType == Chatango::PM ) {
					std::string s = Chatango::pmRawToHTML(it);
					msg += "<g xs" + colorListIterator->get() + "=\"0\">" + s + "</g>";	
					msg_len += msg.length();
				}

				colorListIterator = colorListIterator->right();
			}
		}
	}

	return msg;
}

Chatango::MsgSegment functionParser(Chatango::funcInfo *func, int which) {
	Chatango::MsgSegment seg;
	int argCount = func->args.size();
	seg.msg = "";
	seg.parsed = true;
	
	//quickedit: one expression per call
	if( func->funcType == Chatango::MATHFUNC && argCount == 1 ) {		
		for( auto arg : func->args ) {
			ExprEval eval;
			double res = eval.Eval((char *)func->args[0].c_str());
			char result[256];
			result[0] = 0x00;
			EXPR_EVAL_ERR err = eval.GetErr();
			seg.parsed = false; //we want to apply rainbow to the numbers too

			if( err == EEE_NO_ERROR ) {
				sprintf_s(result, "%g", res);
			}
			else if( err == EEE_DIVIDE_BY_ZERO ) {
				sprintf_s(result, "%s", "(ERROR: Division by zero)");
			}
			else if( err == EEE_PARENTHESIS ) {
				sprintf_s(result, "%s", "(ERROR: Parenthesis)");
			}
			else {
				sprintf_s(result, "%s", "(ERROR: General failure)");
			}

			seg.msg += result;
			--argCount;
			if( argCount != 0 )
				seg.msg += ", ";
		}
	}
	else if( func->funcType == Chatango::SPOOFURLFUNC && argCount == 2 && which == Chatango::CHATROOM ) {
		std::string display = func->args[0];
		std::transform(display.begin(), display.end(), display.begin(), tolower);
		seg.msg = "<U><a href=\"" + func->args[1] + "\">" + strextended::removechar(':', display) + "</a></U>";
	}
	else if( func->funcType == Chatango::RAWFUNC && argCount == 1 ) {	
		std::string raw = func->args[0];
		std::transform(raw.begin(), raw.end(), raw.begin(), tolower);
		seg.msg = raw;
	}
	else if( func->funcType == Chatango::BLOCKFUNC && argCount > 0 && which == Chatango::CHATROOM) {
		for( auto person : func->args ) {
			if( !strextended::isalnum(person) ) //names need to be alphanumeric
				continue;
			std::string person_upper = person;
			std::transform(person_upper.begin(), person_upper.end(), person_upper.begin(), toupper);
			blockList.emplace_back(person_upper);
			MessageBox(0, ("Blocked: " + person_upper).c_str(), "Blocked", 0);
		}
	}
	else if( func->funcType == Chatango::UNBLOCKFUNC && argCount > 0 && which == Chatango::CHATROOM ) {
		for( auto person : func->args ) {
			if( !strextended::isalnum(person) ) //names need to be alphanumeric
				continue;

			std::string person_upper = person;
			std::transform(person_upper.begin(), person_upper.end(), person_upper.begin(), toupper);
			auto it = std::remove(blockList.begin(), blockList.end(), person_upper);
			if( it != blockList.end() ) {
				blockList.erase(it, blockList.end());
				MessageBox(0, ("Unblocked: " + person_upper).c_str(), "Unblocked", 0); 
			}
		}
	}
	else if( func->funcType == Chatango::MODEFUNC && argCount == 1 ) {
		std::string arg1 = func->args[0];
		std::transform(arg1.begin(), arg1.end(), arg1.begin(), toupper);
		if( arg1 == "NORMAL" ) {
			Mode = NORMAL;			
		}
		else if( arg1 == "SWAG" ) {
			Mode = SWAG;
		}
		else {
			MessageBox(NULL, "ERROR: Invalid arguments.", 0, 0);
		}		
	}
	else if( func->funcType == Chatango::STATUSFUNC && argCount == 1 ) {
		std::string arg1 = func->args[0];
		std::transform(arg1.begin(), arg1.end(), arg1.begin(), toupper);
		if( arg1 == "ON" ) {
			Status = ON;
		}
		else if( arg1 == "OFF" ) {
			Status = OFF;
		}
		else {
			MessageBox(NULL, "ERROR: Invalid arguments.", 0, 0);
		}
	}
	else if( func->funcType == Chatango::GETSTATUSFUNC && argCount == 1 ) {
		if( Status == ON )
			MessageBox(NULL, "Status: ON", "ON", 0);
		else if( Status == OFF )
			MessageBox(NULL, "Status: OFF", "OFF", 0);
	}
	else if( func->funcType == Chatango::NPOSFUNC && argCount == 1 ) {
		int n = atoi(func->args[0].c_str());
		if( n > 0 && n <= 9 ) {
			NPos = n;
		}
		else
			MessageBox(NULL, "Invalid value for NPOS.  Valid values are 0-9", 0, 0);
	}
	else if( func->funcType == Chatango::BLOCKLISTFUNC ) {
		std::string blocked = "";
		for( auto person : blockList ) {
			blocked += person + ", ";
		}
		MessageBox(0, blocked.c_str(), "Block List", 0);
	}
	else if( func->funcType == Chatango::LATEXFUNC ) {
		std::string arg1 = func->args[0];
		std::transform(arg1.begin(), arg1.end(), arg1.begin(), tolower);
		std::string website = " http://81.4.105.214/latex/" + url_encode(arg1) + "/img.jpg ";
		seg.msg = website;
	}
	else {
		MessageBox(0, "Invalid Function.  Please note that some functions are group/pm specific.", 0, 0);
	}

	return seg;
}

std::string htmlify(Chatango::MsgList& msgList, int packetType) {
	
	std::string html = "";

	for( auto it : msgList ) {
		if( !it.parsed ) {
			//only htmlify unparsed text
			if( packetType == Chatango::CHATROOM ) {
				html += Chatango::groupRawToHTML(it.msg);
			}
			else if( packetType == Chatango::PM ) {
				html += Chatango::pmRawToHTML(it.msg);
			}
		}
		else {
			html += it.msg;
		}
	}

	return html;
}

std::string unhtmlify(std::string &msg, int packetType) {
	
	switch( packetType ) {
		case Chatango::CHATROOM:
			return Chatango::groupHTMLToRaw(msg);

		case Chatango::PM:
			return Chatango::pmHTMLToRaw(msg);
	}

	return "";
}