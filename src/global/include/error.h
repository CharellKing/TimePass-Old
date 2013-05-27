#ifndef _ERROR_H

#define _ERROR_H

#define MSG_LEN	256

//errno
#define NO_INDEX_EXCEED     19881001
#define NO_INDEX_NONEXIST   19881002
#define NO_CAPACITY_TOMAX   19881003
#define NO_KEY_EXIST        19881004
#define NO_SPACE_SHORTAGE   19881005

//errmsg
#define MSG_INDEX_EXCEED    "the index exceed"
#define MSG_INDEX_NONEXIST  "the index not exist"
#define MSG_CAPACITY_TOMAX  "the capacity reach to max"
#define MSG_KEY_EXIST       "the data with the key existed"
#define MSG_SPACE_SHORTAGE  "the space is shortage"

namespace TimePass {
class Error {
public:
	static void SetErrno(int no) ;
	static void SetErrmsg(const char* errmsg);
	static int GetLastErrno();
	static const char* GetLastErrmsg();
private:
	static int errno;
	static char errmsg[MSG_LEN];
};
};

#endif

