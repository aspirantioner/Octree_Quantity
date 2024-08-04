#ifndef _PLYENCODER_HPP_
#define _PLYENCODER_HPP_

#include "log.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "happly.h"
#include <algorithm>
#include <limits>
#include "octree.hpp"
#define ASSERT_LOG(condition,logger,msg)\
    do{\
        if(!(condition)){\
            LOG(logger,LOGLEVEL::ERROR)<<msg;\
            exit(-1);\
        }\
    }while(0)\

Logger pcc_log("pcc_encoder");

class PlyEncoder{
public:
    PlyEncoder(){
        pcc_log.GetConfig("./logconfig.json");
    }
    void Encode(std::string file_path){
        happly::PLYData plyIn(file_path);
        try{
            plyIn.validate();
        }catch (const std::runtime_error& err) {
            LOG(pcc_log,LOGLEVEL::ERROR)<<err.what();
        }
        auto elem_vec = plyIn.getElementNames();
        
        for(auto elem:elem_vec){
            if(elem=="vertex"){
                CdnDataDeal(plyIn.getElement(elem));
            }
        }
    }
    
    void CdnDataDeal(happly::Element& elem){
        auto data_type = elem.properties[0]->propertyTypeName();
        if(data_type=="uchar"){
            std::array<std::vector<uint8_t>,3> vec_xyz;
            vec_xyz[0] = elem.getPropertyType<uint8_t>("x");
            vec_xyz[1] = elem.getPropertyType<uint8_t>("y");
            vec_xyz[2] = elem.getPropertyType<uint8_t>("z");
            uint8_t max_num_xyz =  std::numeric_limits<uint8_t>::min(); 
            for(auto& vec:vec_xyz){
                auto min_num = *std::min_element(vec.begin(),vec.end());
                LOG(pcc_log,LOGLEVEL::DEBUG)<<"min num is "<<(uint32_t)min_num;
                std::transform(vec.begin(), vec.end(), vec.begin(),
                [min_num](int elem) {return elem - min_num;});
                auto max_num = *std::max_element(vec.begin(),vec.end());
                LOG(pcc_log,LOGLEVEL::DEBUG)<<"max num is "<<(uint32_t)max_num;
                max_num_xyz = std::max(max_num_xyz,max_num);
            }
            
            auto bit_deepth = static_cast<uint8_t>(std::ceil(std::log2(max_num_xyz + 1)));
            LOG(pcc_log,LOGLEVEL::DEBUG)<<"max num is "<<(uint32_t)max_num_xyz<<'\t'<<"bit depth is "<<(uint32_t)bit_deepth;

            Octree<Ocnode> oc_tree1(bit_deepth,vec_xyz[0],vec_xyz[1],vec_xyz[2]);
            auto res1 = oc_tree1.serial();
            Octree<Ocnode> oc_tree2;  
            oc_tree2.deserial(res1);
            auto res2 = oc_tree2.serial();

            assert(res1==res2);
            assert(oc_tree1.get_depth() == oc_tree2.get_depth());
            std::vector<uint8_t> x_vec;
            std::vector<uint8_t> y_vec;
            std::vector<uint8_t> z_vec;
            oc_tree1.extra(x_vec,y_vec,z_vec);

            for(int i = 0;i<vec_xyz[0].size();i++){
                oc_tree2.search(vec_xyz[0][i],vec_xyz[1][i],vec_xyz[2][i]);
            }
            happly::PLYData plyOut;
            // Add elements
            plyOut.addElement("vertex", x_vec.size());
            // Add properties to those elements
            plyOut.getElement("vertex").addProperty<uint8_t>("x", x_vec);
            plyOut.getElement("vertex").addProperty<uint8_t>("y", y_vec);
            plyOut.getElement("vertex").addProperty<uint8_t>("z", z_vec);

            // Write the object to file
            plyOut.write("test2.ply", happly::DataFormat::ASCII);
            
        }
    }
};

#endif