#ifndef SHADEREDIT_H
#define SHADEREDIT_H

#include <QSyntaxHighlighter>
#include <QProgressDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QBoxLayout>

#include "engine/thumbnailer.h"
#include "parameterwidget.h"



class ParenthesisInfo
{
public:
	ParenthesisInfo() {}
	ParenthesisInfo( QString c, int pos ) : character( c ),  position( pos ) {}
	QString character;
	int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
	void add( QString c, int pos ) {
		parenthesis.append( ParenthesisInfo( c, pos ) );
	}
	QVector<ParenthesisInfo> parenthesis;
};



class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	Highlighter( QTextDocument *parent = 0 );

protected:
	void highlightBlock( const QString &text ) Q_DECL_OVERRIDE;

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
    };
	QVector<HighlightingRule> highlightingRules;

	QRegExp commentStartExpression;
	QRegExp commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
};



class ShaderEdit : public ParameterWidget
{
	Q_OBJECT
public:
	ShaderEdit( QWidget *parent, Parameter *p );
	QLayout *getLayout() { return box; }
	void setCompileResult( QString result );
	
	virtual void animValueChanged( double /*val*/ ) {};
	
private slots:
	void localShaderChanged( const QString & text );
	void textChanged();
	void cursorPositionChanged();
	void applyClicked();
	void helpClicked();
	void showEditor( int b );
	
private:
	int searchNextMatch( QTextBlock block, int start, QString open, QString close );
	int searchPreviousMatch( QTextBlock block, int start, QString close, QString open );

	Highlighter *highlighter;
	QComboBox *localShadersCombo;
	QPlainTextEdit *editor;
	QPushButton *applyBtn, *helpBtn;
	QCheckBox *editCheckBox;
	QBoxLayout *box;
	QProgressDialog *progress;
	
signals:
	void compileShaderRequest( ThumbRequest );
};

#endif // SHADEREDIT_H