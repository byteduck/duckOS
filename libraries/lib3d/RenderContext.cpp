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

void RenderContext::line(Vec3f a, Vec3f b, Vec4f color) {
	bool steep = false;
	if(abs(a.x() - b.x()) < abs(a.y() - b.y())) {
		a = {a.y(), a.x(), a.z()};
		b = {b.y(), b.x(), b.z()};
		steep = true;
	}
	if(a.x() > b.x())
		std::swap(a, b);
	Vec3f diff = b - a;
	float derr = abs(diff.y() / (float) diff.x());
	float err = 0;
	float y = a.y();
	float z = a.z();
	float zstep = diff.z() / diff.x();
	for(float x = a.x(); x <= b.x(); x++) {
		int xint = f2i(x);
		int yint = f2i(y);
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
			y += (a.y() < b.y() ? 1 : -1);
			err -= 1.0f;
		}
	}
}

void RenderContext::tri(std::array<Vertex, 3> verts) {
	tri_barycentric(verts);
}

void RenderContext::tri_simple(std::array<Vertex, 3> verts) {
	Vec3f tri[3];
	for(int i = 0; i < 3; i++) {
		tri[i] = screenspace((m_projmat * m_modelmat * verts[i].pos).transpose()[0]);
	}
	Vec4f color = verts[0].color;

	// Sort vertices by y
	if(tri[0].y() > tri[1].y())
		std::swap(tri[0], tri[1]);
	if(tri[0].y() > tri[2].y())
		std::swap(tri[0], tri[2]);
	if(tri[1].y() > tri[2].y())
		std::swap(tri[1], tri[2]);

	float height = tri[2].y() - tri[0].y();
	if(height == 0)
		return;

	// Top half
	float part_height = tri[1].y() - tri[0].y();
	if(part_height == 0)
		goto bottom;
	for(float y = tri[0].y(); y < tri[1].y(); y++) {
		float part_pct = (y - tri[0].y()) / part_height;
		float whole_pct = (y - tri[0].y()) / height;
		Vec3f a = { tri[0].x() + (tri[1].x() - tri[0].x()) * part_pct,  y, tri[0].z() + (tri[1].z() - tri[0].z()) * part_pct };
		Vec3f b = { tri[0].x() + (tri[2].x() - tri[0].x()) * whole_pct, y, tri[0].z() + (tri[2].z() - tri[0].z()) * whole_pct };
		line(a, b, color);
	}

	// Bottom half
	bottom:
	part_height = tri[2].y() - tri[1].y();
	if(part_height == 0)
		return;
	for(float y = tri[1].y(); y <= tri[2].y(); y++) {
		float part_pct = (y - tri[1].y()) / part_height;
		float whole_pct = (y - tri[0].y()) / height;
		Vec3f a = { tri[1].x() + (tri[2].x() - tri[1].x()) * part_pct,  y, tri[1].z() + (tri[2].z() - tri[1].z()) * part_pct };
		Vec3f b = { tri[0].x() + (tri[2].x() - tri[0].x()) * whole_pct, y, tri[0].z() + (tri[2].z() - tri[0].z()) * whole_pct };
		line(a, b, color);
	}
}

void RenderContext::tri_barycentric(std::array<Vertex, 3> verts) {
	// First, transform into screenspace coordinates
	std::array<Vec3f, 3> sstri;
	for(int i = 0; i < 3; i++) {
		sstri[i] = screenspace((m_projmat * m_modelmat * verts[i].pos).transpose()[0]);
	}

	Vec2i bbox_max = {0, 0};
	Vec2i bbox_min = {m_viewport.width - 1, m_viewport.height - 1};
	for(auto vert : sstri) {
		bbox_min.x() = std::min(bbox_min.x(), f2i(vert.x()));
		bbox_min.y() = std::min(bbox_min.y(), f2i(vert.y()));
		bbox_max.x() = std::max(bbox_max.x(), f2i(vert.x()));
		bbox_max.y() = std::max(bbox_max.y(), f2i(vert.y()));
	}
	bbox_min.x() = std::max(bbox_min.x(), 0);
	bbox_min.y() = std::max(bbox_min.y(), 0);
	bbox_max.x() = std::min(bbox_max.x(), m_viewport.width - 1);
	bbox_max.y() = std::min(bbox_max.y(), m_viewport.height - 1);

	const bool pixel = bbox_max.x() == bbox_min.x() && bbox_max.y() == bbox_min.y();

	const Vec3f a {sstri[2].x() - sstri[0].x(), sstri[1].x() - sstri[0].x(), sstri[0].x()};
	const Vec3f b {sstri[2].y() - sstri[0].y(), sstri[1].y() - sstri[0].y(), sstri[0].y()};
	const float abz = a.x() * b.y() - a.y() * b.x(); // z component of a^b

	// If the triangle isn't facing the screen, don't bother
	if(!pixel && std::abs(abz) < 1)
		return;

	for(int y = bbox_min.y(); y <= bbox_max.y(); y++) {
		const float bz = b.z() - y;
		const float ax_x_bz = a.x() * bz;
		const float ay_x_bz = a.y() * bz;
		for(int x = bbox_min.x(); x <= bbox_max.x(); x++) {
			// a^b, but without unnecessarily recalculating the z component
			const float az = a.z() - x;
			const Vec3f u = {
					ay_x_bz - az * b.y(),
					az * b.x() - ax_x_bz,
					abz
			};

			// Calculate barycentric coordinates in triangle
			const Vec3f bary = {1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z()};
			if (!pixel && (bary.x() < 0 || bary.y() < 0 || bary.z() < 0))
				continue;

			float z = 0;
			Vec4f color;
			Vec2f tex;
			for (int i = 0; i < 3; i++) {
				auto baryi = bary[i];
				z += sstri[i].z() * baryi;
				color += {
					verts[i].color[0] * baryi,
					verts[i].color[1] * baryi,
					verts[i].color[2] * baryi,
					verts[i].color[3] * baryi
				};
				tex += {
					verts[i].tex[0] * baryi,
					verts[i].tex[1] * baryi
				};
			}

			if (m_bound_texture) {
				Vec4f texcol = m_bound_texture->buffer().get(tex.x() * m_bound_texture->buffer().width(), tex.y() * m_bound_texture->buffer().height());
				color = {
						color[0] * texcol[0],
						color[1] * texcol[1],
						color[2] * texcol[2],
						color[3] * texcol[3]
				};
			}

			if (m_buffers.depth.at(x, y) < z) {
				m_buffers.color.at(x, y) = color;
				m_buffers.depth.at(x, y) = z;
			}
		}
	}
}
