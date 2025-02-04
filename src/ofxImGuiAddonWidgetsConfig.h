/*
	Configures the user settings on a per-project opt-in basis.
	In c++17 we can auto-enable some with __has_include

	Copyright (C) 2024 Daan de Lange.

	=========================================================================

    04.01.25 - Initial version

*/

#pragma once

#ifndef __ofxImGuiAddonWidgetsConfig__
#define __ofxImGuiAddonWidgetsConfig__

// ofxNDI
#ifndef ofxImGuiAddonWidgets_NDI
#	if __has_include("ofxNDI.h") // C++17 only !
#		define ofxImGuiAddonWidgets_NDI
#	endif
#endif


#endif // __ofxImGuiAddonWidgetsConfig__
