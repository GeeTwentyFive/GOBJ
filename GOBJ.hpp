#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string.h>
#include <stdlib.h>

class GOBJ { public:
struct Vertex {
        float pos[3];
        float normal[3];
        float texcoord[2];
};

std::vector<GOBJ::Vertex> vertices;
std::vector<int> indices;

//private:
// TODO?
public:
GOBJ(const std::filesystem::path& obj_path) {
        std::ifstream obj_file_stream(obj_path, std::ios::binary); if (!obj_file_stream.is_open()) throw std::runtime_error(std::string("ERROR: Failed to open '") + obj_path.string() + "'");
        std::vector<float> v, vn, vt;
        for (std::string line; std::getline(obj_file_stream, line);) { if (line.back() == '\r') line.pop_back(); if (line.empty()) continue;
                const char* token = line.c_str(); token += strspn(token, " \t"); if (token[0] == '\0' || token[0] == '#') continue;
                if (token[0] == 'v' && (token[1] == ' ' || token[1] == '\t')) { // Vertex
                        token += 2; token += strspn(token, " \t");
                        v.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        v.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        v.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        continue;
                }
                if (token[0] == 'v' && token[1] == 'n' && (token[2] == ' ' || token[2] == '\t')) { // Normal
                        token += 3; token += strspn(token, " \t");
                        vn.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        vn.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        vn.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        continue;
                }
                if (token[0] == 'v' && token[1] == 't' && (token[2] == ' ' || token[2] == '\t')) { // Texcoord
                        token += 3; token += strspn(token, " \t");
                        vt.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        vt.push_back((float)atof(token)); token += strcspn(token, " \t\r");
                        continue;
                }
                if (token[0] == 'f' && (token[1] == ' ' || token[1] == '\t')) { // Face
                        token += 2; token += strspn(token, " \t");
                        // TODO
                        continue;
                }
        }

        // TODO
}
};