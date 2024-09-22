#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
    m_progLambert(this), m_progFlat(this),m_lambertTexture(this),m_lambertTextureSkeleton(this),m_lambertSkeleton(this),
      m_glCamera(),  m_vertDisplay(this),
    m_faceDisplay(this),m_halfEdgeDisplay(this),m_jointDisplay(this),
      m_mesh(nullptr), m_selected(nullptr),
      color(glm::vec3())
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
    m_mesh->destroy();
    m_vertDisplay.destroy();
    m_faceDisplay.destroy();
    m_halfEdgeDisplay.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    //m_geomSquare.create();

    // m_mesh->buildCube();
    m_mesh = mkU<Mesh>(this);
    m_mesh->ReadFile("C:/Users/Griffith/Desktop/CIS4600/HW6/scenes/cube.obj");
    m_mesh->ReadTexture("C:/Users/Griffith/Desktop/CIS4600/HW6/textures/smb.jpg");
    m_mesh->create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    printGLErrorLog();
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    printGLErrorLog();
    // Create and set up the flat blinnPhong shader
    m_lambertTexture.create(":/glsl/lambertTexture.vert.glsl", ":/glsl/lambertTexture.frag.glsl");
    printGLErrorLog();
    m_lambertTextureSkeleton.create(":/glsl/lambertTextureSkeleton.vert.glsl", ":/glsl/lambertTextureSkeleton.frag.glsl");
    printGLErrorLog();
    m_lambertSkeleton.create(":/glsl/lambertSkeleton.vert.glsl", ":/glsl/lambertSkeleton.frag.glsl");
    printGLErrorLog();

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindTexture(GL_TEXTURE_2D, m_mesh->m_texture->m_textureHandle);
    glBindVertexArray(vao);

    emit sig_sendVertexData(m_mesh.get());
    emit sig_sendFaceData(m_mesh.get());
    emit sig_sendHalfEdgeData(m_mesh.get());
    emit sig_sendJointData(m_mesh.get());
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_progFlat.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertTexture.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertTextureSkeleton.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertSkeleton.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    glm::mat4 model = glm::mat4();
    m_progLambert.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_progLambert.setCamPos(glm::vec3(m_glCamera.eye));
    m_progLambert.setModelMatrix(model);
    // m_progLambert.draw(m_mesh);

    m_lambertTexture.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertTexture.setCamPos(glm::vec3(m_glCamera.eye));
    m_lambertTexture.setModelMatrix(model);
    m_lambertTexture.setTexture(0);
    glBindTexture(GL_TEXTURE_2D, m_mesh->m_texture->m_textureHandle);


    m_lambertTextureSkeleton.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertTextureSkeleton.setCamPos(glm::vec3(m_glCamera.eye));
    m_lambertTextureSkeleton.setModelMatrix(model);
    m_lambertTextureSkeleton.setTexture(0);

    m_lambertSkeleton.setViewProjMatrix(m_glCamera.getView(),m_glCamera.getProj());
    m_lambertSkeleton.setCamPos(glm::vec3(m_glCamera.eye));
    m_lambertSkeleton.setModelMatrix(model);
    m_lambertSkeleton.setTexture(0);
    glBindTexture(GL_TEXTURE_2D, m_mesh->m_texture->m_textureHandle);

    if(m_mesh->joints.size()>0){
        glm::mat4 bindMatrix[100];
        glm::mat4 currentJointMatrix[100];
        for(size_t i=0;i<m_mesh->joints.size();i++){
            Joint* j = m_mesh->joints[i].get();
            bindMatrix[j->id]=j->bindMatrix;
            currentJointMatrix[j->id]=j->getOverallTransformation();
        }
        if(showTexture){
            m_lambertTextureSkeleton.setBindMatrix(bindMatrix);
            m_lambertTextureSkeleton.setCurrentJointMatrix(currentJointMatrix);
            m_lambertTextureSkeleton.draw(*m_mesh);
        }else{
            m_lambertSkeleton.setBindMatrix(bindMatrix);
            m_lambertSkeleton.setCurrentJointMatrix(currentJointMatrix);
            m_lambertSkeleton.draw(*m_mesh);
        }
    }else{
        if(showTexture)
            m_lambertTexture.draw(*m_mesh);
        else
            m_progLambert.draw(*m_mesh);
        }

    VertexPos* vertex = dynamic_cast<VertexPos*>(m_selected);
    Face* face = dynamic_cast<Face*>(m_selected);
    HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(m_selected);
    // Joint* joint = dynamic_cast<Joint*>(m_selected);

    glDisable(GL_DEPTH_TEST);

    if(m_mesh->joints.size()>0){
        for(const auto& joint: m_mesh->joints){
            m_jointDisplay.setRepresentJoint(joint.get());
            m_jointDisplay.create();
            m_progFlat.draw(m_jointDisplay);
        }
    }

    if (vertex) {
        m_vertDisplay.create();
        m_progFlat.draw(m_vertDisplay);
    } else if (face) {
        m_faceDisplay.create();
        m_progFlat.draw(m_faceDisplay);
    } else if (halfEdge) {
        m_halfEdgeDisplay.create();
        m_progFlat.draw(m_halfEdgeDisplay);
    }
    glEnable(GL_DEPTH_TEST);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    auto val = e->key();
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Return) {
        // m_glCamera.RotateAboutUp(-amount);
        m_glCamera.RotateAboutUpSphere(amount);
    } else if (e->key() == Qt::Key_Left|| e->key() == Qt::Key_Backtab) {
        // m_glCamera.RotateAboutUp(amount);
        m_glCamera.RotateAboutUpSphere(-amount);
    } else if (e->key() == Qt::Key_Up|| e->key() == Qt::Key_Backspace) {
        // m_glCamera.RotateAboutRight(-amount);
        m_glCamera.RotateAboutRightSphere(-amount);
    } else if (e->key() == Qt::Key_Down|| e->key() == Qt::Key_Enter) {
        // m_glCamera.RotateAboutRight(amount);
        m_glCamera.RotateAboutRightSphere(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());

    // visual debugging
    } else if (e->key() == Qt::Key_N) {
        HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(m_selected);
        if (halfEdge) {
            slot_setSelectedHalfEdge(halfEdge->next);
        }
    } else if (e->key() == Qt::Key_M) {
        HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(m_selected);
        if (halfEdge) {
            slot_setSelectedHalfEdge(halfEdge->symm);
        }
    } else if (e->key() == Qt::Key_F) {
        HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(m_selected);
        if (halfEdge) {
            slot_setSelectedFace(halfEdge->face);
        }
    } else if (e->key() == Qt::Key_V) {
        HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(m_selected);
        if (halfEdge) {
            slot_setSelectedVertex(halfEdge->vertex->vertexPos);
        }
    } else if ((e->modifiers() & Qt::ShiftModifier) && e->key() == Qt::Key_H) {
        Face* face = dynamic_cast<Face*>(m_selected);
        if (face) {
            slot_setSelectedHalfEdge(face->edge);
        }
    }
    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}

