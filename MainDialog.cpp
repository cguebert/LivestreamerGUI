#include "MainDialog.h"

#include <QtConcurrent>

// Uncomment to launch a first livestreamer to test the available stream before enabling the launch buttons
//#define TEST_AVAILABLE_STREAMS

MainDialog::MainDialog()
	: m_streamsManager(this)
{
	QVBoxLayout* vMainLayout = new QVBoxLayout;
	m_splitter = new QSplitter;

	m_listWidget = new QListWidget;
	m_listWidget->setMinimumSize(100, 300);
	m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_splitter->addWidget(m_listWidget);

	QVBoxLayout* rightLayout = new QVBoxLayout;
	QVBoxLayout* infoLayout = new QVBoxLayout;
	m_pictureLabel = new QLabel;
	m_pictureLabel->setMinimumSize(320, 200);
	m_nameLabel = new QLabel;
	m_commentLabel = new QLabel;
	m_commentLabel->setWordWrap(true);
	m_viewersLabel = new QLabel;
	m_gameLabel = new QLabel;

	infoLayout->addWidget(m_pictureLabel);
	infoLayout->addWidget(m_nameLabel);
	infoLayout->addWidget(m_commentLabel);
	infoLayout->addWidget(m_viewersLabel);
	infoLayout->addWidget(m_gameLabel);

	QGroupBox* infoBox = new QGroupBox;
	infoBox->setTitle("Stream infos");
	infoBox->setLayout(infoLayout);
	rightLayout->addWidget(infoBox);

	QFormLayout* filterLayout = new QFormLayout;
	m_languageEdit = new QLineEdit;
	m_gamesWidget = new QComboBox;
	m_gamesWidget->addItem("All");

	filterLayout->addRow("Games", m_gamesWidget);
	filterLayout->addRow("Language", m_languageEdit);
	m_languageEdit->setText("en");

	QGroupBox* filterBox = new QGroupBox;
	filterBox->setTitle("Filters");
	filterBox->setLayout(filterLayout);
	rightLayout->addWidget(filterBox);

	QHBoxLayout* launchLayout = new QHBoxLayout;
	m_launchLow = new QPushButton("low");
	m_launchMedium = new QPushButton("medium");
	m_launchHigh = new QPushButton("high");
	m_launchSource = new QPushButton("source");

	enableLaunchButtons(false);

	connect(m_launchLow,	SIGNAL(clicked()), this, SLOT(launchLiveStreamer()));
	connect(m_launchMedium,	SIGNAL(clicked()), this, SLOT(launchLiveStreamer()));
	connect(m_launchHigh,	SIGNAL(clicked()), this, SLOT(launchLiveStreamer()));
	connect(m_launchSource,	SIGNAL(clicked()), this, SLOT(launchLiveStreamer()));

	launchLayout->addWidget(m_launchLow);
	launchLayout->addWidget(m_launchMedium);
	launchLayout->addWidget(m_launchHigh);
	launchLayout->addWidget(m_launchSource);

	QGroupBox* launchBox = new QGroupBox;
	launchBox->setTitle("Launch");
	launchBox->setLayout(launchLayout);
	rightLayout->addWidget(launchBox);

	rightLayout->addStretch();
	QWidget* rightContainer = new QWidget;
	rightContainer->setLayout(rightLayout);
	m_splitter->addWidget(rightContainer);

	vMainLayout->addWidget(m_splitter);

	QPushButton* quitButton = new QPushButton(tr("Quit"), this);
	quitButton->setDefault(true);
	connect(quitButton, SIGNAL(clicked()), this, SLOT(closeDialog()));

	m_updateButton = new QPushButton(tr("Refresh all"), this);
	connect(m_updateButton, SIGNAL(clicked()), this, SLOT(updateEverything()));

	QPushButton* filterButton = new QPushButton(tr("Filter"), this);
	connect(filterButton, SIGNAL(clicked()), this, SLOT(filterStreams()));

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(filterButton);
	buttonsLayout->addWidget(m_updateButton);
	buttonsLayout->addWidget(quitButton);
	vMainLayout->addLayout(buttonsLayout);

	setLayout(vMainLayout);

	connect(&m_streamsManager, SIGNAL(streamsListUpdated()), this, SLOT(updateStreamsList()));
	connect(&m_streamsManager, SIGNAL(previewUpdated(Stream*)), this, SLOT(updatePicture(Stream*)));
	connect(&m_streamsManager, SIGNAL(gamesListUpdated()), this, SLOT(updateGamesList()));
	connect(m_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectionChanged(QListWidgetItem*, QListWidgetItem*)));

	m_updateTimer = new QTimer(this);
	m_updateTimer->start(30000); // Update every 30 seconds
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(automaticUpdate()));

	updateEverything();

	readSettings();

	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
}

