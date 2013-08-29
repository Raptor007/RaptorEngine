/*
 *  Model.cpp
 */

#include "Model.h"

#include <cstddef>
#include <cmath>
#include <fstream>
#include <set>
#include "Str.h"
#include "Rand.h"
#include "Num.h"
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
	ExplodedSeconds = 0.;
}


void Model::Reset( void )
{
	ExplodedSeconds = 0.;
	
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		obj_iter->second.RandomizeExplosionVectors();
}


void Model::BecomeInstance( Model *other )
{
	Clear();
	
	for( std::map<std::string,ModelObject>::const_iterator obj_iter = other->Objects.begin(); obj_iter != other->Objects.end(); obj_iter ++ )
		Objects[ obj_iter->first ].BecomeInstance( &(obj_iter->second) );
	
	for( std::map<std::string,ModelMaterial>::const_iterator mtl_iter = other->Materials.begin(); mtl_iter != other->Materials.end(); mtl_iter ++ )
		Materials[ mtl_iter->first ].BecomeInstance( &(mtl_iter->second) );
	
	ExplodedSeconds = other->ExplodedSeconds;
	
	// Force the original model to get length/width/height, so it only has to calculate once when spawning many copies.
	Length = other->GetLength();
	Width = other->GetWidth();
	Height = other->GetHeight();
}


bool Model::LoadOBJ( std::string filename )
{
	Clear();
	
	bool return_value = IncludeOBJ( filename );
	MakeMaterialArrays();
	return return_value;
}


