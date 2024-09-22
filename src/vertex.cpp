#include "vertex.h"
int Vertex::lastVertex=0;
Vertex::Vertex()
    : vertexPos(nullptr),
      edge(nullptr), id(0), name("vertex ")
{
    id = lastVertex++;
    name.append(QString::number(id));
}

Vertex::Vertex(const Vertex &vertex)
    : vertexPos(vertex.vertexPos),
      edge(vertex.edge), id(vertex.id),
      name(vertex.name)
{}

void Vertex::ReName(){
    assert(edge!=nullptr);
    assert(id>=0);
    name="vertex";
    name.append(QString::number(id));
}

glm::vec3& Vertex::pos(){
    assert(vertexPos!=nullptr);
    return vertexPos->pos;
}


