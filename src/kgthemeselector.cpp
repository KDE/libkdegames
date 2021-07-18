/*
    SPDX-FileCopyrightText: 2009-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgthemeselector.h"
#include "kgthemeselector_p.h"

// KF
#include <KLocalizedString>
#include <KNS3/DownloadDialog>
// Qt
#include <QCloseEvent>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QAbstractItemView>
#include <QApplication>
#include <QListWidget>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDialog>
#include <QIcon>

namespace Metrics
{
	const int Padding = 6;
	const QSize ThumbnailBaseSize(64, 64);
}

//BEGIN KgThemeSelector

class KgThemeSelectorPrivate
{
    public:
        KgThemeSelector* q;
        KgThemeProvider* m_provider;
        KgThemeSelector::Options m_options;
        QListWidget* m_list;
        QPushButton* m_knsButton;

        void fillList();

        KgThemeSelectorPrivate(KgThemeProvider* provider, KgThemeSelector::Options options, KgThemeSelector* q)
            : q(q)
            , m_provider(provider)
            , m_options(options)
            , m_knsButton(nullptr)
        {}

        void _k_updateListSelection(const KgTheme* theme);
        void _k_updateProviderSelection();
        void _k_showNewStuffDialog();
};

KgThemeSelector::KgThemeSelector(KgThemeProvider* provider, Options options, QWidget* parent)
	: QWidget(parent)
	, d(new KgThemeSelectorPrivate(provider, options, this))
{
	d->m_list = new QListWidget(this);
	d->m_list->setSelectionMode(QAbstractItemView::SingleSelection);
	d->m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	//load themes from provider
	d->fillList();

	//setup appearance of the theme list
	KgThemeDelegate* delegate = new KgThemeDelegate(d->m_list);
	QScreen *screen = QWidget::screen();
	QSize screenSize = screen->availableSize();
    if (screenSize.width() < 650 || screenSize.height() < 650) {
		d->m_list->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
		if (parent){
			d->m_list->setMinimumSize(0, 0);
		} else {
			//greater than zero to prevent zero sized dialog in some games
			d->m_list->setMinimumSize(330, 200);
		}
	} else {
		const QSize itemSizeHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
		const QSize scrollBarSizeHint = d->m_list->verticalScrollBar()->sizeHint();
		d->m_list->setMinimumSize(itemSizeHint.width() + 2 * scrollBarSizeHint.width(), 4.1 * itemSizeHint.height());
	}

	//monitor change selection in both directions
	connect(d->m_provider, &KgThemeProvider::currentThemeChanged,
		this, [this](const KgTheme* theme) { d->_k_updateListSelection(theme); });
	connect(d->m_list, &QListWidget::itemSelectionChanged,
		this, [this]() { d->_k_updateProviderSelection(); });
	//setup main layout
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(d->m_list);
	//setup KNS button
	if (options & EnableNewStuffDownload)
	{
		d->m_knsButton = new QPushButton(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")),
			i18n("Get New Themes..."), this);
		QHBoxLayout * hLayout = new QHBoxLayout();
		hLayout->addStretch( 1 );
		hLayout->addWidget( d->m_knsButton );
		layout->addLayout( hLayout );
		connect(d->m_knsButton, &QAbstractButton::clicked,
		        this, [this]() { d->_k_showNewStuffDialog(); });
	}
}

KgThemeSelector::~KgThemeSelector() = default;

void KgThemeSelectorPrivate::fillList()
{
	m_list->clear();
	const auto themes = m_provider->themes();
	for (const KgTheme* theme : themes) {
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

void KgThemeSelectorPrivate::_k_updateListSelection(const KgTheme* theme)
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

void KgThemeSelectorPrivate::_k_updateProviderSelection()
{
	const QListWidgetItem* selItem = m_list->selectedItems().value(0);
	if (!selItem)
	{
		return;
	}
	const QByteArray selId = selItem->data(KgThemeDelegate::IdRole).toByteArray();
	//select the theme with this identifier
	const auto themes = m_provider->themes();
	for (const KgTheme* theme : themes) {
		if (theme->identifier() == selId)
		{
			m_provider->setCurrentTheme(theme);
		}
	}
}

void KgThemeSelectorPrivate::_k_showNewStuffDialog()
{
	QPointer<KNS3::DownloadDialog> dialog(new KNS3::DownloadDialog(q));
	dialog->exec();
	if (dialog && !dialog->changedEntries().isEmpty())
	{
		m_provider->rediscoverThemes();
		fillList();
	}
	//restore previous selection
	_k_updateListSelection(m_provider->currentTheme());
	delete dialog;
}

class KgThemeSelector::Dialog : public QDialog
{
	public:
		Dialog(KgThemeSelector* sel, const QString& caption)
                        : mSelector(sel)
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

				QPushButton *stuff = new QPushButton(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")), btn->text());
				buttonBox->addButton(stuff, QDialogButtonBox::ActionRole);
				buttonBox->addButton(QDialogButtonBox::Close);

			        connect(stuff, &QAbstractButton::clicked, btn, &QAbstractButton::clicked);
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
		void closeEvent(QCloseEvent* event) override
		{
			event->accept();
			//delete myself, but *not* the KgThemeSelector
			mSelector->setParent(nullptr);
			deleteLater();
			//restore the KNS button
			if (mSelector->d->m_knsButton)
			{
				mSelector->d->m_knsButton->show();
			}
		}
       private:
              KgThemeSelector* mSelector;
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
	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);
	//draw thumbnail
	QRect thumbnailBaseRect = this->thumbnailRect(baseRect);
	const QPixmap thumbnail = index.data(Qt::DecorationRole).value<QPixmap>();
	QApplication::style()->drawItemPixmap(painter, thumbnailBaseRect, Qt::AlignCenter, thumbnail);

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
