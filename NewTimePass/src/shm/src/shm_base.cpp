#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sstream>

#include "error.h"

#include "shm_base.h"

using namespace TimePass;

char* ShmBase::CreateShm(const char* name, off_t length, mode_t mode) {
	int fd = shm_open(name, O_CREAT | O_RDWR, mode);
	if (-1 == fd) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}

	int ret = ftruncate(fd, length);
	if(-1 == ret) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}

	char* p_addr = (char*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if ((char*)-1 == p_addr) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}
	close(fd);
	return p_addr;
}

char* ShmBase::AttachShm(const char* name, bool is_readonly) {
	int o_flag = O_RDONLY;
	if (!is_readonly) {
		o_flag = O_RDWR;
	}
	int fd = shm_open(name, o_flag, 0x777);
	if (-1 == fd) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}

	struct stat st;
	if (-1 == fstat(fd, &st)) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}
	int prot = PROT_READ;
	if (!is_readonly) {
		prot |= PROT_WRITE;
	}

	char* p_addr = (char*)mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);
	if ((char*)-1 == p_addr) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}
	close(fd);
	return p_addr;
}

bool ShmBase::DetachShm(char* p_addr, off_t length) {
    if (-1 == munmap(p_addr, length)) {
        Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
        return false;        
    }
    return true;
}

bool ShmBase::Commit(char* p_addr, off_t length, bool is_sync) {
    int flag = MS_INVALIDATE;
    if (is_sync) {
        flag |= MS_SYNC;
    } else {
        flag |= MS_ASYNC;
    }
    if (-1 == msync (p_addr, length, flag)) {
        Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
        return false;
    }
    return true;
}

bool ShmBase::Resize(const char* name, off_t length) {
	int fd = shm_open(name, O_RDWR, 0x777); 
	if (-1 == fd) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return false;
	}
    
    if (-1 == ftruncate(fd, length)) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return false;    
    }
    return true;
}

struct stat* ShmBase::Stat(const char* name, struct stat* s) {
	int fd = shm_open(name, O_RDWR, 0x777); 
	if (-1 == fd) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;
	}
    
    if (-1 == fstat(fd, s)) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return NULL;    
    }
    return s;    
}

bool ShmBase::DestroyShm(const char* name) {
	int ret = shm_unlink(name);
	if (-1 == ret) {
		Error::SetErrno(errno);
		Error::SetErrmsg(strerror(errno));
		return false;
	}
	return true;
}

