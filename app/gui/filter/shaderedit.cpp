#include <QDebug>
#include <QMessageBox>

#include "shaderedit.h"



ShaderEdit::ShaderEdit( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p ), progress( NULL )
{
	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	
	editor = new QPlainTextEdit();
	//QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );
	QFont font( "Sans" );
	font.setPointSize( 12 );
	editor->setFont( font );
	editor->setStyleSheet( "QPlainTextEdit{ background-color:#FFFEC7; color:black; }" );
	editor->setTabStopWidth( QFontMetrics( font ).averageCharWidth() * 2 );
	highlighter = new Highlighter( editor->document() );
	widgets.append( editor );
	box->addWidget( editor );
	
	QBoxLayout *applyLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	applyBtn = new QPushButton( tr("Apply") );
	widgets.append( applyBtn );
	applyLayout->addWidget( applyBtn );
	applyLayout->insertStretch( -1, 1 );
	box->addLayout( applyLayout );
	
	editor->setPlainText( p->value.toString() );
	applyBtn->setEnabled( false );
	
	connect( editor, SIGNAL(textChanged()), this, SLOT(textChanged()) );
	connect( applyBtn, SIGNAL(clicked()), this, SLOT(applyClicked()) );
}



void ShaderEdit::textChanged()
{
	applyBtn->setEnabled( true );
}



void ShaderEdit::applyClicked()
{
	int faultyLine;
	QList<Parameter> params = Parameter::parseShaderParams( editor->toPlainText(), faultyLine );
	if ( faultyLine != -1 ) {
		QMessageBox::warning( this, "Param syntax error", QString( "Syntax error in param declaration at line %1." ).arg( faultyLine ) );
		applyBtn->setEnabled( false );
		return;
	}
	
	emit compileShaderRequest( ThumbRequest( this, editor->toPlainText() + "\n", 1 ) );
	progress = new QProgressDialog( this );
	progress->setMinimum( 0 );
	progress->setMaximum( 0 );
	progress->setMinimumDuration( 0 );
	progress->setCancelButton( 0 );
	progress->setLabelText( tr( "Compiling shader ..." ) );
	progress->exec();
}



void ShaderEdit::setCompileResult( QString result )
{
	applyBtn->setEnabled( false );

	if ( progress ) {
		progress->reset();
		delete progress;
		progress = NULL;
	}

	if ( result.startsWith( "ok" ) ) {
		QString shader = editor->toPlainText();
		if ( !shader.endsWith("\n") )
			shader += "\n";
		emit valueChanged( param, QVariant( shader ) );
	}
	else {
		if ( result.startsWith( "nok" ) )
			result.remove( 0, 3 );
		QMessageBox::warning( this, "Compiler error", result );
	}
}



