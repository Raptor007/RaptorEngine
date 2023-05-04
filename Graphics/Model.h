/*
 *  Model.h
 */

#pragma once
class Model;
class ModelFace;
class ModelArrays;
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


class Model
{
public:
	std::map<std::string,ModelObject> Objects;
	std::map<std::string,ModelMaterial> Materials;
	double Length, Height, Width;
	double MaxRadius;
	
	Model( void );
	virtual ~Model();
	
	void Clear( void );
	void BecomeInstance( Model *other );
	void BecomeCopy( Model *other );
	bool LoadOBJ( std::string filename, bool get_textures = true );
	bool IncludeOBJ( std::string filename, bool get_textures = true );
	void MakeMaterialArrays( void );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	
	void Draw( const Pos3D *pos = NULL, const std::set<std::string> *object_names = NULL, const Color *wireframe = NULL, double exploded = 0., int explosion_seed = 0, double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawAt( const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawObjectsAt( const std::list<std::string> *object_names, const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	void DrawWireframeAt( const Pos3D *pos, Color color, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	
	double DistanceFromLine( const Pos3D *pos, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2a, const Pos3D *pos2b, double block_size = 0. ) const;
	double DistanceFromSphere( const Pos3D *pos, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size = 0. ) const;
	bool CollidesWithSphere( const Pos3D *pos, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size = 0. ) const;
	bool CollidesWithModel( const Pos3D *pos1, const std::set<std::string> *object_names1, std::string *hit1, double exploded1, int explosion_seed1, const Model *model2, const Pos3D *pos2, const std::set<std::string> *object_names2, std::string *hit2, double exploded2, int explosion_seed2, double block_size = 0., bool check_faces = true ) const;
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
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	void MakeWorldSpace( const Pos3D *pos, double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	bool HasWorldSpaceVertex( const GLdouble *vertex ) const;
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
	Vec3D GetExplosionMotion( int seed = 0 ) const;
	Vec3D GetExplosionRotationAxis( int seed = 0 ) const;
	double GetExplosionRotationRate( int seed = 0 ) const;
	
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
