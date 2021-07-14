#include <QDebug>
#include <QMessageBox>
#include <QRegularExpressionMatchIterator>

#include "gui/shadercollection.h"
#include "shaderedit.h"



ShaderEdit::ShaderEdit( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p ), progress( NULL )
{
	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	
	QBoxLayout *allLayout = new QBoxLayout( QBoxLayout::TopToBottom );
	
	QBoxLayout *localLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	localComboLabel = new QLabel( tr("Available effects:") );
	widgets.append( localComboLabel );
	localLayout->addWidget( localComboLabel );
	localShadersCombo = new QComboBox();
	widgets.append( localShadersCombo );
	localLayout->addWidget( localShadersCombo );
	localLayout->addSpacerItem( new QSpacerItem( 10, 1 ) );
	editCheckBox = new QCheckBox( tr("Edit...") );
	widgets.append( editCheckBox );
	localLayout->addWidget( editCheckBox );
	localLayout->setStretch( 0, 1 );
	localLayout->setStretch( 1, 1 );
	localLayout->setStretch( 2, 2 );
	allLayout->addLayout( localLayout );
	
	editor = new QPlainTextEdit();
	//QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );
	QFont font( "Sans" );
	//font.setPointSize( 12 );
	editor->setFont( font );
	editor->setStyleSheet( "QPlainTextEdit{ background-color:#FFFEC7; color:black; }" );
	editor->setTabStopWidth( QFontMetrics( font ).averageCharWidth() * 2 );
	highlighter = new Highlighter( editor->document() );
	widgets.append( editor );
	allLayout->addWidget( editor );
	editor->hide();
	
	QBoxLayout *applyLayout = new QBoxLayout( QBoxLayout::LeftToRight );
	applyBtn = new QPushButton( tr("Apply") );
	applyBtn->hide();
	widgets.append( applyBtn );
	helpBtn = new QPushButton( tr("Help") );
	helpBtn->hide();
	widgets.append( helpBtn );
	applyLayout->addWidget( applyBtn );
	applyLayout->insertStretch( 1, 1 );
	applyLayout->addWidget( helpBtn );
	allLayout->addLayout( applyLayout );
	
	localShadersGroup = new QGroupBox();
	localShadersGroup->setLayout( allLayout );
	
	box->addWidget( localShadersGroup );

	editor->setPlainText( p->value.toString() );
	applyBtn->setEnabled( false );

	QStringList list = ShaderCollection::getGlobalInstance()->localShadersNames();
	foreach( const QString & s, list )
		localShadersCombo->addItem( s );

	QString shaderName = Parameter::getShaderName( p->value.toString() );
	for ( int i = 0; i < localShadersCombo->count(); ++i ) {
		if ( localShadersCombo->itemText( i ) == shaderName ) {
			localShadersCombo->setCurrentIndex( i );
			break;
		}
	}	
	
	connect( editor, SIGNAL(textChanged()), this, SLOT(textChanged()) );
	connect( editor, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()) );
	connect( applyBtn, SIGNAL(clicked()), this, SLOT(applyClicked()) );
	connect( helpBtn, SIGNAL(clicked()), this, SLOT(helpClicked()) );
	connect( editCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showEditor(int)) );
	connect( localShadersCombo, SIGNAL(activated(const QString&)), this, SLOT(localShaderChanged(const QString&)) );
}



