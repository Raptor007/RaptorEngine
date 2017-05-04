/*
 *	Font.cpp
 */

#include "Font.h"

#include <cstddef>
#include "Str.h"
#include "Graphics.h"
#include "ResourceManager.h"
#include "RaptorGame.h"


Font::Font( std::string name, int point_size )
{
	Initialized = false;
	TTFont = NULL;
	
	Name = name;
	PointSize = point_size;
	
	for( int i = 0; i < 256; i ++ )
		Glyphs[ i ].Pic = NULL;
	
	InitFont();
}


Font::~Font()
{
	Initialized = false;
	
	if( TTFont )
		TTF_CloseFont( TTFont );
	TTFont = NULL;
}


void Font::InitFont( void )
{
	Initialized = false;
	
	if( TTFont )
		TTF_CloseFont( TTFont );
	
	TTFont = TTF_OpenFont( Name.c_str(), PointSize );
	if( ! TTFont )
	{
		// FIXME: Use TTF_Error output?
		fprintf( stderr, "Can't open font file: %s\n", Name.c_str() );
		return;
	}
	
	Height = TTF_FontHeight( TTFont );
	Ascent = TTF_FontAscent( TTFont );
	Descent = TTF_FontDescent( TTFont );
	LineSkip = TTF_FontLineSkip( TTFont );
	
	for( int i = 0; i < 256; i ++ )
		Glyphs[ i ].Tex = 0;
	
	LoadedTime.Reset();
	
	Initialized = true;
}


