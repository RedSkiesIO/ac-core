#ifndef USGOV_b05bd77af998c823ed16a99e5cd5a0faa0ef03d000b57ee63148072bf9a65c79
#define USGOV_b05bd77af998c823ed16a99e5cd5a0faa0ef03d000b57ee63148072bf9a65c79

#include "wallet_api.h"
#include "wallet.h"

namespace us{ namespace wallet{
using namespace std;

class wallet_local_api: public wallet_api, public us::wallet::wallet {
public:
    wallet_local_api(const string& homedir, const string& backend_host, uint16_t backend_port);
    virtual ~wallet_local_api();
    using wallet::add_address;
    using wallet_api::hash_t;
    using wallet_api::cash_t;
    using wallet_api::tx_make_p2pkh_input;

#include <us/api/apitool_generated__functions_wallet_cpp_override> //APITOOL

    void gen_keys(ostream&os);
    static void priv_key(const priv_t& privkey, ostream&);
    void mine_public_key(const string& pattern, ostream&);

    bool connect_backend(ostream&os);

private:
    chrono::steady_clock::time_point m_connected_since;
    socket::peer_t m_endpoint;
};

}}

#endif



