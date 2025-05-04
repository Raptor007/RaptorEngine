/*
 *  Model.cpp
 */

#include "Model.h"

#include <cstddef>
#include <cmath>
#include <cfloat>
#include <fstream>
#include <set>
#include <algorithm>
#include "Str.h"
#include "Rand.h"
#include "Num.h"
#include "File.h"
#include "Math2D.h"
#include "Math3D.h"
#include "RaptorGame.h"


Model::Model( void )
{
	Clear();
}


Model::Model( const Model &other )
{
	BecomeCopy( &other );
}


Model::Model( const Model *other )
{
	BecomeCopy( other );
}


Model::~Model()
{
	Clear();
}


void Model::Clear( void )
{
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		delete obj_iter->second;
	Objects.clear();
	
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		delete mtl_iter->second;
	Materials.clear();
	
	MaterialFiles.clear();
	Length = 0.;
	Width = 0.;
	Height = 0.;
	MinFwd = MaxFwd = MinUp = MaxUp = MinRight = MaxRight = 0.;
	MaxRadius = 0.;
}


void Model::BecomeInstance( const Model *other )
{
	if( this == other )
		return;
	
	Clear();
	
	for( std::map<std::string,ModelObject*>::const_iterator obj_iter = other->Objects.begin(); obj_iter != other->Objects.end(); obj_iter ++ )
		Objects[ obj_iter->first ] = new ModelObject( obj_iter->second );
	
	for( std::map<std::string,ModelMaterial*>::const_iterator mtl_iter = other->Materials.begin(); mtl_iter != other->Materials.end(); mtl_iter ++ )
		Materials[ mtl_iter->first ] = new ModelMaterial( mtl_iter->second );
	
	MaterialFiles = other->MaterialFiles;
	
	Length = GetLength();
	Width = GetWidth();
	Height = GetHeight();
	MaxRadius = GetMaxRadius();
}


void Model::BecomeCopy( const Model *other )
{
	if( other != this )
		BecomeInstance( other );
	
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			array_iter->second->BecomeCopy();
	}
	
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second->Arrays.BecomeCopy();
}


bool Model::LoadOBJ( std::string filename, bool get_textures )
{
	Clear();
	
	bool return_value = IncludeOBJ( filename, get_textures );
	MakeMaterialArrays();
	return return_value;
}


bool Model::IncludeOBJ( std::string filename, bool get_textures )
{
	std::ifstream input( filename.c_str() );
	if( input.is_open() )
	{
		char buffer[ 1024 ] = "";
		
		size_t fwd_index = 3, up_index = 2, right_index = 1;
		double fwd_scale = 1., up_scale = 1., right_scale = 1.;
		bool clockwise = false;
		
		std::string obj = "", mtl = "";
		
		std::vector<Vec3D> vertices;
		std::vector<Vec2D> tex_coords;
		std::vector<Vec3D> normals;
		std::vector<ModelFace> faces;
		int smooth_group = 0;
		
		while( ! input.eof() )
		{
			buffer[ 0 ] = '\0';
			input.getline( buffer, 1024 );
			
			// Stop parsing at # for comments, and remove the linefeed chars.
			char *remove = strchr( buffer, '#' );
			if( remove )
				remove[ 0 ] = '\0';
			remove = strchr( buffer, '\n' );
			if( remove )
				remove[ 0 ] = '\0';
			remove = strchr( buffer, '\r' );
			if( remove )
				remove[ 0 ] = '\0';
			
			// Skip whitespace at the start of a line.
			char *buffer_start = buffer;
			while( buffer_start[ 0 ] && strchr( " \t", buffer_start[ 0 ] ) )
				buffer_start ++;
			
			// Don't parse empty/comment lines.
			if( ! buffer_start[ 0 ] )
				continue;
			
			std::vector<std::string> elements = CStr::SplitToVector( buffer_start, " " );
			
			if( elements.size() )
			{
				// OBJ
				
				if( elements.at( 0 ) == "v" )
				{
					if( elements.size() >= 4 )
					{
						double fwd = atof( elements.at( fwd_index ).c_str() ) * fwd_scale;
						double up = atof( elements.at( up_index ).c_str() ) * up_scale;
						double right = atof( elements.at( right_index ).c_str() ) * right_scale;
						
						vertices.push_back( Vec3D( fwd, up, right ) );
					}
				}
				else if( elements.at( 0 ) == "vt" )
				{
					if( elements.size() >= 3 )
					{
						double x = atof( elements.at( 1 ).c_str() );
						double y = 1. - atof( elements.at( 2 ).c_str() );
						
						tex_coords.push_back( Vec2D( x, y ) );
					}
				}
				else if( elements.at( 0 ) == "vn" )
				{
					if( elements.size() >= 4 )
					{
						double fwd = atof( elements.at( fwd_index ).c_str() ) * fwd_scale;
						double up = atof( elements.at( up_index ).c_str() ) * up_scale;
						double right = atof( elements.at( right_index ).c_str() ) * right_scale;
						
						Vec3D normal( fwd, up, right );
						normal.ScaleTo( 1. );
						normals.push_back( normal );
					}
				}
				else if( elements.at( 0 ) == "f" )
				{
					std::vector<Vec3D> face_vertices;
					std::vector<Vec2D> face_tex_coords;
					std::vector<Vec3D> face_normals;
					Vec3D calc_normal( 0., 0., 1. );
					
					for( size_t i = 1; i < elements.size(); i ++ )
					{
						std::vector<std::string> element_items = Str::SplitToVector( elements.at(i), "/" );
						int vertex_num = atoi(element_items.at(0).c_str());
						if( vertex_num )
						{
							if( vertex_num < 0 )
								vertex_num += vertices.size() + 1;
							else if( ! vertex_num )
								continue;
							if( (size_t)vertex_num > vertices.size() )
								continue;
							
							face_vertices.push_back( vertices[ vertex_num - 1 ] );
							
							if( element_items.size() >= 2 )
							{
								int tex_coord_num = atoi(element_items.at(1).c_str());
								if( tex_coord_num < 0 )
									tex_coord_num += tex_coords.size() + 1;
								
								if( tex_coord_num && ((size_t)tex_coord_num <= tex_coords.size()) )
									face_tex_coords.push_back( tex_coords[ tex_coord_num - 1 ] );
								else
									face_tex_coords.push_back( Vec2D( 0.5, 0.5 ) );
							}
							else
								face_tex_coords.push_back( Vec2D( 0.5, 0.5 ) );
							
							if( element_items.size() >= 3 )
							{
								int normal_num = atoi(element_items.at(2).c_str());
								if( normal_num < 0 )
									normal_num += normals.size() + 1;
								
								if( normal_num && ((size_t)normal_num <= normals.size()) )
									face_normals.push_back( normals[ normal_num - 1 ] );
								else
									face_normals.push_back( Vec3D( 0., 0., 0. ) );
							}
							else
								face_normals.push_back( Vec3D( 0., 0., 0. ) );
						}
					}
					
					if( clockwise )
					{
						std::vector<Vec3D>::iterator v_iter = face_vertices.begin();
						std::vector<Vec2D>::iterator t_iter = face_tex_coords.begin();
						std::vector<Vec3D>::iterator n_iter = face_normals.begin();
						v_iter ++;
						t_iter ++;
						n_iter ++;
						std::reverse( v_iter, face_vertices.end() );
						std::reverse( t_iter, face_tex_coords.end() );
						std::reverse( n_iter, face_normals.end() );
					}
					
					// Calculate a normal, in case we didn't have one specified in the model.
					if( face_vertices.size() >= 3 )
					{
						calc_normal = ( face_vertices[ 1 ] - face_vertices[ 0 ] ).Cross( face_vertices[ 2 ] - face_vertices[ 0 ] );
						calc_normal.ScaleTo( 1. );
					}
					
					// Replace any zero-length normals with our calculated one.
					for( std::vector<Vec3D>::iterator normal_iter = face_normals.begin(); normal_iter != face_normals.end(); normal_iter ++ )
					{
						if( normal_iter->Length() < 0.01 )
							normal_iter->Copy( &calc_normal );
						else
							normal_iter->ScaleTo( 1. );
					}
					
					// Add to the list.
					if( face_vertices.size() )
						faces.push_back( ModelFace( face_vertices, face_tex_coords, face_normals, smooth_group ) );
				}
				else if( elements.at( 0 ) == "o" )
				{
					std::string new_obj = (elements.size() >= 2) ? elements.at( 1 ) : "";
					
					if( (obj != new_obj) && ! faces.empty() )
					{
						// Build the previous object's arrays, since we're changing objects now.
						if( ! Objects[ obj ] )
							Objects[ obj ] = new ModelObject( obj );
						Objects[ obj ]->AddFaces( mtl, faces );
						faces.clear();
					}
					
					obj = new_obj;
				}
				else if( elements.at( 0 ) == "s" )
				{
					if( elements.size() >= 2 )
						smooth_group = atoi( elements.at( 1 ).c_str() );
					else
						smooth_group = 1;
				}
				else if( elements.at( 0 ) == "usemtl" )
				{
					std::string new_mtl = (elements.size() >= 2) ? elements.at( 1 ) : "";
					if( ! Materials[ new_mtl ] )
						Materials[ new_mtl ] = new ModelMaterial();
					
					if( (mtl != new_mtl) && ! faces.empty() )
					{
						// Build the previous object's arrays, since we're changing materials now.
						if( ! Objects[ obj ] )
							Objects[ obj ] = new ModelObject( obj );
						Objects[ obj ]->AddFaces( mtl, faces );
						faces.clear();
					}
					
					mtl = new_mtl;
				}
				else if( elements.at( 0 ) == "p" )
				{
					for( size_t i = 1; i < elements.size(); i ++ )
					{
						int vertex_num = atoi(elements.at(i).c_str());
						if( vertex_num )
						{
							if( vertex_num < 0 )
								vertex_num += vertices.size() + 1;
							if( (size_t)vertex_num > vertices.size() )
								continue;
							
							if( ! Objects[ obj ] )
								Objects[ obj ] = new ModelObject( obj );
							Objects[ obj ]->Points.push_back( vertices[ vertex_num - 1 ] );
						}
					}
				}
				else if( elements.at( 0 ) == "l" )
				{
					if( ! Objects[ obj ] )
						Objects[ obj ] = new ModelObject( obj );
					Objects[ obj ]->Lines.push_back( std::vector<Vec3D>() );
					
					for( size_t i = 1; i < elements.size(); i ++ )
					{
						int vertex_num = atoi(elements.at(i).c_str());
						if( vertex_num )
						{
							if( vertex_num < 0 )
								vertex_num += vertices.size() + 1;
							if( (size_t)vertex_num > vertices.size() )
								continue;
							
							if( ! Objects[ obj ] )
								Objects[ obj ] = new ModelObject( obj );
							Objects[ obj ]->Lines.back().push_back( vertices[ vertex_num - 1 ] );
						}
					}
				}
				else if( elements.at( 0 ) == "ccw" )  // Custom command to restore normal ccw vertex wind order.
				{
					clockwise = false;
				}
				else if( elements.at( 0 ) == "clockwise" )  // Custom command to fix models with clockwise faces.
				{
					clockwise = true;
				}
				else if( elements.at( 0 ) == "mtllib" )
				{
					if( elements.size() >= 2 )
					{
						std::string mtl_filename = elements.at( 1 );
						MaterialFiles.push_back( mtl_filename );
						
						// Use local path if possible, otherwise ues ResourceManager SearchPath.
						
						if( mtl_filename[ 0 ] == '/' )
							mtl_filename = mtl_filename.substr( 1 );
						else
						{
							std::list<std::string> path = Str::SplitToList( filename, "/" );
							path.pop_back();
							path.push_back( mtl_filename );
							std::string filename_here = Str::Join( path, "/" );
							
							if( File::Exists( filename_here.c_str() ) )
								mtl_filename = filename_here;
							else
								mtl_filename = Raptor::Game->Res.Find( mtl_filename );
						}
						
						if( mtl_filename != filename )
							IncludeOBJ( mtl_filename, get_textures );
					}
				}
				
				
				// MTL
				
				else if( elements.at( 0 ) == "newmtl" )
				{
					mtl = (elements.size() >= 2) ? elements.at( 1 ) : "";
					if( ! Materials[ mtl ] )
						Materials[ mtl ] = new ModelMaterial();
				}
				else if( (elements.at( 0 ) == "map_Kd") || (elements.at( 0 ) == "map_bump") )
				{
					if( get_textures && (elements.size() >= 2) )
					{
						std::string tex_filename = elements.at( 1 );
						
						// Use local path if possible, otherwise use ResourceManager SearchPath.
						
						if( tex_filename[ 0 ] != '/' )
						{
							std::list<std::string> path = Str::SplitToList( filename, "/" );
							path.pop_back();
							path.push_back( tex_filename );
							std::string filename_here = Str::Join( path, "/" );
							
							if( File::Exists( filename_here.c_str() ) )
								tex_filename = std::string("/") + filename_here;
						}
						
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						
						if( elements.at( 0 ) == "map_bump" )
						{
							Materials[ mtl ]->BumpMap.BecomeInstance( Raptor::Game->Res.GetAnimation( tex_filename ) );
							if( ! Materials[ mtl ]->BumpScale )
								Materials[ mtl ]->BumpScale = 1.f;
						}
						else
							Materials[ mtl ]->Texture.BecomeInstance( Raptor::Game->Res.GetAnimation( tex_filename ) );
					}
				}
				else if( elements.at( 0 ) == "bump_scale" )
				{
					if( elements.size() >= 2 )
					{
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->BumpScale = atof( elements.at( 1 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "Ka" )
				{
					if( elements.size() >= 4 )
					{
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->Ambient.Red   = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ]->Ambient.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ]->Ambient.Blue  = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "Kd" )
				{
					if( elements.size() >= 4 )
					{
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->Diffuse.Red   = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ]->Diffuse.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ]->Diffuse.Blue  = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "Ks" )
				{
					if( elements.size() >= 4 )
					{
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->Specular.Red   = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ]->Specular.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ]->Specular.Blue  = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "d" )
				{
					if( elements.size() >= 2 )
					{
						float alpha = atof( elements.at( 1 ).c_str() );
						// NOTE: Currently, only Ambient.Alpha is passed to the shaders.
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->Ambient.Alpha = alpha;
						Materials[ mtl ]->Diffuse.Alpha = alpha;
					}
				}
				else if( elements.at( 0 ) == "Ns" )
				{
					if( elements.size() >= 2 )
					{
						if( ! Materials[ mtl ] )
							Materials[ mtl ] = new ModelMaterial();
						Materials[ mtl ]->Shininess = atof( elements.at( 1 ).c_str() );
					}
				}
			}
		}
		
		if( ! faces.empty() )
		{
			// Build the final object's arrays.
			if( ! Objects[ obj ] )
				Objects[ obj ] = new ModelObject( obj );
			Objects[ obj ]->AddFaces( mtl, faces );
			faces.clear();
		}
		
		input.close();
		
		// Return true for success.
		return true;
	}
	
	// If something went wrong, return false.
	return false;
}


