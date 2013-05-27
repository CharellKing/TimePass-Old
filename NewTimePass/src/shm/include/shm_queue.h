#ifndef _SHM_QUEUE_H

#define _SHM_QUEUE_H

#include "shm_base.h"

#include <cstring>

#include <fstream>

namespace TimePass {
struct QueueHead {
	QueueHead() : front(-1), back(-1), size(0), capacity(0) {

	}
    off_t front;        //头部
    off_t back;         //尾部
	off_t size;			//数组的长度
	off_t capacity;		//数组的容量
};

template <typename T, typename EXTEND = char>
class ShmQueue{
public:
    ShmQueue(const char* name):p_addr_(NULL), p_head_(NULL), p_ext_(NULL){
        strncpy(name_, name, 255);
    }
    
    //创建共享内存
    bool CreateShm(off_t capacity) {
        p_head_ = (QueueHead*)ShmBase::CreateShm(name_, capacity * sizeof(T) + sizeof(QueueHead) + sizeof(EXTEND));
        if (NULL == p_head_) {
            return false;
        }
		
		p_ext_ = (EXTEND*) ((char*)p_head_ + sizeof(QueueHead));
        p_addr_ = (T*)((char*)p_head_ + sizeof(QueueHead) + sizeof(EXTEND));
		
        p_head_->front = -1;
        p_head_->back = -1;
        p_head_->capacity = capacity;
        p_head_->size = 0;
		*p_ext_ = EXTEND();
		
        return true;
    }
    
    //加载共享内存
    bool AttachShm(bool is_readonly = false) {
       p_head_ = (QueueHead*)ShmBase::AttachShm(name_, is_readonly);
       if (NULL == p_head_) {
            return false;
       }
		p_ext_ = (EXTEND*) ((char*)p_head_ + sizeof(QueueHead));
        p_addr_ = (T*)((char*)p_head_ + sizeof(QueueHead) + sizeof(EXTEND));
       return true;
    }
    
    //销毁共享内存
    bool DestroyShm() {
        return ShmBase::DestroyShm(name_);
    }
    //获取队列的长度
    off_t Size()const {
        return p_head_->size;
    }
    
    //获取数组的容量
    off_t Capacity() const {
        return p_head_->capacity;
    }
    
    //获取共享内存的名称
    const char* Name()const {
        return name_;
    }
    
    //获取共享内存的头部
    const QueueHead* Head()const {
        return p_head_;
    }
    
    //设置共享内存的额外信息
    void SetExtend(const EXTEND& ext) {
        *p_ext_ = ext;
    }
	
	//获取共享内存扩展信息
	const EXTEND* GetExtend()const {
		return p_ext_;
	}
	
    //在队尾加入元素
    bool Push(const T& elem) {
        if (p_head_->size < p_head_->capacity) {
            ++ p_head_->back;
            if (p_head_->back >= p_head_->capacity) {
                p_head_->back = 0;
            }
            *(p_addr_ + p_head_->back) = elem;
            //刚开始队列为空，重置front
            if (-1 == p_head_->front) {
                p_head_->front = p_head_->back;
            }
            ++ p_head_->size;
            return true;
        }
		Error::SetErrno(NO_SPACE_SHORTAGE);
		Error::SetErrmsg(MSG_SPACE_SHORTAGE);	
		return false;
    }
    
    //在对头删除一个元素
    T* Pop() {
        T* p_ret = NULL;
        if (p_head_->size > 0) {
            p_ret = (p_addr_ + p_head_->front);
            -- p_head_->size;
            if (0 == p_head_->size) {
                p_head_->front = -1;
                p_head_->back = -1;
            } else {
                ++ p_head_->front;
                if (p_head_->front >= p_head_->capacity) {
                    p_head_->front = 0;
                }
            }
        }
        return p_ret;
    }

    //获取队头元素
    T* Front() {
        if (p_head_->size > 0) {
            return p_addr_ + p_head_->front;
        }
        return NULL;
    }
    
    //获取队尾元素
    T* Back() {
        if (p_head_->size > 0) {
            return p_addr_ + p_head_->back;
        }
        return NULL;
    }

    //清空队列
    void Clear() {
        p_head_->front = -1;
        p_head_->back = -1;
        p_head_->size = 0;
    }

    //将队列以图形的形式显示，转化为dot language 识别的脚本
	bool ToDot(const std::string& filename, const std::string (*Label)(const T& value) ) const{
		std::ofstream out;
		char t_label[500];
		out.open(filename.c_str());
        out.write("digraph G {\n", strlen("digraph G {\n"));
        int number = 0;
		if (p_head_->size > 0) {
            number = *(p_addr_ + p_head_->front);
			snprintf(t_label, 499, "queue [shape=record, label=\"<f0>%s", Label(number).c_str());
			out.write(t_label, strlen(t_label));
		}
		for(int index = 1; index < p_head_->size; ++ index) {
            number = *(p_addr_ + (p_head_->front + index));
			snprintf(t_label, 499, "|<f%d> %s", index, Label(number).c_str());
			out.write(t_label, strlen(t_label));
		} 
		if (p_head_->size > 0) {
			out.write("\"];\n", strlen("\"];\n"));
		}

		out.write("}\n", strlen("}\n"));
		out.close();
		return true;
	}

private:
    char name_[256];
    QueueHead* p_head_;
	EXTEND* p_ext_;
    T* p_addr_;
};
};

#endif
