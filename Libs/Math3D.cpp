/*
 *  Math3D.cpp
 */

#include "Math3D.h"
#include "Num.h"
#include <cmath>


#ifndef EPSILON
#define EPSILON 0.0001
#endif


double Math3D::PointToPointDist( const Point3D *pt1, const Point3D *pt2 )
{
	Vec3D diff( pt2->X - pt1->X, pt2->Y - pt1->Y, pt2->Z - pt1->Z );
	return diff.Length();
}


double Math3D::PointToLineSegDist( const Point3D *pt, const Point3D *end1, const Point3D *end2 )
{
	// http://forums.codeguru.com/printthread.php?t=194400
	
	Vec3D end1_to_end2( end2->X - end1->X, end2->Y - end1->Y, end2->Z - end1->Z );
	Vec3D end1_to_pt( pt->X - end1->X, pt->Y - end1->Y, pt->Z - end1->Z );
	double len = end1_to_end2.Length();
	double r = end1_to_end2.Dot( &end1_to_pt ) / (len*len);
	
	if( r <= 0. )
		return PointToPointDist( pt, end1 );
	else if( r >= 1. )
		return PointToPointDist( pt, end2 );
	
	Point3D pt_on_line( end1 );
	end1_to_end2.ScaleBy( r );
	pt_on_line += end1_to_end2;
	return PointToPointDist( pt, &pt_on_line );
}


double Math3D::MinimumDistance( const Point3D *pt1, const Vec3D *motion1, const Point3D *pt2, const Vec3D *motion2, double dt )
{
	Point3D pt1_end( motion1 );
	pt1_end -= *motion2;
	pt1_end.ScaleBy( dt );
	pt1_end += *pt1;
	
	return PointToLineSegDist( pt2, pt1, &pt1_end );
}


double Math3D::MinimumDistance( const Pos3D *pos1, const Vec3D *motion1, const Pos3D *pos2, const Vec3D *motion2, double dt )
{
	Point3D pt1( pos1->X, pos1->Y, pos1->Z );
	Point3D pt2( pos2->X, pos2->Y, pos2->Z );
	return MinimumDistance( &pt1, motion1, &pt2, motion2, dt );
}


bool Math3D::LineIntersectsPlane( const Point3D *end1, const Point3D *end2, const Plane3D *plane )
{
	Vec3D difference( plane->X - end1->X, plane->Y - end1->Y, plane->Z - end1->Z );
	double dist1 = difference.Dot( &(plane->Up) );
	
	difference.Set( plane->X - end2->X, plane->Y - end2->Y, plane->Z - end2->Z );
	double dist2 = difference.Dot( &(plane->Up) );
	
	return ((dist1 * dist2) < 0.0);
}


bool Math3D::PointWithinFace( const Point3D *pt, const double *vertex_array, int vertex_count )
{
	Point3D corner( vertex_array[ 0 ], vertex_array[ 1 ], vertex_array[ 2 ] );
	
	for( int i = 0; i < vertex_count - 2; i ++ )
	{
		Vec3D vec_a( vertex_array[ 3 * i + 3 ] - corner.X, vertex_array[ 3 * i + 4 ] - corner.Y, vertex_array[ 3 * i + 5 ] - corner.Z );
		Vec3D vec_b( vertex_array[ 3 * i + 6 ] - corner.X, vertex_array[ 3 * i + 7 ] - corner.Y, vertex_array[ 3 * i + 8 ] - corner.Z );
		double len_a = vec_a.Length();
		double len_b = vec_b.Length();
		Vec3D vec_from_corner( pt->X - corner.X, pt->Y - corner.Y, pt->Z - corner.Z );
		double dot_a = vec_from_corner.Dot( &vec_a ) / (len_a * len_a);
		double dot_b = vec_from_corner.Dot( &vec_b ) / (len_b * len_b);
		if( (dot_a >= 0.) && (dot_b >= 0.) && ((dot_a + dot_b) <= 1.) )
			return true;
	}
	
	return false;
}


