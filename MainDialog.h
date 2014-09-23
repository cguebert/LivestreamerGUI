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
	void updateLanguagesList();
	void launchLiveStreamer();
	void selectionChanged(QListWidgetItem*, QListWidgetItem*);
	void filterStreams();
	void automaticUpdate();
	void gamesSelectionChanged(const QStringList& items);
	void languagesSelectionChanged(const QStringList& items);

protected:
	virtual void closeEvent(QCloseEvent*);
	void readSettings();
	void writeSettings();
	void testAvailableStreams(StreamsManager::StreamPtr stream);
	void enableLaunchButtons(QStringList availableStreams);
	void enableLaunchButtons(bool enable);
	void checkAllGamesOption(const QStringList& checked);
	void checkAllLanguagesOption(const QStringList& checked);

	StreamsManager m_streamsManager;
	StreamsManager::StreamPtr m_selectedStream;

	QStringList m_gamesSelection;
	Qt::CheckState m_allGamesCheckState;
	bool m_changingGamesList;

	QStringList m_languagesSelection;
	Qt::CheckState m_allLanguagesCheckState;
	bool m_changingLanguagesList;

	QSplitter* m_splitter;
	QListWidget* m_listWidget;
	QPushButton* m_launchButton;
	QPushButton* m_updateButton;
	QxtCheckComboBox* m_gamesComboxBox;
	QxtCheckComboBox* m_languageComboxBox;
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
