#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QFontComboBox>
#include <QToolButton>
#include <QSpinBox>
#include <QBoxLayout>

#include "parameterwidget.h"



class TextEdit : public ParameterWidget
{
	Q_OBJECT
public:
	TextEdit( QWidget *parent, Parameter *p );
	QLayout *getLayout() { return box; }
	
	virtual void animValueChanged( double /*val*/ ) {};
	
private slots:
	void fontChanged( const QFont &font );
	void fontSizeChanged( int val );
	void showColorDialog();
	void outlineSizeChanged( int val );
	void showOutlineColorDialog();
	void textChanged();
	
	void outlineColorChanged( const QColor& );
	void colorChanged( const QColor& );
	
private:
	QTextEdit *editor;
	QFontComboBox *fontCombo;
	QSpinBox *fontSizeSpin, *outlineSizeSpin;
	QToolButton *boldBtn, *italicBtn, *colorBtn, *outlineColorBtn;
	QToolButton *alignLeftBtn, *alignCenterBtn, *alignRightBtn;
	QBoxLayout *box;
	
	QColor fontColor, outlineColor;
};

#endif // TEXTEDIT_H