void Model::MakeMaterialArrays( void )
{
	// Build material vertex arrays from object vertex arrays.
	
	std::map<std::string,size_t> vertex_counts;
	
	// First count vertices for each material.
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
		{
			if( vertex_counts.find( array_iter->first ) != vertex_counts.end() )
				vertex_counts[ array_iter->first ] += array_iter->second->VertexCount;
			else
				vertex_counts[ array_iter->first ] = array_iter->second->VertexCount;
		}
	}
	
	// Allocate and fill material arrays.
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
	{
		size_t vertex_count = 0;
		if( vertex_counts.find( mtl_iter->first ) != vertex_counts.end() )
			vertex_count = vertex_counts[ mtl_iter->first ];
		
		mtl_iter->second->Arrays.Resize( vertex_count );
		
		if( vertex_count )
		{
			size_t vertices_filled = 0;
			for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
			{
				std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.find( mtl_iter->first );
				if( (array_iter != obj_iter->second->Arrays.end()) && array_iter->second->VertexCount )
				{
					memcpy( ((char*)( mtl_iter->second->Arrays.VertexArray    )) + vertices_filled * 3 * sizeof(GLdouble), array_iter->second->VertexArray,    array_iter->second->VertexCount * 3 * sizeof(GLdouble) );
					memcpy( ((char*)( mtl_iter->second->Arrays.TexCoordArray  )) + vertices_filled * 2 * sizeof(GLfloat),  array_iter->second->TexCoordArray,  array_iter->second->VertexCount * 2 * sizeof(GLfloat) );
					memcpy( ((char*)( mtl_iter->second->Arrays.NormalArray    )) + vertices_filled * 3 * sizeof(GLfloat),  array_iter->second->NormalArray,    array_iter->second->VertexCount * 3 * sizeof(GLfloat) );
					memcpy( ((char*)( mtl_iter->second->Arrays.TangentArray   )) + vertices_filled * 3 * sizeof(GLfloat),  array_iter->second->TangentArray,   array_iter->second->VertexCount * 3 * sizeof(GLfloat) );
					memcpy( ((char*)( mtl_iter->second->Arrays.BitangentArray )) + vertices_filled * 3 * sizeof(GLfloat),  array_iter->second->BitangentArray, array_iter->second->VertexCount * 3 * sizeof(GLfloat) );
					
					vertices_filled += array_iter->second->VertexCount;
				}
			}
		}
	}
}


void Model::CalculateNormals( void )
{
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second->CalculateNormals();
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second->CalculateNormals();
}


void Model::ReverseNormals( void )
{
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second->ReverseNormals();
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second->ReverseNormals();
}


void Model::SmoothNormals( void )
{
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second->SmoothNormals();
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second->SmoothNormals();
}


void Model::Optimize( double vertex_tolerance, double normal_tolerance, double dot_tolerance )
{
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second->Arrays.Optimize( vertex_tolerance, normal_tolerance, dot_tolerance );
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			array_iter->second->Optimize( vertex_tolerance, normal_tolerance, dot_tolerance );
	}
}


void Model::Draw( const Pos3D *pos, const std::set<std::string> *object_names, const Color *wireframe, double exploded, int explosion_seed, double fwd_scale, double up_scale, double right_scale )
{
	bool use_shaders = Raptor::Game->ShaderMgr.Active();
	GLint tangent_loc = -1, bitangent_loc = -1;
	if( use_shaders )
	{
		tangent_loc   = Raptor::Game->ShaderMgr.AttribLoc( "BumpTangent"   );
		bitangent_loc = Raptor::Game->ShaderMgr.AttribLoc( "BumpBitangent" );
	}
	bool use_bumpmap = (tangent_loc >= 0) && (bitangent_loc >= 0) && (Raptor::Game->Gfx.LightQuality >= 3);
	
	Pos3D zero;
	if( ! pos )
		pos = &zero;
	
	glEnableClientState( GL_VERTEX_ARRAY );
	
	if( ! wireframe )
	{
		glEnable( GL_TEXTURE_2D );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glColor4f( 1.f, 1.f, 1.f, 1.f );
	}
	else
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glColor4f( wireframe->Red, wireframe->Green, wireframe->Blue, wireframe->Alpha );
		
		if( use_shaders )
		{
			Raptor::Game->ShaderMgr.Set3f( "AmbientColor", wireframe->Red, wireframe->Green, wireframe->Blue );
			Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set3f( "SpecularColor", 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set1f( "Alpha", wireframe->Alpha );
			Raptor::Game->ShaderMgr.Set1f( "Shininess", 0. );
			Raptor::Game->ShaderMgr.Set1f( "BumpScale", 0. );
		}
	}
	
	if( (exploded <= 0.) && ! object_names )
	{
		// The model is not exploding or missing pieces, so draw per-material arrays (faster).
		
		if( use_shaders )
		{
			// In model space, X = fwd, Y = up, Z = right.
			Vec3D x_vec( pos->Fwd.X * fwd_scale, pos->Up.X * up_scale, pos->Right.X * right_scale );
			Vec3D y_vec( pos->Fwd.Y * fwd_scale, pos->Up.Y * up_scale, pos->Right.Y * right_scale );
			Vec3D z_vec( pos->Fwd.Z * fwd_scale, pos->Up.Z * up_scale, pos->Right.Z * right_scale );
			
			Raptor::Game->ShaderMgr.Set3f( "Pos", pos->X, pos->Y, pos->Z );
			Raptor::Game->ShaderMgr.Set3f( "XVec", x_vec.X, x_vec.Y, x_vec.Z );
			Raptor::Game->ShaderMgr.Set3f( "YVec", y_vec.X, y_vec.Y, y_vec.Z );
			Raptor::Game->ShaderMgr.Set3f( "ZVec", z_vec.X, z_vec.Y, z_vec.Z );
		}
		
		for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		{
			if( mtl_iter->second->Arrays.VertexCount )
			{
				if( use_shaders )
				{
					// FIXME: Pull this out of the loop, or enable different shader per material?
					if( tangent_loc >= 0 )
						glEnableVertexAttribArray( tangent_loc );
					if( bitangent_loc >= 0 )
						glEnableVertexAttribArray( bitangent_loc );
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second->Arrays.VertexArray );
					
					if( ! wireframe )
					{
						Raptor::Game->ShaderMgr.Set3f( "AmbientColor",  mtl_iter->second->Ambient.Red,  mtl_iter->second->Ambient.Green,  mtl_iter->second->Ambient.Blue );
						Raptor::Game->ShaderMgr.Set3f( "DiffuseColor",  mtl_iter->second->Diffuse.Red,  mtl_iter->second->Diffuse.Green,  mtl_iter->second->Diffuse.Blue );
						Raptor::Game->ShaderMgr.Set3f( "SpecularColor", mtl_iter->second->Specular.Red, mtl_iter->second->Specular.Green, mtl_iter->second->Specular.Blue );
						Raptor::Game->ShaderMgr.Set1f( "Alpha", mtl_iter->second->Ambient.Alpha );
						Raptor::Game->ShaderMgr.Set1f( "Shininess", mtl_iter->second->Shininess );
						
						Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
						Raptor::Game->ShaderMgr.Set1i( "BumpMap", 1 );
						
						glActiveTexture( GL_TEXTURE0 + 1 ); // BumpMap
						
						if( use_bumpmap && mtl_iter->second->BumpMap.Frames.size() )
						{
							glBindTexture( GL_TEXTURE_2D, mtl_iter->second->BumpMap.CurrentFrame() );
							glVertexAttribPointer( tangent_loc,   3, GL_FLOAT, GL_TRUE, 0, mtl_iter->second->Arrays.TangentArray );
							glVertexAttribPointer( bitangent_loc, 3, GL_FLOAT, GL_TRUE, 0, mtl_iter->second->Arrays.BitangentArray );
							Raptor::Game->ShaderMgr.Set1f( "BumpScale", mtl_iter->second->BumpScale );
						}
						else
						{
							glBindTexture( GL_TEXTURE_2D, 0 );
							if( tangent_loc >= 0 )
								glVertexAttribPointer( tangent_loc,   3, GL_FLOAT, GL_TRUE, 0, mtl_iter->second->Arrays.NormalArray );
							if( bitangent_loc >= 0 )
								glVertexAttribPointer( bitangent_loc, 3, GL_FLOAT, GL_TRUE, 0, mtl_iter->second->Arrays.NormalArray );
							Raptor::Game->ShaderMgr.Set1f( "BumpScale", 0. );
						}
						
						glActiveTexture( GL_TEXTURE0 + 0 ); // Texture
					}
				}
				else
				{
					// Calculate worldspace coordinates on the CPU (slow for complex models).
					mtl_iter->second->Arrays.MakeWorldSpace( pos, fwd_scale, up_scale, right_scale );
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second->Arrays.WorldSpaceVertexArray );
				}
				
				if( ! wireframe )
				{
					glBindTexture( GL_TEXTURE_2D, mtl_iter->second->Texture.CurrentFrame() );
					
					glTexCoordPointer( 2, GL_FLOAT, 0, mtl_iter->second->Arrays.TexCoordArray );
					glNormalPointer( GL_FLOAT, 0, mtl_iter->second->Arrays.NormalArray );
				}
				
				glDrawArrays( GL_TRIANGLES, 0, mtl_iter->second->Arrays.VertexCount );
				
				// FIXME: Pull this out of the loop, or enable different shader per material?
				if( tangent_loc >= 0 )
					glDisableVertexAttribArray( tangent_loc );
				if( bitangent_loc >= 0 )
					glDisableVertexAttribArray( bitangent_loc );
			}
		}
	}
	else
	{
		// The model is exploding, so draw per-object arrays (slower, but allows multiple positions and rotations).
		
		Pos3D draw_pos;
		Vec3D x_vec, y_vec, z_vec;
		Randomizer randomizer(explosion_seed);
		
		for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			if( object_names && (object_names->find( obj_iter->first ) == object_names->end()) )
				continue;
			
			draw_pos.Copy( pos );
			
			if( exploded > 0. )
			{
				// Convert explosion vectors to worldspace.
				Vec3D explosion_motion = obj_iter->second->GetExplosionMotion( explosion_seed, &randomizer ) * exploded;
				Vec3D modelspace_rotation_axis = obj_iter->second->GetExplosionRotationAxis( explosion_seed );
				Vec3D worldspace_rotation_axis = (pos->Fwd * modelspace_rotation_axis.X) + (pos->Up * modelspace_rotation_axis.Y) + (pos->Right * modelspace_rotation_axis.Z);
				
				draw_pos.MoveAlong( &(pos->Fwd),   explosion_motion.X * fwd_scale   );
				draw_pos.MoveAlong( &(pos->Up),    explosion_motion.Y * up_scale    );
				draw_pos.MoveAlong( &(pos->Right), explosion_motion.Z * right_scale );
				
				double explosion_rotation_rate = obj_iter->second->GetExplosionRotationRate( explosion_seed );
				draw_pos.Fwd.RotateAround( &worldspace_rotation_axis, exploded * explosion_rotation_rate );
				draw_pos.Up.RotateAround( &worldspace_rotation_axis, exploded * explosion_rotation_rate );
				draw_pos.Right.RotateAround( &worldspace_rotation_axis, exploded * explosion_rotation_rate );
			}
			
			if( use_shaders )
			{
				// In model space, X = fwd, Y = up, Z = right.
				x_vec.Set( draw_pos.Fwd.X * fwd_scale, draw_pos.Up.X * up_scale, draw_pos.Right.X * right_scale );
				y_vec.Set( draw_pos.Fwd.Y * fwd_scale, draw_pos.Up.Y * up_scale, draw_pos.Right.Y * right_scale );
				z_vec.Set( draw_pos.Fwd.Z * fwd_scale, draw_pos.Up.Z * up_scale, draw_pos.Right.Z * right_scale );
				
				Raptor::Game->ShaderMgr.Set3f( "Pos", draw_pos.X, draw_pos.Y, draw_pos.Z );
				Raptor::Game->ShaderMgr.Set3f( "XVec", x_vec.X, x_vec.Y, x_vec.Z );
				Raptor::Game->ShaderMgr.Set3f( "YVec", y_vec.X, y_vec.Y, y_vec.Z );
				Raptor::Game->ShaderMgr.Set3f( "ZVec", z_vec.X, z_vec.Y, z_vec.Z );
			}
			
			for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				if( array_iter->second->VertexCount )
				{
					if( use_shaders )
					{
						// FIXME: Pull this out of the loop, or enable different shader per material?
						if( tangent_loc >= 0 )
							glEnableVertexAttribArray( tangent_loc );
						if( bitangent_loc >= 0 )
							glEnableVertexAttribArray( bitangent_loc );
						
						if( ! wireframe )
						{
							if( ! Materials[ array_iter->first ] )
								Materials[ array_iter->first ] = new ModelMaterial();
							ModelMaterial *mtl = Materials[ array_iter->first ];
							
							Raptor::Game->ShaderMgr.Set3f( "AmbientColor",  mtl->Ambient.Red,  mtl->Ambient.Green,  mtl->Ambient.Blue );
							Raptor::Game->ShaderMgr.Set3f( "DiffuseColor",  mtl->Diffuse.Red,  mtl->Diffuse.Green,  mtl->Diffuse.Blue );
							Raptor::Game->ShaderMgr.Set3f( "SpecularColor", mtl->Specular.Red, mtl->Specular.Green, mtl->Specular.Blue );
							Raptor::Game->ShaderMgr.Set1f( "Alpha", mtl->Ambient.Alpha );
							Raptor::Game->ShaderMgr.Set1f( "Shininess", mtl->Shininess );
							
							Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
							Raptor::Game->ShaderMgr.Set1i( "BumpMap", 1 );
							
							glActiveTexture( GL_TEXTURE0 + 1 ); // BumpMap
							
							if( use_bumpmap && mtl->BumpMap.Frames.size() )
							{
								glBindTexture( GL_TEXTURE_2D, mtl->BumpMap.CurrentFrame() );
								glVertexAttribPointer( tangent_loc,   3, GL_FLOAT, GL_TRUE, 0, array_iter->second->TangentArray );
								glVertexAttribPointer( bitangent_loc, 3, GL_FLOAT, GL_TRUE, 0, array_iter->second->BitangentArray );
								Raptor::Game->ShaderMgr.Set1f( "BumpScale", mtl->BumpScale );
							}
							else
							{
								glBindTexture( GL_TEXTURE_2D, 0 );
								if( tangent_loc >= 0 )
									glVertexAttribPointer( tangent_loc,   3, GL_FLOAT, GL_TRUE, 0, array_iter->second->NormalArray );
								if( bitangent_loc >= 0 )
									glVertexAttribPointer( bitangent_loc, 3, GL_FLOAT, GL_TRUE, 0, array_iter->second->NormalArray );
								Raptor::Game->ShaderMgr.Set1f( "BumpScale", 0. );
							}
							
							glActiveTexture( GL_TEXTURE0 + 0 ); // Texture
						}
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second->VertexArray );
					}
					else
					{
						// Calculate worldspace coordinates on the CPU (slow for complex models).
						array_iter->second->MakeWorldSpace( &draw_pos, fwd_scale, up_scale, right_scale );
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second->WorldSpaceVertexArray );
					}
					
					if( ! wireframe )
					{
						if( ! Materials[ array_iter->first ] )
							Materials[ array_iter->first ] = new ModelMaterial();
						glBindTexture( GL_TEXTURE_2D, Materials[ array_iter->first ]->Texture.CurrentFrame() );
						
						glTexCoordPointer( 2, GL_FLOAT, 0, array_iter->second->TexCoordArray );
						glNormalPointer( GL_FLOAT, 0, array_iter->second->NormalArray );
					}
					
					glDrawArrays( GL_TRIANGLES, 0, array_iter->second->VertexCount );
					
					// FIXME: Pull this out of the loop, or enable different shader per material?
					if( tangent_loc >= 0 )
						glDisableVertexAttribArray( tangent_loc );
					if( bitangent_loc >= 0 )
						glDisableVertexAttribArray( bitangent_loc );
				}
			}
		}
	}
	
	if( ! wireframe )
	{
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisable( GL_TEXTURE_2D );
	}
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	
	glDisableClientState( GL_VERTEX_ARRAY );
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.Set3f( "Pos", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "XVec", 1., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "YVec", 0., 1., 0. );
		Raptor::Game->ShaderMgr.Set3f( "ZVec", 0., 0., 1. );
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1., 1., 1. );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "SpecularColor", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set1f( "Alpha", 1. );
		Raptor::Game->ShaderMgr.Set1f( "Shininess", 0. );
		Raptor::Game->ShaderMgr.Set1f( "BumpScale", 0. );
	}
}


