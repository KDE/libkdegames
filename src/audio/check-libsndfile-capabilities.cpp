/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <sndfile.h>

int main()
{
	//We need Ogg/Vorbis support. If sndfile.h is too old, these enum values
	//will be missing and compiler errors will be generated.
	(void) SF_FORMAT_OGG;
	(void) SF_FORMAT_VORBIS;

	return 0;
}
