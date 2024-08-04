#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "log.hpp"
#include <iostream>
#include <fstream>
#include <string>

extern "C"{
    #include "stb_image.h"
    #include "stb_image_write.h"
}
#include "huffman.hpp"
#include "color_quantity.hpp"
#include "plyparser.hpp"
#include <unordered_map>


int main(){
    Logger logger("octree_color_quantity");
    logger.GetConfig("./logconfig.json");

    // 读取 JPEG 图像
    const char* filename = "test.jpeg";
    int width, height, channels;
    uint8_t* img_data =static_cast<uint8_t*>(stbi_load(filename, &width, &height, &channels, 3));
    if (!img_data) {
        LOG(logger,LOGLEVEL::ERROR)<<"load image data file!";
        return -1;
    }
    LOG(logger,LOGLEVEL::DEBUG)<<"picture width is "<<width<<", height is "<<height;
    ColorOctree color_tree(8);
    auto data_end = img_data+width*height*3;
    auto origin_ptr = img_data;
    while(img_data!=data_end){
        auto x = *(img_data++);
        auto y = *(img_data++);
        auto z = *(img_data++);
        color_tree.insert(x,y,z);
    }
    
    std::cout<<color_tree.get_type_num()<<std::endl;
    color_tree.quantity(color_tree.get_type_num()/2000);
    std::cout<<color_tree.get_type_num()<<std::endl;
    
    img_data = origin_ptr;
    std::vector<uint8_t> rgbData;
    int count = 0;
    std::array<uint32_t,256> x_arr;
    std::array<uint32_t,256> y_arr;
    std::array<uint32_t,256> z_arr;
    x_arr.fill(0);
    y_arr.fill(0);
    z_arr.fill(0);

    
    std::unordered_map<uint32_t,uint32_t> s;
    color_tree.extra(x_arr,y_arr,z_arr,s);

    std::vector<HuffNode<uint32_t,uint32_t>> vec;
    for(auto elem:s){
        vec.push_back(HuffNode<uint32_t,uint32_t>(elem.first,elem.second));
    }
    s.clear();
    std::sort(vec.begin(),vec.end());
    for(int i = 0;i<vec.size();i++){
        s[vec[i].val_] = i;
    }
    
    while(img_data!=data_end){
        auto x = *(img_data++);
        auto y = *(img_data++);
        auto z = *(img_data++);
        auto arr = color_tree.query(x,y,z);
        // if((abs(arr[0]-x)+abs(arr[1]-y)+abs(arr[2]-z))>50){
        //     arr[0] = x;
        //     arr[1] = y;
        //     arr[2] = z;
        //     count++; 
        // }
        uint8_t val = (x<<16)+(y<<8)+z;
        auto res = s[val];
        rgbData.push_back(abs(arr[0]));
        rgbData.push_back(abs(arr[1]));
        rgbData.push_back(abs(arr[2]));
    }

    //std::cout<<count<<std::endl;
    // 将 RGB 数据保存为 JPEG 文件
    int quality = 90; // JPEG 压缩质量，范围从 0 到 100
    
    if (stbi_write_jpg("output.jpg", width, height, 3, rgbData.data(), quality)) {
        std::cout << "图像成功保存为 output.jpg" << std::endl;
    } else {
        std::cout << "图像保存失败" << std::endl;
    }

    return 0;
}