void MyGL::slot_vertPosX(double x) {
    if (m_selected) {
        VertexPos* vertex = dynamic_cast<VertexPos*>(m_selected);

        if (vertex) {
            vertex->pos.x = x;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}


void MyGL::slot_vertPosY(double y) {
    if (m_selected) {
        VertexPos* vertex = dynamic_cast<VertexPos*>(m_selected);

        if (vertex) {
            vertex->pos.y = y;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_vertPosZ(double z) {
    if (m_selected) {
        VertexPos* vertex = dynamic_cast<VertexPos*>(m_selected);

        if (vertex) {
            vertex->pos.z = z;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_jointPosX(double x) {
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->pos.x = x;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_jointRotateX(){
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->rotateByMyself(5.0f,glm::vec3(1,0,0));
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}
void MyGL::slot_jointRotateY(){
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->rotateByMyself(5.0f,glm::vec3(0,1,0));
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }

}
void MyGL::slot_jointRotateZ(){
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->rotateByMyself(5.0f,glm::vec3(0,0,1));
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }

}


void MyGL::slot_jointPosY(double y) {
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->pos.y = y;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_jointPosZ(double z) {
    if (m_selected) {
        Joint* joint = dynamic_cast<Joint*>(m_selected);

        if (joint) {
            joint->pos.z = z;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_faceRed(double red) {
    if (m_selected) {
        Face* face = dynamic_cast<Face*>(m_selected);

        if (face) {
            color.r = red;
            face->color = color;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_faceGreen(double green) {
    if (m_selected) {
        Face* face = dynamic_cast<Face*>(m_selected);

        if (face) {
            color.g = green;
            face->color = color;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_faceBlue(double blue) {
    if (m_selected) {
        Face* face = dynamic_cast<Face*>(m_selected);

        if (face) {
            color.b = blue;
            face->color = color;
            m_mesh->destroy();
            m_mesh->create();
            update();
        }
    }
}

void MyGL::slot_setSelectedVertex(QListWidgetItem *v) {
    if (v) {
        m_selected = v;
        VertexPos* vertex = dynamic_cast<VertexPos*>(v);

        m_vertDisplay.destroy();

        if (vertex) {
            m_vertDisplay.updateVertex(vertex);
        }
        update();
    }
    emit sig_selectedUpdate(m_selected);
}

void MyGL::slot_setSelectedFace(QListWidgetItem *f) {
    if (f) {
        m_selected = f;
        Face* face = dynamic_cast<Face*>(f);

        m_faceDisplay.destroy();

        if (face) {
            m_faceDisplay.updateFace(face);
        }
        update();
    }
    emit sig_selectedUpdate(m_selected);
}

void MyGL::slot_setSelectedHalfEdge(QListWidgetItem *he) {
    if (he) {
        m_selected = he;
        HalfEdge* halfEdge = dynamic_cast<HalfEdge*>(he);

        m_halfEdgeDisplay.destroy();

        if (halfEdge) {
            m_halfEdgeDisplay.updateHalfEdge(halfEdge);
        }
        update();
    }
    emit sig_selectedUpdate(m_selected);
}

void MyGL::slot_setSelectedJoint(QListWidgetItem *j){
    if (j) {
        m_selected = j;
        Joint* joint = dynamic_cast<Joint*>(j);

        m_jointDisplay.destroy();

        if (joint) {
            m_jointDisplay.updateJoint(joint);
        }
        update();
    }
    emit sig_selectedUpdate(m_selected);
}


void MyGL::slot_subdivision() {
    m_mesh->Subdivision();
    m_mesh->destroy();
    m_mesh->create();
    update();
    emit sig_sendVertexData(m_mesh.get());
    emit sig_sendFaceData(m_mesh.get());
    emit sig_sendHalfEdgeData(m_mesh.get());
}

void MyGL::slot_readMeshFromFile(){
    QString openFileName = QFileDialog::getOpenFileName(
        nullptr,                                // Parent widget, usually `this` if inside a member function of a widget
        "Open File",                            // Dialog title
        "../../scenes",                         // Default directory
        "3D Models (*.obj)"  // Filter for file types
        );

    if (!openFileName.isEmpty()) {
        QString openTextureName = QFileDialog::getOpenFileName(
            nullptr,                                // Parent widget, usually `this` if inside a member function of a widget
            "Choose Texture",                            // Dialog title
            "../../textures",                         // Default directory
            "Images (*.jpg *.jpeg *.png *.bmp *.gif *.tiff *.ico)"  // Filter for file types
            );
        if(!openTextureName.isEmpty()){
            m_mesh->ReadFile(openFileName.toStdString());
            m_mesh->ReadTexture(openTextureName.toStdString());
            m_mesh->destroy();
            m_mesh->create();
            update();
            emit sig_sendVertexData(m_mesh.get());
            emit sig_sendFaceData(m_mesh.get());
            emit sig_sendHalfEdgeData(m_mesh.get());
        }
    }
}

void MyGL::slot_readSkeletonFromFile(){
    QString openFileName = QFileDialog::getOpenFileName(
        nullptr,                                // Parent widget, usually `this` if inside a member function of a widget
        "Open File",                            // Dialog title
        "../../jsons",                         // Default directory
        "Skeletons (*.json)"  // Filter for file types
        );
    if (!openFileName.isEmpty()) {
            m_mesh->ReadSkeletonJson(openFileName.toStdString());
            m_mesh->destroy();
            m_mesh->create();
            update();
            emit sig_sendJointData(m_mesh.get());
    }
}
