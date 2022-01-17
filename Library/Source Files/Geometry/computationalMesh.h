#pragma once
#include "mesh.h"
#include <algorithm>

namespace LavaCake {
  namespace Geometry {
    struct polygone_t;
    struct edge_t;
    struct vertex_t;

    struct vertex_t {
      
      vec3f p;
      vec3f n;

      uint32_t indice = 0;

      std::vector<edge_t*> edges;
      std::vector<polygone_t*> faces;
    };

    struct edge_t {
      vertex_t* v1;
      vertex_t* v2;

      uint32_t indice = 0;

      std::vector<polygone_t*> faces;
    };

    struct polygone_t {
      std::vector<vertex_t*> vertices;
      std::vector<edge_t*> edges;


      uint32_t indice = 0;

      bool marked = false;
    };

    float area(vec3f v1, vec3f v2, vec3f v3) {
      vec3f B = v2 - v1;
      float BL = sqrt(Dot(B,B));

      vec3f b = Normalize(B);
      vec3f e = v3 - v1;


      vec3f v = b * Dot(e, b);

      vec3f H = v3 - v;


      return 1.0f / 2.0f * sqrt(Dot(H,H)) * BL;
    }

     
    //plan.first : origin, plan.second : normal
    vec3f projectToPlan(std::pair<vec3f, vec3f> plan, vec3f vertex) {

      vec3f v = vertex - plan.first;

      float s = Dot(v, plan.second);
      return vertex - s * plan.second;
    };

    class PolygonalMesh {
      public :
      PolygonalMesh() {};


      PolygonalMesh(TriangleIndexedMesh* m) {
        int pos = -1;
        
        int norm = -1;
        
        int offset = 0;
        for (size_t s = 0;  s<m->getFormat().description().size(); s++ ) {
          if (m->getFormat().description()[s] == POS3) {
            pos = offset;
          }

          if (m->getFormat().description()[s] == NORM3) {
            norm = offset;
          }
          offset += (int)toSize(m->getFormat().description()[s]);

        };

        if (pos == -1) {
          return;
        }

        for (size_t s = 0; s < m->vertices().size(); s+= m->vertexSize()) {
          vertex_t* v = new vertex_t();

          v->p = vec3f({ m->vertices()[s + pos], m->vertices()[s + pos + 1],  m->vertices()[s + pos + 2] });

          if (norm != -1) {
            v->n = vec3f({
             m->vertices()[s + norm],
             m->vertices()[s + norm + 1],
             m->vertices()[s + norm + 2],
              });
          }
          else {
            v->n = vec3f({0.0f,0.0f,0.0f});
          }

          v->indice = (uint32_t)vertices.size();

          vertices.emplace_back(v);
        }

        for (size_t s = 0; s < m->indices().size(); s += 3) {
          polygone_t* p = new polygone_t();

          p->vertices.emplace_back(vertices[m->indices()[s]]);
          p->vertices.emplace_back(vertices[m->indices()[s+1]]);
          p->vertices.emplace_back(vertices[m->indices()[s+2]]);

          vertices[m->indices()[s]]->faces.emplace_back(p);
          vertices[m->indices()[s + 1]]->faces.emplace_back(p);
          vertices[m->indices()[s + 2]]->faces.emplace_back(p);

          edge_t* e1 = new edge_t();
          e1->v1 = vertices[m->indices()[s]];
          e1->v2 = vertices[m->indices()[s + 1]];

          edge_t* e2 = new edge_t();
          e2->v1 = vertices[m->indices()[s + 1]];
          e2->v2 = vertices[m->indices()[s + 2]];

          edge_t* e3 = new edge_t();
          e3->v1 = vertices[m->indices()[s + 2]];
          e3->v2 = vertices[m->indices()[s]];

          bool found = false;

          for (size_t i = 0; i < vertices[m->indices()[s]]->edges.size(); i++) {
            if ((vertices[m->indices()[s]]->edges[i]->v1 == e1->v1 || vertices[m->indices()[s]]->edges[i]->v1 == e1->v2) &&
              (vertices[m->indices()[s]]->edges[i]->v2 == e1->v1 || vertices[m->indices()[s]]->edges[i]->v2 == e1->v2)) {
              e1 = vertices[m->indices()[s]]->edges[i];
              found = true;
              break;
            }
          }

          e1->faces.push_back(p);
          if (!found) {
            vertices[m->indices()[s]]->edges.emplace_back(e1);
            vertices[m->indices()[s+1]]->edges.emplace_back(e1);
            e1->indice = (uint32_t)edges.size();
            edges.emplace_back(e1);
            
          }
          found = false;

          for (size_t i = 0; i < vertices[m->indices()[s+1]]->edges.size(); i++) {
            if ((vertices[m->indices()[s+1]]->edges[i]->v1 == e2->v1 || vertices[m->indices()[s+1]]->edges[i]->v1 == e2->v2) &&
              (vertices[m->indices()[s+1]]->edges[i]->v2 == e2->v1 || vertices[m->indices()[s+1]]->edges[i]->v2 == e2->v2)) {
              e2 = vertices[m->indices()[s+1]]->edges[i];
              found = true;
              break;
            }
          }
          e2->faces.push_back(p);
          if (!found) {
            vertices[m->indices()[s + 1]]->edges.emplace_back(e2);
            vertices[m->indices()[s + 2]]->edges.emplace_back(e2);
            e2->indice = (uint32_t)edges.size();
            edges.emplace_back(e2);
            
          }
          found = false;

          for (size_t i = 0; i < vertices[m->indices()[s+2]]->edges.size(); i++) {
            if ((vertices[m->indices()[s + 2]]->edges[i]->v1 == e3->v1 || vertices[m->indices()[s + 2]]->edges[i]->v1 == e3->v2) &&
              (vertices[m->indices()[s + 2]]->edges[i]->v2 == e3->v1 || vertices[m->indices()[s + 2]]->edges[i]->v2 == e3->v2)) {
              e3 = vertices[m->indices()[s + 2]]->edges[i];
              found = true;
              break;
            }
          }
          e3->faces.push_back(p);
          if (!found) {
            vertices[m->indices()[s + 2]]->edges.emplace_back(e3);
            vertices[m->indices()[s    ]]->edges.emplace_back(e3);
            e3->indice = (uint32_t)edges.size();
            edges.emplace_back(e3);   
          }
          found = false;

          p->edges.emplace_back(e1);
          p->edges.emplace_back(e2);
          p->edges.emplace_back(e3);
          p->indice = (uint32_t)faces.size();
          faces.emplace_back(p);
        }
        onlyTriangle = true;
      };

