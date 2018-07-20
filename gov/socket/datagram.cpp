/** @file */
#include "datagram.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <us/gov/likely.h>
#include <stdio.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"

using namespace std;
using namespace us::gov::socket;
typedef us::gov::socket::datagram c;

constexpr size_t c::h;
constexpr size_t c::maxsize;

struct kk {
int a;
};


/// \brief  Main function
/// \param  argc An integer argument count of the command line arguments
/// \param  argv An argument vector of the command line arguments
/// \return an integer 0 upon exit success
c::datagram(): dend(0) {
}

c::datagram(uint16_t service):service(service) {
	resize(h);
	encode_size(size());
	encode_service(service);
	dend=size();
}

c::datagram(uint16_t service, uint16_t payload):service(service) {
	resize(h+2);
	encode_size(size());
	encode_service(service);
	(*this)[h]=payload&0xff;
	(*this)[h+1]=payload>>8; //&0xff;
	dend=size();
}

c::datagram(uint16_t service, const string& payload):service(service) {
	assert(h+payload.size()<maxsize);
	resize(h+payload.size());
	encode_size(size());
	encode_service(service);
	::memcpy(&(*this)[h],payload.c_str(),payload.size());
	dend=size();
}

void c::encode_size(uint32_t sz) {
	assert(h==6);
	assert(size()>=h); //This is little-endian
	(*this)[0]=sz&0xff;
	(*this)[1]=sz>>8&0xff;
	(*this)[2]=sz>>16&0xff;
	(*this)[3]=sz>>24&0xff;
}

uint32_t c::decode_size() const {
	assert(size()>3);
	uint32_t sz=(*this)[0];
	sz|=(*this)[1]<<8;
	sz|=(*this)[2]<<16;
	sz|=(*this)[3]<<24;
	return sz;
}

void c::encode_service(uint16_t svc) {
	assert(h==6);
	assert(size()>=h);
	(*this)[4]=svc&0xff;
	(*this)[5]=svc>>8&0xff;
}

uint16_t c::decode_service() const {
	assert(size()>5);
	uint16_t svc=(*this)[4];
	svc|=(*this)[5]<<8;
	return svc;
}

bool c::completed() const {
//cout << "completed?: size=" << size() << " dend=" << dend << endl << " decodedsz=" << decode_size() << endl;
	return dend==size() && !empty();
}
/*
string c::recv(int sock, int timeout_seconds) {
	return recv(sock);
}
*/
string c::recv(int sock) {
	if (unlikely(sock==0)) {
        return "Error. Connection is closed.";
    }
    //static constexpr int response_timeout_secs={3};
/*
   	struct timeval tv;
    tv.tv_sec = 3; //timeout_seconds;
   	tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
*/

	if (dend<h) {
		if (size()<h) resize(h);
		ssize_t nread = ::recv(sock, &(*this)[dend], h-dend, 0);
		if (unlikely(nread<=0)) {
			if (errno==EINPROGRESS || errno==EAGAIN) { //https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
				cout << "socket: client: 1 EINPROGRESS fd" << sock  << endl;
//				error=4;
    			return "Timeout waiting for data from peer.";
			}
			else {
//				error=nread==0?1:2;
    			return "Connection ended by peer.";
			}
		}
		dend+=nread;
		if (dend<h) {
            return ""; //need to recv more
        }
		uint32_t sz=decode_size();
		if (sz>maxsize) {
			//error=3;
			return "Error. Incoming datagram is too big.";
		}
		resize(sz);
		service=decode_service();
		if (dend==sz) {
            return "";
        }
	}
	ssize_t nread = ::recv(sock, &(*this)[dend], size()-dend,0);
	if (nread<=0) {
		if (errno==EINPROGRESS || errno==EAGAIN) { //https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
			cout << "socket: client: EINPROGRESS fd" << sock  << endl;
//			error=4;
   			return "Error.3 Timeout waiting for data from peer.";
		}
		else {
//			error=nread==0?1:2;
			return "Error. Incoming datagram is too big.";
		}
	}
	dend+=nread;
	return "";
}

string c::send(int sock) const {
	if (unlikely(size()>=maxsize)) {
        return "Error. Datagram is too big.";
    }
	if (unlikely(sock==0)) {
        return "Error. Connection is closed.";
    }
	uint8_t sz[h];
	auto n = ::write(sock, &(*this)[0], size());
	if (unlikely(n<0)) {
		return "Error. Failure writting to socket.";
	}
	if (unlikely(n!=size())) {
		return "Error. Unexpected returning size while wrtting to socket";
	}
	return "";
}

c::hash_t c::compute_hash() const {
	hasher_t hasher;
	hasher.write(reinterpret_cast<const unsigned char*>(&*begin()),size());
	hasher_t::value_type v;
	hasher.finalize(v);
	return move(v);
}

vector<string> c::parse_strings() const {
	vector<string> ans;
	ans.reserve(100);
	const unsigned char *s=0;
	size_t sz=0;
	for (auto i=begin()+h; i!=end(); ++i) {
		if (!s) s=static_cast<const unsigned char*>(&*i);
		if (*i=='\n') {
			ans.emplace_back(string(reinterpret_cast<const char*>(s),sz));
			sz=0;
			s=0;
		}
		else ++sz;
	}
	if (s) {
		ans.emplace_back(string(reinterpret_cast<const char*>(s),sz));
	}
	return move(ans);
}

string c::parse_string() const {
	return string(reinterpret_cast<const char*>(&*(begin()+h)),size()-h);
}

uint16_t c::parse_uint16() const {
	if (size()!=h+2) return 0;
	uint16_t pl=(*this)[h];
	pl|=(*this)[h+1]<<8;
	return pl;
}

void c::dump(ostream&os) const {
os << "size " << size() << endl;
os << "service " << decode_service() << endl;
os << "payload size " << decode_size() << endl;
os << "payload as str " << parse_string() << endl;
os << "completed " << completed() << endl;

}




