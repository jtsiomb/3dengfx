/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include <string>
#include "n3dmath2/n3dmath2.hpp"
#include "gfx/animation.hpp"
#include "gfx/color.hpp"

#define LIGHTCOL_AMBIENT	1
#define LIGHTCOL_DIFFUSE	2
#define LIGHTCOL_SPECULAR	4

// abstract base class Light
class Light : public XFormNode {
protected:
	Color ambient_color, diffuse_color, specular_color;
	scalar_t intensity;
	scalar_t attenuation[3];
	bool cast_shadows;

public:
	Light();
	virtual ~Light();
	
	virtual void set_color(const Color &c, unsigned short color_flags = 0);
	virtual void set_color(const Color &amb, const Color &diff, const Color &spec);
	virtual Color get_color(unsigned short which) const;
	
	virtual void set_intensity(scalar_t intensity);
	virtual scalar_t get_intensity() const;
	
	virtual void set_attenuation(scalar_t att0, scalar_t att1, scalar_t att2);
	virtual scalar_t get_attenuation(int which) const;
	virtual Vector3 get_attenuation_vector() const;

	virtual void set_shadow_casting(bool shd);
	virtual bool casts_shadows() const;
	
	virtual void set_gl_light(int n, unsigned long time = XFORM_LOCAL_PRS) const = 0;
};


class PointLight : public Light {
public:
	PointLight(const Vector3 &pos=Vector3(0,0,0), const Color &col=Color(1.0f, 1.0f, 1.0f));
	virtual ~PointLight();

	virtual void set_gl_light(int n, unsigned long time = XFORM_LOCAL_PRS) const;
};

class DirLight : public Light {
private:
	Vector3 dir;	// actually get rid of this and work with the PRS directly?

public:
	DirLight(const Vector3 &dir=Vector3(0, 0, 1), const Color &col=Color(1.0f, 1.0f, 1.0f));
	virtual ~DirLight();

	Vector3 get_direction();
	
	virtual void set_gl_light(int n, unsigned long time = XFORM_LOCAL_PRS) const;
};

#endif	// _LIGHT_HPP_
