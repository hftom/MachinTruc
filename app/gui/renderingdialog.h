#ifndef RENDERINGDIALOG_H
#define RENDERINGDIALOG_H

#include "output/output_ff.h"
#include "ui_render.h"



class RenderingDialog : public QDialog, protected Ui::RenderDialog
{
	Q_OBJECT
public:
	RenderingDialog( QWidget *parent, Profile &p, double playhead,
		double sceneLen, MQueue<Frame*> *af, MQueue<Frame*> *vf );
	~RenderingDialog();
	
public slots:
	void timelineReady();
	
private slots:
	void openFile();
	void startRender();
	void canceled();
	void done( int r );
	void outputFinished();
	void frameEncoded( Frame *f );
	
private:
	void enableUI( bool b );

	double playheadPts;
	double encodeStartPts;
	double encodeLength;
	double timelineLength;
	Profile profile;
	bool encoderRunning;
	OutputFF *out;
	
signals:
	void renderStarted( double startPts );
	void renderFinished( double pts );
	void showFrame( Frame* );
};

#endif // RENDERINGDIALOG_H
