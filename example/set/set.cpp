#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt

#include "global/error.h"
#include "shm/shm_set.h"


using namespace std;

const int array[] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};
const int len = sizeof(array) / sizeof(int);

int Compare(const int& x, const int& y) {
	if (x > y) return 1;
	if (x < y) return -1;
	return 0;
}

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

void ToDotPs(const char* name, const TimePass::ShmSet<int, Compare>* p_l) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_l->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
	//system("rm -rf rbtree.dot");
}


void Create(off_t len) {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Insert() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	char name[10];
	for(int i = 0; i < len; ++ i) {
		numbers.Insert(array[i]);
		snprintf(name, 9, "%02d", i);
		ToDotPs(name, &numbers);
	}
}

void Remove() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	char name[10];
	srand(time(NULL));
	int len = rand() % 10;
	for(int i = 0; i < len; ++ i) {
		numbers.Remove(rand() % 20);
	}
}

void Show() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("set", &numbers);	
}

void Clear() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	numbers.Clear();
}

void Optimize() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	if (false == numbers.Optimize()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
		return ;
	}
}

void Read() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	cout << "升序：";
    TimePass::RbtreeNode<int>* p_beg = numbers.Begin();
	while (NULL != p_beg) {
		cout << p_beg->data << " ";
		p_beg = numbers.Next(p_beg);
	}
	cout << endl;

	cout << "降序:";
    p_beg = numbers.RBegin();
	while (NULL != p_beg) {
		cout << p_beg->data << " ";
		p_beg = numbers.RNext(p_beg);
	}
	cout << endl;
}

void About() {
	TimePass::ShmSet<int, Compare> numbers("/tmp/set");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}

	cout << "capacity = " << numbers.Capacity() << " size = " << numbers.Size() << " total_size = " << numbers.TotalSize() << " used_size = " << numbers.UsedSize() << endl;

	if (false == numbers.DetachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
}

int main(int argc, char** argv) {
	int result = getopt(argc, argv, "baoeirdsc:");
	if (-1 == result) {
		printf("usage:\n"
			     "-c [capacity] for create\n"
				 "-a for about\n"
				 "-r for read\n"
				 "-o for optimize\n"
			     "-d for destroy\n"
			     "-i for write\n"
				 "-b for remove\n"
				 "-s for dot\n"
				 "-e for clear\n");
		return 0;
	}

	switch(result) {
		case 'a':
		    About();
			break;
		case 'r':
		    Read();
			break;
		case 'o':
		    Optimize();
			break;
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
		case 'b':
			Remove();
			break;
		case 'e':
			Clear();
			break;
	}
	return 0;
}


