/*
 *  DropDown.cpp
 */

#include "DropDown.h"

#include "RaptorGame.h"


DropDown::DropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, Animation *normal, Animation *mouse_down, Animation *mouse_over )
: LabelledButton( rect, font, "", align, normal, mouse_down, mouse_over )
{
	ScrollBarSize = scroll_bar_size;
	MyListBox = NULL;
}


DropDown::~DropDown()
{
}


void DropDown::AddItem( std::string value, std::string text )
{
	Items.push_back( ListBoxItem( value, text ) );
}


int DropDown::FindItem( std::string value )
{
	int size = Items.size();
	for( int i = 0; i < size; i ++ )
	{
		try
		{
			if( Items.at( i ).Value == value )
				return i;
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "DropDown::FindItem: std::out_of_range\n" );
		}
	}

	return -1;
}


void DropDown::RemoveItem( std::string value )
{
	int index = FindItem( value );
	RemoveItem( index );
}


void DropDown::RemoveItem( int index )
{
	if( index >= 0 )
	{
		try
		{
			// Create an iterator so the vector::erase method will work properly.
			std::vector<ListBoxItem>::iterator iter = Items.begin() + index;
			Items.erase( iter );
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "DropDown::RemoveItem: std::out_of_range\n" );
		}
	}
}


void DropDown::Clear( void )
{
	Items.clear();
	Selected = NULL;
}


void DropDown::Update( void )
{
	for( std::vector<ListBoxItem>::const_iterator item_iter = Items.begin(); item_iter != Items.end(); item_iter ++ )
	{
		if( item_iter->Value == Value )
		{
			LabelText = item_iter->Text;
			break;
		}
	}
	
	if( MyListBox )
	{
		MyListBox->Items = Items;
		MyListBox->AutoSize();
		MyListBox->ScrollTo( Value, Rect.y - MyListBox->Rect.y );
	}
}


void DropDown::SizeToText( void )
{
	if( LabelFont && Items.size() )
	{
		Uint16 max_w = 0;
		for( std::vector<ListBoxItem>::const_iterator item_iter = Items.begin(); item_iter != Items.end(); item_iter ++ )
		{
			SDL_Rect rect = {0,0,0,0};
			LabelFont->TextSize( item_iter->Text, &rect );
			if( rect.w > max_w )
				max_w = rect.w;
		}
		Rect.w = max_w;
	}
}


bool DropDown::HandleEvent( SDL_Event *event )
{
	bool handled = LabelledButton::HandleEvent( event );
	
	if( (! handled) && MyListBox && ! MouseIsWithin )
	{
		if( event->type == SDL_MOUSEBUTTONDOWN )
			return true;
		else if( event->type == SDL_MOUSEBUTTONUP )
		{
			if( ! MyListBox->ClickedScrollBar )
				Close();
			return true;
		}
	}
	
	return handled;
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
			
			Changed();
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
			
			Changed();
		}
	}
	else if( ! MyListBox )
	{
		MyListBox = new DropDownListBox(this);
		MyListBox->TextAlign = LabelAlign;
		Container->AddElement( MyListBox );
	}
}


void DropDown::Close( void )
{
	MyListBox->Remove();
	MyListBox = NULL;
}


bool DropDown::Select( std::string value )
{
	for( std::vector<ListBoxItem>::const_iterator item_iter = Items.begin(); item_iter != Items.end(); item_iter ++ )
	{
		if( item_iter->Value == value )
		{
			Value = value;
			LabelText = item_iter->Text;
			
			Changed();
			
			return true;
		}
	}
	
	return false;
}


bool DropDown::Select( int index )
{
	if( (index < 0) || (index >= (int) Items.size()) )
		return false;
	
	Value = Items[ index ].Value;
	LabelText = Items[ index ].Text;
	
	Changed();
	
	return true;
}


void DropDown::Changed( void )
{
	if( MyListBox )
		MyListBox->ScrollTo( Value );
}


// ---------------------------------------------------------------------------


DropDownListBox::DropDownListBox( DropDown *dropdown )
: ListBox( &(dropdown->Rect), dropdown->LabelFont, dropdown->ScrollBarSize, dropdown->Items )
{
	CalledBy = dropdown;
	Alpha = 1.f;
	
	AutoSize();
	ScrollTo( CalledBy->Value, CalledBy->Rect.y - Rect.y );
}


DropDownListBox::~DropDownListBox()
{
}


void DropDownListBox::AutoSize( void )
{
	Rect.y = CalledBy->Rect.y;
	
	int min_y = CalledBy->Container ? -(CalledBy->Container->CalcRect.y) : 0;
	int max_h = Raptor::Game->Gfx.H;
	
	int selected_index = FindItem( CalledBy->Value );
	if( selected_index >= 0 )
		Rect.y -= selected_index * LineScroll();
	
	Rect.h = Items.size() * LineScroll();
	
	// FIXME: This check is needed because DropDown::Clicked happens when the dimensions are not the VR eye.
	if( !( Raptor::Game->Head.VR && Raptor::Game->Cfg.SettingAsBool("vr_enable") ) )
	{
		if( Rect.h > max_h )
			Rect.h = max_h;
		
		int offscreen = (CalledBy->Container ? CalledBy->Container->CalcRect.y : 0) + Rect.y + Rect.h - Raptor::Game->Gfx.H;
		if( offscreen > 0 )
			Rect.y -= offscreen;
		if( Rect.y < min_y )
			Rect.y = min_y;
	}
}


void DropDownListBox::Changed( void )
{
	CalledBy->LabelText = SelectedText();
	CalledBy->Value = SelectedValue();
	CalledBy->MyListBox = NULL;
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
		CalledBy->Close();
		return true;
	}
	
	return false;
}