bool Model::IncludeOBJ( std::string filename )
{
	std::ifstream input( filename.c_str() );
	if( input.is_open() )
	{
		char buffer[ 1024 ] = "";
		
		int fwd_index = 3, up_index = 2, right_index = 1;
		double fwd_scale = 1., up_scale = 1., right_scale = 1.;
		
		std::string obj = "", mtl = "";
		
		std::vector<Vec3D> vertices;
		std::vector<Vec2D> tex_coords;
		std::vector<Vec3D> normals;
		std::vector<ModelFace> faces;
		
		while( ! input.eof() )
		{
			buffer[ 0 ] = '\0';
			input.getline( buffer, 1024 );
			
			// Skip whitespace at the start of a line.
			char *buffer_start = buffer;
			while( strchr( " \t", buffer_start[0] ) )
				buffer_start ++;
			
			// Remove unnecessary characters from the buffer and skip empty lines.
			snprintf( buffer, 1024, "%s", Str::Join( CStr::SplitToList( buffer_start, "\r\n" ), "" ).c_str() );
			if( ! strlen(buffer) )
				continue;
			
			std::vector<std::string> elements = CStr::SplitToVector( buffer, " " );
			
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
							face_vertices.push_back( vertices[ vertex_num - 1 ] );
							
							if( element_items.size() >= 2 )
							{
								int tex_coord_num = atoi(element_items.at(1).c_str());
								if( tex_coord_num )
									face_tex_coords.push_back( tex_coords[ tex_coord_num - 1 ] );
								else
									face_tex_coords.push_back( Vec2D( 0.5, 0.5 ) );
							}
							else
								face_tex_coords.push_back( Vec2D( 0.5, 0.5 ) );
							
							if( element_items.size() >= 3 )
							{
								int normal_num = atoi(element_items.at(2).c_str());
								if( normal_num )
									face_normals.push_back( normals[ normal_num - 1 ] );
								else
									face_normals.push_back( Vec3D( 0., 0., 0. ) );
							}
							else
								face_normals.push_back( Vec3D( 0., 0., 0. ) );
						}
					}
					
					// Calculate a normal, in case we didn't have one specified in the model.
					if( face_vertices.size() >= 3 )
					{
						calc_normal.Copy( ( face_vertices[ 1 ] - face_vertices[ 0 ] ).Cross( face_vertices[ 2 ] - face_vertices[ 0 ] ) );
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
						faces.push_back( ModelFace( face_vertices, face_tex_coords, face_normals ) );
				}
				else if( elements.at( 0 ) == "o" )
				{
					// Build the previous object's arrays, since we're changing objects now.
					Objects[ obj ].AddFaces( mtl, faces );
					faces.clear();
					
					if( elements.size() >= 2 )
						obj = elements.at( 1 );
					else
						obj = "";
				}
				else if( elements.at( 0 ) == "usemtl" )
				{
					// Build the previous object's arrays, since we're changing materials now.
					Objects[ obj ].AddFaces( mtl, faces );
					faces.clear();
					
					if( elements.size() >= 2 )
						mtl = elements.at( 1 );
					else
						mtl = "";
				}
				else if( elements.at( 0 ) == "mtllib" )
				{
					if( elements.size() >= 2 )
					{
						std::list<std::string> path = Str::SplitToList( filename, "/" );
						path.pop_back();
						path.push_back( elements.at( 1 ) );
						std::string mtl_filename = Str::Join( path, "/" );
						if( mtl_filename != filename )
							IncludeOBJ( mtl_filename );
					}
				}
				
				
				// MTL
				
				else if( elements.at( 0 ) == "newmtl" )
				{
					if( elements.size() >= 2 )
					{
						Materials[ elements.at( 1 ) ];
						mtl = elements.at( 1 );
					}
					else
						mtl = "";
				}
				else if( elements.at( 0 ) == "map_Kd" )
				{
					if( elements.size() >= 2 )
						Materials[ mtl ].Texture.BecomeInstance( Raptor::Game->Res.GetAnimation( elements.at( 1 ) ) );
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
			}
			
			// Build the final object's arrays.
			Objects[ obj ].AddFaces( mtl, faces );
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
	
	std::map<std::string,int> vertex_counts;
	
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
		int vertex_count = 0;
		if( vertex_counts.find( mtl_iter->first ) != vertex_counts.end() )
			vertex_count = vertex_counts[ mtl_iter->first ];
		
		mtl_iter->second.Arrays.Resize( vertex_count );
		
		if( vertex_count )
		{
			int vertices_filled = 0;
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


void Model::DrawAt( const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	bool use_shaders = Raptor::Game->ShaderMgr.Active();
	
	glEnable( GL_TEXTURE_2D );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	if( ExplodedSeconds <= 0. )
	{
		// The model is not exploding, so draw per-material arrays (faster).
		
		if( use_shaders )
		{
			// In model space, X = fwd, Y = up, Z = right.
			Vec3D x_vec( pos->Fwd.X * fwd_scale, pos->Up.X * up_scale, pos->Right.X * right_scale );
			Vec3D y_vec( pos->Fwd.Y * fwd_scale, pos->Up.Y * up_scale, pos->Right.Y * right_scale );
			Vec3D z_vec( pos->Fwd.Z * fwd_scale, pos->Up.Z * up_scale, pos->Right.Z * right_scale );
			
			x_vec *= scale;
			y_vec *= scale;
			z_vec *= scale;
			
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
					Raptor::Game->ShaderMgr.Set3f( "AmbientColor", mtl_iter->second.Ambient.Red, mtl_iter->second.Ambient.Green, mtl_iter->second.Ambient.Blue );
					Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", mtl_iter->second.Diffuse.Red, mtl_iter->second.Diffuse.Green, mtl_iter->second.Diffuse.Blue );
					
					glActiveTexture( GL_TEXTURE0 + 0 );
					Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second.Arrays.VertexArray );
				}
				else
				{
					// Calculate worldspace coordinates on the CPU (slow for complex models).
					mtl_iter->second.Arrays.MakeWorldSpace( pos, scale, fwd_scale, up_scale, right_scale );
					
					glVertexPointer( 3, GL_DOUBLE, 0, mtl_iter->second.Arrays.WorldSpaceVertexArray );
				}
				
				glBindTexture( GL_TEXTURE_2D, mtl_iter->second.Texture.CurrentFrame() );
				
				glTexCoordPointer( 2, GL_FLOAT, 0, mtl_iter->second.Arrays.TexCoordArray );
				glNormalPointer( GL_FLOAT, 0, mtl_iter->second.Arrays.NormalArray );
				
				glDrawArrays( GL_TRIANGLES, 0, mtl_iter->second.Arrays.VertexCount );
			}
		}
	}
	else
	{
		// The model is exploding, so draw per-object arrays (slower, but allows multiple positions and rotations).
		
		Pos3D draw_pos;
		Vec3D x_vec, y_vec, z_vec;
		
		int obj_num = 0;
		for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		{
			draw_pos.Copy( pos );
			
			draw_pos += obj_iter->second.GetExplosionMotion() * ExplodedSeconds;
			
			draw_pos.Fwd.RotateAround( &(obj_iter->second.ExplosionRotationAxis), ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
			draw_pos.Up.RotateAround( &(obj_iter->second.ExplosionRotationAxis), ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
			draw_pos.Right.RotateAround( &(obj_iter->second.ExplosionRotationAxis), ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
			
			if( use_shaders )
			{
				// In model space, X = fwd, Y = up, Z = right.
				x_vec.Set( draw_pos.Fwd.X * fwd_scale, draw_pos.Up.X * up_scale, draw_pos.Right.X * right_scale );
				y_vec.Set( draw_pos.Fwd.Y * fwd_scale, draw_pos.Up.Y * up_scale, draw_pos.Right.Y * right_scale );
				z_vec.Set( draw_pos.Fwd.Z * fwd_scale, draw_pos.Up.Z * up_scale, draw_pos.Right.Z * right_scale );
				
				x_vec *= scale;
				y_vec *= scale;
				z_vec *= scale;
				
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
						Raptor::Game->ShaderMgr.Set3f( "AmbientColor", Materials[ array_iter->first ].Ambient.Red, Materials[ array_iter->first ].Ambient.Green, Materials[ array_iter->first ].Ambient.Blue );
						Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", Materials[ array_iter->first ].Diffuse.Red, Materials[ array_iter->first ].Diffuse.Green, Materials[ array_iter->first ].Diffuse.Blue );
						
						glActiveTexture( GL_TEXTURE0 + 0 );
						Raptor::Game->ShaderMgr.Set1i( "Texture", 0 );
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second.VertexArray );
					}
					else
					{
						// Calculate worldspace coordinates on the CPU (slow for complex models).
						array_iter->second.MakeWorldSpace( &draw_pos, scale, fwd_scale, up_scale, right_scale );
						
						glVertexPointer( 3, GL_DOUBLE, 0, array_iter->second.WorldSpaceVertexArray );
					}
					
					glBindTexture( GL_TEXTURE_2D, Materials[ array_iter->first ].Texture.CurrentFrame() );
					
					glTexCoordPointer( 2, GL_FLOAT, 0, array_iter->second.TexCoordArray );
					glNormalPointer( GL_FLOAT, 0, array_iter->second.NormalArray );
					
					glDrawArrays( GL_TRIANGLES, 0, array_iter->second.VertexCount );
				}
			}
			
			obj_num ++;
		}
	}
	
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisable( GL_TEXTURE_2D );
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.Set3f( "Pos", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "XVec", 1., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "YVec", 0., 1., 0. );
		Raptor::Game->ShaderMgr.Set3f( "ZVec", 0., 0., 1. );
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1., 1., 1. );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0., 0., 0. );
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
			for( int i = 0; i < array_iter->second.VertexCount; i ++ )
			{
				array_iter->second.VertexArray[ i*3     ] *= fwd_scale;
				array_iter->second.VertexArray[ i*3 + 1 ] *= up_scale;
				array_iter->second.VertexArray[ i*3 + 2 ] *= right_scale;
			}
		}
	}
	
	MakeMaterialArrays();
	
	Length *= fwd_scale;
	Height *= up_scale;
	Width *= right_scale;
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
				for( int i = 0; i < array_iter->second.VertexCount; i ++ )
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
				for( int i = 0; i < array_iter->second.VertexCount; i ++ )
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
				for( int i = 0; i < array_iter->second.VertexCount; i ++ )
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


