/*
 *  Math3D.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <cstddef>
#include <stdint.h>
#include "Vec.h"
#include "Pos.h"


typedef Pos3D Plane3D;


namespace Math3D
{
	Vec3D WorldspaceVec( const Pos3D *pos, double fwd, double up, double right );
	Pos3D WorldspacePos( const Pos3D *pos, double fwd, double up, double right );
	
	double Length( double x, double y, double z );
	double PointToPointDist( const Pos3D *pt1, const Pos3D *pt2 );
	double PointToLineSegDist( const Pos3D *pt, const Pos3D *end1, const Pos3D *end2, Pos3D *pt_on_line = NULL );
	double LineSegToLineSegDist( const Pos3D *line1end1, const Pos3D *line1end2, const Pos3D *line2end1, const Pos3D *line2end2 );
	double MinimumDistance( const Pos3D *pt1, const Vec3D *motion1, const Pos3D *pt2, const Vec3D *motion2, double dt = 1. );
	Pos3D NearestPointOnLine( const Pos3D *pt, const Pos3D *end1, const Pos3D *end2 );
	
	bool LineIntersectsPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	Pos3D CollisionPoint( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	Pos3D NearestPointToPlane( const Pos3D *end1, const Pos3D *end2, const Plane3D *plane );
	
	Pos3D FaceCenter( const double *vertex_array, int vertex_count = 3 );
	Plane3D FaceToPlane( const double *vertex_array, int vertex_count = 3 );
	bool PointWithinFace( const Pos3D *pt, const double *vertex_array, int vertex_count = 3, Pos3D *pt_on_face = NULL );
	double PointDistFromFace( const Pos3D *pt, const double *vertex_array, int vertex_count );
	double LineSegDistFromFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count, Pos3D *intersection = NULL );
	bool LineIntersectsFace( const Pos3D *end1, const Pos3D *end2, const double *vertex_array, int vertex_count = 3, Pos3D *at = NULL );
	
	Pos3D NearestPointOnSphere( const Pos3D *pt, const Pos3D *center, double radius );
	
	uint64_t BlockMapIndex( double x, double y, double z, double block_size );
	uint64_t BlockMapIndex( double x, double y, double z, double block_size, int64_t *bx, int64_t *by, int64_t *bz );
	uint64_t BlockFromParts( int64_t bx, int64_t by, int64_t bz );
	void BlockToParts( uint64_t index, int64_t *bx, int64_t *by, int64_t *bz );
	void BlockCenter( uint64_t index, double block_size, double *x, double *y, double *z );
	std::set<uint64_t> BlocksInCube( double min_x, double min_y, double min_z, double max_x, double max_y, double max_z, double block_size );
	void BlocksInCube( std::set<uint64_t> *blocks, double min_x, double min_y, double min_z, double max_x, double max_y, double max_z, double block_size );
	std::set<uint64_t> BlocksInCube( int64_t min_bx, int64_t min_by, int64_t min_bz, int64_t max_bx, int64_t max_by, int64_t max_bz );
	void BlocksInCube( std::set<uint64_t> *blocks, int64_t min_bx, int64_t min_by, int64_t min_bz, int64_t max_bx, int64_t max_by, int64_t max_bz );
	std::set<uint64_t> BlocksInRadius( double x, double y, double z, double block_size, double radius );
	void BlocksInRadius( std::set<uint64_t> *blocks, double x, double y, double z, double block_size, double radius );
	std::set<uint64_t> BlocksInLine( double x1, double y1, double z1, double x2, double y2, double z2, double block_size );
	void BlocksInLine( std::set<uint64_t> *blocks, double x1, double y1, double z1, double x2, double y2, double z2, double block_size );
	std::set<uint64_t> BlocksInTriangle( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double block_size, int max_depth = 7 );
	void BlocksInTriangle( std::set<uint64_t> *blocks, double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double block_size, int max_depth = 7, int depth = 0 );
	std::set<uint64_t> BlocksNearTriangle( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double block_size );
	void BlocksNearTriangle( std::set<uint64_t> *blocks, double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double block_size );
}
