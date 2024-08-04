#ifndef _OCTREE_HPP_
#define _OCTREE_HPP_
#include <array>
#include <stdint.h>
#include <assert.h>
#include <queue>
#include "log.hpp"
#include "common.hpp"

class Ocnode{
public:
    Ocnode(){
        std::fill(child_arr_.begin(),child_arr_.end(),nullptr);
    }
    Ocnode* insert(uint8_t index){
        assert(index>=0 && index<8);
        if(child_arr_[index]==nullptr)
            child_arr_[index] = new Ocnode();
        return child_arr_[index];
    }
    uint8_t get_child_num(){
        uint8_t res = 0;
        for(auto elem:child_arr_){
            if(elem)
                res++;
        }
        return res;
    }
    uint8_t serial(){
        uint8_t res = 0;
        for(int i = 0;i<8;i++){
            if(child_arr_[7-i]){
                res |= (1<<i); 
            }
        }
        return res;
    }
    bool is_leaf_node(){
        for(auto elem:child_arr_){
            if(elem!=nullptr){
                return false;
            }
        }
        return true;
    }
    void deserial(uint8_t val,std::queue<Ocnode*> &que){
        for(int i  = 7;i>=0;i--){
            if(get_bit(val,i)){
                child_arr_[7-i] = new Ocnode();
                que.push(child_arr_[7-i]);
            }
        }
    }
    const std::array<Ocnode*,8>& get_child_arr(){
        return this->child_arr_;
    }
protected:
    std::array<Ocnode*,8> child_arr_;

};
template <typename NodeType>
class Octree{
    public:
        Octree(){};
        Octree(uint8_t depth):depth_(depth){};
        Octree(uint8_t depth,std::vector<uint8_t>& x_vec,std::vector<uint8_t>& y_vec,std::vector<uint8_t>& z_vec){
            depth_ = depth;
            assert(x_vec.size()==y_vec.size() && y_vec.size()==z_vec.size());
            for(int i = 0;i<x_vec.size();i++){
                Octree::insert(x_vec[i],y_vec[i],z_vec[i]);
            }
        }
        void deserial(std::vector<uint8_t>& vec){
            if(vec.empty()){
                return;
            }
            std::queue<NodeType*> node_que;
            node_que.push(static_cast<NodeType*>(&this->root_));
            int count = 0;
            int size = 0;
            while(count<vec.size()){
                size = node_que.size();
                for(int i = 0;i<size;i++){
                    auto elem = node_que.front();
                    node_que.pop();
                    elem->deserial(vec[count++],node_que);
                }
                depth_++;
            }
            return;
        }
        NodeType* insert(uint8_t x,uint8_t y,uint8_t z){
            auto root = static_cast<NodeType*>(&this->root_);
            for(int i = depth_-1;i>=0;i--){
                uint8_t val = (get_bit(x,i)<<2)+(get_bit(y,i)<<1)+get_bit(z,i);
                assert(val>=0 && val<=7);
                root = root->insert(7-val);
            }
            return root;
        }
        std::vector<uint8_t> serial(){
            std::queue<NodeType*> node_que;
            std::vector<uint8_t> res;
            node_que.push(static_cast<NodeType*>(&this->root_));
            while(!node_que.front()->is_leaf_node()){
                int size = node_que.size();
                for(int i = 0;i<size;i++){
                    auto elem = node_que.front();
                    node_que.pop();
                    for(auto child:elem->get_child_arr()){
                        if(child)
                            node_que.push(child);
                    }
                    auto byte = elem->serial();
                    res.push_back(byte);
                }
            }
            return res;
        }
        void extra(std::vector<uint8_t>& x_vec,std::vector<uint8_t>& y_vec,std::vector<uint8_t>& z_vec){
            if(depth_==0){
                return;
            }
            std::queue<NodeType*> node_que;
            auto get_bit = [](uint8_t val,uint8_t bit)->uint8_t{return (val&(1<<bit))!=0?1:0;};
            for(int i = 0;i<8;i++){
                auto node =this->root_.get_child_arr()[i];
                if(node){
                    node_que.push(node);
                    x_vec.push_back(get_bit(7-i,2));
                    y_vec.push_back(get_bit(7-i,1));
                    z_vec.push_back(get_bit(7-i,0));
                }
            }
            while(true){
                int size = node_que.size();
                if(node_que.front()->is_leaf_node()){
                    break;
                }
                for(int i = 0;i<size;i++){
                    auto node =  node_que.front();
                    node_que.pop();
                    auto x_val = x_vec.front();
                    x_vec.erase(x_vec.begin());
                    auto y_val = y_vec.front();
                    y_vec.erase(y_vec.begin());
                    auto z_val = z_vec.front(); 
                    z_vec.erase(z_vec.begin());
                    for(int i = 0;i<8;i++){
                        if(node->get_child_arr()[i]){
                            node_que.push(node->get_child_arr()[i]);
                            x_vec.push_back((x_val<<1) + get_bit(7-i,2));
                            y_vec.push_back((y_val<<1) + get_bit(7-i,1));
                            z_vec.push_back((z_val<<1) + get_bit(7-i,0));
                        }
                    }
                }
            }
            return;
        };
        uint8_t get_depth(){
            return depth_;
        }
        NodeType* search(uint8_t x,uint8_t y,uint8_t z){
            auto root = &root_;
            for(int i = depth_-1;i>=0;i--){
                uint8_t val = (get_bit(x,i)<<2)+(get_bit(y,i)<<1)+get_bit(z,i);
                assert(val>=0 && val<=7);
                root = static_cast<NodeType*>(root->get_child_arr()[7-val]);
            }
            return root;
        }
    public:
        NodeType root_;
        uint8_t depth_=0;
};
#endif