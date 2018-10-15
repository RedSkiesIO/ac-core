/** @file */

#include "datagram.h"

#include <asm-generic/errno.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>

#include "../likely.h"

using namespace std;
using namespace us::gov::socket;

constexpr size_t datagram::h;
constexpr size_t datagram::maxsize;

datagram::datagram(): dend(0) {
}

datagram::datagram(uint16_t service):service(service) {
    resize(h);
    encode_size(size());
    encode_service(service);
    dend=size();
}

datagram::datagram(uint16_t service, uint16_t payload):service(service) {
    resize(h+2);
    encode_size(size());
    encode_service(service);
    (*this)[h]=payload&0xff;
    (*this)[h+1]=payload>>8; //&0xff;
    dend=size();
}

datagram::datagram(uint16_t service, const string& payload):service(service) {
    assert(h+payload.size()<maxsize);
    resize(h+payload.size());
    encode_size(size());
    encode_service(service);
    ::memcpy(&(*this)[h],payload.c_str(),payload.size()); //no trailing zro
    dend=size();
}

void datagram::encode_size(uint32_t sz) {
    assert(h==6);
    assert(size()>=h); //This is little-endian
    (*this)[0]=sz&0xff;
    (*this)[1]=sz>>8&0xff;
    (*this)[2]=sz>>16&0xff;
    (*this)[3]=sz>>24&0xff;
}

uint32_t datagram::decode_size() const {
    assert(size()>3);
    uint32_t sz=(*this)[0];
    sz|=(*this)[1]<<8;
    sz|=(*this)[2]<<16;
    sz|=(*this)[3]<<24;
    return sz;
}

void datagram::encode_service(uint16_t svc) {
    assert(h==6);
    assert(size()>=h);
    (*this)[4]=svc&0xff;
    (*this)[5]=svc>>8&0xff;
}

uint16_t datagram::decode_service() const {
    assert(size()>5);
    uint16_t svc=(*this)[4];
    svc|=(*this)[5]<<8;
    return svc;
}

bool datagram::completed() const {
    return dend==size() && !empty();
}

#ifdef SIM
/*
string datagram::sendto(int sock) const {
}
string datagram::recvfrom(int sock) {
}
*/
#else
string datagram::sendto(int sock) const {
    if (unlikely(size()>=maxsize)) {
        return "Error. Datagram is too big.";
    }

    if (unlikely(sock==0)) {
        return "Error. Connection is closed.";
    }

    auto n = ::write(sock, &(*this)[0], size());
    if (unlikely(n<0)) {
        return "Error. Failure writting to socket.";
    }

    if (unlikely(n!=size())) {
        return "Error. Unexpected returning size while wrtting to socket";
    }

    return "";
}

string datagram::recvfrom(int sock) {
    if (unlikely(sock==0)) {
        return "Error. Connection is closed.";
    }

    if (dend<h) {
        if (size()<h) resize(h);
        ssize_t nread = ::recv(sock, &(*this)[dend], h-dend, 0);
        if (unlikely(nread<=0)) {
            if (errno==EINPROGRESS || errno==EAGAIN) { //https://stackoverflow.com/questions/2876024
                return "Timeout waiting for data from peer.";
            }
            else {
                return "Connection ended by peer.";
            }
        }

        dend+=nread;
        if (dend<h) {
            return ""; //need to recv more
        }
        uint32_t sz=decode_size();
        if (sz>maxsize) {
            return "Error. Incoming datagram exceeded size limit.";
        }
        resize(sz);
        service=decode_service();
        if (dend==sz) {
            return "";
        }
    }

    ssize_t nread = ::recv(sock, &(*this)[dend], size()-dend,0);
    if (nread<=0) {
        if (errno==EINPROGRESS || errno==EAGAIN) { //https://stackoverflow.com/questions/2876024
            return "Error.3 Timeout waiting for data from peer.";
        }
        else {
            return "Error. Incoming datagram is too big.";
        }
    }
    dend+=nread;
    return "";
}
#endif

datagram::hash_t datagram::compute_hash() const {
    hasher_t hasher;
    hasher.write(reinterpret_cast<const unsigned char*>(&*begin()),size());
    hasher_t::value_type v;
    hasher.finalize(v);
    return move(v);
}

datagram::hash_t datagram::compute_payload_hash() const {
    hasher_t hasher;
    hasher.write(reinterpret_cast<const unsigned char*>(&*(begin()+h)),size()-h);
    hasher_t::value_type v;
    hasher.finalize(v);
    return move(v);
}

string datagram::parse_string() const {
    int sz=size()-h+1;
    if (unlikely(sz<2)) return ""; //minimum 2bytes for a non-empty c string
    char cstr[sz];
    memcpy(cstr,&*(cbegin()+h),sz-1);
    cstr[sz-1]='\0';
    return string(cstr);
}

uint16_t datagram::parse_uint16() const {
    if (size()!=h+2) return 0;
    uint16_t pl=(*this)[h];
    pl|=(*this)[h+1]<<8;
    return pl;
}

void datagram::dump(ostream&os) const {
    os << "size " << size() << endl;
    os << "service " << decode_service() << endl;
    os << "payload size " << decode_size() << endl;
    os << "payload as str " << parse_string() << endl;
    os << "completed " << completed() << endl;
}

