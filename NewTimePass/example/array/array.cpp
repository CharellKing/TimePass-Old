#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt
#include <iostream>

#include "global/error.h"
#include "shm/shm_array.h"


using namespace std;

const string Label(const int& num) {
    char t_num[128];
    snprintf(t_num, 127, "%d", num);
    return string(t_num);
}

void ToDotPs(const char* name, const TimePass::ShmArray<int>* p_v) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_v->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
	//system("rm -rf rbtree.dot");
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

void Create(off_t len) {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Write() {
	int method = 0;
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}

	srand(time(NULL));
	int count = rand() % 5 + 1, pos = 0, number = 0;							//产生1～3之间的随机数
	
	for(int i = 0; i < count; ++i) {
		method = rand() % 2;							//产生0，1 随机数									
		if (0 == method) {
			if (false == numbers.Insert(pos = rand() % (numbers.Size() + 1), number = rand() % 100)) {
				printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
					TimePass::Error::GetLastErrmsg());
				return ;
			}
			printf("<insert %d>", pos);
		} else {
			if (false == numbers.PushBack(number = rand() % 100)) {
				printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
					TimePass::Error::GetLastErrmsg());
				return ;
			}
			printf("<push_back>");
		}
		printf("%d ", number);
	}
	printf("共插入数据的条数%d\n", count);
}

void Remove() {
	int method = 0;
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}

	srand(time(NULL));
	int count = rand() % 5 + 1, pos = 0, number = 0;							//产生1～3之间的随机数
	
	for(int i = 0; i < count; ++i) {
		method = rand() % 2;							//产生0，1 随机数									
		if (0 == method) {
			if (false == numbers.Remove(pos = rand() % (numbers.Size() + 1))) {
				printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
					TimePass::Error::GetLastErrmsg());
				return ;
			}
			printf("<remove %d>", pos);
		} else {
			if (false == numbers.PopBack()) {
				printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
					TimePass::Error::GetLastErrmsg());
				return ;
			}
			printf("<pop_back>");
		}
	}
	printf("共插入删除的条数%d\n", count);
}

void Read() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	for(off_t i = 0; i < numbers.Size(); ++ i) {
		printf("%d ", *numbers.At(i));
	}
	printf("\n");
}

void Show() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("array", &numbers);	
}

void Clear() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	numbers.Clear();
}

void About() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
    cout << "name = " << numbers.Name() << " capacity = " << numbers.Capacity() << " size = " << numbers.Size() << " total_size = " << numbers.TotalSize() << "bytes used_size = " << numbers.UsedSize() << "byts" << endl;
}

void Optimize() {
	TimePass::ShmArray<int> numbers("/tmp/array");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
    if (false == numbers.Optimize()) {
        printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ; 
    }
}

int main(int argc, char** argv) {
	int result = getopt(argc, argv, "oabewrdsc:");
	if (-1 == result) {
		printf("usage:\n"
                 "-o for optimize\n"
                 "-a for info\n"
			     "-c [capacity] for create\n"
			     "-d for destroy\n"
			     "-w for write\n"
			     "-r for read\n"
                 "-b for remove\n"
				 "-s for dot\n"
				 "-e for clear\n");
		return 0;
	}

	switch(result) {
        case 'o':
            Optimize();
            break;
        case 'a':
            About();
            break;
        case 'b':
            Remove();
            break;
		case 'c':
			Create(Convert(optarg));
			break;
		case 'd':
			Destroy();
			break;
		case 'w':
			Write();
			break;
		case 'r':
			Read();
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