void MainDialog::closeDialog()
{
	writeSettings();
	accept();
}

void MainDialog::selectionChanged(QListWidgetItem* current, QListWidgetItem*)
{
	if(!current)
	{
		m_selectedStream.reset();
		enableLaunchButtons(false);
	}
	else
	{
		QString url = current->data(Qt::UserRole).toString();
		if(!url.isEmpty())
		{
			m_selectedStream = m_streamsManager.getStream(url);
			if(m_selectedStream)
			{
				m_nameLabel->setText(m_selectedStream->name);
				m_commentLabel->setText(m_selectedStream->comment);
				m_viewersLabel->setText(QString("%1 viewers\tlanguage: %2")
										.arg(m_selectedStream->viewers).arg(m_selectedStream->language));
				m_gameLabel->setText(m_selectedStream->game);

				if(m_selectedStream->preview.isNull())
					m_pictureLabel->clear();
				else
					m_pictureLabel->setPixmap(m_selectedStream->preview);

				// Will update the picture if it does not exist or is too old
				m_streamsManager.updatePicture(*m_selectedStream);

#ifdef TEST_AVAILABLE_STREAMS
				if(m_selectedStream->availableStreams.isEmpty() && !m_selectedStream->updatingAvailableStreams)
				{
					m_launchLow->setEnabled(false);
					m_launchMedium->setEnabled(false);
					m_launchHigh->setEnabled(false);
					m_launchSource->setEnabled(false);

					QtConcurrent::run(this, &MainDialog::testAvailableStreams, m_selectedStream);
				}
				else
					enableLaunchButtons(m_selectedStream->availableStreams);
#else
				enableLaunchButtons(true);
#endif
			}
		}
	}
}

void MainDialog::updateEverything()
{
	m_updateButton->setEnabled(false);

	m_listWidget->clear();
	QListWidgetItem* item = new QListWidgetItem("Updating...", m_listWidget);
	item->setForeground(QColor(0,128,0));
	m_listWidget->addItem(item);

	m_streamsManager.updateStreamsList();
	m_streamsManager.updateGames();
}

void MainDialog::updateStreamsList()
{
	QString currentUrl;
	StreamsManager::StreamPtr prevSelection = m_selectedStream;
	if(m_selectedStream)
		currentUrl = m_selectedStream->url;

	const auto& streams = m_streamsManager.getStreams();
	m_listWidget->clear(); // Will call selectionChanged and reset m_selectedStream

	QString language = m_languageEdit->text();

	for(const auto& stream : streams)
	{
		if(!language.isEmpty() && !stream->language.startsWith(language, Qt::CaseInsensitive))
			continue;

		QListWidgetItem* item = new QListWidgetItem(stream->name, m_listWidget);
		item->setData(Qt::UserRole, stream->url);
		m_listWidget->addItem(item);

		if(stream->url == currentUrl)
		{
			stream->preview = prevSelection->preview;
			stream->previewTimestamp = prevSelection->previewTimestamp;
			m_listWidget->setCurrentItem(item); // Will call selectionChanged and set m_selectedStream
		}
	}

	if(streams.isEmpty())
	{
		QListWidgetItem* item = new QListWidgetItem("Update error", m_listWidget);
		item->setForeground(QColor(255,0,0));
		m_listWidget->addItem(item);
	}

	m_updateButton->setEnabled(true);
	enableLaunchButtons(m_selectedStream);
}

