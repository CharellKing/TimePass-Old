#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt

#include "global/error.h"
#include "shm/shm_multimap.h"


using namespace std;

int key[] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};
char value[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D','E', 'F', 'G', 'H', 'I', 'J'};

int len = sizeof(key) / sizeof(int);

int Compare(const int& x, const int& y) {
	if (x > y) return 1;
	if (x < y) return -1;
	return 0;
}

const string Label(const TimePass::ShmPair<int, char, Compare>& a) {
    char t_num[128];
    snprintf(t_num, 127, "%c", a.second);
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

void ToDotPs(const char* name, const TimePass::ShmMultimap<int, char, Compare>* p_l) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_l->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
	//system("rm -rf rbtree.dot");
}


void Create(off_t len) {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Insert() {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	char name[10];
	for(int i = 0; i < len; ++ i) {
		numbers.Insert(TimePass::ShmPair<int, char, Compare>(key[i], value[i]));
		snprintf(name, 9, "%02d", i);
		ToDotPs(name, &numbers);
	}
}

void Remove() {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	char name[10];
	for(int i = 0; i < len; ++ i) {
		numbers.Remove(key[i]);
		snprintf(name, 9, "%02d", i);
		ToDotPs(name, &numbers);
	}
}

void Show() {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("multimap", &numbers);	
}

void Clear() {
	TimePass::ShmMultimap<int, char, Compare> numbers("/tmp/multimap");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	numbers.Clear();
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


