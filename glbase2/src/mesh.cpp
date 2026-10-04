#include "CG/mesh.h"


CG::Mesh::Mesh(Mesh&& other) noexcept
    : vao(other.vao), vbos(other.vbos), index_count(other.index_count)
{
	other.vao = 0;
    other.vbos.fill(0);
}

CG::Mesh& CG::Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other) {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
        }
        for (int i = 0; i < 5; i++) {
            if (vbos[i] != 0) {
                glDeleteBuffers(1, &vbos[i]);
            }
        }
		vao = other.vao;
        vbos = other.vbos;
        index_count = other.index_count;
		other.vao = 0;
        other.vbos.fill(0);
	}
	return *this;
}

CG::Mesh::~Mesh()
{
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    for (int i = 0; i < 5; i++) {
        if (vbos[i] != 0) {
            glDeleteBuffers(1, &vbos[i]);
            vbos[i] = 0;
        }
    }
}



std::shared_ptr<CG::Mesh> CG::MeshBuilder::build() const
{
	auto mesh = std::make_shared<Mesh>();
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    if (!positions.empty()) {
        glGenBuffers(1, &mesh->vbos[BufferIndex_Positions]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[BufferIndex_Positions]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(BufferIndex_Positions, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(BufferIndex_Positions);
    }
    if (!normals.empty()) {
        glGenBuffers(1, &mesh->vbos[BufferIndex_Normals]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[BufferIndex_Normals]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(BufferIndex_Normals, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(BufferIndex_Normals);
    }
    if (!texcoords.empty()) {
        glGenBuffers(1, &mesh->vbos[BufferIndex_Texcoords]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[BufferIndex_Texcoords]);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(BufferIndex_Texcoords, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(BufferIndex_Texcoords);
    }
    if (!colors.empty()) {
        glGenBuffers(1, &mesh->vbos[BufferIndex_Colors]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[BufferIndex_Colors]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::ubvec3), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(BufferIndex_Colors, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
        glEnableVertexAttribArray(BufferIndex_Colors);
    }
    if (!indices.empty()) {
        glGenBuffers(1, &mesh->vbos[BufferIndex_Indices]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbos[BufferIndex_Indices]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        mesh->index_count = static_cast<GLsizei>(indices.size());
    }
    else {
        mesh->index_count = static_cast<GLsizei>(positions.size());
    }
    glBindVertexArray(0);

	return mesh;
}