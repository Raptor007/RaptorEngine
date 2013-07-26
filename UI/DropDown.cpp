/*
 *  DropDown.cpp
 */

#include "DropDown.h"


DropDown::DropDown( Layer *container, SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, Animation *normal, Animation *mouse_down, Animation *mouse_over )
: LabelledButton( container, rect, font, "", align, normal, mouse_down, mouse_over )
{
	ScrollBarSize = scroll_bar_size;
}


DropDown::~DropDown()
{
}


void DropDown::Update( void )
{
	for( std::vector<ListBoxItem>::iterator item_iter = Items.begin(); item_iter != Items.end(); item_iter ++ )
	{
		if( item_iter->Value == Value )
		{
			LabelText = item_iter->Text;
			break;
		}
	}
}


void DropDown::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_WHEELUP )
	{
		// Scrolling up should move to the previous item without showing the listbox.
		
		if( Items.size() )
		{
			bool found = false;
			
			for( size_t i = 1; i < Items.size(); i ++ )
			{
				if( Items[ i ].Value == Value )
				{
					Value = Items[ i - 1 ].Value;
					LabelText = Items[ i - 1 ].Text;
					found = true;
					break;
				}
			}
			
			if( ! found )
			{
				Value = Items.front().Value;
				LabelText = Items.front().Text;
			}
		}
	}
	else if( button == SDL_BUTTON_WHEELDOWN )
	{
		// Scrolling down should move to the next item without showing the listbox.
		
		if( Items.size() )
		{
			bool found = false;
			
			for( size_t i = 0; i < Items.size() - 1; i ++ )
			{
				if( Items[ i ].Value == Value )
				{
					Value = Items[ i + 1 ].Value;
					LabelText = Items[ i + 1 ].Text;
					found = true;
					break;
				}
			}
			
			if( ! found )
			{
				Value = Items.back().Value;
				LabelText = Items.back().Text;
			}
		}
	}
	else
		Container->AddElement( new DropDownListBox(this) );
}


void DropDown::Changed( void )
{
}


// ---------------------------------------------------------------------------


DropDownListBox::DropDownListBox( DropDown *dropdown )
: ListBox( dropdown->Container, &(dropdown->Rect), dropdown->LabelFont, dropdown->ScrollBarSize, dropdown->Items )
{
	CalledBy = dropdown;
	
	Alpha = 1.f;
	
	int selected_index = FindItem( CalledBy->Value );
	if( selected_index >= 0 )
		Rect.y -= selected_index * TextFont->GetHeight();
	
	Rect.h = Items.size() * TextFont->GetHeight();
	if( Rect.h > Container->Rect.h )
		Rect.h = Container->Rect.h;
	
	int offscreen = Rect.y + Rect.h - Container->Rect.h;
	if( offscreen > 0 )
		Rect.y -= offscreen;
	if( Rect.y < 0 )
		Rect.y = 0;
}


DropDownListBox::~DropDownListBox()
{
}


void DropDownListBox::Changed( void )
{
	CalledBy->LabelText = SelectedText();
	CalledBy->Value = SelectedValue();
	CalledBy->Changed();
	Remove();
}


bool DropDownListBox::KeyDown( SDLKey key )
{
	if( key == SDLK_ESCAPE )
		return true;
	
	return false;
}


bool DropDownListBox::KeyUp( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		Remove();
		return true;
	}
	
	return false;
}
