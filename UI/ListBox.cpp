/*
 *  ListBox.cpp
 */

#include "ListBox.h"

#include <cstddef>
#include "RaptorGame.h"


ListBox::ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size ) : Layer( rect )
{
	Selected = NULL;
	AllowDeselect = true;
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
	
	VRHeight = 0;
	PadX = 0;
	
	Scroll = 0;
	ClickedScrollBar = false;
	
	AddElement( new ListBoxButton( ListBox::SCROLL_UP,   Raptor::Game->Res.GetAnimation("arrow_up.ani"),   Raptor::Game->Res.GetAnimation("arrow_up_mdown.ani")   ) );
	AddElement( new ListBoxButton( ListBox::SCROLL_DOWN, Raptor::Game->Res.GetAnimation("arrow_down.ani"), Raptor::Game->Res.GetAnimation("arrow_down_mdown.ani") ) );
	UpdateRects();
}


ListBox::ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::vector<ListBoxItem> items ) : Layer( rect )
{
	Selected = NULL;
	AllowDeselect = true;
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
	
	VRHeight = 0;
	PadX = 0;
	
	Scroll = 0;
	ClickedScrollBar = false;
	
	AddElement( new ListBoxButton( ListBox::SCROLL_UP,   Raptor::Game->Res.GetAnimation("arrow_up.ani"),   Raptor::Game->Res.GetAnimation("arrow_up_mdown.ani")   ) );
	AddElement( new ListBoxButton( ListBox::SCROLL_DOWN, Raptor::Game->Res.GetAnimation("arrow_down.ani"), Raptor::Game->Res.GetAnimation("arrow_down_mdown.ani") ) );
	UpdateRects();
	
	Items = items;
}


ListBox::~ListBox()
{
}


void ListBox::AddItem( std::string value, std::string text, const Color *color )
{
	Items.push_back( ListBoxItem( value, text ) );
	
	if( color )
		Items.back().SetColor( color->Red, color->Green, color->Blue, color->Alpha );
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
	int h = Rect.h;
	if( UIScaleMode == Raptor::ScaleMode::IN_PLACE )
		h /= Raptor::Game->UIScale;
	
	return std::max<int>( 0, Items.size() * LineScroll() - h );
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
	
	int x_offset = 0;
	if( ! Raptor::Game->Gfx.DrawTo )
		;
	else if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeL )
		x_offset = VRHeight;
	else if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeR )
		x_offset = -VRHeight;
	
	glColor4f( Red, Green, Blue, Alpha );
	glBegin( GL_QUADS );
		glVertex2i( x_offset, 0 );
		glVertex2i( CalcRect.w + x_offset, 0 );
		glVertex2i( CalcRect.w + x_offset, CalcRect.h );
		glVertex2i( x_offset, CalcRect.h );
	glEnd();
	
	float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
	int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
	
	int max_scroll = MaxScroll();
	if( max_scroll > 0 )
	{
		int rect_h = (UIScaleMode == Raptor::ScaleMode::IN_PLACE) ? (Rect.h / Raptor::Game->UIScale) : Rect.h;
		double percent_displayed = rect_h / (double)( LineScroll() * Items.size() );
		double scroll_start = (Scroll / (double) max_scroll) * (1. - percent_displayed);
		int h = CalcRect.h - scaled_scroll_bar * 2;
		Raptor::Game->Gfx.DrawRect2D( CalcRect.w + x_offset - scaled_scroll_bar, (int)( scaled_scroll_bar + h * scroll_start ), CalcRect.w + x_offset, (int)( scaled_scroll_bar + h * (scroll_start + percent_displayed) ), 0, ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha );
	}
	
	glPushMatrix();
	glPushAttrib( GL_VIEWPORT_BIT );
	if( Raptor::Game->Gfx.DrawTo )  // FIXME: Dirty hack to fix VR text positioning.  Find the real bug elsewhere, then remove this!
		x_offset += ( ((CalcRect.x + CalcRect.w / 2.) / (double) Raptor::Game->Gfx.DrawTo->W) - 0.5 ) * 100.;  // Arbitrary adjustment that looks about right.
	Raptor::Game->Gfx.SetViewport( CalcRect.x + x_offset, CalcRect.y, CalcRect.w - scaled_scroll_bar, CalcRect.h );
	Raptor::Game->Gfx.Setup2D( 0, 0, CalcRect.w - scaled_scroll_bar, CalcRect.h );
	
	if( TextFont )
	{
		SDL_Rect text_rect;
		text_rect.x = PadX * ui_scale;
		text_rect.y = -Scroll * ui_scale;
		text_rect.w = CalcRect.w - scaled_scroll_bar;
		text_rect.h = TextFont->GetHeight() * ui_scale;
		
		for( std::vector<ListBoxItem>::iterator iter = Items.begin(); iter != Items.end(); iter ++ )
		{
			if( (text_rect.y < CalcRect.h) && ((text_rect.y + LineScroll() * ui_scale) >= 0) )
				DrawItem( &(*iter), &text_rect );
			
			text_rect.y += LineScroll() * ui_scale;
		}
	}
	
	glPopAttrib();
	glPopMatrix();
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
}


