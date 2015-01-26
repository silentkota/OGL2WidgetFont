#ifndef FONT_H
#define FONT_H
#include <vector>
#include <string>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H

#if defined ( _WIN32 )
	typedef __int8 int8_t;
	typedef __int16 int16_t;
	typedef __int32 int32_t;
	typedef __int64 int64_t;
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;
#endif

#include <QImage>
#include <QPixmap>

#define FONT_LEN_WORD 8
#define FONT_NUMBER_CHARS 1500
#define FONT_TEXTURE_MAX_WIDTH 1024
#define FONT_PIX_SHIFT 6

struct vertex_node
{
	vertex_node() : x( 0.0f ), y( 0.0f ), z( 0.0f ){}
	float x, y, z;
};

struct font_border_info
{
	float leftShift;
	float rightSift;
	font_border_info() : leftShift( 0.0f ), rightSift( 0.0f ){}
};

struct uv_node
{
	uv_node() : u( 0.0f ), v( 0.0f ){}
	float u, v;
};

struct Fontcontainer
{
	struct char_info_t
	{
		int x;
		int y;
		int width;
		int height;
		int left;
		int top;
		int advance;
		int row;

		uv_node     uv[4]; //tex glyph
		vertex_node v[4];  //vertex glyph
		uint8_t* bitmap;   //temp bitmap atlas glyph
	};

	struct font_info_t
	{
		int max_height;

		float scaleX;
		float scaleY;
		float scaleSpacing;
		float shiftY; 
		bool useMono;
		float monoWidth;

		std::vector<Fontcontainer::char_info_t> ch;

		font_info_t() : max_height( 0 ), scaleX( 1.0f ), scaleY( 1.0f ), scaleSpacing( 1.0f ), shiftY( 1.0f ), useMono( false ), monoWidth( 13.0f ) {}
	};

	font_info_t info;
};

class CFTFont
{
public:
	enum FontType{ FONT_TTF = 0, FONT_BDF };
	enum FontTextID{ FONT_SMALL, FONT_BASE, FONT_MAX };
	enum TextAlignment { taLEFT = 0, taCENTER, taRIGHT };

private:
	Fontcontainer m_Font[ FONT_MAX ]; 
	int m_FontSize[ FONT_MAX ]; 

	uint8_t* m_TextureData[ FONT_MAX ];
	QImage m_FontPixmap[ FONT_MAX ]; //atlas glyph
	FontTextID m_CurrentFont;
	bool m_Valid;

public:
	CFTFont( );
	virtual ~CFTFont();

	QImage *GetTexture( int );	//get tex for load in GPU
	void SetFont( FontTextID id ){ m_CurrentFont = id; }
	FontTextID GetFont(){ return m_CurrentFont; }

	Fontcontainer::char_info_t *GetFontInfo( int arg ){ 
		return &m_Font[ m_CurrentFont ].info.ch[  arg  ]; 
	}

	float GetStrSize( const char*, int size );
	float GetMaxHeight() { return m_Font[ m_CurrentFont ].info.max_height * m_Font[ m_CurrentFont ].info.scaleY; }

	float GetMonoWidthFont(){ return m_Font[ m_CurrentFont ].info.monoWidth; }

	float GetGlythSiftX(){ return m_Font[ m_CurrentFont ].info.ch[48].left * m_Font[ m_CurrentFont ].info.scaleX; }	//основываясь на символе 0

	font_border_info GetBoarderInfo( const char *str );

	float GetScaleX(){ return m_Font[ m_CurrentFont ].info.scaleX; }
	float GetScaleY(){ return m_Font[ m_CurrentFont ].info.scaleY; }
	float GetScaleSpacing(){ return m_Font[ m_CurrentFont ].info.scaleSpacing; }
	float GetShiftY(){ return m_Font[ m_CurrentFont ].info.shiftY; }
	bool IsValid(){ return m_Valid; }

	void SetMonoMode( bool arg ){  m_Font[ m_CurrentFont ].info.useMono = arg; }
	bool IsMonoMode(){ return m_Font[ m_CurrentFont ].info.useMono; }
	void SetMonoWidth( FontTextID id, float x ){ m_Font[ id ].info.monoWidth = x; }	//set Width glif
private:
	
	void SetScaleX( FontTextID id, float arg ){ m_Font[ id ].info.scaleX = arg; }				//use before LoadFont
	void SetScaleY( FontTextID id, float arg ){ m_Font[ id ].info.scaleY = arg; }				//use before LoadFont
	void SetScaleSpacing( FontTextID id, float arg ){ m_Font[ id ].info.scaleSpacing = arg; }	//use before LoadFont
	void SetShiftY( FontTextID id, float arg ){ m_Font[ id ].info.shiftY = arg; }				//use before LoadFont

	void LoadFont( const std::string& font_file_path, FontType, FontTextID );

	void FillTextureData( uint32_t ch, Fontcontainer::font_info_t* font, uint32_t texture_width, uint8_t* texture_data ); 
	void FillTextureBDFData( uint32_t ch, Fontcontainer::font_info_t* font, uint32_t texture_width, uint8_t* texture_data );
	
	int Excluder( int );		//excluding characters

	inline int next_p2 ( int a )
	{
		int rval = 1;
		while( rval < a ) rval <<= 1;
		return rval;
	}
};


#endif // FONT_H
