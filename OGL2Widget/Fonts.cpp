/******************************************************************************
 *   Author: Ilya Kozubovskiy
 *   Date: 03.12.2014
 *   Âescription: CFTFont clas for load fonts use freetype2 for OpenGL ES 2.0
 *	 LoadFont, support ttg, bdf format
 *   for add font update FontTextID è m_FontSize
 *   Fontcontainer 
 *			uv_node     uv[4]; //tex
 *			vertex_node  v[4]; //vertex
 ******************************************************************************/
#include "Fonts.h"

#ifdef _WIN32
#pragma comment( lib, "freetype253_D.lib" )
#endif

#include <freetype2/ftbdf.h>

#define FONT_PATH ""

#define FONT_NAME "arial.ttf"
#define LOAD_BDF 0
#define DEBUG_FONTS 0

CFTFont::CFTFont() :
m_Valid( true )
{

#if( LOAD_BDF )
	int FontSize[ FONT_MAX ] = { 8, 24, 16, 24, 32 };
	memcpy ( &m_FontSize, &FontSize, sizeof(FontSize) );

	for( int i = 0; i < FONT_MAX; i++ ) {
		SetScaleX( (FontTextID)i, 0.72f ); 
		SetScaleY( (FontTextID)i, 1.0f ); 
		SetScaleSpacing( (FontTextID)i, 0.72f ); 
		SetShiftY( (FontTextID)i, 3.0f ); 
	}
#else
	int FontSize[ FONT_MAX ] =  { 25, 35 };

	memcpy ( &m_FontSize, &FontSize, sizeof(FontSize) );

	//scale down for better quality
	 for( int i = 0; i < FONT_MAX; i++ ) {
		 SetScaleX( (FontTextID)i, 0.8f ); 
		 SetScaleY( (FontTextID)i, 0.8f ); 
		 SetScaleSpacing( (FontTextID)i, 0.9f ); 
		 SetShiftY( (FontTextID)i, 3.0f ); 
		 
	 }

	 //SetMonoWidth( FONT_SMALL, 20 ); 
	 //SetMonoWidth( FONT_BASE, 30 ); 

#endif

	char fname[100];

#if( LOAD_BDF )
	 sprintf( fname, "%s%s", FONT_PATH, "x2.bdf" );
	 LoadFont( fname, FONT_BDF, FONT_SMALL  );
	 sprintf( fname, "%s%s", FONT_PATH, "x3.bdf" );
	 LoadFont( fname, FONT_BDF, FONT_BASE  );
#else
	sprintf( fname, "%s%s", FONT_PATH, FONT_NAME );

	 //font load
	 for( int i = 0; i < FONT_MAX; i++ ) {
		 LoadFont( fname, FONT_TTF, (FontTextID)i );
	 }
#endif

	 if( m_Valid == false ) {
		 char fname2[100];
		 sprintf( fname2, "Can`t load fonts %s\n", fname );
	 }
}

CFTFont::~CFTFont()
{
	for( int i = 0; i < FONT_MAX; ++i ) {
		delete [] m_TextureData[ i ];
	}
}