void Model::DrawAt( const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	Draw( pos, NULL, NULL, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


void Model::DrawObjectsAt( const std::list<std::string> *object_names, const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	std::set<std::string> object_name_set;
	if( object_names )
	{
		for( std::list<std::string>::const_iterator obj_iter = object_names->begin(); obj_iter != object_names->end(); obj_iter ++ )
			object_name_set.insert( *obj_iter );
	}
	else
	{
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
			object_name_set.insert( obj_iter->first );
	}
	
	Draw( pos, &object_name_set, NULL, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


void Model::DrawWireframeAt( const Pos3D *pos, Color color, double scale, double fwd_scale, double up_scale, double right_scale )
{
	Draw( pos, NULL, &color, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


static void Model_SetHit( const std::vector< std::pair<ModelArrays*,std::string> > *arrays, const GLdouble *face, std::string *hit )
{
	// Determine the name of the model object with this face.
	if( arrays && face && hit )
	{
		for( std::vector< std::pair<ModelArrays*,std::string> >::const_iterator array_iter = arrays->begin(); array_iter != arrays->end(); array_iter ++ )
		{
			if( array_iter->first->HasWorldSpaceVertex( face ) )
			{
				*hit = array_iter->second;
				break;
			}
		}
	}
}


double Model::DistanceFromLine( const Pos3D *pos, Pos3D *nearest, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2a, const Pos3D *pos2b, double block_size ) const
{
	Vec3D motion( pos2b->X - pos2a->X, pos2b->Y - pos2a->Y, pos2b->Z - pos2a->Z );
	return DistanceFromSphere( pos, nearest, object_names, hit, exploded, explosion_seed, pos2a, &motion, 0., block_size );
}


double Model::DistanceFromSphere( const Pos3D *pos, Pos3D *nearest, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size ) const
{
	std::map< uint64_t, std::set<const GLdouble*> > blockmap;
	std::vector< std::pair<ModelArrays*,std::string> > arrays;
	Pos3D pos2b( pos2 );
	std::set<const GLdouble*> faces;
	
	if( ! block_size )
		block_size = std::max<double>( GetMaxTriangleEdge(), moved2 ? moved2->Length() : 0. );
	
	MarkBlockMap( &blockmap, &arrays, pos, object_names, exploded, explosion_seed, block_size );
	
	std::set<uint64_t> blocks;
	Math3D::BlocksInRadius( &blocks, pos2->X, pos2->Y, pos2->Z, block_size, radius );
	
	if( moved2 && moved2->Length() )
	{
		pos2b += *moved2;
		uint64_t end_index = Math3D::BlockMapIndex( pos2b.X, pos2b.Y, pos2b.Z, block_size );
		Pos3D intermediate( pos2 );
		uint64_t index = Math3D::BlockMapIndex( intermediate.X, intermediate.Y, intermediate.Z, block_size );
		
		while( index != end_index )
		{
			Vec3D diff = pos2b - intermediate;
			if( diff.Length() > block_size )
				diff.ScaleTo( block_size );
			intermediate += diff;
			
			Math3D::BlocksInRadius( &blocks, intermediate.X, intermediate.Y, intermediate.Z, block_size, radius );
			
			index = Math3D::BlockMapIndex( intermediate.X, intermediate.Y, intermediate.Z, block_size );
		}
	}
	
	for( std::set<uint64_t>::const_iterator block1 = blocks.begin(); block1 != blocks.end(); block1 ++ )
	{
		std::map< uint64_t, std::set<const GLdouble*> >::const_iterator block2 = blockmap.find( *block1 );
		if( block2 != blockmap.end() )
			faces.insert( block2->second.begin(), block2->second.end() );
	}
	
	double min_dist = FLT_MAX, min_dist_pos2 = FLT_MAX;
	const GLdouble *hit_face = NULL;
	Pos3D intersection, best_intersection;
	
	for( std::set<const GLdouble*>::const_iterator face = faces.begin(); face != faces.end(); face ++ )
	{
		double dist = Math3D::LineSegDistFromFace( pos2, &pos2b, *face, 3, &intersection );
		double dist_pos2 = Math3D::FaceCenter( *face ).Dist( pos2 );
		if( (dist < min_dist) || ((dist == min_dist) && (dist_pos2 < min_dist_pos2)) )
		{
			min_dist = dist;
			min_dist_pos2 = dist_pos2;
			hit_face = *face;
			best_intersection.Copy( &intersection );
			intersection.SetPos(0,0,0);
		}
	}
	
	Model_SetHit( &arrays, hit_face, hit );
	
	if( nearest )
		nearest->Copy( &best_intersection );
	
	for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays.begin(); array_iter != arrays.end(); array_iter ++ )
		delete array_iter->first;
	
	return min_dist;
}


bool Model::CollidesWithSphere( const Pos3D *pos, Pos3D *at, const std::set<std::string> *object_names, std::string *hit, double exploded, int explosion_seed, const Pos3D *pos2, const Vec3D *moved2, double radius, double block_size ) const
{
	return (DistanceFromSphere( pos, at, object_names, hit, exploded, explosion_seed, pos2, moved2, radius, block_size ) <= radius);
}


bool Model::CollidesWithModel( const Pos3D *pos1, Pos3D *at, const std::set<std::string> *object_names1, std::string *hit1, double exploded1, int explosion_seed1, const Model *model2, const Pos3D *pos2, const std::set<std::string> *object_names2, std::string *hit2, double exploded2, int explosion_seed2, double block_size, bool check_faces ) const
{
	return CollidesWithModel( pos1, at, object_names1, hit1, exploded1, explosion_seed1, model2, pos2, NULL, object_names2, hit2, exploded2, explosion_seed2, block_size, check_faces );
}


bool Model::CollidesWithModel( const Pos3D *pos1, Pos3D *at, const std::set<std::string> *object_names1, std::string *hit1, double exploded1, int explosion_seed1, const Model *model2, const Pos3D *pos2, const Vec3D *moved2, const std::set<std::string> *object_names2, std::string *hit2, double exploded2, int explosion_seed2, double block_size, bool check_faces ) const
{
	std::map< uint64_t, std::set<const GLdouble*> > blockmap1, blockmap2;
	std::vector< std::pair<ModelArrays*,std::string> > arrays1, arrays2;
	
	if( ! block_size )
		block_size = std::max<double>( std::max<double>( GetMaxTriangleEdge(), model2->GetMaxTriangleEdge() ), moved2 ? moved2->Length() : 0. );
	
	MarkBlockMap( &blockmap1, &arrays1, pos1, object_names1, exploded1, explosion_seed1, block_size );
	model2->MarkBlockMap( &blockmap2, &arrays2, pos2, moved2, object_names2, exploded2, explosion_seed2, block_size );
	
	for( std::map< uint64_t, std::set<const GLdouble*> >::const_iterator block1 = blockmap1.begin(); block1 != blockmap1.end(); block1 ++ )
	{
		std::map< uint64_t, std::set<const GLdouble*> >::const_iterator block2 = blockmap2.find( block1->first );
		if( block2 != blockmap2.end() )
		{
			// Both models marked the same block.
			if( ! check_faces )
			{
				for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
					delete array_iter->first;
				for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
					delete array_iter->first;
				
				return true;
			}
			
			Pos3D vertices[ 6 ], intersection;
			double line_motion[ 12 ] = {0};
			
			for( std::set<const GLdouble*>::const_iterator face1 = block1->second.begin(); face1 != block1->second.end(); face1 ++ )
			{
				for( std::set<const GLdouble*>::const_iterator face2 = block2->second.begin(); face2 != block2->second.end(); face2 ++ )
				{
					vertices[ 0 ].SetPos( (*face1)[ 0 ], (*face1)[ 1 ], (*face1)[ 2 ] );
					vertices[ 1 ].SetPos( (*face1)[ 3 ], (*face1)[ 4 ], (*face1)[ 5 ] );
					vertices[ 2 ].SetPos( (*face1)[ 6 ], (*face1)[ 7 ], (*face1)[ 8 ] );
					if( Math3D::LineIntersectsFace( &(vertices[ 0 ]), &(vertices[ 1 ]), *face2, 3, &intersection )
					||  Math3D::LineIntersectsFace( &(vertices[ 1 ]), &(vertices[ 2 ]), *face2, 3, &intersection )
					||  Math3D::LineIntersectsFace( &(vertices[ 2 ]), &(vertices[ 0 ]), *face2, 3, &intersection ) )
					{
						Model_SetHit( &arrays1, *face1, hit1 );
						Model_SetHit( &arrays2, *face2, hit2 );
						if( at )
							at->Copy( &intersection );
						
						for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
							delete array_iter->first;
						for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
							delete array_iter->first;
						
						return true;
					}
					
					vertices[ 3 ].SetPos( (*face2)[ 0 ], (*face2)[ 1 ], (*face2)[ 2 ] );
					vertices[ 4 ].SetPos( (*face2)[ 3 ], (*face2)[ 4 ], (*face2)[ 5 ] );
					vertices[ 5 ].SetPos( (*face2)[ 6 ], (*face2)[ 7 ], (*face2)[ 8 ] );
					if( Math3D::LineIntersectsFace( &(vertices[ 3 ]), &(vertices[ 4 ]), *face1, 3, &intersection )
					||  Math3D::LineIntersectsFace( &(vertices[ 4 ]), &(vertices[ 5 ]), *face1, 3, &intersection )
					||  Math3D::LineIntersectsFace( &(vertices[ 5 ]), &(vertices[ 3 ]), *face1, 3, &intersection ) )
					{
						Model_SetHit( &arrays1, *face1, hit1 );
						Model_SetHit( &arrays2, *face2, hit2 );
						if( at )
							at->Copy( &intersection );
						
						for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
							delete array_iter->first;
						for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
							delete array_iter->first;
						
						return true;
					}
					
					if( moved2 )
					{
						line_motion[  0 ] = (*face2)[ 0 ];
						line_motion[  1 ] = (*face2)[ 1 ];
						line_motion[  2 ] = (*face2)[ 2 ];
						line_motion[  3 ] = line_motion[ 0 ] + moved2->X;
						line_motion[  4 ] = line_motion[ 1 ] + moved2->Y;
						line_motion[  5 ] = line_motion[ 2 ] + moved2->Z;
						line_motion[  9 ] = (*face2)[ 3 ];
						line_motion[ 10 ] = (*face2)[ 4 ];
						line_motion[ 11 ] = (*face2)[ 5 ];
						line_motion[  6 ] = line_motion[  9 ] + moved2->X;
						line_motion[  7 ] = line_motion[ 10 ] + moved2->Y;
						line_motion[  8 ] = line_motion[ 11 ] + moved2->Z;
						if( Math3D::LineIntersectsFace( &(vertices[ 0 ]), &(vertices[ 1 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 1 ]), &(vertices[ 2 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 2 ]), &(vertices[ 0 ]), line_motion, 4, &intersection ) )
						{
							Model_SetHit( &arrays1, *face1, hit1 );
							Model_SetHit( &arrays2, *face2, hit2 );
							if( at )
								at->Copy( &intersection );
							
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
								delete array_iter->first;
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
								delete array_iter->first;
							
							return true;
						}
						
						line_motion[ 0 ] = (*face2)[ 6 ];
						line_motion[ 1 ] = (*face2)[ 7 ];
						line_motion[ 2 ] = (*face2)[ 8 ];
						line_motion[ 3 ] = line_motion[ 0 ] + moved2->X;
						line_motion[ 4 ] = line_motion[ 1 ] + moved2->Y;
						line_motion[ 5 ] = line_motion[ 2 ] + moved2->Z;
						if( Math3D::LineIntersectsFace( &(vertices[ 0 ]), &(vertices[ 1 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 1 ]), &(vertices[ 2 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 2 ]), &(vertices[ 0 ]), line_motion, 4, &intersection ) )
						{
							Model_SetHit( &arrays1, *face1, hit1 );
							Model_SetHit( &arrays2, *face2, hit2 );
							if( at )
								at->Copy( &intersection );
							
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
								delete array_iter->first;
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
								delete array_iter->first;
							
							return true;
						}
						
						line_motion[  9 ] = (*face2)[ 0 ];
						line_motion[ 10 ] = (*face2)[ 1 ];
						line_motion[ 11 ] = (*face2)[ 2 ];
						line_motion[  6 ] = line_motion[  9 ] + moved2->X;
						line_motion[  7 ] = line_motion[ 10 ] + moved2->Y;
						line_motion[  8 ] = line_motion[ 11 ] + moved2->Z;
						if( Math3D::LineIntersectsFace( &(vertices[ 0 ]), &(vertices[ 1 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 1 ]), &(vertices[ 2 ]), line_motion, 4, &intersection )
						||  Math3D::LineIntersectsFace( &(vertices[ 2 ]), &(vertices[ 0 ]), line_motion, 4, &intersection ) )
						{
							Model_SetHit( &arrays1, *face1, hit1 );
							Model_SetHit( &arrays2, *face2, hit2 );
							if( at )
								at->Copy( &intersection );
							
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
								delete array_iter->first;
							for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
								delete array_iter->first;
							
							return true;
						}
					}
				}
			}
		}
	}
	
	for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays1.begin(); array_iter != arrays1.end(); array_iter ++ )
		delete array_iter->first;
	for( std::vector< std::pair<ModelArrays*,std::string> >::iterator array_iter = arrays2.begin(); array_iter != arrays2.end(); array_iter ++ )
		delete array_iter->first;
	
	return false;
}


void Model::MarkBlockMap( std::map< uint64_t, std::set<const GLdouble*> > *blockmap, std::vector< std::pair<ModelArrays*,std::string> > *keep_arrays, const Pos3D *pos, const std::set<std::string> *object_names, double exploded, int explosion_seed, double block_size ) const
{
	return MarkBlockMap( blockmap, keep_arrays, pos, NULL, object_names, exploded, explosion_seed, block_size );
}


void Model::MarkBlockMap( std::map< uint64_t, std::set<const GLdouble*> > *blockmap, std::vector< std::pair<ModelArrays*,std::string> > *keep_arrays, const Pos3D *pos, const Vec3D *motion, const std::set<std::string> *object_names, double exploded, int explosion_seed, double block_size ) const
{
	if( ! block_size )
		block_size = std::max<double>( GetMaxTriangleEdge(), motion ? motion->Length() : 0. );
	
	Randomizer randomizer(explosion_seed);
	
	for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		if( object_names && (object_names->find( obj_iter->first ) == object_names->end()) )
			continue;
		
		Pos3D obj_pos( pos );
		
		if( exploded > 0. )
		{
			// Convert explosion vectors to worldspace.
			Vec3D explosion_motion = obj_iter->second->GetExplosionMotion( explosion_seed, &randomizer ) * exploded;
			Vec3D modelspace_rotation_axis = obj_iter->second->GetExplosionRotationAxis( explosion_seed );
			Vec3D worldspace_rotation_axis = (pos->Fwd * modelspace_rotation_axis.X) + (pos->Up * modelspace_rotation_axis.Y) + (pos->Right * modelspace_rotation_axis.Z);
			
			obj_pos.MoveAlong( &(pos->Fwd),   explosion_motion.X );
			obj_pos.MoveAlong( &(pos->Up),    explosion_motion.Y );
			obj_pos.MoveAlong( &(pos->Right), explosion_motion.Z );
			
			double explosion_rotation_rate = obj_iter->second->GetExplosionRotationRate( explosion_seed );
			obj_pos.Fwd.RotateAround(   &worldspace_rotation_axis, exploded * explosion_rotation_rate );
			obj_pos.Up.RotateAround(    &worldspace_rotation_axis, exploded * explosion_rotation_rate );
			obj_pos.Right.RotateAround( &worldspace_rotation_axis, exploded * explosion_rotation_rate );
		}
		
		for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
		{
			if( array_iter->second->VertexCount )
			{
				ModelArrays *arrays = new ModelArrays( array_iter->second );
				arrays->MakeWorldSpace( &obj_pos );  // FIXME: This is slow, and we may call MarkBlockMap multiple times per server frame!  Reuse somehow!
				keep_arrays->push_back( std::pair<ModelArrays*,std::string>( arrays, obj_iter->first ) );
				
				const GLdouble *worldspace_vertex_array = arrays->WorldSpaceVertexArray;
				size_t vertex_count = arrays->VertexCount;
				if( motion )
				{
					for( size_t i = 0; (i + 2) < vertex_count; i += 3 )
					{
						const GLdouble *triangle = &( worldspace_vertex_array[ i * 3 ] );  // Pointer to first vertex of the triangle face.
						std::set<uint64_t> blocks;
						Math3D::BlocksInTriangle( &blocks, triangle[0],triangle[1],triangle[2], triangle[3],triangle[4],triangle[5], triangle[6],triangle[7],triangle[8], block_size );
						Math3D::BlocksInTriangle( &blocks, triangle[0]+motion->X,triangle[1]+motion->Y,triangle[2]+motion->Z, triangle[3]+motion->X,triangle[4]+motion->Y,triangle[5]+motion->Z, triangle[6]+motion->X,triangle[7]+motion->Y,triangle[8]+motion->Z, block_size );
						Pos3D center = Math3D::FaceCenter( triangle, 3 );
						Math3D::BlocksInCube( &blocks, center.X,center.Y,center.Z, center.X+motion->X,center.Y+motion->Y,center.Z+motion->Z, block_size );
						Math3D::BlocksInCube( &blocks, triangle[0],triangle[1],triangle[2], triangle[0]+motion->X,triangle[1]+motion->Y,triangle[2]+motion->Z, block_size );
						Math3D::BlocksInCube( &blocks, triangle[3],triangle[4],triangle[5], triangle[3]+motion->X,triangle[4]+motion->Y,triangle[5]+motion->Z, block_size );
						Math3D::BlocksInCube( &blocks, triangle[6],triangle[7],triangle[8], triangle[6]+motion->X,triangle[7]+motion->Y,triangle[8]+motion->Z, block_size );
						for( std::set<uint64_t>::const_iterator block_iter = blocks.begin(); block_iter != blocks.end(); block_iter ++ )
							(*blockmap)[ *block_iter ].insert( triangle );
					}
				}
				else
				{
					for( size_t i = 0; (i + 2) < vertex_count; i += 3 )
					{
						const GLdouble *triangle = &( worldspace_vertex_array[ i * 3 ] );  // Pointer to first vertex of the triangle face.
						std::set<uint64_t> blocks;
						Math3D::BlocksNearTriangle( &blocks, triangle[0],triangle[1],triangle[2], triangle[3],triangle[4],triangle[5], triangle[6],triangle[7],triangle[8], block_size );
						for( std::set<uint64_t>::const_iterator block_iter = blocks.begin(); block_iter != blocks.end(); block_iter ++ )
							(*blockmap)[ *block_iter ].insert( triangle );
					}
				}
			}
		}
	}
}


void Model::Move( double fwd, double up, double right )
{
	if( !( fwd || up || right ) )
		return;
	
	BecomeCopy( this );
	
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
		{
			for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
			{
				array_iter->second->VertexArray[ i*3     ] += fwd;
				array_iter->second->VertexArray[ i*3 + 1 ] += up;
				array_iter->second->VertexArray[ i*3 + 2 ] += right;
			}
		}
		
		obj_iter->second->CenterPoint.Move( fwd, up, right );
		obj_iter->second->MinFwd   += fwd;
		obj_iter->second->MaxFwd   += fwd;
		obj_iter->second->MinUp    += up;
		obj_iter->second->MaxUp    += up;
		obj_iter->second->MinRight += right;
		obj_iter->second->MaxRight += right;
	}
	
	MakeMaterialArrays();
	
	if( MaxRadius >= 0. )
	{
		MaxRadius = 0.;
		GetMaxRadius();
	}
}


void Model::ScaleBy( double scale )
{
	ScaleBy( scale, scale, scale );
}


void Model::ScaleBy( double fwd_scale, double up_scale, double right_scale )
{
	if( (fwd_scale == 1.) && (up_scale == 1.) && (right_scale == 1.) )
		return;
	
	BecomeCopy( this );
	
	for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
		{
			for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
			{
				array_iter->second->VertexArray[ i*3     ] *= fwd_scale;
				array_iter->second->VertexArray[ i*3 + 1 ] *= up_scale;
				array_iter->second->VertexArray[ i*3 + 2 ] *= right_scale;
			}
		}
		
		for( std::vector<Vec3D>::iterator point_iter = obj_iter->second->Points.begin(); point_iter != obj_iter->second->Points.end(); point_iter ++ )
		{
			point_iter->X *= fwd_scale;
			point_iter->Y *= up_scale;
			point_iter->Z *= right_scale;
		}
		
		for( std::vector< std::vector<Vec3D> >::iterator line_iter = obj_iter->second->Lines.begin(); line_iter != obj_iter->second->Lines.end(); line_iter ++ )
		{
			for( std::vector<Vec3D>::iterator vertex_iter = line_iter->begin(); vertex_iter != line_iter->end(); vertex_iter ++ )
			{
				vertex_iter->X *= fwd_scale;
				vertex_iter->Y *= up_scale;
				vertex_iter->Z *= right_scale;
			}
		}
		
		obj_iter->second->Recalc();
	}
	
	MakeMaterialArrays();
	
	Length *= fwd_scale;
	Height *= up_scale;
	Width *= right_scale;
	
	if( MaxRadius >= 0. )
	{
		MaxRadius = 0.;
		GetMaxRadius();
	}
}


double Model::GetLength( void )
{
	if( Length <= 0. )
	{	
		double front = 0., rear = 0.;
		bool found_front = false, found_rear = false;
		
		for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
				{
					double x = array_iter->second->VertexArray[ i*3     ];
					
					if( (x > front) || ! found_front )
					{
						front = x;
						found_front = true;
					}
					
					if( (x < rear) || ! found_rear )
					{
						rear = x;
						found_rear = true;
					}
				}
			}
		}
		
		Length = front - rear;
		
		MaxFwd = front;
		MinFwd = rear;
	}
	
	return Length;
}


