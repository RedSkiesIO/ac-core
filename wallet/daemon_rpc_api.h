#ifndef US_GOV_WALLET_DAEMON_RPC_API_H
#define US_GOV_WALLET_DAEMON_RPC_API_H

#include "daemon_api.h"
#include "wallet_rpc_api.h"
#include "pairing_rpc_api.h"

namespace us{ namespace wallet{
using namespace std;

class daemon_rpc_api: public daemon_api, virtual public wallet_rpc_api, virtual public pairing_rpc_api {
public:
    typedef wallet_rpc_api w;
    typedef pairing_rpc_api p;
    daemon_rpc_api(const w::keys&k, 
                   const string& whost, 
                   uint16_t wport): w(k, whost, wport), 
                                    p(k, whost, wport), 
                                    rpc_api(k, whost, wport) {}
    virtual ~daemon_rpc_api() {}
    using w::pub_t;
    #include <us/api/apitool_generated__functions_wallet-daemon_cpp_impl> //APITOOL
};
}}

#endif



