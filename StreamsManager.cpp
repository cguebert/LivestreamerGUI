#include "StreamsManager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <map>
#include <vector>

Stream::Stream(const QJsonObject& jsonObject)
	: updatingPreview(false)
	, updatingAvailableStreams(false)
	, previewTimestamp(-1)
{
	auto channel = jsonObject.value("channel").toObject();
	auto preview = jsonObject.value("preview").toObject();

	viewers = jsonObject.value("viewers").toInt();
	game = jsonObject.value("game").toString();
	if(game.isEmpty())
		game = "Not defined";

	name = channel.value("display_name").toString();
	url = channel.value("url").toString();
	comment = channel.value("status").toString();
	language = channel.value("language").toString();
	if(language.isEmpty())
		language = "Not defined";

	previewUrl = preview.value("medium").toString();
}

//****************************************************************************//

StreamsManager::StreamsManager(QObject* parent)
	: QObject(parent)
	, m_networkManager(this)
{
	m_elapsedTimer.start();
}

void StreamsManager::updateStreamsList()
{
	QNetworkRequest request = QNetworkRequest(QUrl("https://api.twitch.tv/kraken/streams?limit=100"));
	QNetworkReply* reply = m_networkManager.get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(streamsReply()));
}

void StreamsManager::updateStreamsList(QString game)
{
	QString url = QString("https://api.twitch.tv/kraken/streams?limit=100&");
	if(!game.isEmpty())
	{
		game.replace(" ", "+");
		url += "&game=" + game;
	}
	QNetworkRequest request = QNetworkRequest(QUrl(url));
	QNetworkReply* reply = m_networkManager.get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(streamsReply()));
}

void StreamsManager::updatePicture(Stream& stream)
{
	if(stream.updatingPreview)
		return;

	if(stream.previewTimestamp > 0 && m_elapsedTimer.elapsed() - stream.previewTimestamp < 20000)
		return;

	stream.updatingPreview = true;
	QNetworkRequest request = QNetworkRequest(QUrl(stream.previewUrl));
	request.setAttribute(QNetworkRequest::User, QVariant(stream.url));
	QNetworkReply* reply = m_networkManager.get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(previewReply()));
}

void StreamsManager::updateGames()
{
	QNetworkRequest request = QNetworkRequest(QUrl("https://api.twitch.tv/kraken/games/top?limit=25"));
	QNetworkReply* reply = m_networkManager.get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(gamesReply()));
}

void StreamsManager::streamsReply()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	auto streams = doc.object().value("streams").toArray();

	m_streams.clear();
	for(const auto& stream : streams)
		m_streams.push_back(StreamPtr(new Stream(stream.toObject())));

	computeGamesList();

	m_languages.clear();
	for(const auto& stream : m_streams)
	{
		if(!m_languages.contains(stream->language))
			m_languages << stream->language;
	}
	std::sort(m_languages.begin(), m_languages.end());

	emit gamesListUpdated();
	emit languagesListUpdated();
	emit streamsListUpdated();
	reply->deleteLater();
}

void StreamsManager::previewReply()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	QString url = reply->request().attribute(QNetworkRequest::User).toString();
	auto stream = getStream(url);
	if(stream)
	{
		stream->preview.loadFromData(reply->readAll());
		stream->previewTimestamp = m_elapsedTimer.elapsed();
		stream->updatingPreview = false;
		emit previewUpdated(stream.data());
	}

	reply->deleteLater();
}

void StreamsManager::gamesReply()
{
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	auto games = doc.object().value("top").toArray();

	m_games.clear();
	for(const auto& game : games)
	{
		QString name = game.toObject().value("game").toObject().value("name").toString();
		if(!name.isEmpty())
			m_games.push_back(name);
	}

	emit gamesListUpdated();
}

void StreamsManager::computeGamesList()
{
	std::map<QString, int> gamesMap;
	for(const auto& stream : m_streams)
		gamesMap[stream->game] += stream->viewers;

	typedef std::pair<QString, int> GamePair;
	std::vector<GamePair> gamesPairs;
	for(const auto& game : gamesMap)
		gamesPairs.push_back(std::make_pair(game.first, game.second));

	std::sort(gamesPairs.begin(), gamesPairs.end(), [](const GamePair& lhs, const GamePair& rhs){
		return lhs.second > rhs.second;
	});

	m_games.clear();
	for(const auto& game : gamesPairs)
		m_games.push_back(game.first);

}

const StreamsManager::StreamsList& StreamsManager::getStreams()
{
	return m_streams;
}

StreamsManager::StreamPtr StreamsManager::getStream(QString url)
{
	auto iter = std::find_if(m_streams.begin(), m_streams.end(), [url](const StreamPtr& s){
		return s->url == url;
	});
	if(iter != m_streams.end())
		return *iter;
	else
		return StreamPtr();
}

const QStringList& StreamsManager::getGames()
{
	return m_games;
}

const QStringList& StreamsManager::getLanguages()
{
	return m_languages;
}
