/*  Copyright (c) 2022, Karsten Ohme
 *  This file is part of GlobalPlatform.
 *
 *  GlobalPlatform is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GlobalPlatform is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with GlobalPlatform.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "init.h"
#include <openssl/opensslv.h>
#if defined OPENSSL_VERSION_MAJOR && (OPENSSL_VERSION_MAJOR >= 3)
#define OPENSSL3
#include <stdlib.h>
#include <stdio.h>
#include <openssl/provider.h>

OSSL_PROVIDER *legacy;
OSSL_PROVIDER *deflt;

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#define LEGACY_DLL "legacy.dll"
#define OSSL_WIN32_DIR "C:\\Program Files (x86)\\OpenSSL-Win32\\bin\\"
#endif
#endif

/**
 * Checks the OpenSSL version and enables necessary legacy algorithms.
 */
CONSTRUCTOR void init(void) {
#ifdef OPENSSL3
#ifdef WIN32
	if (access(".\\" LEGACY_DLL, F_OK) == 0) {
		OSSL_PROVIDER_set_default_search_path(NULL, ".\\");
	} else if (access(OSSL_WIN32_DIR LEGACY_DLL, F_OK) == 0) {
		OSSL_PROVIDER_set_default_search_path(NULL, OSSL_WIN32_DIR);
	} else {
		printf("Could not find '%s' in current or '%s' directory.", LEGACY_DLL, OSSL_WIN32_DIR);
	}
#endif
	legacy = OSSL_PROVIDER_load(NULL, "legacy");
	if (legacy == NULL) {
		printf("Failed to load OpenSSL legacy provider\n");
		exit(EXIT_FAILURE);
	}
	deflt = OSSL_PROVIDER_load(NULL, "default");
	if (deflt == NULL) {
		printf("Failed to load OpenSSL default provider\n");
		OSSL_PROVIDER_unload(legacy);
		exit(EXIT_FAILURE);
	}
#endif
}

/**
 * Checks the OpenSSL version and unloads legacy algorithms.
 */
DESTRUCTOR void fini(void) {
#ifdef OPENSSL3
	OSSL_PROVIDER_unload(legacy);
	OSSL_PROVIDER_unload(deflt);
#endif
}

