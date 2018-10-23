#ifndef US_GOV_WALLET_DAEMON_LOCAL_API_H
#define US_GOV_WALLET_DAEMON_LOCAL_API_H

#include "daemon_api.h"
#include "wallet_local_api.h"
#include "pairing_local_api.h"

namespace us{ namespace wallet {
using namespace std;

class daemon_local_api: public daemon_api, public wallet_local_api, public pairing_local_api {
public:
    typedef wallet_local_api w;
    typedef pairing_local_api p;
    daemon_local_api(const string& home, 
                     const string& backend_host, 
                     uint16_t backend_port): 
                        w(home,backend_host,backend_port), p(home) {}
    virtual ~daemon_local_api() {}
    using w::pub_t;
    using w::hash_t;
    using w::cash_t;
    using w::tx_make_p2pkh_input;

#include <us/api/apitool_generated__functions_wallet-daemon_cpp_impl>  //APITOOL
};
}}

#endif