void Font::LoadChar( char c )
{
	if( ! Glyphs[ (unsigned char) c ].Pic )
	{
		char letter[ 2 ] = { c, 0 };
		
		TTF_GlyphMetrics( TTFont, (Uint16) c,
			&(Glyphs[ (unsigned char) c ].MinX), &(Glyphs[ (unsigned char) c ].MaxX),
			&(Glyphs[ (unsigned char) c ].MinY), &(Glyphs[ (unsigned char) c ].MaxY), 
			&(Glyphs[ (unsigned char) c ].Advance) );
		
		SDL_Color foreground = { 255, 255, 255, 255 };
		Glyphs[ (unsigned char) c ].Pic = TTF_RenderText_Blended( TTFont, letter, foreground );
	}
	
	if( (! Glyphs[ (unsigned char) c ].Tex ) && Glyphs[ (unsigned char) c ].Pic )
	{
		GLfloat texcoord[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
		Glyphs[ (unsigned char) c ].Tex = Raptor::Game->Gfx.MakeTexture( Glyphs[ (unsigned char) c ].Pic, GL_LINEAR, GL_CLAMP, texcoord );
		Glyphs[ (unsigned char) c ].TexMinX = texcoord[ 0 ];
		Glyphs[ (unsigned char) c ].TexMinY = texcoord[ 1 ];
		Glyphs[ (unsigned char) c ].TexMaxX = texcoord[ 2 ];
		Glyphs[ (unsigned char) c ].TexMaxY = texcoord[ 3 ];
	}
}


int Font::GetLineSkip( void )
{
	return LineSkip;
}


int Font::GetHeight( void )
{
	return Height;
}


int Font::GetAscent( void )
{
	return Ascent;
}


int Font::GetDescent( void )
{
	return Descent;
}


void Font::TextSize( std::string text, SDL_Rect *r )
{
	if( ! Initialized )
		return;
	
	int advance = 0;
	int w_largest = 0;
	char lastchar = 0;
	
	r->x = 0;
	r->y = 0;
	r->w = 0;
	r->h = Height;
	
	const char *text_ptr = text.c_str();
	while( *text_ptr != '\0' )
	{
		lastchar = *text_ptr;
		if( *text_ptr == '\n' )
		{
			r->h += LineSkip;
			if( r->w > w_largest )
				w_largest = r->w;
			r->w = 0;
		}
		else
		{
			LoadChar( *text_ptr );
			
			advance = Glyphs[ (unsigned char) *text_ptr ].Advance;
			r->w += advance;
		}
		
		text_ptr ++;
	}
	
	if( lastchar != '\n' )
	{
		if( r->w > w_largest )
			w_largest = r->w;
	}
	else
		r->h -= LineSkip;
	
	if( w_largest > r->w )
		r->w = w_largest;
}


int Font::TextHeight( std::string text )
{
	if( ! Initialized )
		return 0;
	
	int h = Height;
	
	const char *text_ptr = text.c_str();
	while( (text_ptr = strchr( text_ptr, '\n' )) )
	{
		h += LineSkip;
		text_ptr ++;
	}
	
	return h;
}


int Font::LineWidth( std::string text )
{
	if( ! Initialized )
		return 0;
	
	int w = 0;
	
	const char *text_ptr = text.c_str();
	while( (*text_ptr != '\0') && (*text_ptr != '\n') )
	{
		LoadChar( *text_ptr );
		w += Glyphs[ (unsigned char) *text_ptr ].Advance;
		text_ptr ++;
	}
	
	return w;
}


void Font::DrawText( std::string text, int x, int y, uint8_t align )
{
	DrawText( text, x, y, align, 1.f, 1.f, 1.f, 1.f );
}


void Font::DrawText( std::string text, int x, int y, uint8_t align, float r, float g, float b, float a )
{
	if( ! Initialized )
		return;
	
	if( LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		InitFont();
	
	// Adjust for text vertical alignment.
	int text_height = TextHeight( text );
	switch( align )
	{
		case ALIGN_MIDDLE_LEFT:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_MIDDLE_RIGHT:
			y -= text_height / 2;
			break;
		case ALIGN_BASELINE_LEFT:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BASELINE_RIGHT:
			y -= Ascent;
			break;
		case ALIGN_BOTTOM_LEFT:
		case ALIGN_BOTTOM_CENTER:
		case ALIGN_BOTTOM_RIGHT:
			y -= text_height;
			break;
	}
	
	GLfloat left = 0.0f, right = 0.0f;
	GLfloat top = 0.0f, bottom = 0.0f;
	GLfloat tex_min_x = 0.0f, tex_min_y = 0.0f;
	GLfloat tex_max_x = 0.0f, tex_max_y = 0.0f;
	GLfloat min_x = 0.0f;
	GLfloat base_left = x;
	
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glEnable( GL_TEXTURE_2D );
	glColor4f( r, g, b, a );
	
	// Adjust for text horizontal alignment.
	int line_width = LineWidth( text );
	switch( align )
	{
		case ALIGN_TOP_CENTER:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BOTTOM_CENTER:
			x -= line_width / 2;
			break;
		case ALIGN_TOP_RIGHT:
		case ALIGN_MIDDLE_RIGHT:
		case ALIGN_BASELINE_RIGHT:
		case ALIGN_BOTTOM_RIGHT:
			x -= line_width;
			break;
	}
	
	const char *text_ptr = text.c_str();
	while( *text_ptr != '\0' )
	{
		if( *text_ptr == '\n' )
		{
			x = base_left;
			y += LineSkip;
			
			// Adjust for text horizontal alignment.
			line_width = LineWidth( text_ptr + 1 );
			switch( align )
			{
				case ALIGN_TOP_CENTER:
				case ALIGN_MIDDLE_CENTER:
				case ALIGN_BASELINE_CENTER:
				case ALIGN_BOTTOM_CENTER:
					x -= line_width / 2;
					break;
				case ALIGN_TOP_RIGHT:
				case ALIGN_MIDDLE_RIGHT:
				case ALIGN_BASELINE_RIGHT:
				case ALIGN_BOTTOM_RIGHT:
					x -= line_width;
					break;
			}
		}
		else
		{
			LoadChar( *text_ptr );
			
			tex_min_x = Glyphs[ (unsigned char) *text_ptr ].TexMinX;
			tex_min_y = Glyphs[ (unsigned char) *text_ptr ].TexMinY;
			tex_max_x = Glyphs[ (unsigned char) *text_ptr ].TexMaxX;
			tex_max_y = Glyphs[ (unsigned char) *text_ptr ].TexMaxY;
			
			min_x = Glyphs[ (unsigned char) *text_ptr ].MinX;
			
			left = x + min_x;
			right = x + Glyphs[ (unsigned char) *text_ptr ].Pic->w + min_x;
			top = y;
			bottom = y + Glyphs[ (unsigned char) *text_ptr ].Pic->h;
			
			glBindTexture( GL_TEXTURE_2D, Glyphs[ (unsigned char) *text_ptr ].Tex );
			
			glBegin( GL_QUADS );
				glTexCoord2f( tex_min_x, tex_min_y ); glVertex2f( left, top );
				glTexCoord2f( tex_max_x, tex_min_y ); glVertex2f( right, top );
				glTexCoord2f( tex_max_x, tex_max_y ); glVertex2f( right, bottom );
				glTexCoord2f( tex_min_x, tex_max_y ); glVertex2f( left, bottom );
			glEnd();
			
			x += Glyphs[ (unsigned char) *text_ptr ].Advance;
		}
		
		text_ptr ++;
	}
	
	glPopAttrib();
}


void Font::DrawText( std::string text, int x1, int y1, int w, int h, uint8_t align )
{
	DrawText( text, x1, y1, w, h, align, 1.f, 1.f, 1.f, 1.f );
}


void Font::DrawText( std::string text, int x1, int y1, int w, int h, uint8_t align, float r, float g, float b, float a )
{
	int x = 0, y = 0;
	
	switch( align )
	{
		case ALIGN_TOP_LEFT:
		case ALIGN_TOP_CENTER:
		case ALIGN_TOP_RIGHT:
			y = y1;
			break;
		case ALIGN_MIDDLE_LEFT:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_MIDDLE_RIGHT:
			y = y1 + h/2;
			break;
		case ALIGN_BASELINE_LEFT:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BASELINE_RIGHT:
			y = y1 + Ascent;
			break;
		case ALIGN_BOTTOM_LEFT:
		case ALIGN_BOTTOM_CENTER:
		case ALIGN_BOTTOM_RIGHT:
			y = y1 + h;
			break;
	}
	
	switch( align )
	{
		case ALIGN_TOP_LEFT:
		case ALIGN_MIDDLE_LEFT:
		case ALIGN_BASELINE_LEFT:
		case ALIGN_BOTTOM_LEFT:
			x = x1;
			break;
		case ALIGN_TOP_CENTER:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BOTTOM_CENTER:
			x = x1 + w/2;
			break;
		case ALIGN_TOP_RIGHT:
		case ALIGN_MIDDLE_RIGHT:
		case ALIGN_BASELINE_RIGHT:
		case ALIGN_BOTTOM_RIGHT:
			x = x1 + w;
			break;
	}
	
	DrawText( text, x, y, align, r, g, b, a );
}


void Font::DrawText( std::string text, const SDL_Rect *rect, uint8_t align )
{
	DrawText( text, rect->x, rect->y, rect->w, rect->h, align, 1.f, 1.f, 1.f, 1.f );
}


void Font::DrawText( std::string text, const SDL_Rect *rect, uint8_t align, float r, float g, float b, float a )
{
	DrawText( text, rect->x, rect->y, rect->w, rect->h, align, r, g, b, a );
}


void Font::DrawText3D( std::string text, const Pos3D *pos, uint8_t align, double scale )
{
	DrawText3D( text, pos, align, 1.f, 1.f, 1.f, 1.f, scale );
}


void Font::DrawText3D( std::string text, const Pos3D *pos, uint8_t align, float r, float g, float b, float a, double scale )
{
	if( ! Initialized )
		return;
	
	if( LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		InitFont();
	
	GLdouble x = 0., y = 0.;
	Pos3D tl, tr, br, bl;
	
	// Adjust for text vertical alignment.
	int text_height = TextHeight( text );
	switch( align )
	{
		case ALIGN_MIDDLE_LEFT:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_MIDDLE_RIGHT:
			y -= text_height / 2;
			break;
		case ALIGN_BASELINE_LEFT:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BASELINE_RIGHT:
			y -= Ascent;
			break;
		case ALIGN_BOTTOM_LEFT:
		case ALIGN_BOTTOM_CENTER:
		case ALIGN_BOTTOM_RIGHT:
			y -= text_height;
			break;
	}
	
	GLdouble left = 0., right = 0.;
	GLdouble top = 0., bottom = 0.;
	GLfloat tex_min_x = 0.f, tex_min_y = 0.f;
	GLfloat tex_max_x = 0.f, tex_max_y = 0.f;
	GLdouble min_x = 0.;
	GLdouble base_left = x;
	
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glEnable( GL_TEXTURE_2D );
	glColor4f( r, g, b, a );
	
	// Adjust for text horizontal alignment.
	int line_width = LineWidth( text );
	switch( align )
	{
		case ALIGN_TOP_CENTER:
		case ALIGN_MIDDLE_CENTER:
		case ALIGN_BASELINE_CENTER:
		case ALIGN_BOTTOM_CENTER:
			x -= line_width / 2;
			break;
		case ALIGN_TOP_RIGHT:
		case ALIGN_MIDDLE_RIGHT:
		case ALIGN_BASELINE_RIGHT:
		case ALIGN_BOTTOM_RIGHT:
			x -= line_width;
			break;
	}
	
	const char *text_ptr = text.c_str();
	while( *text_ptr != '\0' )
	{
		if( *text_ptr == '\n' )
		{
			x = base_left;
			y += LineSkip;
			
			// Adjust for text horizontal alignment.
			line_width = LineWidth( text_ptr + 1 );
			switch( align )
			{
				case ALIGN_TOP_CENTER:
				case ALIGN_MIDDLE_CENTER:
				case ALIGN_BASELINE_CENTER:
				case ALIGN_BOTTOM_CENTER:
					x -= line_width / 2;
					break;
				case ALIGN_TOP_RIGHT:
				case ALIGN_MIDDLE_RIGHT:
				case ALIGN_BASELINE_RIGHT:
				case ALIGN_BOTTOM_RIGHT:
					x -= line_width;
					break;
			}
		}
		else
		{
			LoadChar( *text_ptr );
			
			tex_min_x = Glyphs[ (unsigned char) *text_ptr ].TexMinX;
			tex_min_y = Glyphs[ (unsigned char) *text_ptr ].TexMinY;
			tex_max_x = Glyphs[ (unsigned char) *text_ptr ].TexMaxX;
			tex_max_y = Glyphs[ (unsigned char) *text_ptr ].TexMaxY;
			
			min_x = Glyphs[ (unsigned char) *text_ptr ].MinX;
			
			left = x + min_x;
			right = x + Glyphs[ (unsigned char) *text_ptr ].Pic->w + min_x;
			top = y;
			bottom = y + Glyphs[ (unsigned char) *text_ptr ].Pic->h;
			
			tl.Copy( pos );
			tl.MoveAlong( &(pos->Right), left * scale );
			tl.MoveAlong( &(pos->Up), -top * scale );
			tr.Copy( pos );
			tr.MoveAlong( &(pos->Right), right * scale );
			tr.MoveAlong( &(pos->Up), -top * scale );
			br.Copy( pos );
			br.MoveAlong( &(pos->Right), right * scale );
			br.MoveAlong( &(pos->Up), -bottom * scale );
			bl.Copy( pos );
			bl.MoveAlong( &(pos->Right), left * scale );
			bl.MoveAlong( &(pos->Up), -bottom * scale );
			
			glBindTexture( GL_TEXTURE_2D, Glyphs[ (unsigned char) *text_ptr ].Tex );
			
			glBegin( GL_QUADS );
				glTexCoord2f( tex_min_x, tex_min_y ); glVertex3d( tl.X, tl.Y, tl.Z );
				glTexCoord2f( tex_max_x, tex_min_y ); glVertex3d( tr.X, tr.Y, tr.Z );
				glTexCoord2f( tex_max_x, tex_max_y ); glVertex3d( br.X, br.Y, br.Z );
				glTexCoord2f( tex_min_x, tex_max_y ); glVertex3d( bl.X, bl.Y, bl.Z );
			glEnd();
			
			x += Glyphs[ (unsigned char) *text_ptr ].Advance;
		}
		
		text_ptr ++;
	}
	
	glPopAttrib();
}


// ---------------------------------------------------------------------------


FontID::FontID( std::string name, int point_size )
{
	Name = name;
	PointSize = point_size;
}


bool FontID::operator < ( const FontID &other ) const
{
	int cmp = strcmp( Name.c_str(), other.Name.c_str() );
	if( cmp < 0 )
		return true;
	if( cmp > 0 )
		return false;
	
	return (PointSize < other.PointSize);
}

bool FontID::operator == ( const FontID &other ) const
{
	int cmp = strcmp( Name.c_str(), other.Name.c_str() );
	if( cmp )
		return false;
	
	return PointSize == other.PointSize;
}

bool FontID::operator != ( const FontID &other ) const
{
	return !( *this == other );
}
