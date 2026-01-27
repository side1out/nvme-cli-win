// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This file is part of nvme-cli
 *
 * Copyright (c) 2022 Daniel Wagner, SUSE
 */

#include <errno.h>
#include <stdlib.h>

#ifdef WINDOWS_GCC
#include <winsock2.h>
#include <windows.h>
#include "../subprojects/libnvme/src/libnvme.h"
#include "windows/compat.h"
#else
#include <dlfcn.h>
#include <libnvme.h>
#endif
#include "nvme-print.h"

#define PROTO(args...) args
#define ARGS(args...) args

#define VOID_FN(name, proto, args)					\

#ifdef WINDOWS_GCC

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Provided by the linker; points at the module base */
extern IMAGE_DOS_HEADER __ImageBase;
#ifdef __cplusplus
}
#endif

static inline HMODULE get_current_module(void) {
    return (HMODULE)&__ImageBase;
}

#ifndef NVME_REAL_DLL
#define NVME_REAL_DLL "libnvme_real.dll"
#endif

static HMODULE _nvme_real_handle = NULL;

static HMODULE _nvme_load_real(void) {
    if (_nvme_real_handle) return _nvme_real_handle;

    // Load from the same directory as the proxy DLL if possible
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA((HMODULE)&__ImageBase, path, MAX_PATH);
    if (n && n < MAX_PATH) {
        // replace filename with NVME_REAL_DLL
        char *slash = strrchr(path, '\\');
        if (slash) {
            ++slash;
            *slash = '\0';
            strncat(path, NVME_REAL_DLL, MAX_PATH - (slash - path) - 1);
            _nvme_real_handle = LoadLibraryA(path);
        }
    }
    if (!_nvme_real_handle) {
        // fallback to normal search path (same dir / PATH)
        _nvme_real_handle = LoadLibraryA(NVME_REAL_DLL);
    }
    return _nvme_real_handle;
}

static void* _nvme_getproc(const char* name) {
    HMODULE h = _nvme_load_real();
    if (!h) return NULL;
    return (void*)GetProcAddress(h, name);
}

// Provide a default error reporter stub if you don’t have one on Windows
#ifndef nvme_show_error
static void nvme_show_error(const char* msg) {
    MessageBoxA(NULL, msg, "nvme proxy error", MB_ICONERROR | MB_OK);
}
#endif

#define PROTO(args...) args
#define ARGS(args...)  args

#define VOID_FN(name, proto, args)                                \
void name(proto) {                                                \
    typedef void (__cdecl *fp_t)(proto);                          \
    static fp_t fp = NULL;                                        \
    if (!fp) {                                                    \
        fp = (fp_t)_nvme_getproc(#name);                          \
        if (!fp) {                                                \
            nvme_show_error("libnvme function " #name " not found"); \
            ExitProcess(EXIT_FAILURE);                            \
        }                                                         \
    }                                                             \
    fp(args);                                                     \
}

#define FN(name, rtype, proto, args, defret)                      \
rtype name(proto) {                                               \
    typedef rtype (__cdecl *fp_t)(proto);                         \
    static fp_t fp = NULL;                                        \
    if (!fp) {                                                    \
        fp = (fp_t)_nvme_getproc(#name);                          \
        if (!fp)                                                  \
            return (defret);                                      \
    }                                                             \
    return fp(args);                                              \
}
#else
void __attribute__((weak)) name(proto)					\
{									\
	void (*fn)(proto);						\
	fn = dlsym(RTLD_NEXT, #name);					\
	if (!fn) {							\
		nvme_show_error("libnvme function " #name " not found");\
		exit(EXIT_FAILURE);					\
	}								\
	fn(args);						 	\
}
#define FN(name, rtype, proto, args, defret)			\
rtype __attribute__((weak)) name(proto)				\
{								\
	rtype (*fn)(proto);					\
	fn = dlsym(RTLD_NEXT, #name);				\
	if (fn)							\
		return fn(args);				\
	return defret;						\
}

FN(nvme_get_version,
	const char *, PROTO(enum nvme_version type),
	ARGS(type), "n/a")

VOID_FN(nvme_init_copy_range_f1,
	PROTO(struct nvme_copy_range_f1 *copy, __u16 *nlbs,
	      __u64 *slbas, __u64 *eilbrts, __u32 *elbatms,
	      __u32 *elbats, __u16 nr),
	ARGS(copy, nlbs, slbas, eilbrts, elbatms, elbats, nr))

VOID_FN(nvme_init_copy_range_f2,
	PROTO(struct nvme_copy_range_f2 *copy, __u32 *snsids,
	      __u16 *nlbs, __u64 *slbas, __u16 *sopts, __u32 *eilbrts,
	      __u32 *elbatms, __u32 *elbats, __u16 nr),
	ARGS(copy, snsids, nlbs, slbas, sopts, eilbrts, elbatms, elbats, nr))

VOID_FN(nvme_init_copy_range_f3,
	PROTO(struct nvme_copy_range_f3 *copy, __u32 *snsids,
	      __u16 *nlbs, __u64 *slbas, __u16 *sopts, __u64 *eilbrts,
	      __u32 *elbatms, __u32 *elbats, __u16 nr),
	ARGS(copy, snsids, nlbs, slbas, sopts, eilbrts, elbatms, elbats, nr))

FN(nvme_get_feature_length2,
	int,
	PROTO(int fid, __u32 cdw11, enum nvme_data_tfr dir,
	      __u32 *len),
	ARGS(fid, cdw11, dir, len),
	-EEXIST)

FN(nvme_ctrl_is_persistent,
	bool,
	PROTO(nvme_ctrl_t c),
	ARGS(c),
	false)
#endif // win
