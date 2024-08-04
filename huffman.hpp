#ifndef _HUFFMAN_HPP_
#define _HUFFMAN_HPP_
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <unordered_set>

template <typename ValType,typename WeightType>
class HuffNode{
public:
    HuffNode(){
        val_ = 0;
        weight_ = 0;
    }
    HuffNode(ValType val,WeightType weight){
        val_ = val;
        weight_ = weight;
    }
    
    bool operator<(const HuffNode<ValType,WeightType>& node){
        return weight_<node.weight_;
    }
    void operator=(const HuffNode<ValType,WeightType>& node){
        val_ = node.val_;
        lchild_ = node.lchild_;
        rchild_ = node.rchild_;
        weight_ = node.weight_;
    }
    bool is_leaf_node(){
        return lchild_== nullptr && rchild_ == nullptr;
    }
    HuffNode* lchild_=nullptr;
    HuffNode* rchild_=nullptr;
    ValType val_;
    WeightType weight_;
};
class HuffCode{
    public:
        uint32_t val_;
        uint8_t length_;
};
template <typename ValType,typename WeightType>
class HuffTree{
    public:
    HuffTree(std::vector<HuffNode<ValType,WeightType>>& vec){
        auto node_num = vec.size();
        vec.reserve(node_num*2-1);
        auto start= vec.begin();
        auto end = start+node_num;
        int  sub_index = node_num*2-2;
        auto cmp = [](HuffNode<ValType,WeightType>& node1,HuffNode<ValType,WeightType>& node2)->bool{return node1.weight_>=node2.weight_;};
        std::make_heap(start,end,cmp);
        while(end-start>1){
            std::pop_heap(start,end,cmp);
            auto node1 = --end;
            vec[sub_index--] = *node1;
            std::pop_heap(start,end,cmp);
            auto node2 = end-1;
            vec[sub_index--] = *node2;
            
            node2->weight_ += node1->weight_;
            node2->lchild_ = &vec[sub_index+1];
            node2->rchild_ = &vec[sub_index+2];
            std::push_heap(start,end,cmp);      
        }
        root_ = vec.data();
    }
    void gen_table(std::unordered_map<ValType,HuffCode>& map){
        if(root_==nullptr){
            return;
        }
        std::queue<HuffNode<ValType,WeightType>*> que;
        que.push(root_);
        uint8_t len = 0;
        uint8_t val = 0;
        while(!que.empty()){
            auto size = que.size();
            for(int i = 0;i<size;i++){
                auto node = que.front();
                que.pop();
                if(node->is_leaf_node()){
                    map[node->val_] = HuffCode{val++,len};
                }else{
                    if(node->lchild_){
                        que.push(node->lchild_);
                    }
                    if(node->rchild_){
                        que.push(node->rchild_);
                    }
                }
            }
            len++;
            val*=2;
        }
    }
    private:
        HuffNode<ValType,WeightType>* root_=nullptr;
};

#endif