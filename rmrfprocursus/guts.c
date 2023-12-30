//
//  guts.c
//  rmrfprocursus
//
//  Created by Nick Chan on 31/12/2023.
//

#include "guts.h"

void NSLog(CFStringRef, ...);

int jailbreak_get_bmhash(char* hash)
{
  const UInt8 *bytes;
  CFIndex length;
  io_registry_entry_t chosen = IORegistryEntryFromPath(0, "IODeviceTree:/chosen");
  if (!MACH_PORT_VALID(chosen)) return 1;
  CFDataRef manifestHash = (CFDataRef)IORegistryEntryCreateCFProperty(chosen, CFSTR("boot-manifest-hash"), kCFAllocatorDefault, 0);
  IOObjectRelease(chosen);
  if (manifestHash == NULL || CFGetTypeID(manifestHash) != CFDataGetTypeID())
  {
    if (manifestHash != NULL) CFRelease(manifestHash);
    return 1;
  }
  length = CFDataGetLength(manifestHash);
  bytes = CFDataGetBytePtr(manifestHash);
  for (int i = 0; i < length; i++)
  {
    snprintf(&hash[i * 2], 3, "%02X", bytes[i]);
  }
  CFRelease(manifestHash);
  return 0;
}

int jailbreak_get_prebootPath(char jbPath[150]) {
    struct utsname name;
    int ret = uname(&name);
    if (ret) return errno;
    if (atoi(name.release) < 20) return ENOTSUP;
    char bmhash[97];
    jailbreak_get_bmhash(bmhash);
    snprintf(jbPath, 150, "/private/preboot/%s", bmhash);
    DIR* dir = opendir(jbPath);
    if (!dir) {
        return ENOENT;
    }
    char jbPathName[20];
    bool has_prebootjb = false;
    struct dirent* d;
    while ((d = readdir(dir)) != NULL) {
        if (strncmp(d->d_name, "jb-", 3)) continue;
        if (has_prebootjb == true) {
            closedir(dir);
            return EEXIST;
        }
        snprintf(jbPathName, 20, "%s" ,d->d_name);
        has_prebootjb = true;
    }
    closedir(dir);
    if (!has_prebootjb) return ENOENT;
    snprintf(jbPath, 150, "/private/preboot/%s/%s/procursus", bmhash, jbPathName);
    struct stat st;
    if ((stat(jbPath, &st))) {
        return ENXIO;
    }
    if (!S_ISDIR(st.st_mode)) {
        return ENOTDIR;
    }
    return KERN_SUCCESS;
}


int remount(void) {
    struct statfs fs;
    int ret = statfs("/private/preboot", &fs);
    if (ret == ENOENT) return 0;
    if (ret) return ret;
    int mntflags = MNT_UPDATE;
    apfs_mount_args_t arg = { fs.f_mntfromname, MNT_UPDATE, APFS_MOUNT_FILESYSTEM };
    return mount(fs.f_fstypename, fs.f_mntonname, mntflags, &arg);
}

int32_t doit_forreal(void) {
    int ret = remount();
    if (ret) {
        NSLog(CFSTR("Failed to remount preboot"));
        return ret;
    }
    char bmhash[97], prebootPath[150], jbPaths[150];
    ret = jailbreak_get_bmhash(bmhash);
    if (ret) {
        NSLog(CFSTR("Failed to get boot-manifest-hash"));
        return ret;
    }
    snprintf(prebootPath, 150, "/private/preboot/%s", bmhash);
    DIR* dir = opendir(prebootPath);
    if (!dir) return 0;
    struct dirent* d;
    while ((d = readdir(dir)) != NULL) {
        if (strncmp(d->d_name, "jb-", 3)) continue;
        snprintf(jbPaths, 150, "%s/%s", prebootPath, d->d_name);
        if ((strcmp(jbPaths, prebootPath) == 0)) {
            NSLog(CFSTR("disaster averted (tried to delete preboot)"));
            continue;
        }
        NSLog(CFSTR("delete %s\n"), jbPaths);
        ret = removefile(jbPaths, NULL, REMOVEFILE_RECURSIVE);
        if (ret && errno != ENOENT) {
            NSLog(CFSTR("Failed to delete %s"), jbPaths);
        }
    }
    NSLog(CFSTR("delete /var/jb\n"));
    ret = removefile("/var/jb", NULL, REMOVEFILE_RECURSIVE);
    if (ret && errno != ENOENT) {
        printf("Failed to delete /var/jb");
        return ret;
    }
    NSLog(CFSTR("Success."));
    return 0;
}

char* waitpid_decode(int status) {
    char* retbuf = calloc(50, 1);
    assert(retbuf);
    
    if (WIFEXITED(status)) {
        snprintf(retbuf, 50, "exited with code %d", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        if (WCOREDUMP(status))
            snprintf(retbuf, 50, "terminated by signal %d (Core Dumped)", WTERMSIG(status));
        else
            snprintf(retbuf, 50, "terminated by signal %d", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        snprintf(retbuf, 50, "stopped by signal %d", WTERMSIG(status));
    }
    
    return retbuf;
}

char* doit(void) {
    int pipefd[2];
    int ret;
    pipe(pipefd);
    
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_set_persona_np(&attr, 99, POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE);
    posix_spawnattr_set_persona_uid_np(&attr, 0);
    
    char* buffer = calloc(1, 1024);
    if (!buffer) {
        return NULL;
    }
    
    char path[MAXPATHLEN];
    uint32_t maxpathlen = MAXPATHLEN;
    ret = _NSGetExecutablePath(path, &maxpathlen);
    
    if (ret) {
        snprintf(buffer, 1024, "_NSGetExecutablePath() failed");
        return buffer;
    }
    
    
    int pid, status;
    ret = posix_spawnp(&pid, path, NULL, &attr, (char*[]){ path, NULL }, environ);
    posix_spawnattr_destroy(&attr);
    
    if (ret) {
        snprintf(buffer, 1024, "spawn %s failed: %d (%s)", path, ret, strerror(ret));
        return buffer;
    }
    
    waitpid(pid, &status, 0);
    
    if (status == 0) {
        snprintf(buffer, 1024, "%s", "Success.");
    } else {
        char* statusStr = waitpid_decode(status);
        snprintf(buffer, 1024, "%s %s", path, statusStr);
    }
    
    return buffer;
}
