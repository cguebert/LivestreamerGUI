#include "MainDialog.h"

#include <QtConcurrent>

// Uncomment to launch a first livestreamer to test the available stream before enabling the launch buttons
//#define TEST_AVAILABLE_STREAMS

MainDialog::MainDialog()
	: m_streamsManager(this)
	, m_changingGamesList(false)
	, m_changingLanguagesList(false)
	, m_inverseGamesFilter(false)
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

	m_allGamesString = tr("All games");
	m_allLanguagesString = tr("All languages");

	QFormLayout* filterLayout = new QFormLayout;
	m_gamesComboxBox = new QxtCheckComboBox;
	m_gamesComboxBox->addItem(m_allGamesString);
	m_gamesComboxBox->setDefaultText("No filter");
	m_gamesComboxBox->setDisplayMultipleSelection(false);
	m_gamesComboxBox->setMultipleSelectionText("%1 games selected");

	m_languageComboxBox = new QxtCheckComboBox;
	m_languageComboxBox->addItem(m_allLanguagesString);
	m_languageComboxBox->setDefaultText("No filter");
	m_languageComboxBox->setDisplayMultipleSelection(false);
	m_languageComboxBox->setMultipleSelectionText("%1 languages selected");

	m_inverseGamesFilterCheckBox = new QCheckBox;
	m_inverseGamesFilterCheckBox->setText("Inverse games filter");
	connect(m_inverseGamesFilterCheckBox, SIGNAL(stateChanged(int)), this, SLOT(gamesFilterState(int)));

	filterLayout->addRow("Games", m_gamesComboxBox);
	filterLayout->addRow("Language", m_languageComboxBox);
	filterLayout->addRow(m_inverseGamesFilterCheckBox);

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

//	QPushButton* filterButton = new QPushButton(tr("Filter"), this);
//	connect(filterButton, SIGNAL(clicked()), this, SLOT(filterStreams()));

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
//	buttonsLayout->addWidget(filterButton);
	buttonsLayout->addWidget(m_updateButton);
	buttonsLayout->addWidget(quitButton);
	vMainLayout->addLayout(buttonsLayout);

	setLayout(vMainLayout);

	connect(&m_streamsManager, SIGNAL(streamsListUpdated()), this, SLOT(updateStreamsList()));
	connect(&m_streamsManager, SIGNAL(previewUpdated(Stream*)), this, SLOT(updatePicture(Stream*)));
	connect(&m_streamsManager, SIGNAL(gamesListUpdated()), this, SLOT(updateGamesList()));
	connect(&m_streamsManager, SIGNAL(languagesListUpdated()), this, SLOT(updateLanguagesList()));
	connect(m_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectionChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(m_gamesComboxBox, SIGNAL(checkedItemsChanged(QStringList)), this, SLOT(gamesSelectionChanged(QStringList)));
	connect(m_languageComboxBox, SIGNAL(checkedItemsChanged(QStringList)), this, SLOT(languagesSelectionChanged(QStringList)));

	m_updateTimer = new QTimer(this);
	m_updateTimer->start(30000); // Update every 30 seconds
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(automaticUpdate()));

	updateEverything();

	readSettings();

	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	flags |= Qt::WindowMinimizeButtonHint;
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

		m_nameLabel->clear();
		m_commentLabel->clear();
		m_viewersLabel->clear();
		m_gameLabel->clear();
		m_pictureLabel->clear();
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
					enableLaunchButtons(false);
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

void MainDialog::gamesSelectionChanged(const QStringList& items)
{
	if(m_changingGamesList)
		return;

	m_changingGamesList = true;
	Qt::CheckState tmpAllGamesState = m_gamesComboxBox->itemCheckState(0);
	if(tmpAllGamesState != Qt::PartiallyChecked && tmpAllGamesState != m_allGamesCheckState)
	{
		m_allGamesCheckState = (tmpAllGamesState == Qt::Checked ? Qt::Checked : Qt::Unchecked);
		for(int i=0, nb=m_gamesComboxBox->count(); i<nb; ++i)
			m_gamesComboxBox->setItemCheckState(i, m_allGamesCheckState);
	}
	else
		checkAllGamesOption(items);

	m_changingGamesList = false;
	computeGamesSelection();

	int nbChecked = m_gamesComboxBox->checkedItems().size();
	if(nbChecked == m_gamesComboxBox->count()) // We don't want to count "all games" in the selection count
		m_gamesComboxBox->setEditText(QString("%1 games selected").arg(nbChecked-1));

	if(!m_streamsManager.getStreams().empty())
		updateStreamsList();
}

