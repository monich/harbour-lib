/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gutil_log.h"

#include <dlfcn.h>
#include <stdint.h>

#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/md5.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

/*
 * What is this code?
 *
 * Sailfish OS comes in 3 flavors wrt openssl versions:
 *
 * 1. Sailfish OS up to 3.4 had openssl 1.0
 * 2. Sailfish 4.0..4.2  had both openssl 1.0 and openssl 1.1 libraries
 * 3. Sailfish 4.3 dropped openssl 1.0 libraries and only has openssl 1.1
 *
 * Which means that explicitly linking with any version of openssl
 * makes your app incompatible with either Sailfish OS <= 3.4 or
 * Sailfish OS >= 4.3 (nice symmetry).
 *
 * If you want to build an app which runs on both Sailfish OS <= 3.4
 * and Sailfish OS >= 4.3 you have two options:
 *
 * 1. Link openssl statically
 * 2. Load libcrypts.so dynamically with dlopen/dlsym
 *
 * This module allows you to implement the second strategy. It works
 * because openssl 1.0 and 1.1 (luckily!) appear to be binary compatible.
 * It doesn't load all libcrypto functions, only those referenced by my
 * apps but the list can be easily expanded.
 */

#ifndef LIBDIR
#  ifdef __aarch64__
#    define LIBDIR "/usr/lib64"
#  else
#    define LIBDIR "/usr/lib"
#  endif
#endif

/* f(name,params,args) */
#define LIBCRYPTO_FUNCTIONS1(f) \
    f(AES_cbc_encrypt, \
     (const unsigned char* in, unsigned char* out, size_t length, \
      const AES_KEY* key, unsigned char* ivec, const int enc), \
     (in, out, length, key, ivec, enc)) \
    f(AES_cfb128_encrypt, \
     (const unsigned char* in, unsigned char* out, size_t length, \
      const AES_KEY* key,unsigned char* ivec, int* num, const int enc), \
     (in, out, length, key, ivec, num, enc)) \
    f(AES_ecb_encrypt, \
     (const unsigned char* in, unsigned char* out, const AES_KEY* key, \
      const int enc), \
     (in, out, key, enc)) \
    f(BN_free, (BIGNUM *a), (a)) \
    f(DES_ede3_cbc_encrypt, \
     (const unsigned char* input, unsigned char* output, long length, \
      DES_key_schedule* ks1, DES_key_schedule* ks2, DES_key_schedule* ks3, \
      DES_cblock* ivec, int enc), \
     (input, output, length, ks1, ks2, ks3, ivec, enc)) \
    f(RSA_free, (RSA* r), (r))

