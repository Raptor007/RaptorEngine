/*
 *  CheckBox.cpp
 */

#include "CheckBox.h"
#include "Str.h"


CheckBox::CheckBox( SDL_Rect *rect, Font *font, std::string text, bool checked, Animation *u_normal, Animation *u_down, Animation *u_over, Animation *c_normal, Animation *c_down, Animation *c_over )
: LabelledButton( rect, font, text, Font::ALIGN_MIDDLE_LEFT, u_normal, u_down, u_over )
{
	ImageNormalChecked = c_normal;
	ImageMouseDownChecked = c_down;
	ImageMouseOverChecked = c_over;
	
	Checked = checked;
	if( Checked )
		Image.BecomeInstance( ImageNormalChecked );
}


CheckBox::~CheckBox()
{
}


void CheckBox::Draw( void )
{
	// Need this to display a texture
	glEnable( GL_TEXTURE_2D );
	
	// Bind the texture to which subsequent calls refer to
	glBindTexture( GL_TEXTURE_2D, Image.CurrentFrame() );
	
	// Set the color.
	glColor4f( Red, Green, Blue, Alpha );
	
	glBegin( GL_QUADS );
		
		// Top-left
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		
		// Bottom-left
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, Rect.h );
		
		// Bottom-right
		glTexCoord2i( 1, 1 );
		glVertex2i( Rect.h, Rect.h );
		
		// Top-right
		glTexCoord2i( 1, 0 );
		glVertex2i( Rect.h, 0 );
		
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
	
	
	if( LabelFont )
	{
		int x = 0, y = 0;
		
		switch( LabelAlign )
		{
			case Font::ALIGN_TOP_LEFT:
			case Font::ALIGN_TOP_CENTER:
			case Font::ALIGN_TOP_RIGHT:
				y = 0;
				break;
			case Font::ALIGN_MIDDLE_LEFT:
			case Font::ALIGN_MIDDLE_CENTER:
			case Font::ALIGN_MIDDLE_RIGHT:
				y = Rect.h / 2;
				break;
			case Font::ALIGN_BASELINE_LEFT:
			case Font::ALIGN_BASELINE_CENTER:
			case Font::ALIGN_BASELINE_RIGHT:
				y = LabelFont->GetAscent();
				break;
			case Font::ALIGN_BOTTOM_LEFT:
			case Font::ALIGN_BOTTOM_CENTER:
			case Font::ALIGN_BOTTOM_RIGHT:
				y = Rect.h;
				break;
		}
		
		switch( LabelAlign )
		{
			case Font::ALIGN_TOP_LEFT:
			case Font::ALIGN_MIDDLE_LEFT:
			case Font::ALIGN_BASELINE_LEFT:
			case Font::ALIGN_BOTTOM_LEFT:
				x = Rect.h;
				break;
			case Font::ALIGN_TOP_CENTER:
			case Font::ALIGN_MIDDLE_CENTER:
			case Font::ALIGN_BASELINE_CENTER:
			case Font::ALIGN_BOTTOM_CENTER:
				x = (Rect.h + Rect.w) / 2;
				break;
			case Font::ALIGN_TOP_RIGHT:
			case Font::ALIGN_MIDDLE_RIGHT:
			case Font::ALIGN_BASELINE_RIGHT:
			case Font::ALIGN_BOTTOM_RIGHT:
				x = Rect.w;
				break;
		}
		
		if( MouseIsWithin )
		{
			if( MouseIsDown )
				LabelFont->DrawText( LabelText, x, y, LabelAlign, RedDown, GreenDown, BlueDown, AlphaDown );
			else 
				LabelFont->DrawText( LabelText, x, y, LabelAlign, RedOver, GreenOver, BlueOver, AlphaOver );
		}
		else
			LabelFont->DrawText( LabelText, x, y, LabelAlign, Red, Green, Blue, Alpha );
	}
}


void CheckBox::MouseEnter( void )
{
	if( Checked )
	{
		if( ImageMouseDownChecked && MouseIsDown )
			Image.BecomeInstance( ImageMouseDownChecked );
		else if( ImageMouseOverChecked )
			Image.BecomeInstance( ImageMouseOverChecked );
	}
	else
	{
		if( ImageMouseDown && MouseIsDown )
			Image.BecomeInstance( ImageMouseDown );
		else if( ImageMouseOver )
			Image.BecomeInstance( ImageMouseOver );
	}
}


void CheckBox::MouseLeave( void )
{
	if( Checked )
	{
		if( ImageMouseOverChecked || (ImageMouseDownChecked && MouseIsDown) )
			Image.BecomeInstance( ImageNormalChecked );
	}
	else
	{
		if( ImageMouseOver || (ImageMouseDown && MouseIsDown) )
			Image.BecomeInstance( ImageNormal );
	}
}


bool CheckBox::MouseDown( Uint8 button )
{
	if( Checked )
	{
		if( ImageMouseDownChecked )
			Image.BecomeInstance( ImageMouseDownChecked );
	}
	else
	{
		if( ImageMouseDown )
			Image.BecomeInstance( ImageMouseDown );
	}
	
	return true;
}


bool CheckBox::MouseUp( Uint8 button )
{
	if( Checked )
	{
		if( ImageMouseDownChecked )
			Image.BecomeInstance( ImageNormalChecked );
	}
	else
	{
		if( ImageMouseDown )
			Image.BecomeInstance( ImageNormal );
	}
	
	Clicked();
	
	return true;
}


void CheckBox::Clicked( Uint8 button )
{
	Checked = ! Checked;
	
	if( Checked )
	{
		if( MouseIsWithin && ImageMouseOverChecked )
			Image.BecomeInstance( ImageMouseOverChecked );
		else
			Image.BecomeInstance( ImageNormalChecked );
	}
	else
	{
		if( MouseIsWithin && ImageMouseOver )
			Image.BecomeInstance( ImageMouseOver );
		else
			Image.BecomeInstance( ImageNormal );
	}
	
	Changed();
}


void CheckBox::Changed( void )
{
}


void CheckBox::SizeToText( void )
{
	if( LabelFont )
	{
		SDL_Rect rect = {0,0,0,0};
		LabelFont->TextSize( LabelText, &rect );
		Rect.w = Rect.h + rect.w;
	}
}