void Model::Explode( double dt )
{
	ExplodedSeconds += dt;
}


int Model::ArrayCount( void )
{
	int count = 0;
	
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		count += obj_iter->second.Arrays.size();
	
	return count;
}


int Model::TriangleCount( void )
{
	return VertexCount() / 3;
}


int Model::VertexCount( void )
{
	int count = 0;
	
	for( std::map<std::string,ModelObject>::iterator obj_iter = Objects.begin(); obj_iter != Objects.end(); obj_iter ++ )
		for( std::map<std::string,ModelArrays>::iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
			count += array_iter->second.VertexCount;
	
	return count;
}


// ---------------------------------------------------------------------------


ModelFace::ModelFace( std::vector<Vec3D> &vertices, std::vector<Vec2D> &tex_coords, std::vector<Vec3D> &normals )
{
	Vertices = vertices;
	TexCoords = tex_coords;
	Normals = normals;
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


void ModelArrays::BecomeInstance( const ModelArrays *other )
{
	Clear();
	
	VertexCount = other->VertexCount;
	
	VertexArray = other->VertexArray;
	TexCoordArray = other->TexCoordArray;
	NormalArray = other->NormalArray;
	
	// FIXME: This shouldn't be necessary, and it wastes memory!
	BecomeCopy( other );
}


void ModelArrays::Resize( int vertex_count )
{
	if( vertex_count == VertexCount )
		;
	else if( vertex_count )
	{
		// FIXME: What does this mean if another model is using our memory?
		
		int old_vertex_count = VertexCount;
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
	
	for( std::vector<ModelFace>::iterator face_iter = faces.begin(); face_iter != faces.end(); face_iter ++ )
	{
		int count = 0;
		for( std::vector<Vec3D>::iterator vertex_iter = face_iter->Vertices.begin(); vertex_iter != face_iter->Vertices.end(); vertex_iter ++ )
		{
			// If it's a quad or other large shape, add copies first.
			if( count >= 3 )
			{
				Vec3D recent_vertex = vertices.back();
				vertices.push_back( vertices.front() );
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
				tex_coords.push_back( tex_coords.front() );
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
				normals.push_back( normals.front() );
				normals.push_back( recent_normal );
			}
			
			normals.push_back( *normal_iter );
			count ++;
		}
	}
	
	
	if( vertices.size() )
	{
		// Get rid of the old worldspace array; it will be regenerated later if necessary.
		if( WorldSpaceVertexArray )
			free( WorldSpaceVertexArray );
		WorldSpaceVertexArray = NULL;
		
		
		// We have vertices to add, so we'll need to increase our allocations or make new ones.
		int old_vertex_count = VertexCount;
		Resize( VertexCount + vertices.size() );
		
		
		// Fill the new parts of the arrays.
		// FIXME: Vectorize and/or parallelize?
		
		int vertices_size = vertices.size();
		int tex_coords_size = tex_coords.size();
		int normals_size = normals.size();
		
		for( int i = 0; i < vertices_size; i ++ )
		{
			VertexArray[ old_vertex_count*3 + i*3     ] = vertices[ i ].X;
			VertexArray[ old_vertex_count*3 + i*3 + 1 ] = vertices[ i ].Y;
			VertexArray[ old_vertex_count*3 + i*3 + 2 ] = vertices[ i ].Z;
		}
		
		for( int i = 0; i < tex_coords_size; i ++ )
		{
			TexCoordArray[ old_vertex_count*2 + i*2     ] = tex_coords[ i ].X;
			TexCoordArray[ old_vertex_count*2 + i*2 + 1 ] = tex_coords[ i ].Y;
		}
		
		for( int i = 0; i < normals_size; i ++ )
		{
			NormalArray[ old_vertex_count*3 + i*3     ] = normals[ i ].X;
			NormalArray[ old_vertex_count*3 + i*3 + 1 ] = normals[ i ].Y;
			NormalArray[ old_vertex_count*3 + i*3 + 2 ] = normals[ i ].Z;
		}
	}
}


void ModelArrays::CalculateNormals( void )
{
	Vec3D a, b, c;
	for( int i = 0; i < VertexCount; i += 3 )
	{
		a.X = VertexArray[ i*3 + 3 ] - VertexArray[ i*3 ];
		a.Y = VertexArray[ i*3 + 4 ] - VertexArray[ i*3 + 1 ];
		a.Z = VertexArray[ i*3 + 5 ] - VertexArray[ i*3 + 2 ];
		b.X = VertexArray[ i*3 + 6 ] - VertexArray[ i*3 ];
		b.Y = VertexArray[ i*3 + 7 ] - VertexArray[ i*3 + 1 ];
		b.Z = VertexArray[ i*3 + 8 ] - VertexArray[ i*3 + 2 ];
		c = a.Cross( b );
		c.ScaleTo( 1. );
		NormalArray[ i*3 ] = c.X;
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


void ModelArrays::MakeWorldSpace( const Pos3D *pos, double scale, double fwd_scale, double up_scale, double right_scale )
{
	if( VertexCount )
	{
		// In model space, X = fwd, Y = up, Z = right.
		Vec3D x_vec( pos->Fwd.X * fwd_scale, pos->Up.X * up_scale, pos->Right.X * right_scale );
		Vec3D y_vec( pos->Fwd.Y * fwd_scale, pos->Up.Y * up_scale, pos->Right.Y * right_scale );
		Vec3D z_vec( pos->Fwd.Z * fwd_scale, pos->Up.Z * up_scale, pos->Right.Z * right_scale );
		
		x_vec *= scale;
		y_vec *= scale;
		z_vec *= scale;
		
		// Make sure we have an array allocated for worldspace vertices.
		if( ! WorldSpaceVertexArray )
		{
			size_t vertex_array_mem = sizeof(GLdouble) * 3 * VertexCount;
			WorldSpaceVertexArray = (GLdouble*) malloc( vertex_array_mem );
		}
		
		// Translate from modelspace to worldspace.
		// FIXME: Vectorize and/or parallelize?
		for( int i = 0; i < VertexCount; i ++ )
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
	CenterPoint.Set( 0., 0., 0. );
	NeedsRecalc = false;
	
	RandomizeExplosionVectors();
}


ModelObject::ModelObject( const ModelObject &other )
{
	Arrays = other.Arrays;
	
	CenterPoint = other.CenterPoint;
	NeedsRecalc = other.NeedsRecalc;
	
	ExplosionRotationAxis = other.ExplosionRotationAxis;
	ExplosionRotationRate = other.ExplosionRotationRate;
}


ModelObject::~ModelObject()
{
	Arrays.clear();
}


void ModelObject::BecomeInstance( const ModelObject *other )
{
	Arrays.clear();
	
	ExplosionMotion = other->ExplosionMotion;
	ExplosionRotationAxis = other->ExplosionRotationAxis;
	ExplosionRotationRate = other->ExplosionRotationRate;
	
	for( std::map<std::string,ModelArrays>::const_iterator array_iter = other->Arrays.begin(); array_iter != other->Arrays.end(); array_iter ++ )
		Arrays[ array_iter->first ].BecomeInstance( &(array_iter->second) );
	
	NeedsRecalc = true;
}


void ModelObject::RandomizeExplosionVectors( void )
{
	// Generate a random rotation axis for this chunk of debris.
	ExplosionRotationAxis.Set( Rand::Double( -1., 1. ), Rand::Double( -1., 1. ), Rand::Double( -1., 1. ) );
	ExplosionRotationAxis.ScaleTo( 1. );
	ExplosionRotationRate = Rand::Double( 360., 720. );
}


void ModelObject::AddFaces( std::string mtl, std::vector<ModelFace> &faces )
{
	Arrays[ mtl ].AddFaces( faces );
	
	NeedsRecalc = true;
}


void ModelObject::Recalc( void )
{
	// FIXME: Use a better algorithm than this?
	// FIXME: Vectorize and/or parallelize?
	
	CenterPoint.Set( 0., 0., 0. );
	int vertex_count = 0;
	
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
	{
		vertex_count += array_iter->second.VertexCount;
		
		for( int i = 0; i < array_iter->second.VertexCount; i ++ )
		{
			CenterPoint.X += array_iter->second.VertexArray[ i*3     ];
			CenterPoint.Y += array_iter->second.VertexArray[ i*3 + 1 ];
			CenterPoint.Z += array_iter->second.VertexArray[ i*3 + 2 ];
		}
	}
	
	if( vertex_count )
	{
		CenterPoint.X /= vertex_count;
		CenterPoint.Y /= vertex_count;
		CenterPoint.Z /= vertex_count;
	}
	
	ExplosionMotion = CenterPoint * Rand::Double( 10., 40. );
	ExplosionMotion.X += Rand::Double( -5., 5. );
	ExplosionMotion.Y += Rand::Double( -5., 5. );
	ExplosionMotion.Z += Rand::Double( -5., 5. );
	
	NeedsRecalc = false;
}


void ModelObject::CalculateNormals( void )
{
	for( std::map<std::string,ModelArrays>::iterator array_iter = Arrays.begin(); array_iter != Arrays.end(); array_iter ++ )
		array_iter->second.CalculateNormals();
}


Vec3D ModelObject::GetCenter( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return CenterPoint;
}


Vec3D ModelObject::GetExplosionMotion( void )
{
	if( NeedsRecalc )
		Recalc();
	
	return ExplosionMotion;
}


// ---------------------------------------------------------------------------


ModelMaterial::ModelMaterial( std::string name )
{
	Name = name;
	Ambient.Set( 0.2f, 0.2f, 0.2f, 1.f );
	Diffuse.Set( 0.8f, 0.8f, 0.8f, 1.f );
}


ModelMaterial::ModelMaterial( const ModelMaterial &other )
{
	Name = other.Name;
	Texture.BecomeInstance( &(other.Texture) );
	Ambient = other.Ambient;
	Diffuse = other.Diffuse;
	Arrays = other.Arrays;
}


ModelMaterial::~ModelMaterial()
{
	Arrays.Clear();
}


void ModelMaterial::BecomeInstance( const ModelMaterial *other )
{
	Name = other->Name;
	Texture.BecomeInstance( &(other->Texture) );
	Ambient = other->Ambient;
	Diffuse = other->Diffuse;
	Arrays.BecomeInstance( &(other->Arrays) );
}


void ModelMaterial::CalculateNormals( void )
{
	Arrays.CalculateNormals();
}
