#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>



struct Vertex {
	glm::vec3 Position;
	glm::vec2 Texture;
	glm::vec3 Normal;
};


class Mesh
{
public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Vertex> vertices;

	int numVertices;

	GLuint VBO, VAO;

	// glm::mat4 model = glm::mat4(1.0);


	Mesh(const char* path) {

		std::ifstream infile(path);
		//TODO Error management
		std::string line;
		while (std::getline(infile, line))
		{
			std::istringstream iss(line);
			std::string indice;
			iss >> indice;
			//std::cout << "indice : " << indice << std::endl;
			if (indice == "v") {
				float x, y, z;
				iss >> x >> y >> z;
				positions.push_back(glm::vec3(x, y, z));

			}
			else if (indice == "vn") {
				float x, y, z;
				iss >> x >> y >> z;
				normals.push_back(glm::vec3(x, y, z));
			}
			else if (indice == "vt") {
				float u, v;
				iss >> u >> v;
				textures.push_back(glm::vec2(u, v));
			}
			else if (indice == "f") {
				std::string f1, f2, f3;
				iss >> f1 >> f2 >> f3;

				std::string p, t, n;

				//for face 1
				Vertex v1;

				p = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);

				t = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);

				n = f1.substr(0, f1.find("/"));


				v1.Position = positions.at(std::stof(p) - 1);
				v1.Normal = normals.at(std::stof(n) - 1);
				v1.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v1);

				//for face 2
				Vertex v2;

				p = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);

				t = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);

				n = f2.substr(0, f2.find("/"));


				v2.Position = positions.at(std::stof(p) - 1);
				v2.Normal = normals.at(std::stof(n) - 1);
				v2.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v2);

				//for face 3
				Vertex v3;

				p = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);

				t = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);

				n = f3.substr(0, f3.find("/"));


				v3.Position = positions.at(std::stof(p) - 1);
				v3.Normal = normals.at(std::stof(n) - 1);
				v3.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v3);
			}
		}
		//std::cout << positions.size() << std::endl;
		//std::cout << normals.size() << std::endl;
		//std::cout << textures.size() << std::endl;
		std::cout << "Loaded mesh with " << vertices.size() << " vertices" << std::endl;

		infile.close();

		numVertices = vertices.size();

        makeMesh();
	}

    void draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
        // glBindVertexArray(0);
	}

private:
	void makeMesh() {
		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW);

		// TODO : find a proper way to set the attribute locations
		// auto att_pos = glGetAttribLocation(shader.ID, "position");
		auto att_pos = 0;
		glEnableVertexAttribArray(att_pos);
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);
		// glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, sizeof(Vertex), (void*)0);

        // auto att_tex = glGetAttribLocation(shader.ID, "tex_coords");
        auto att_tex = 1;
        glEnableVertexAttribArray(att_tex);
        glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, Texture));
		
		// auto att_norm = glGetAttribLocation(shader.ID, "normal");
		auto att_norm = 2;
		glEnableVertexAttribArray(att_norm);
		glVertexAttribPointer(att_norm, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		// glVertexAttribPointer(att_norm, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		
		//desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data;

		// std::cout << "Made model with " << numVertices << " vertices" << std::endl;
	}
};

#endif /* MESH_H */