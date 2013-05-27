#include <string.h>

#include "error.h"

using namespace TimePass;

int Error::errno = 0;
char Error::errmsg[MSG_LEN] = "";

void Error::SetErrno(int no) {
	Error::errno = no;
}

void Error::SetErrmsg(const char* errmsg) {
	strncpy(Error::errmsg, errmsg, MSG_LEN - 1);
}

int Error::GetLastErrno() {
	return errno;
}

const char* Error::GetLastErrmsg() {
	return errmsg;
}

