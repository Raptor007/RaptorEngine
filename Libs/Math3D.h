/*
 *  Math3D.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <cstddef>
#include "Vec.h"
#include "Pos.h"


typedef Pos3D Plane3D;


namespace Math3D
{
	double PointToPointDist( const Pos3D *pt1, const Pos3D *pt2 );
	double PointToLineSegDist( const Pos3D *pt, const Pos3D *end1, const Pos3D *end2 );
	double LineSegToLineSegDist( const Pos3D *line1end1, const Pos3D *line1end2, const Pos3D *line2end1, const Pos3D *line2end2 );
	double MinimumDistance( const Pos3D *pt1, const Vec3D *motion1, const Pos3D *pt2, const Vec3D *motion2, double dt = 1. );
	
	bool LineIntersectsPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	Pos3D CollisionPoint( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	Pos3D NearestPointToPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	
	Pos3D FaceCenter( const double *vertex_array, int vertex_count = 3 );
	Plane3D FaceToPlane( const double *vertex_array, int vertex_count = 3 );
	bool PointWithinFace( const Pos3D *pt, const double *vertex_array, int vertex_count = 3 );
	double PointDistFromFace( const Pos3D *pt, const double *vertex_array, int vertex_count );
	double LineSegDistFromFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count );
	bool LineIntersectsFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count = 3, Pos3D *at = NULL );
}
