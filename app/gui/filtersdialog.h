#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include "ui_filters.h"
#include "engine/sampler.h"
#include "engine/source.h"

#define MODEVIDEO 0
#define MODEAUDIO 1



class FiltersDialog : public QDialog, protected Ui::FiltersDlg
{
	Q_OBJECT
public:
	FiltersDialog( QWidget *parent, Source *src, Sampler *samp );
	~FiltersDialog();
	
private slots:
	void videoFilterActivated( int row );
	void showVideoFiltersList();
	void addVideoFilter( int i );
	void removeCurrentVideoFilter();
	
	void audioFilterActivated( int row );
	void showAudioFiltersList();
	void addAudioFilter( int i );
	void removeCurrentAudioFilter();
	
private:
	Source *source;
	Sampler *sampler;
	QWidget *currentVideoWidget;
	QWidget *currentAudioWidget;
};



class FiltersListDlg : public QDialog
{
	Q_OBJECT
public:
	FiltersListDlg( int m, QWidget *parent );
	
private slots:
	void filterSelected( QListWidgetItem* );
	
private:
	int mode;
	
signals:
	void addVideoFilter( int );
	void addAudioFilter( int );
};

#endif // FILTERSDIALOG_H