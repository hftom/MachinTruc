#ifndef SHADEREDIT_H
#define SHADEREDIT_H

#include <QSyntaxHighlighter>
#include <QProgressDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>

#include "engine/thumbnailer.h"
#include "parameterwidget.h"



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
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
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
	void textChanged();
	void applyClicked();
	void helpClicked();
	void showEditor( int b );
	
private:
	Highlighter *highlighter;
	QPlainTextEdit *editor;
	QPushButton *applyBtn, *helpBtn;
	QCheckBox *editCheckBox;
	QBoxLayout *box;
	QProgressDialog *progress;
	
signals:
	void compileShaderRequest( ThumbRequest );
};

#endif // SHADEREDIT_H