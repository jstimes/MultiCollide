#pragma once
#include "tiny_obj_loader.h"

#include <iostream>
#include <sstream>

class ObjMesh : public Shape {

public:

	std::vector<GLfloat> vertices;

	virtual void InitVAOandVBO(Shader &shader) override {

		shader.Use();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(shader.getAttribute("position"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	virtual void Draw(Shader &shader) override {

		shader.Use();
		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

	virtual void ComputeInertia() {
		//TODO
		glm::mat3 i3;
		this->setAngularInertia(i3);
	}

	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) override {
		return glm::vec3(1.0f, 1.0f, 1.0f);
	}

	virtual std::string getShapeCSVline1() override {
		return "TODO";
	}

	virtual ~ObjMesh() {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	ObjMesh(const char *file_contents) {
		std::string file_str(file_contents);
		std::istringstream *stream = new std::istringstream(file_str);

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;

		//This function opens a file:
		//bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), (const char*)0, true);

		//This one takes in a istream containing file's contents:
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, stream, nullptr, true);

		if (!err.empty()) { // `err` may contain warning message.
			std::cout << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		float max_dist = 0.0f;

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

					glm::vec3 pt(vx, vy, vz);
					float dist = glm::length(pt);
					if (dist > max_dist) {
						max_dist = dist;
					}
					glm::vec3 normal(1.0f);

					if (idx.normal_index > 0) {
						tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
						tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
						tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
						normal.x = nx;
						normal.y = ny;
						normal.z = nz;
					}

					ShapeUtils::AddPoint(vertices, pt, normal);

					if (idx.texcoord_index > 0) {
						tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					}
				}
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}

		this->scaling = 1.0f / max_dist;
		this->boundingSphereRadius = 1.0f + this->BoundingSphereBuffer;

	}

};