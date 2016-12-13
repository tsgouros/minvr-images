#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vecTypes.h"

bool loadOBJ(
	const char * path, 
	std::vector<MVec3> & out_vertices, 
	std::vector<MVec2> & out_uvs, 
	std::vector<MVec3> & out_normals
);

#endif
