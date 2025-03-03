/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright (C) 2020 Intel Labs */

#pragma once

#include <mbedtls/x509_crt.h>
#include <stdint.h>

#include "sgx_arch.h"
#include "sgx_attest.h"

#define RA_TLS_EPID_API_KEY "RA_TLS_EPID_API_KEY"

#define RA_TLS_ALLOW_OUTDATED_TCB_INSECURE  "RA_TLS_ALLOW_OUTDATED_TCB_INSECURE"
#define RA_TLS_ALLOW_DEBUG_ENCLAVE_INSECURE "RA_TLS_ALLOW_DEBUG_ENCLAVE_INSECURE"

#define RA_TLS_MRSIGNER    "RA_TLS_MRSIGNER"
#define RA_TLS_MRENCLAVE   "RA_TLS_MRENCLAVE"
#define RA_TLS_ISV_PROD_ID "RA_TLS_ISV_PROD_ID"
#define RA_TLS_ISV_SVN     "RA_TLS_ISV_SVN"

#define RA_TLS_IAS_PUB_KEY_PEM "RA_TLS_IAS_PUB_KEY_PEM"
#define RA_TLS_IAS_REPORT_URL  "RA_TLS_IAS_REPORT_URL"
#define RA_TLS_IAS_SIGRL_URL   "RA_TLS_IAS_SIGRL_URL"

#define RA_TLS_CERT_TIMESTAMP_NOT_BEFORE "RA_TLS_CERT_TIMESTAMP_NOT_BEFORE"
#define RA_TLS_CERT_TIMESTAMP_NOT_AFTER  "RA_TLS_CERT_TIMESTAMP_NOT_AFTER"

#define SHA256_DIGEST_SIZE       32
#define PUB_KEY_SIZE_MAX         128 /* enough for the only currently supported algo (ECDSA-384) */
#define IAS_REQUEST_NONCE_LEN    32

#define OID(N) \
    { 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF8, 0x4D, 0x8A, 0x39, (N) }
static const uint8_t quote_oid[] = OID(0x06);
static const size_t quote_oid_len = sizeof(quote_oid);

typedef int (*verify_measurements_cb_t)(const char* mrenclave, const char* mrsigner,
                                        const char* isv_prod_id, const char* isv_svn);

/* internally used functions, not exported */
__attribute__ ((visibility("hidden")))
bool getenv_allow_outdated_tcb(void);

__attribute__ ((visibility("hidden")))
bool getenv_allow_debug_enclave(void);

__attribute__ ((visibility("hidden")))
int find_oid(const uint8_t* exts, size_t exts_len, const uint8_t* oid, size_t oid_len,
             uint8_t** val, size_t* len);

__attribute__ ((visibility("hidden")))
int cmp_crt_pk_against_quote_report_data(mbedtls_x509_crt* crt, sgx_quote_t* quote);

__attribute__ ((visibility("hidden")))
int verify_quote_body_against_envvar_measurements(const sgx_quote_body_t* quote_body);

__attribute__ ((visibility("hidden")))
int ra_tls_verify_callback(void* data, mbedtls_x509_crt* crt, int depth, uint32_t* flags);

__attribute__ ((visibility("hidden")))
int ra_tls_create_key_and_crt(mbedtls_pk_context* key, mbedtls_x509_crt* crt);

/*!
 * \brief Callback for user-specific verification of measurements in SGX quote.
 *
 * \param f_cb  Callback for user-specific verification; RA-TLS passes pointers to MRENCLAVE,
 *              MRSIGNER, ISV_PROD_ID, ISV_SVN measurements in SGX quote. Use NULL to revert to
 *              default behavior of RA-TLS.
 *
 * \returns 0 on success, specific error code (negative int) otherwise.
 *
 * If this callback is registered before RA-TLS session, then RA-TLS verification will invoke this
 * callback to allow for user-specific checks on SGX measurements reported in the SGX quote. If no
 * callback is registered (or registered as NULL), then RA-TLS defaults to verifying SGX
 * measurements against `RA_TLS_*` environment variables (if any).
 */
__attribute__ ((visibility("default")))
void ra_tls_set_measurement_callback(verify_measurements_cb_t f_cb);

/*!
 * \brief Generic verification callback for EPID-based (IAS) or ECDSA-based (DCAP) quote
 *        verification (DER format).
 *
 * \param der_crt       Self-signed RA-TLS certificate with SGX quote embedded in DER format.
 * \param der_crt_size  Size of the RA-TLS certificate.
 *
 * \returns 0 on success, specific mbedTLS error code (negative int) otherwise.
 *
 * This function must be called from a non-mbedTLS verification callback, e.g., from a user-defined
 * OpenSSL callback for SSL_CTX_set_cert_verify_callback(). All parameters required for the SGX
 * quote, IAS attestation report verification, and/or DCAP quote verification must be passed in the
 * corresponding RA-TLS environment variables.
 */
__attribute__ ((visibility("default")))
int ra_tls_verify_callback_der(uint8_t* der_crt, size_t der_crt_size);

/*!
 * \brief Generic function to generate a key and a corresponding RA-TLS certificate (DER format).
 *
 * \param[out] der_key       Pointer to buffer populated with generated ECDSA keypair in DER format.
 * \param[out] der_key_size  Pointer to size of generated ECDSA keypair.
 * \param[out] der_crt       Pointer to buffer populated with self-signed RA-TLS certificate.
 * \param[out] der_crt_size  Pointer to size of self-signed RA-TLS certificate.
 *
 * \returns 0 on success, specific mbedTLS error code (negative int) otherwise.
 *
 * The function first generates a random ECDSA keypair with NIST P-384 (SECP384R1) elliptic curve.
 * Then it calculates the SHA256 hash over the generated public key and retrieves an SGX quote with
 * report_data equal to the calculated hash (this ties the generated certificate key to the SGX
 * quote). Finally, it generates the X.509 self-signed certificate with this key and the SGX quote
 * embedded. The function allocates memory for key and certificate; user is expected to free them
 * after use.
 */
__attribute__ ((visibility("default")))
int ra_tls_create_key_and_crt_der(uint8_t** der_key, size_t* der_key_size, uint8_t** der_crt,
                                  size_t* der_crt_size);
