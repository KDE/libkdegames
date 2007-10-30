
install( FILES 
	KCardDialog
	KChat
	KChatBase
	KChatDialog
	KExtHighscore
	KGameCanvas
	KGameClock
	KGameDifficulty
	KGameLCD
	KGameMisc
	KGamePopupItem
	KGameProgress
	KGameSvgDigits
	KGameSvgDocument
	KGameTheme
	KGameThemeSelector
	KGrid2D
	KHighscore
	KScoreDialog
	KStandardGameAction
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE )

install( FILES
	KGame/KGame
	KGame/KGameChat
	KGame/KGameConnectDialog
	KGame/KGameDebugDialog
	KGame/KGameDialog
	KGame/KGameDialogConfig
	KGame/KGameError
	KGame/KGameErrorDialog
	KGame/KGameIO
	KGame/KGameMessage
	KGame/KGameNetwork
	KGame/KGameProcess
	KGame/KGameProperty
	KGame/KGamePropertyArray
	KGame/KGamePropertyHandler
	KGame/KGamePropertyList
	KGame/KGameSequence
	KGame/KGameVersion
	KGame/KMessageClient
	KGame/KMessageIO
	KGame/KMessageServer
	KGame/KPlayer
DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KGame )
