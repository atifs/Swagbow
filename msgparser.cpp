/*
	Created: 4/28/2014
	Author: Cyrus
	Purpose: Parse chatango packets

	Chatroom packet example:
		bmsg:gwm3:<nC/><f x14FFF="0">hello world\r\n\0
	Chatroom regex:
		(bmsg:.+?<f .+?>)(.*?)\r\n\0
*/

#include "msgparser.h"
#include "strextended.h"

const static std::string urls[] = {
	"http://", "https://"
};

const static std::string emotes[] = {
	"*stop*", ":P", ":|", ":(", ":x", ":o", ":D", ":@", ";(", ";)", "8)", "(:", "*pukes*", "*h*", "*hb*", "*blush*", "*waves*", "~V", "o_o)", "*bored*", "*burger*", "*lol*", "*rolleyes*", ":)", "*snow*", "8|8", "><>", "}|", "[~]", "(~)", "(o_o", "~p", "~dd", "~d", ":*", "=_=", "(o_o)", "-O_O-", "*:)", "(=)", "{b)", "0:)", "]:)", "*bball*", "*soccer*", "*$*", "*cat*", "*monkey*", "*panda*", "*dog*", "*poop*"
};


Chatango::MsgPacket::MsgPacket(char *data) {
	std::string holder = data;
	prefix = "";
	body = "";
	postfix = "";
		
	if( strextended::startswith("bmsg:", holder) ) {
		//We sent a message in a chatroom
		type = Chatango::CHATROOM;

		std::tr1::cmatch res; //Our match will be stored here
		//std::tr1::regex rx("(bmsg:.+?<f .+?>)(.*?)\\r\\n");
		//Include <b> tags etc		
		std::tr1::regex rx("(bmsg:.+?<f .+?>(?:<[BIUbiu]>)*)(.*?)((?:<\\/[BIUbiu]>)*\\r\\n)");
		
		if( !std::tr1::regex_search(holder.c_str(), res, rx) ) {
			//Something went wrong
			type = Chatango::INVALID;
			return;
		}

		//Regex got matches
		prefix = res[1].str();
		body = res[2].str();
		postfix = res[3].str();		

	}
	else if( strextended::startswith("msg:", holder) ) {
		//We sent a message in a PM
		type = Chatango::PM;
		std::tr1::regex rx("(msg:.+\">)(.*?)(<\\/g>.+?\\r\\n)");
		std::tr1::cmatch res;
		if( !std::tr1::regex_search(holder.c_str(), res, rx) ) {
			//Something went wrong
			type = Chatango::INVALID;
			return;
		}

		//Regex got matches
		prefix = res[1].str();
		body = res[2].str();
		postfix = res[3].str();
	}
	else if( strextended::startswith("connect:", holder) ) {
		//on connect, use block packet to bypass PMR
		type = Chatango::INVALID;
		int u_start = strextended::find(1, ':', holder.c_str(), holder.length());
		int u_end = strextended::find(1, '\r', holder.c_str(), holder.length());
		if( u_end != -1 && u_start != -1 ) {
			prefix = "connect";
			++u_start;
			body = holder.substr(u_start, u_end - u_start);
			type = Chatango::PM_CONNECT;
		}		
	}
	else {
		//This isn't a message packet
		type = Chatango::INVALID;
	}
}

Chatango::MsgPacket::~MsgPacket() {
	
}

Chatango::MsgList Chatango::createSegment(std::string body) {
	MsgList list;
	
	strextended::split3 split3 = strextended::splatter("$(", ")$", body);
	if( split3.middle != body ) {
		//there was a match
		MsgList pre, middle, post;

		//keep going
		if( split3.before != "" )
			pre = createSegment(split3.before);
		if( split3.middle != "" )
			middle = createSegment(split3.middle);
		if( split3.after != "" )
			post = createSegment(split3.after);

		//concat vectors
		if( pre.size() != 0 )
			list.insert( list.end(), pre.begin(), pre.end() );
		if( middle.size() != 0 )
			list.insert( list.end(), middle.begin(), middle.end() );
		if( post.size() != 0 )
			list.insert( list.end(), post.begin(), post.end() );
	}
	else {
		//cant go any further
		MsgSegment seg = {false, body};
		list.emplace_back(seg);
	}

	return list;
}

void Chatango::setParsed(Chatango::MsgList& list) {
	//set true for command elements so they arent splattered into smaller strings
	for( auto it = list.begin(); it != list.end(); ++it ) {
		if( strextended::startswith("$(", it->msg) )
			it->parsed = true;
		else
			it->parsed = false;
	}

}

Chatango::funcInfo::funcInfo(std::string& funcstr) {
	std::tr1::cmatch res;
	funcType = INVALIDFUNC;

	//make string uppercase
	std::transform(funcstr.begin(), funcstr.end(), funcstr.begin(), toupper);

	if( std::tr1::regex_search(funcstr.c_str(), res, FuncRegex) ) {	
		//The syntax was valid, $1 holds functype, $2 holds args
		std::string func = res[1].str();
		//Remove all spaces, so that "arg1, arg2" is the same as "arg1,arg2" etc
		std::string argl = strextended::removechar(' ', res[2].str());		

		for( int i = 0; i < sizeof(FuncNames); i++ ) {
			if( FuncNames[i] == func ) {
				funcType = i;
				args = strextended::splitter(",", argl);
				break;
			}
		}	

	}
}

