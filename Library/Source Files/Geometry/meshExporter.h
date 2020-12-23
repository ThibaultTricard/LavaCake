#pragma once
#include "mesh.h"
#include <fstream> 

namespace LavaCake {
  namespace Geometry {

    bool exportToPly(Mesh_t * m, char* filename) {
      std::ofstream ofs;
      ofs.open(filename, std::ofstream::out | std::ofstream::trunc);
      auto vertices = m->vertices();
      auto indices = m->indices();
      
      ofs << "ply" << std::endl
        << "format ascii 1.0 " << std::endl
        << "comment Made by us" << std::endl
        << "element vertex " << vertices.size() / m->vertexSize() << std::endl;
      auto format = m->getFormat();
      for (int i = 0; i < format.description().size(); i++) {
        if (format.description()[i] == POS2) {
          ofs << "property float x" << std::endl
            << "property float y" << std::endl;
        }
        if (format.description()[i] == POS3) {
          ofs << "property float x" << std::endl
              << "property float y" << std::endl
              << "property float z" << std::endl;
        }
        if (format.description()[i] == NORM3) {
          ofs << "property float nx" << std::endl
            << "property float ny" << std::endl
            << "property float nz" << std::endl;
        }
        if (format.description()[i] == UV) {
          ofs << "property float u" << std::endl
            << "property float v" << std::endl;
        }
        if (format.description()[i] == COL3) {
          ofs << "property float red" << std::endl
            << "property float green" << std::endl
            << "property float blue" << std::endl;
        }
        if (format.description()[i] == COL4) {
          ofs << "property float red" << std::endl
            << "property float green" << std::endl
            << "property float blue" << std::endl
            << "property float alpha" << std::endl;
        }
        if (format.description()[i] == F1) {
          ofs << "property float f" << std::to_string(i) << 1 << std::endl;
        }
        if (format.description()[i] == F2) {
          ofs << "property float f" << std::to_string(i) << 1 << std::endl;
          ofs << "property float f" << std::to_string(i) << 2 << std::endl;
        }
        if (format.description()[i] == F3) {
          ofs << "property float f" << std::to_string(i) << 1 << std::endl;
          ofs << "property float f" << std::to_string(i) << 2 << std::endl;
          ofs << "property float f" << std::to_string(i) << 3 << std::endl;
        }
        if (format.description()[i] == F4) {
          ofs << "property float f" << std::to_string(i) << 1 << std::endl;
          ofs << "property float f" << std::to_string(i) << 2 << std::endl;
          ofs << "property float f" << std::to_string(i) << 3 << std::endl;
          ofs << "property float f" << std::to_string(i) << 4 << std::endl;
        }
      }
        

      if (m->getTopology() == TRIANGLE) {
        if (m->isIndexed()) {
          ofs << "element face " << indices.size() / 3 << std::endl
            << "property list uchar uint vertex_indices" << std::endl
            << "end_header" << std::endl;
        }
        else {
          ofs << "element face " << vertices.size() / m->vertexSize() / 3 << std::endl
            << "property list uchar uint vertex_indices" << std::endl
            << "end_header" << std::endl;
        }
      }
      if (m->getTopology() == LINE) {
        if (m->isIndexed()) {
          ofs << "element edge " << indices.size() / 2 << std::endl
            << "property int vertex1" << std::endl
            << "property int vertex2" << std::endl
            << "end_header" << std::endl;
        }
        else {
          ofs << "element edge " << vertices.size() / m->vertexSize() / 2 << std::endl
            << "property int vertex1" << std::endl
            << "property int vertex2" << std::endl
            << "end_header" << std::endl;
        }
      }

      for (int i = 0; i < vertices.size(); i += m->vertexSize()) {
        for (int j = 0; j < m->vertexSize(); j++) {
          ofs << std::to_string(vertices[i + j]) + " ";
        }
        ofs << "\n";
      }

      if (m->getTopology() == TRIANGLE) {
        if (m->isIndexed()) {
          for (int i = 0; i < indices.size(); i += 3) {
            ofs << "3 " + std::to_string(indices[i]) + " " + std::to_string(indices[i + 1]) + " " + std::to_string(indices[i + 2]) + "\n";
          }
        }
        else {
          for (int i = 0; i < vertices.size() / m->vertexSize() ; i += 3) {
            ofs << "3 " + std::to_string(i) + " " + std::to_string(i + 1) + " " + std::to_string(i + 2) + "\n";
          }
        }
      }
      if (m->getTopology() == LINE) {
        if (m->isIndexed()) {
          for (int i = 0; i < indices.size(); i += 2) {
            ofs << "" + std::to_string(indices[i]) + " " + std::to_string(indices[i + 1]) + "\n";
          }
        }
        else {
          for (int i = 0; i < vertices.size() / m->vertexSize() ; i += 2) {
            ofs << "" + std::to_string(i) + " " + std::to_string(i + 1) + "\n";
          }
        }
      }
      ofs.close();
      return true;
    }
    
  }
}