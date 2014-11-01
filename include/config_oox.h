/* config_host/config_oox.h.  Generated from config_oox.h.in by configure.  */
#ifndef CONFIG_OOX_H
#define CONFIG_OOX_H

/*

Which TLS backend to use for cryptographic operations.

*/

#if SUPD == 310
#define USE_TLS_OPENSSL 1
#define USE_TLS_NSS 0
#else	// SUPD == 310
#define USE_TLS_OPENSSL 0
#define USE_TLS_NSS 1
#endif	// SUPD == 310

#endif