double Model::GetHeight( void )
{
	if( Height <= 0. )
	{
		double top = 0., bottom = 0.;
		bool found_top = false, found_bottom = false;
		
		for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
				{
					double y = array_iter->second->VertexArray[ i*3 + 1 ];
					
					if( (y > top) || ! found_top )
					{
						top = y;
						found_top = true;
					}
					
					if( (y < bottom) || ! found_bottom )
					{
						bottom = y;
						found_bottom = true;
					}
				}
			}
		}
		
		Height = top - bottom;
		
		MaxUp = top;
		MinUp = bottom;
	}
	
	return Height;
}


double Model::GetWidth( void )
{
	if( Width <= 0. )
	{
		double right = 0., left = 0.;
		bool found_right = false, found_left = false;
		
		for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
				{
					double z = array_iter->second->VertexArray[ i*3 + 2 ];
					
					if( (z > right) || ! found_right )
					{
						right = z;
						found_right = true;
					}
					
					if( (z < left) || ! found_left )
					{
						left = z;
						found_left = true;
					}
				}
			}
		}
		
		Width = right - left;
		
		MaxRight = right;
		MinRight = left;
	}
	
	return Width;
}


double Model::GetTriagonal( void )
{
	double l = GetLength(), w = GetWidth(), h = GetHeight();
	return sqrt( l*l + w*w + h*h );
}


double Model::Triagonal( void ) const
{
	return sqrt( Length*Length + Width*Width + Height*Height );
}


double Model::GetMaxDim( void )
{
	double l = GetLength(), w = GetWidth(), h = GetHeight();
	double dim = l;
	if( w > dim )
		dim = w;
	if( h > dim )
		dim = h;
	return dim;
}


double Model::GetMaxRadius( void )
{
	if( MaxRadius <= 0. )
	{
		double x = 0., y = 0., z = 0.;
		double dist = 0.;
		
		for( std::map<std::string,ModelObject*>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays*>::iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
				{
					x = array_iter->second->VertexArray[ i*3 ];
					y = array_iter->second->VertexArray[ i*3 + 1 ];
					z = array_iter->second->VertexArray[ i*3 + 2 ];
					
					dist = sqrt( x*x + y*y + z*z );
					
					if( dist > MaxRadius )
						MaxRadius = dist;
				}
			}
		}
	}
	
	return MaxRadius;
}


double Model::GetMaxTriangleEdge( void ) const
{
	double max_triangle_edge = 0.;
	
	for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		if( obj_iter->second->MaxTriangleEdge > max_triangle_edge )
			max_triangle_edge = obj_iter->second->MaxTriangleEdge;
	}
	
	return max_triangle_edge;
}


size_t Model::ArrayCount( void ) const
{
	size_t count = 0;
	
	for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		count += obj_iter->second->Arrays.size();
	
	return count;
}


size_t Model::TriangleCount( void ) const
{
	return VertexCount() / 3;
}


size_t Model::VertexCount( void ) const
{
	size_t count = 0;
	
	for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			count += array_iter->second->VertexCount;
	
	return count;
}


// ---------------------------------------------------------------------------


ModelFace::ModelFace( std::vector<Vec3D> &vertices, std::vector<Vec2D> &tex_coords, std::vector<Vec3D> &normals, int smooth_group )
{
	Vertices = vertices;
	TexCoords = tex_coords;
	Normals = normals;
	SmoothGroup = smooth_group;
}


ModelFace::~ModelFace()
{
}


// ---------------------------------------------------------------------------


ModelArrays::ModelArrays( void )
{
	VertexCount = 0;
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	TangentArray = NULL;
	BitangentArray = NULL;
	WorldSpaceVertexArray = NULL;
	Allocated = false;
	AllocatedWorldSpace = false;
}


ModelArrays::ModelArrays( const ModelArrays &other )
{
	VertexCount = 0;
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	TangentArray = NULL;
	BitangentArray = NULL;
	WorldSpaceVertexArray = NULL;
	Allocated = false;
	AllocatedWorldSpace = false;
	
	BecomeInstance( &other );
}