/* f(ret,name,params,args,def) */
#define LIBCRYPTO_FUNCTIONS2(f) \
    f(int, AES_set_decrypt_key, \
     (const unsigned char* userKey, const int bits, AES_KEY* key), \
     (userKey, bits, key), -1) \
    f(int, AES_set_encrypt_key, \
     (const unsigned char* userKey, const int bits, AES_KEY* key), \
     (userKey, bits, key), -1) \
    f(BIGNUM*, BN_bin2bn, \
     (const unsigned char* s, int len, BIGNUM* ret), (s, len, ret), NULL) \
    f(int, BN_bn2bin, \
     (const BIGNUM* a, unsigned char* to), (a, to), 0) \
    f(BIGNUM*, BN_new, (void), (), NULL) \
    f(int, BN_num_bits, (const BIGNUM *a), (a), 0) \
    f(int, BN_set_word, (BIGNUM* a, BN_ULONG w), (a, w), 0) \
    f(int, DES_check_key_parity, (const_DES_cblock* key), (key), 0) \
    f(int, DES_is_weak_key, (const_DES_cblock* key), (key), 0) \
    f(int, DES_set_key, \
     (const_DES_cblock* key, DES_key_schedule* schedule), \
     (key, schedule), -1) \
    f(char*, ERR_error_string, (unsigned long e, char* buf), (e, buf), NULL) \
    f(unsigned long, ERR_get_error, (void), (), 0) \
    f(unsigned char*, MD5, \
     (const unsigned char* d, size_t n, unsigned char* md), (d, n, md), NULL) \
    f(int, MD5_Final, (unsigned char* md, MD5_CTX* c), (md, c), 0) \
    f(int, MD5_Init, (MD5_CTX* c), (c), 0) \
    f(int, MD5_Update, (MD5_CTX* c, const void* data, size_t len), \
     (c, data, len), 0) \
    f(int, OPENSSL_init_crypto, (uint64_t opts, void* settings), \
     (opts, settings), 0) \
    f(int, RAND_bytes, (unsigned char* buf, int num), (buf, num), 0) \
    f(int, RAND_poll, (void), (), 0) \
    f(RSA*, RSAPrivateKey_dup, (RSA* rsa), (rsa), NULL) \
    f(RSA*, RSAPublicKey_dup, (RSA* rsa), (rsa), NULL) \
    f(int, RSA_generate_key_ex, \
     (RSA* rsa, int bits, BIGNUM* e, BN_GENCB* cb), \
     (rsa, bits, e, cb), 0) \
    f(RSA*, RSA_new, (void), (), NULL) \
    f(int, RSA_private_decrypt, \
     (int flen, const unsigned char* from, unsigned char* to, RSA* rsa, \
      int padding), (flen, from, to, rsa, padding), 0) \
    f(int, RSA_private_encrypt, \
     (int flen, const unsigned char* from, unsigned char* to, RSA* rsa, \
      int padding), (flen, from, to, rsa, padding), 0) \
    f(int, RSA_public_decrypt, \
     (int flen, const unsigned char* from, unsigned char* to, RSA* rsa, \
      int padding), (flen, from, to, rsa, padding), 0) \
    f(int, RSA_public_encrypt, \
     (int flen, const unsigned char* from, unsigned char* to, RSA* rsa, \
      int padding), (flen, from, to, rsa, padding), 0) \
    f(int, RSA_size, (const RSA* rsa), (rsa), 0) \
    f(unsigned char*, SHA1, (const unsigned char* d, size_t n, \
      unsigned char* md), (d, n, md), NULL) \
    f(int, SHA1_Final, (unsigned char* md, SHA_CTX* c), (md, c), 0) \
    f(int, SHA1_Init, (SHA_CTX* c), (c), 0) \
    f(int, SHA1_Update, (SHA_CTX* c, const void* data, size_t len), \
     (c, data, len), 0) \
    f(unsigned char*, SHA256, (const unsigned char* d, size_t n, \
      unsigned char* md), (d, n, md), NULL) \
    f(int, SHA256_Final, (unsigned char* md, SHA256_CTX* c), (md, c), 0) \
    f(int, SHA256_Init, (SHA256_CTX* c), (c), 0) \
    f(int, SHA256_Update, (SHA256_CTX* c, const void* data, size_t len), \
     (c, data, len), 0) \
    f(unsigned char*, SHA512, (const unsigned char* d, size_t n, \
      unsigned char* md), (d, n, md), NULL) \
    f(int, SHA512_Final, (unsigned char* md, SHA512_CTX* c), (md, c), 0) \
    f(int, SHA512_Init, (SHA512_CTX* c), (c), 0) \
    f(int, SHA512_Update, (SHA512_CTX* c, const void* data, size_t len), \
     (c, data, len), 0)

static const char* libcrypto_so_path[] = {
    LIBDIR "/libcrypto.so.1.1",
    "/lib/libcrypto.so.10"
};

static const char* libcrypto_names[] = {
    #define FN_NAME1(name,params,args) #name,
    #define FN_NAME2(ret,name,params,args,def) #name,
    LIBCRYPTO_FUNCTIONS1(FN_NAME1)
    LIBCRYPTO_FUNCTIONS2(FN_NAME2)
};

static struct {
    void* handle;
    union {
        struct libcrypto_proc {
            #define FN_PTR1(name,params,args) void (* name) params;
            #define FN_PTR2(ret,name,params,args,def) ret (* name) params;
            LIBCRYPTO_FUNCTIONS1(FN_PTR1)
            LIBCRYPTO_FUNCTIONS2(FN_PTR2)
        } crypto;
        void* entry[G_N_ELEMENTS(libcrypto_names)];
    } fn;
} libcrypto;

G_STATIC_ASSERT(G_N_ELEMENTS(libcrypto_names) == \
    G_N_ELEMENTS(libcrypto.fn.entry));

static
void
libcrypto_load(void)
{
    static gboolean failed = FALSE;

    if (!libcrypto.handle && !failed) {
        unsigned int i;

        for (i = 0; i < G_N_ELEMENTS(libcrypto_so_path); i++) {
            const char* lib = libcrypto_so_path[i];

            libcrypto.handle = dlopen(lib, RTLD_LAZY);
            if (libcrypto.handle) {
                GINFO("Loaded %s", lib);
                for (i = 0; i < G_N_ELEMENTS(libcrypto_names); i++) {
                    const char* fn = libcrypto_names[i];
                    void* f = dlsym(libcrypto.handle, fn);

                    if (G_LIKELY(f)) {
                        libcrypto.fn.entry[i] = f;
                    } else {
                        GWARN("%s not found in %s", fn, lib);
                    }
                }
                break;
            } else {
                GWARN("%s not found", lib);
            }
        }
        failed = !libcrypto.handle;
    }
}

#define FN_IMPL1(name,params,args) void name params { libcrypto_load(); \
    if (libcrypto.fn.crypto.name) libcrypto.fn.crypto.name args; }
#define FN_IMPL2(ret,name,params,args,def) ret name params { libcrypto_load(); \
    return libcrypto.fn.crypto.name ? libcrypto.fn.crypto.name args : def; }
LIBCRYPTO_FUNCTIONS1(FN_IMPL1)
LIBCRYPTO_FUNCTIONS2(FN_IMPL2)
