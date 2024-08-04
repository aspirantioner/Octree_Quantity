#ifndef _COLOR_QUANTITY_HPP_
#define _COLOR_QUANTITY_HPP_

#include "octree.hpp"
#include <queue>
#include <deque>
#include <unordered_map>
const uint8_t lsb_trunc_mask[] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

struct DataInfo{
    uint32_t count = 0;
    uint8_t depth=0;
    uint8_t lsb_x=0;
    uint8_t lsb_y=0;
    uint8_t lsb_z=0;
};

class ColorOcnode:public Ocnode{
    public:
    ColorOcnode(){};
    void add(int num=1){
        count_+=num;
    };
    void clear(){
        std::fill(child_arr_.begin(),child_arr_.end(),nullptr);
    }
    ColorOcnode* insert(uint8_t index){
        assert(index>=0 && index<8);
        if(child_arr_[index]==nullptr){
            child_arr_[index] = new ColorOcnode();
        }
        auto res = static_cast<ColorOcnode*>(child_arr_[index]);  
        res->parent_ = this; 
        res->add();
        return res;
    }
    const uint32_t get_descendant_num(){
        return count_;
    }
    void get_lsb(uint8_t& x,uint8_t& y,uint8_t& z){
        DataInfo* info_ptr = reinterpret_cast<DataInfo*>(&child_arr_[0]);
        x = info_ptr->lsb_x;
        y = info_ptr->lsb_y;
        z = info_ptr->lsb_z;
    }
    void set_lsb(uint8_t x,uint8_t y,uint8_t z){
        DataInfo* info_ptr = reinterpret_cast<DataInfo*>(&child_arr_[0]);
        info_ptr->lsb_x = x;
        info_ptr->lsb_y = y;
        info_ptr->lsb_z = z;
    }
    uint8_t get_lsb_lenght(){
        DataInfo* info_ptr = reinterpret_cast<DataInfo*>(&child_arr_[0]);
        return info_ptr->depth;
    }
    void toend(uint8_t depth){
        DataInfo* info_ptr = reinterpret_cast<DataInfo*>(child_arr_.data());
        info_ptr->count = count_;
        count_ = 0;
        info_ptr->depth = depth;
    }
    uint32_t get_end_num(){
        DataInfo* info_ptr = reinterpret_cast<DataInfo*>(child_arr_.data());
        return info_ptr->count;
    }
    bool is_end(){
        return count_==0;
    }
    ColorOcnode* get_parent(){
        return parent_;
    }
    bool all_end_node(){
        for(auto elem:child_arr_){
            auto child = static_cast<ColorOcnode*>(elem);
            if(child && !child->is_end()){
                return false;
            }
        }
        return true;
    }
    void extra(std::array<uint32_t,256>& x_map,std::array<uint32_t,256>& y_map,std::array<uint32_t,256>& z_map,uint8_t x,uint8_t y,uint8_t z){
        if(is_end()){
            auto length = get_lsb_lenght();
            if(length){
                uint8_t lsb_x,lsb_y,lsb_z;
                get_lsb(lsb_x,lsb_y,lsb_z);
                uint8_t new_x=(x<<length)+lsb_x;
                uint8_t new_y=(y<<length)+lsb_y;
                uint8_t new_z=(z<<length)+lsb_z;
                assert(new_x>=x);
                assert(new_y>=y);
                assert(new_z>=z);
                x = new_x;
                y = new_y;
                z = new_z;
            }
            if(x==0){
                std::cout<<(int)x<<(int)y<<(int)z<<std::endl;
            }
            x_map[x]++;
            y_map[y]++;
            z_map[z]++;
            return;
        }
        for(int i = 0;i<8;i++){
            auto elem = static_cast<ColorOcnode*>(this->get_child_arr()[i]);
            if(elem){
                uint8_t lsb = 7-i;
                auto get_bit = [](uint8_t val,uint8_t bit)->uint8_t{return (val&(1<<bit))!=0?1:0;};
                // x = (x<<1)+get_bit(lsb,2);
                // y = (y<<1)+get_bit(lsb,1);
                // z = (z<<1)+get_bit(lsb,0);
                // elem->extra(x_map,y_map,z_map,x,y,z);
                uint8_t new_x = (x<<1)+get_bit(lsb,2);
                uint8_t new_y = (y<<1)+get_bit(lsb,1);
                uint8_t new_z = (z<<1)+get_bit(lsb,0);
                assert(new_x>=x);
                assert(new_y>=y);
                assert(new_z>=z);
                elem->extra(x_map,y_map,z_map,new_x,new_y,new_z);
            }
        }
    }
    private:
    ColorOcnode* parent_=nullptr;
    uint32_t count_ = 0;

};