Highlighter::Highlighter( QTextDocument *parent )
	: QSyntaxHighlighter( parent )
{
	HighlightingRule rule;

	keywordFormat.setForeground( Qt::black );
	keywordFormat.setFontWeight( QFont::Bold );
	QStringList keywordPatterns;
	keywordPatterns << "\\bconst\\b" << "\\bbreak\\b" << "\\bcontinue\\b"
					<< "\\bdo\\b" << "\\bfor\\b" << "\\bwhile\\b"
					<< "\\bif\\b" << "\\belse\\b" << "\\btrue\\b"
					<< "\\bfalse\\b" << "\\bdiscard\\b" << "\\breturn\\b"
					<< "\\bstruct\\b" << "\\bswitch\\b" << "\\bcase\\b"
					<< "\\bdefault\\b";
	foreach( const QString &pattern, keywordPatterns ) {
		rule.pattern = QRegExp( pattern );
		rule.format = keywordFormat;
		highlightingRules.append( rule );
	}
	
	keywordFormat.setForeground( Qt::darkRed );
	keywordFormat.setFontWeight( QFont::Normal );
	keywordPatterns.clear();
	keywordPatterns << "\\bfloat\\b" << "\\bint\\b" << "\\bvoid\\b"
					<< "\\bbool\\b" << "\\bmat2\\b" << "\\bmat3\\b"
					<< "\\bmat4\\b" << "\\bvec2\\b" << "\\bvec3\\b"
					<< "\\bvec4\\b" << "\\bivec2\\b" << "\\bivec3\\b"
					<< "\\bivec4\\b" << "\\bbvec2\\b" << "\\bbvec3\\b"
					<< "\\bbvec4\\b" << "\\bmat2x2\\b" << "\\bmat3x2\\b"
					<< "\\bmat4x2\\b" << "\\bmat2x3\\b" << "\\bmat3x3\\b"
					<< "\\bmat4x3\\b" << "\\bmat2x4\\b" << "\\bmat3x4\\b"
					<< "\\bmat4x4\\b" << "\\buint\\b" << "\\buvec2\\b"
					<< "\\buvec3\\b" << "\\buvec4\\b";
	foreach( const QString &pattern, keywordPatterns ) {
		rule.pattern = QRegExp( pattern );
		rule.format = keywordFormat;
		highlightingRules.append( rule );
	}
	
	keywordFormat.setForeground( Qt::darkGreen );
	keywordPatterns.clear();
	keywordPatterns << "\\bradians\\b" << "\\bdegrees\\b" << "\\bsin\\b"
					<< "\\bcos\\b" << "\\btan\\b" << "\\basin\\b"
					<< "\\bacos\\b" << "\\bpow\\b" << "\\bexp\\b"
					<< "\\blog\\b" << "\\bexp2\\b" << "\\blog2\\b"
					<< "\\bsqrt\\b" << "\\binversesqrt\\b" << "\\babs\\b"
					<< "\\bsign\\b" << "\\bfloor\\b" << "\\bceil\\b"
					<< "\\bfract\\b" << "\\bmod\\b" << "\\bmin\\b"
					<< "\\bmax\\b" << "\\bclamp\\b" << "\\bmix\\b"
					<< "\\bstep\\b" << "\\bsmoothstep\\b" << "\\blength\\b"
					<< "\\bdistance\\b" << "\\bdot\\b" << "\\bcross\\b"
					<< "\\bnormalize\\b" << "\\bfaceforward\\b" << "\\breflect\\b"
					<< "\\brefract\\b" << "\\bmatrixCompMult\\b" << "\\blessThan\\b"
					<< "\\blessThanEqual\\b" << "\\bgreaterThan\\b" << "\\bgreaterThanEqual\\b"
					<< "\\bequal\\b" << "\\bnotEqual\\b" << "\\bany\\b"
					<< "\\ball\\b" << "\\bnot\\b" << "\\bdFdx\\b"
					<< "\\bdFdy\\b" << "\\bfwidth\\b" << "\\btrunc\\b"
					<< "\\bround\\b" << "\\broundEven\\b" << "\\bmodf\\b"
					<< "\\bisnan\\b" << "\\bisinf\\b" << "\\bsinh\\b"
					<< "\\bcosh\\b" << "\\btanh\\b" << "\\basinh\\b"
					<< "\\bacosh\\b" << "\\batanh\\b" << "\\bdeterminant\\b"
					<< "\\binverse\\b";
	foreach( const QString &pattern, keywordPatterns ) {
		rule.pattern = QRegExp( pattern );
		rule.format = keywordFormat;
		highlightingRules.append( rule );
	}
	
	keywordFormat.setForeground( Qt::blue );
	keywordPatterns.clear();
	keywordPatterns << "\\bINPUT\\b" << "\\bPREFIX\\b" << "\\bFUNCNAME\\b";
	foreach( const QString &pattern, keywordPatterns ) {
		rule.pattern = QRegExp( pattern );
		rule.format = keywordFormat;
		highlightingRules.append( rule );
	}

	singleLineCommentFormat.setForeground( Qt::darkGray );
	rule.pattern = QRegExp( "//[^\n]*" );
	rule.format = singleLineCommentFormat;
	highlightingRules.append( rule );

	multiLineCommentFormat.setForeground( Qt::darkGray );

	/*classFormat.setFontWeight( QFont::Bold );
	classFormat.setForeground( Qt::darkMagenta );
	rule.pattern = QRegExp( "\\bQ[A-Za-z]+\\b" );
	rule.format = classFormat;
	highlightingRules.append( rule );	
	
	quotationFormat.setForeground( Qt::red );
	rule.pattern = QRegExp( "\".*\"" );
	rule.format = quotationFormat;
	highlightingRules.append( rule );

	functionFormat.setFontItalic( true );
	functionFormat.setForeground( Qt::blue );
	rule.pattern = QRegExp( "\\b[A-Za-z0-9_]+(?=\\()" );
	rule.format = functionFormat;
	highlightingRules.append( rule );*/

	commentStartExpression = QRegExp( "/\\*" );
	commentEndExpression = QRegExp( "\\*/" );
}



void Highlighter::highlightBlock(const QString &text)
{
	foreach( const HighlightingRule &rule, highlightingRules ) {
		QRegExp expression( rule.pattern );
		int index = expression.indexIn( text );
		while ( index >= 0 ) {
			int length = expression.matchedLength();
			setFormat( index, length, rule.format );
			index = expression.indexIn( text, index + length );
		}
	}
	setCurrentBlockState( 0 );

	int startIndex = 0;
	if ( previousBlockState() != 1 )
		startIndex = commentStartExpression.indexIn( text );

	while (startIndex >= 0) {
		int endIndex = commentEndExpression.indexIn( text, startIndex );
		int commentLength;
		if ( endIndex == -1 ) {
			setCurrentBlockState( 1 );
			commentLength = text.length() - startIndex;
		} else {
			commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
		}
		setFormat( startIndex, commentLength, multiLineCommentFormat );
		startIndex = commentStartExpression.indexIn( text, startIndex + commentLength );
	}
}