ModelArrays::ModelArrays( const ModelArrays *other )
{
	VertexCount = 0;
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	TangentArray = NULL;
	BitangentArray = NULL;
	WorldSpaceVertexArray = NULL;
	Allocated = false;
	AllocatedWorldSpace = false;
	
	BecomeInstance( other );
}


ModelArrays::~ModelArrays()
{
	Clear();
}


void ModelArrays::Clear( void )
{
	VertexCount = 0;
	
	if( Allocated )
	{
		if( VertexArray )
			free( VertexArray );
		if( TexCoordArray )
			free( TexCoordArray );
		if( NormalArray )
			free( NormalArray );
		if( TangentArray )
			free( TangentArray );
		if( BitangentArray )
			free( BitangentArray );
		Allocated = false;
	}
	
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	TangentArray = NULL;
	BitangentArray = NULL;
	
	// Each instance is always responsible for its own WorldSpaceVertexArray.
	if( AllocatedWorldSpace && WorldSpaceVertexArray )
		free( WorldSpaceVertexArray );
	WorldSpaceVertexArray = NULL;
	AllocatedWorldSpace = false;
}


void ModelArrays::BecomeCopy( void )
{
	BecomeCopy( this );
}


void ModelArrays::BecomeCopy( const ModelArrays *other )
{
	if( (this == other) && Allocated )
		return;
	
	// Tuck away these values before Clear in case we are copying ourself.
	size_t vertex_count             = other->VertexCount;
	const GLdouble *vertex_array    = other->VertexArray;
	const GLfloat  *tex_coord_array = other->TexCoordArray;
	const GLfloat  *normal_array    = other->NormalArray;
	const GLfloat  *tangent_array   = other->TangentArray;
	const GLfloat  *bitangent_array = other->BitangentArray;
	
	Clear();
	
	VertexCount = vertex_count;
	
	if( vertex_array )
	{
		size_t vertex_array_mem = sizeof(GLdouble) * 3 * VertexCount;
		VertexArray = (GLdouble*) malloc( vertex_array_mem );
		if( VertexArray )
		{
			memcpy( VertexArray, vertex_array, vertex_array_mem );
			Allocated = true;
		}
	}
	
	if( tex_coord_array )
	{
		size_t tex_coord_array_mem = sizeof(GLfloat) * 2 * VertexCount;
		TexCoordArray = (GLfloat*) malloc( tex_coord_array_mem );
		if( TexCoordArray )
		{
			memcpy( TexCoordArray, tex_coord_array, tex_coord_array_mem );
			Allocated = true;
		}
	}
	
	if( normal_array )
	{
		size_t normal_array_mem = sizeof(GLfloat) * 3 * VertexCount;
		NormalArray = (GLfloat*) malloc( normal_array_mem );
		if( NormalArray )
		{
			memcpy( NormalArray, normal_array, normal_array_mem );
			Allocated = true;
		}
	}
	
	if( tangent_array )
	{
		size_t tangent_array_mem = sizeof(GLfloat) * 3 * VertexCount;
		TangentArray = (GLfloat*) malloc( tangent_array_mem );
		if( TangentArray )
		{
			memcpy( TangentArray, tangent_array, tangent_array_mem );
			Allocated = true;
		}
	}
	
	if( bitangent_array )
	{
		size_t bitangent_array_mem = sizeof(GLfloat) * 3 * VertexCount;
		BitangentArray = (GLfloat*) malloc( bitangent_array_mem );
		if( BitangentArray )
		{
			memcpy( BitangentArray, bitangent_array, bitangent_array_mem );
			Allocated = true;
		}
	}
}


void ModelArrays::BecomeInstance( const ModelArrays *other )
{
	if( this != other )
	{
		Clear();
		VertexCount    = other->VertexCount;
		VertexArray    = other->VertexArray;
		TexCoordArray  = other->TexCoordArray;
		NormalArray    = other->NormalArray;
		TangentArray   = other->TangentArray;
		BitangentArray = other->BitangentArray;
	}
}


void ModelArrays::Resize( size_t vertex_count )
{
	if( vertex_count == VertexCount )
		;
	else if( vertex_count )
	{
		if( ! Allocated )
			BecomeCopy( this );
		
		VertexCount = vertex_count;
		
		size_t vertex_array_mem    = sizeof(GLdouble) * 3 * VertexCount;
		size_t tex_coord_array_mem = sizeof(GLfloat)  * 2 * VertexCount;
		size_t normal_array_mem    = sizeof(GLfloat)  * 3 * VertexCount;
		
		VertexArray    = (GLdouble*)( VertexArray    ? realloc( VertexArray,      vertex_array_mem )    : malloc( vertex_array_mem )    );
		TexCoordArray  = (GLfloat*)(  TexCoordArray  ? realloc( TexCoordArray,    tex_coord_array_mem ) : malloc( tex_coord_array_mem ) );
		NormalArray    = (GLfloat*)(  NormalArray    ? realloc( NormalArray,      normal_array_mem )    : malloc( normal_array_mem )    );
		TangentArray   = (GLfloat*)(  TangentArray   ? realloc( TangentArray,     normal_array_mem )    : malloc( normal_array_mem )    );
		BitangentArray = (GLfloat*)(  BitangentArray ? realloc( BitangentArray,   normal_array_mem )    : malloc( normal_array_mem )    );
		
		Allocated = true;
	}
	else
		Clear();
}


void ModelArrays::AddFaces( std::vector<ModelFace> &faces )
{
	std::vector<Vec3D> vertices;
	std::vector<Vec2D> tex_coords;
	std::vector<Vec3D> normals;
	std::vector<int> smooth_groups;
	
	for( std::vector<ModelFace>::iterator face_iter = faces.begin(); face_iter != faces.end(); face_iter ++ )
	{
		size_t count = 0;
		for( std::vector<Vec3D>::iterator vertex_iter = face_iter->Vertices.begin(); vertex_iter != face_iter->Vertices.end(); vertex_iter ++ )
		{
			// If it's a quad or other large shape, add copies of reused vertices first.
			if( count >= 3 )
			{
				Vec3D recent_vertex = vertices.back();
				vertices.push_back( *(face_iter->Vertices.begin()) );
				vertices.push_back( recent_vertex );
			}
			
			vertices.push_back( *vertex_iter );
			count ++;
		}
		
		count = 0;
		for( std::vector<Vec2D>::iterator tex_coord_iter = face_iter->TexCoords.begin(); tex_coord_iter != face_iter->TexCoords.end(); tex_coord_iter ++ )
		{
			// If it's a quad or other large shape, add copies for reused vertices first.
			if( count >= 3 )
			{
				Vec2D recent_tex_coord = tex_coords.back();
				tex_coords.push_back( *(face_iter->TexCoords.begin()) );
				tex_coords.push_back( recent_tex_coord );
			}
			
			tex_coords.push_back( *tex_coord_iter );
			count ++;
		}
		
		count = 0;
		for( std::vector<Vec3D>::iterator normal_iter = face_iter->Normals.begin(); normal_iter != face_iter->Normals.end(); normal_iter ++ )
		{
			// If it's a quad or other large shape, add copies for reused vertices first.
			if( count >= 3 )
			{
				Vec3D recent_normal = normals.back();
				normals.push_back( *(face_iter->Normals.begin()) );
				normals.push_back( recent_normal );
			}
			
			normals.push_back( *normal_iter );
			count ++;
		}
		
		while( smooth_groups.size() < normals.size() )
			smooth_groups.push_back( face_iter->SmoothGroup );
	}
	
	
	size_t vertices_size = vertices.size();
	if( vertices_size )
	{
		// Get rid of the old worldspace array; it will be regenerated later if necessary.
		if( AllocatedWorldSpace && WorldSpaceVertexArray )
			free( WorldSpaceVertexArray );
		WorldSpaceVertexArray = NULL;
		AllocatedWorldSpace = false;
		
		// We have vertices to add, so we'll need to increase our allocations or make new ones.
		size_t old_vertex_count = VertexCount;
		Resize( VertexCount + vertices_size );
		
		
		// Fill the new parts of the arrays.
		
		size_t tex_coords_size = tex_coords.size();
		size_t normals_size = normals.size();
		
		for( size_t i = 0; i < vertices_size; i ++ )
		{
			VertexArray[ old_vertex_count*3 + i*3     ] = vertices[ i ].X;
			VertexArray[ old_vertex_count*3 + i*3 + 1 ] = vertices[ i ].Y;
			VertexArray[ old_vertex_count*3 + i*3 + 2 ] = vertices[ i ].Z;
		}
		
		for( size_t i = 0; i < tex_coords_size; i ++ )
		{
			TexCoordArray[ old_vertex_count*2 + i*2     ] = tex_coords[ i ].X;
			TexCoordArray[ old_vertex_count*2 + i*2 + 1 ] = tex_coords[ i ].Y;
		}
		
		for( size_t i = 0; i < normals_size; i ++ )
		{
			int smooth_group = smooth_groups[ i ];
			if( (! smooth_group) || (i >= vertices_size) )
			{
				NormalArray[ old_vertex_count*3 + i*3     ] = normals[ i ].X;
				NormalArray[ old_vertex_count*3 + i*3 + 1 ] = normals[ i ].Y;
				NormalArray[ old_vertex_count*3 + i*3 + 2 ] = normals[ i ].Z;
			}
			else
			{
				std::set<Vec3D> unique;
				Vec3D normal(0,0,0);
				for( size_t j = 0; j < vertices_size; j ++ )
				{
					if( (vertices[ i ].X == vertices[ j ].X)
					&&  (vertices[ i ].Y == vertices[ j ].Y)
					&&  (vertices[ i ].Z == vertices[ j ].Z)
					&&  (j < normals_size) && (smooth_group == smooth_groups[ j ]) )
					{
						// Make sure each exact same direction is only added once to the average.
						Vec3D n( normals[ j ].X, normals[ j ].Y, normals[ j ].Z );
						if( unique.find( n ) == unique.end() )
						{
							unique.insert( n );
							normal += n;
						}
					}
				}
				normal.ScaleTo( 1. );
				NormalArray[ old_vertex_count*3 + i*3     ] = normal.X;
				NormalArray[ old_vertex_count*3 + i*3 + 1 ] = normal.Y;
				NormalArray[ old_vertex_count*3 + i*3 + 2 ] = normal.Z;
			}
		}
		
		CalculateTangents( old_vertex_count );
	}
}


void ModelArrays::RemoveFace( size_t face_index )
{
	size_t first_vertex = face_index * 3;
	if( (first_vertex + 2) >= VertexCount )
		return;
	
	if( ! Allocated )
		BecomeCopy( this );
	
	if( VertexArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			VertexArray[ i*3     ] = VertexArray[ (i+3)*3     ];
			VertexArray[ i*3 + 1 ] = VertexArray[ (i+3)*3 + 1 ];
			VertexArray[ i*3 + 2 ] = VertexArray[ (i+3)*3 + 2 ];
		}
	}
	
	if( WorldSpaceVertexArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			WorldSpaceVertexArray[ i*3     ] = WorldSpaceVertexArray[ (i+3)*3     ];
			WorldSpaceVertexArray[ i*3 + 1 ] = WorldSpaceVertexArray[ (i+3)*3 + 1 ];
			WorldSpaceVertexArray[ i*3 + 2 ] = WorldSpaceVertexArray[ (i+3)*3 + 2 ];
		}
	}
	
	if( TexCoordArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			TexCoordArray[ i*2     ] = TexCoordArray[ (i+3)*2     ];
			TexCoordArray[ i*2 + 1 ] = TexCoordArray[ (i+3)*2 + 1 ];
		}
	}
	
	if( NormalArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			NormalArray[ i*3     ] = NormalArray[ (i+3)*3     ];
			NormalArray[ i*3 + 1 ] = NormalArray[ (i+3)*3 + 1 ];
			NormalArray[ i*3 + 2 ] = NormalArray[ (i+3)*3 + 2 ];
		}
	}
	
	if( TangentArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			TangentArray[ i*3     ] = TangentArray[ (i+3)*3     ];
			TangentArray[ i*3 + 1 ] = TangentArray[ (i+3)*3 + 1 ];
			TangentArray[ i*3 + 2 ] = TangentArray[ (i+3)*3 + 2 ];
		}
	}
	
	if( BitangentArray )
	{
		for( size_t i = first_vertex; (i + 3) < VertexCount; i ++ )
		{
			BitangentArray[ i*3     ] = BitangentArray[ (i+3)*3     ];
			BitangentArray[ i*3 + 1 ] = BitangentArray[ (i+3)*3 + 1 ];
			BitangentArray[ i*3 + 2 ] = BitangentArray[ (i+3)*3 + 2 ];
		}
	}
	
	Resize( VertexCount - 3 );
}


void ModelArrays::CalculateNormals( size_t start_vertex )
{
	if( ! Allocated )
		BecomeCopy( this );
	
	Vec3D a, b, c;
	for( size_t i = start_vertex; i < VertexCount; i += 3 )
	{
		a.X = VertexArray[ i*3 + 3 ] - VertexArray[ i*3     ];
		a.Y = VertexArray[ i*3 + 4 ] - VertexArray[ i*3 + 1 ];
		a.Z = VertexArray[ i*3 + 5 ] - VertexArray[ i*3 + 2 ];
		b.X = VertexArray[ i*3 + 6 ] - VertexArray[ i*3     ];
		b.Y = VertexArray[ i*3 + 7 ] - VertexArray[ i*3 + 1 ];
		b.Z = VertexArray[ i*3 + 8 ] - VertexArray[ i*3 + 2 ];
		c = a.Cross( b );
		c.ScaleTo( 1. );
		NormalArray[ i*3     ] = c.X;
		NormalArray[ i*3 + 1 ] = c.Y;
		NormalArray[ i*3 + 2 ] = c.Z;
		NormalArray[ i*3 + 3 ] = c.X;
		NormalArray[ i*3 + 4 ] = c.Y;
		NormalArray[ i*3 + 5 ] = c.Z;
		NormalArray[ i*3 + 6 ] = c.X;
		NormalArray[ i*3 + 7 ] = c.Y;
		NormalArray[ i*3 + 8 ] = c.Z;
	}
	
	CalculateTangents( start_vertex );
}


void ModelArrays::ReverseNormals( size_t start_vertex )
{
	if( ! Allocated )
		BecomeCopy( this );
	
	Vec3D a, b, c;
	for( size_t i = start_vertex; i < VertexCount; i ++ )
	{
		NormalArray[ i*3     ] *= -1.;
		NormalArray[ i*3 + 1 ] *= -1.;
		NormalArray[ i*3 + 2 ] *= -1.;
	}
	
	CalculateTangents( start_vertex );
}


