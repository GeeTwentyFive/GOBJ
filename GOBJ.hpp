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

private:
struct obj_index {int v_idx, vn_idx, vt_idx;}; // Indices into OBJ triplet arrays (v, vn, & vt), which combined, form one vertex
std::vector<std::vector<obj_index>> faces; // Collection of faces (face = collection of vertices (in this case, in the form of OBJ's triplet indices)) which, when combined, make a mesh
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
                        std::vector<obj_index> face;
                        const int v_count = v.size()/3; const int vn_count = vn.size()/3; const int vt_count = vt.size()/3; // for making index zero-base and supporting relative indexing
                        while (!(token[0] == '\r' || token[0] == '\n' || token[0] == '\0')) {
                                obj_index oi; oi.v_idx = -1; oi.vn_idx = -1; oi.vt_idx = -1;
                                oi.v_idx = atoi(token); if (oi.v_idx > 0) oi.v_idx -= 1; else if (oi.v_idx < 0) oi.v_idx = v_count + oi.v_idx;
                                token += strcspn(token, "/ \t\r");
                                if (token[0] == '/') { // not just vertex pos alone:
                                        token++;
                                        if (token[0] == '/') { // 'v//vn' (vertex + normal, no texcoord)
                                                token++;
                                                oi.vn_idx = atoi(token); if (oi.vn_idx > 0) oi.vn_idx -= 1; else if (oi.vn_idx < 0) oi.vn_idx = v_count + oi.vn_idx;
                                                token += strcspn(token, "/ \t\r");
                                        } else { // 'v/vt' or 'v/vt/vn'
                                                oi.vt_idx = atoi(token); if (oi.vt_idx > 0) oi.vt_idx -= 1; else if (oi.vt_idx < 0) oi.vt_idx = v_count + oi.vt_idx;
                                                token += strcspn(token, "/ \t\r");
                                                if (token[0] == '/') { // 'v/vt/vn' (vertex + texcoord + normal)
                                                        token++;
                                                        oi.vn_idx = atoi(token); if (oi.vn_idx > 0) oi.vn_idx -= 1; else if (oi.vn_idx < 0) oi.vn_idx = v_count + oi.vn_idx;
                                                        token += strcspn(token, "/ \t\r");
                                                }
                                        }
                                }
                                face.push_back(oi);
                                token += strspn(token, " \t\r");
                        }
                        faces.push_back(std::move(face));
                        continue;
                }
        }

        // TODO
}
};