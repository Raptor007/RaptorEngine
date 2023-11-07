/*
 *  ListBox.cpp
 */

#include "ListBox.h"

#include <cstddef>
#include "RaptorGame.h"


ListBox::ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size ) : Layer( rect )
{
	Selected = NULL;
	TextAlign = Font::ALIGN_TOP_LEFT;
	TextFont = font;
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 0.0f;
	Alpha = 0.75f;
	
	TextRed = 1.0f;
	TextGreen = 1.0f;
	TextBlue = 1.0f;
	TextAlpha = 1.0f;
	
	SelectedRed = 1.0f;
	SelectedGreen = 1.0f;
	SelectedBlue = 0.0f;
	SelectedAlpha = 1.0f;
	
	ScrollBarSize = scroll_bar_size;
	ScrollBarRed = 1.0f;
	ScrollBarGreen = 1.0f;
	ScrollBarBlue = 1.0f;
	ScrollBarAlpha = 1.0f;
	
	Scroll = 0;
	ClickedScrollBar = false;
	
	AddElement( new ListBoxButton( ListBox::SCROLL_UP, Raptor::Game->Res.GetAnimation("arrow_up.ani"), Raptor::Game->Res.GetAnimation("arrow_up_mdown.ani") ) );
	AddElement( new ListBoxButton( ListBox::SCROLL_DOWN, Raptor::Game->Res.GetAnimation("arrow_down.ani"), Raptor::Game->Res.GetAnimation("arrow_down_mdown.ani") ) );
	UpdateRects();
}


ListBox::ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::vector<ListBoxItem> items ) : Layer( rect )
{
	Selected = NULL;
	TextAlign = Font::ALIGN_TOP_LEFT;
	TextFont = font;
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 0.0f;
	Alpha = 0.75f;
	
	TextRed = 1.0f;
	TextGreen = 1.0f;
	TextBlue = 1.0f;
	TextAlpha = 1.0f;
	
	SelectedRed = 1.0f;
	SelectedGreen = 1.0f;
	SelectedBlue = 0.0f;
	SelectedAlpha = 1.0f;
	
	ScrollBarSize = scroll_bar_size;
	ScrollBarRed = 1.0f;
	ScrollBarGreen = 1.0f;
	ScrollBarBlue = 1.0f;
	ScrollBarAlpha = 1.0f;
	
	Scroll = 0;
	ClickedScrollBar = false;
	
	AddElement( new ListBoxButton( ListBox::SCROLL_UP, Raptor::Game->Res.GetAnimation("arrow_up.ani"), Raptor::Game->Res.GetAnimation("arrow_up_mdown.ani") ) );
	AddElement( new ListBoxButton( ListBox::SCROLL_DOWN, Raptor::Game->Res.GetAnimation("arrow_down.ani"), Raptor::Game->Res.GetAnimation("arrow_down_mdown.ani") ) );
	UpdateRects();
	
	Items = items;
}


ListBox::~ListBox()
{
}


void ListBox::AddItem( std::string value, std::string text )
{
	Items.push_back( ListBoxItem( value, text ) );
}


int ListBox::FindItem( std::string value )
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
			fprintf( stderr, "ListBox::FindItem: std::out_of_range\n" );
		}
	}

	return -1;
}


void ListBox::RemoveItem( std::string value )
{
	int index = FindItem( value );
	RemoveItem( index );
}


void ListBox::RemoveItem( int index )
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
			fprintf( stderr, "ListBox::RemoveItem: std::out_of_range\n" );
		}
	}
}


void ListBox::Clear( void )
{
	Items.clear();
	Selected = NULL;
}


int ListBox::LineScroll( void )
{
	if( TextFont )
		return TextFont->GetLineSkip();
	return 10;
}


int ListBox::MaxScroll( void )
{
	int max_scroll = 0;
	max_scroll = Items.size() * LineScroll() - Rect.h;
	
	if( max_scroll >= 0 )
		return max_scroll;
	return 0;
}


void ListBox::ScrollUp( int lines )
{
	Scroll -= LineScroll() * lines;
	
	if( Scroll < 0 )
		Scroll = 0;
}


void ListBox::ScrollDown( int lines )
{
	Scroll += LineScroll() * lines;
	
	int max_scroll = MaxScroll();
	if( Scroll > max_scroll )
		Scroll = max_scroll;
}


