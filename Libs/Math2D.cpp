/*
 *  Math2D.cpp
 */

#include "Math2D.h"
#include <cstddef>
#include "Num.h"
#include <cmath>
#include <algorithm>


#ifndef EPSILON
#define EPSILON 0.0001
#endif


double Math2D::Length( double x, double y )
{
	return sqrt( x*x + y*y );
}


double Math2D::Angle( double x, double y )
{
	// These parameters are intentionally "wrong", since I use up as 0 and increase counter-clockwise.
	double angle = atan2( x, -y );
	
	// If the value of Angle is valid, try to keep it in the (-Pi,Pi] range.
	if( fabs(angle) < 1000.0 )
	{
		while( angle > M_PI )
			angle -= M_PI*2.0;
		while( angle < -M_PI )
			angle += M_PI*2.0;
	}
	
	return angle;
}


double Math2D::X( double angle )
{
	// These parameters are intentionally "wrong", since I use up as 0 and increase counter-clockwise.
	return sin( angle );
}

double Math2D::Y( double angle )
{
	// These parameters are intentionally "wrong", since I use up as 0 and increase counter-clockwise.
	return -cos( angle );
}


double Math2D::DotProduct( double x1, double y1, double x2, double y2 )
{
	return x1*x2 + y1*y2;
}


// ---------------------------------------------------------------------------


double Math2D::PointToPointDist( double xa, double ya, double xb, double yb )
{
	return Length( xb - xa, + yb - ya );
}


double Math2D::PointToLineSegDist( double xp, double yp, double xa, double ya, double xb, double yb )
{
	double xu = xp - xa;
	double yu = yp - ya;
	double xv = xb - xa;
	double yv = yb - ya;
	if( xu*xv + yu*yv < 0 )
		return sqrt( (xp-xa)*(xp-xa) + (yp-ya)*(yp-ya) );
	
	xu = xp - xb;
	yu = yp - yb;
	xv *= -1.0;
	yv *= -1.0;
	if( xu*xv + yu*yv < 0 )
		return sqrt( (xp-xb)*(xp-xb) + (yp-yb)*(yp-yb) );
	
	return fabs( ( xp*(ya-yb) + yp*(xb-xa) + (xa*yb - xb*ya) ) / sqrt( (xb-xa)*(xb-xa) + (yb-ya)*(yb-ya) ) );
}


double Math2D::LineSegToLineSegDist( double a1x, double a1y, double a2x, double a2y, double b1x, double b1y, double b2x, double b2y )
{
	bool does_intersect = LineIntersection( a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y, NULL, NULL );
	
	if( does_intersect )
		return 0;
	
	double dist_a1 = PointToLineSegDist( a1x, a1y, b1x, b1y, b2x, b2y );
	double dist_a2 = PointToLineSegDist( a2x, a2y, b1x, b1y, b2x, b2y );
	double dist_b1 = PointToLineSegDist( b1x, b1y, a1x, a1y, a2x, a2y );
	double dist_b2 = PointToLineSegDist( b2x, b2y, a1x, a1y, a2x, a2y );
	
	double min = dist_a1;
	if( dist_a2 < min )
		min = dist_a2;
	if( dist_b1 < min )
		min = dist_b1;
	if( dist_b2 < min )
		min = dist_b2;
	
	return min;
}


