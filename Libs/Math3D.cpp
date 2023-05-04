/*
 *  Math3D.cpp
 */

#include "Math3D.h"
#include "Num.h"
#include <cmath>
#include <cfloat>


#ifndef EPSILON
#define EPSILON 0.0001
#endif


// ----------------------------------------------------------------------------


Vec3D Math3D::WorldspaceVec( const Pos3D *pos, double fwd, double up, double right )
{
	// Return vector translated to worldspace.
	// Does not add pos X/Y/Z.
	return ((pos->Fwd * fwd) + (pos->Up * up) + (pos->Right * right));
}


Pos3D Math3D::WorldspacePos( const Pos3D *pos, double fwd, double up, double right )
{
	// Return position translated to worldspace.
	// Does add pos X/Y/Z.
	return (*pos + (pos->Fwd * fwd) + (pos->Up * up) + (pos->Right * right));
}


// ----------------------------------------------------------------------------


double Math3D::PointToPointDist( const Pos3D *pt1, const Pos3D *pt2 )
{
	Vec3D diff( pt2->X - pt1->X, pt2->Y - pt1->Y, pt2->Z - pt1->Z );
	return diff.Length();
}


double Math3D::PointToLineSegDist( const Pos3D *pt, const Pos3D *end1, const Pos3D *end2 )
{
	// http://forums.codeguru.com/printthread.php?t=194400
	
	Vec3D end1_to_end2( end2->X - end1->X, end2->Y - end1->Y, end2->Z - end1->Z );
	double len = end1_to_end2.Length();
	
	if( len <= 0. )
		return PointToPointDist( pt, end1 );
	
	Vec3D end1_to_pt( pt->X - end1->X, pt->Y - end1->Y, pt->Z - end1->Z );
	double r = end1_to_end2.Dot( &end1_to_pt ) / (len*len);
	
	if( r <= 0. )
		return PointToPointDist( pt, end1 );
	else if( r >= 1. )
		return PointToPointDist( pt, end2 );
	
	Pos3D pt_on_line( end1 );
	end1_to_end2.ScaleBy( r );
	pt_on_line += end1_to_end2;
	
	return PointToPointDist( pt, &pt_on_line );
}


