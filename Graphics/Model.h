/*
 *  Model.h
 */

#pragma once
class Model;
class ModelFace;
class ModelArrays;
class ModelVertex;   // Temporarily used while optimizing.
class ModelTriangle; //
class ModelEdge;     //
class ModelShape;    //
class ModelObject;
class ModelMaterial;

#include "PlatformSpecific.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <stdint.h>
#include "RaptorGL.h"
#include "Vec.h"
#include "Pos.h"
#include "Animation.h"
#include "Color.h"
#include "Rand.h"
#include "Randomizer.h"


class Model
{
public:
	std::map<std::string,ModelObject> Objects;
	std::map<std::string,ModelMaterial> Materials;
	std::vector<std::string> MaterialFiles;
	double Length, Height, Width, MinFwd, MaxFwd, MinUp, MaxUp, MinRight, MaxRight, MaxRadius;
	
	Model( void );
	virtual ~Model();
	
	void Clear( void );
	void BecomeInstance( const Model *other );
	void BecomeCopy( const Model *other );
	bool LoadOBJ( std::string filename, bool get_textures = true );
	bool IncludeOBJ( std::string filename, bool get_textures = true );
	void MakeMaterialArrays( void );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	void Optimize( double vertex_tolerance = 0., double normal_tolerance = 0.001, double dot_tolerance = 0.0001 );
	
