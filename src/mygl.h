#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <QListWidget>

#include <mesh.h>
#include <vertexdisplay.h>
#include <facedisplay.h>
#include <halfedgedisplay.h>
#include <jointdisplay.h>
#include <texture.h>


class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_lambertTexture;// A shader program that uses blinnPhong
    ShaderProgram m_lambertTextureSkeleton;
    ShaderProgram m_lambertSkeleton;


    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // Have an instance of [...]Display that is
    // drawn in paintGL, and has VBO data representing
    // the position of the currently selected [...] so
    // it can be drawn as a GL_POINTS
    VertexDisplay m_vertDisplay;
    FaceDisplay m_faceDisplay;
    HalfEdgeDisplay m_halfEdgeDisplay;
    JointDisplay m_jointDisplay;
    uPtr<Mesh> m_mesh;

    QListWidgetItem* m_selected;
    glm::vec3 color;
    bool showTexture=true;

protected:
    void keyPressEvent(QKeyEvent *e);

public slots:
    void slot_vertPosX(double x);
    void slot_vertPosY(double y);
    void slot_vertPosZ(double z);

    void slot_jointPosX(double x);
    void slot_jointPosY(double y);
    void slot_jointPosZ(double z);

    void slot_jointRotateX();
    void slot_jointRotateY();
    void slot_jointRotateZ();

    void slot_faceRed(double red);
    void slot_faceGreen(double green);
    void slot_faceBlue(double blue);

    void slot_subdivision();

    void slot_setSelectedVertex(QListWidgetItem *v);
    void slot_setSelectedFace(QListWidgetItem *f);
    void slot_setSelectedHalfEdge(QListWidgetItem *he);
    void slot_setSelectedJoint(QListWidgetItem *j);

    void slot_readMeshFromFile();
    void slot_readSkeletonFromFile();
    void slot_switchTextureMode(){
        showTexture=!showTexture;
        update();
    }

signals:
    void sig_sendVertexData(Mesh *mesh);
    void sig_sendFaceData(Mesh *mesh);
    void sig_sendHalfEdgeData(Mesh *mesh);
    void sig_sendJointData(Mesh *mesh);
    void sig_selectedUpdate(QListWidgetItem* selected);

};


#endif // MYGL_H