double Math3D::LineSegToLineSegDist( const Pos3D *line1end1, const Pos3D *line1end2, const Pos3D *line2end1, const Pos3D *line2end2 )
{
	// http://geomalgorithms.com/a07-_distance.html
	
	Vec3D u( line1end2->X - line1end1->X, line1end2->Y - line1end1->Y, line1end2->Z - line1end1->Z );
	if( u.Length() <= 0. )
		return PointToLineSegDist( line1end1, line2end1, line2end2 );
	
	Vec3D v( line2end2->X - line2end1->X, line2end2->Y - line2end1->Y, line2end2->Z - line2end1->Z );
	if( v.Length() <= 0. )
		return PointToLineSegDist( line2end1, line1end1, line1end2 );
	
	Vec3D w( line1end1->X - line2end1->X, line1end1->Y - line2end1->Y, line1end1->Z - line2end1->Z );
	if( w.Length() <= 0. )
		return 0.;
	
	double a = u.Dot(&u);
	double b = u.Dot(&v);
	double c = v.Dot(&v);
	double d = u.Dot(&w);
	double e = v.Dot(&w);
	double D = a*c - b*b;
	double sD = D;
	double tD = D;
	double sc = 0., sN = 0., tc = 0., tN = 0.;
	
	// compute the line parameters of the two closest points
	if( D <= EPSILON )
	{
		// the lines are parallel
		sN = 0.;  // force using point P0 on segment S1
		sD = 1.;  // to prevent possible division by 0 later
		tN = e;
		tD = c;
	}
	else
	{
		// get the closest points on the infinite lines
		sN = b*e - c*d;
		tN = a*e - b*d;
		if( sN < 0. )
		{
			// sc < 0 => the s=0 edge is visible
			sN = 0.;
			tN = e;
			tD = c;
		}
		else if( sN > sD )
		{
			// sc > 1 => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if( tN < 0. )
	{
		// tc < 0 => the t=0 edge is visible
		tN = 0.;
		// recompute sc for this edge
		if( -d < 0. )
			sN = 0.;
		else if( -d > a )
			sN = sD;
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if( tN > tD )
	{
		// tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if( (-d + b) < 0. )
			sN = 0.;
		else if( (-d + b) > a )
			sN = sD;
		else
		{
			sN = -d + b;
			sD = a;
		}
	}
	
	// finally do the division to get sc and tc
	sc = (fabs(sN) < EPSILON ? 0. : sN / sD);
	tc = (fabs(tN) < EPSILON ? 0. : tN / tD);
	
	// get the difference of the two closest points
	Vec3D dP = w + (u * sc) - (v * tc);  // = S1(sc) - S2(tc)
	
	return dP.Length();   // return the closest distance
}


double Math3D::MinimumDistance( const Pos3D *pt1, const Vec3D *motion1, const Pos3D *pt2, const Vec3D *motion2, double dt )
{
	Pos3D pt1_end( pt1 );
	pt1_end += (*motion1 * dt) - (*motion2 * dt);
	
	return PointToLineSegDist( pt2, pt1, &pt1_end );
}


// ----------------------------------------------------------------------------


bool Math3D::LineIntersectsPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane )
{
	Vec3D difference( plane->X - end1->X, plane->Y - end1->Y, plane->Z - end1->Z );
	double dist1 = difference.Dot( &(plane->Up) );
	
	difference.Set( plane->X - end2->X, plane->Y - end2->Y, plane->Z - end2->Z );
	double dist2 = difference.Dot( &(plane->Up) );
	
	return ((dist1 * dist2) < 0.);
}


Pos3D Math3D::CollisionPoint( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane )
{
	Vec3D ab( end2->X - end1->X, end2->Y - end1->Y, end2->Z - end1->Z );
	Vec3D ac( plane->X - end1->X, plane->Y - end1->Y, plane->Z - end1->Z );
	const Vec3D *normal = &(plane->Up);
	
	Pos3D pt;
	pt.X = end1->X + ab.X * ac.Dot( normal ) / ab.Dot( normal );
	pt.Y = end1->Y + ab.Y * ac.Dot( normal ) / ab.Dot( normal );
	pt.Z = end1->Z + ab.Z * ac.Dot( normal ) / ab.Dot( normal );
	return pt;
}


Pos3D Math3D::NearestPointToPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane )
{
	Pos3D pt_on_plane = CollisionPoint( end1, end2, plane );
	
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


// ----------------------------------------------------------------------------


Pos3D Math3D::FaceCenter( const double *vertex_array, int vertex_count )
{
	Pos3D center( 0., 0., 0. );
	
	for( int i = 0; i < vertex_count; i ++ )
	{
		center.X += vertex_array[ i*3 ];
		center.Y += vertex_array[ i*3 + 1 ];
		center.Z += vertex_array[ i*3 + 2 ];
	}
	
	if( vertex_count )
	{
		center.X /= (double) vertex_count;
		center.Y /= (double) vertex_count;
		center.Z /= (double) vertex_count;
	}
	
	return center;
}


Plane3D Math3D::FaceToPlane( const double *vertex_array, int vertex_count )
{
	if( vertex_count >= 3 )
	{
		Plane3D plane( vertex_array[ 0 ], vertex_array[ 1 ], vertex_array[ 2 ] );
		plane.Fwd.Set( vertex_array[ 3 ] - vertex_array[ 0 ], vertex_array[ 4 ] - vertex_array[ 1 ], vertex_array[ 5 ] - vertex_array[ 2 ] );
		if( plane.Fwd.Length() <= 0. )
			plane.Fwd.Set( 1., 0., 0. );
		plane.Right.Set( vertex_array[ 6 ] - vertex_array[ 0 ], vertex_array[ 7 ] - vertex_array[ 1 ], vertex_array[ 8 ] - vertex_array[ 2 ] );
		if( plane.Right.Length() <= 0. )
			plane.Right.Set( 0., 1., 0. );
		plane.Up = plane.Fwd.Cross(&(plane.Right));
		if( plane.Up.Length() <= 0. )
			plane.Up.Set( 0., 0., 1. );
		else
			plane.Up.ScaleTo( 1. );
		
		return plane;
	}
	
	return Plane3D();
}


bool Math3D::PointWithinFace( const Pos3D *pt, const double *vertex_array, int vertex_count )
{
	Plane3D plane = FaceToPlane( vertex_array, vertex_count );
	
	if( vertex_count >= 2 )
	{
		Pos3D corner( vertex_array[ vertex_count*3 - 3 ], vertex_array[ vertex_count*3 - 2 ], vertex_array[ vertex_count*3 - 1 ] );
		Vec3D edge( vertex_array[ 0 ] - corner.X, vertex_array[ 1 ] - corner.Y, vertex_array[ 2 ] - corner.Z );
		Vec3D out = edge.Cross( &(plane.Up) );
		if( pt->DistAlong( &out, &corner ) > 0. )
			return false;
		
		for( int i = 0; i + 1 < vertex_count; i ++ )
		{
			corner.SetPos( vertex_array[ i*3 ], vertex_array[ i*3 + 1 ], vertex_array[ i*3 + 2 ] );
			edge.Set( vertex_array[ i*3 + 3 ] - corner.X, vertex_array[ i*3 + 4 ] - corner.Y, vertex_array[ i*3 + 5 ] - corner.Z );
			out = edge.Cross( &(plane.Up) );
			if( pt->DistAlong( &out, &corner ) > 0. )
				return false;
		}
		
		return true;
	}
	else if( vertex_count )
	{
		Pos3D face_dot( vertex_array[ 0 ], vertex_array[ 1 ], vertex_array[ 2 ] );
		return (PointToPointDist( pt, &face_dot ) < EPSILON);
	}
	
	return false;
}


double Math3D::PointDistFromFace( const Pos3D *pt, const double *vertex_array, int vertex_count )
{
	if( PointWithinFace( pt, vertex_array, vertex_count ) )
	{
		Plane3D plane = FaceToPlane( vertex_array, vertex_count );
		return fabs( pt->DistAlong( &(plane.Up), &plane ) );
	}
	
	Pos3D corner( vertex_array[ 0 ], vertex_array[ 1 ], vertex_array[ 2 ] );
	double nearest = DBL_MAX;
	
	for( int i = 1; i + 1 < vertex_count; i ++ )
	{
		Pos3D pt1( vertex_array[ 3 * i ], vertex_array[ 3 * i + 1 ], vertex_array[ 3 * i + 2 ] );
		Pos3D pt2( vertex_array[ 3 * i + 3 ], vertex_array[ 3 * i + 4 ], vertex_array[ 3 * i + 5 ] );
		
		nearest = PointToLineSegDist( pt, &pt1, &pt2 );
		double dist = PointToLineSegDist( pt, &corner, &pt1 );
		if( dist < nearest )
			nearest = dist;
		dist = PointToLineSegDist( pt, &corner, &pt2 );
		if( dist < nearest )
			nearest = dist;
	}
	
	return nearest;
}


double Math3D::LineSegDistFromFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count )
{
	if( LineIntersectsFace( end1, end2, vertex_array, vertex_count ) )
		return 0.;
	else if( vertex_count )
	{
		// First check v[0],v[n-1] side.
		Pos3D pt1( vertex_array[ 0 ], vertex_array[ 1 ], vertex_array[ 2 ] );
		Pos3D pt2( vertex_array[ 3*vertex_count - 3 ], vertex_array[ 3*vertex_count - 2 ], vertex_array[ 3*vertex_count - 1 ] );
		double nearest = LineSegToLineSegDist( end1, end2, &pt1, &pt2 );
		
		// Check v[i],v[i+1] sides.
		for( int i = 0; i + 1 < vertex_count; i ++ )
		{
			pt1.SetPos( vertex_array[ 3*i ], vertex_array[ 3*i + 1 ], vertex_array[ 3*i + 2 ] );
			pt2.SetPos( vertex_array[ 3*i + 3 ], vertex_array[ 3*i + 4 ], vertex_array[ 3*i + 5 ] );
			
			double dist = LineSegToLineSegDist( end1, end2, &pt1, &pt2 );
			if( dist < nearest )
				nearest = dist;
		}
		
		// Check for a nearer point to the plane.
		if( vertex_count >= 3 )
		{
			Plane3D plane = FaceToPlane( vertex_array, vertex_count );
			
			Pos3D pt = NearestPointToPlane( end1, end2, &plane );
			if( PointWithinFace( &pt, vertex_array, vertex_count ) )
			{
				double dist = fabs( pt.DistAlong( &(plane.Up), &plane ) );
				if( dist < nearest )
					nearest = dist;
			}
		}
		
		return nearest;
	}
	else
		return DBL_MAX;
}


bool Math3D::LineIntersectsFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count, Pos3D *at )
{
	Plane3D plane = FaceToPlane( vertex_array, vertex_count );
	
	if( LineIntersectsPlane( end1, end2, &plane ) )
	{
		Pos3D pt_on_plane = CollisionPoint( end1, end2, &plane );

		if( PointWithinFace( &pt_on_plane, vertex_array, vertex_count ) )
		{
			if( at )
				at->Copy( &pt_on_plane );
			
			return true;
		}
	}
	
	return false;
}


