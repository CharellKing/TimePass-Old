#ifndef _SHM_SET_H

#define _SHM_SET_H

#include "shm_rbtree.h"
#include "shm_base.h"


namespace TimePass {
template <typename T, int (*Compare)(const T& a, const T& b) = T::Compare, typename EXTEND = char>
class ShmSet {
public:
    ShmSet(const char* name):p_head_(NULL), p_addr_(NULL) {
        snprintf(name_, 255, name);
    }
    
    //创建共享内存
    bool CreateShm(off_t capacity) {
        p_head_ =  (RbtreeHead*)ShmBase::CreateShm(name_, capacity * sizeof(RbtreeNode<T>) + sizeof(EXTEND) + sizeof(RbtreeHead));
        if (NULL == p_head_) {
            return false;
        }
        
        p_head_->root = -1;
        p_head_->size = 0;
        p_head_->capacity = capacity;
        p_head_->free_stack = -1;
        
        p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(RbtreeHead)); 
        
        p_addr_ = (RbtreeNode<T>*)((char*)p_head_ + sizeof(RbtreeHead) + sizeof(EXTEND));
        return true;
    }
    
    //加载共享内存
    bool AttachShm(bool is_readonly = false) {
        p_head_ = (RbtreeHead*)ShmBase::AttachShm(name_, is_readonly);
        if (NULL == p_head_) {
            return false;
        }
        
        p_ext_ = (EXTEND*)((char*)p_head_ + sizeof(RbtreeHead)); 
        
        p_addr_ = (RbtreeNode<T>*)((char*)p_head_ + sizeof(RbtreeHead) + sizeof(EXTEND));
        return true;        
    }
    
    //卸载共享内存
    bool DetachShm() {
        return ShmBase::DetachShm((char*)p_head_, TotalSize());
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
    off_t Capacity()const {
        return p_head_->capacity;
    }
    
    //获取共享内存的标志
    const char* Name()const {
        return name_;
    }
    
    //设置扩展信息
    void SetExtend(const EXTEND& ext) {
        *p_ext_ = ext;
    }
    
    //获取扩展信息
    const EXTEND* GetExtend()const {
        return p_ext_;
    }
    
    //根结点
    RbtreeNode<T>* RootNode() {
        return p_addr_ + p_head_->root;
    }
    
    //获取节点的左节点
    RbtreeNode<T>* LeftNode(const RbtreeNode<T>* p_node) {
        return p_node ? (p_addr_ + p_node->left) : NULL;
    }
    
    //获取节点的右节点
    RbtreeNode<T>* RightNode(const RbtreeNode<T>* p_node) {
        return p_node ? (p_addr_ + p_node->right) : NULL;
    }
    
    //插入节点
    bool Insert(const T& data) {
        return ShmRbtree<T, Compare, EXTEND>::InsertUnique(name_, p_head_, p_addr_, data);
    }
    
    //删除节点, 返回删除节点的个数
    off_t Remove(const T& data) {
        return ShmRbtree<T, Compare, EXTEND>::Remove(p_head_, p_addr_, data);
    }
    
    //清空集合
    void Clear() {
        p_head_->free_stack = -1;
        p_head_->root = -1;
        p_head_->size = 0;
    }

    //升序遍历红黑树， 起始节点
    RbtreeNode<T>* Begin() {
        return ShmRbtree<T, Compare, EXTEND>::Begin(p_head_, p_addr_);
    }
    
    //升序遍历红黑树， 下一个节点
    RbtreeNode<T>* Next(const RbtreeNode<T>* p_cur) {
        return ShmRbtree<T, Compare, EXTEND>::Next(p_addr_, p_cur);        
    }
    
    //升序遍历红黑树， 起始节点
    const RbtreeNode<T>* Begin()const {
        return ShmRbtree<T, Compare, EXTEND>::Begin(p_head_, p_addr_);        
    }
    
    //升序遍历红黑树， 下一个节点
    const RbtreeNode<T>* Next(const RbtreeNode<T>* p_cur)const {
        return ShmRbtree<T, Compare, EXTEND>::Next(p_addr_, p_cur);
    }
    
    //降序遍历红黑树， 起始节点
    RbtreeNode<T>* RBegin() {
        return ShmRbtree<T, Compare, EXTEND>::RBegin(p_head_, p_addr_);
    }
    
    //降序遍历红黑树， 下一个节点
    RbtreeNode<T>* RNext(const RbtreeNode<T>* p_cur) {
        return ShmRbtree<T, Compare, EXTEND>::RNext(p_addr_, p_cur);
    }
    
    //降序遍历红黑树， 起始节点
    const RbtreeNode<T>* RBegin()const {
        return ShmRbtree<T, Compare, EXTEND>::RBegin(p_head_, p_addr_);
    }
    
    //升序遍历红黑树， 下一个节点
    const RbtreeNode<T>* RNext(const RbtreeNode<T>* p_cur)const {
        return ShmRbtree<T, Compare, EXTEND>::RNext(p_head_, p_addr_);
    }
    
    RbtreeNode<T>* LowerBound(const T& data) {
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::LowerBound(p_addr_, p_head_->root, data)));
    }
    
    const RbtreeNode<T>* LowerBound(const T& data)const{
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::LowerBound(p_addr_, p_head_->root, data)));
    }
    
    RbtreeNode<T>* UppperBound(const T& data) {
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::EqualRange(p_addr_, p_head_->root, data)));
    }
    
    const RbtreeNode<T>* UppperBound(const T& data)const{
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::EqualRange(p_addr_, p_head_->root, data)));
    }
    
    RbtreeNode<T>* EqualRange(const T& data) {
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::EqualRange(p_addr_, p_head_->root, data)));
    }
    
    const RbtreeNode<T>* EqualRange(const T& data)const {
        return AT(p_addr_, (ShmRbtree<T, Compare, EXTEND>::EqualRange(p_addr_, p_head_->root, data)));
    }
    
    const off_t Count(const T& data)const {
        return ShmRbtree<T, Compare, EXTEND>::Count(p_addr_, p_head_->root, data);
    }
    
    //共享内存总占用空间
    off_t TotalSize()const {
        return sizeof(RbtreeHead) + sizeof(EXTEND) + p_head_->capacity * sizeof(RbtreeNode<T>);
    }
    
    //共享内存已经占用的空间
    off_t UsedSize()const {
        return sizeof(RbtreeHead) + sizeof(EXTEND) + p_head_->size * sizeof(RbtreeNode<T>);
    }
    
    bool Optimize() {
        return ShmRbtree<T, Compare, EXTEND>::Optimize(name_, p_addr_, p_head_);        
    }

    //将集合的树形状，以dot language识别的脚本表示出来
    bool ToDot(const std::string& filename, const std::string (*Label)(const T& data)) const {
        return ShmRbtree<T, Compare, EXTEND>::ToDot(p_head_, p_addr_, filename, Label);
    }

private:
    char name_[256];
    RbtreeHead* p_head_;
    EXTEND* p_ext_;
    RbtreeNode<T>* p_addr_;
};
};

#endif