void ListBox::ScrollTo( std::string value, int at )
{
	for( size_t i = 0; i < Items.size(); i ++ )
	{
		if( Items[ i ].Value == value )
		{
			ScrollTo( (int) i, at );
			break;
		}
	}
}


void ListBox::ScrollTo( int index, int at )
{
	Scroll = LineScroll() * index - at;
	
	if( Scroll < 0 )
		Scroll = 0;
	else
	{
		int max_scroll = MaxScroll();
		if( Scroll > max_scroll )
			Scroll = max_scroll;
	}
}


void ListBox::ScrollToSelected( int at )
{
	ScrollTo( SelectedValue(), at );
}


void ListBox::UpdateRects( void )
{
	// FIXME: This method assumes the only elements are ListBoxButtons.
	for( std::list<Layer*>::iterator element_iter = Elements.begin(); element_iter != Elements.end(); element_iter ++ )
	{
		ListBoxButton *button = (ListBoxButton*)(*element_iter);
		button->UpdateRect();
	}
}


void ListBox::Draw( void )
{
	UpdateRects();
	
	glColor4f( Red, Green, Blue, Alpha );
	glBegin( GL_QUADS );
		glVertex2i( 0, 0 );
		glVertex2i( Rect.w, 0 );
		glVertex2i( Rect.w, Rect.h );
		glVertex2i( 0, Rect.h );
	glEnd();
	
	int max_scroll = MaxScroll();
	if( max_scroll > 0 )
	{
		double percent_displayed = Rect.h / (double)( LineScroll() * Items.size() );
		double scroll_start = (Scroll / (double) max_scroll) * (1. - percent_displayed);
		int h = Rect.h - ScrollBarSize * 2;
		Raptor::Game->Gfx.DrawRect2D( Rect.w - ScrollBarSize, (int)( ScrollBarSize + h * scroll_start ), Rect.w, (int)( ScrollBarSize + h * (scroll_start + percent_displayed) ), 0, ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha );
	}
	
	glPushMatrix();
	glPushAttrib( GL_VIEWPORT_BIT );
	Raptor::Game->Gfx.Setup2D( 0, 0, Rect.w - ScrollBarSize, Rect.h );
	Raptor::Game->Gfx.SetViewport( CalcRect.x, CalcRect.y, Rect.w - ScrollBarSize, Rect.h );
	
	if( TextFont )
	{
		SDL_Rect text_rect;
		text_rect.x = 0;
		text_rect.y = -Scroll;
		text_rect.w = Rect.w - ScrollBarSize;
		text_rect.h = TextFont->GetHeight();
		
		for( std::vector<ListBoxItem>::iterator iter = Items.begin(); iter != Items.end(); iter ++ )
		{
			if( (text_rect.y < Rect.h) && ((text_rect.y + LineScroll()) >= 0) )
				DrawItem( &(*iter), &text_rect );
			
			text_rect.y += LineScroll();
		}
	}
	
	glPopAttrib();
	glPopMatrix();
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
}


void ListBox::DrawItem( const ListBoxItem *item, const SDL_Rect *rect )
{
	if( Selected == item )
		TextFont->DrawText( item->Text, rect, TextAlign, SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha );
	else
		TextFont->DrawText( item->Text, rect, TextAlign, TextRed, TextGreen, TextBlue, TextAlpha );
}


void ListBox::TrackEvent( SDL_Event *event )
{
	Layer::TrackEvent( event );
	
	if( MouseIsDown && ClickedScrollBar && (event->type == SDL_MOUSEMOTION) )
	{
		// Update scroll state as we drag the scroll bar.
		
		int y = Raptor::Game->Mouse.Y - CalcRect.y - ScrollBarSize;
		int h = Rect.h - ScrollBarSize * 2;
		
		int line = h ? (y / (float) h) * Items.size() - Rect.h * 0.5f / LineScroll() : 0;
		if( line < 0 )
			line = 0;
		
		Scroll = 0;
		ScrollDown( line );
	}
}


bool ListBox::HandleEvent( SDL_Event *event )
{
#if SDL_VERSION_ATLEAST(2,0,0)
	if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y && WithinCalcRect( event->wheel.mouseX, event->wheel.mouseY ) )
	{
		if( event->wheel.y > 0 )
			ScrollUp();
		else
			ScrollDown();
		return true;
	}
