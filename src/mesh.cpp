#include "mesh.h"
#include <fstream>
#include <regex>
#include <string>
#include <algorithm>
#include <utility>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <unordered_set>
#include <queue>

Mesh::Mesh(OpenGLContext* context):Drawable(context), faces{}, halfEdges{}, vertices{},verticesPos{},joints{}{
    m_texture = mkU<Texture>(context);
}

void Mesh::Clear(){
    faces.clear();
    halfEdges.clear();
    vertices.clear();
    verticesPos.clear();
    VertexPos::lastVertexPos=0;
    Vertex::lastVertex=0;
    Face::lastFace=0;
    HalfEdge::lastHalfEdge=0;
}

void Mesh::create(){
    ReIndex();
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec4> color;
    std::vector<glm::vec2> uv;
    std::vector<GLuint> idx;

    for(const auto&vertex:vertices){
        pos.push_back(glm::vec4(vertex->pos(),1));
        nor.push_back(glm::vec4(vertex->normal,1));
        color.push_back(glm::vec4(vertex->edge->face->color,0));
        uv.push_back(vertex->uv);
    }

    for(const auto&face :faces){
        HalfEdge* start =face->edge;
        HalfEdge* current =start->next;
        while(current->next!=start){
            idx.push_back(start->vertex->id);
            idx.push_back(current->vertex->id);
            idx.push_back(current->next->vertex->id);
            current=current->next;
        }
    }

    count = idx.size();

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    bindNor();
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateCol();
    bindCol();
    mp_context->glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec4), color.data(), GL_STATIC_DRAW);


    if(m_texture){
        m_texture->load(0);
        m_texture->bind(0);
        generateUV();
        bindUV();
        mp_context->glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);
    }

    if(joints.size()>0){
        std::vector<glm::ivec2>jointIds;
        std::vector<glm::vec2>jointFactors;
        for(const auto&vertex:vertices){
            jointIds.emplace_back(vertex->vertexPos->joints[0].joint->id,vertex->vertexPos->joints[1].joint->id);
            jointFactors.emplace_back(vertex->vertexPos->joints[0].factor,vertex->vertexPos->joints[1].factor);
        }
        generateJointIds();
        bindJointIds();
        mp_context->glBufferData(GL_ARRAY_BUFFER, jointIds.size() * sizeof(glm::ivec2), jointIds.data(), GL_STATIC_DRAW);
        generateJointFactors();
        bindJointFactors();
        mp_context->glBufferData(GL_ARRAY_BUFFER, jointFactors.size() * sizeof(glm::vec2), jointFactors.data(), GL_STATIC_DRAW);
    }
}
GLenum Mesh::drawMode() {return GL_TRIANGLES;}

