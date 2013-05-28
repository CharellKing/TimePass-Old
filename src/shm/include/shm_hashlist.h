#ifndef _SHM_HASHLIST_H

#define _SHM_HASHLIST_H

#include "common_list.h"

#include <sstream>

namespace TimePass {
struct ListBucket {
    off_t head;   //链表头部 
};

struct HashlistHead{
    static float factor;    //系数因子，即capacity / bucket_size = factor, 一般为0.75或者1
    off_t capacity;         //容量
    off_t size;             //数据个数
    off_t free_stack;       //回收站
    off_t bucket_size;      //桶的个数
};

float HashlistHead::factor = 1.0;

template <typename T, int (*Compare)(const T& a, const T& b) = T::Compare, off_t HashFunc(const T& a) = T::HashFunc, typename EXTEND = char>
class ShmHashlist {
public:
    ShmHashlist(const char* name):p_head_(NULL), p_ext_(NULL), p_bucket_(NULL), p_addr_(NULL){
        snprintf(name_, 255, name);
    }
    
    //创建共享内存
    bool CreateShm(off_t capacity) {
        off_t tmp = (off_t)capacity / HashlistHead::factor;
        off_t bucket_size = 1;
        
        while (bucket_size < tmp) { //保证bucket_size为2^n
            bucket_size <<= 1;
        }
        
        p_head_ = (HashlistHead*)ShmBase::CreateShm(name_, sizeof(HashlistHead) + sizeof(EXTEND) + sizeof(ListBucket) * bucket_size + sizeof(ListNode<T>) * capacity);
        
        if (NULL == p_head_) {
            return false;
        }
        
        p_head_->capacity = capacity;
        p_head_->bucket_size = bucket_size;
        p_head_->size = 0;
        p_head_->free_stack = -1;
        		
		p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(HashlistHead));
		p_bucket_ = (ListBucket*)((char*)p_ext_ + sizeof(EXTEND));
		p_addr_ = (ListNode<T>*)((char*)p_bucket_ +  p_head_->bucket_size * sizeof(ListBucket));
		
		for(off_t i = 0; i < p_head_->bucket_size; ++ i) {
			p_bucket_[i].head = -1;
		}
        
        return true;    
    }
    