void ModelArrays::SmoothNormals( size_t start_vertex )
{
	if( ! Allocated )
		BecomeCopy( this );
	
	for( size_t i = start_vertex; i < VertexCount; i ++ )
	{
		Vec3D normal(0,0,0);
		std::set<Vec3D> unique;
		for( size_t j = start_vertex; j < VertexCount; j ++ )
		{
			if( (VertexArray[ i*3     ] == VertexArray[ j*3     ])
			&&  (VertexArray[ i*3 + 1 ] == VertexArray[ j*3 + 1 ])
			&&  (VertexArray[ i*3 + 2 ] == VertexArray[ j*3 + 2 ]) )
			{
				// Make sure each exact same direction is only added once to the average.
				Vec3D n( NormalArray[ j*3 ], NormalArray[ j*3 + 1 ], NormalArray[ j*3 + 2 ] );
				if( unique.find( n ) == unique.end() )
				{
					unique.insert( n );
					normal += n;
				}
			}
		}
		normal.ScaleTo( 1. );
		NormalArray[ i*3     ] = normal.X;
		NormalArray[ i*3 + 1 ] = normal.Y;
		NormalArray[ i*3 + 2 ] = normal.Z;
	}
	
	CalculateTangents( start_vertex );
}


void ModelArrays::CalculateTangents( size_t start_vertex )
{
	// Remain perpendicular to the normal along curved surfaces.
	Pos3D tbn;
	
	for( size_t i = start_vertex; i < VertexCount; i += 3 )
	{
		Vec3D edge1( VertexArray[ (i+1)*3 ] - VertexArray[ i*3 ], VertexArray[ (i+1)*3 + 1 ] - VertexArray[ i*3 + 1 ], VertexArray[ (i+1)*3 + 2 ] - VertexArray[ i*3 + 2 ] );
		Vec3D edge2( VertexArray[ (i+2)*3 ] - VertexArray[ i*3 ], VertexArray[ (i+2)*3 + 1 ] - VertexArray[ i*3 + 1 ], VertexArray[ (i+2)*3 + 2 ] - VertexArray[ i*3 + 2 ] );
		Vec2D deltaUV1( TexCoordArray[ (i+1)*2 ] - TexCoordArray[ i*2 ], TexCoordArray[ (i+1)*2 + 1 ] - TexCoordArray[ i*2 + 1 ] );
		Vec2D deltaUV2( TexCoordArray[ (i+2)*2 ] - TexCoordArray[ i*2 ], TexCoordArray[ (i+2)*2 + 1 ] - TexCoordArray[ i*2 + 1 ] );
		
		if( deltaUV1.X * deltaUV2.Y - deltaUV2.X * deltaUV1.Y )
		{
			for( size_t j = 0; j < 3; j ++ )
			{
				size_t v = i + j;
				tbn.Fwd.Set( NormalArray[ v*3 ], NormalArray[ v*3 + 1 ], NormalArray[ v*3 + 2 ] );
				
				// Tangent
				tbn.Up.Set( deltaUV2.Y * edge1.X - deltaUV1.Y * edge2.X, deltaUV2.Y * edge1.Y - deltaUV1.Y * edge2.Y, deltaUV2.Y * edge1.Z - deltaUV1.Y * edge2.Z );
				tbn.FixVectors();
				TangentArray[ v*3     ] = tbn.Up.X;
				TangentArray[ v*3 + 1 ] = tbn.Up.Y;
				TangentArray[ v*3 + 2 ] = tbn.Up.Z;
				
				// Bitangent
				tbn.Up.Set( deltaUV2.X * edge1.X - deltaUV1.X * edge2.X, deltaUV2.X * edge1.Y - deltaUV1.X * edge2.Y, deltaUV2.X * edge1.Z - deltaUV1.X * edge2.Z );
				tbn.FixVectors();
				BitangentArray[ v*3     ] = tbn.Up.X;
				BitangentArray[ v*3 + 1 ] = tbn.Up.Y;
				BitangentArray[ v*3 + 2 ] = tbn.Up.Z;
			}
		}
		else
		{
			// Can't bump-map without valid texture coordinates.
			TangentArray[  i   *3     ] = BitangentArray[  i   *3     ] = NormalArray[  i   *3     ];
			TangentArray[  i   *3 + 1 ] = BitangentArray[  i   *3 + 1 ] = NormalArray[  i   *3 + 1 ];
			TangentArray[  i   *3 + 2 ] = BitangentArray[  i   *3 + 2 ] = NormalArray[  i   *3 + 2 ];
			TangentArray[ (i+1)*3     ] = BitangentArray[ (i+1)*3     ] = NormalArray[ (i+1)*3     ];
			TangentArray[ (i+1)*3 + 1 ] = BitangentArray[ (i+1)*3 + 1 ] = NormalArray[ (i+1)*3 + 1 ];
			TangentArray[ (i+1)*3 + 2 ] = BitangentArray[ (i+1)*3 + 2 ] = NormalArray[ (i+1)*3 + 2 ];
			TangentArray[ (i+2)*3     ] = BitangentArray[ (i+2)*3     ] = NormalArray[ (i+2)*3     ];
			TangentArray[ (i+2)*3 + 1 ] = BitangentArray[ (i+2)*3 + 1 ] = NormalArray[ (i+2)*3 + 1 ];
			TangentArray[ (i+2)*3 + 2 ] = BitangentArray[ (i+2)*3 + 2 ] = NormalArray[ (i+2)*3 + 2 ];
		}
	}
}


void ModelArrays::Optimize( double vertex_tolerance, double normal_tolerance, double dot_tolerance )
{
	// 1. For each (unused) triangle, find others that share an edge and are on the same plane (within reason).
	// 2. Continue recurisvely off every found triangle.
	// 3. Any edge that is NOT shared with any other triangle of the group is on the outer edge of the shape.
	// 4. Get the set of all outer edges, then find any that line up.  Combine those into a new set of edges.
	// 5. From the new minimal outer edge shape, create a new minimal set of triangles to cover the shape.
	// 6. Replace original triangles with optimized shape.  (Sanity check to make sure it's actually better?)
	
	std::vector<ModelTriangle> triangles;
	for( size_t i = 0; i*3 < VertexCount; i ++ )
		triangles.push_back( ModelTriangle( this, i ) );
	
	std::vector<ModelShape> shapes;
	while( triangles.size() )
		shapes.push_back( ModelShape( &triangles, vertex_tolerance, normal_tolerance ) );
	
	std::vector<ModelFace> faces;
	for( std::vector<ModelShape>::const_iterator shape_iter = shapes.begin(); shape_iter != shapes.end(); shape_iter ++ )
	{
		// Get the set of all outer edges, then find any that line up.  Combine those into a new set of edges.
		std::vector< std::vector<ModelVertex> > edges = shape_iter->OptimizedTriangles( vertex_tolerance, normal_tolerance, dot_tolerance );
		
		// If the new shape is no more efficient than the old, just keep the old.
		if( edges.size() >= shape_iter->Triangles.size() )
		{
			for( std::vector<ModelTriangle>::const_iterator tri_iter = shape_iter->Triangles.begin(); tri_iter != shape_iter->Triangles.end(); tri_iter ++ )
			{
				std::vector<Vec3D> vertices, normals;
				std::vector<Vec2D> tex_coords;
				vertices.push_back  ( tri_iter->Vertices[ 0 ].Vertex   );
				vertices.push_back  ( tri_iter->Vertices[ 1 ].Vertex   );
				vertices.push_back  ( tri_iter->Vertices[ 2 ].Vertex   );
				tex_coords.push_back( tri_iter->Vertices[ 0 ].TexCoord );
				tex_coords.push_back( tri_iter->Vertices[ 1 ].TexCoord );
				tex_coords.push_back( tri_iter->Vertices[ 2 ].TexCoord );
				normals.push_back   ( tri_iter->Vertices[ 0 ].Normal   );
				normals.push_back   ( tri_iter->Vertices[ 1 ].Normal   );
				normals.push_back   ( tri_iter->Vertices[ 2 ].Normal   );
				faces.push_back( ModelFace( vertices, tex_coords, normals ) );
			}
			continue;
		}
		
		// From each new minimal outer edge shape, create a new minimal set of triangles to cover the shape.
		for( std::vector< std::vector<ModelVertex> >::const_iterator edge = edges.begin(); edge != edges.end(); edge ++ )
		{
			std::vector<Vec3D> vertices, normals;
			std::vector<Vec2D> tex_coords;
			for( std::vector<ModelVertex>::const_iterator vertex_iter = edge->begin(); vertex_iter != edge->end(); vertex_iter ++ )
			{
				vertices.push_back  ( vertex_iter->Vertex   );
				tex_coords.push_back( vertex_iter->TexCoord );
				normals.push_back   ( vertex_iter->Normal   );
			}
			faces.push_back( ModelFace( vertices, tex_coords, normals ) );
		}
	}
	
	// Replace original triangles with optimized shape.
	if( faces.size() )
	{
		Clear();
		AddFaces( faces );
	}
}


void ModelArrays::MakeWorldSpace( const Pos3D *pos, double fwd_scale, double up_scale, double right_scale )
{
	if( VertexCount )
	{
		// In model space, X = fwd, Y = up, Z = right.
		Vec3D x_vec( pos->Fwd.X * fwd_scale, pos->Up.X * up_scale, pos->Right.X * right_scale );
		Vec3D y_vec( pos->Fwd.Y * fwd_scale, pos->Up.Y * up_scale, pos->Right.Y * right_scale );
		Vec3D z_vec( pos->Fwd.Z * fwd_scale, pos->Up.Z * up_scale, pos->Right.Z * right_scale );
		
		// Make sure we have an array allocated for worldspace vertices.
		if( ! WorldSpaceVertexArray )
		{
			size_t vertex_array_mem = sizeof(GLdouble) * 3 * VertexCount;
			WorldSpaceVertexArray = (GLdouble*) malloc( vertex_array_mem );
			AllocatedWorldSpace = WorldSpaceVertexArray;
		}
		
		// Translate from modelspace to worldspace.
		if( WorldSpaceVertexArray )
		{
			for( size_t i = 0; i < VertexCount; i ++ )
			{
				Vec3D vertex( VertexArray[ i*3 ], VertexArray[ i*3 + 1 ], VertexArray[ i*3 + 2 ] );
				WorldSpaceVertexArray[ i*3     ] = pos->X + x_vec.Dot(&vertex);
				WorldSpaceVertexArray[ i*3 + 1 ] = pos->Y + y_vec.Dot(&vertex);
				WorldSpaceVertexArray[ i*3 + 2 ] = pos->Z + z_vec.Dot(&vertex);
			}
		}
	}
}


bool ModelArrays::HasWorldSpaceVertex( const GLdouble *vertex ) const
{
	return WorldSpaceVertexArray && (vertex >= WorldSpaceVertexArray) && (vertex < WorldSpaceVertexArray + VertexCount * 3);
}


// ---------------------------------------------------------------------------


bool ModelVertex::operator < ( const ModelVertex &other ) const
{
	return (Vertex < other.Vertex);
}


// ---------------------------------------------------------------------------


ModelTriangle::ModelTriangle( const ModelArrays *arrays, size_t face_index )
{
	FaceIndex = face_index;
	
	if( arrays->VertexArray )
	{
		Vertices[ 0 ].Vertex.Set( arrays->VertexArray[ face_index * 9     ], arrays->VertexArray[ face_index * 9 + 1 ], arrays->VertexArray[ face_index * 9 + 2 ] );
		Vertices[ 1 ].Vertex.Set( arrays->VertexArray[ face_index * 9 + 3 ], arrays->VertexArray[ face_index * 9 + 4 ], arrays->VertexArray[ face_index * 9 + 5 ] );
		Vertices[ 2 ].Vertex.Set( arrays->VertexArray[ face_index * 9 + 6 ], arrays->VertexArray[ face_index * 9 + 7 ], arrays->VertexArray[ face_index * 9 + 8 ] );
	}
	if( arrays->TexCoordArray )
	{
		Vertices[ 0 ].TexCoord.Set( arrays->TexCoordArray[ face_index * 6     ], arrays->TexCoordArray[ face_index * 6 + 1 ] );
		Vertices[ 1 ].TexCoord.Set( arrays->TexCoordArray[ face_index * 6 + 2 ], arrays->TexCoordArray[ face_index * 6 + 3 ] );
		Vertices[ 2 ].TexCoord.Set( arrays->TexCoordArray[ face_index * 6 + 4 ], arrays->TexCoordArray[ face_index * 6 + 5 ] );
	}
	if( arrays->NormalArray )
	{
		Vertices[ 0 ].Normal.Set( arrays->NormalArray[ face_index * 9     ], arrays->NormalArray[ face_index * 9 + 1 ], arrays->NormalArray[ face_index * 9 + 2 ] );
		Vertices[ 1 ].Normal.Set( arrays->NormalArray[ face_index * 9 + 3 ], arrays->NormalArray[ face_index * 9 + 4 ], arrays->NormalArray[ face_index * 9 + 5 ] );
		Vertices[ 2 ].Normal.Set( arrays->NormalArray[ face_index * 9 + 6 ], arrays->NormalArray[ face_index * 9 + 7 ], arrays->NormalArray[ face_index * 9 + 8 ] );
	}
	
	Vec3D a = Vertices[ 1 ].Vertex - Vertices[ 0 ].Vertex, b = Vertices[ 2 ].Vertex - Vertices[ 0 ].Vertex;
	CalculatedNormal = a.Cross( b );
	CalculatedNormal.ScaleTo( 1. );
	int8_t normal_dir = Num::Sign( Vertices[ 0 ].Normal.Dot( &CalculatedNormal ) )
	                  + Num::Sign( Vertices[ 1 ].Normal.Dot( &CalculatedNormal ) )
	                  + Num::Sign( Vertices[ 2 ].Normal.Dot( &CalculatedNormal ) );
	if( normal_dir < 0 )
		CalculatedNormal.ScaleBy( -1. );
}


