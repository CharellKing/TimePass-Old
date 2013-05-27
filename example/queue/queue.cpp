#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt

#include "global/error.h"
#include "shm/shm_queue.h"


using namespace std;

const string Label(const int& num) {
    char t_num[128];
    snprintf(t_num, 127, "%d", num);
    return string(t_num);
}

off_t Convert(const char* digit) {
	int ret = -1;
	if (sizeof(off_t) == sizeof(long)) {
		ret = atol(digit);
	} else if (sizeof(off_t) == sizeof(long long)) {
		ret = atoll(digit);
	}
	return ret;
}

void ToDotPs(const char* name, const TimePass::ShmQueue<int>* p_l) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_l->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
	//system("rm -rf rbtree.dot");
}


void Create(off_t len) {
	TimePass::ShmQueue<int> numbers("/tmp/queue");
	if (false == numbers.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmQueue<int> numbers("/tmp/queue");
	if (false == numbers.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Write() {
	TimePass::ShmQueue<int> numbers("/tmp/queue");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}


	srand(time(NULL));
	int count = rand() % 100 + 1;							//产生1～20之间的随机数
	int number = 0, method = 0;
	for(int i = 0; i < count; ++i) {
        method = rand() % 2;
        if (0 == method) {
            number = rand() % 100;
            if (false == numbers.Push(number)) {
                printf("push 满\n");
                return ;
            }
            printf("push %d\n", number);
        } else {
            int* elem = numbers.Pop();
            if (elem) {
                printf("pop %d\n", *elem);
            } else {
                printf("pop 空\n");
            }
        }
	}
}

void Show() {
	TimePass::ShmQueue<int> numbers("/tmp/queue");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("queue", &numbers);	
}

void Clear() {
	TimePass::ShmQueue<int> numbers("/tmp/queue");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	numbers.Clear();
}

int main(int argc, char** argv) {
	int result = getopt(argc, argv, "ewdsc:");
	if (-1 == result) {
		printf("usage:\n"
			     "-c [capacity] for create\n"
			     "-d for destroy\n"
			     "-w for write\n"
			     "-r for read\n"
				 "-s for dot\n"
				 "-e for clear\n");
		return 0;
	}

	switch(result) {
		case 'c':
			Create(Convert(optarg));
			break;
		case 'd':
			Destroy();
			break;
		case 'w':
			Write();
			break;
		case 's':
			Show();
			break;
		case 'e':
			Clear();
			break;
	}
	return 0;
}