void MainDialog::languagesSelectionChanged(const QStringList& items)
{
	if(m_changingLanguagesList)
		return;

	m_changingLanguagesList = true;
	Qt::CheckState tmpAllLanguagesState = m_languageComboxBox->itemCheckState(0);
	if(tmpAllLanguagesState != Qt::PartiallyChecked && tmpAllLanguagesState != m_allLanguagesCheckState)
	{
		m_allLanguagesCheckState = tmpAllLanguagesState;
		for(int i=0, nb=m_languageComboxBox->count(); i<nb; ++i)
			m_languageComboxBox->setItemCheckState(i, m_allLanguagesCheckState);
	}
	else
		checkAllLanguagesOption(items);

	m_changingLanguagesList = false;
	computeLanguagesSelection();

	int nbChecked = m_languageComboxBox->checkedItems().size();
	if(nbChecked == m_languageComboxBox->count()) // We don't want to count "all languages" in the selection count
		m_languageComboxBox->setEditText(QString("%1 languages selected").arg(nbChecked-1));

	if(!m_streamsManager.getStreams().empty())
		updateStreamsList();
}

void MainDialog::checkAllGamesOption(const QStringList& checked)
{
	int nbGames = m_gamesComboxBox->count();
	if(nbGames == 1)
		return;

	if(m_allGamesCheckState != Qt::Checked && checked.size() == nbGames - 1) // All selected
		m_allGamesCheckState = Qt::Checked;
	else if(checked.empty()) // None selected
		m_allGamesCheckState = Qt::Unchecked;
	else
		m_allGamesCheckState = Qt::PartiallyChecked;
	m_gamesComboxBox->setItemCheckState(0, m_allGamesCheckState);
}

void MainDialog::checkAllLanguagesOption(const QStringList& checked)
{
	int nbLanguages = m_languageComboxBox->count();
	if(nbLanguages == 1)
		return;

	if(m_allLanguagesCheckState != Qt::Checked && checked.size() == nbLanguages - 1) // All selected
		m_allLanguagesCheckState = Qt::Checked;
	else if(checked.empty()) // None selected
		m_allLanguagesCheckState = Qt::Unchecked;
	else
		m_allLanguagesCheckState = Qt::PartiallyChecked;
	m_languageComboxBox->setItemCheckState(0, m_allLanguagesCheckState);
}

void MainDialog::updateEverything()
{
	m_updateButton->setEnabled(false);

	m_listWidget->clear();
	QListWidgetItem* item = new QListWidgetItem("Updating...", m_listWidget);
	item->setForeground(QColor(0,128,0));
	m_listWidget->addItem(item);

	m_streamsManager.updateStreamsList();
//	m_streamsManager.updateGames();
}

