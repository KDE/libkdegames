/*
    This file is part of the KDE games library
    Copyright 2008 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __CARDDECKINFO_H_
#define __CARDDECKINFO_H_

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <libkdegames_export.h>

/**
 * Namespace to supply access to card deck information, such as a list of all
 * card decks as well as allowing to access the actual files to render the
 * decks.
 */
namespace CardDeckInfo
{
   /** Retrieve the SVG file belonging to the given card deck (back side). 
    * @param name The name of the back deck.
    * @return The file name and path to the SVG file or QString() if not available. 
    */
   KDEGAMES_EXPORT QString backSVGFilePath(const QString& name);

   /** Retrieve the SVG file belonging to the given card set (front side). 
    * The SVG IDs used for the card back is '1_club' for Ace of clubs, '10_spade' for
    * 10 of spades, 'queen_heart' for Queen of Hearts, '2_diamond' for 2 of diamonds and
    * so on.
    * @param name The name of the card set.
    * @return The file name and path to the SVG file or QString() if not available. 
    */
   KDEGAMES_EXPORT QString frontSVGFilePath(const QString& name);

   /** Check whether the card set is SVG or not.
    * @param name The name of the card set.
    * @return True if SVG data is available.
    */
   KDEGAMES_EXPORT bool isSVGBack(const QString& name);

   /** Check whether the card back deck contains also an SVG file.
    * @param name The name of the card deck.
    * @return True if SVG data is available.
    */
   KDEGAMES_EXPORT bool isSVGFront(const QString& name);
   
   /** Retrieve the name of the default card set (front side).
    * @param pAllowPNG  Allow selection of fixed size cards sets.
    * @return The default card set name.
    */
   KDEGAMES_EXPORT QString defaultFrontName(bool pAllowPNG = true);
   
   /** Retrieve the name of the default card deck (back side).
    * @param pAllowPNG  Allow selection of fixed size cards sets.
    * @return The default card deck name.
    */
   KDEGAMES_EXPORT QString defaultBackName(bool pAllowPNG = true);
   
   /** Retrieve a random card set (front side).
    * @param pAllowPNG  Allow selection of fixed size cards sets.
    * @return A radnom card set name.
    */
   KDEGAMES_EXPORT QString randomFrontName(bool pAllowPNG = true);
 
   /** Retrieve a random card deck (back side).
    * @param pAllowPNG  Allow selection of fixed size cards sets.
    * @return A radnom card deck name.
    */
   KDEGAMES_EXPORT QString randomBackName(bool pAllowPNG = true);

   /**
    * Retrieve the directory where the card front sides are stored. The cards are
    * named 1.png, 2.png, etc. For SVG card decks use @ref cardSVGFilePath.
    * @param name The name of the card set.
    * @return The directory.
    */
   KDEGAMES_EXPORT QString frontDir(const QString& name);

   /**
    * Retrieve the filename of the card back side. 
    * For SVG  decks use @ref deckSVGFilePath.
    * @param name The name of the card deck.
    * @return The filename.
    */
   KDEGAMES_EXPORT QString backFilename(const QString& name);

   /**
    * retrieve a list of all installed backsides
    * @returns a list of backside names, which can be 
    * used as input to the other functions.
    */
   KDEGAMES_EXPORT QStringList backNames();

   /**
    * retrieve a list of all installed frontsides
    * @return a list of frontside names, which can be
    * used as input to the other functions.
    */
   KDEGAMES_EXPORT QStringList frontNames();

}

#endif
