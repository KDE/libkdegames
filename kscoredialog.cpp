/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>.
All rights reserved.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

#include "config.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>

#include "kscoredialog.h"

KScoreDialog::KScoreDialog(int latest, int fields, QWidget *parent, const char *oname)
        : KDialogBase(parent, oname, true, i18n("%1 High Scores").arg(kapp->caption()), Ok, Ok, true)
{
    KConfig *config = kapp->config();
    config->setGroup("High Score");

    QFrame *page = makeMainWidget();
    int colRank = 0;
    int colName = 1;
    int colLevel = 2;
    int colScore = 2;
    if (fields & Level)
       colScore++;
    int nrCols = colScore+1;

    QGridLayout *layout = new QGridLayout(page, 12, nrCols, marginHint() + 20, spacingHint());
    layout->addRowSpacing(1, 15);
    layout->addColSpacing(colRank, 50);
    layout->addColSpacing(colName, 50);
    if (fields & Level)
       layout->addColSpacing(colLevel, 50);
    layout->addColSpacing(colScore, 50);

    QFont bold = font();
    bold.setBold(true);

    QLabel *label;
    label = new QLabel(i18n("Rank"), page);
    layout->addWidget(label, 0, colRank);
    label->setFont(bold);
    label = new QLabel(i18n("Name"), page);
    layout->addWidget(label, 0, colName);
    label->setFont(bold);
    if (fields & Level)
    {
       label = new QLabel(i18n("Level"), page);
       layout->addWidget(label, 0, colLevel, AlignRight);
       label->setFont(bold);
    }
    label = new QLabel(i18n("Score"), page);
    layout->addWidget(label, 0, colScore, AlignRight);
    label->setFont(bold);
    
    KSeparator *sep = new KSeparator(Horizontal, page);
    layout->addMultiCellWidget(sep, 1, 1, 0, nrCols-1);

    
    QString s, name, level, score, num;
    for (int i = 1; i <= 10; ++i) {
        num.setNum(i);
        if (fields & Level)
        {
           s = "Pos" + num + "Level";
           level = config->readEntry(s, "0");
        }
        s = "Pos" + num + "Score";
        score = config->readEntry(s, "0");
        s = "Pos" + num + "Name";
        name = config->readEntry(s, i18n("Noname"));
        s = "#" + num;
	label = new QLabel(s, page);
	if (i == latest) label->setFont(bold);
        layout->addWidget(label, i+1, colRank);
        label = new QLabel(name, page);
	if (i == latest) label->setFont(bold);
        layout->addWidget(label, i+1, colName);
        if (fields & Level)
        {
	    label = new QLabel(level, page);
	    if (i == latest) label->setFont(bold);
            layout->addWidget(label, i+1, colLevel, AlignRight);
        }
        label = new QLabel(score, page);
	if (i == latest) label->setFont(bold);
        layout->addWidget(label, i+1, colScore, AlignRight);
    }
    incInitialSize(QSize(), true); // Fixed.
}
