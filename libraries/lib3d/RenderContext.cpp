/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "RenderContext.h"

using namespace Lib3D;

RenderContext::RenderContext(Gfx::Dimensions dimensions):
	m_viewport({0, 0, dimensions}),
	m_buffers(dimensions)
{}

void RenderContext::set_viewport(Gfx::Rect rect) {
	m_viewport = rect;
	m_buffers = BufferSet(rect.dimensions());
}

void RenderContext::clear(Vec4f color) {
	m_buffers.depth.fill(-INFINITY);
	m_buffers.color.fill(color);
}

#define f2i(f) ((int) ((f)))

void RenderContext::line(Vertex a, Vertex b) {
	bool steep = false;
	if(abs(a.pos.x() - b.pos.x()) < abs(a.pos.y() - b.pos.y())) {
		a.pos = {a.pos.y(), a.pos.x(), a.pos.z()};
		b.pos = {b.pos.y(), b.pos.x(), b.pos.z()};
		steep = true;
	}
	if(a.pos.x() > b.pos.x())
		std::swap(a, b);
	auto diff = b.pos - a.pos;
	const float derr = abs(diff.y() / (float) diff.x());
	float err = 0;
	float y = a.pos.y();
	float z = a.pos.z();
	const float zstep = diff.z() / diff.x();
	for(float x = a.pos.x(); x <= b.pos.x(); x++) {
		const int xint = f2i(x);
		const int yint = f2i(y);
		const float lerp = (x - a.pos.x()) / diff.x();
		const Vec4f color = a.color * (1.0f - lerp) + b.color * lerp;
		if(steep) {
			if (m_buffers.depth.get(yint, xint) < -z) {
				m_buffers.color.set(yint, xint, color);
				m_buffers.depth.set(yint, xint, -z);
			}
		} else {
			if (m_buffers.depth.get(xint, yint) < -z) {
				m_buffers.color.set(xint, yint, color);
				m_buffers.depth.set(xint, yint, -z);
			}
		}
		err += derr;
		z += zstep;
		if (err > 0.5f) {
			y += (a.pos.y() < b.pos.y() ? 1.f : -1.f);
			err -= 1.0f;
		}
	}
}

void RenderContext::tri(std::array<Vertex, 3> verts) {
	tri_barycentric(verts);
}

