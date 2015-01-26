/**************************************************************************
 *	 OGL2Widget.h
 *   Author: kozubovskiy
 *   Date: 10.01.2015
 *************************************************************************/

#ifndef OGL2WIDGET_H
#define OGL2WIDGET_H

#include <QtWidgets/QWidget>
#include <qvector3d.h>
#include <qmatrix4x4.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QVector>
#include <QGLWidget>
#include <QTimer>
#include "Fonts.h"

struct GlParam {	
	QVector<QVector3D> Vertices;
	QVector<QVector3D> Colors;

	QGLShaderProgram SProgramFont;
	QGLShaderProgram SProgram;

	QMatrix4x4 Modelview;
	QMatrix4x4 Proj3D;

	int VertexAttr;
	int ColorUniform;
	int UMatrix;

	int VertexAttrTex;
	int UMatrixTex;
	int VTexCoord;
	int UTexture;

	int VertexAttrTexFont;
	int UMatrixTexFont;
	int VTexCoordFont;
	int ColorUniformFont;
};

class OGL2Widget : public QGLWidget
{
	Q_OBJECT

	QTimer m_RedrawTimer;
	GlParam m_P;
	GlParam *m_GLParam;

	//font
	CFTFont FMS_Fonts; 
	GLuint m_FontID[ CFTFont::FONT_MAX ];

public:
	OGL2Widget( QWidget *parent = 0,  Qt::WindowFlags flags = 0 );
	~OGL2Widget();

protected:
	void paintGL ();
	void initializeGL ();
	void resizeGL(int w, int h);

private:
	void GLDrawRect(float x, float y,  float width, float height,  QColor &ddcolor, float alpha = 255 );

	void CDrawText( QString qstr, float x, float y, CFTFont::FontTextID style, QColor color,CFTFont::TextAlignment align );

	void SetMatrixTransform( QMatrix4x4 *matrixOut, float x, float y, float angle, float shift_x = 0.0f, float shift_y = 0.0f );
	//font
	void GenFonts();
	void DelFonts();
};

#endif // OGL2WIDGET_H
