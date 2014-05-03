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
#include "RaptorGL.h"
#include "Vec.h"
#include "Pos.h"
#include "Animation.h"
#include "Color.h"


class Model
{
public:
	std::map<std::string,ModelObject> Objects;
	std::map<std::string,ModelMaterial> Materials;
	double ExplodedSeconds;
	
	Model( void );
	virtual ~Model();
	
	void Clear( void );
	void Reset( void );
	void RandomizeExplosionVectors( double speed_scale = 1. );
	void SeedExplosionVectors( int seed, double speed_scale = 1. );
	void BecomeInstance( Model *other );
	bool LoadOBJ( std::string filename, bool get_textures = true );
	bool IncludeOBJ( std::string filename, bool get_textures = true );
	void MakeMaterialArrays( void );
	void CalculateNormals( void );
	
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
	double PrecalculatedTriagonal( void ) const;
	double GetMaxDim( void );
	double GetMaxRadius( void );
	double PrecalculatedMaxRadius( void ) const;
	
	void Explode( double dt );
	
	int ArrayCount( void ) const;
	int TriangleCount( void ) const;
	int VertexCount( void ) const;
	
private:
	double Length, Height, Width;
	double MaxRadius;
};


class ModelFace
{
public:
	std::vector<Vec3D> Vertices;
	std::vector<Vec2D> TexCoords;
	std::vector<Vec3D> Normals;
	
	ModelFace( std::vector<Vec3D> &vertices, std::vector<Vec2D> &tex_coords, std::vector<Vec3D> &normals );
	virtual ~ModelFace();
};


class ModelArrays
{
public:
	int VertexCount;
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
	void Resize( int vertex_count );
	void AddFaces( std::vector<ModelFace> &faces );
	void CalculateNormals( void );
	void MakeWorldSpace( const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
};


class ModelObject
{
public:
	std::string Name;
	std::map<std::string,ModelArrays> Arrays;
	Vec3D ExplosionRotationAxis, ExplosionMotion;
	double ExplosionRotationRate;
	
	ModelObject( void );
	ModelObject( const ModelObject &other );
	virtual ~ModelObject();
	
	void BecomeInstance( const ModelObject *other );
	void RandomizeExplosionVectors( double speed_scale = 1. );
	void SeedExplosionVectors( int seed, double speed_scale = 1. );
	void AddFaces( std::string mtl, std::vector<ModelFace> &faces );
	void Recalc( void );
	void CalculateNormals( void );
	Pos3D GetCenterPoint( void );
	Pos3D PrecalculatedCenterPoint( void ) const;
	double GetMaxRadius( void );
	double PrecalculatedMaxRadius( void ) const;
	Vec3D GetExplosionMotion( void );
	
private:
	Pos3D CenterPoint;
	double MaxRadius;
	bool NeedsRecalc;
};


class ModelMaterial
{
public:
	std::string Name;
	Animation Texture;
	Color Ambient, Diffuse;
	ModelArrays Arrays;
	
	ModelMaterial( std::string name = "" );
	ModelMaterial( const ModelMaterial &other );
	virtual ~ModelMaterial();
	
	void BecomeInstance( const ModelMaterial *other );
	void CalculateNormals( void );
};