void MainDialog::updatePicture(Stream* stream)
{
	if(stream && stream == m_selectedStream)
	{
		m_pictureLabel->setPixmap(stream->preview);
	}
}

void MainDialog::updateGamesList()
{
	auto games = m_streamsManager.getGames();
	games.insert(0, "All");

	m_gamesWidget->clear();
	m_gamesWidget->addItems(games);
}

void MainDialog::filterStreams()
{
	QString game;
	if(m_gamesWidget->currentIndex() != 0)
		game = m_gamesWidget->currentText();

	if(game.isEmpty())
		m_streamsManager.updateStreamsList();
	else
		m_streamsManager.updateStreamsList(game);
}

void MainDialog::automaticUpdate()
{
	m_updateButton->setEnabled(false);
	m_streamsManager.updateStreamsList();
}

void MainDialog::enableLaunchButtons(QStringList availableStreams)
{
	m_launchLow->setEnabled(availableStreams.contains("low"));
	m_launchMedium->setEnabled(availableStreams.contains("medium"));
	m_launchHigh->setEnabled(availableStreams.contains("high"));
	m_launchSource->setEnabled(availableStreams.contains("source"));
}

void MainDialog::enableLaunchButtons(bool enable)
{
	m_launchLow->setEnabled(enable);
	m_launchMedium->setEnabled(enable);
	m_launchHigh->setEnabled(enable);
	m_launchSource->setEnabled(enable);
}

void MainDialog::closeEvent(QCloseEvent* event)
{
	writeSettings();
	QWidget::closeEvent(event);
}

void MainDialog::readSettings()
{
	QSettings settings("Christophe Guebert", "LiveStreamerGUI");

	if(settings.contains("geometry"))
		restoreGeometry(settings.value("geometry").toByteArray());
	else
		resize(500, 300);

	if(settings.contains("splitter"))
	{
		auto tempSizes = settings.value("splitter").toList();
		QList<int> splitterSizes;
		for(const auto& size : tempSizes)
			splitterSizes << size.toInt();
		m_splitter->setSizes(splitterSizes);
	}
	else // Create a 0.4/0.6 separation by default
	{
		int width = m_splitter->width();
		QList<int> splitterSizes;
		splitterSizes << width * 0.4 << width * 0.6;
		m_splitter->setSizes(splitterSizes);
	}
}

void MainDialog::writeSettings()
{
	QSettings settings("Christophe Guebert", "LiveStreamerGUI");

	settings.setValue("geometry", saveGeometry());

	auto splitterSizes = m_splitter->sizes();
	QList<QVariant> tempSizes;
	for(const auto& size : splitterSizes)
		tempSizes << size;
	settings.setValue("splitter", tempSizes);
}

void MainDialog::launchLiveStreamer()
{
	QPushButton* button = qobject_cast<QPushButton*>(sender());

	auto selection = m_listWidget->selectedItems();
	if(selection.size() != 1)
		return;

	auto url = selection[0]->data(Qt::UserRole);
	QStringList arguments;
	arguments << url.toString() << button->text();

	QProcess::startDetached("livestreamer", arguments);
}

void MainDialog::testAvailableStreams(StreamsManager::StreamPtr stream)
{
	stream->updatingAvailableStreams = true;

	QProcess process;
	process.start("livestreamer", QStringList(stream->url));
	process.waitForFinished();

	stream->availableStreams.clear();
	QString test = "Available streams: ";
	QString output = QString::fromLatin1(process.readAllStandardOutput());
	int start = output.indexOf(test);
	if(start != -1)
	{
		QString sub = output.mid(start + test.length());
		sub.replace(",", "");
		QStringList qualities = sub.split(" ", QString::SkipEmptyParts);
		for(const auto& quality : qualities)
		{
			if(quality.startsWith("("))
				continue;
			stream->availableStreams << quality;
		}
	}

	if(stream == m_selectedStream)
		enableLaunchButtons(stream->availableStreams);
}

