/* **************************************************************************
                           KGameMisc Class
                           -------------------
    begin                : 15 April 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni
    email                : b_mann@gmx.de and martin@heni-online.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
/*
    $Id$
*/

#include <krandomsequence.h>
#include <klocale.h>

#include "kgamemisc.h"

class KGameMiscPrivate
{
public:
	KGameMiscPrivate()
	{
	}

};

KGameMisc::KGameMisc()
{
// not yet used
// d = new KGamePrivate;
}

KGameMisc::~KGameMisc()
{
 // don't forget to delete it as soon as it is used!
// delete d;
}

QString KGameMisc::randomName()// do we need i18n? I think yes
{
//AB: perhaps we need locales here? Ie do not return "Andreas" for a chinese
//country?

// this is not an index but a count - starting with 1 not 0
// increase this if you add a name!
 int players = 279;

 KRandomSequence random;
 switch(random.getLong(players)) {
 // I guess a copyright (c) 2001 by kde-common/accounts would fit well ;)
   case 0: return i18n("Achim");
   case 1: return i18n("Adam");
   case 2: return i18n("Adriaan");
   case 3: return i18n("Adrian");
   case 4: return i18n("Alain");
   case 5: return i18n("Albert");
   case 6: return i18n("Aleksey");
   case 7: return i18n("Alessandro");
   case 8: return i18n("Alex");
   case 9: return i18n("Alexander");
   case 10: return i18n("Alexei");
   case 11: return i18n("Alistair");
   case 12: return i18n("Anders");
   case 13: return i18n("Andre");
   case 14: return i18n("Andrea");
   case 15: return i18n("Andreas");
   case 16: return i18n("Andrej");
   case 17: return i18n("Andrew");
   case 18: return i18n("Andris");
   case 19: return i18n("Andy");
   case 20: return i18n("Antonio");
   case 21: return i18n("Antti");
   case 22: return i18n("Armen");
   case 23: return i18n("Arnt");
   case 24: return i18n("Artur");
   case 25: return i18n("Bart");
   case 26: return i18n("Bavo");
   case 27: return i18n("Ben");
   case 28: return i18n("Benoit");
   case 29: return i18n("Bernd");
   case 30: return i18n("Bertrand");
   case 31: return i18n("Bill");
   case 32: return i18n("Billy");
   case 33: return i18n("Bo");
   case 34: return i18n("Boris");
   case 35: return i18n("Bradley");
   case 36: return i18n("Brian");
   case 37: return i18n("Burkhard");
   case 38: return i18n("Carlo");
   case 39: return i18n("Carsten");
   case 40: return i18n("Charles");
   case 41: return i18n("Chris");
   case 42: return i18n("Christian");
   case 43: return i18n("Christoph");
   case 44: return i18n("Christophe");
   case 45: return i18n("Christopher");
   case 46: return i18n("Chuck");
   case 47: return i18n("Claudiu");
   case 48: return i18n("Cornelius");
   case 49: return i18n("Corrin");
   case 50: return i18n("Cristian");
   case 51: return i18n("Daniel");
   case 52: return i18n("Danko");
   case 53: return i18n("Darian");
   case 54: return i18n("Darius");
   case 55: return i18n("Dave");
   case 56: return i18n("David");
   case 57: return i18n("Dawit");
   case 58: return i18n("Denis");
   case 59: return i18n("Denny");
   case 60: return i18n("Derek");
   case 61: return i18n("Dieter");
   case 62: return i18n("Dima");
   case 63: return i18n("Dimitris");
   case 64: return i18n("Dirk");
   case 65: return i18n("Don");
   case 66: return i18n("Doreen");
   case 67: return i18n("Duncan");
   case 68: return i18n("Ed");
   case 69: return i18n("Eggert");
   case 70: return i18n("Elfed");
   case 71: return i18n("Emily");
   case 72: return i18n("Enno");
   case 73: return i18n("Eric");
   case 74: return i18n("Erik");
   case 75: return i18n("Espen");
   case 76: return i18n("Falk");
   case 77: return i18n("Ferdinand");
   case 78: return i18n("Florian");
   case 79: return i18n("Florin");
   case 80: return i18n("Francois");
   case 81: return i18n("Frank");
   case 82: return i18n("Frederik");
   case 83: return i18n("Frerich");
   case 84: return i18n("Frugal");
   case 85: return i18n("Gary");
   case 86: return i18n("Gaute");
   case 87: return i18n("Geert");
   case 88: return i18n("Geoff");
   case 89: return i18n("George");
   case 90: return i18n("Gerrit");
   case 91: return i18n("Gino");
   case 92: return i18n("Gintaras");
   case 93: return i18n("Glen");
   case 94: return i18n("Gorkem");
   case 95: return i18n("Greg");
   case 96: return i18n("Gregor");
   case 97: return i18n("Gregory");
   case 98: return i18n("Guillaume");
   case 99: return i18n("Guillermo");
   case 100: return i18n("Hagen");
   case 101: return i18n("Hans");
   case 102: return i18n("Harri");
   case 103: return i18n("Hasso");
   case 104: return i18n("Hauke");
   case 105: return i18n("Helge");
   case 106: return i18n("Henning");
   case 107: return i18n("Hermann");
   case 108: return i18n("Holger");
   case 109: return i18n("Ian");
   case 110: return i18n("Igor");
   case 111: return i18n("Ilya");
   case 112: return i18n("Ingimar");
   case 113: return i18n("Ivan");
   case 114: return i18n("Jacek");
   case 115: return i18n("Jaime");
   case 116: return i18n("Jake");
   case 117: return i18n("James");
   case 118: return i18n("Jan");
   case 119: return i18n("Jaromir");
   case 120: return i18n("Jason");
   case 121: return i18n("Jean");
   case 122: return i18n("Jeff");
   case 123: return i18n("Jens");
   case 124: return i18n("Jeremy");
   case 125: return i18n("Jesper");
   case 126: return i18n("Jim");
   case 127: return i18n("Jing-Jong");
   case 128: return i18n("Jo");
   case 129: return i18n("Joan");
   case 130: return i18n("Jochen");
   case 131: return i18n("Joerg");
   case 132: return i18n("Johannes");
   case 133: return i18n("John");
   case 134: return i18n("Jonas");
   case 135: return i18n("Jonathan");
   case 136: return i18n("Jono");
   case 137: return i18n("Josef");
   case 138: return i18n("Joseph");
   case 139: return i18n("Jost");
   case 140: return i18n("Judin");
   case 141: return i18n("Juliet");
   case 142: return i18n("Kai");
   case 143: return i18n("Kalle");
   case 144: return i18n("Kari");
   case 145: return i18n("Karl");
   case 146: return i18n("Karol");
   case 147: return i18n("Keith");
   case 148: return i18n("Ken");
   case 149: return i18n("Kevin");
   case 150: return i18n("Kim");
   case 151: return i18n("Kirk");
   case 152: return i18n("Klaas");
   case 153: return i18n("Klaudius");
   case 154: return i18n("Kurt");
   case 155: return i18n("Lars");
   case 156: return i18n("Laurent");
   case 157: return i18n("Lauri");
   case 158: return i18n("Lennart");
   case 159: return i18n("Leon");
   case 160: return i18n("Ljubisa");
   case 161: return i18n("Logi");
   case 162: return i18n("Lotzi");
   case 163: return i18n("Lubos");
   case 164: return i18n("Lukas");
   case 165: return i18n("Malte");
   case 166: return i18n("Marcell");
   case 167: return i18n("Marco");
   case 168: return i18n("Marcos");
   case 169: return i18n("Marcus");
   case 170: return i18n("Mario");
   case 171: return i18n("Mark");
   case 172: return i18n("Markku");
   case 173: return i18n("Marko");
   case 174: return i18n("Markus");
   case 175: return i18n("Martijn");
   case 176: return i18n("Martin");
   case 177: return i18n("Mathias");
   case 178: return i18n("Mats");
   case 179: return i18n("Matt");
   case 180: return i18n("Matthias");
   case 181: return i18n("Meinolf");
   case 182: return i18n("Meni");
   case 183: return i18n("Michael");
   case 184: return i18n("Michel");
   case 185: return i18n("Mike");
   case 186: return i18n("Ming");
   case 187: return i18n("Mirko");
   case 188: return i18n("Miroslav");
   case 189: return i18n("Navindra");
   case 190: return i18n("Neil");
   case 191: return i18n("Nick");
   case 192: return i18n("Nicolas");
   case 193: return i18n("Niels");
   case 194: return i18n("Nikita");
   case 195: return i18n("Nikolas");
   case 196: return i18n("Oleg");
   case 197: return i18n("Oliver");
   case 198: return i18n("Omid");
   case 199: return i18n("Oswald");
   case 200: return i18n("Otto");
   case 201: return i18n("Pascal");
   case 202: return i18n("Patrick");
   case 203: return i18n("Paul");
   case 204: return i18n("Pedro");
   case 205: return i18n("Percy");
   case 206: return i18n("Peter");
   case 207: return i18n("Petter");
   case 208: return i18n("Phillipe");
   case 209: return i18n("Phlip");
   case 210: return i18n("Pietro");
   case 211: return i18n("Preston");
   case 212: return i18n("Radko");
   case 213: return i18n("Ralf");
   case 214: return i18n("Reginald");
   case 215: return i18n("Rekha");
   case 216: return i18n("Rene");
   case 217: return i18n("Richard");
   case 218: return i18n("Rik");
   case 219: return i18n("Rinse");
   case 220: return i18n("Robert");
   case 221: return i18n("Roberto");
   case 222: return i18n("Roland");
   case 223: return i18n("Roman");
   case 224: return i18n("Russell");
   case 225: return i18n("Sam");
   case 226: return i18n("Sammy");
   case 227: return i18n("Sander");
   case 228: return i18n("Sandy");
   case 229: return i18n("Schalk");
   case 230: return i18n("Scott");
   case 231: return i18n("Sean");
   case 232: return i18n("Serge");
   case 233: return i18n("Sergey");
   case 234: return i18n("Shaheed");
   case 235: return i18n("Shlomi");
   case 236: return i18n("Simon");
   case 237: return i18n("Sirtaj");
   case 238: return i18n("Sivakumar");
   case 239: return i18n("Stanislav");
   case 240: return i18n("Stefan");
   case 241: return i18n("Stefanie");
   case 242: return i18n("Steffen");
   case 243: return i18n("Stephan");
   case 244: return i18n("Sven");
   case 245: return i18n("Taiki");
   case 246: return i18n("Teodor");
   case 247: return i18n("Than");
   case 248: return i18n("Theodore");
   case 249: return i18n("Thiago");
   case 250: return i18n("Thomas");
   case 251: return i18n("Thorarinn");
   case 252: return i18n("Thorsten");
   case 253: return i18n("Till");
   case 254: return i18n("Tim");
   case 255: return i18n("Timo");
   case 256: return i18n("Tobias");
   case 257: return i18n("Toivo");
   case 258: return i18n("Torben");
   case 259: return i18n("Torsten");
   case 260: return i18n("Troy");
   case 261: return i18n("Ulrich");
   case 262: return i18n("Uwe");
   case 263: return i18n("Vadim");
   case 264: return i18n("Vasudeva");
   case 265: return i18n("Vladimir");
   case 266: return i18n("Vlatko");
   case 267: return i18n("Waldo");
   case 268: return i18n("Walter");
   case 269: return i18n("Wang");
   case 270: return i18n("Warwick");
   case 271: return i18n("Wayne");
   case 272: return i18n("Werner");
   case 273: return i18n("Wilco");
   case 274: return i18n("Wilton");
   case 275: return i18n("Wolfram");
   case 276: return i18n("Woohyun");
   case 277: return i18n("Wynn");
   case 278: return i18n("Yves");
   default: return i18n("Unknwon");
 };
}