      void triangulation() {
        for (size_t s = 0; s < faces.size(); s++) {
          if (faces[s]->vertices.size() > 3) {

          }
        }
      };

      std::vector<float> GaussianCurvature(float radius) {
        if (!onlyTriangle) {
          return {};
        }
        std::vector<float> res;

        std::vector<float> projected;

        //initialisation 
        for (size_t s = 0; s < faces.size(); s++) {
          faceArea.push_back(area(faces[s]->vertices[0]->p, faces[s]->vertices[1]->p, faces[s]->vertices[2]->p));

          vec3f na = faces[s]->vertices[0]->n;
          vec3f nb = faces[s]->vertices[1]->n;
          vec3f nc = faces[s]->vertices[2]->n;

          vec3f tab = Normalize(nb - na - (Dot(nb - na, na)) * na);
          vec3f tba = Normalize(na - nb - (Dot(na - nb, nb)) * nb);

          vec3f tac = Normalize(nc - na - (Dot(nc - na, na)) * na);
          vec3f tca = Normalize(na - nc - (Dot(na - nc, nc)) * nc);

          vec3f tbc = Normalize(nc - nb - (Dot(nc - nb, nb)) * nb);
          vec3f tcb = Normalize(nb - nc - (Dot(nb - nc, nc)) * nc);

          float thetaA = acos(Dot(tab, tac));
          float thetaB = acos(Dot(tbc, tba));
          float thetaC = acos(Dot(tca, tcb));
          faceCurvature.push_back(thetaA + thetaB + thetaC - 3.14159265f);

        }



        for (size_t s = 0; s < vertices.size(); s++) {



          float totalarea = 0.0f;
          float projected = 0.0f;

          for (size_t i = 0; i < vertices[s]->faces.size(); i++) {
            totalarea += faceArea[vertices[s]->faces[i]->indice];
            projected += faceCurvature[vertices[s]->faces[i]->indice];
          }

          verticesCurvature.push_back(projected / totalarea);
        }
      }

      std::vector<vertex_t*> vertices;
      std::vector<edge_t*> edges;
      std::vector<polygone_t*> faces;

      std::vector<float> verticesCurvature;
      std::vector<float> faceArea;
      std::vector<float> faceCurvature;

      private : 
        bool onlyTriangle = false;
    };
      

  }
}