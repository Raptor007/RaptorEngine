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
#include "RaptorGame.h"


Model::Model( void )
{
	Clear();
}


Model::~Model()
{
}


void Model::Clear( void )
{
	Objects.clear();
	Materials.clear();
	Length = 0.;
	Width = 0.;
	Height = 0.;
	MaxRadius = 0.;
}


void Model::BecomeInstance( Model *other )
{
	Clear();
	
	for( std::map<std::string,ModelObject>::const_iterator obj_iter = other->Objects.begin(); obj_iter != other->Objects.end(); obj_iter ++ )
		Objects[ obj_iter->first ].BecomeInstance( &(obj_iter->second) );
	
	for( std::map<std::string,ModelMaterial>::const_iterator mtl_iter = other->Materials.begin(); mtl_iter != other->Materials.end(); mtl_iter ++ )
		Materials[ mtl_iter->first ].BecomeInstance( &(mtl_iter->second) );
	
	// Force the original model to get length/width/height, so it only has to calculate once when spawning many copies.
	Length = other->GetLength();
	Width = other->GetWidth();
	Height = other->GetHeight();
	MaxRadius = other->GetMaxRadius();
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
						Objects[ obj ].AddFaces( mtl, faces );
						Objects[ obj ].Name = obj;
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
					Materials[ new_mtl ];
					
					if( (mtl != new_mtl) && ! faces.empty() )
					{
						// Build the previous object's arrays, since we're changing materials now.
						Objects[ obj ].AddFaces( mtl, faces );
						Objects[ obj ].Name = obj;
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
							
							Objects[ obj ].Points.push_back( vertices[ vertex_num - 1 ] );
						}
					}
				}
				else if( elements.at( 0 ) == "l" )
				{
					Objects[ obj ].Lines.push_back( std::vector<Vec3D>() );
					
					for( size_t i = 1; i < elements.size(); i ++ )
					{
						int vertex_num = atoi(elements.at(i).c_str());
						if( vertex_num )
						{
							if( vertex_num < 0 )
								vertex_num += vertices.size() + 1;
							
							Objects[ obj ].Lines.back().push_back( vertices[ vertex_num - 1 ] );
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
					Materials[ mtl ];
				}
				else if( elements.at( 0 ) == "map_Kd" )
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
						
						Materials[ mtl ].Texture.BecomeInstance( Raptor::Game->Res.GetAnimation( tex_filename ) );
					}
				}
				else if( elements.at( 0 ) == "Ka" )
				{
					if( elements.size() >= 4 )
					{
						Materials[ mtl ].Ambient.Red = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ].Ambient.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ].Ambient.Blue = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "Kd" )
				{
					if( elements.size() >= 4 )
					{
						Materials[ mtl ].Diffuse.Red = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ].Diffuse.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ].Diffuse.Blue = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "Ks" )
				{
					if( elements.size() >= 4 )
					{
						Materials[ mtl ].Specular.Red = atof( elements.at( 1 ).c_str() );
						Materials[ mtl ].Specular.Green = atof( elements.at( 2 ).c_str() );
						Materials[ mtl ].Specular.Blue = atof( elements.at( 3 ).c_str() );
					}
				}
				else if( elements.at( 0 ) == "d" )
				{
					if( elements.size() >= 2 )
					{
						float alpha = atof( elements.at( 1 ).c_str() );
						// NOTE: Currently, only Ambient.Alpha is passed to the shaders.
						Materials[ mtl ].Ambient.Alpha = alpha;
						Materials[ mtl ].Diffuse.Alpha = alpha;
					}
				}
				else if( elements.at( 0 ) == "Ns" )
				{
					if( elements.size() >= 2 )
						Materials[ mtl ].Shininess = atof( elements.at( 1 ).c_str() );
				}
			}
		}
		
		if( ! faces.empty() )
		{
			// Build the final object's arrays.
			Objects[ obj ].AddFaces( mtl, faces );
			Objects[ obj ].Name = obj;
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
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
		{
			if( vertex_counts.find( array_iter->first ) != vertex_counts.end() )
				vertex_counts[ array_iter->first ] += array_iter->second.VertexCount;
			else
				vertex_counts[ array_iter->first ] = array_iter->second.VertexCount;
		}
	}
	
	// Allocate and fill material arrays.
	for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
	{
		size_t vertex_count = 0;
		if( vertex_counts.find( mtl_iter->first ) != vertex_counts.end() )
			vertex_count = vertex_counts[ mtl_iter->first ];
		
		mtl_iter->second.Arrays.Resize( vertex_count );
		
		if( vertex_count )
		{
			size_t vertices_filled = 0;
			for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
			{
				std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.find( mtl_iter->first );
				if( (array_iter != obj_iter->second.Arrays.end()) && array_iter->second.VertexCount )
				{
					memcpy( ((char*)( mtl_iter->second.Arrays.VertexArray )) + vertices_filled * 3 * sizeof(GLdouble), array_iter->second.VertexArray, array_iter->second.VertexCount * 3 * sizeof(GLdouble) );
					memcpy( ((char*)( mtl_iter->second.Arrays.TexCoordArray )) + vertices_filled * 2 * sizeof(GLfloat), array_iter->second.TexCoordArray, array_iter->second.VertexCount * 2 * sizeof(GLfloat) );
					memcpy( ((char*)( mtl_iter->second.Arrays.NormalArray )) + vertices_filled * 3 * sizeof(GLfloat), array_iter->second.NormalArray, array_iter->second.VertexCount * 3 * sizeof(GLfloat) );
					
					vertices_filled += array_iter->second.VertexCount;
				}
			}
		}
	}
}