	void Draw( const Pos3D *pos = NULL, const std::set<std::string> *object_names = NULL, const Color *wireframe = NULL, double exploded = 0., int explosion_seed = 0, double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawAt( const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawObjectsAt( const std::list<std::string> *object_names, const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawWireframeAt( const Pos3D *pos, Color color, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	
	double DistanceFromLine( const Pos3D *pos, Pos3D *nearest, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2a, const Pos3D *pos2b, double block_size = 0. ) const;
	double DistanceFromSphere( const Pos3D *pos, Pos3D *nearest, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size = 0. ) const;
	bool CollidesWithSphere( const Pos3D *pos, Pos3D *at, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size = 0. ) const;
	bool CollidesWithModel( const Pos3D *pos1, Pos3D *at, const std::set<std::string> *object_names1, std::string *hit1, double exploded1, int explosion_seed1, const Model *model2, const Pos3D *pos2, const std::set<std::string> *object_names2, std::string *hit2, double exploded2, int explosion_seed2, double block_size = 0., bool check_faces = true ) const;
	void MarkBlockMap( std::map< uint64_t, std::set<const GLdouble*> > *blockmap, std::vector< std::pair<ModelArrays,std::string> > *keep_arrays, const Pos3D *pos, const std::set<std::string> *object_names, double exploded, int explosion_seed, double block_size = 0. ) const;
	
	void Move( double fwd, double up, double right );
	void ScaleBy( double scale );
	void ScaleBy( double fwd_scale, double up_scale, double right_scale );
	
	double GetLength( void );
	double GetHeight( void );
	double GetWidth( void );
	double GetTriagonal( void );
	double Triagonal( void ) const;
	double GetMaxDim( void );
	double GetMaxRadius( void );
	double GetMaxTriangleEdge( void ) const;
	
	size_t ArrayCount( void ) const;
	size_t TriangleCount( void ) const;
	size_t VertexCount( void ) const;
};


class ModelFace
{
public:
	std::vector<Vec3D> Vertices;
	std::vector<Vec2D> TexCoords;
	std::vector<Vec3D> Normals;
	int SmoothGroup;
	
	ModelFace( std::vector<Vec3D> &vertices, std::vector<Vec2D> &tex_coords, std::vector<Vec3D> &normals, int smooth_group = 0 );
	virtual ~ModelFace();
};


class ModelArrays
{
public:
	size_t VertexCount;
	GLdouble *VertexArray, *WorldSpaceVertexArray;
	GLfloat *TexCoordArray;
	GLfloat *NormalArray;
	bool Allocated;
	
	ModelArrays( void );
	ModelArrays( const ModelArrays &other );
	virtual ~ModelArrays();
	
	void Clear( void );
	void BecomeCopy( const ModelArrays *other );
	void BecomeInstance( const ModelArrays *other );
	void Resize( size_t vertex_count );
	void AddFaces( std::vector<ModelFace> &faces );
	void RemoveFace( size_t face_index );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	void Optimize( double vertex_tolerance = 0.0001, double normal_tolerance = 0.1, double dot_tolerance = 0.001 );
	void MakeWorldSpace( const Pos3D *pos, double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	bool HasWorldSpaceVertex( const GLdouble *vertex ) const;
};


class ModelVertex
{
public:
	Vec3D Vertex;
	Vec2D TexCoord;
	Vec3D Normal;
	std::set<size_t> EdgeIndices;
	bool operator < ( const ModelVertex &other ) const;
};


class ModelTriangle
{
public:
	size_t FaceIndex;
	ModelVertex Vertices[ 3 ];
	Vec3D CalculatedNormal;
	
	ModelTriangle( const ModelArrays *arrays, size_t face_index );
	uint8_t SharesEdge( const ModelTriangle *other, double vertex_tolerance, double normal_tolerance, const Vec3D *normal = NULL ) const;
};


class ModelEdge
{
public:
	ModelVertex Vertices[ 2 ];
	Vec2D OnPlane[ 2 ];
	bool Outside;
	
	ModelEdge( const ModelVertex &v1, const ModelVertex &v2, const Vec3D &u, const Vec3D &v, bool outside );
	bool Intersects( const ModelEdge &other ) const;
	bool Intersects( const Vec2D &end1, const Vec2D &end2 ) const;
};


class ModelShape
{
public:
	std::vector<ModelTriangle> Triangles;
	Vec3D Normal, Center, Min, Max;
	
	ModelShape( std::vector<ModelTriangle> *triangles, double vertex_tolerance, double normal_tolerance );
	void AddTriangles( const ModelTriangle triangle, std::vector<ModelTriangle> *triangles, double vertex_tolerance, double normal_tolerance );
	std::vector< std::vector<ModelVertex> > AllEdges( double vertex_tolerance, double normal_tolerance ) const;
	std::vector<ModelVertex> OptimizeEdge( std::vector<ModelVertex> &vertices, double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const;
	std::vector< std::vector<ModelVertex> > AllOptimizedEdges( double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const;
	std::vector< std::vector<ModelVertex> > OptimizedTriangles( double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const;
};


class ModelObject
{
public:
	std::string Name;
	std::map<std::string,ModelArrays> Arrays;
	std::vector<Vec3D> Points;
	std::vector< std::vector<Vec3D> > Lines;
	Pos3D CenterPoint;
	double MinFwd, MaxFwd, MinUp, MaxUp, MinRight, MaxRight, MaxRadius, MaxTriangleEdge;
	
	ModelObject( void );
	ModelObject( const ModelObject &other );
	virtual ~ModelObject();
	
	void BecomeInstance( const ModelObject *other );
	void AddFaces( std::string mtl, std::vector<ModelFace> &faces );
	
	void Recalc( void );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	Pos3D GetCenterPoint( void );
	double GetLength( void );
	double GetHeight( void );
	double GetWidth( void );
	double GetMaxRadius( void );
	Vec3D GetExplosionMotion( int seed = 0, Randomizer *randomizer = &GlobalRandomizer ) const;
	Vec3D GetExplosionRotationAxis( int seed = 0, Randomizer *randomizer = &GlobalRandomizer ) const;
	double GetExplosionRotationRate( int seed = 0, Randomizer *randomizer = &GlobalRandomizer ) const;
	
	static Randomizer GlobalRandomizer;
	
private:
	bool NeedsRecalc;
};


class ModelMaterial
{
public:
	Animation Texture;
	Color Ambient, Diffuse, Specular;
	float Shininess;
	ModelArrays Arrays;
	
	ModelMaterial( void );
	ModelMaterial( const ModelMaterial &other );
	virtual ~ModelMaterial();
	
	void BecomeInstance( const ModelMaterial *other );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
};
