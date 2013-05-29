#ifndef SHM_CONFIG_H_

#define SHM_CONFIG_H_

#include "global/error.h"

namespace TimePass{
struct ShmConfig {
    //计算此次扩大的尺寸,返回add_size
    static off_t ExpandSize(off_t size) {
        if (size >= ShmConfig::max_capacity) {//size 超过了 max_capacity不扩容
            Error::SetErrno(NO_CAPACITY_TOMAX);
            Error::SetErrmsg(MSG_CAPACITY_TOMAX);
            return 0;
        }
        
        if (size <= ShmConfig::max_expand) {
            return size;
        }
        
        if (size > ShmConfig::max_expand) {
            if (size + ShmConfig::max_expand <= ShmConfig::max_capacity) { //保证最终的扩容不会超过max_capacity
                return ShmConfig::max_expand;
            } else {
                return ShmConfig::max_capacity - size;
            }
        }
        
        return 0;
    }
    
    //计算此次缩减容器后的容量,返回new_capacity
    static off_t ReduceSize(off_t capacity, off_t size) {
        if (capacity <= (size << 1) + ShmConfig::min_reduce) {
            return capacity;
        } else {
            if (size > 0) {
                capacity = (size << 1);
            } else {//保证capacity不为0
                capacity = default_size;
            }
        }
        return capacity;
    }
    
    static off_t max_expand;
    static off_t min_reduce;
    static off_t default_size;
    static off_t max_capacity;
};

//设置的时候, max_capacity远远大于max_expand
off_t ShmConfig::max_expand = 1 << 10;      //一次最大扩展
off_t ShmConfig::min_reduce = 1 << 10;      //一次最小缩减 
off_t ShmConfig::default_size = 1 << 10;    //默认容器的size  
off_t ShmConfig::max_capacity = 1 << 30;    //最大容量

};


#endif
