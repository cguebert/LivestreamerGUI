/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "QxtCheckComboBox.h"
#include <QLineEdit>
#include <QKeyEvent>

/*!
    \class QxtCheckComboBox
    \inmodule QxtGui
    \brief The QxtCheckComboBox widget is an extended QComboBox with checkable items.

    QxtComboBox is a specialized combo box with checkable items.
    Checked items are collected together in the line edit.

    \code
    QxtCheckComboBox* comboBox = new QxtCheckComboBox(this);
    comboBox->addItems(...);
    comboBox->setItemCheckState(2, Qt::Checked);
    comboBox->setItemCheckState(4, Qt::Checked);
    // OR
    comboBox->setCheckedItems(QStringList() << "dolor" << "amet");
    \endcode
 */

/*!
    \fn QxtCheckComboBox::checkedItemsChanged(const QStringList& items)

    This signal is emitted whenever the checked \a items have been changed.
 */

/*!
    Constructs a new QxtCheckComboBox with \a parent.
 */
QxtCheckComboBox::QxtCheckComboBox(QWidget* parent) : QComboBox(parent), m_containerMousePress(false)
{
    setModel(new QxtCheckComboModel(this));
	connect(this, SIGNAL(activated(int)), this, SLOT(toggleCheckState(int)));
	connect(model(), SIGNAL(checkStateChanged()), this, SLOT(updateCheckedItems()));
	connect(model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(updateCheckedItems()));
	connect(model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(updateCheckedItems()));

    // read-only contents
    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setReadOnly(true);
    setLineEdit(lineEdit);
    lineEdit->disconnect(this);
    setInsertPolicy(QComboBox::NoInsert);

	view()->installEventFilter(this);
	view()->window()->installEventFilter(this);
	view()->viewport()->installEventFilter(this);
	this->installEventFilter(this);

	m_separator = QLatin1String(",");
}

/*!
    Destructs the combo box.
 */
QxtCheckComboBox::~QxtCheckComboBox()
{
}

/*!
    \reimp
 */
void QxtCheckComboBox::hidePopup()
{
	if (m_containerMousePress)
        QComboBox::hidePopup();
}

/*!
    Returns the check state of the item at \a index.
 */
Qt::CheckState QxtCheckComboBox::itemCheckState(int index) const
{
    return static_cast<Qt::CheckState>(itemData(index, Qt::CheckStateRole).toInt());
}

/*!
    Sets the check \a state of the item at \a index.
 */
void QxtCheckComboBox::setItemCheckState(int index, Qt::CheckState state)
{
    setItemData(index, state, Qt::CheckStateRole);
}

/*!
    \property QxtCheckComboBox::checkedItems
    \brief the checked items.
 */
QStringList QxtCheckComboBox::checkedItems() const
{
    QStringList items;
    if (model())
    {
        QModelIndex index = model()->index(0, modelColumn(), rootModelIndex());
        QModelIndexList indexes = model()->match(index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly);
        foreach(const QModelIndex& index, indexes)
			items += index.data().toString();
    }
    return items;
}

void QxtCheckComboBox::setCheckedItems(const QStringList& items)
{
    // not the most efficient solution but most likely nobody
    // will put too many items into a combo box anyway so...
    foreach(const QString& text, items)
    {
        const int index = findText(text);
        setItemCheckState(index, index != -1 ? Qt::Checked : Qt::Unchecked);
    }
}

/*!
    \property QxtCheckComboBox::defaultText
    \brief the default text.

    The default text is shown when there are no checked items.
    The default value is an empty string.
 */
QString QxtCheckComboBox::defaultText() const
{
	return m_defaultText;
}

void QxtCheckComboBox::setDefaultText(const QString& text)
{
	if (m_defaultText != text)
    {
		m_defaultText = text;
		updateCheckedItems();
    }
}

/*!
	\property QxtCheckComboBox::multipleSelectionText
	\brief the multiple selection text.

	When displayMultipleSelection if true, the multiple selection text is shown when there are more than 1 checked item.
 */
QString QxtCheckComboBox::multipleSelectionText() const
{
	return m_multipleSelectionText;
}

void QxtCheckComboBox::setMultipleSelectionText(const QString& text)
{
	if (m_multipleSelectionText != text)
	{
		m_multipleSelectionText = text;
		updateCheckedItems();
	}
}

/*!
    \property QxtCheckComboBox::separator
    \brief the default separator.

    The checked items are joined together with the separator string.
    The default value is a comma (",").
 */
QString QxtCheckComboBox::separator() const
{
	return m_separator;
}

void QxtCheckComboBox::setSeparator(const QString& separator)
{
	if (m_separator != separator)
    {
		m_separator = separator;
		updateCheckedItems();
    }
}

/*!
	\property QxtCheckComboBox::displayMultipleSelection
	\brief the display multiple selection flag.

	Flag to either display the selection or only the number of items selected
 */
bool QxtCheckComboBox::displayMultipleSelection() const
{
	return m_displayMultipleSelection;
}

void QxtCheckComboBox::setDisplayMultipleSelection(bool display)
{
	m_displayMultipleSelection = display;
}

bool QxtCheckComboBox::eventFilter(QObject* receiver, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (receiver == this && (keyEvent->key() == Qt::Key_Up ||
								 keyEvent->key() == Qt::Key_Down))
		{
			showPopup();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Enter ||
				 keyEvent->key() == Qt::Key_Return ||
				 keyEvent->key() == Qt::Key_Escape)
		{
			// it is important to call QComboBox implementation
			this->QComboBox::hidePopup();
			if (keyEvent->key() != Qt::Key_Escape)
				return true;
		}
	}
	case QEvent::MouseButtonPress:
		m_containerMousePress = (receiver == view()->window());
		break;
	case QEvent::MouseButtonRelease:
		m_containerMousePress = false;;
		break;
	default:
		break;
	}
	return false;
}

void QxtCheckComboBox::updateCheckedItems()
{
	QStringList items = checkedItems();
	if (items.isEmpty())
		setEditText(m_defaultText);
	else
	{
		if(m_displayMultipleSelection || items.size() == 1)
			setEditText(items.join(m_separator));
		else
			setEditText(m_multipleSelectionText.arg(items.size()));
	}

	emit checkedItemsChanged(items);
}

void QxtCheckComboBox::toggleCheckState(int index)
{
	QVariant value = itemData(index, Qt::CheckStateRole);
	if (value.isValid())
	{
		Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
		setItemData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
	}
}

QxtCheckComboModel::QxtCheckComboModel(QObject* parent)
		: QStandardItemModel(0, 1, parent) // rows,cols
{
}

Qt::ItemFlags QxtCheckComboModel::flags(const QModelIndex& index) const
{
	return QStandardItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant QxtCheckComboModel::data(const QModelIndex& index, int role) const
{
	QVariant value = QStandardItemModel::data(index, role);
	if (index.isValid() && role == Qt::CheckStateRole && !value.isValid())
		value = Qt::Unchecked;
	return value;
}

bool QxtCheckComboModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	bool ok = QStandardItemModel::setData(index, value, role);
	if (ok && role == Qt::CheckStateRole)
	{
		emit dataChanged(index, index);
		emit checkStateChanged();
	}
	return ok;
}
