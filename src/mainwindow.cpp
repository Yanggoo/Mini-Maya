#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    connect(ui->mygl, SIGNAL(sig_sendVertexData(Mesh*)),
            this, SLOT(slot_receiveVertexData(Mesh*)));
    connect(ui->mygl, SIGNAL(sig_sendFaceData(Mesh*)),
            this, SLOT(slot_receiveFaceData(Mesh*)));
    connect(ui->mygl, SIGNAL(sig_sendHalfEdgeData(Mesh*)),
            this, SLOT(slot_receiveHalfEdgeData(Mesh*)));
    connect(ui->mygl, SIGNAL(sig_sendJointData(Mesh*)),
            this, SLOT(slot_receiveJointData(Mesh*)));

    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedVertex(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedFace(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedHalfEdge(QListWidgetItem*)));
    connect(ui->jointsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedJoint(QListWidgetItem*)));

    // vertex position spin boxes
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_vertPosX(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_vertPosY(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_vertPosZ(double)));

    // joint position spin boxes
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_jointPosX(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_jointPosY(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_jointPosZ(double)));

    // joint rotation buttons
    connect(ui->jointRotationX,SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_jointRotateX()));
    connect(ui->jointRotationY,SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_jointRotateY()));
    connect(ui->jointRotationZ,SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_jointRotateZ()));

    // color spin boxes
    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_faceRed(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_faceGreen(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_faceBlue(double)));

    // topology editing functions
    connect(ui->subdivisionButton, SIGNAL(clicked(bool)), this, SLOT(slot_subDivision()));

    //Read File
    connect(ui->readFileButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(slot_readMeshFromFile()));
    connect(ui->readSkeletonButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(slot_readSkeletonFromFile()));

    //Selected Update
    connect(ui->mygl, SIGNAL(sig_selectedUpdate(QListWidgetItem*)),
            this, SLOT(slot_updateSeletedInformation(QListWidgetItem*)));

    connect(ui->switchRenderModeButton, SIGNAL(clicked(bool)), ui->mygl, SLOT(slot_switchTextureMode()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::slot_receiveVertexData(Mesh *mesh) {
    // //ui->vertsListWidget->clear();
    for (const uPtr<VertexPos> &v : mesh->verticesPos) {
        ui->vertsListWidget->addItem(v.get());
    }
}

void MainWindow::slot_receiveFaceData(Mesh *mesh) {
    // //ui->facesListWidget->clear();
    for (const uPtr<Face> &f : mesh->faces) {
        ui->facesListWidget->addItem(f.get());
    }
}

void MainWindow::slot_receiveHalfEdgeData(Mesh *mesh) {
    //ui->halfEdgesListWidget->clear();
    Face::lastFace=0;
    for (const uPtr<HalfEdge> &he : mesh->halfEdges) {
        ui->halfEdgesListWidget->addItem(he.get());
    }
}

void MainWindow::slot_receiveJointData(Mesh *mesh) {
    Joint::lastJoint=0;
    for (const uPtr<Joint> &j : mesh->joints) {
        ui->jointsListWidget->addItem(j.get());
    }
}

void MainWindow::slot_subDivision(){
    ui->mygl->slot_subdivision();
}

void MainWindow::slot_updateSeletedInformation(QListWidgetItem* selected){
    ui->faceRedSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);

    ui->vertPosXSpinBox->setValue(0);
    ui->vertPosYSpinBox->setValue(0);
    ui->vertPosZSpinBox->setValue(0);
    ui->faceRedSpinBox->setValue(0);
    ui->faceBlueSpinBox->setValue(0);
    ui->faceGreenSpinBox->setValue(0);
    if(selected){
        Face* face = dynamic_cast<Face*>(selected);
        VertexPos* vertex = dynamic_cast<VertexPos*>(selected);
        Joint* joint = dynamic_cast<Joint*>(selected);
        if(face){
            ui->faceRedSpinBox->setValue(face->color[0]);
            ui->faceGreenSpinBox->setValue(face->color[1]);
            ui->faceBlueSpinBox->setValue(face->color[2]);
        }
        if(vertex){
            ui->vertPosXSpinBox->setValue(vertex->pos.x);
            ui->vertPosYSpinBox->setValue(vertex->pos.y);
            ui->vertPosZSpinBox->setValue(vertex->pos.z);
        }
        if(joint){
            ui->vertPosXSpinBox->setValue(joint->pos.x);
            ui->vertPosYSpinBox->setValue(joint->pos.y);
            ui->vertPosZSpinBox->setValue(joint->pos.z);
        }

    }

    ui->faceRedSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
}
