/*
 *  Math3D.h
 */

#pragma once

#include "platforms.h"

#include "Vec.h"
#include "Pos.h"


typedef Vec3D Point3D;
typedef Pos3D Plane3D;


namespace Math3D
{
	double PointToPointDist( const Point3D *pt1, const Point3D *pt2 );
	double PointToLineSegDist( const Point3D *pt, const Point3D *end1, const Point3D *end2 );
	double MinimumDistance( const Point3D *pt1, const Vec3D *motion1, const Point3D *pt2, const Vec3D *motion2, double dt = 1. );
	double MinimumDistance( const Pos3D *pos1, const Vec3D *motion1, const Pos3D *pos2, const Vec3D *motion2, double dt = 1. );
	bool LineIntersectsPlane( const Point3D *end1, const Point3D *end2, const Plane3D *plane );
	bool PointWithinFace( const Point3D *pt, const double *vertex_array, int vertex_count = 3 );
	Point3D CollisionPoint( const Point3D *end1, const Point3D *end2, const Plane3D *plane );
	Point3D CollisionPoint( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	Point3D NearestPointToPlane( const Point3D *end1, const Point3D *end2, const Plane3D *plane );
	Point3D NearestPointToPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
}