uint8_t ModelTriangle::SharesEdge( const ModelTriangle *other, double vertex_tolerance, double normal_tolerance, const Vec3D *normal ) const
{
	if( ! normal )
		normal = &CalculatedNormal;
	if( (*normal - other->CalculatedNormal).Length() > normal_tolerance )
		return 0;
	
	bool corners[ 3 ] = { false, false, false };
	for( size_t i = 0; i < 3; i ++ )
	{
		for( size_t j = 0; j < 3; j ++ )
		{
			if( (Vertices[ i ].Vertex - other->Vertices[ j ].Vertex).Length() <= vertex_tolerance )
			{
				corners[ i ] = true;
				break;
			}
		}
	}
	
	if( corners[ 0 ] && corners[ 1 ] && corners[ 2 ] )
		return 0x7;
	if( corners[ 0 ] && corners[ 1 ] )
		return 0x1;
	if( corners[ 1 ] && corners[ 2 ] )
		return 0x2;
	if( corners[ 2 ] && corners[ 0 ] )
		return 0x4;
	return 0;
}


// ---------------------------------------------------------------------------


ModelEdge::ModelEdge( const ModelVertex &v1, const ModelVertex &v2, const Vec3D &u, const Vec3D &v, bool outside )
{
	Vertices[ 0 ] = v1;
	Vertices[ 1 ] = v2;
	OnPlane[ 0 ].X = v1.Vertex.Dot( u );
	OnPlane[ 0 ].Y = v1.Vertex.Dot( v );
	OnPlane[ 1 ].X = v2.Vertex.Dot( u );
	OnPlane[ 1 ].Y = v2.Vertex.Dot( v );
	Outside = outside;
}


bool ModelEdge::Intersects( const ModelEdge &other ) const
{
	// Consider them intersecting if they share both vertices.
	if( ((OnPlane[ 0 ] == other.OnPlane[ 0 ]) && (OnPlane[ 1 ] == other.OnPlane[ 1 ])) || ((OnPlane[ 0 ] == other.OnPlane[ 1 ]) && (OnPlane[ 1 ] == other.OnPlane[ 0 ])) )
		return true;
	
	// DON'T consider them intersecting if they share only one vertex.
	if( (OnPlane[ 0 ] == other.OnPlane[ 0 ]) || (OnPlane[ 0 ] == other.OnPlane[ 1 ]) || (OnPlane[ 1 ] == other.OnPlane[ 0 ]) || (OnPlane[ 1 ] == other.OnPlane[ 1 ]) )
		return false;
	
	return Math2D::LineIntersection( OnPlane[ 0 ].X, OnPlane[ 0 ].Y, OnPlane[ 1 ].X, OnPlane[ 1 ].Y, other.OnPlane[ 0 ].X, other.OnPlane[ 0 ].Y, other.OnPlane[ 1 ].X, other.OnPlane[ 1 ].Y );
}


bool ModelEdge::Intersects( const Vec2D &end1, const Vec2D &end2 ) const
{
	return Math2D::LineIntersection( OnPlane[ 0 ].X, OnPlane[ 0 ].Y, OnPlane[ 1 ].X, OnPlane[ 1 ].Y, end1.X, end1.Y, end2.X, end2.Y );
}


// ---------------------------------------------------------------------------


ModelShape::ModelShape( std::vector<ModelTriangle> *triangles, double vertex_tolerance, double normal_tolerance )
{
	if( triangles && ! triangles->empty() )
	{
		Normal = triangles->begin()->CalculatedNormal;
		
		AddTriangles( *(triangles->begin()), triangles, vertex_tolerance, normal_tolerance );
		
		Min = Triangles.begin()->Vertices[ 0 ].Vertex;
		Max = Min;
		for( std::vector<ModelTriangle>::const_iterator tri_iter = Triangles.begin(); tri_iter != Triangles.end(); tri_iter ++ )
		{
			if( Min.X > tri_iter->Vertices[ 0 ].Vertex.X )
				Min.X = tri_iter->Vertices[ 0 ].Vertex.X;
			if( Min.Y > tri_iter->Vertices[ 0 ].Vertex.Y )
				Min.Y = tri_iter->Vertices[ 0 ].Vertex.Y;
			if( Min.Z > tri_iter->Vertices[ 0 ].Vertex.Z )
				Min.Z = tri_iter->Vertices[ 0 ].Vertex.Z;
			if( Max.X < tri_iter->Vertices[ 0 ].Vertex.X )
				Max.X = tri_iter->Vertices[ 0 ].Vertex.X;
			if( Max.Y < tri_iter->Vertices[ 0 ].Vertex.Y )
				Max.Y = tri_iter->Vertices[ 0 ].Vertex.Y;
			if( Max.Z < tri_iter->Vertices[ 0 ].Vertex.Z )
				Max.Z = tri_iter->Vertices[ 0 ].Vertex.Z;
			
			if( Min.X > tri_iter->Vertices[ 1 ].Vertex.X )
				Min.X = tri_iter->Vertices[ 1 ].Vertex.X;
			if( Min.Y > tri_iter->Vertices[ 1 ].Vertex.Y )
				Min.Y = tri_iter->Vertices[ 1 ].Vertex.Y;
			if( Min.Z > tri_iter->Vertices[ 1 ].Vertex.Z )
				Min.Z = tri_iter->Vertices[ 1 ].Vertex.Z;
			if( Max.X < tri_iter->Vertices[ 1 ].Vertex.X )
				Max.X = tri_iter->Vertices[ 1 ].Vertex.X;
			if( Max.Y < tri_iter->Vertices[ 1 ].Vertex.Y )
				Max.Y = tri_iter->Vertices[ 1 ].Vertex.Y;
			if( Max.Z < tri_iter->Vertices[ 1 ].Vertex.Z )
				Max.Z = tri_iter->Vertices[ 1 ].Vertex.Z;
			
			if( Min.X > tri_iter->Vertices[ 2 ].Vertex.X )
				Min.X = tri_iter->Vertices[ 2 ].Vertex.X;
			if( Min.Y > tri_iter->Vertices[ 2 ].Vertex.Y )
				Min.Y = tri_iter->Vertices[ 2 ].Vertex.Y;
			if( Min.Z > tri_iter->Vertices[ 2 ].Vertex.Z )
				Min.Z = tri_iter->Vertices[ 2 ].Vertex.Z;
			if( Max.X < tri_iter->Vertices[ 2 ].Vertex.X )
				Max.X = tri_iter->Vertices[ 2 ].Vertex.X;
			if( Max.Y < tri_iter->Vertices[ 2 ].Vertex.Y )
				Max.Y = tri_iter->Vertices[ 2 ].Vertex.Y;
			if( Max.Z < tri_iter->Vertices[ 2 ].Vertex.Z )
				Max.Z = tri_iter->Vertices[ 2 ].Vertex.Z;
		}
		Center = (Min + Max) / 2.;
	}
}


void ModelShape::AddTriangles( const ModelTriangle triangle, std::vector<ModelTriangle> *triangles, double vertex_tolerance, double normal_tolerance )
{
	Triangles.push_back( triangle );
	
	// For each (unmarked) triangle, find others that share an edge and are on the same plane (within reason).
	std::vector<ModelTriangle> matches;
	for( size_t i = 0; i < triangles->size(); i ++ )
	{
		const ModelTriangle *other_triangle = &((*triangles)[ i ]);
		if( (triangle.Vertices[ 0 ].Vertex == other_triangle->Vertices[ 0 ].Vertex)
		&&  (triangle.Vertices[ 1 ].Vertex == other_triangle->Vertices[ 1 ].Vertex)
		&&  (triangle.Vertices[ 2 ].Vertex == other_triangle->Vertices[ 2 ].Vertex) )
		{
			triangles->erase( triangles->begin() + i );
			i --;
		}
		else if( triangle.SharesEdge( other_triangle, vertex_tolerance, normal_tolerance, &Normal ) )
			matches.push_back( *other_triangle );
	}
	
	// Continue recurisvely off every found triangle.
	for( std::vector<ModelTriangle>::const_iterator match = matches.begin(); match != matches.end(); match ++ )
		AddTriangles( *match, triangles, vertex_tolerance, normal_tolerance );
}


std::vector< std::vector<ModelVertex> > ModelShape::AllEdges( double vertex_tolerance, double normal_tolerance ) const
{
	// Any edge that is NOT shared with any other triangle of the group is on the outer edge of the shape.
	std::set< std::pair<ModelVertex,ModelVertex> > edges;
	for( std::vector<ModelTriangle>::const_iterator tri_iter1 = Triangles.begin(); tri_iter1 != Triangles.end(); tri_iter1 ++ )
	{
		// FIXME: Does this correctly handle overlapping triangles along the edge of a shape?  (Should I even care if it does?)
		uint8_t shared_edges = 0;
		for( std::vector<ModelTriangle>::const_iterator tri_iter2 = Triangles.begin(); tri_iter2 != Triangles.end(); tri_iter2 ++ )
		{
			if( tri_iter1->FaceIndex != tri_iter2->FaceIndex )
				shared_edges |= tri_iter1->SharesEdge( &*tri_iter2, vertex_tolerance, normal_tolerance, &Normal );
		}
		
		if( (Triangles.size() > 1) && ! shared_edges ) // Sanity check: There should never be a lone triangle in a contiguous shape.
			continue;
		
		if( !(shared_edges & 0x1) )
			edges.insert( std::pair<ModelVertex,ModelVertex>( tri_iter1->Vertices[ 0 ], tri_iter1->Vertices[ 1 ] ) );
		if( !(shared_edges & 0x2) )
			edges.insert( std::pair<ModelVertex,ModelVertex>( tri_iter1->Vertices[ 1 ], tri_iter1->Vertices[ 2 ] ) );
		if( !(shared_edges & 0x4) )
			edges.insert( std::pair<ModelVertex,ModelVertex>( tri_iter1->Vertices[ 2 ], tri_iter1->Vertices[ 0 ] ) );
	}
	
	// Sort the edge vertices going around the outside of the shape.
	std::vector< std::vector<ModelVertex> > shapes;
	shapes.push_back( std::vector<ModelVertex>() );
	if( edges.size() )
	{
		shapes.back().push_back( edges.begin()->first );
		shapes.back().push_back( edges.begin()->second );
		edges.erase( edges.begin() );
		
		while( edges.size() )
		{
			double min_dist = FLT_MAX;
			std::set< std::pair<ModelVertex,ModelVertex> >::iterator next_edge = edges.begin();
			bool reverse = false;
			
			for( std::set< std::pair<ModelVertex,ModelVertex> >::iterator edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter ++ )
			{
				double dist1 = (edge_iter->first.Vertex  - shapes.back().back().Vertex).Length();
				double dist2 = (edge_iter->second.Vertex - shapes.back().back().Vertex).Length();
				if( dist1 < min_dist )
				{
					min_dist = dist1;
					next_edge = edge_iter;
					reverse = false;
				}
				if( dist2 < min_dist )
				{
					min_dist = dist2;
					next_edge = edge_iter;
					reverse = true;
				}
			}
			
			const ModelVertex *first  = &(reverse ? next_edge->second : next_edge->first);
			const ModelVertex *second = &(reverse ? next_edge->first : next_edge->second);
			
			if( min_dist > vertex_tolerance )
			{
				// There are hollow spaces in the shape that create multiple distinct edges.
				shapes.push_back( std::vector<ModelVertex>() );
				shapes.back().push_back( *first );
			}
			if( (first->Vertex - second->Vertex).Length() > vertex_tolerance )
				shapes.back().push_back( *second );
			edges.erase( next_edge );
		}
	}
	return shapes;
}


