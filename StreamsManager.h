#ifndef STREAMSMANAGER_H
#define STREAMSMANAGER_H

#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QElapsedTimer>

class QJsonObject;

class Stream
{
public:
	Stream(const QJsonObject& jsonObject);
	Stream() : updatingPreview(false), updatingAvailableStreams(false) {}

	QString url, name, comment, game, language;
	QString previewUrl;
	QPixmap preview;
	qint64 previewTimestamp;
	QStringList availableStreams;
	int viewers;
	bool updatingPreview, updatingAvailableStreams;
};

//****************************************************************************//

class StreamsManager : public QObject
{
	Q_OBJECT
public:
	StreamsManager(QObject* parent = nullptr);

	void updateStreamsList();
	void updateStreamsList(QString game);
	void updatePicture(Stream &stream);
	void updateGames(); // Get the list of games from the Twitch API

	typedef QSharedPointer<Stream> StreamPtr;
	typedef QVector<StreamPtr> StreamsList;

	const StreamsList& getStreams();
	StreamPtr getStream(QString url);

	const QStringList& getGames();
	const QStringList& getLanguages();

public slots:
	void streamsReply();
	void previewReply();
	void gamesReply();

signals:
	void streamsListUpdated();
	void previewUpdated(Stream*);
	void gamesListUpdated();
	void languagesListUpdated();

protected:
	void computeGamesList(); // Compute the list of games from the list of streams we previously got

	QNetworkAccessManager m_networkManager;

	StreamsList m_streams;
	QStringList m_games, m_languages;
	QElapsedTimer m_elapsedTimer;
};

#endif // QUERYMANAGER_H