#endif
	
	return Layer::HandleEvent( event );
}


void ListBox::MouseEnter( void )
{
}


void ListBox::MouseLeave( void )
{
}


bool ListBox::MouseDown( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
	{
		ClickedScrollBar = Raptor::Game->Mouse.X > CalcRect.x + CalcRect.w - ScrollBarSize;
		if( ClickedScrollBar )
		{
			int y = Raptor::Game->Mouse.Y - CalcRect.y - ScrollBarSize;
			int h = Rect.h - ScrollBarSize * 2;
			
			int line = h ? (y / (float) h) * Items.size() - Rect.h * 0.5f / LineScroll() : 0;
			if( line < 0 )
				line = 0;
			
			Scroll = 0;
			ScrollDown( line );
		}
	}
	
	return true;
}


bool ListBox::MouseUp( Uint8 button )
{
	if( button == SDL_BUTTON_WHEELDOWN )
		ScrollDown();
	else if( button == SDL_BUTTON_WHEELUP )
		ScrollUp();
	else if( (button == SDL_BUTTON_LEFT) && ClickedScrollBar )
	{
		ClickedScrollBar = false;
		
		int y = Raptor::Game->Mouse.Y - CalcRect.y - ScrollBarSize;
		int h = Rect.h - ScrollBarSize * 2;
		
		int line = h ? (y / (float) h) * Items.size() - Rect.h * 0.5f / LineScroll() : 0;
		if( line < 0 )
			line = 0;
		
		Scroll = 0;
		ScrollDown( line );
	}
	else
	{
		int y = Raptor::Game->Mouse.Y - CalcRect.y;
		
		int index = (y + Scroll) / LineScroll();
		
		if( (size_t) index < Items.size() )
		{
			try
			{
				Selected = &(Items.at( index ));
				Changed();
			}
			catch( std::out_of_range &exception )
			{
				fprintf( stderr, "ListBox::MouseUp: std::out_of_range\n" );
			}
		}
	}
	
	return true;
}


void ListBox::Changed( void )
{
}


std::string ListBox::SelectedValue( void )
{
	if( Selected )
		return Selected->Value;
	return "";
}


std::string ListBox::SelectedText( void )
{
	if( Selected )
		return Selected->Text;
	return "";
}


void ListBox::Select( std::string value )
{
	for( std::vector<ListBoxItem>::iterator iter = Items.begin(); iter != Items.end(); iter ++ )
	{
		if( (*iter).Value == value )
		{
			Selected = &(*iter);
			Changed();
			break;
		}
	}
}


void ListBox::Select( int index )
{
	if( index < 0 )
	{
		Selected = NULL;
		Changed();
	}
	else if( (size_t) index < Items.size() )
		Select( Items[ index ].Value );
}


// ---------------------------------------------------------------------------


ListBoxItem::ListBoxItem( std::string value, std::string text )
{
	Value = value;
	Text = text;
}


// ---------------------------------------------------------------------------


ListBoxButton::ListBoxButton( uint8_t action, Animation *normal, Animation *mouse_down ) : Button( NULL, normal, mouse_down )
{
	Action = action;
}


void ListBoxButton::UpdateRect( void )
{
	if( ! Container )
		return;
	
	ListBox *list_box = (ListBox*) Container;
	
	int scroll_button_h = list_box->ScrollBarSize;
	if( scroll_button_h * 2 > list_box->Rect.h )
		scroll_button_h = list_box->Rect.h / 2;
	
	if( Action == ListBox::SCROLL_UP )
	{
		Rect.x = list_box->Rect.w - list_box->ScrollBarSize;
		Rect.y = 0;
		Rect.w = list_box->ScrollBarSize;
		Rect.h = scroll_button_h;
	}
	else if( Action == ListBox::SCROLL_DOWN )
	{
		Rect.x = list_box->Rect.w - list_box->ScrollBarSize;
		Rect.y = list_box->Rect.h - list_box->ScrollBarSize;
		Rect.w = list_box->ScrollBarSize;
		Rect.h = scroll_button_h;
	}
}


void ListBoxButton::Clicked( Uint8 button )
{
	ListBox *list_box = (ListBox*) Container;
	if( (Action == ListBox::SCROLL_UP) && list_box )
		list_box->ScrollUp();
	else if( (Action == ListBox::SCROLL_DOWN) && list_box )
		list_box->ScrollDown();
}

