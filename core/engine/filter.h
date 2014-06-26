#ifndef FILTER_H
#define FILTER_H

#include <QWidget>
#include <QSlider>
#include <QMouseEvent>
#include <QStyle>
#include <QVariant>
#include <QMutex>



class Parameter
{
public:
	QString name;
	int type;
	QVariant min;
	QVariant max;
	bool keyframeable;
	void *value;
};



class Filter : public QObject
{
	Q_OBJECT
public:
	Filter( QString id, QString name );
	~Filter();
	void use();
	void release();
	
	QString getFilterName() { return filterName; }
	QString getIdentifier() { return identifier; }
	// the caller takes ownership of this widget
	virtual QWidget* getWidget();

public slots:
	void valueChanged( Parameter *p, QVariant val );

protected:
	enum ParameterType{ PFLOAT, PINT };
	
	void addParameter( QString name, int type, QVariant min, QVariant max, bool keyframeable, void *value );
	
private:
	QLayout* layoutPFLOAT( QWidget *w, Parameter *p );
	QLayout* layoutPINT( QWidget *w, Parameter *p );
	
	QList<Parameter*> parameters;
	
	int refCount;
	QMutex rcMutex;
	QString filterName, identifier;
	
signals:
	void updateFrame();
};



class FSlider : public QSlider
{
	Q_OBJECT
public:
	explicit FSlider( Parameter *param, QWidget *parent) : QSlider( Qt::Horizontal, parent) {
		parameter = param;
		connect( this, SIGNAL(valueChanged(int)), this, SLOT(changedValue(int)) );
	}
	~FSlider() { }

private:
	void mousePressEvent(QMouseEvent *event) {
		int buttons = style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons);
		Qt::MouseButton button = static_cast<Qt::MouseButton>(buttons & (~(buttons - 1)));
		QMouseEvent modifiedEvent(event->type(), event->pos(), event->globalPos(), button,
			event->buttons() ^ event->button() ^ button, event->modifiers());
		QSlider::mousePressEvent(&modifiedEvent);
	}
	
	Parameter *parameter;
	
private slots:
	void changedValue( int val ) {
		emit valueChanged( parameter, QVariant(val) );
	}
	
signals:
	void valueChanged( Parameter*, QVariant );
};

#endif // FILTER_H
