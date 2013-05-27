#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//getopt

#include "global/error.h"
#include "shm/shm_list.h"


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

void ToDotPs(const char* name, const TimePass::ShmList<int>* p_l) {
	char cmd[100];
	char file[100];
	snprintf(file, 99, "%s.dot", name);
	p_l->ToDot(file, Label);
	snprintf(cmd, 99, "dot -Tps %s.dot -o %s.ps && rm %s", name, name, file);
	system(cmd);
	//system("rm -rf rbtree.dot");
}


void Create(off_t len) {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.CreateShm(len)) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), TimePass::Error::GetLastErrmsg());
	}
}

void Destroy() {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.DestroyShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
	}
}

void Write() {
	int method = 0;
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}


	srand(time(NULL));
	int count = rand() % 5 + 1;							//产生1～3之间的随机数
	int number = 0, pos = 0;
	for(int i = 0; i < count; ++i) {								

        if (NULL == numbers.Insert(pos =rand() % (numbers.Size() + 1), number = rand() % 100)) {
			printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
				TimePass::Error::GetLastErrmsg());
			return ;
		}
        printf ("<insert %d>:", pos);
        printf("number = %d size = %d ", number, numbers.Size());
	}
	printf("共插入数据的条数%d\n", count);
}

void Remove() {
	int method = 0;
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}

	srand(time(NULL));
	int count = rand() % 5 + 1;							//产生1～3之间的随机数
	int number = 0, pos = 0;
    const TimePass::ListNode<int>* p_ret = NULL;
	for(int i = 0; i < count; ++i) {

        if ((p_ret = numbers.Remove(pos =rand() % (numbers.Size() + 1))) == NULL) {
            printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
                TimePass::Error::GetLastErrmsg());
            return ;
        }
        printf ("<remove %d>:", pos);
        printf("number = %d size = %d\n", p_ret->data, numbers.Size());
	}
	printf("共删除数据的条数%d\n", count);    
}

void Read() {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
    const TimePass::ListNode<int>* p_beg = numbers.HeadNode();
    while (p_beg) {
        printf("%d ", p_beg->data);
        p_beg = numbers.NextNode(p_beg);
    }
    putchar('\n');
}

void Show() {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	ToDotPs("list", &numbers);	
}

void Clear() {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
	numbers.Clear();
}


void About() {
	TimePass::ShmList<int> numbers("/tmp/list");
	if (false == numbers.AttachShm()) {
		printf("errno = %d, errmsg = %s\n", TimePass::Error::GetLastErrno(), 
			TimePass::Error::GetLastErrmsg());
		return ;
	}
    cout << "name = " << numbers.Name() << " capacity = " << numbers.Capacity() << " size = " << numbers.Size() << " total_size = " << numbers.TotalSize() << "bytes used_size = " << numbers.UsedSize() << "byts" << endl;
}

void Optimize() {
	TimePass::ShmList<int> numbers("/tmp/list");
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
			     "-c [capacity] for create\n"
			     "-d for destroy\n"
                 "-o for optimize\n"
			     "-w for write\n"
			     "-r for read\n"
				 "-s for dot\n"
				 "-e for clear\n");
		return 0;
	}

	switch(result) {
        case 'a':
            About();
            break;
        case 'b':
            Remove();
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