class ColorOctree:public Octree<ColorOcnode>{
    public:
        ColorOctree(uint8_t depth){this->depth_ = depth;};
        void insert(uint8_t x,uint8_t y,uint8_t z){
            this->root_.add();
            auto leaf_node = this->Octree::insert(x,y,z);    
            if(leaf_node->get_descendant_num()==1){
                type_num_++;
            }
        }
        std::array<uint8_t,3> query(uint8_t x,uint8_t y,uint8_t z){
            auto root = &this->root_;
            std::array<uint8_t,3> res{x,y,z};
            for(int i = 7;i>=0;i--){
                if(root->is_end()){
                    auto depth = root->get_lsb_lenght();
                    root->get_lsb(res[0],res[1],res[2]);
                    res[0] += x&lsb_trunc_mask[7-depth];
                    res[1] += y&lsb_trunc_mask[7-depth];
                    res[2] += z&lsb_trunc_mask[7-depth]; 
                    break;
                }
                auto get_bit = [](uint8_t val,uint8_t bit)->uint8_t{return (val&(1<<bit))!=0?1:0;};
                uint8_t val = (get_bit(x,i)<<2)+(get_bit(y,i)<<1)+get_bit(z,i);
                root = static_cast<ColorOcnode*>(root->get_child_arr()[7-val]);
            }
            return res;
        }
        void quantity(uint32_t dst_num){
            
            auto comp = [](ColorOcnode* node1,ColorOcnode* node2)->bool{return node1->get_descendant_num()>node2->get_descendant_num();};
            std::priority_queue<ColorOcnode*,std::deque<ColorOcnode*>,decltype(comp)> que(comp);
        
            std::queue<ColorOcnode*> node_que;
            node_que.push(&this->root_);

            while(!node_que.front()->is_leaf_node()){
                int size = node_que.size();
                for(int i = 0;i<size;i++){
                    auto elem = node_que.front();
                    node_que.pop();
                    for(auto node:elem->get_child_arr()){
                        if(node!=nullptr)
                            node_que.push(static_cast<ColorOcnode*>(node));
                    }
                }
            }
            int size = node_que.size();
            assert(size == get_type_num());
            ColorOcnode* parent_node = nullptr;
            for(int i = 0;i<size;i++){
                auto elem = node_que.front();
                node_que.pop();
                if(parent_node!=elem->get_parent()){
                    que.push(elem->get_parent());
                }
                parent_node = elem->get_parent();
                elem->clear();
                elem->toend(0);
            }

            uint8_t depth = 0; 
            ColorOcnode* node;
            while(type_num_>dst_num){
                node = que.top();
                que.pop();
                uint8_t x=0,y=0,z=0;
                uint32_t x_sum = 0,y_sum =0,z_sum=0;
                
                for(int i = 0;i<8;i++){
                    auto elem = static_cast<ColorOcnode*>(node->get_child_arr()[i]);
                    if(elem!=nullptr){
                        depth = elem->get_lsb_lenght();
                        parent_node = elem->get_parent();
                        break;
                    }
                }
                
                int node_num=0;
                int count = 0;
                for(int i = 0;i<8;i++){
                    auto elem = static_cast<ColorOcnode*>(node->get_child_arr()[i]);
                    if(elem==nullptr){
                        continue;
                    }
                    assert(elem->is_end());
                    node_num++;
                    auto msb = 7-i;
                    elem->get_lsb(x,y,z);
                    x += uint8_t(get_bit(msb,2) <<depth);
                    y += uint8_t(get_bit(msb,1) <<depth);
                    z += uint8_t(get_bit(msb,0) <<depth);
                    auto num = elem->get_end_num();
                    count+=num;
                    x_sum += x*num;
                    y_sum += y*num;
                    z_sum += z*num; 
                    delete elem;
                }
                assert(node->get_descendant_num()==count);
                x_sum /= node->get_descendant_num();
                y_sum /= node->get_descendant_num();
                z_sum /= node->get_descendant_num();
                node->clear();
                node->set_lsb(x_sum,y_sum,z_sum);
                type_num_ -=(node_num-1); 
                node->toend(depth+1);
                if(node->get_parent()==nullptr){
                    break;
                }
                if(node->get_parent()->all_end_node()){
                    que.push(node->get_parent());
                }
            }
        }
        
