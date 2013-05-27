#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt

#include "global/error.h"
#include "shm/shm_multihashset.h"
#include "shm/shm_hash.h"

using namespace std;

const char* t_month[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
int len = sizeof(t_month) / sizeof(char*);
const int  n_month[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

struct Month {
	Month(const char* t_month, int n_month) : n_month(n_month) {
		strcpy(this->t_month, t_month);
	}
	static int Compare(const Month& a, const Month& b) {
		return strcmp(a.t_month, b.t_month);
	}
	static off_t HashFunc(const Month& a) {
        return TimePass::Hash::HashFunc(a.t_month);
    }
	char t_month[16];
	int	 n_month;
};

const string Label(const Month& a) {
	return string(a.t_month);
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

void ToDotPs(const char* name, const TimePass::ShmMultihashset<Month>* p_l) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_l->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
}


void Create(off_t len) {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Insert() {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	for(int i = 0; i < 12; ++ i) {
		months.Insert(Month(t_month[i], n_month[i]));
	}
}

void Remove() {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	char name[10];
	for(int i = 0; i < len; ++ i) {
		months.Remove(Month(t_month[i], n_month[i]));
		snprintf(name, 9, "%02d", i);
		ToDotPs(name, &months);
	}
}

void Show() {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("multihashset", &months);	
}

void Clear() {
	TimePass::ShmMultihashset<Month> months("/tmp/multihashset");
	if (false == months.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	months.Clear();
}

int main(int argc, char** argv) {
	int result = getopt(argc, argv, "eirdsc:");
	if (-1 == result) {
		printf("usage:\n"
			     "-c [capacity] for create\n"
			     "-d for destroy\n"
			     "-i for write\n"
				 "-r for remove\n"
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
		case 'i':
			Insert();
			break;
		case 's':
			Show();
			break;
		case 'r':
			Remove();
			break;
		case 'e':
			Clear();
			break;
	}
	return 0;
}
