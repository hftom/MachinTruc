#include <QDebug>
#include <QLabel>
#include <QColorDialog>

#include "textedit.h"



TextEdit::TextEdit( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p )
{
	fontColor.setRgb( 0, 0, 0 );
	backgroundColor.setRgb( 0, 0, 0, 0 );
	outlineColor.setRgb( 0, 0, 0 );

	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	
	QBoxLayout *fontLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	fontCombo = new QFontComboBox();
	widgets.append( fontCombo );
	fontLayout->addWidget( fontCombo );
	
	fontSizeSpin = new QSpinBox();
	widgets.append( fontSizeSpin );
	fontSizeSpin->setRange( 1, 999 );
	fontSizeSpin->setValue( 10 );
	fontLayout->addWidget( fontSizeSpin );
	
	boldBtn = new QToolButton();
	widgets.append( boldBtn );
	boldBtn->setIcon( QIcon(":/toolbar/icons/format-text-bold.png") );
	boldBtn->setCheckable( true );
	fontLayout->addWidget( boldBtn );
	
	italicBtn = new QToolButton();
	widgets.append( italicBtn );
	italicBtn->setIcon( QIcon(":/toolbar/icons/format-text-italic.png") );
	italicBtn->setCheckable( true );
	fontLayout->addWidget( italicBtn );
	
	colorBtn = new QToolButton();
	widgets.append( colorBtn );
	colorBtn->setIcon( QIcon(":/toolbar/icons/format-text-color.png") );
	fontLayout->addWidget( colorBtn );
	
	backgroundColorBtn = new QToolButton();
	widgets.append( backgroundColorBtn );
	backgroundColorBtn->setIcon( QIcon(":/toolbar/icons/fill-color.png") );
	fontLayout->addWidget( backgroundColorBtn );
	
	fontLayout->insertStretch( -1, 1 );
	box->addLayout( fontLayout );
	
	QBoxLayout *outlineLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	alignLeftBtn = new QToolButton();
	widgets.append( alignLeftBtn );
	alignLeftBtn->setIcon( QIcon(":/toolbar/icons/format-justify-left.png") );
	alignLeftBtn->setCheckable( true );
	alignLeftBtn->setAutoExclusive( true );
	outlineLayout->addWidget( alignLeftBtn );
	
	alignCenterBtn = new QToolButton();
	widgets.append( alignCenterBtn );
	alignCenterBtn->setIcon( QIcon(":/toolbar/icons/format-justify-center.png") );
	alignCenterBtn->setCheckable( true );
	alignCenterBtn->setAutoExclusive( true );
	outlineLayout->addWidget( alignCenterBtn );
	
	alignRightBtn = new QToolButton();
	widgets.append( alignRightBtn );
	alignRightBtn->setIcon( QIcon(":/toolbar/icons/format-justify-right.png") );
	alignRightBtn->setCheckable( true );
	alignRightBtn->setAutoExclusive( true );
	outlineLayout->addWidget( alignRightBtn );
	
	QLabel *lab = new QLabel( tr("Outline") );
	widgets.append( lab );
	outlineLayout->addWidget( lab );
	
	outlineSizeSpin = new QSpinBox();
	widgets.append( outlineSizeSpin );
	outlineSizeSpin->setRange( 0, 999 );
	outlineLayout->addWidget( outlineSizeSpin );
	
	outlineColorBtn = new QToolButton();
	widgets.append( outlineColorBtn );
	outlineColorBtn->setIcon( QIcon(":/toolbar/icons/format-text-color.png") );
	outlineLayout->addWidget( outlineColorBtn );
	
	outlineLayout->insertStretch( -1, 1 );
	box->addLayout( outlineLayout );
	
	editor = new QTextEdit();
	widgets.append( editor );
	box->addWidget( editor );
	
	QStringList sl = p->value.toString().split("\n");
	if ( sl.count() ) {
		QStringList desc = sl[0].split("|");
		if ( desc.count() == 9 ) {
			QFont f;
			f.fromString( desc[0] );
			f.setPointSize( desc[1].toInt() );
			fontSizeSpin->setValue( desc[1].toInt() );
			f.setBold( desc[2].toInt() );
			boldBtn->setChecked( desc[2].toInt() );
			f.setItalic( desc[3].toInt() );
			italicBtn->setChecked( desc[3].toInt() );
			fontCombo->setCurrentFont( f );
			
			QStringList fc = desc[4].split( "." );
			if ( fc.count() == 2 ) {
				fontColor.setNamedColor( fc[ 0 ] );
				fontColor.setAlpha( fc[ 1 ].toInt() );
			}
			
			QStringList bc = desc[5].split( "." );
			if ( bc.count() == 2 ) {
				backgroundColor.setNamedColor( bc[ 0 ] );
				backgroundColor.setAlpha( bc[ 1 ].toInt() );
			}

			alignLeftBtn->setChecked( false );
			switch ( desc[6].toInt() ) {
				case 1: alignLeftBtn->setChecked( true ); break;
				case 2: alignCenterBtn->setChecked( true ); break;
				case 3: alignRightBtn->setChecked( true ); break;
			}					
			
			outlineSizeSpin->setValue( desc[7].toInt() );
			
			QStringList oc = desc[8].split( "." );
			if ( oc.count() == 2 ) {
				outlineColor.setNamedColor( oc[ 0 ] );
				outlineColor.setAlpha( oc[ 1 ].toInt() );
			}
			
			sl.takeFirst();
			editor->setPlainText( sl.join("\n") );
		}
	}

	connect( fontCombo, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)) );
	connect( fontSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)) );
	connect( boldBtn, SIGNAL(clicked()), this, SLOT(textChanged()) );
	connect( italicBtn, SIGNAL(clicked()), this, SLOT(textChanged()) );
	connect( alignLeftBtn, SIGNAL(clicked()), this, SLOT(textChanged()) );
	connect( alignRightBtn, SIGNAL(clicked()), this, SLOT(textChanged()) );
	connect( alignCenterBtn, SIGNAL(clicked()), this, SLOT(textChanged()) );
	connect( colorBtn, SIGNAL(clicked()), this, SLOT(showColorDialog()) );
	connect( backgroundColorBtn, SIGNAL(clicked()), this, SLOT(showBackgroundColorDialog()) );
	connect( outlineSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(outlineSizeChanged(int)) );
	connect( outlineColorBtn, SIGNAL(clicked()), this, SLOT(showOutlineColorDialog()) );
	connect( editor, SIGNAL(textChanged()), this, SLOT(textChanged()) );
}