Point3D Math3D::CollisionPoint( const Point3D *end1, const Point3D *end2, const Plane3D *plane )
{
	Vec3D ab( end2->X - end1->X, end2->Y - end1->Y, end2->Z - end1->Z );
	Vec3D ac( plane->X - end1->X, plane->Y - end1->Y, plane->Z - end1->Z );
	const Vec3D *normal = &(plane->Up);
	
	Point3D pt;
	pt.X = end1->X + ab.X * ac.Dot( normal ) / ab.Dot( normal );
	pt.Y = end1->Y + ab.Y * ac.Dot( normal ) / ab.Dot( normal );
	pt.Z = end1->Z + ab.Z * ac.Dot( normal ) / ab.Dot( normal );
	return pt;
}


Point3D Math3D::CollisionPoint( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane )
{
	Point3D pt1( end1->X, end1->Y, end1->Z );
	Point3D pt2( end2->X, end2->Y, end2->Z );
	return CollisionPoint( &pt1, &pt2, plane );
}


Point3D Math3D::NearestPointToPlane( const Point3D *end1, const Point3D *end2, const Plane3D *plane )
{
	Point3D pt_on_plane = CollisionPoint( end1, end2, plane );
	
	if( LineIntersectsPlane( end1, end2, plane ) )
		return pt_on_plane;
	else
	{
		Vec3D vec1( end1->X - pt_on_plane.X, end1->Y - pt_on_plane.Y, end1->Z - pt_on_plane.Z );
		Vec3D vec2( end2->X - pt_on_plane.X, end2->Y - pt_on_plane.Y, end2->Z - pt_on_plane.Z );
		if( vec2.Length() < vec1.Length() )
			return *end2;
		else
			return *end1;
	}
}


Point3D Math3D::NearestPointToPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane )
{
	Point3D pt1( end1->X, end1->Y, end1->Z );
	Point3D pt2( end2->X, end2->Y, end2->Z );
	return NearestPointToPlane( &pt1, &pt2, plane );
}


/*
void Vec2d_reflect (struct Vec2d *output, const struct Vec2d *original, const struct Vec2d *normal)
{
	double dp;
	
	dp = Vec2d_dotProduct (original, normal);
	
	output->x = original->x - 2.0 * dp * normal->x;
	output->y = original->y - 2.0 * dp * normal->y;
}

void Vec3d_reflect (struct Vec3d *output, const struct Vec3d *original, const struct Vec3d *normal)
{
	// Note: The general formula is: Out = In - 2*Normal*(In dot Normal)
	
	double dp;
	
	dp = Vec3d_dotProduct (original, normal);
	
	output->x = original->x - 2.0 * dp * normal->x;
	output->y = original->y - 2.0 * dp * normal->y;
	output->z = original->z - 2.0 * dp * normal->z;
}

void Vec2d_reflectAnySide (struct Vec2d *output, const struct Vec2d *original, const struct Vec2d *normal)
{
	struct Vec2d newNormal;
	
	if (Vec2d_dotProduct (original, normal) <= 0.0)
		return Vec2d_reflect (output, original, normal);
	
	else
	{
		newNormal.x = -1.0 * normal->x;
		newNormal.y = -1.0 * normal->y;
		
		return Vec2d_reflect (output, original, &newNormal);
	}
}

void Vec3d_reflectAnySide (struct Vec3d *output, const struct Vec3d *original, const struct Vec3d *normal)
{
	struct Vec3d newNormal;
	
	if (Vec3d_dotProduct (original, normal) <= 0.0)
		return Vec3d_reflect (output, original, normal);
	
	else
	{
		newNormal.x = -1.0 * normal->x;
		newNormal.y = -1.0 * normal->y;
		newNormal.z = -1.0 * normal->z;
		
		return Vec3d_reflect (output, original, &newNormal);
	}
}
*/
