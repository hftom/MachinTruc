#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QMouseEvent>

#include "engine/parameter.h"



class FSlider : public QSlider
{
	Q_OBJECT
public:
	explicit FSlider( QWidget *parent ) : QSlider( Qt::Horizontal, parent ) {}

private:
	void mousePressEvent( QMouseEvent *event ) {
		int buttons = style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons);
		Qt::MouseButton button = static_cast<Qt::MouseButton>(buttons & (~(buttons - 1)));
		QMouseEvent modifiedEvent(event->type(), event->pos(), event->globalPos(), button,
			event->buttons() ^ event->button() ^ button, event->modifiers());
		QSlider::mousePressEvent(&modifiedEvent);
	}
};



class ParameterWidget : public QWidget
{
	Q_OBJECT
public:
	ParameterWidget( QWidget *parent, Parameter *p ) : QWidget( parent ),
		param( p ),
		animBtn( NULL ),
		animActive( false )
	{
	}
	
	virtual QLayout *getLayout() = 0;
	virtual void animValueChanged( double val ) = 0;
	virtual void ovdValueChanged() {}

	void setAnimActive( bool b ) {
		int i;
		
		if ( !param->keyframeable )
			return;
		
		animActive = b;

		if ( !param->graph.keys.count() ) {
			b = true;
			if ( animBtn )
				animBtn->setIcon( QIcon(":/toolbar/icons/lines-connector.png") );
		}
		else {
			if ( b )
				animBtn->setIcon( QIcon(":/toolbar/icons/lines-connector-active.png") );
			else
				animBtn->setIcon( QIcon(":/toolbar/icons/lines-connector-on.png") );
		}

		for ( i = 0; i < widgets.count(); ++i ) {
			widgets[i]->setEnabled( b );
		}
	}
	
	Parameter* getParameter() { return param; }
	
protected:
	void addAnimBtn( QWidget *parent ) {
		animBtn = new QToolButton( parent );
		connect( animBtn, SIGNAL(clicked()), this, SLOT(animButtonClicked()) );
		if ( param->graph.keys.count() )
			setActive( false );
		else
			setActive( true );
		animBtn->setIconSize( QSize(12,12) );
	}
	
	Parameter *param;
	
	QToolButton *animBtn;
	QList<QWidget*> widgets;
	bool animActive;
	
protected slots:
	void animButtonClicked() {
		emit showAnimation( this, param );
	}
	
private:
	void setActive( bool b ) {
		int i;
		if ( !param->graph.keys.count() ) {
			b = true;
			if ( animBtn )
				animBtn->setIcon( QIcon(":/toolbar/icons/lines-connector.png") );
		}
		else
			animBtn->setIcon( QIcon(":/toolbar/icons/lines-connector-on.png") );

		for ( i = 0; i < widgets.count(); ++i ) {
			widgets[i]->setEnabled( b );
		}
	}
	
signals:
	void valueChanged( Parameter*, QVariant );
	void keyValueChanged( Parameter*, QVariant );
	void showAnimation(ParameterWidget*, Parameter* );
};

#endif // PARAMETERWIDGET_H