void Mesh::ReadTexture(std::string openFilePath){
    m_texture->create(openFilePath.c_str());
}
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        // 使用 std::hash 来计算两个指针的哈希值并组合
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};
void Mesh::ReadFile(std::string openFilePath){
    Clear();
    std::unordered_map<std::pair<void*,void*>,HalfEdge*,pair_hash> EdgeHash;
    std::ifstream file(openFilePath);
    std::vector<glm::vec3>_pos,_normal;
    std::vector<glm::vec2>_uv;
    std::vector<glm::vec3>_faces;
    std::string line;
    std::regex patternPos(R"(v\s*[+-]?\d*\.\d+\s*[+-]?\d*\.\d+\s*[+-]?\d*\.\d+)");
    std::regex patternN(R"(vn\s*[+-]?\d*\.\d+\s*[+-]?\d*\.\d+\s*[+-]?\d*\.\d+)");
    std::regex patternUV(R"(vt\s*[+-]?\d*\.\d+\s*[+-]?\d*\.\d+)");
    std::regex patternF1(R"(f\s+(\d+/\d+/\d+\s*)+)");
    std::regex patternF2(R"(\d+/\d+/\d+\s*)");
    std::smatch matches;
    while(getline(file,line)){
        if(std::regex_match(line,matches,patternPos)){
            float v1,v2,v3;
            sscanf(line.c_str(),"v %f %f %f",&v1,&v2,&v3);
            _pos.emplace_back(v1,v2,v3);
        }else if(std::regex_match(line,matches,patternN)){
            float v1,v2,v3;
            sscanf(line.c_str(),"vn %f %f %f",&v1,&v2,&v3);
            _normal.emplace_back(v1,v2,v3);
        }else if(std::regex_match(line,matches,patternUV)){
            float v1,v2;
            sscanf(line.c_str(),"vt %f %f",&v1,&v2);
            _uv.emplace_back(v1,v2);
        }else if(std::regex_match(line,matches,patternF1)){
            if(verticesPos.empty()){
                for(const auto& v:_pos){
                    auto pos = mkU<VertexPos>();
                    pos->pos = v;
                    verticesPos.push_back(std::move(pos));
                }
            }
            char current[100]{};
            char remain[100]{};
            sscanf(line.c_str(),"f %[^\n]",current);
            int v1,v2,v3;
            while(sscanf(current,"%d/%d/%d %[^\n]",&v1,&v2,&v3,remain)==4){
                _faces.emplace_back(v1,v2,v3);
                strcpy(current,remain);
            }
            if (sscanf(current, "%d/%d/%d", &v1, &v2, &v3) == 3) {
                _faces.emplace_back(v1, v2, v3);
            }
            //face
            uPtr<Face> face = mkU<Face>();
            face->color = glm::vec3((rand()%256)/256.0,(rand()%256)/256.0,(rand()%256)/256.0);
            //vertices
            int verticesOffset = vertices.size();
            for(const auto& v:_faces){
                auto vertex = mkU<Vertex>();
                vertex->vertexPos = verticesPos.at(v[0]-1).get();
                vertex->normal = _normal.at(v[2]-1);
                vertex->uv = _uv.at(v[1]-1);
                vertices.push_back(std::move(vertex));
            }
            //halfedges
            int halfedgesOffset = halfEdges.size();
            for(size_t i=0;i<_faces.size();i++){
                halfEdges.push_back(mkU<HalfEdge>());
            }
            //check order
            auto p0 = vertices.at(verticesOffset).get();
            auto p1 = vertices.at(verticesOffset+1).get();
            auto p2 = vertices.at(verticesOffset+2).get();
            // if(glm::dot(glm::cross(p1->pos()-p0->pos(),p2->pos()-p1->pos()),p1->normal)>0){
                for(size_t i=0;i<_faces.size();i++){
                    auto thisEdge = halfEdges.at(halfedgesOffset+i).get();
                    thisEdge->face = face.get();
                    face->edge = thisEdge;

                    thisEdge->next = halfEdges.at(halfedgesOffset+(i+1)%_faces.size()).get();

                    thisEdge->vertex = vertices.at(verticesOffset+(i+1)%_faces.size()).get();
                    vertices.at(verticesOffset+(i+1)%_faces.size())->edge = thisEdge;
                    auto thisVertex = vertices.at(verticesOffset+i).get();
                    // assert(EdgeHash[std::make_pair(thisVertex->vertexPos,thisEdge->vertex->vertexPos)]==nullptr);
                    EdgeHash[std::make_pair(thisVertex->vertexPos,thisEdge->vertex->vertexPos)]=thisEdge;
                    HalfEdge* symm = EdgeHash[std::make_pair(thisEdge->vertex->vertexPos,thisVertex->vertexPos)];
                    if(symm){
                        symm->symm = thisEdge;
                        thisEdge->symm=symm;
                        //assert(thisVertex->vertexPos == symm->vertex->vertexPos);
                    }
                }
            // }else{
            //     for(size_t i=0;i<_faces.size();i++){
            //         auto thisEdge = halfEdges.at(halfedgesOffset+i).get();
            //         thisEdge->face = face.get();
            //         face->edge = thisEdge;

            //         thisEdge->next = halfEdges.at(halfedgesOffset+(i-1)%_faces.size()).get();
            //         thisEdge->vertex = vertices.at(verticesOffset+i).get();
            //         vertices.at(verticesOffset+i)->edge = thisEdge;
            //         auto thisVertex = vertices.at(verticesOffset+(i+1)%_faces.size()).get();
            //         // assert(EdgeHash[std::make_pair(thisVertex->vertexPos,thisEdge->vertex->vertexPos)]==nullptr);
            //         EdgeHash[std::make_pair(thisVertex->vertexPos,thisEdge->vertex->vertexPos)]=thisEdge;
            //         HalfEdge* symm = EdgeHash[std::make_pair(thisEdge->vertex->vertexPos,thisVertex->vertexPos)];
            //         if(symm){
            //             symm->symm = thisEdge;
            //             thisEdge->symm=symm;
            //             //assert(thisVertex->vertexPos == symm->vertex->vertexPos);
            //         }
            //     }
            // }

            faces.push_back(std::move(face));
            _faces.clear();
        }
    }
    EdgeHash.clear();
    Update();
}

