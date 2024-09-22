#ifndef JOINTDISPLAY_H
#define JOINTDISPLAY_H

#include <drawable.h>
#include <joint.h>
#include <la.h>


class JointDisplay : public Drawable
{
protected:
    Joint* representJoint;
    Joint* selectedJoint;
public:
    void virtual create() override;
    void updateJoint(Joint*);
    void setRepresentJoint(Joint*);
    GLenum drawMode() override;
    JointDisplay(OpenGLContext* context);
    ~JointDisplay();
};

#endif // JOINTDISPLAY_H