        uint32_t get_type_num(){
            return type_num_;
        }
        uint32_t get_node_num(){
            return root_.get_descendant_num();
        }
        void extra(std::array<uint32_t,256>& x_arr,std::array<uint32_t,256>& y_arr,std::array<uint32_t,256>& z_arr,std::unordered_map<uint32_t,uint32_t>& map){
            auto root = &this->root_; 
            std::queue<std::pair<ColorOcnode*,std::array<uint8_t,3>>> que;       
            que.push(std::pair<ColorOcnode*,std::array<uint8_t,3>>(root,{0,0,0}));
            uint32_t count = 0;
            uint32_t sum = 0;
            int layer = 0;
            while(!que.empty()){
                int size = que.size();
                for(int i = 0;i<size;i++){
                    auto node = que.front();
                    que.pop();
                    if(node.first->is_end()){
                        std::array<uint8_t,3> res;
                        auto depth = node.first->get_lsb_lenght();
                        node.first->get_lsb(res[0],res[1],res[2]);
                        uint8_t new_x = res[0]+uint8_t(node.second[0]<<(depth%8));
                        uint8_t new_y = res[1]+uint8_t(node.second[1]<<(depth%8));
                        uint8_t new_z = res[2]+uint8_t(node.second[2]<<(depth%8));
                        uint32_t key =  (new_x<<16)+(new_y<<8)+new_z;
                        map[key] = node.first->get_end_num();
                        assert(layer+depth==8);
                        if(new_z==31){
                            int a = 0;
                        }
                        x_arr[new_x]+=node.first->get_end_num();
                        y_arr[new_y]+=node.first->get_end_num();
                        z_arr[new_z]+=node.first->get_end_num();
                        sum+=node.first->get_end_num();
                        count++;
                    }
                    else{
                        for(int i = 0 ;i<8;i++){
                            auto elem = static_cast<ColorOcnode*>(node.first->get_child_arr()[i]);
                            if(elem){
                                uint8_t lsb = 7-i;
                                uint8_t new_x = uint8_t(node.second[0]*2)+get_bit(lsb,2);
                                uint8_t new_y = uint8_t(node.second[1]*2)+get_bit(lsb,1);
                                uint8_t new_z = uint8_t(node.second[2]*2)+get_bit(lsb,0);
                                que.push(std::pair<ColorOcnode*,std::array<uint8_t,3>>(elem,{new_x,new_y,new_z}));
                            }
                        }
                    }
                }
                layer++;
            }
            assert(count==type_num_);
            
        }
        void extra(std::array<uint32_t,256>& x_map,std::array<uint32_t,256>& y_map,std::array<uint32_t,256>& z_map,uint8_t x,uint8_t y,uint8_t z){
            this->root_.extra(x_map,y_map,z_map,0,0,0);
        }
    private:
        uint32_t type_num_ = 0;
};
#endif