void ShaderEdit::cursorPositionChanged()
{
	QList<QTextEdit::ExtraSelection> selections;

	int curPos = editor->textCursor().positionInBlock();
	QTextBlock currentBlock = editor->textCursor().block();
	TextBlockData *data = (TextBlockData*)currentBlock.userData();
	if ( !data ) {
		editor->setExtraSelections( selections );
		return;
	}

	int searchPos = curPos;
	int matchPos = -1;
	for ( int i = 0; i < data->parenthesis.count(); ++i ) {
		ParenthesisInfo pi = data->parenthesis[i];
		if ( pi.position == curPos || pi.position == curPos - 1 ) {
			searchPos = currentBlock.position() + pi.position;
			QString s = pi.character;
			if ( s == "(" )
				matchPos = searchNextMatch( currentBlock, i, s, ")" );
			else if ( s == "{" )
				matchPos = searchNextMatch( currentBlock, i, s, "}" );
			else if ( s == "[" )
				matchPos = searchNextMatch( currentBlock, i, s, "]" );
			else if ( s == ")" )
				matchPos = searchPreviousMatch( currentBlock, i, s, "(" );
			else if ( s == "}" )
				matchPos = searchPreviousMatch( currentBlock, i, s, "{" );
			else if ( s == "]" )
				matchPos = searchPreviousMatch( currentBlock, i, s, "[" );
			break;
		}
	}

	if ( matchPos > -1 ) {
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground( Qt::yellow );
		QTextCursor cursor = editor->textCursor();
		cursor.setPosition( searchPos );
		cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
		selection.cursor = cursor;
		selections.append( selection );
		
		cursor.setPosition( matchPos );
		cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
		selection.cursor = cursor;
		selections.append( selection );
	}
	
	editor->setExtraSelections( selections );
}



int ShaderEdit::searchNextMatch( QTextBlock block, int start, QString open, QString close )
{
	int skipNext = 0;
	int i = start + 1;
	
	do {
		TextBlockData *data = (TextBlockData*)block.userData();
		if ( data ) {
			for ( ; i < data->parenthesis.count(); ++i ) {
				ParenthesisInfo pi = data->parenthesis[i];
				QString s = pi.character;
				if ( s == close ) {
					if ( skipNext > 0 )
						--skipNext;
					else
						return block.position() + pi.position;
				}
				else if ( s == open )
					++skipNext;
			}
		}
		block = block.next();
		i = 0;
	} while ( block.isValid() );
	
	return -1;
}



int ShaderEdit::searchPreviousMatch( QTextBlock block, int start, QString close, QString open )
{
	int skipNext = 0;
	--start;
	TextBlockData *data = (TextBlockData*)block.userData();
	
	do {
		if ( data ) {
			for ( int i = start; i >= 0; --i ) {
				ParenthesisInfo pi = data->parenthesis[i];
				QString s = pi.character;
				if ( s == open ) {
					if ( skipNext > 0 )
						--skipNext;
					else
						return block.position() + pi.position;
				}
				else if ( s == close )
					++skipNext;
			}
		}
		block = block.previous();
		data = (TextBlockData*)block.userData();
		if ( data )
			start = data->parenthesis.count() - 1;
	} while ( block.isValid() );
	
	return -1;
}



void ShaderEdit::showEditor( int b )
{
	if ( b == Qt::Checked ) {
		editor->show();
		applyBtn->show();
		helpBtn->show();
	}
	else {
		editor->hide();
		applyBtn->hide();
		helpBtn->hide();
	}
}



