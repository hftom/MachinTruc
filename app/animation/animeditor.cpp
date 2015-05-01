#include <QMessageBox>

#include "animeditor.h"



AnimEditor::AnimEditor( QWidget *parent ) : QWidget( parent )
{
	setupUi( this );
	
	animItem = new AnimItem();
	connect( animItem, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( animItem, SIGNAL(ovdValueChanged(ParameterWidget*)), this, SIGNAL(ovdValueChanged(ParameterWidget*)) );
	animScene = new AnimScene( animItem );
	
	graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	graphicsView->setFocusPolicy( Qt::NoFocus );
	graphicsView->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	graphicsView->setScene( animScene );
	
	connect( graphicsView, SIGNAL(sizeChanged(const QSize&)), animScene, SLOT(viewSizeChanged(const QSize&)) );
	connect( quitBtn, SIGNAL(clicked()), this, SLOT(editorQuit()) );
	connect( removeGraphBtn, SIGNAL(clicked()), this, SLOT(removeGraph()) );
}



void AnimEditor::removeGraph()
{
	if ( QMessageBox::question( this, tr( "Remove animation" ), tr( "Do you want to remove this animation?" ),
		QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok ) == QMessageBox::Ok )
	{
		animItem->removeGraph();
		animItem->quitEditor();
		emit quitEditor();
	}
}



void AnimEditor::editorQuit()
{
	animItem->quitEditor();
	emit quitEditor();
}



void AnimEditor::setCurrentParam( FilterWidget *f, ParameterWidget *pw, Parameter *p )
{
	animItem->setCurrentParam( f, pw, p );
}



void AnimEditor::ovdUpdate( QList<OVDUpdateMessage> msg )
{
	animItem->ovdUpdate( msg );
}


void AnimEditor::filterDeleted( Clip *c, QSharedPointer<Filter> f )
{
	if ( animItem->filterDeleted( c, f ) )
		emit quitEditor();
}