void CFTFont::LoadFont( const std::string& font_file_path, FontType fonttype, FontTextID font_id )
{
	// Initialize Freetype
	FT_Library ft_library;
	FT_Face    ft_face;

	if ( FT_Init_FreeType( &ft_library ) ) {
		m_Valid = false;
		return;
	}

	// Load the requested face
	if ( FT_New_Face( ft_library, font_file_path.c_str(), 0, &ft_face ) ) {
		m_Valid = false;
		return;
	}

	if( fonttype == FONT_TTF ) {
		if( FT_Set_Char_Size( ft_face, 0, m_FontSize[ font_id ] << 6, 98, 98) )  //*64 ðàçìåðà â äîëÿõ îò 64
			return;
	} 
	else if ( fonttype == FONT_BDF ) {
		const char* acharset_encoding = "";
		const char* acharset_registry = "";

		FT_Get_BDF_Charset_ID( ft_face, &acharset_encoding, &acharset_registry );
	}

	//Initialize the char_info array
	m_Font[ font_id ].info.ch.resize( FONT_NUMBER_CHARS  );

	// Variables to hold the size the texture must have to hold all the bitmaps
	int max_width = 0;
	int max_rows = 0;

	// Create all the characters
	for( int  ch = 0; ch < FONT_NUMBER_CHARS; ch++ ) {
		ch = Excluder( ch );
		if( ch == FONT_NUMBER_CHARS )
			break;

		FT_UInt glyph_index = 0;
		glyph_index = FT_Get_Char_Index( ft_face, ch );

		//Load the Glyph for our character.
		if( FT_Load_Glyph( ft_face, glyph_index, FT_LOAD_DEFAULT |FT_LOAD_PEDANTIC | FT_LOAD_NO_HINTING | FT_RENDER_MODE_NORMAL  ) ) //FT_LOAD_NO_HINTING FT_LOAD_FORCE_AUTOHINT FT_LOAD_CROP_BITMAP FT_RENDER_MODE_MONO 
			return;

		//Move the face's glyph into a Glyph object.
		FT_Glyph glyph;
		if( FT_Get_Glyph( ft_face->glyph, &glyph ) )
			return;

		//Convert the glyph to a bitmap.
		if(	FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 ) )
			return;

		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		//This reference will make accessing the bitmap easier
		FT_Bitmap& bitmap = bitmap_glyph->bitmap;
		
		//Get a pointer to the char info for easy access
		Fontcontainer::char_info_t* char_info = &m_Font[ font_id ].info.ch[ ch ];

		char_info->width = bitmap.width;
		char_info->height = bitmap.rows;

		uint8_t* pChar_bmp;

		if( fonttype == FONT_TTF ) {  
			//Allocate memory for the bitmap data.
			char_info->bitmap = new uint8_t[ 2* char_info->width * char_info->height ];
			pChar_bmp = char_info->bitmap;

			 //Fill the bitmap data
			for( int j=0; j < bitmap.rows;j++ ) {
				for( int i=0; i < bitmap.width; i++ ) {
						//pChar_bmp[ i+j*char_info->width ] =  (i>=bitmap.width || j>=bitmap.rows) ? 0 : bitmap.buffer[ i + bitmap.width*j ];
						pChar_bmp[ i+j*char_info->width ] =   bitmap.buffer[ i + bitmap.width*j ];
				}
			}
		}
		else { //FONT_BDF
			//Allocate memory for the bitmap data.
			char_info->bitmap = new uint8_t[ bitmap.pitch * bitmap.rows ];
			pChar_bmp = char_info->bitmap;

			//Fill BDF data
			memcpy( char_info->bitmap, bitmap.buffer, bitmap.pitch * bitmap.rows );
		}

#if( DEBUG_FONTS )
		if( ch == 90 ) {
			QImage glyphImage1( bitmap.buffer ,
				char_info->width,
				char_info->height,
				QImage::Format_Indexed8);

			QColor color("green");
			QVector<QRgb> colorTable;

			for ( int c = 0; c < 256; ++c )
				colorTable << qRgba( color.red(), color.green(), color.blue(), c );
			glyphImage1.setColorTable(colorTable);
			glyphImage1.save( "fontGlyph1.png" );

			QImage glyphImage2( pChar_bmp,
				char_info->width,
				char_info->height,
				QImage::Format_Indexed8);

			glyphImage2.save( "fontGlyph2.png" );
		}

#endif

		// Accumulate the width of the bitmaps. Increase the rows if we reached the width of the texture
		max_width += char_info->width + FONT_PIX_SHIFT * 2;
		if ( max_width > FONT_TEXTURE_MAX_WIDTH - 1) {
			max_width = char_info->width + FONT_PIX_SHIFT * 2;
			max_rows++;
		}

		// Determine what's the maximum height a given character might have
		if ( char_info->height > m_Font[ font_id ].info.max_height ) {
			m_Font[ font_id ].info.max_height = char_info->height ;
		}

		// Store information about this character. We will use this to print it.
		char_info->row = max_rows;
		char_info->left = bitmap_glyph->left;
		char_info->top = bitmap_glyph->top;
		char_info->advance = ft_face->glyph->advance.x >> 6;
		char_info->x = max_width - char_info->width - FONT_PIX_SHIFT * 2;

		FT_Done_Glyph( glyph ); //Destroy a given glyph.
	}

	// Multiply the maximum height a character might have by the amount of rows and calculate the proper (power of two) height the texture must have.
	int texture_height = next_p2( ( m_Font[ font_id ].info.max_height + FONT_PIX_SHIFT )*( max_rows + 1 ) );

	// Create the temporary buffer for the texture
	m_TextureData[ font_id ] = new uint8_t[ FONT_TEXTURE_MAX_WIDTH * texture_height * 2 ];

#if( DEBUG_FONTS )
	QImage glyphImage5( m_Font[ font_id ].info.ch[90].bitmap,
		m_Font[ font_id ].info.ch[90].width,
		m_Font[ font_id ].info.ch[90].height,
		QImage::Format_Indexed8);

	QVector<QRgb> colorTable;

	QColor color("green");
	for ( int c = 0; c < 256; ++c )
		colorTable << qRgba( color.red(), color.green(), color.blue(), c );
	glyphImage5.setColorTable(colorTable);

	glyphImage5.save( "font65.png" );
