/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>.
Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
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

#ifndef KSCOREDIALOG_H
#define KSCOREDIALOG_H

#include <kdialogbase.h>

/**
 * A simple high score dialog.
 */
class KScoreDialog : public KDialogBase {
public:
        enum Fields { None = 0, Level = 1 };
        /**
         * @param latest Ranking of the latest entry. [1 - 10]
         * Use 0 for none.
         * @param fields Show additional fields besides Rank, Name and Score.
         */
	KScoreDialog(int latest=0, int fields = None, QWidget *parent=0, const char *name=0);
};

#endif // !KSCOREDIALOG_H
