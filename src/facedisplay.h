#ifndef FACEDISPLAY_H
#define FACEDISPLAY_H

#include <drawable.h>
#include <face.h>
#include <la.h>
#include <halfedge.h>

class FaceDisplay : public Drawable
{
protected:
    Face *representedFace;

public:
    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void virtual create() override;
    // Change which Face representedFace points to
    void updateFace(Face*);

    GLenum drawMode() override;

    FaceDisplay(OpenGLContext* context);

    ~FaceDisplay();
};

#endif // FACEDISPLAY_H