void Model::CalculateNormals( void )
{
	for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second.CalculateNormals();
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second.CalculateNormals();
}


void Model::ReverseNormals( void )
{
	for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second.ReverseNormals();
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second.ReverseNormals();
}


void Model::SmoothNormals( void )
{
	for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		mtl_iter->second.SmoothNormals();
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second.SmoothNormals();
}


void Model::Draw( const Pos3D *pos, const std::set<std::string> *object_names, const Color *wireframe, double exploded, int explosion_seed, double fwd_scale, double up_scale, double right_scale )
{
	bool use_shaders = Raptor::Game->ShaderMgr.Active();
	
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
		
		for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Materials.begin(); mtl_iter != Materials.end(); mtl_iter ++ )
		{
			if( mtl_iter->second.Arrays.VertexCount )
			{
				if( use_shaders )
				{
					if( ! wireframe )
					{
						Raptor::Game->ShaderMgr.Set3f( "AmbientColor", mtl_iter->second.Ambient.Red, mtl_iter->second.Ambient.Green, mtl_iter->second.Ambient.Blue );
						Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", mtl_iter->second.Diffuse.Red, mtl_iter->second.Diffuse.Green, mtl_iter->second.Diffuse.Blue );
						Raptor::Game->ShaderMgr.Set3f( "SpecularColor", mtl_iter->second.Specular.Red, mtl_iter->second.Specular.Green, mtl_iter->second.Specular.Blue );
						Raptor::Game->ShaderMgr.Set1f( "Alpha", mtl_iter->second.Ambient.Alpha );
						Raptor::Game->ShaderMgr.Set1f( "Shininess", mtl_iter->second.Shininess );
						
						glActiveTexture( GL_TEXTURE0 + 0 );
						Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
					}
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second.Arrays.VertexArray );
				}
				else
				{
					// Calculate worldspace coordinates on the CPU (slow for complex models).
					mtl_iter->second.Arrays.MakeWorldSpace( pos, fwd_scale, up_scale, right_scale );
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second.Arrays.WorldSpaceVertexArray );
				}
				
				if( ! wireframe )
				{
					glBindTexture( GL_TEXTURE_2D, mtl_iter->second.Texture.CurrentFrame() );
					
					glTexCoordPointer( 2, GL_FLOAT, 0, mtl_iter->second.Arrays.TexCoordArray );
					glNormalPointer( GL_FLOAT, 0, mtl_iter->second.Arrays.NormalArray );
				}
				
				glDrawArrays( GL_TRIANGLES, 0, mtl_iter->second.Arrays.VertexCount );
			}
		}
	}
	else
	{
		// The model is exploding, so draw per-object arrays (slower, but allows multiple positions and rotations).
		
		Pos3D draw_pos;
		Vec3D x_vec, y_vec, z_vec;
		
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			if( object_names && (object_names->find( obj_iter->first ) == object_names->end()) )
				continue;
			
			draw_pos.Copy( pos );
			
			if( exploded > 0. )
			{
				// Convert explosion vectors to worldspace.
				Vec3D explosion_motion = obj_iter->second.GetExplosionMotion( explosion_seed ) * exploded;
				Vec3D modelspace_rotation_axis = obj_iter->second.GetExplosionRotationAxis( explosion_seed );
				Vec3D worldspace_rotation_axis = (pos->Fwd * modelspace_rotation_axis.X) + (pos->Up * modelspace_rotation_axis.Y) + (pos->Right * modelspace_rotation_axis.Z);
				
				draw_pos.MoveAlong( &(pos->Fwd), explosion_motion.X );
				draw_pos.MoveAlong( &(pos->Up), explosion_motion.Y );
				draw_pos.MoveAlong( &(pos->Right), explosion_motion.Z );
				
				double explosion_rotation_rate = obj_iter->second.GetExplosionRotationRate( explosion_seed );
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
			
			for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			{
				if( array_iter->second.VertexCount )
				{
					if( use_shaders )
					{
						if( ! wireframe )
						{
							Raptor::Game->ShaderMgr.Set3f( "AmbientColor", Materials[ array_iter->first ].Ambient.Red, Materials[ array_iter->first ].Ambient.Green, Materials[ array_iter->first ].Ambient.Blue );
							Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", Materials[ array_iter->first ].Diffuse.Red, Materials[ array_iter->first ].Diffuse.Green, Materials[ array_iter->first ].Diffuse.Blue );
							Raptor::Game->ShaderMgr.Set3f( "SpecularColor", Materials[ array_iter->first ].Specular.Red, Materials[ array_iter->first ].Specular.Green, Materials[ array_iter->first ].Specular.Blue );
							Raptor::Game->ShaderMgr.Set1f( "Alpha", Materials[ array_iter->first ].Ambient.Alpha );
							Raptor::Game->ShaderMgr.Set1f( "Shininess", Materials[ array_iter->first ].Shininess );
							
							glActiveTexture( GL_TEXTURE0 + 0 );
							Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
						}
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second.VertexArray );
					}
					else
					{
						// Calculate worldspace coordinates on the CPU (slow for complex models).
						array_iter->second.MakeWorldSpace( &draw_pos, fwd_scale, up_scale, right_scale );
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second.WorldSpaceVertexArray );
					}
					
					if( ! wireframe )
					{
						glBindTexture( GL_TEXTURE_2D, Materials[ array_iter->first ].Texture.CurrentFrame() );
						
						glTexCoordPointer( 2, GL_FLOAT, 0, array_iter->second.TexCoordArray );
						glNormalPointer( GL_FLOAT, 0, array_iter->second.NormalArray );
					}
					
					glDrawArrays( GL_TRIANGLES, 0, array_iter->second.VertexCount );
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
	}
}


