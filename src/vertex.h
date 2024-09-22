#ifndef VERTEX_H
#define VERTEX_H

#include <QListWidget>
#include <la.h>
#include <vertexpos.h>

class HalfEdge;


class Vertex
{
public:
    static int lastVertex;
    VertexPos* vertexPos;
    HalfEdge* edge;     // pointer to one of the HalfEdges that points to this Vertex
    int id;             // unique integer to identify the Vertex
    QString name;
    glm::vec3 normal;
    glm::vec2 uv;

    Vertex();
    Vertex(const Vertex &vertex);
    void ReName();
    glm::vec3& pos();
};

#endif // VERTEX_H
