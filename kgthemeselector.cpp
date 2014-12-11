/***************************************************************************
 *   Copyright 2009-2012 Stefan Majewsky <majewsky@gmx.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kgthemeselector.h"
#include "kgthemeselector_p.h"

#include <QtGui/QCloseEvent>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDialog>

#include <QIcon>
#include <KDE/KLocalizedString>
#include <KNS3/DownloadDialog>

namespace Metrics
{
	const int Padding = 6;
	const QSize ThumbnailBaseSize(64, 64);
}

//BEGIN KgThemeSelector

class KgThemeSelector::Private
{
    public:
        KgThemeSelector* q;
        KgThemeProvider* m_provider;
        Options m_options;
        QListWidget* m_list;
        QPushButton* m_knsButton;

        void fillList();

        Private(KgThemeProvider* provider, Options options, KgThemeSelector* q) : q(q), m_provider(provider), m_options(options), m_knsButton(0) {}

        void _k_updateListSelection(const KgTheme* theme);
        void _k_updateProviderSelection();
        void _k_showNewStuffDialog();
};

KgThemeSelector::KgThemeSelector(KgThemeProvider* provider, Options options, QWidget* parent)
	: QWidget(parent)
	, d(new Private(provider, options, this))
{
	d->m_list = new QListWidget(this);
	d->m_list->setSelectionMode(QAbstractItemView::SingleSelection);
	d->m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	//load themes from provider
	d->fillList();
	//setup appearance of the theme list (min. size = 4 items)
	KgThemeDelegate* delegate = new KgThemeDelegate(d->m_list);
	const QSize itemSizeHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
	const QSize scrollBarSizeHint = d->m_list->verticalScrollBar()->sizeHint();
	d->m_list->setMinimumSize(itemSizeHint.width() + 2 * scrollBarSizeHint.width(), 4.1 * itemSizeHint.height());
	//monitor change selection in both directions
	connect(d->m_provider, SIGNAL(currentThemeChanged(const KgTheme*)),
		SLOT(_k_updateListSelection(const KgTheme*)));
	connect(d->m_list, SIGNAL(itemSelectionChanged()),
		SLOT(_k_updateProviderSelection()));
	//setup main layout
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(d->m_list);
	//setup KNS button
	if (options & EnableNewStuffDownload)
	{
		d->m_knsButton = new QPushButton(QIcon::fromTheme("get-hot-new-stuff"),
			i18n("Get New Themes..."), this);
		layout->addWidget(d->m_knsButton);
		connect(d->m_knsButton, SIGNAL(clicked()), SLOT(_k_showNewStuffDialog()));
	}
	
}

KgThemeSelector::~KgThemeSelector()
{
	delete d;
}

void KgThemeSelector::Private::fillList()
{
	m_list->clear();
	foreach (const KgTheme* theme, m_provider->themes())
	{
		QListWidgetItem* item = new QListWidgetItem(theme->name(), m_list);
		item->setData(Qt::DecorationRole,
			m_provider->generatePreview(theme, Metrics::ThumbnailBaseSize));
		item->setData(KgThemeDelegate::DescriptionRole, theme->description());
		item->setData(KgThemeDelegate::AuthorRole, theme->author());
		item->setData(KgThemeDelegate::AuthorEmailRole, theme->authorEmail());
		item->setData(KgThemeDelegate::IdRole, theme->identifier());
	}
	_k_updateListSelection(m_provider->currentTheme());
}

void KgThemeSelector::Private::_k_updateListSelection(const KgTheme* theme)
{
	for (int idx = 0; idx < m_list->count(); ++idx)
	{
		QListWidgetItem* item = m_list->item(idx);
		const QByteArray thisId = item->data(KgThemeDelegate::IdRole).toByteArray();
		if (thisId == theme->identifier())
		{
			m_list->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
			return;
		}
	}
	//make sure that something is selected
	if (m_list->count() > 0)
	{
		m_list->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
	}
}

void KgThemeSelector::Private::_k_updateProviderSelection()
{
	const QListWidgetItem* selItem = m_list->selectedItems().value(0);
	if (!selItem)
	{
		return;
	}
	const QByteArray selId = selItem->data(KgThemeDelegate::IdRole).toByteArray();
	//select the theme with this identifier
	foreach (const KgTheme* theme, m_provider->themes())
	{
		if (theme->identifier() == selId)
		{
			m_provider->setCurrentTheme(theme);
		}
	}
}

void KgThemeSelector::Private::_k_showNewStuffDialog()
{
	KNS3::DownloadDialog dialog(q);
	dialog.exec();
	if (!dialog.changedEntries().isEmpty())
	{
		m_provider->rediscoverThemes();
		fillList();
	}
	//restore previous selection
	_k_updateListSelection(m_provider->currentTheme());
}

class KgThemeSelector::Dialog : public QDialog
{
	public:
		Dialog(KgThemeSelector* sel, const QString& caption)
		{
			QVBoxLayout *mainLayout = new QVBoxLayout;
			setLayout(mainLayout);
			mainLayout->addWidget(sel);
			
			//replace
			QPushButton* btn = sel->d->m_knsButton;
			
			QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
			
			if (btn)
			{
				btn->hide();
								
				QPushButton *stuff = new QPushButton(QIcon::fromTheme("get-hot-new-stuff"), btn->text());
				buttonBox->addButton(stuff, QDialogButtonBox::ActionRole);
				buttonBox->addButton(QDialogButtonBox::Close);

			        connect(stuff, SIGNAL(clicked()), btn, SIGNAL(clicked()));	
				connect(buttonBox, &QDialogButtonBox::rejected, this, &KgThemeSelector::Dialog::reject);
			}
			else
			{
				buttonBox->setStandardButtons(QDialogButtonBox::Close);
				connect(buttonBox, &QDialogButtonBox::rejected, this, &KgThemeSelector::Dialog::reject);
			}
			//window caption
			if (caption.isEmpty())
			{
				setWindowTitle(i18nc("@title:window config dialog", "Select theme"));
			}
			else
			{
				setWindowTitle(caption);
			}
			
			mainLayout->addWidget(buttonBox);
			show();
		}
	protected:
		virtual void closeEvent(QCloseEvent* event)
		{
			event->accept();
			KgThemeSelector* sel = qobject_cast<KgThemeSelector*>(new QWidget());	//Unsure
			//delete myself, but *not* the KgThemeSelector
			sel->setParent(0);
			deleteLater();
			//restore the KNS button
			if (sel->d->m_knsButton)
			{
				sel->d->m_knsButton->show();
			}
		}
};

void KgThemeSelector::showAsDialog(const QString& caption)
{
	if (!isVisible())
	{
		new KgThemeSelector::Dialog(this, caption);
	}
}

//END KgThemeSelector
//BEGIN KgThemeDelegate

KgThemeDelegate::KgThemeDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent);
	if (view)
		view->setItemDelegate(this);
}

QRect KgThemeDelegate::thumbnailRect(const QRect& baseRect) const
{
	QRect thumbnailBaseRect(QPoint(Metrics::Padding + baseRect.left(), 0), Metrics::ThumbnailBaseSize);
	thumbnailBaseRect.moveCenter(QPoint(thumbnailBaseRect.center().x(), baseRect.center().y()));
	if (QApplication::isRightToLeft())
		thumbnailBaseRect.moveRight(baseRect.right() - Metrics::Padding);
	return thumbnailBaseRect;
}

void KgThemeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const bool rtl = option.direction == Qt::RightToLeft;
	QRect baseRect = option.rect;
	//draw background
	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
	//draw thumbnail
	QRect thumbnailBaseRect = this->thumbnailRect(baseRect);
	const QPixmap thumbnail = index.data(Qt::DecorationRole).value<QPixmap>().scaled(Metrics::ThumbnailBaseSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QRect thumbnailRect(thumbnailBaseRect.topLeft(), thumbnail.size());
	thumbnailRect.translate( //center inside thumbnailBaseRect
		(thumbnailBaseRect.width() - thumbnailRect.width()) / 2,
		(thumbnailBaseRect.height() - thumbnailRect.height()) / 2
	);
	painter->drawPixmap(thumbnailRect.topLeft(), thumbnail);
	//find metrics: text
	QStringList texts; QList<QFont> fonts;
	{
		QString name = index.data(Qt::DisplayRole).toString();
		if (name.isEmpty())
			name = i18n("[No name]");
		texts << name;
		QFont theFont(painter->font()); theFont.setBold(true); fonts << theFont;
	}{
		QString comment = index.data(DescriptionRole).toString();
		if (!comment.isEmpty())
		{
			texts << comment;
			fonts << painter->font();
		}
	}{
		QString author = index.data(AuthorRole).toString();
		if (!author.isEmpty())
		{
			const QString authorString = ki18nc("Author attribution, e.g. \"by Jack\"", "by %1").subs(author).toString();
			texts << authorString;
			QFont theFont(painter->font()); theFont.setItalic(true); fonts << theFont;
		}
	}
	//TODO: display AuthorEmailRole
	QList<QRect> textRects; int totalTextHeight = 0;
	for (int i = 0; i < texts.count(); ++i)
	{
		QFontMetrics fm(fonts[i]);
		textRects << fm.boundingRect(texts[i]);
		textRects[i].setHeight(qMax(textRects[i].height(), fm.lineSpacing()));
		totalTextHeight += textRects[i].height();
	}
	QRect textBaseRect(baseRect);
	if (rtl)
	{
		textBaseRect.setRight(thumbnailBaseRect.left() - Metrics::Padding);
		textBaseRect.adjust(Metrics::Padding, Metrics::Padding, 0, -Metrics::Padding);
	}
	else
	{
		textBaseRect.setLeft(thumbnailBaseRect.right() + Metrics::Padding);
		textBaseRect.adjust(0, Metrics::Padding, -Metrics::Padding, -Metrics::Padding);
	}
	textBaseRect.setHeight(totalTextHeight);
	textBaseRect.moveTop(baseRect.top() + (baseRect.height() - textBaseRect.height()) / 2);
	//draw texts
	QRect currentTextRect(textBaseRect);
	painter->save();
	for (int i = 0; i < texts.count(); ++i)
	{
		painter->setFont(fonts[i]);
		const QRect& textRect = textRects[i];
		currentTextRect.setHeight(textRect.height());
		const QFontMetrics fm(fonts[i]);
		const QString text = fm.elidedText(texts[i], Qt::ElideRight, currentTextRect.width());
		painter->drawText(currentTextRect, Qt::AlignLeft | Qt::AlignVCenter, text);
		currentTextRect.moveTop(currentTextRect.bottom());
	}
	painter->restore();
}

QSize KgThemeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option) Q_UNUSED(index)
	//TODO: take text size into account
	return QSize(400, Metrics::ThumbnailBaseSize.height() + 2 * Metrics::Padding);
}

//END KgThemeDelegate

#include "moc_kgthemeselector.cpp"