uint64_t Math3D::BlockMapIndex( double x, double y, double z, double block_size )
{
	int64_t bx = x / block_size + ((x >= 0.) ? 0.5 : -0.5);
	int64_t by = y / block_size + ((y >= 0.) ? 0.5 : -0.5);
	int64_t bz = z / block_size + ((z >= 0.) ? 0.5 : -0.5);
	return (bx & 0x00000000001FFFFF)
	    | ((by & 0x00000000001FFFFF) << 21)
	    | ((bz & 0x00000000003FFFFF) << 42);
}


std::set<uint64_t> Math3D::BlocksInRadius( double x, double y, double z, double block_size, double radius )
{
	std::set<uint64_t> blocks;
	int steps = ceil( radius / block_size );
	
	for( int i = -steps; i <= steps; i ++ )
	{
		for( int j = -steps; j <= steps; j ++ )
		{
			for( int k = -steps; k <= steps; k ++ )
			{
				if( i && j && k )
				{
					// For any cube not along a cardinal axis, check if its nearest corner is within the radius.
					double dx = (i + ((i<0) ? 0.5 : -0.5)) * block_size;
					double dy = (j + ((j<0) ? 0.5 : -0.5)) * block_size;
					double dz = (k + ((k<0) ? 0.5 : -0.5)) * block_size;
					if( sqrt( dx*dx + dy*dy + dz*dz ) > radius )
						continue;
				}
				
				double cx = x + i * block_size;
				double cy = y + j * block_size;
				double cz = z + k * block_size;
				blocks.insert( BlockMapIndex( cx, cy, cz, block_size ) );
			}
		}
	}
	
	return blocks;
}
