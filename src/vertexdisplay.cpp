#include "vertexdisplay.h"

VertexDisplay::VertexDisplay(OpenGLContext* context)
    : Drawable(context), representedVertex(nullptr)
{}

VertexDisplay::~VertexDisplay()
{}

void VertexDisplay::create() {
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;

    if (representedVertex) {
        pos.push_back(glm::vec4(representedVertex->pos, 1));
        nor.push_back(glm::vec4(0, 0, 1, 1));
        col.push_back(glm::vec4(1, 1, 1, 1));
        idx.push_back(0);
    }

    count = idx.size();

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    bindCol();
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum VertexDisplay::drawMode() {
    return GL_POINTS;
}

void VertexDisplay::updateVertex(VertexPos * v) {
    representedVertex = v;
}