void ShaderEdit::helpClicked()
{
	QString help = "The shader entry point is FUNCNAME.\n"
					"Instead of texture2D( sampler, coord ), you sample with INPUT( tc )\n"
					"FUNCNAME returns a vec4 (rgba).\n"
					"ATTENTION: input is and output must be alpha premultiplied.\n"
					"The shader name is set in a comment that starts with //NAME:\n"
					"You can declare user settable parameters in comments:\n"
					"//PARAM : id : ui name : type : default value : min : max : keyframeable\n"
					"where:\n"
					"type = float, rgb or rgba\n"
					"default value = a decimal for float, a hex string for rgb (e.g. ffcc00) and rgba (e.g. ffcc0080)\n"
					"min and max = float only\n"
					"keyframeable = (float only) false or true if this parameter can be animated\n"
					"\n"
					"You then get a uniform that you can access with PREFIX(id)\n"
					"In addition, there are 2 uniforms that are always available:\n"
					"PREFIX(time) = float, time in seconds\n"
					"PREFIX(texelSize) = vec2( 1.0 / imageWidth, 1.0 / imageHeight)\n"
					"\n"
					"Each global name (const or function) must be PREFIXed.\n"
					"\n"
					"Example:\n"
					"\n"
					"//NAME:Example\n"
					"//PARAM:amount:Amount:float: 0.5 : 0 : 1 : false\n"
					"//PARAM:col:Color:rgba:ff0000ff\n"
					"\n"
					"const float PREFIX(pi) = 3.14159265359;\n"
					"\n"
					"float PREFIX(foo)( float f ) {\n"
					"  return f / PREFIX(pi);\n"
					"}\n"
					"\n"
					"vec4 FUNCNAME( vec2 tc ) {\n"
					"  float a = PREFIX(foo)( PREFIX(amount) );\n"
					"  a *= sin( PREFIX(time) );\n"
					"  return mix( INPUT( tc ), PREFIX(col), a );\n"
					"}\n"
					"\n"
					"Note also that any #define must be #undef.\n";

	QMessageBox::information( this, tr("Help"), help );
}



void ShaderEdit::textChanged()
{
	if ( !applyBtn->isEnabled() )
		applyBtn->setEnabled( true );
}



void ShaderEdit::localShaderChanged( const QString & text )
{
	QString shader = ShaderCollection::getGlobalInstance()->getLocalShader( text );
	if ( !shader.isEmpty() ) {
		if ( !shader.endsWith("\n") )
			shader += "\n";
		emit valueChanged( param, QVariant( shader ) );
	}
}



void ShaderEdit::applyClicked()
{
	QString shader = editor->toPlainText();
	int faultyLine;
	QList<Parameter> params = Parameter::parseShaderParams( shader, faultyLine );
	if ( faultyLine != -1 ) {
		QMessageBox::warning( this, tr("Param syntax error"), QString( tr("Syntax error in param declaration at line %1.") ).arg( faultyLine ) );
		applyBtn->setEnabled( false );
		return;
	}
	
	QString name = Parameter::getShaderName( shader );
	if ( name.isEmpty() || name.length() < 3 ) {
		QMessageBox::warning( this, tr("Invalid name"), tr( "The effect name length must be at least 3 chars." ) );
		applyBtn->setEnabled( false );
		return;
	}
	
	if ( ShaderCollection::getGlobalInstance()->localShaderExists( name ) ) {
		QMessageBox::StandardButton ret = QMessageBox::warning( this, tr("Shader exists"),
													QString( tr("%1 already exists\nDo you want to overwrite?") ).arg( name ),
													QMessageBox::Yes | QMessageBox::No,
													QMessageBox::No );
		if ( ret == QMessageBox::No ) {
			applyBtn->setEnabled( false );
			return;
		}
	}
	
	emit compileShaderRequest( ThumbRequest( this, shader + "\n", 1 ) );
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
		QString name = Parameter::getShaderName( shader );
		if ( !ShaderCollection::getGlobalInstance()->saveLocalShader( name, shader ) ) {
			QMessageBox::warning( this, tr("Error"), tr( "Not able to save the shader." ) );
			return;
		}
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

	commentStartExpression = QRegExp( "/\\*" );
	commentEndExpression = QRegExp( "\\*/" );
}



void Highlighter::highlightBlock(const QString &text)
{
	//store parenthesis infos
	TextBlockData *data = new TextBlockData();
	setCurrentBlockUserData( data );
	QRegularExpression re( "\\{|\\}|\\(|\\)|\\[|\\]" );
	QRegularExpressionMatchIterator mi = re.globalMatch( text );
	while ( mi.hasNext() ) {
		QRegularExpressionMatch match = mi.next();
		data->add( match.captured( 0 ), match.capturedStart( 0 ) );
	}

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