void Mesh::Subdivision(){
    // Generate new centroids
    std::vector<uPtr<Vertex>> centroids(faces.size());
    std::generate_n(centroids.begin(), faces.size(), []() {
        return std::make_unique<Vertex>();
    });

    //For each Face, compute its centroid
    for(size_t i=0;i<faces.size();i++){
        Face* face = faces.at(i).get();
        HalfEdge* start = face->edge;
        HalfEdge* current = start;
        size_t faceNum=0;
        glm::vec3 sumPos(0,0,0);
        glm::vec2 sumUV(0,0);
        glm::vec3 sumNormal(0,0,0);
        do{
            sumPos+=current->vertex->pos();
            sumUV+=current->vertex->uv;
            sumNormal+=current->vertex->normal;
            faceNum++;
            current = current->next;
        }while(current!=start);
        sumPos/=faceNum;
        sumUV/=faceNum;
        sumNormal/=faceNum;
        verticesPos.push_back(mkU<VertexPos>(sumPos));
        centroids.at(face->id)->vertexPos = verticesPos.back().get();
        centroids.at(face->id)->uv = sumUV;
        centroids.at(face->id)->normal = sumNormal;
        centroids.at(face->id)->edge = face->edge;//used to get face;
    }

    // Generate new mid points
    std::vector<uPtr<Vertex>> midPoints(halfEdges.size());
    std::generate_n(midPoints.begin(), halfEdges.size(), []() {
        return std::make_unique<Vertex>();
    });

    //For each Face, compute its midPoints
    for(size_t i=0;i<faces.size();i++){
        Face* face = faces.at(i).get();
        Vertex* center = centroids.at(face->id).get();
        HalfEdge* start = face->edge;
        HalfEdge* current = start;
        do{
            Vertex* p0 = current->vertex;
            Vertex* p1 = current->next->vertex;
            size_t midPointIdx=current->next->id;
            Vertex* thisMidpoint = midPoints.at(midPointIdx).get();
            HalfEdge* symm = current->next->symm;

            if(!symm){
                glm::vec3 position = p0->pos()+p1->pos()+center->pos();
                glm::vec2 uv = p0->uv+p1->uv+center->uv;
                glm::vec3 normal = p0->normal+p1->normal+center->normal;
                position/=3;
                uv/=3;
                normal/=3;
                verticesPos.push_back(mkU<VertexPos>(position));
                thisMidpoint->uv=uv;
                thisMidpoint->vertexPos=verticesPos.back().get();
                thisMidpoint->normal=normal;
                thisMidpoint->edge=current->next;//set now, not right
            }else{
                if(thisMidpoint->vertexPos == nullptr){
                    Vertex* anotherCenter = centroids.at(symm->face->id).get();
                    glm::vec3 position = p0->pos()+p1->pos()+anotherCenter->pos()+center->pos();
                    glm::vec2 uv = p0->uv+p1->uv+center->uv+anotherCenter->uv;
                    glm::vec3 normal = p0->normal+p1->normal+center->normal+anotherCenter->normal;
                    position/=4;
                    uv/=4;
                    normal/=4;
                    verticesPos.push_back(mkU<VertexPos>(position));
                    thisMidpoint->vertexPos=verticesPos.back().get();
                    thisMidpoint->uv=uv;
                    thisMidpoint->normal=normal;
                    thisMidpoint->edge=current->next;//set now, not right
                    Vertex* symMidpoint=midPoints.at(symm->id).get();
                    symMidpoint->vertexPos=verticesPos.back().get();
                    // symMidpoint->edge=symm;//set now, not right
                    // symMidpoint->uv=uv;
                    // symMidpoint->normal=normal;
                }else{
                    Vertex* anotherCenter = centroids.at(symm->face->id).get();
                    glm::vec3 position = p0->pos()+p1->pos()+anotherCenter->pos()+center->pos();
                    glm::vec2 uv = p0->uv+p1->uv+center->uv+anotherCenter->uv;
                    glm::vec3 normal = p0->normal+p1->normal+center->normal+anotherCenter->normal;
                    position/=4;
                    uv/=4;
                    normal/=4;
                    thisMidpoint->uv=uv;
                    thisMidpoint->normal=normal;
                    thisMidpoint->edge=current->next;//set now, not right
                }
            }
            current=current->next;
        }while(current!=start);
    }

    // Smooth the original vertices
    std::unordered_map<void*,glm::vec3> sumMidPos;
    std::unordered_map<void*,glm::vec3> sumCentroidPos;
    std::unordered_map<void*,float> ns;
    for(size_t i=0;i<faces.size();i++){
        Face* face = faces.at(i).get();
        HalfEdge* start = face->edge;
        HalfEdge* current = start;
        Vertex* center = centroids.at(face->id).get();
        do{
            Vertex* p0 = current->vertex;
            Vertex* p1 = current->next->vertex;
            Vertex* midPoint = midPoints.at(current->next->id).get();
            sumCentroidPos[p0->vertexPos]+=center->pos();
            if(current->symm){
                ns[p0->vertexPos]+=0.5;
                ns[p1->vertexPos]+=0.5;
                glm::vec3 midPos=midPoint->pos();
                midPos/=2;
                sumMidPos[p0->vertexPos]+=midPos;
                sumMidPos[p1->vertexPos]+=midPos;
            }else{
                ns[p0->vertexPos]+=1;
                ns[p1->vertexPos]+=1;
                glm::vec3 midPos=midPoint->pos();
                sumMidPos[p0->vertexPos]+=midPos;
                sumMidPos[p1->vertexPos]+=midPos;
            }
            current = current->next;
        }while(current!=start);
    }
    //Update original vertices
    for(auto& pos:verticesPos){
        float n = ns[pos.get()];
        if(n>0){
            glm::vec3 oringalPos = pos->pos;
            pos->pos=((n-2)*n*oringalPos
                    +sumMidPos[pos.get()]
                    +sumCentroidPos[pos.get()])/(n*n);
        }else{
            break;
        }
    }
    sumMidPos.clear();
    sumCentroidPos.clear();
    ns.clear();

    //Splite faces
    //Generate new halfEdges
    std::vector<uPtr<HalfEdge>> midPointHalfEdges(halfEdges.size());
    std::generate_n(midPointHalfEdges.begin(), halfEdges.size(), []() {
        return std::make_unique<HalfEdge>();
    });

    //Link vertices and midpoints
    for(auto& splitedEgde:halfEdges){
        Vertex* midPoint = midPoints.at(splitedEgde->id).get();
        HalfEdge* addedEdge = midPointHalfEdges.at(splitedEgde->id).get();
        addedEdge->vertex = splitedEgde->vertex;
        addedEdge->vertex->edge=addedEdge;
        addedEdge->next = splitedEgde->next;
        addedEdge->face = splitedEgde->face;
        splitedEgde->next = addedEdge;
        splitedEgde->vertex = midPoint;
        midPoint->edge = splitedEgde.get();
    }
    for(auto& splitedEgde:halfEdges){
        if(splitedEgde->next->symm==nullptr){
            HalfEdge* splitedEgdeSym = splitedEgde->symm;
            if(splitedEgdeSym){
                splitedEgdeSym->next->symm = splitedEgde.get();
                splitedEgde->symm = splitedEgdeSym->next;
                splitedEgdeSym->symm = splitedEgde->next;
                splitedEgde->next->symm = splitedEgdeSym;
            }/*else{
                assert(false);
            }*/
        }
    }

    //Generate new faces, centroidHalfEdges, dupilcatedCentroids
    std::vector<uPtr<Face>> newFaces(midPoints.size());
    std::generate_n(newFaces.begin(), midPoints.size(), []() {
        return std::make_unique<Face>();
    });

    for(auto&edge:midPointHalfEdges){
        halfEdges.push_back(std::move(edge));
    }
    midPointHalfEdges.clear();

    //Link centroids
    // Generate centroidHalfEdges
    std::unordered_map<std::pair<void*,void*>,HalfEdge*,pair_hash> EdgeHash;
    auto addedEdgeStart = halfEdges.end();
    size_t newFaceId=0;
    for(size_t i=0;i<faces.size();i++){
        Face* face = faces.at(i).get();
        HalfEdge* start = face->edge;
        HalfEdge* current = start;
        Vertex* center = centroids.at(face->id).get();
        HalfEdge* e1 = current->next;
        do{
            HalfEdge* e2 = e1->next;
            uPtr<HalfEdge>e0=mkU<HalfEdge>();
            uPtr<HalfEdge>e3=mkU<HalfEdge>();
            e0->face=newFaces.at(newFaceId).get();
            e3->face=newFaces.at(newFaceId).get();
            e1->face=newFaces.at(newFaceId).get();
            e2->face=newFaces.at(newFaceId).get();

            e0->next=e1;
            e3->next=e0.get();
            e0->vertex=current->vertex;
            current->vertex->edge=e0.get();
            e3->vertex=center;
            center->edge=e3.get();
            newFaces.at(newFaceId)->edge=e0.get();

            current=e2;
            e1 = current->next;
            e2->next=e3.get();

            auto e0symm = EdgeHash[std::make_pair(&e0->vertex->pos(),&center->pos())];
            if(e0symm){
                e0->symm = e0symm;
                e0symm->symm = e0.get();
            }else{
                EdgeHash[std::make_pair(&center->pos(),&e0->vertex->pos())]=e0.get();
            }

            auto e3symm = EdgeHash[std::make_pair(&e3->vertex->pos(),&e2->vertex->pos())];
            if(e3symm){
                e3->symm = e3symm;
                e3symm->symm = e3.get();
            }else{
                EdgeHash[std::make_pair(&e2->vertex->pos(),&e3->vertex->pos())]=e3.get();
            }
            halfEdges.push_back(std::move(e0));
            halfEdges.push_back(std::move(e3));
            newFaceId++;
        }while(current!=start);
    }
    EdgeHash.clear();

    //push new elments
    for(auto&vertex:centroids){
        vertices.push_back(std::move(vertex));
    }
    centroids.clear();
    for(auto&vertex:midPoints){
        vertices.push_back(std::move(vertex));
    }
    midPoints.clear();
    faces.clear();
    for(auto&face:newFaces){
        face->color = glm::vec3((rand()%256)/256.0,(rand()%256)/256.0,(rand()%256)/256.0);
        faces.push_back(std::move(face));
    }
    newFaces.clear();
    ReIndex();
    Update();
}

