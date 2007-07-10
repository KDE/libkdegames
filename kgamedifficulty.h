/*
Copyright (c) 2007, Nicolas Roffet, <nicolas-kde@roffet.com>

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef KGAMEDIFFICULTY_H
#define KGAMEDIFFICULTY_H



#include <QtCore/QStringList>


class KSelectAction;
class KXmlGuiWindow;


// Pourquoi?
#include <libkdegames_export.h>



//TODO: Use D-Pointer
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
		KDEGAMES_EXPORT KGameDifficulty(KXmlGuiWindow* window, const bool restartByChange, const int number);

		/**
		 * @brief Constructor with custom difficulty levels
		 *
		 * @param window The window where to add menu items.
		 * @param restartByChange Does the game have to be restarted, if the difficulty level changes? In this case, a confirmation dialog is displayed.
		 * @param texts List of custumized level texts.
		 */
		KDEGAMES_EXPORT KGameDifficulty(KXmlGuiWindow* window, const bool restartByChange, QStringList& texts);


		KDEGAMES_EXPORT ~KGameDifficulty();


		/**
		 * @brief A special item is added for custom difficulty level
		 */
		KDEGAMES_EXPORT void addCustomLevel();


	Q_SIGNALS:
		KDEGAMES_EXPORT void levelChanged(const int level);


	public Q_SLOTS:
		/**
		 * @brief Player wants to change the difficulty level
		 *
		 * The difference with the mehtode "setLevel" is that the user may have to confirm that he agrees to end the current game (if needed).
		 */
		KDEGAMES_EXPORT void changeLevel(const int level);

		/**
		 * @brief Set if the difficulty level may be changed.
		 * If not, all the actions are disabled.
		 * Default is "true".
		 */
		KDEGAMES_EXPORT void setEnabled(const bool enabled);

		/**
		 * @brief Set the new current difficulty level
		 */
		KDEGAMES_EXPORT void setLevel(const int level);

		/**
		 * @brief Set the game state: Running or not
		 * The game state should be defined if the value of m_restartByChange is true.
		 */
		KDEGAMES_EXPORT void setRunning(const bool running);


	private:
		void createActionsAndMore(KXmlGuiWindow* window, const bool restartByChange);


		KSelectAction* m_menu;

		/**
		 * @brief Current difficulty level
		 */
		int m_level;

		bool m_restartByChange;
		bool m_running;

		QStringList m_texts;
};

#endif //KGAMEDIFFICULTY_H