bool Math2D::LineIntersection( double a1x, double a1y, double a2x, double a2y, double b1x, double b1y, double b2x, double b2y, double *pt_x, double *pt_y )
{
	bool segments_intersect = true;
	
	// Check if either segment is zero-length.
	if( ((a1x == a2x) && (a1y == a2y)) || ((b1x == b2x) && (b1y == b2y)) )
	{
		// Assume they won't intersect since a segment is zero-length.
		segments_intersect = false;
		
		// Check if endpoints touch at a1.
		if( ((a1x == b1x) && (a1y == b1y)) || ((a1x == b2x) && (a1y == b2y)) )
		{
			if( pt_x )
				*pt_x = a1x;
			if( pt_y )
				*pt_y = a1y;
			
			// Endpoints touch, so they do intersect.
			return true;
		}
		
		// Check if endpoints touch at a2.
		if( ((a2x == b1x) && (a2y == b1y)) || ((a2x == b2x) && (a2y == b2y)) )
		{
			if( pt_x )
				*pt_x = a2x;
			if( pt_y )
				*pt_y = a2y;
			
			// Endpoints touch, so they do intersect.
			return true;
		}
	}
	
	// Calculate the relative vectors.
	
	double a1a2x = a2x - a1x;
	double a1a2y = a2y - a1y;
	
	double a1b1x = b1x - a1x;
	double a1b1y = b1y - a1y;
	
	double b1b2px = b1y - b2y;
	double b1b2py = b2x - b1x;
	
	// The distance along the a1a2 vector is the dot product here.
	double t = DotProduct( b1b2px, b1b2py, a1b1x, a1b1y ) / DotProduct( b1b2px, b1b2py, a1a2x, a1a2y );
	
	// Calculate the intersect point.
	double result_x = a1x + t * a1a2x;
	double result_y = a1y + t * a1a2y;
	
	// If pointers were passed, store the intersect point.
	if( pt_x )
		*pt_x = result_x;
	if( pt_y )
		*pt_y = result_y;
	
	// Make sure it exists on the a1a2 line segment.
	if( (t < 0.0) || (t > 1.0) )
		segments_intersect = false;
	
	// Make sure it exists on the b1b2 line segment.
	double b1b2l = PointToPointDist( b1x, b1y, b2x, b2y );
	if( (PointToPointDist( result_x, result_y, b1x, b1y ) > b1b2l) || (PointToPointDist( result_x, result_y, b2x, b2y ) > b1b2l) )
		segments_intersect = false;
	
	// Return a boolean for whether the two segments actually intersect or not.
	return segments_intersect;
}


void Math2D::NearestPointOnLineSeg( double px, double py, double a1x, double a1y, double a2x, double a2y, double *pt_x, double *pt_y )
{
	double dist = PointToLineSegDist( px, py, a1x, a1y, a2x, a2y );
	
	double result_x = 0.0;
	double result_y = 0.0;
	
	if( PointToPointDist( px, py, a1x, a1y ) - EPSILON <= dist )
	{
		// The point is nearest to endpoint a1.
		result_x = a1x;
		result_y = a1y;
	}
	else if( PointToPointDist( px, py, a2x, a2y ) - EPSILON <= dist )
	{
		// The point is nearest to endpoint a2.
		result_x = a2x;
		result_y = a2y;
	}
	else
	{
		// The point is nearest to some midpoint; build a perpendicular line and find the intersection.
		LineIntersection( a1x, a1y, a2x, a2y, px, py, px+(a2y-a1y), py-(a2x-a1x), &result_x, &result_y );
	}
	
	if( pt_x )
		*pt_x = result_x;
	if( pt_y )
		*pt_y = result_y;
}


// ---------------------------------------------------------------------------


bool Math2D::WithinRect( double px, double py, double x1, double y1, double x2, double y2 )
{
	double min_x = x1, max_x = x2, min_y = y1, max_y = y2;
	if( x1 > x2 )
	{
		min_x = x2;
		max_x = x1;
	}
	if( y1 > y2 )
	{
		min_y = y2;
		max_y = y1;
	}
	
	return (px >= min_x) && (px <= max_x) && (py >= min_y) && (py <= max_y);
}


bool Math2D::WithinRect( int px, int py, int x1, int y1, int x2, int y2 )
{
	int min_x = x1, max_x = x2, min_y = y1, max_y = y2;
	if( x1 > x2 )
	{
		min_x = x2;
		max_x = x1;
	}
	if( y1 > y2 )
	{
		min_y = y2;
		max_y = y1;
	}
	
	return (px >= min_x) && (px <= max_x) && (py >= min_y) && (py <= max_y);
}
