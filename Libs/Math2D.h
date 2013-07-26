/*
 *  Math2D.h
 */

#pragma once

#include "PlatformSpecific.h"


namespace Math2D
{
	double Length( double x, double y );
	double Angle( double x, double y );
	double X( double angle );
	double Y( double angle );
	double DotProduct( double x1, double y1, double x2, double y2 );
	
	double PointToPointDist( double x1, double y1, double x2, double y2 );
	double PointToLineSegDist( double px, double py, double a1x, double a1y, double a2x, double a2y );
	double LineSegToLineSegDist( double a1x, double a1y, double a2x, double a2y, double b1x, double b1y, double b2x, double b2y );
	bool LineIntersection( double a1x, double a1y, double a2x, double a2y, double b1x, double b1y, double b2x, double b2y, double *pt_x, double *pt_y );
	void NearestPointOnLineSeg( double px, double py, double a1x, double a1y, double a2x, double a2y, double *pt_x, double *pt_y );
	
	bool WithinRect( double px, double py, double x1, double y1, double x2, double y2 );
	bool WithinRect( int px, int py, int x1, int y1, int x2, int y2 );
}
