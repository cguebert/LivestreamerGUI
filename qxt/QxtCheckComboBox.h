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

// Modified by Christophe Guebert 2014 for use in LiveStreamerGUI
//  Merged the private class into the main class
//  Changed the text displayed when many items are selected

#ifndef QXTCHECKCOMBOBOX_H
#define QXTCHECKCOMBOBOX_H

#include <QComboBox>
#include <QListView>
#include <QStandardItemModel>

class QxtCheckComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit QxtCheckComboBox(QWidget* parent = 0);
    virtual ~QxtCheckComboBox();

    virtual void hidePopup();

	QString defaultText() const;
	void setDefaultText(const QString& text);

	QString multipleSelectionText() const;
	void setMultipleSelectionText(const QString& text);

    Qt::CheckState itemCheckState(int index) const;
    void setItemCheckState(int index, Qt::CheckState state);

    QString separator() const;
    void setSeparator(const QString& separator);

	bool displayMultipleSelection() const;
	void setDisplayMultipleSelection(bool display);

    QStringList checkedItems() const;
	QStringList uncheckedItems() const;

public Q_SLOTS:
    void setCheckedItems(const QStringList& items);
	void updateCheckedItems();
	void toggleCheckState(int index);

protected:
	bool eventFilter(QObject* receiver, QEvent* event);
	QString m_separator, m_defaultText, m_multipleSelectionText;
	bool m_displayMultipleSelection, m_containerMousePress;

Q_SIGNALS:
    void checkedItemsChanged(const QStringList& items);
};

class QxtCheckComboModel : public QStandardItemModel
{
	Q_OBJECT

public:
	explicit QxtCheckComboModel(QObject* parent = 0);

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

Q_SIGNALS:
	void checkStateChanged();
};

#endif // QXTCHECKCOMBOBOX_H