std::vector<ModelVertex> ModelShape::OptimizeEdge( std::vector<ModelVertex> &vertices, double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const
{
	if( vertices.size() >= 4 )
	{
		// Find any that line up, and remove the middle ones.
		for( size_t i = 1; i + 1 < vertices.size(); i ++ )
		{
			Vec3D edge1 = (vertices[ i + 1 ].Vertex - vertices[ i ].Vertex).Unit();
			Vec3D edge2 = (vertices[ i ].Vertex - vertices[ i - 1 ].Vertex).Unit();
			
			if( edge1.Dot( &edge2 ) >= (1. - dot_tolerance) )
			{
				vertices.erase( vertices.begin() + i );
				i --;
			}
		}
	}
	
	// Remove the last vertex if it's the same as the first (it probably is).
	if( (vertices.size() >= 4) && ((vertices.front().Vertex - vertices.back().Vertex).Length() <= vertex_tolerance) )
		vertices.pop_back();
	
	return vertices;
}


std::vector< std::vector<ModelVertex> > ModelShape::AllOptimizedEdges( double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const
{
	std::vector< std::vector<ModelVertex> > all_edges = AllEdges( vertex_tolerance, normal_tolerance );
	
	for( std::vector< std::vector<ModelVertex> >::iterator shape_iter = all_edges.begin(); shape_iter != all_edges.end(); shape_iter ++ )
		OptimizeEdge( *shape_iter, vertex_tolerance, normal_tolerance, dot_tolerance );
	
	return all_edges;
}


std::vector< std::vector<ModelVertex> > ModelShape::OptimizedTriangles( double vertex_tolerance, double normal_tolerance, double dot_tolerance ) const
{
	std::vector< std::vector<ModelVertex> > shapes;
	if( ! Triangles.size() )
		return shapes;
	
	std::vector< std::vector<ModelVertex> > all_edges = AllOptimizedEdges( vertex_tolerance, normal_tolerance, dot_tolerance );
	
	// No need to make a new triangulation if we already have just one triangle!  (And it would fail in my algorithm.)
	if( (all_edges.size() == 1) && (all_edges[ 0 ].size() <= 3) )
	{
		bool reverse = false;
		if( all_edges[ 0 ].size() == 3 )
		{
			// Make sure the winding is counter-clockwise.
			Vec3D a = all_edges[ 0 ][ 1 ].Vertex - all_edges[ 0 ][ 0 ].Vertex, b = all_edges[ 0 ][ 2 ].Vertex - all_edges[ 0 ][ 0 ].Vertex;
			Vec3D normal = a.Cross( b ).Unit();
			int8_t normal_dir = Num::Sign( all_edges[ 0 ][ 0 ].Normal.Dot( &normal ) )
			                  + Num::Sign( all_edges[ 0 ][ 1 ].Normal.Dot( &normal ) )
			                  + Num::Sign( all_edges[ 0 ][ 2 ].Normal.Dot( &normal ) );
			reverse = (normal_dir < 0);
		}
		
		shapes.push_back( std::vector<ModelVertex>() );
		if( reverse )
		{
			shapes.back().push_back( all_edges[ 0 ][ 2 ] );
			shapes.back().push_back( all_edges[ 0 ][ 1 ] );
			shapes.back().push_back( all_edges[ 0 ][ 0 ] );
		}
		else
		{
			for( std::vector<ModelVertex>::const_iterator vertex_iter = all_edges[ 0 ].begin(); vertex_iter != all_edges[ 0 ].end(); vertex_iter ++ )
				shapes.back().push_back( *vertex_iter );
		}
		return shapes;
	}
	
	Vec3D u = Normal.Cross( Triangles.begin()->Vertices[ 1 ].Vertex - Triangles.begin()->Vertices[ 0 ].Vertex ).Unit();
	Vec3D v = Normal.Cross( u ).Unit();
	
	Vec3D outside_3d( Max.X + 100., Max.Y + 200., Max.Z + 300. );
	Vec2D outside( outside_3d.Dot( u ), outside_3d.Dot( v ) );
	
	// Combine all the vertices and edges from the separate shapes (in case of hollow sections).
	std::vector<ModelVertex> vertices;
	std::vector<ModelEdge> edges;
	for( std::vector< std::vector<ModelVertex> >::iterator shape_iter = all_edges.begin(); shape_iter != all_edges.end(); shape_iter ++ )
	{
		size_t first_shape_vertex = vertices.size();
		
		for( size_t i = 0; i < shape_iter->size(); i ++ )
			vertices.push_back( shape_iter->at( i ) );
		
		for( size_t i = 0; i < shape_iter->size(); i ++ )
		{
			size_t j = i + 1;
			if( j >= vertices.size() )
				j = first_shape_vertex;
			
			edges.push_back( ModelEdge( vertices[ i ], vertices[ j ], u, v, true ) );
			vertices[ i ].EdgeIndices.insert( edges.size() - 1 );
			vertices[ j ].EdgeIndices.insert( edges.size() - 1 );
		}
	}
	
	// Add edges wherever possible without crossing another edge to triangulate the shape.
	for( size_t i = 0; i < vertices.size(); i ++ )
	{
		for( size_t j = i + 1; j < vertices.size(); j ++ )
		{
			ModelEdge new_edge( vertices[ i ], vertices[ j ], u, v, false );
			bool intersection = false;
			for( size_t k = 0; k < edges.size(); k ++ )
			{
				if( new_edge.Intersects( edges[ k ] ) )
				{
					intersection = true;
					break;
				}
			}
			
			if( ! intersection )
			{
				// Make sure this new line is within the original shape.
				Vec2D mid = (new_edge.OnPlane[ 0 ] + new_edge.OnPlane[ 1 ]) / 2.;
				size_t crossings = 0;
				for( size_t k = 0; k < edges.size(); k ++ )
				{
					if( ! edges[ k ].Outside )
						break;
					if( edges[ k ].Intersects( outside, mid ) )
						crossings ++;
				}
				
				// Make sure we don't add triangles outside the shape.
				if( (crossings % 2) == 0 )
					continue;
				
				// Whenever we complete a triangle, add it as a new shape.
				for( std::set<size_t>::const_iterator i_iter = vertices[ i ].EdgeIndices.begin(); i_iter != vertices[ i ].EdgeIndices.end(); i_iter ++ )
				{
					for( std::set<size_t>::const_iterator j_iter = vertices[ j ].EdgeIndices.begin(); j_iter != vertices[ j ].EdgeIndices.end(); j_iter ++ )
					{
						if( *i_iter == *j_iter )
							continue;
						std::set<ModelVertex> tri_vertices;
						tri_vertices.insert( vertices[ i ] );
						tri_vertices.insert( vertices[ j ] );
						tri_vertices.insert( edges[ *i_iter ].Vertices[ 0 ] );
						tri_vertices.insert( edges[ *i_iter ].Vertices[ 1 ] );
						tri_vertices.insert( edges[ *j_iter ].Vertices[ 0 ] );
						tri_vertices.insert( edges[ *j_iter ].Vertices[ 1 ] );
						tri_vertices.erase( vertices[ i ] );
						tri_vertices.erase( vertices[ j ] );
						if( tri_vertices.size() == 1 )
						{
							// Make sure the winding is counter-clockwise.
							Vec3D a = vertices[ i ].Vertex - vertices[ j ].Vertex, b = tri_vertices.begin()->Vertex - vertices[ j ].Vertex;
							Vec3D normal = a.Cross( b ).Unit();
							int8_t normal_dir = Num::Sign( vertices[ i ].Normal.Dot( &normal ) )
							                  + Num::Sign( vertices[ j ].Normal.Dot( &normal ) )
							                  + Num::Sign( tri_vertices.begin()->Normal.Dot( &normal ) );
							bool reverse = (normal_dir < 0);
							
							shapes.push_back( std::vector<ModelVertex>() );
							if( reverse )
							{
								shapes.back().push_back( vertices[ i ] );
								shapes.back().push_back( vertices[ j ] );
							}
							else
							{
								shapes.back().push_back( vertices[ j ] );
								shapes.back().push_back( vertices[ i ] );
							}
							shapes.back().push_back( *(tri_vertices.begin()) );
						}
					}
				}
				
				edges.push_back( new_edge );
				vertices[ i ].EdgeIndices.insert( edges.size() - 1 );
				vertices[ j ].EdgeIndices.insert( edges.size() - 1 );
			}
		}
	}
	
	return shapes;
}


// ---------------------------------------------------------------------------


Randomizer ModelObject::GlobalRandomizer;


ModelObject::ModelObject( void )
{
	CenterPoint.SetPos( 0., 0., 0. );
	MinFwd = MaxFwd = MinUp = MaxUp = MinRight = MaxRight = MaxRadius = MaxTriangleEdge = 0.;
	NeedsRecalc = false;
}


ModelObject::ModelObject( const std::string &name )
{
	CenterPoint.SetPos( 0., 0., 0. );
	MinFwd = MaxFwd = MinUp = MaxUp = MinRight = MaxRight = MaxRadius = MaxTriangleEdge = 0.;
	NeedsRecalc = false;
	
	Name = name;
}


ModelObject::ModelObject( const ModelObject &other )
{
	BecomeInstance( &other );
}


ModelObject::ModelObject( const ModelObject *other )
{
	BecomeInstance( other );
}


ModelObject::~ModelObject()
{
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		delete array_iter->second;
	Arrays.clear();
}


void ModelObject::BecomeInstance( const ModelObject *other )
{
	if( this == other )  // Prevent clobbering Arrays if I do something dumb.
		return;
	
	Name = other->Name;
	
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		delete array_iter->second;
	Arrays.clear();
	
	for( std::map<std::string,ModelArrays*>::const_iterator array_iter = other->Arrays.begin(); array_iter != other->Arrays.end(); array_iter ++ )
		Arrays[ array_iter->first ] = new ModelArrays( array_iter->second );
	
	Points = other->Points;
	Lines = other->Lines;
	
	CenterPoint = other->CenterPoint;
	MinFwd = other->MinFwd;
	MaxFwd = other->MaxFwd;
	MinUp = other->MinUp;
	MaxUp = other->MaxUp;
	MinRight = other->MinRight;
	MaxRight = other->MaxRight;
	MaxRadius = other->MaxRadius;
	MaxTriangleEdge = other->MaxTriangleEdge;
	
	NeedsRecalc = other->NeedsRecalc;
}


void ModelObject::AddFaces( std::string mtl, std::vector<ModelFace> &faces )
{
	if( ! Arrays[ mtl ] )
		Arrays[ mtl ] = new ModelArrays();
	Arrays[ mtl ]->AddFaces( faces );
	
	NeedsRecalc = true;
}


void ModelObject::Recalc( void )
{
	CenterPoint.SetPos( 0., 0., 0. );
	MaxRadius = 0.;
	double max_triangle_edge_squared = 0.;
	size_t vertex_count = 0;

	MinFwd = FLT_MAX;
	MinUp = FLT_MAX;
	MinRight = FLT_MAX;
	MaxFwd = -FLT_MAX;
	MaxUp = -FLT_MAX;
	MaxRight = -FLT_MAX;
	
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
	{
		vertex_count += array_iter->second->VertexCount;
		
		for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
		{
			// In model space, X = fwd, Y = up, Z = right.
			double x = array_iter->second->VertexArray[ i*3     ];
			double y = array_iter->second->VertexArray[ i*3 + 1 ];
			double z = array_iter->second->VertexArray[ i*3 + 2 ];
			
			if( x < MinFwd )
				MinFwd = x;
			if( x > MaxFwd )
				MaxFwd = x;
			
			if( y < MinUp )
				MinUp = y;
			if( y > MaxUp )
				MaxUp = y;
			
			if( z < MinRight )
				MinRight = z;
			if( z > MaxRight )
				MaxRight = z;
			
			if( (i % 3) == 2 )
			{
				// Check each triangle for edge lengths.
				double x1 = array_iter->second->VertexArray[ i*3 - 6 ];
				double y1 = array_iter->second->VertexArray[ i*3 - 5 ];
				double z1 = array_iter->second->VertexArray[ i*3 - 4 ];
				double x2 = array_iter->second->VertexArray[ i*3 - 3 ];
				double y2 = array_iter->second->VertexArray[ i*3 - 2 ];
				double z2 = array_iter->second->VertexArray[ i*3 - 1 ];
				double len_sq = (x1-x) * (x1-x) + (y1-y) * (y1-y) + (z1-z) * (z1-z);
				if( len_sq > max_triangle_edge_squared )
					max_triangle_edge_squared = len_sq;
				len_sq        = (x2-x) * (x2-x) + (y2-y) * (y2-y) + (z2-z) * (z2-z);
				if( len_sq > max_triangle_edge_squared )
					max_triangle_edge_squared = len_sq;
				len_sq        = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
				if( len_sq > max_triangle_edge_squared )
					max_triangle_edge_squared = len_sq;
			}
		}
	}
	
	if( vertex_count )
	{
		// In model space, X = fwd, Y = up, Z = right.
		CenterPoint.X = (MinFwd + MaxFwd) / 2.;
		CenterPoint.Y = (MinUp + MaxUp) / 2.;
		CenterPoint.Z = (MinRight + MaxRight) / 2.;
		MaxTriangleEdge = sqrt(max_triangle_edge_squared);
		
		for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		{
			for( size_t i = 0; i < array_iter->second->VertexCount; i ++ )
			{
				double x = array_iter->second->VertexArray[ i*3     ] - CenterPoint.X;
				double y = array_iter->second->VertexArray[ i*3 + 1 ] - CenterPoint.Y;
				double z = array_iter->second->VertexArray[ i*3 + 2 ] - CenterPoint.Z;
				double dist = sqrt( x*x + y*y + z*z );
				if( dist > MaxRadius )
					MaxRadius = dist;
			}
		}
	}
	else
	{
		MinFwd = 0.;
		MinUp = 0.;
		MinRight = 0.;
		MaxFwd = 0.;
		MaxUp = 0.;
		MaxRight = 0.;
		MaxTriangleEdge = 0.;
	}
	
	NeedsRecalc = false;
}


void ModelObject::CalculateNormals( void )
{
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second->CalculateNormals();
}


void ModelObject::ReverseNormals( void )
{
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second->ReverseNormals();
}


void ModelObject::SmoothNormals( void )
{
	for( std::map<std::string,ModelArrays*>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second->SmoothNormals();
}


Pos3D ModelObject::GetCenterPoint( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return CenterPoint;
}


double ModelObject::GetLength( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return MaxFwd - MinFwd;
}


double ModelObject::GetHeight( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return MaxUp - MinUp;
}


double ModelObject::GetWidth( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return MaxRight - MinRight;
}


double ModelObject::GetMaxRadius( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return MaxRadius;
}


Vec3D ModelObject::GetExplosionMotion( int seed, Randomizer *randomizer ) const
{
	// Generate a per-object seed based on the object name.
	seed *= Name.length();
	for( size_t i = 0; i < Name.length(); i ++ )
		seed += (i + 1) * (int)( Name[ i ] );
	seed = abs(seed);
	
	// Generate a predictable motion axis, mostly away from object center, based on seed.
	randomizer->Seed( seed );
	double center_motion_scale = randomizer->Double( 7., 11. );
	double rx = randomizer->Double( -30., 30. );
	double ry = randomizer->Double( -30., 30. );
	double rz = randomizer->Double( -30., 30. );
	return Vec3D( CenterPoint.X * center_motion_scale + rx,
	              CenterPoint.Y * center_motion_scale + ry,
	              CenterPoint.Z * center_motion_scale + rz );
}


Vec3D ModelObject::GetExplosionRotationAxis( int seed, Randomizer *randomizer ) const
{
	// Generate predictable rotation axis for this chunk of debris, based on a seed value.
	return GetExplosionMotion( seed + 1, randomizer ).Unit();
}


double ModelObject::GetExplosionRotationRate( int seed, Randomizer *randomizer ) const
{
	// Generate a per-object seed based on the object name.
	seed *= Name.length();
	for( size_t i = 0; i < Name.length(); i ++ )
		seed += (i + 1) * (int)( Name[ i ] );
	seed = abs(seed);
	
	// Generate a predictable rotation rate based on the seed value.
	randomizer->Seed( seed );
	return randomizer->Double( 360., 720. ) * (randomizer->Bool() ? -1. : 1.);
}


// ---------------------------------------------------------------------------


ModelMaterial::ModelMaterial( void )
{
	Ambient.Set( 0.2f, 0.2f, 0.2f, 1.f );
	Diffuse.Set( 0.8f, 0.8f, 0.8f, 1.f );
	Specular.Set( 0.2f, 0.2f, 0.2f, 1.f );
	Shininess = 1.f;
	BumpScale = 0.f;
}


ModelMaterial::ModelMaterial( const ModelMaterial &other )
{
	BecomeInstance( &other );
}


ModelMaterial::ModelMaterial( const ModelMaterial *other )
{
	BecomeInstance( other );
}


ModelMaterial::~ModelMaterial()
{
}


void ModelMaterial::BecomeInstance( const ModelMaterial *other )
{
	Texture.BecomeInstance( &(other->Texture) );
	BumpMap.BecomeInstance( &(other->BumpMap) );
	Ambient = other->Ambient;
	Diffuse = other->Diffuse;
	Specular = other->Specular;
	Shininess = other->Shininess;
	BumpScale = other->BumpScale;
	Arrays.BecomeInstance( &(other->Arrays) );
}


void ModelMaterial::CalculateNormals( void )
{
	Arrays.CalculateNormals();
}


void ModelMaterial::ReverseNormals( void )
{
	Arrays.ReverseNormals();
}


void ModelMaterial::SmoothNormals( void )
{
	Arrays.SmoothNormals();
}