std::string Chatango::groupHTMLToRaw(std::string &html) {
	std::string raw = strextended::replacestr(html, "&amp;", "&");
	raw = strextended::replacestr(html, "&lt;", "<");
	raw = strextended::replacestr(html, "&gt;", ">");
	raw = strextended::replacestr(html, "&quot;", "\"");
	raw = strextended::replacestr(html, "&apos;", "'");
	return raw;
}

std::string Chatango::groupRawToHTML(std::string &raw) {
	std::string html = strextended::replacestr(raw, "&", "&amp;");
	html = strextended::replacestr(raw, "<", "&lt;");
	html = strextended::replacestr(raw, ">", "&gt;");
	html = strextended::replacestr(raw, "\"", "&quot;");
	html = strextended::replacestr(raw, "'", "&apos;");
	return html;
}

std::string Chatango::pmHTMLToRaw(std::string &html) {
	std::string raw = strextended::replacestr(html, "&lt;", "<");
	raw = strextended::replacestr(html, "&gt;", ">");	
	return raw;
}

std::string Chatango::pmRawToHTML(std::string &raw) {
	std::string html = strextended::replacestr(raw, "<", "&lt;");
	html = strextended::replacestr(raw, ">", "&gt;");
	return html;
}

bool Chatango::find_first(std::string &subject, int &position, int &length) {
	int pos = 9999;
	int identifier = 9999;	

	for( int i = 0; i < 2; ++i ) {
		std::string url = urls[i];
		int n = subject.find(url);
		if( n != std::string::npos ) {
			if( n == 0 ) {
				position = n;
				//try to find a space delim
				int space_pos = subject.find(' ', n);
				if( space_pos != std::string::npos ) {
					//space
					length = space_pos - n;
					return true;
				}
				else {
					//assume whole length
					length = subject.length() - n;
					return true;
				}
			}
			else {
				if( n < pos ) {
					pos = n;
					identifier = -1;
				}
			}
		}
	}

	for(int i = 0; i < 51; ++i ) {
		std::string emote = emotes[i];
		int n = subject.find(emote);
		if( n != std::string::npos ) {
			if( n == 0 ) {
				position = n;
				identifier = i;
				length = emotes[identifier].length();
				return true;
			}
			else {
				if( n < pos ) {
					pos = n;
					identifier = i;
				}
			}
				
		}
	}
	
	if( pos != 9999 ) {
		position = pos;
		if( identifier > 0 ) {
			//emote			
			length = emotes[identifier].length();
			return true;
		}
		else {
			//url
			//try to find a space delim
			int space_pos = subject.find(' ', pos);
			if( space_pos != std::string::npos ) {
				//space
				length = space_pos - pos;
				return true;
			}
			else {
				//assume whole length
				length = subject.length() - pos;
				return true;
			}
		}
	}

	return false;
}

std::vector<Chatango::MsgSegment> Chatango::str_to_seg(std::string &str) {
	std::vector<MsgSegment> ret;
	std::string before = "", middle = "", after = "";
	int pos, len;

	if( find_first(str, pos, len) ) {
		if( pos == 0 ) {
			middle = str.substr(0, len);
			after = str.substr(pos + len, str.length());
		}
		else {
			before = str.substr(0, pos);
			middle = str.substr(pos, len);			
		}

		if( unsigned(pos + len) < str.length() )
			after = str.substr(pos + len, str.length());
	}
	else {		
		MsgSegment seg = {false, str};
		ret.emplace_back(seg);
		return ret;
	}

	if( before != "" ) {
		MsgSegment seg = {false, before};
		ret.emplace_back(seg);
	}
	if( middle != "" ) {
		MsgSegment seg = {true, middle};
		ret.emplace_back(seg);
	}
	if( after != "" ) {
		std::vector<MsgSegment> nret = str_to_seg(after);
		if( nret.size() != 0 ) {
			std::vector<MsgSegment> fret;
			fret.reserve( ret.size() + nret.size() );
			fret.insert( fret.end(), ret.begin(), ret.end() );
			fret.insert( fret.end(), nret.begin(), nret.end() );
			return fret;
		}
		else {
			MsgSegment seg = {false, after};
			ret.emplace_back(seg);
		}
	}

	return ret;
}

std::vector<Chatango::MsgSegment> Chatango::linkify(std::vector<MsgSegment> &list) {
	std::vector<MsgSegment> ret;	

	for( auto _list : list ) {
		if( _list.parsed ) {
			ret.emplace_back(_list);
		}
		else {
			std::string msg = _list.msg;			
			int n = msg.find("https://www.youtube.com");
			if( n != std::string::npos ) {
				msg.replace(n, 5, "http");
			}
			std::vector<MsgSegment> mlist = str_to_seg(msg);
			ret.insert(ret.end(), mlist.begin(), mlist.end());
		}
	}

	return ret;
}