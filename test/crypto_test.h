#ifndef USGOV_296d255860d7eb1d7f99f43acfe346f819a5817d87ded20155e78515525d26be
#define USGOV_296d255860d7eb1d7f99f43acfe346f819a5817d87ded20155e78515525d26be

#include <string>
#include <us/gov/crypto/ec.h>
#include <stdio.h>

    typedef us::gov::crypto::ec::keys keys;
    bool test_symmetric_encryption();
    bool test_encrypt_decrypt_keys(std::string, const keys::priv_t&, const keys::pub_t&, const keys::priv_t&, const keys::pub_t&);
    bool test_encrypt_decrypt(std::string);
    bool test_encrypt_multiple(std::string);
    bool test_decrypt_multiple(std::string);

#endif
