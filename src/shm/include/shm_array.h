#ifndef _SHM_ARRAY_H

#define _SHM_ARRAY_H

#include "shm_base.h"

#include "shm_config.h"

#include <cstring>

#include <sstream>

#include <fstream>


struct ArrayHead {
	ArrayHead() : size(0), capacity(0) {

	}
	off_t size;			//数组的长度
	off_t capacity;		//数组的容量
};

namespace TimePass {
template <typename T, typename EXTEND = char>
class ShmArray {
public:
	ShmArray(const char* name):p_head_(NULL), p_addr_(NULL), p_ext_(NULL){
		strncpy(name_, name, 255);
	}
	
	//创建共享内存
	bool CreateShm(off_t capacity){
		p_head_ = (ArrayHead*)ShmBase::CreateShm(name_, capacity * sizeof (T) + sizeof(ArrayHead) + sizeof(EXTEND));
		if (NULL == p_head_) {
			return false;
		}

		p_head_->capacity = capacity;
		p_head_->size = 0;
        
		p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(ArrayHead));
		p_addr_  = (T*)((char*)p_ext_ + sizeof(EXTEND));

		return true;
	}
	
    //加载共享内存
	bool AttachShm(bool is_readonly = false) {
		p_head_ = (ArrayHead*)ShmBase::AttachShm(name_, is_readonly);
		if (NULL == p_head_) {
			return false;
		}
        
		p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(ArrayHead));
		p_addr_  = (T*)((char*)p_ext_ + sizeof(EXTEND));
		return true;
	}

    //卸载共享内存
    bool DetachShm() {
        return ShmBase::DetachShm((char*)p_head_, TotalSize());
    }
    
	//销毁共享内存
	bool DestroyShm() {
		return ShmBase::DestroyShm(name_);
	}

	//获取数组的尺寸
	off_t Size()const {
		return p_head_->size;
	}

	//获取数组的容量
	off_t Capacity()const{
		return p_head_->capacity;
	}

	//获取共享内存的名称
	const char* Name()const{
		return name_;
	}

	//获取共享内存的头部
	const ArrayHead* Head() const{
		return p_head_;
	}

	//设置共享内存的额外信息
	void SetExtend(const EXTEND& ext) {
		*p_ext_ = ext;
	}

	//获取扩展信息
	const EXTEND* GetExtend()const {
		return p_ext_;
	}
	
	//获取下标为index的元素
	const T* At(off_t index)const {
		if (index >= 0 && index < p_head_->size) {
			return p_addr_ + index;
		} 
		Error::SetErrno(NO_INDEX_NONEXIST);
		Error::SetErrmsg(MSG_INDEX_NONEXIST);
		return NULL;
	}

	//获取下标为index的元素[0, index)
	T* At(off_t index) {
		if (index >= 0 && index < p_head_->size) {
			return p_addr_ + index;
		}
		Error::SetErrno(NO_INDEX_NONEXIST);
		Error::SetErrmsg(MSG_INDEX_NONEXIST);
		return NULL;
	}

	//在共享内存的尾部压入元素
	bool PushBack(const T& elem) {
		if (p_head_->size >= p_head_->capacity) {//空间不足扩展
		    	off_t expand_size = ShmConfig::ExpandSize(p_head_->size);
		    	if (expand_size <= 0) {
		        return false;
		    	}
			bool ret = ShmBase::Resize(name_, TotalSize() + expand_size * sizeof(T));
		    	if (true == ret) {
		    	    p_head_->capacity += expand_size;
		    	} else {
		     	return false;
		    	}
		}
        
		*(p_addr_ + p_head_->size) = elem;
		++ p_head_->size;
		return true;
	}

    //向array尾部弹出数据
    const T* PopBack() {
        if (p_head_->size <= 0) {
            return NULL;
        }
        
        -- p_head_->size;
        return p_addr_ + p_head_->size;
    }
    
    //删除数据
    bool Remove(off_t index) {
		if (index >= p_head_->size || index < 0) {//判断index是否在[0, size)之内
			Error::SetErrno(NO_INDEX_NONEXIST);
			Error::SetErrmsg(MSG_INDEX_NONEXIST);
			return false;
		}

		T* p_beg = p_addr_ + index;
		T* p_end = p_addr_ + p_head_->size;
        //index(包括index这个元素)向后移动一个单位
		while(p_beg != p_end) {
			(*p_beg) = *(p_beg + 1);
            ++ p_beg;
		}
        -- p_head_->size;
		return true;    
    }
    
	//在数组中间插入元素
	bool Insert(off_t index, const T& elem) {
        if (p_head_->size >= p_head_->capacity) {//空间不足扩展
            off_t expand_size = ShmConfig::ExpandSize(p_head_->size);
            bool ret = ShmBase::Resize(name_, TotalSize() + expand_size * sizeof(T));
            if (true == ret) {
                p_head_->capacity += expand_size;
            } else {
                return false;
            }
        }

		if (index > p_head_->size) {//判断index是否在[0, index]之内
			Error::SetErrno(NO_INDEX_NONEXIST);
			Error::SetErrmsg(MSG_INDEX_NONEXIST);
			return false;
		}

		T* p_beg = p_addr_ + index;
		T* p_end = p_addr_ + p_head_->size;
        //index(包括index这个元素)向后移动一个单位
		while(p_end != p_beg) {
			(*p_end) = *(p_end - 1);
			-- p_end;
		}
		*p_beg = elem;
		++ p_head_->size;
		return true;
	}

	//清空数组
	void Clear() {
		p_head_->size = 0;
	}
	
	//将数组以图形的形式显示，转化为dot language 识别的脚本
	bool ToDot(const std::string& filename, const std::string (*Label)(const T& value) ) const{
		std::ofstream out;
		std::ostringstream oss;
		out.open(filename.c_str());
		oss << "digraph G {\n";
		if (p_head_->size > 0) {
			oss << "array [shape=record, label=\"<f0>" << Label(*At(0));
		}
		
        char t_num[32];
		for(off_t index = 1; index < p_head_->size; ++ index) {
			oss << "|<f" << index << ">" << Label(*At(index));
		} 
		if (p_head_->size > 0) {
			oss << "\"];\n";
		}
		oss << "}\n";
		out << oss.str();
		out.close();
		return true;
	}
    
    //内存优化
    bool Optimize(){
        off_t new_capacity = ShmConfig::ReduceSize(p_head_->capacity, p_head_->size);
        if (new_capacity == p_head_->capacity) {
            return true;
        }
        
        off_t len = sizeof(ArrayHead) + sizeof(EXTEND) + sizeof(T) * new_capacity;
        if (true == ShmBase::Resize(name_, len)) {
            p_head_->capacity = new_capacity;
            return true;
        }
        return false;
    }
	
    //空间的总size
    off_t TotalSize()const{
        return sizeof(ArrayHead) + sizeof(EXTEND) + sizeof(T) * p_head_->capacity;
    }
    
    //使用空间
    off_t UsedSize()const{
        return sizeof(ArrayHead) + sizeof(EXTEND) + sizeof(T)* p_head_->size;
    }
	
	//提交共享内存所作的改变
	bool Commit(bool is_sync) {
		return ShmBase::Commit((char*)p_head_, TotalSize(), is_sync);
	}
private:
	char name_[256];
	ArrayHead* p_head_;
	T* p_addr_;
	EXTEND* p_ext_;
};

};
#endif

