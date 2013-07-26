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
	
	Model( void );
	virtual ~Model();
	
	void Clear( void );
	void Reset( void );
	void BecomeInstance( Model *other );
	bool LoadOBJ( std::string filename );
	bool IncludeOBJ( std::string filename );
	void MakeMaterialArrays( void );
	void CalculateNormals( void );
	
	void DrawAt( const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
	
	void ScaleBy( double scale );
	void ScaleBy( double fwd_scale, double up_scale, double right_scale );
	double GetLength( void );
	double GetHeight( void );
	double GetWidth( void );
	double GetTriagonal( void );
	double GetMaxDim( void );
	
	void Explode( double dt );
	
	int ArrayCount( void );
	int TriangleCount( void );
	int VertexCount( void );
	
private:
	double Length, Height, Width;
	double ExplodedSeconds;
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
	void BecomeInstance( const ModelArrays *other );
	void Resize( int vertex_count );
	void AddFaces( std::vector<ModelFace> &faces );
	void CalculateNormals( void );
	void MakeWorldSpace( const Pos3D *pos, double scale = 1., double fwd_scale = 1., double up_scale = 1., double right_scale = 1. );
};


class ModelObject
{
public:
	std::map<std::string,ModelArrays> Arrays;
	Vec3D ExplosionRotationAxis, ExplosionMotion;
	double ExplosionRotationRate;
	
	ModelObject( void );
	ModelObject( const ModelObject &other );
	virtual ~ModelObject();
	
	void BecomeInstance( const ModelObject *other );
	void RandomizeExplosionVectors( void );
	void AddFaces( std::string mtl, std::vector<ModelFace> &faces );
	void Recalc( void );
	void CalculateNormals( void );
	Vec3D GetCenter( void );
	Vec3D GetExplosionMotion( void );
	
private:
	Vec3D CenterPoint;
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
