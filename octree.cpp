#include <iostream>
#include "plyparser.hpp"

int main(){
    PlyEncoder encoder;
    encoder.Encode("./chair2851.ply");
    return 0;
}