void MainDialog::updateStreamsList()
{
	QString currentUrl;
	StreamsManager::StreamPtr prevSelection = m_selectedStream;
	if(m_selectedStream)
		currentUrl = m_selectedStream->url;

	const auto& streams = m_streamsManager.getStreams();
	m_listWidget->clear(); // Will call selectionChanged and reset m_selectedStream

	bool filterGames = !m_gamesComboxBox->checkedItems().empty();
	bool filterLanguages = !m_languageComboxBox->checkedItems().empty();

	for(const auto& stream : streams)
	{
		if(filterLanguages && !m_languagesSelection.contains(stream->language))
			continue;

		if(filterGames && m_inverseGamesFilter == m_gamesSelection.contains(stream->game))
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

	if(!m_listWidget->count())
	{
		QListWidgetItem* item = new QListWidgetItem("No stream corresponding to the filters", m_listWidget);
		item->setForeground(QColor(255,128,0));
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
	games.insert(0, m_allGamesString);

	m_changingGamesList = true;
	m_gamesComboxBox->clear();
	m_gamesComboxBox->addItems(games);

	if(m_allGamesCheckState == Qt::Checked)
	{
		for(int i=0, nb=games.size(); i<nb; ++i)
			m_gamesComboxBox->setItemCheckState(i, Qt::Checked);
	}
	else
	{
		m_gamesComboxBox->setCheckedItems(m_gamesSelection);
		checkAllGamesOption(m_gamesSelection);
	}

	m_changingGamesList = false;
	computeGamesSelection();
}

void MainDialog::updateLanguagesList()
{
	auto languages = m_streamsManager.getLanguages();
	languages.insert(0, m_allLanguagesString);

	m_changingLanguagesList = true;
	m_languageComboxBox->clear();
	m_languageComboxBox->addItems(languages);

	if(m_allLanguagesCheckState == Qt::Checked)
	{
		for(int i=0, nb=languages.size(); i<nb; ++i)
			m_languageComboxBox->setItemCheckState(i, Qt::Checked);
	}
	else
	{
		m_languageComboxBox->setCheckedItems(m_languagesSelection);
		checkAllLanguagesOption(m_languagesSelection);
	}

	m_changingLanguagesList = false;
	computeLanguagesSelection();
}

void MainDialog::computeGamesSelection()
{	// We merge the saved selection with the new one, removing items that are unchecked
	QStringList checked = m_gamesComboxBox->checkedItems();
	QStringList unchecked = m_gamesComboxBox->uncheckedItems();
	QStringList prevSelection = m_gamesSelection;
	QStringList newSelection;
	m_gamesSelection.clear();
	unchecked.push_front(m_allGamesString);
	unchecked.push_front("");

	std::sort(checked.begin(), checked.end());
	std::sort(unchecked.begin(), unchecked.end());
	std::sort(prevSelection.begin(), prevSelection.end());

	// Remove unchecked items from the selection
	std::set_difference(prevSelection.begin(), prevSelection.end(),
						unchecked.begin(), unchecked.end(),
						std::back_inserter(newSelection));

	// Adding the new checked items (more efficient than using QStringList::append)
	std::set_union(newSelection.begin(), newSelection.end(),
				   checked.begin(), checked.end(),
				   std::back_inserter(m_gamesSelection));
}

void MainDialog::computeLanguagesSelection()
{
	QStringList checked = m_languageComboxBox->checkedItems();
	QStringList unchecked = m_languageComboxBox->uncheckedItems();
	QStringList prevSelection = m_languagesSelection;
	QStringList newSelection;
	m_languagesSelection.clear();
	unchecked.push_front(m_allLanguagesString);
	unchecked.push_front("");

	std::sort(checked.begin(), checked.end());
	std::sort(unchecked.begin(), unchecked.end());
	std::sort(prevSelection.begin(), prevSelection.end());

	std::set_difference(prevSelection.begin(), prevSelection.end(),
						unchecked.begin(), unchecked.end(),
						std::back_inserter(newSelection));

	std::set_union(newSelection.begin(), newSelection.end(),
				   checked.begin(), checked.end(),
				   std::back_inserter(m_languagesSelection));
}

void MainDialog::filterStreams()
{
	QString game;
	if(m_gamesComboxBox->currentIndex() != 0)
		game = m_gamesComboxBox->currentText();

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

	QString tmpGames = settings.value("games").toString();
	m_gamesSelection = tmpGames.split(";");
	m_allGamesCheckState = (m_gamesSelection.contains(m_allGamesString) ? Qt::Checked : Qt::Unchecked);

	QString tmpLanguages = settings.value("languages").toString();
	m_languagesSelection = tmpLanguages.split(";");
	m_allLanguagesCheckState = (m_languagesSelection.contains(m_allLanguagesString) ? Qt::Checked : Qt::Unchecked);

	m_inverseGamesFilter = settings.value("invertGamesFilter").toBool();
	m_inverseGamesFilterCheckBox->setCheckState(m_inverseGamesFilter ? Qt::Checked : Qt::Unchecked);
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

	settings.setValue("games", m_gamesSelection.join(";"));
	settings.setValue("languages", m_languagesSelection.join(";"));
	settings.setValue("invertGamesFilter", m_inverseGamesFilter);
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

void MainDialog::gamesFilterState(int state)
{
	m_inverseGamesFilter = (state == Qt::Checked);
	if(!m_streamsManager.getStreams().empty())
		updateStreamsList();
}