    //加载共享内存
    bool AttachShm(bool is_readonly = false) {
        p_head_ = (HashlistHead*)ShmBase::AttachShm(name_, is_readonly);
        if (NULL == p_head_) {
            return false;
        }
		p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(HashlistHead));
		p_bucket_ = (ListBucket*)((char*)p_ext_ + sizeof(EXTEND));
		p_addr_ = (ListNode<T>*)((char*)p_bucket_ +  p_head_->bucket_size * sizeof(ListBucket));
        return true;
    }
    
    //删除共享内存
    bool DestroyShm() {
        return ShmBase::DestroyShm(name_);
    }
    
    //获取数据个数
    off_t Size() const {
        return p_head_->size;
    }
    
    //获取容量
    off_t Capacity() const {
        return p_head_->capacity;
    }
    
    //获取共享内存的标志
    const char* Name() const {
        return name_;
    }
    
    //获取链表的头部
    const ListHead* Head()const {
        return p_head_;
    }
    
    //设置扩展信息
    void SetExtend(const EXTEND& ext) {
        memcpy(p_ext_, &ext, sizeof(EXTEND));
    }
    
	//获得扩展信息
	const EXTEND* GetExtend()const {
		return p_ext_;
	}
	
	//遍历时的第一个节点
    ListNode<T>* BeginNode() {
        for(off_t i = 0; i < p_head_->bucket_size; ++ i) {
            if (-1 != p_bucket_[i].head) {
                return p_addr_ + p_bucket_[i].head;
            }
        }
		return NULL;
    }
    
	//遍历时的第一个节点
    const ListNode<T>* BeginNode()const {
        for(off_t i = 0; i < p_head_->bucket_size; ++ i) {
            if (-1 != p_bucket_[i].head) {
                return p_addr_ + p_bucket_[i].head;
            }
        }
		return NULL;
    }
    
    //下一个节点
    ListNode<T>* NextNode(ListNode<T>* p_node) {
        if (-1 == p_node->next) {
            off_t n_bucket = HashFunc(p_node->data) & (p_head_->bucket_size - 1);
			for(++ n_bucket; n_bucket < p_head_->bucket_size; ++ n_bucket) {
				if (-1 != p_bucket_[n_bucket].head) {
					return p_addr_ + p_bucket_[n_bucket].head;
				}
			}
            
        } else {
            return p_addr_ + p_node->next;
        }
        return NULL;
    }
   
    //下一个节点
    const ListNode<T>* NextNode(const ListNode<T>* p_node)const {
        if (-1 == p_node->next) {
            off_t n_bucket = HashFunc(p_node->data) & (p_head_->bucket_size - 1);
			for(++ n_bucket; n_bucket < p_head_->bucket_size; ++ n_bucket) {
				if (-1 != p_bucket_[n_bucket].head) {
					return p_addr_ + p_bucket_[n_bucket].head;
				}
			}
            
        } else {
            return p_addr_ + p_node->next;
        }
        return NULL;
    }
    
	//清空hashlist
	void Clear() {
        p_head_->size = 0;
        p_head_->free_stack = -1;
		
		for(off_t i = 0; i < p_head_->bucket_size; ++ i) {
			p_bucket_[i].head = -1;
		}
	}
	
	//向hashlist里面插入数据
    bool Insert(const T& data) {
        off_t n_bucket = HashFunc(data) & (p_head_->bucket_size - 1); 
        off_t tmp = p_bucket_[n_bucket].head;
        ListNode<T>* p_node = NULL;
		
		//当前key是否已经存在
        while (-1 != tmp) {
            p_node = (p_addr_ + tmp);
            if (0 == Compare(p_node->data, data)) {
                Error::SetErrno(NO_KEY_EXIST);
                Error::SetErrmsg(MSG_KEY_EXIST);
                return false;
            }
            tmp = p_node->next;
        }
        
		//插入数据
        ListHead list_head(p_bucket_[n_bucket].head, -1, p_head_->capacity, p_head_->size, p_head_->free_stack);
        
        if (NULL != CommonList<T, EXTEND>::PushFront(NULL, p_addr_, &list_head, data, false)) {
            p_head_->size = list_head.size;
            p_head_->free_stack = list_head.free_stack;
            p_bucket_[n_bucket].head = list_head.head;
            return true;
        }
        
        return false;
        
    }
    
    const ListNode<T>* Remove(const T& data) {
        off_t n_bucket = HashFunc(data) & (p_head_->bucket_size - 1);
        ListNode<T>* p_prep = NULL;
        ListNode<T>* p_cur = AT(p_addr_, p_bucket_[n_bucket].head);
        while (NULL != p_cur) {
            if (0 == Compare(p_cur->data, data)) {
                Error::SetErrno(NO_KEY_EXIST);
                Error::SetErrmsg(MSG_KEY_EXIST);
                break;
            }
            p_prep = p_cur;
            p_cur = AT(p_addr_, p_cur->next);
        }
        
        if (NULL == p_cur) {//没找到相关key节点
            return NULL;
        }

        off_t freenode = -1;        
        if (NULL == p_prep) {//找到的是头节点
            freenode = p_bucket_[n_bucket].head;
            p_bucket_[n_bucket].head = p_cur->next;
        }  else {
            freenode = p_prep->next;
            p_prep->next = p_cur->next;
        }
        //回收节点
        AT(p_addr_, freenode)->next = p_head_->free_stack;
        p_head_->free_stack = freenode;
        return p_cur;
    }
    
    ListNode<T>* Find(const T& data) {
        off_t n_bucket = HashFunc(data) & (p_head_->bucket_size - 1); 
        off_t tmp = p_bucket_[n_bucket].head;
        ListNode<T>* p_node = NULL;
        while (-1 != tmp) {
            p_node = (p_addr_ + tmp);
            if (0 == Compare(p_node->data, data)) {
                return p_node;
            }
            tmp = p_node->next;
        } 
        return NULL;
    }
   
    const ListNode<T>* Find(const T& data)const {
        off_t n_bucket = HashFunc(data) & (p_head_->bucket_size - 1); 
        off_t offset = p_bucket_[n_bucket].head;
        const ListNode<T>* p_node = NULL;
        while (-1 != offset) {
            p_node = p_addr_ + offset;
            if (0 == Compare(p_node->data, data)) {
                return p_node;
            }
            offset = p_node->next;
        } 
        return NULL;
    }
    
    //将链表以图形的形式显示，转化为dot language 识别的脚本
	bool ToDot(const std::string& filename, const std::string (*Label)(const T& value) ) const{
		std::ofstream out;
		std::ostringstream oss;
		const ListNode<T>* p_head = NULL;
		out.open(filename.c_str());
		
		oss << "digraph G {\nrankdir=LR;\n";
		out << oss.str();
		oss.str("");
		
		DrawArray(out, p_head_->bucket_size);			//先画出hash列
		for(off_t i = 0; i < p_head_->bucket_size; ++ i) {
            if (-1 != p_bucket_[i].head) {
                CommonList<T, EXTEND>::DrawList(p_addr_, p_bucket_[i].head, out, Label);	//画出链表

                //连接bucket[i]和Node[p_bucket_[i]]
                oss << "bucket:f" << i << "->Node" << p_bucket_[i].head << "\n";
				out << oss.str();
				oss.str("");
            }
		}
		
		oss << "}\n";
		out << oss.str();
		
		out.close();
		return true;
	}    


	off_t TotalSize() const {
		return sizeof(HashlistHead) + sizeof(EXTEND) + sizeof(ListBucket) * p_head_->bucket_size + sizeof(ListNode<T>) * p_head_->capacity;	
	}
	
	off_t UsedSize() const {
		return sizeof(HashlistHead) + sizeof(EXTEND) + sizeof(ListBucket) * p_head_->bucket_size + sizeof(ListNode<T>) * p_head_->size;			
	}
	
	//提交共享内存所作的改变
	bool Commit(bool is_sync) {
		return ShmBase::Commit((char*)p_head_, TotalSize(), is_sync);
	}
private:

    //用dot language画一个垂直数组
    static void DrawArray(std::ofstream& out, off_t bucket_size) {
		std::ostringstream oss;
        if (bucket_size > 0) {
			oss << "bucket [shape=record, label=\"<f0> 0";
        }
        
        for (off_t i = 1; i < bucket_size; ++ i) {
			oss << "|<f" << i << ">" << i;
        }
        
        if (bucket_size > 0) {
			oss << "\"];\n";
		}
		
		out << oss.str();
    }
    
    char name_[256];
    HashlistHead* p_head_;
    EXTEND* p_ext_;
    ListBucket* p_bucket_;
    ListNode<T>* p_addr_;
};
};

#endif