void Mesh::ReIndex(){
    for(size_t i=0;i<vertices.size();i++){
        vertices.at(i)->id=i;
        vertices.at(i)->ReName();
    }
    for(size_t i=0;i<faces.size();i++){
        faces.at(i)->id=i;
        faces.at(i)->ReName();
    }
    for(size_t i=0;i<halfEdges.size();i++){
        halfEdges.at(i)->id=i;
        halfEdges.at(i)->ReName();
    }
}

void Mesh::PocessJsonRecursively(const QJsonObject& data, Joint* currentJoint){
    QString name = data.value("name").toString();
    QJsonArray posArray = data.value("pos").toArray();
    glm::vec3 pos = glm::vec3(posArray[0].toDouble(),posArray[1].toDouble(),posArray[2].toDouble());
    QJsonArray rotArray = data.value("rot").toArray();
    currentJoint->name = name;
    currentJoint->pos = pos;
    glm::vec3 axis = glm::vec3(rotArray[1].toDouble(),rotArray[2].toDouble(),rotArray[3].toDouble());
    axis = glm::normalize(axis);
    float radian = glm::radians(rotArray[0].toDouble());
    currentJoint->rotation = glm::quat(glm::cos(radian/2),glm::sin(radian/2)*axis);
    QJsonArray children = data.value("children").toArray();
    for(size_t i=0;i<children.size();i++){
        QJsonObject childData = children[i].toObject();
        uPtr<Joint>child = mkU<Joint>();
        child->parent = currentJoint;
        currentJoint->children.push_back(child.get());
        joints.push_back(std::move(child));
        PocessJsonRecursively(childData,joints.back().get());
    }
}