void ListBox::DrawItem( const ListBoxItem *item, const SDL_Rect *rect )
{
	if( Selected == item )
		TextFont->DrawText( item->Text, rect, TextAlign, SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha, UIScaleMode ? Raptor::Game->UIScale : 1.f );
	else if( item->HasCustomColor )
		TextFont->DrawText( item->Text, rect, TextAlign, item->CustomColor.Red, item->CustomColor.Green, item->CustomColor.Blue, item->CustomColor.Alpha, UIScaleMode ? Raptor::Game->UIScale : 1.f );
	else
		TextFont->DrawText( item->Text, rect, TextAlign, TextRed, TextGreen, TextBlue, TextAlpha, UIScaleMode ? Raptor::Game->UIScale : 1.f );
}


void ListBox::TrackEvent( SDL_Event *event )
{
	Layer::TrackEvent( event );
	
	if( MouseIsDown && ClickedScrollBar && (event->type == SDL_MOUSEMOTION) )
	{
		// Update scroll state as we drag the scroll bar.
		
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
		int h = CalcRect.h - scaled_scroll_bar * 2;
		
		int line = h ? (y / (float) h) * Items.size() - CalcRect.h * 0.5f / LineScroll() : 0;
		if( line < 0 )
			line = 0;
		
		Scroll = 0;
		ScrollDown( line );
	}
}


bool ListBox::HandleEvent( SDL_Event *event )
{
#if SDL_VERSION_ATLEAST(2,0,0)
#if SDL_VERSION_ATLEAST(2,26,0)
	if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y && WithinCalcRect( event->wheel.mouseX, event->wheel.mouseY ) )
#else
	if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y && WithinCalcRect( Raptor::Game->Mouse.X - Raptor::Game->Mouse.OffsetX, Raptor::Game->Mouse.Y - Raptor::Game->Mouse.OffsetY ) ) // FIXME: Messy.
#endif
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
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		ClickedScrollBar = Raptor::Game->Mouse.X > CalcRect.x + CalcRect.w - scaled_scroll_bar;
		if( ClickedScrollBar )
		{
			int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
			int h = CalcRect.h - scaled_scroll_bar * 2;
			
			int line = h ? (y / (float) h) * Items.size() - CalcRect.h * 0.5f / LineScroll() : 0;
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
		
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
		int h = CalcRect.h - scaled_scroll_bar * 2;
		
		int line = h ? (y / (float) h) * Items.size() - CalcRect.h * 0.5f / LineScroll() : 0;
		if( line < 0 )
			line = 0;
		
		Scroll = 0;
		ScrollDown( line );
	}
	else
	{
		int y = (Raptor::Game->Mouse.Y - CalcRect.y) / (UIScaleMode ? Raptor::Game->UIScale : 1.f) + 0.5f;
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
		else if( AllowDeselect )
		{
			Selected = NULL;
			Changed();
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
	HasCustomColor = false;
}


void ListBoxItem::SetColor( float r, float g, float b, float a )
{
	CustomColor.Red   = r;
	CustomColor.Green = g;
	CustomColor.Blue  = b;
	CustomColor.Alpha = a;
	HasCustomColor = true;
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
	
	int scroll_button_w = list_box->ScrollBarSize;
	int scroll_button_h = list_box->ScrollBarSize;
	if( scroll_button_h * 2 > list_box->Rect.h )
		scroll_button_h = list_box->Rect.h / 2;
	
	if( list_box->UIScaleMode == Raptor::ScaleMode::IN_PLACE )
	{
		scroll_button_w *= Raptor::Game->UIScale;
		scroll_button_h *= Raptor::Game->UIScale;
	}
	
	if( Action == ListBox::SCROLL_UP )
	{
		Rect.x = list_box->Rect.w - scroll_button_w;
		Rect.y = 0;
		Rect.w = scroll_button_w;
		Rect.h = scroll_button_h;
	}
	else if( Action == ListBox::SCROLL_DOWN )
	{
		Rect.x = list_box->Rect.w - scroll_button_w;
		Rect.y = list_box->Rect.h - scroll_button_h;
		Rect.w = scroll_button_w;
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

