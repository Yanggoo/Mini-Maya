#ifndef HALFEDGEDISPLAY_H
#define HALFEDGEDISPLAY_H

#include <drawable.h>
#include <halfedge.h>
#include <la.h>

class HalfEdgeDisplay : public Drawable
{
protected:
    HalfEdge *representedHalfEdge;
public:
    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void virtual create() override;
    // Change which HalfEdge representedHalfEdge points to
    void updateHalfEdge(HalfEdge*);

    GLenum drawMode() override;

    HalfEdgeDisplay(OpenGLContext* context);

    ~HalfEdgeDisplay();
};

#endif // HALFEDGEDISPLAY_H
