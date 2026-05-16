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
#include <map>

struct GOBJ {
struct Vertex {
        float pos[3] = {0.0f, 0.0f, 0.0f};
        float normal[3] = {0.0f, 0.0f, 0.0f};
        float texcoord[2] = {0.0f, 0.0f};
};

std::vector<GOBJ::Vertex> vertices;
std::vector<int> indices;

private:
struct OBJIndex {
        int v_idx = -1, vn_idx = -1, vt_idx = -1; // Indices into OBJ triplet arrays (v, vn, & vt), which combined, form one vertex
        bool operator<(const OBJIndex& b) const { // for std::map
                if (this->v_idx != b.v_idx) return (this->v_idx < b.v_idx);
                if (this->vn_idx != b.vn_idx) return (this->vn_idx < b.vn_idx);
                if (this->vt_idx != b.vt_idx) return (this->vt_idx < b.vt_idx);
                return false;
        }
};
public:
GOBJ(const std::filesystem::path& obj_path) {
        // Parse OBJ:
        std::ifstream obj_file_stream(obj_path, std::ios::binary); if (!obj_file_stream.is_open()) throw std::runtime_error(std::string("ERROR: Failed to open '") + obj_path.string() + "'");
        std::vector<float> v, vn, vt; // OBJ vertex triplets; v = pos, vn = normal, vt = texcoord
        std::vector<std::vector<OBJIndex>> faces; // Collection of faces (face = collection of vertices (in this case, in the form of OBJ's triplet indices)) which, when combined, make a mesh
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
                        std::vector<OBJIndex> face;
                        const int v_count = v.size()/3; const int vn_count = vn.size()/3; const int vt_count = vt.size()/3; // for making index zero-base and supporting relative indexing
                        while (!(token[0] == '\r' || token[0] == '\n' || token[0] == '\0')) {
                                OBJIndex oi;
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
        } if (faces.empty()) throw std::runtime_error(std::string("ERROR: No faces found in '") + obj_path.string() + "'");

        // Triangulate & assemble parsed OBJ data into mesh:
        std::map<GOBJ::OBJIndex, int> cache; // For indices (re-using vertices)
        for (const std::vector<GOBJ::OBJIndex>& face : faces) {
                OBJIndex i0 = face[0];
                OBJIndex i1;
                OBJIndex i2 = face[1];
                for (size_t vtx = 2; vtx < face.size(); vtx++) {
                        i1 = i2;
                        i2 = face[vtx];

                        const std::map<OBJIndex, int>::iterator it0 = cache.find(i0);
                        if (it0 != cache.end()) indices.push_back(it0->second);
                        else {
                                vertices.emplace_back();
                                vertices.back().pos[0] = v[3*i0.v_idx + 0];
                                vertices.back().pos[1] = v[3*i0.v_idx + 1];
                                vertices.back().pos[2] = v[3*i0.v_idx + 2];
                                if (i0.vn_idx >= 0 && (3*i0.vn_idx + 2) < vn.size()) {
                                        vertices.back().normal[0] = vn[3*i0.vn_idx + 0];
                                        vertices.back().normal[1] = vn[3*i0.vn_idx + 1];
                                        vertices.back().normal[2] = vn[3*i0.vn_idx + 2];
                                }
                                if (i0.vt_idx >= 0 && (2*i0.vt_idx + 1) < vt.size()) {
                                        vertices.back().texcoord[0] = vt[2*i0.vt_idx + 0];
                                        vertices.back().texcoord[1] = vt[2*i0.vt_idx + 1];
                                }
                                indices.push_back(vertices.size() - 1);
                                cache[i0] = vertices.size() - 1;
                        }

                        const std::map<OBJIndex, int>::iterator it1 = cache.find(i1);
                        if (it1 != cache.end()) indices.push_back(it1->second);
                        else {
                                vertices.emplace_back();
                                vertices.back().pos[0] = v[3*i1.v_idx + 0];
                                vertices.back().pos[1] = v[3*i1.v_idx + 1];
                                vertices.back().pos[2] = v[3*i1.v_idx + 2];
                                if (i1.vn_idx >= 0 && (3*i1.vn_idx + 2) < vn.size()) {
                                        vertices.back().normal[0] = vn[3*i1.vn_idx + 0];
                                        vertices.back().normal[1] = vn[3*i1.vn_idx + 1];
                                        vertices.back().normal[2] = vn[3*i1.vn_idx + 2];
                                }
                                if (i1.vt_idx >= 0 && (2*i1.vt_idx + 1) < vt.size()) {
                                        vertices.back().texcoord[0] = vt[2*i1.vt_idx + 0];
                                        vertices.back().texcoord[1] = vt[2*i1.vt_idx + 1];
                                }
                                indices.push_back(vertices.size() - 1);
                                cache[i1] = vertices.size() - 1;
                        }

                        const std::map<OBJIndex, int>::iterator it2 = cache.find(i2);
                        if (it2 != cache.end()) indices.push_back(it2->second);
                        else {
                                vertices.emplace_back();
                                vertices.back().pos[0] = v[3*i2.v_idx + 0];
                                vertices.back().pos[1] = v[3*i2.v_idx + 1];
                                vertices.back().pos[2] = v[3*i2.v_idx + 2];
                                if (i2.vn_idx >= 0 && (3*i2.vn_idx + 2) < vn.size()) {
                                        vertices.back().normal[0] = vn[3*i2.vn_idx + 0];
                                        vertices.back().normal[1] = vn[3*i2.vn_idx + 1];
                                        vertices.back().normal[2] = vn[3*i2.vn_idx + 2];
                                }
                                if (i2.vt_idx >= 0 && (2*i2.vt_idx + 1) < vt.size()) {
                                        vertices.back().texcoord[0] = vt[2*i2.vt_idx + 0];
                                        vertices.back().texcoord[1] = vt[2*i2.vt_idx + 1];
                                }
                                indices.push_back(vertices.size() - 1);
                                cache[i2] = vertices.size() - 1;
                        }
                }
        }
}
};