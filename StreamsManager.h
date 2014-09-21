#ifndef STREAMSMANAGER_H
#define STREAMSMANAGER_H

#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QVector>
#include <QString>
#include <QPixmap>

class QJsonObject;

class Stream
{
public:
	Stream(const QJsonObject& jsonObject);
	Stream() : updatingPreview(false), updatingAvailableStreams(false) {}

	QString url, name, comment, game, language;
	QString previewUrl;
	QPixmap preview;
	QStringList availableStreams;
	int viewers;
	bool updatingPreview, updatingAvailableStreams;
};

class StreamsManager : public QObject
{
	Q_OBJECT
public:
	StreamsManager(QObject* parent = nullptr);

	void updateStreamsList();
	void updateStreamsList(QString game);
	void updatePicture(Stream &stream);
	void updateGames();

	typedef QSharedPointer<Stream> StreamPtr;
	typedef QVector<StreamPtr> StreamsList;

	const StreamsList& getStreams();
	StreamPtr getStream(QString url);

	const QStringList& getGames();

public slots:
	void streamsReply();
	void previewReply();
	void gamesReply();

signals:
	void streamsListUpdated();
	void previewUpdated(Stream*);
	void gamesListUpdated();

protected:
	QNetworkAccessManager m_networkManager;

	StreamsList m_streams;
	QStringList m_games;
};

#endif // QUERYMANAGER_H