void TextEdit::showColorDialog()
{
	QColorDialog dlg;
	dlg.setOption( QColorDialog::ShowAlphaChannel );
	dlg.setOption( QColorDialog::NoButtons );
	dlg.setCurrentColor( fontColor );
	connect( &dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(colorChanged(const QColor&)) );
	dlg.exec();
}



void TextEdit::colorChanged( const QColor &col )
{
	fontColor = col;
	textChanged();
}



void TextEdit::showBackgroundColorDialog()
{
	QColorDialog dlg;
	dlg.setOption( QColorDialog::ShowAlphaChannel );
	dlg.setOption( QColorDialog::NoButtons );
	dlg.setCurrentColor( backgroundColor );
	connect( &dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(backgroundColorChanged(const QColor&)) );
	dlg.exec();
}



void TextEdit::backgroundColorChanged( const QColor &col )
{
	backgroundColor = col;
	textChanged();
}



void TextEdit::showOutlineColorDialog()
{
	QColorDialog dlg;
	dlg.setOption( QColorDialog::ShowAlphaChannel );
	dlg.setOption( QColorDialog::NoButtons );
	dlg.setCurrentColor( outlineColor );
	connect( &dlg, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(outlineColorChanged(const QColor&)) );
	dlg.exec();;
}



void TextEdit::outlineColorChanged( const QColor &col )
{
	outlineColor = col;
	textChanged();
}



void TextEdit::fontSizeChanged( int )
{
	textChanged();
}



void TextEdit::outlineSizeChanged( int )
{
	textChanged();
}



void TextEdit::fontChanged( const QFont& )
{
	textChanged();
}



void TextEdit::textChanged()
{
	int align = 1;
	if ( alignCenterBtn->isChecked() )
		align = 2;
	else if ( alignRightBtn->isChecked() )
		align = 3;

	QString s = QString( "%1|%2|%3|%4|%5|%6|%7|%8|%9\n" ).arg( fontCombo->currentFont().toString() )
												.arg( fontSizeSpin->value() )
												.arg( boldBtn->isChecked() )
												.arg( italicBtn->isChecked() )
												.arg( fontColor.name() + "." + QString::number( fontColor.alpha() ) )
												.arg( backgroundColor.name() + "." + QString::number( backgroundColor.alpha() ) )
												.arg( align )
												.arg( outlineSizeSpin->value() )
												.arg( outlineColor.name() + "." + QString::number( outlineColor.alpha() ) );
	s += editor->toPlainText();
	emit valueChanged( param, QVariant( s ) );
}