void Model::DrawAt( const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	Draw( pos, NULL, NULL, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


void Model::DrawObjectsAt( const std::list<std::string> *object_names, const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	std::set<std::string> object_name_set;
	for( std::list<std::string>::const_iterator obj_iter = object_names->begin(); obj_iter != object_names->end(); obj_iter ++ )
		object_name_set.insert( *obj_iter );
	
	Draw( pos, &object_name_set, NULL, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


void Model::DrawWireframeAt( const Pos3D *pos, Color color, double scale, double fwd_scale, double up_scale, double right_scale )
{
	Draw( pos, NULL, &color, 0., 0, scale * fwd_scale, scale * up_scale, scale * right_scale );
}


void Model::Move( double fwd, double up, double right )
{
	// FIXME: What does this mean if we're using someone else's memory?
	
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
		{
			// FIXME: Vectorize and/or parallelize?
			for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
			{
				array_iter->second.VertexArray[ i*3     ] += fwd;
				array_iter->second.VertexArray[ i*3 + 1 ] += up;
				array_iter->second.VertexArray[ i*3 + 2 ] += right;
			}
		}
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
	// FIXME: What does this mean if we're using someone else's memory?
	
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
	{
		for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
		{
			// FIXME: Vectorize and/or parallelize?
			for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
			{
				array_iter->second.VertexArray[ i*3     ] *= fwd_scale;
				array_iter->second.VertexArray[ i*3 + 1 ] *= up_scale;
				array_iter->second.VertexArray[ i*3 + 2 ] *= right_scale;
			}
		}
		
		for( std::vector<Vec3D>::iterator point_iter = obj_iter->second.Points.begin(); point_iter != obj_iter->second.Points.end(); point_iter ++ )
		{
			point_iter->X *= fwd_scale;
			point_iter->Y *= up_scale;
			point_iter->Z *= right_scale;
		}
		
		for( std::vector< std::vector<Vec3D> >::iterator line_iter = obj_iter->second.Lines.begin(); line_iter != obj_iter->second.Lines.end(); line_iter ++ )
		{
			for( std::vector<Vec3D>::iterator vertex_iter = line_iter->begin(); vertex_iter != line_iter->end(); vertex_iter ++ )
			{
				vertex_iter->X *= fwd_scale;
				vertex_iter->Y *= up_scale;
				vertex_iter->Z *= right_scale;
			}
		}
		
		obj_iter->second.Recalc();
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
		
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
				{
					double x = array_iter->second.VertexArray[ i*3     ];
					
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
	}
	
	return Length;
}


double Model::GetHeight( void )
{
	if( Height <= 0. )
	{
		double top = 0., bottom = 0.;
		bool found_top = false, found_bottom = false;
		
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
				{
					double y = array_iter->second.VertexArray[ i*3 + 1 ];
					
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
	}
	
	return Height;
}


double Model::GetWidth( void )
{
	if( Width <= 0. )
	{
		double right = 0., left = 0.;
		bool found_right = false, found_left = false;
		
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
				{
					double z = array_iter->second.VertexArray[ i*3 + 2 ];
					
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
		
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			{
				for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
				{
					x = array_iter->second.VertexArray[ i*3 ];
					y = array_iter->second.VertexArray[ i*3 + 1 ];
					z = array_iter->second.VertexArray[ i*3 + 2 ];
					
					dist = sqrt( x*x + y*y + z*z );
					
					if( dist > MaxRadius )
						MaxRadius = dist;
				}
			}
		}
	}
	
	return MaxRadius;
}


size_t Model::ArrayCount( void ) const
{
	size_t count = 0;
	
	for( std::map<std::string,ModelObject>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		count += obj_iter->second.Arrays.size();
	
	return count;
}


size_t Model::TriangleCount( void ) const
{
	return VertexCount() / 3;
}


size_t Model::VertexCount( void ) const
{
	size_t count = 0;
	
	for( std::map<std::string,ModelObject>::const_iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			count += array_iter->second.VertexCount;
	
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
	Allocated = false;
	WorldSpaceVertexArray = NULL;
}


ModelArrays::ModelArrays( const ModelArrays &other )
{
	VertexCount = 0;
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	Allocated = false;
	WorldSpaceVertexArray = NULL;
	
	BecomeCopy( &other );
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
	}
	
	VertexArray = NULL;
	TexCoordArray = NULL;
	NormalArray = NULL;
	Allocated = false;
	
	// Each instance is always responsible for its own WorldSpaceVertexArray.
	if( WorldSpaceVertexArray )
		free( WorldSpaceVertexArray );
	WorldSpaceVertexArray = NULL;
}


void ModelArrays::BecomeCopy( const ModelArrays *other )
{
	Clear();
	
	VertexCount = other->VertexCount;
	
	if( other->VertexArray )
	{
		size_t vertex_array_mem = sizeof(GLdouble) * 3 * VertexCount;
		VertexArray = (GLdouble*) malloc( vertex_array_mem );
		if( VertexArray )
		{
			memcpy( VertexArray, other->VertexArray, vertex_array_mem );
			Allocated = true;
		}
	}
	
	if( other->TexCoordArray )
	{
		size_t tex_coord_array_mem = sizeof(GLfloat) * 2 * VertexCount;
		TexCoordArray = (GLfloat*) malloc( tex_coord_array_mem );
		if( TexCoordArray )
		{
			memcpy( TexCoordArray, other->TexCoordArray, tex_coord_array_mem );
			Allocated = true;
		}
	}
	
	if( other->NormalArray )
	{
		size_t normal_array_mem = sizeof(GLfloat) * 3 * VertexCount;
		NormalArray = (GLfloat*) malloc( normal_array_mem );
		if( NormalArray )
		{
			memcpy( NormalArray, other->NormalArray, normal_array_mem );
			Allocated = true;
		}
	}
}


void ModelArrays::BecomeInstance( const ModelArrays *other, bool copy )
{
	if( copy )
		BecomeCopy( other );
	else
	{
		Clear();
		
		VertexCount = other->VertexCount;
		
		VertexArray = other->VertexArray;
		TexCoordArray = other->TexCoordArray;
		NormalArray = other->NormalArray;
	}
}


void ModelArrays::Resize( size_t vertex_count )
{
	if( vertex_count == VertexCount )
		;
	else if( vertex_count )
	{
		// FIXME: What does this mean if another model is using our memory?
		
		size_t old_vertex_count = VertexCount;
		VertexCount = vertex_count;
		
		size_t vertex_array_mem = sizeof(GLdouble) * 3 * VertexCount;
		size_t tex_coord_array_mem = sizeof(GLfloat) * 2 * VertexCount;
		size_t normal_array_mem = sizeof(GLfloat) * 3 * VertexCount;
		
		if( VertexArray )
		{
			if( Allocated )
				VertexArray = (GLdouble*) realloc( VertexArray, vertex_array_mem );
			else
			{
				size_t old_mem = sizeof(GLdouble) * 3 * old_vertex_count;
				GLdouble *old_ptr = VertexArray;
				VertexArray = (GLdouble*) malloc( vertex_array_mem );
				if( VertexArray )
					memcpy( VertexArray, old_ptr, old_mem );
			}
		}
		else
			VertexArray = (GLdouble*) malloc( vertex_array_mem );
		
		if( TexCoordArray )
		{
			if( Allocated )
				TexCoordArray = (GLfloat*) realloc( TexCoordArray, tex_coord_array_mem );
			else
			{
				size_t old_mem = sizeof(GLfloat) * 2 * old_vertex_count;
				GLfloat *old_ptr = TexCoordArray;
				TexCoordArray = (GLfloat*) malloc( tex_coord_array_mem );
				if( TexCoordArray )
					memcpy( TexCoordArray, old_ptr, old_mem );
			}
		}
		else
			TexCoordArray = (GLfloat*) malloc( tex_coord_array_mem );
		
		if( NormalArray )
		{
			if( Allocated )
				NormalArray = (GLfloat*) realloc( NormalArray, normal_array_mem );
			else
			{
				size_t old_mem = sizeof(GLfloat) * 3 * old_vertex_count;
				GLfloat *old_ptr = NormalArray;
				NormalArray = (GLfloat*) malloc( normal_array_mem );
				if( NormalArray )
					memcpy( NormalArray, old_ptr, old_mem );
			}
		}
		else
			NormalArray = (GLfloat*) malloc( normal_array_mem );
		
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
			// If it's a quad or other large shape, add copies first.
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
			// If it's a quad or other large shape, add copies first.
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
			// If it's a quad or other large shape, add copies first.
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
	
	
	if( vertices.size() )
	{
		// Get rid of the old worldspace array; it will be regenerated later if necessary.
		if( WorldSpaceVertexArray )
			free( WorldSpaceVertexArray );
		WorldSpaceVertexArray = NULL;
		
		
		// We have vertices to add, so we'll need to increase our allocations or make new ones.
		size_t old_vertex_count = VertexCount;
		Resize( VertexCount + vertices.size() );
		
		
		// Fill the new parts of the arrays.
		// FIXME: Vectorize and/or parallelize?
		
		size_t vertices_size = vertices.size();
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
	}
}


void ModelArrays::CalculateNormals( void )
{
	Vec3D a, b, c;
	for( size_t i = 0; i < VertexCount; i += 3 )
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
}


void ModelArrays::ReverseNormals( void )
{
	Vec3D a, b, c;
	for( size_t i = 0; i < VertexCount; i ++ )
	{
		NormalArray[ i*3     ] *= -1.;
		NormalArray[ i*3 + 1 ] *= -1.;
		NormalArray[ i*3 + 2 ] *= -1.;
	}
}


void ModelArrays::SmoothNormals( void )
{
	for( size_t i = 0; i < VertexCount; i ++ )
	{
		Vec3D normal(0,0,0);
		std::set<Vec3D> unique;
		for( size_t j = 0; j < VertexCount; j ++ )
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
		}
		
		// Translate from modelspace to worldspace.
		// FIXME: Vectorize and/or parallelize?
		for( size_t i = 0; i < VertexCount; i ++ )
		{
			Vec3D vertex( VertexArray[ i*3 ], VertexArray[ i*3 + 1 ], VertexArray[ i*3 + 2 ] );
			WorldSpaceVertexArray[ i*3     ] = pos->X + x_vec.Dot(&vertex);
			WorldSpaceVertexArray[ i*3 + 1 ] = pos->Y + y_vec.Dot(&vertex);
			WorldSpaceVertexArray[ i*3 + 2 ] = pos->Z + z_vec.Dot(&vertex);
		}
	}
}


// ---------------------------------------------------------------------------


ModelObject::ModelObject( void )
{
	CenterPoint.SetPos( 0., 0., 0. );
	MaxRadius = 0.;
	NeedsRecalc = false;
}


ModelObject::ModelObject( const ModelObject &other )
{
	Arrays = other.Arrays;
	Points = other.Points;
	
	CenterPoint = other.CenterPoint;
	MaxRadius = other.MaxRadius;
	NeedsRecalc = other.NeedsRecalc;
}


ModelObject::~ModelObject()
{
}


void ModelObject::BecomeInstance( const ModelObject *other )
{
	Arrays.clear();
	for( std::map<std::string,ModelArrays>::const_iterator array_iter = other->Arrays.begin(); array_iter != other->Arrays.end(); array_iter ++ )
		Arrays[ array_iter->first ].BecomeInstance( &(array_iter->second) );
	
	Points = other->Points;
	
	Name = other->Name;
	
	NeedsRecalc = true;
}


void ModelObject::AddFaces( std::string mtl, std::vector<ModelFace> &faces )
{
	Arrays[ mtl ].AddFaces( faces );
	
	NeedsRecalc = true;
}


void ModelObject::Recalc( void )
{
	// FIXME: Vectorize and/or parallelize?
	
	CenterPoint.SetPos( 0., 0., 0. );
	MaxRadius = 0.;
	size_t vertex_count = 0;

	MinFwd = FLT_MAX;
	MinUp = FLT_MAX;
	MinRight = FLT_MAX;
	MaxFwd = -FLT_MAX;
	MaxUp = -FLT_MAX;
	MaxRight = -FLT_MAX;
	
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
	{
		vertex_count += array_iter->second.VertexCount;
		
		for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
		{
			// In model space, X = fwd, Y = up, Z = right.
			double x = array_iter->second.VertexArray[ i*3     ];
			double y = array_iter->second.VertexArray[ i*3 + 1 ];
			double z = array_iter->second.VertexArray[ i*3 + 2 ];
			
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
		}
	}
	
	if( vertex_count )
	{
		// In model space, X = fwd, Y = up, Z = right.
		CenterPoint.X = (MinFwd + MaxFwd) / 2.;
		CenterPoint.Y = (MinUp + MaxUp) / 2.;
		CenterPoint.Z = (MinRight + MaxRight) / 2.;
		
		for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		{
			for( size_t i = 0; i < array_iter->second.VertexCount; i ++ )
			{
				double x = array_iter->second.VertexArray[ i*3     ] - CenterPoint.X;
				double y = array_iter->second.VertexArray[ i*3 + 1 ] - CenterPoint.Y;
				double z = array_iter->second.VertexArray[ i*3 + 2 ] - CenterPoint.Z;
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
	}
	
	NeedsRecalc = false;
}


void ModelObject::CalculateNormals( void )
{
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second.CalculateNormals();
}


void ModelObject::ReverseNormals( void )
{
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second.ReverseNormals();
}


void ModelObject::SmoothNormals( void )
{
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second.SmoothNormals();
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


Vec3D ModelObject::GetExplosionMotion( int seed ) const
{
	/*
	if( NeedsRecalc )
		Recalc();
	*/
	
	// Generate a per-object seed based on the object name.
	seed *= Name.length();
	for( size_t i = 0; i < Name.length(); i ++ )
		seed += (i + 1) * (int)( Name[ i ] );
	
	// Generate a predictable motion axis, mostly away from object center, based on seed.
	double center_motion_scale = 10. + ((seed*(seed+2)) % 300) / 10.;
	return Vec3D( CenterPoint.X * center_motion_scale - 5. + 10. * ((seed*(seed+2)) % 344) / 343.,
	              CenterPoint.Y * center_motion_scale - 5. + 10. * ((seed*(seed+4)) % 344) / 343.,
	              CenterPoint.Z * center_motion_scale - 5. + 10. * ((seed*(seed+6)) % 344) / 343. );
}


Vec3D ModelObject::GetExplosionRotationAxis( int seed ) const
{
	// Generate predictable rotation axis for this chunk of debris, based on a seed value.
	Vec3D explosion_rotation_axis = GetExplosionMotion( seed );
	explosion_rotation_axis.ScaleTo( 1. );
	return explosion_rotation_axis;
}


double ModelObject::GetExplosionRotationRate( int seed ) const
{
	// Generate a per-object seed based on the object name.
	seed *= Name.length();
	for( size_t i = 0; i < Name.length(); i ++ )
		seed += (i + 1) * (int)( Name[ i ] );
	
	// Generate a predictable rotation rate based on the seed value.
	return 360. + (seed*(seed+2)) % 361;
}


// ---------------------------------------------------------------------------


ModelMaterial::ModelMaterial( void )
{
	Ambient.Set( 0.2f, 0.2f, 0.2f, 1.f );
	Diffuse.Set( 0.8f, 0.8f, 0.8f, 1.f );
	Specular.Set( 0.2f, 0.2f, 0.2f, 1.f );
	Shininess = 1.f;
}


ModelMaterial::ModelMaterial( const ModelMaterial &other )
{
	Texture.BecomeInstance( &(other.Texture) );
	Ambient = other.Ambient;
	Diffuse = other.Diffuse;
	Specular = other.Specular;
	Shininess = other.Shininess;
	Arrays = other.Arrays;
}


ModelMaterial::~ModelMaterial()
{
	Arrays.Clear();
}


void ModelMaterial::BecomeInstance( const ModelMaterial *other )
{
	Texture.BecomeInstance( &(other->Texture) );
	Ambient = other->Ambient;
	Diffuse = other->Diffuse;
	Specular = other->Specular;
	Shininess = other->Shininess;
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
