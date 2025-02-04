/*

	OpenFrameworks ofxNDI + ofxImGui example

	Requires ofxImGui : https://github.com/jvcleave/ofxImGui/tree/develop

	Copyright (C) 2024 Daan de Lange.

	=========================================================================
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================

	04.10.24 - Initial creation

*/
#pragma once

#include "ofMain.h"

#include "ofxImGui.h" // Gui
#include "ofxImGuiAddonWidgets.h"

#ifdef ofxImGuiAddonWidgets_NDI
#	include "ofxNDI.h" // NDI classes
#endif

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed( ofKeyEventArgs & key );

		// GUI
		ofxImGui::Gui gui;
		bool bShowGui = true;

		// Demo App
		ofFbo appFbo;               // Fbo used for graphics and sending
		unsigned int appFboWidth = 0;  // Width of the sender output
		unsigned int appFboHeight = 0; // Height of the sender output
		void drawAppGraphics();     // Rotating cube draw

		// Demo App internals
		float rotX = 0.0f;
		float rotY = 0.0f;             // Cube rotation increment
		float hueSpeed = 0.2f;
		float rotXSpeed = 0.551 *.5f;
		float rotYSpeed = 0.624 *.5f;

		// ofxAddon Specific

		// ofxNDI
#ifdef ofxImGuiAddonWidgets_NDI
		// Receiver
		ofxNDIreceiver ndiReceiver;
		ofTexture receiverTexture;
		bool bShowReceiverCtrls = true;

		// Sender
		ofxNDIsender ndiSender;        // NDI sender
		std::string senderName;        // Sender name
		bool bShowSenderCtrls = true;
#endif

};
