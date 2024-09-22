#include "halfedge.h"
int HalfEdge::lastHalfEdge=0;
HalfEdge::HalfEdge()
    : QListWidgetItem(), next(nullptr), symm(nullptr),
      face(nullptr), vertex(nullptr), id(0), name("HalfEdge ")
{
    id = lastHalfEdge++;
    name.append(QString::number(id));
    this->setText(name);
}

HalfEdge::HalfEdge(const HalfEdge &halfEdge)
    : QListWidgetItem(), next(halfEdge.next),
      symm(halfEdge.symm), face(halfEdge.face),
      vertex(halfEdge.vertex), id(halfEdge.id), name(halfEdge.name)
{}

void HalfEdge::ReName(){
    assert(next!=nullptr);
    // assert(symm!=nullptr);
    assert(vertex!=nullptr);
    assert(face!=nullptr);
    assert(id>=0);
    name="HalfEdge ";
    name.append(QString::number(id));
}