#endif

	// Fill the texture, set the vertex and uv array values and delete the bitmap
	for( int ch = 0; ch < FONT_NUMBER_CHARS; ch++ ) {
		ch = Excluder( ch );
		if( ch == FONT_NUMBER_CHARS )
			break;

		Fontcontainer::char_info_t* char_info = &m_Font[ font_id ].info.ch[ ch ];

		char_info->y = ( m_Font[ font_id ].info.max_height + FONT_PIX_SHIFT )*char_info->row;

		if( fonttype == FONT_TTF)
			FillTextureData( ch, &m_Font[ font_id ].info, FONT_TEXTURE_MAX_WIDTH,  m_TextureData[ font_id ] );
		else if( fonttype == FONT_BDF ) {
			FillTextureBDFData( ch, &m_Font[ font_id ].info, FONT_TEXTURE_MAX_WIDTH,  m_TextureData[ font_id ] );
		}

		float scaleX =  m_Font[ font_id ].info.scaleX;
		float scaleY =  m_Font[ font_id ].info.scaleY;
		float shiftY =  m_Font[ font_id ].info.shiftY;

		//(x,h)
		char_info->uv[0].u = (float)( char_info->x  + FONT_PIX_SHIFT ) / FONT_TEXTURE_MAX_WIDTH;
		char_info->uv[0].v = (float)(-char_info->y  - FONT_PIX_SHIFT ) / texture_height;
		char_info->v[0].x  = char_info->left * scaleX;
		char_info->v[0].y  = ( -char_info->top )  * scaleY - shiftY;

		//(x,y)
		char_info->uv[1].u = (float)( char_info->x  + FONT_PIX_SHIFT ) / FONT_TEXTURE_MAX_WIDTH;               
		char_info->uv[1].v = (float)(-char_info->y - char_info->height - FONT_PIX_SHIFT ) / texture_height;
		char_info->v[1].x  = char_info->left * scaleX;
		char_info->v[1].y  = ( char_info->height  - char_info->top ) * scaleY - shiftY;

		//(w,y)
		char_info->uv[3].u = (float)( char_info->x + char_info->width  + FONT_PIX_SHIFT ) / FONT_TEXTURE_MAX_WIDTH; 
		char_info->uv[3].v = (float)(-char_info->y - char_info->height - FONT_PIX_SHIFT ) / texture_height;
		char_info->v[3].x  = ( char_info->width  + char_info->left ) * scaleX;
		char_info->v[3].y  = ( char_info->height  - char_info->top ) * scaleY - shiftY;

		//(w,h)
		char_info->uv[2].u = (float)( char_info->x + char_info->width + FONT_PIX_SHIFT  ) / FONT_TEXTURE_MAX_WIDTH; 
		char_info->uv[2].v = (float)(-char_info->y - FONT_PIX_SHIFT ) / texture_height;
		char_info->v[2].x  = ( char_info->width + char_info->left ) * scaleX;
		char_info->v[2].y  = ( -char_info->top ) * scaleY - shiftY;

		delete [] m_Font[ font_id ].info.ch[ ch ].bitmap;
	}

	m_FontPixmap[ font_id ] = QImage( m_TextureData[ font_id ],
		FONT_TEXTURE_MAX_WIDTH,
		texture_height,
		QImage::Format_Indexed8 );

	QVector<QRgb> colorTable1;
	for ( int c = 0; c < 256; ++c ){
		colorTable1 << qRgba( 255, 255, 255, c );
	}
	m_FontPixmap[ font_id ].setColorTable( colorTable1 );

#if( DEBUG_FONTS )
	QString fileName = QString( "font%1.png" ).arg( m_FontSize[ font_id ] );
	m_FontPixmap[ font_id ].save( fileName );
#endif

	FT_Done_Face( ft_face );
	FT_Done_FreeType( ft_library );
}

float CFTFont::GetStrSize( const char* str, int len )
{
	float size = 0;
	for( int i = 0; i <  len; i++ ) {
		if( IsMonoMode())
			size +=  GetMonoWidthFont();
		else
			size +=  GetFontInfo( str[i] )->advance;
	}
	return size;
}

QImage *CFTFont::GetTexture( int font_id )
{
	return &m_FontPixmap[ font_id ];
}

int CFTFont::Excluder( int count )
{
	if( count < 32 )
		return 32;
	if( ( count >255 ) && ( count < 1024 ) )
		return 1024;
	if( count > 1279  )
		return FONT_NUMBER_CHARS;
	return count;
}

