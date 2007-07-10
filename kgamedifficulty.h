/*
Copyright (c) 2007, Nicolas Roffet, <nicolas-kde@roffet.com>

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef KGAMEDIFFICULTY_H
#define KGAMEDIFFICULTY_H

#include <QtCore/QStringList>

class KXmlGuiWindow;

#include <libkdegames_export.h>

class KGameDifficultyPrivate;

/**
 * @brief KGameDifficuty
 */
class KDEGAMES_EXPORT KGameDifficulty : public QObject
{
	Q_OBJECT

	public:
		/**
		 * @brief Constructor with standard difficuty levels
		 *
		 * @param window The window where to add menu items.
		 * @param restartByChange Does the game have to be restarted, if the difficulty level changes? In this case, a confirmation dialog is displayed.
		 * @param number Number of different difficulty levels
		 */
		KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, int number);

		/**
		 * @brief Constructor with custom difficulty levels
		 *
		 * @param window The window where to add menu items.
		 * @param restartByChange Does the game have to be restarted, if the difficulty level changes? In this case, a confirmation dialog is displayed.
		 * @param texts List of custumized level texts.
		 */
		KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, QStringList& texts);

		virtual ~KGameDifficulty();

		/**
		 * @brief A special item is added for custom difficulty level
		 */
		void addCustomLevel();

	Q_SIGNALS:
		void levelChanged(int level);


	public Q_SLOTS:
		/**
		 * @brief Player wants to change the difficulty level
		 *
		 * The difference with the mehtode "setLevel" is that the user may have to confirm that he agrees to end the current game (if needed).
		 */
		void changeLevel(int level);

		/**
		 * @brief Set if the difficulty level may be changed.
		 * If not, all the actions are disabled.
		 * Default is "true".
		 */
		void setEnabled(bool enabled);

		/**
		 * @brief Set the new current difficulty level
		 */
		void setLevel(int level);

		/**
		 * @brief Set the game state: Running or not
		 * The game state should be defined if the value of m_restartByChange is true.
		 */
		void setRunning(bool running);

	private:
		friend class KGameDifficultyPrivate;
		KGameDifficultyPrivate* const d;

		Q_DISABLE_COPY(KGameDifficulty)
};

#endif //KGAMEDIFFICULTY_H
