//
//  guts.h
//  rmrfprocursus
//
//  Created by Nick Chan on 31/12/2023.
//

#ifndef guts_h
#define guts_h

#include <stdio.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <removefile.h>
#include <sys/mount.h>
#include <spawn.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <mach-o/dyld.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>

enum {
    APFS_MOUNT_AS_ROOT = 0, /* mount the default snapshot */
    APFS_MOUNT_FILESYSTEM, /* mount live fs */
    APFS_MOUNT_SNAPSHOT, /* mount custom snapshot in apfs_mountarg.snapshot */
    APFS_MOUNT_FOR_CONVERSION, /* mount snapshot while suppling some representation of im4p and im4m */
    APFS_MOUNT_FOR_VERIFICATION, /* Fusion mount with tier 1 & 2, set by mount_apfs when -C is used (Conversion mount) */
    APFS_MOUNT_FOR_INVERSION, /* Fusion mount with tier 1 only, set by mount_apfs when -c is used */
    APFS_MOUNT_MODE_SIX,  /* ??????? */
    APFS_MOUNT_FOR_INVERT, /* ??? mount for invert */
    APFS_MOUNT_IMG4 /* mount live fs while suppling some representation of im4p and im4m */
};

/* Fourth argument to mount(2) when mounting apfs */
struct apfs_mount_args {
#ifndef KERNEL
    char* fspec; /* path to device to mount from */
#endif
    uint64_t apfs_flags; /* The standard mount flags, OR'd with apfs-specific flags (APFS_FLAGS_* above) */
    uint32_t mount_mode; /* APFS_MOUNT_* */
    uint32_t pad1; /* padding */
    uint32_t unk_flags; /* yet another type some sort of flags (bitfield), possibly volume role related */
    union {
        char snapshot[256]; /* snapshot name */
        struct {
            char tier1_dev[128]; /* Tier 1 device (Fusion mount) */
            char tier2_dev[128]; /* Tier 2 device (Fusion mount) */
        };
    };
    void* im4p_ptr;
    uint32_t im4p_size;
    uint32_t pad2; /* padding */
    void* im4m_ptr;
    uint32_t im4m_size;
    uint32_t pad3; /* padding */
    uint32_t cryptex_type; /* APFS_CRYPTEX_TYPE_* */
    int32_t auth_mode; /* APFS_AUTH_ENV_* */
    uid_t uid;
    gid_t gid;
}__attribute__((packed, aligned(4)));

typedef struct apfs_mount_args apfs_mount_args_t;
extern char** environ;

#define POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE 1
int posix_spawnattr_set_persona_np(const posix_spawnattr_t* __restrict, uid_t, uint32_t);
int posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t* __restrict, uid_t);
int posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t* __restrict, uid_t);

int jailbreak_get_prebootPath(char jbPath[150]);
int jailbreak_get_bmhash(char* hash);
int remount(void);
int32_t doit_forreal(void);
char* doit(void);

#endif /* guts_h */
