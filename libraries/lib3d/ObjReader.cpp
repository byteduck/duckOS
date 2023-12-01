/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ObjReader.h"
#include <libduck/StringStream.h>
#include <libduck/Log.h>
#include <array>

using namespace Lib3D;

enum VertexType {
	Pos, Normal, Texture
};

ObjReader::Obj ObjReader::read_obj(Duck::InputStream& stream) {
	stream.set_delimeter('\n');

	int pos_idx = 0, normal_idx = 0, texture_idx = 0;

	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::string line;
	std::string component;

	auto readVertex = [&](std::string& line) {
		VertexType type;
		switch (line[1]) {
			case ' ':
				type = Pos;
				break;
			case 'n':
				type = Normal;
				break;
			case 't':
				type = Texture;
				break;
			default:
				return;
		}
		int& idx = type == Normal ? normal_idx : (type == Texture ? texture_idx : pos_idx);

		if (vertices.size() <= idx)
			vertices.resize(idx + 1);
		Vertex& vert = vertices[idx];

		const char* str = line.c_str();
		while(!isspace(*str))
			str++;
		while(isspace(*str))
			str++;

		if (type == Pos || type == Normal) {
			Vec4f& vec = type == Pos ? vert.pos : vert.norm;
			(void) sscanf(str, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		} else {
			(void) sscanf(str, "%f %f %f", &vert.tex[0], &vert.tex[1], &vert.tex[2]);
		}

		idx++;
	};

	auto readFace = [&](std::string& line) {
		const char* str = line.c_str() + 1;
		while(isspace(*str))
			str++;

		std::array<std::array<int, 3>, 3> face = {};

		int type_idx = 0, vert_idx = 0;
		while(vert_idx < 3) {
			while(type_idx < 3) {
				face[type_idx][vert_idx] = strtol(str, (char**) &str, 10) - 1;
				if(*str != '/')
					break;
				type_idx++;
				str++;
			}
			while(*str && isspace(*str))
				str++;
			if(!*str)
				break;
			vert_idx++;
			type_idx = 0;
		}

		faces.push_back(Face {
			.pos = face[0],
			.tex = face[1],
			.norm = face[2]
		});
	};

	while(!stream.eof()) {
		stream >> line;
		switch(line[0]) {
			case 'v':
				readVertex(line);
				break;
			case 'f': {
				readFace(line);
				break;
			}
			default:
				break;
		}
	}

	return { vertices, faces };
}

std::vector<std::array<Vertex, 3>> ObjReader::read(Duck::InputStream& stream) {
	auto obj = Lib3D::ObjReader::read_obj(stream);
	std::vector<std::array<Lib3D::Vertex, 3>> faces;
	for(auto& face : obj.faces) {
		std::array<Lib3D::Vertex, 3> verts;
		for(int i = 0; i < 3; i++) {
			verts[i].pos = obj.verts[face.pos[i]].pos;
			verts[i].tex = obj.verts[face.tex[i]].tex;
			verts[i].norm = obj.verts[face.norm[i]].norm;
			verts[i].color = {1.0, 1.0, 1.0, 1.0};
		}
		faces.push_back(verts);
	}
	return std::move(faces);
}