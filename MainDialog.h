#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QtWidgets>

#include "StreamsManager.h"
#include "qxt/QxtCheckComboBox.h"

class MainDialog : public QDialog
{
	Q_OBJECT
public:
	MainDialog();

protected slots:
	void closeDialog();
	void updateEverything();
	void updateStreamsList();
	void updatePicture(Stream*);
	void updateGamesList();
	void launchLiveStreamer();
	void selectionChanged(QListWidgetItem*, QListWidgetItem*);
	void filterStreams();
	void automaticUpdate();
	void gamesSelectionChanged(const QStringList& items);

protected:
	virtual void closeEvent(QCloseEvent*);
	void readSettings();
	void writeSettings();
	void testAvailableStreams(StreamsManager::StreamPtr stream);
	void enableLaunchButtons(QStringList availableStreams);
	void enableLaunchButtons(bool enable);

	StreamsManager m_streamsManager;
	StreamsManager::StreamPtr m_selectedStream;

	QStringList m_gamesSelection;
	Qt::CheckState m_allGamesCheckState;
	bool m_changingGamesList;

	QSplitter* m_splitter;
	QListWidget* m_listWidget;
	QPushButton* m_launchButton;
	QPushButton* m_updateButton;
	QxtCheckComboBox* m_gamesWidget;
	QLineEdit* m_languageEdit;
	QLabel* m_pictureLabel;
	QLabel* m_nameLabel;
	QLabel* m_commentLabel;
	QLabel* m_viewersLabel;
	QLabel* m_gameLabel;
	QPushButton* m_launchLow;
	QPushButton* m_launchMedium;
	QPushButton* m_launchHigh;
	QPushButton* m_launchSource;
	QTimer* m_updateTimer;
};

#endif // MAINDIALOG_H
