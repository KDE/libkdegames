/*
Copyright (c) 2007, Nicolas Roffet, <nicolas-kde@roffet.com>
Copyright (c) 2007, Pino Toscano, <toscano.pino@tiscali.it>

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef KGAMEDIFFICULTY_H
#define KGAMEDIFFICULTY_H



class QObject;


class KGameDifficultyPrivate;
class KXmlGuiWindow;
#include <libkdegames_export.h>



/**
 * @brief KGameDifficuty
 *
 * KGameDifficulty manages the difficulty levels of a game. It provides standard actions on the main menu.
 * This class is a singleton.
 */
class KDEGAMES_EXPORT KGameDifficulty
{
	public:
		/**
		 * @brief Behavior on change
		 *
		 * Does the game have to be restarted, when the player changes the difficulty level? In this case and if the game is running, a confirmation dialog is displayed and the player can cancel the change.
		 */
		enum onChange {
			restartOnChange, /** The current game has to be canceled and a new game will be started on change. */
			noRestartOnChange /** The current game can continue on change. */
		};

		/**
		 * @brief Standard difficulty levels
		 *
		 * If they fit the needs of the game, the standard appellations of the difficulty levels should be used.
		 */
		enum standardLevel {
			ridiculouslyEasy = 10, /** Level "Ridiculously easy" */
			veryEasy = 20, /** Level "Very easy" */
			easy = 30, /** Level "Easy" */
			medium = 40, /** Level "Medium" */
			hard = 50, /** Level "Hard" */
			veryHard = 60, /** Level "Very hard" */
			extremelyHard = 70, /** Level "Extremely hard" */
			impossible = 80, /** Level "Impossible"  */
			configurable = 90, /** Level "Custom". This is a special item to let the player configure the difficulty level. The configuration of the user level has to be implemented in each game using it with an adapted dialog. Example: In a minesweeper game like KMines, the player wants to define the number of rows, columns and mines. */
			custom = 100, /** Any custom appellations for levels */
			noLevel = 110 /** No level */
		};


		/**
		 * @brief Destructor
		 */
		virtual ~KGameDifficulty();

		/**
		 * @brief Initialize the difficulty class
		 *
		 * You must call this class before using the singleton.
		 *
		 * @param window The window where to add menu items.
		 * @param recvr Object that receives the signals and have the following slots.
		 * @param slotStandard Slot to call when the player changed the difficulty level to a standard one. Slot should have the signature like: SLOT(levelChanged(KGameDifficulty::standardLevel))
		 * @param slotCustom Slot to call when the player changed the difficulty level to a custom one. (Optional). Slot should have the signature like: SLOT(customLevelChanged(int))
		 */
		static void init(KXmlGuiWindow* window, const QObject *recvr, const char* slotStandard, const char* slotCustom = 0);

		/**
		 * @brief Set if a new game has to be started by change
		 *
		 * Default is restartOnChange.
		 * @param restart Behavior on change
		 */
		static void setRestartOnChange(onChange restart);

		/**
		 * @brief Add a standard difficulty level
		 *
		 * You should add all the standard difficulty levels you need after the initialization of the class.
		 * Standard difficulty levels are displayed before custom levels (if available).
		 * @param level Standard difficulty level to add
		 */
		static void addStandardLevel(standardLevel level);

		/**
		 * @brief Remove a standard difficulty level
		 *
		 * @param level Standard difficulty level to remove
		 */
		static void removeStandardLevel(standardLevel level);

		/**
		 * @brief Add a custom difficulty level
		 *
		 * If you need custom appellations like "8x8", "Coward", "Courageous", "Tired of life" or whatever, you can define them with this method.
		 * Custom difficulty levels are displayed after standard levels (if available).
		 * @param key Custom level identifier. (It must be distinct for every different level. Trying to add a new level with an allocated key replace the previous level.).
		 * @param appellation Custom level appellation.
		 */
		static void addCustomLevel(int key, const QString& appellation);

		/**
		 * @brief Remove a custom difficulty level
		 *
		 * @param key Custom level identifier.
		 */
		static void removeCustomLevel(int key);

		/**
		 * @brief Set if the difficulty level may be changed.
		 *
		 * If not, all the actions are disabled.
		 * Default is "true".
		 * @param enabled State.
		 */
		static void setEnabled(bool enabled);

		/**
		 * @brief Set the new current difficulty level as a standard one
		 *
		 * @param level Standard level.
		 */
		static void setLevel(standardLevel level);

		/**
		 * @brief Get the current standard difficulty level
		 *
		 * @return The current standard level, or customLevel if a custom level is selected, or noLevel if no difficulty level is selected.
		 */
		static standardLevel level();
                /**
                 * @return current standard level string name
                 */
                static QString levelString();
		/**
		 * @brief Set the new current difficulty level as a custom one
		 *
		 * This sets also the value of the standard level to "custom".
		 * @param key Custom level identifier.
		 */
		static void setLevelCustom(int key);

		/**
		 * @brief Get the current custom difficulty level
		 *
		 * It does only make sense to get the current custom difficulty level, if the value of the level is "custom".
		 * @return The current custom level identifier.
		 */
		static int levelCustom();

		/**
		 * @brief Set the game state: Running or not
		 *
		 * The game state should be defined if the current onChange value is restartByChange.
		 * Default: false.
		 * @param running Running state.
		 */
		static void setRunning(bool running);


	private:
		/**
		 * Private constructor: we are a singleton
		 */
		KGameDifficulty();
		/**
		 * @brief Access to the unique instance of the class
		 *
		 * Be aware to call init first.
		 * @see init
		 */
		static KGameDifficulty* self();

		static KGameDifficulty* instance;
		friend class KGameDifficultyPrivate;
		KGameDifficultyPrivate* const d;

		Q_DISABLE_COPY(KGameDifficulty)
};

#endif //KGAMEDIFFICULTY_H