void CFTFont::FillTextureBDFData( uint32_t ch, Fontcontainer::font_info_t* font, uint32_t texture_width, uint8_t* texture_data )
{
	Fontcontainer::char_info_t* char_info = &font->ch[ ch ];
	uint8_t* char_bmp = char_info->bitmap;

	int bmp_pos = 0;
	int tex_pos = 0;

	int char_x = char_info->x;
	int char_y = char_info->y;
	int char_width = char_info->width;
	int char_height = char_info->height;

	uint8_t* char_bmpNe = new uint8_t[ char_width*char_height ];

	//len pickt str in BDF format, byte
	int len =  ( char_width / FONT_LEN_WORD + ( ( char_width % FONT_LEN_WORD ) ? 1 : 0 ) );

	//convert BDF
	int mask = 0x80;
	for( int bmp_y=0; bmp_y < ( char_height  ); bmp_y++ ) {

		//str in BDF
		uint8_t* str = new uint8_t[ len ];
		memcpy( str, &char_bmp[ bmp_y * len], len );
		int shift = 0;
		for( int bmp_x=0; bmp_x < ( char_width ); bmp_x++ )	{
			bmp_pos =  ( bmp_x + bmp_y * char_width );

			uint8_t pos_str = ( bmp_x  ) / 8;
			uint8_t word = str[ pos_str ];
			word = word << shift;

			if( word & mask )
				char_bmpNe[ bmp_pos ] = 255;
			else
				char_bmpNe[ bmp_pos ] = 0;

			shift ++;
			if( shift == 8 )
				shift = 0;
		}

		delete[] str;
	}

	//clear
	for( int bmp_y=0; bmp_y < ( char_height + FONT_PIX_SHIFT * 2  ); bmp_y++ ) {
		for( int bmp_x=0; bmp_x < ( char_width + FONT_PIX_SHIFT * 2 ); bmp_x++ )	{
			tex_pos =  ( ( char_x + bmp_x ) + ( ( char_y + bmp_y  ) * texture_width ) );
			texture_data[ tex_pos ] = 0;
		}
	}

	//copy
	for( int bmp_y=0; bmp_y < ( char_height  ); bmp_y++ ) {
		for( int bmp_x=0; bmp_x <( char_width ); bmp_x++ )	{
			bmp_pos =  ( bmp_x + bmp_y * char_width );
			tex_pos =  ( ( char_x + bmp_x + FONT_PIX_SHIFT ) + ( ( char_y + bmp_y + FONT_PIX_SHIFT ) * texture_width ) );
			texture_data[ tex_pos ] = char_bmpNe[ bmp_pos ];
		}
	}

	delete [] char_bmpNe;
}

void CFTFont::FillTextureData( uint32_t ch, Fontcontainer::font_info_t* font, uint32_t texture_width, uint8_t* texture_data )
{
	Fontcontainer::char_info_t* char_info = &font->ch[ ch ];
	uint8_t* char_bmp = char_info->bitmap;

	int bmp_pos = 0;
	int tex_pos = 0;

	int char_x = char_info->x;
	int char_y = char_info->y;
	int char_width = char_info->width;
	int char_height = char_info->height;

	//clear
	for( int bmp_y=0; bmp_y < ( char_height + FONT_PIX_SHIFT * 2  ); bmp_y++ ) {
		for( int bmp_x=0; bmp_x <( char_width + FONT_PIX_SHIFT * 2 ); bmp_x++ )	{
			tex_pos =  ( ( char_x + bmp_x ) + ( ( char_y + bmp_y  ) * texture_width ) );
			texture_data[ tex_pos ] = 0;
		}
	}

	//copy glyph
	for( int bmp_y=0; bmp_y < ( char_height  ); bmp_y++ ) {
		for( int bmp_x=0; bmp_x <( char_width ); bmp_x++ )	{
			bmp_pos =  ( bmp_x + bmp_y * char_width );
			tex_pos =  ( ( char_x + bmp_x + FONT_PIX_SHIFT ) + ( ( char_y + bmp_y + FONT_PIX_SHIFT ) * texture_width ) );
			texture_data[ tex_pos ] = char_bmp[ bmp_pos ];
		}
	}
}

font_border_info CFTFont::GetBoarderInfo( const char *str )
{
	 font_border_info info;
	 info.leftShift = 0 * m_Font[ m_CurrentFont ].info.scaleX;
	 info.rightSift = 2 * m_Font[ m_CurrentFont ].info.scaleX;
	 return info;
}


