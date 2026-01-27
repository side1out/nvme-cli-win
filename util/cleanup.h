/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __CLEANUP_H
#define __CLEANUP_H

#include <stdlib.h>

#ifdef WINDOWS_GCC
#include "../subprojects/libnvme/src/libnvme.h"
#include "windows/types.h"
#include "windows/compat.h"
#else
#include <libnvme.h>
#include <unistd.h>
#endif

#include "util/mem.h"

#define __cleanup__(fn) __attribute__((cleanup(fn)))

#define DECLARE_CLEANUP_FUNC(name, type) \
	void name(type *__p)

#define DEFINE_CLEANUP_FUNC(name, type, free_fn)\
DECLARE_CLEANUP_FUNC(name, type)		\
{						\
	if (*__p)				\
		free_fn(*__p);			\
}
#ifdef WINDOWS_GCC
static inline void freep(void *p)
{
    void **pp = (void **)p;
    //fprintf(stderr, "freep: p=%p *p=%p\n", p, pp ? *pp : NULL);
    fflush(stderr);

    if (pp && *pp) {
        free(*pp);
        *pp = NULL;
    }
}
#else
static inline void freep(void *p)
{
	free(*(void **)p);
}
#endif
#define _cleanup_free_ __cleanup__(freep)

#define _cleanup_huge_ __cleanup__(nvme_free_huge)

static inline void cleanup_fd(int *fd)
{
	if (*fd > STDERR_FILENO)
		close(*fd);
}
#define _cleanup_fd_ __cleanup__(cleanup_fd)

static inline void cleanup_nvme_root(nvme_root_t *r)
{
	nvme_free_tree(*r);
}
#define _cleanup_nvme_root_ __cleanup__(cleanup_nvme_root)

static inline DEFINE_CLEANUP_FUNC(cleanup_nvme_ctrl, nvme_ctrl_t, nvme_free_ctrl)
#define _cleanup_nvme_ctrl_ __cleanup__(cleanup_nvme_ctrl)

#ifdef FBS
static inline void free_uri(struct nvme_fabrics_uri **uri)
{
	if (*uri)
		nvme_free_uri(*uri);
}
#define _cleanup_uri_ __cleanup__(free_uri)
#endif

static inline DEFINE_CLEANUP_FUNC(cleanup_file, FILE *, fclose)
#define _cleanup_file_ __cleanup__(cleanup_file)

#endif /* __CLEANUP_H */
