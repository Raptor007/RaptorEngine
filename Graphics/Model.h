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
	void BecomeInstance( const ModelArrays *other, bool copy = true );  // FIXME: Copy shouldn't be necessary!
	void Resize( size_t vertex_count );
	void AddFaces( std::vector<ModelFace> &faces );
	void CalculateNormals( void );
	void ReverseNormals( void );
	void SmoothNormals( void );
	void MakeWorldSpace( const Pos3D *pos, double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
};


class ModelObject
{
public:
	std::string Name;
	std::map<std::string,ModelArrays> Arrays;
	std::vector<Vec3D> Points;
	std::vector< std::vector<Vec3D> > Lines;
	Pos3D CenterPoint;
	double MinFwd, MaxFwd, MinUp, MaxUp, MinRight, MaxRight;
	double MaxRadius;
	
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
