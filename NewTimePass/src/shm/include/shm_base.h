#ifndef _SHM_BASE_H

#define _SHM_BASE_H

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

#ifndef S_IRUSR
#define S_IRUSR 0100000000
#endif

#ifndef S_IWUSR
#define S_IWUSR 0010000000
#endif

#ifndef S_IRGRP
#define S_IRGRP 0001000000
#endif

#ifndef S_ROTH
#define S_ROTH  0000000100
#endif


namespace TimePass {
class ShmBase {
public:
	static char* CreateShm(const char* name, off_t length, mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_ROTH);
	static char* AttachShm(const char* name, bool is_readonly = false);
    static bool DetachShm(char* p_addr, off_t length);
    static bool Commit(char* p_addr, off_t length, bool is_sync = true);
    static bool Resize(const char* name, off_t length);
    static struct stat* Stat(const char* name, struct stat* s);
	static bool DestroyShm(const char* name);
};
};


#endif