void Mesh::ReadSkeletonJson(std::string openFilePath){

    QFile file(QString::fromStdString(openFilePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open file!";
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(fileData);

    if (document.isNull() || document.isEmpty()) {
        qDebug() << "Invalid json file!";
        return;
    }

    if (document.isObject()) {
        joints.clear();
        Joint::lastJoint=0;
        QJsonObject jsonObject = document.object();
        if (jsonObject.contains("root") && jsonObject["root"].isObject()) {
            QJsonObject data = jsonObject.value("root").toObject();
            uPtr<Joint> root = mkU<Joint>();
            root->parent=nullptr;
            joints.push_back(std::move(root));
            PocessJsonRecursively(data,joints[0].get());
        }
        for(auto& joint:joints){
            joint->InitializeBindMatrix();
        }
    }
    Update();
}

void Mesh::CalculateJointInfluenceDistanceBased(){
    for(auto& vertexPos: verticesPos){
        glm::vec3 pos = vertexPos->pos;
        for(const auto& joint :joints){
            glm::vec3 jointPos = glm::vec3(joint->getOverallTransformation()*glm::vec4(joint->pos,1));
            float dist = 1.0/glm::max(1.0f,glm::distance(pos,jointPos));
            vertexPos->addJointFactor(joint.get(),dist);
        }
        assert(vertexPos->joints[1].joint);
        assert(vertexPos->joints[0].joint);
        float dist0 = 1.0/vertexPos->joints[0].factor;
        float dist1 = 1.0/vertexPos->joints[1].factor;
        vertexPos->joints[1].factor = dist1/(dist0+dist1);
        vertexPos->joints[0].factor = dist0/(dist0+dist1);
    }
}
struct FaceAndFactor{
    Face* face;
    Vertex* vertex;
    float factor;
    FaceAndFactor(Face* face,Vertex*v,float f):face(face),vertex(v),factor(f){};
};

void Mesh::CalculateJointInfluenceHeatDiffusionBased(){
    std::unordered_set<Face*>reviewedFace;
    for(const auto& j:joints){
        reviewedFace.clear();
        glm::vec3 jointPos = glm::vec3(j->getOverallTransformation()*glm::vec4(j->pos,1));
        Face* startFace = FindClosetVertex(jointPos)->edge->face;
        std::queue<FaceAndFactor> q;
        q.push(FaceAndFactor(startFace, startFace->edge->vertex,1.0f));
        reviewedFace.insert(startFace);
        while(!q.empty()){
            size_t qSize = q.size();
            for(size_t i=0;i<qSize;i++){
                auto t = q.front();
                t.vertex->vertexPos->addJointFactor(j.get(),t.factor);
                float startFactor = t.factor;
                if(true){
                    HalfEdge* startEdge = t.face->edge;
                    HalfEdge* currentEdge = startEdge;
                    do{
                        Face* faceNeighbor = currentEdge->symm ? currentEdge->symm->face:nullptr;
                        if(faceNeighbor&&reviewedFace.count(faceNeighbor)==0){
                            float factor = startFactor/glm::max(1.0f,glm::pow(glm::length2(t.vertex->pos()-faceNeighbor->edge->vertex->pos()),1.5f));
                            q.push(FaceAndFactor(faceNeighbor, faceNeighbor->edge->vertex,factor));
                            reviewedFace.insert(faceNeighbor);
                        }
                        currentEdge = currentEdge->next;
                    }while(currentEdge!=startEdge);
                }
                q.pop();
            }
        }

        for(auto&face:faces){
            if(reviewedFace.count(face.get())==0){
                HalfEdge* startEdge = face->edge;
                HalfEdge* currentEdge = startEdge;
                do{
                    float factor = 0.001/glm::max(1.0f,glm::pow(glm::length2(currentEdge->vertex->pos()-startFace->edge->vertex->pos()),1.5f));
                    currentEdge->vertex->vertexPos->addJointFactor(j.get(),factor);
                    currentEdge = currentEdge->next;
                }while(currentEdge!=startEdge);
            }
        }
    }



    for(auto& vertexPos: verticesPos){
        if(vertexPos->joints[0].joint && vertexPos->joints[1].joint){
            float factor0 = vertexPos->joints[0].factor;
            float factor1 = vertexPos->joints[1].factor;
            vertexPos->joints[1].factor =factor1/(factor0+factor1);
            vertexPos->joints[0].factor =factor0/(factor0+factor1);
        }else{
            glm::vec3 pos = vertexPos->pos;
            for(const auto& joint :joints){
                glm::vec3 jointPos = glm::vec3(joint->getOverallTransformation()*glm::vec4(joint->pos,1));
                float dist = 1.0/glm::max(1.0f,glm::distance(pos,jointPos));
                vertexPos->addJointFactor(joint.get(),dist);
            }
            assert(vertexPos->joints[1].joint);
            assert(vertexPos->joints[0].joint);
            float dist0 = 1.0/vertexPos->joints[0].factor;
            float dist1 = 1.0/vertexPos->joints[1].factor;
            vertexPos->joints[1].factor = dist1/(dist0+dist1);
            vertexPos->joints[0].factor = dist0/(dist0+dist1);
        }
    }

}
// void Mesh::HeatDiffuseFrom(Joint* joint,Vertex* startVertex, float startFactor){
//     static std::unordered_set<VertexPos*>reviewedVertex;
//     reviewedVertex.insert(startVertex->vertexPos);
//     HalfEdge* startEdge = startVertex->edge;
//     HalfEdge* currentEdge = startEdge;
//     do{
//         Vertex* vertex = currentEdge->next->vertex;
//         if(reviewedVertex.count(vertex->vertexPos)>0)
//             continue;
//         float factor = startFactor/(1.0f+glm::pow(glm::length2(vertex->pos()-startVertex->pos()),1.5f));
//         if(factor>0.05 && vertex->vertexPos->addJointFactor(joint,factor)){
//             HeatDiffuseFrom(joint, vertex, factor);
//         }
//         currentEdge = currentEdge->next->symm;
//     }while(currentEdge!=startEdge);
// }

void Mesh::Update(){
    if(verticesPos.size()>0 && joints.size() >0){
        CalculateJointInfluenceDistanceBased();
    }
}

Vertex* Mesh::FindClosetVertex(glm::vec3 pos){
    Vertex* closestVertex = nullptr;
    float minDistanceSquared = std::numeric_limits<float>::max();

    for (const auto& vertexPtr : vertices) {
        Vertex* vertex = vertexPtr.get();
        float distanceSquared = glm::length2(vertex->pos() - pos);

        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            closestVertex = vertex;
        }
    }
    return closestVertex;
}