void RenderContext::tri_barycentric(std::array<Vertex, 3> verts) {
	// Transform into world coords
	std::array<Vec3f, 3> tri;
	for(int i = 0; i < 3; i++) {
		auto pt = project(verts[i].pos);
		tri[i] = {pt.x(), pt.y(), pt.z()};
	}

	// Backface cull and lighting calculation
	Vec3f norm = ((tri[2]-tri[0])^(tri[1]-tri[0])).normalize();
	if (m_backface_culling && norm.z() < 0)
		return;
	const float light = m_backface_culling ?
			norm * Vec3f(0, 0, 1) :
			Vec3f(std::abs(norm.x()), std::abs(norm.y()), std::abs(norm.z())) * Vec3f(0, 0, 1);

	// Then, transform into screenspace coordinates and calculate bounding box
	std::array<Vec3f, 3> sstri;
	Vec2i bbox_max = {0, 0};
	Vec2i bbox_min = {m_viewport.width - 1, m_viewport.height - 1};
	for(int i = 0; i < 3; i++) {
		sstri[i] = screenspace(tri[i]);
		bbox_min.x() = std::min(bbox_min.x(), f2i(sstri[i].x() + 0.5f));
		bbox_min.y() = std::min(bbox_min.y(), f2i(sstri[i].y() + 0.5f));
		bbox_max.x() = std::max(bbox_max.x(), f2i(sstri[i].x() + 0.5f));
		bbox_max.y() = std::max(bbox_max.y(), f2i(sstri[i].y() + 0.5f));
	}
	bbox_min.x() = std::max(bbox_min.x(), 0);
	bbox_min.y() = std::max(bbox_min.y(), 0);
	bbox_max.x() = std::min(bbox_max.x(), m_viewport.width - 1);
	bbox_max.y() = std::min(bbox_max.y(), m_viewport.height - 1);

	// Whether the tri takes up just one pixel
	const bool pixel = bbox_max.x() == bbox_min.x() && bbox_max.y() == bbox_min.y();

	// Calculate the barycentric coordinates of the top-left of our bounding box
	const Vec3f a {sstri[2].x() - sstri[0].x(), sstri[1].x() - sstri[0].x(), sstri[0].x() - (float) bbox_min.x()};
	const Vec3f b {sstri[2].y() - sstri[0].y(), sstri[1].y() - sstri[0].y(), sstri[0].y() - (float) bbox_min.y()};
	const Vec3f u = a^b;
	Vec3f bary = {
		1.0f - (u.x() + u.y()) / u.z(),
		u.y() / u.z(),
		u.x() / u.z()
	};

	// Calculate x and y steps for barycentric coordinates
	const Vec3f barystep_x = {
		-(b.y() + -b.x()) / u.z(),
		-b.x() / u.z(),
		b.y() / u.z()
	};
	const Vec3f barystep_y = {
			-(a.x() - a.y()) / u.z(),
			a.x() / u.z(),
			-a.y() / u.z()
	};

	// Calculate x and y step for tex coords, color, and depth
	Vec2f tex;
	Vec2f texstep_x;
	Vec2f texstep_y;

	Vec4f color;
	Vec4f colorstep_x;
	Vec4f colorstep_y;

	float z = 0;
	float zstep_x = 0;
	float zstep_y = 0;

	for (int i = 0; i < 3; i++) {
		tex += verts[i].tex * bary[i];
		texstep_x += verts[i].tex * barystep_x[i];
		texstep_y += verts[i].tex * barystep_y[i];

		color += {
				verts[i].color[0] * bary[i] * light,
				verts[i].color[1] * bary[i] * light,
				verts[i].color[2] * bary[i] * light,
				verts[i].color[3] * bary[i]
		};
		colorstep_x += verts[i].color * light * barystep_x[i];
		colorstep_y += verts[i].color * light * barystep_y[i];

		z += sstri[i].z() * bary[i];
		zstep_x += sstri[i].z() * barystep_x[i];
		zstep_y += sstri[i].z() * barystep_y[i];
	}

	for(int y = bbox_min.y(); y <= bbox_max.y(); y++) {
		// Save our barycentric coords, texcoords, and color at the start of the line
		const auto obary = bary;
		const auto otex = tex;
		const auto ocolor = color;
		const auto oz = z;
		bool was_inside = false;

		for(int x = bbox_min.x(); x <= bbox_max.x(); x++) {
			if (!pixel && (bary.x() < 0 || bary.y() < 0 || bary.z() < 0)) {
				// If we were previously inside the triangle on this line, we're done and can skip to the next line
				if (was_inside)
					break;
				goto done;
			}
			was_inside = true;

			if (m_depth_testing && m_buffers.depth.at(x, y) >= z)
				goto done;

			if (m_bound_texture) {
				const Vec4f texcol = m_bound_texture->buffer().get(tex.x() * m_bound_texture->buffer().width(), tex.y() * m_bound_texture->buffer().height());
				const Vec4f sampled_color = {
						color[0] * texcol[0],
						color[1] * texcol[1],
						color[2] * texcol[2],
						color[3] * texcol[3]
				};
				if (m_alpha_testing && sampled_color.w() <= 0)
					goto done;
				m_buffers.color.at(x, y) = sampled_color;
			} else {
				if (m_alpha_testing && color.w() <= 0)
					goto done;
				m_buffers.color.at(x, y) = color;
			}

			m_buffers.depth.at(x, y) = z;

			done:
			tex += texstep_x;
			bary += barystep_x;
			color += colorstep_x;
			z += zstep_x;
		}

		// Step bary, tex, color by y step
		bary = obary + barystep_y;
		tex = otex + texstep_y;
		color = ocolor + colorstep_y;
		z = oz + zstep_y;
	}
}

void RenderContext::tri_wireframe(std::array<Vertex, 3> verts) {
	// Transform into world coords
	for(int i = 0; i < 3; i++) {
		auto pos = screenspace(project(verts[i].pos));
		verts[i].pos = {pos.x(), pos.y(), pos.z(), verts[i].pos.w()};
	}

	line(verts[0], verts[1]);
	line(verts[1], verts[2]);
	line(verts[2], verts[0]);
}
