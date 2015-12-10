# LivestreamerGUI

Graphical interface used to launch [Livestreamer](http://livestreamer.tanuki.se/) from links to the most watched [Twitch](twitch.tv) streams.

## Features
* Using the Twitch API, the currently 100 most watched streams are shown.
* Automatic refresh of the streams list.
* Thumbnail for selected stream, along with basic stream informations.
* Filter by language or game.
* Inverse filter (if say you do not want to watch LoL).
* Filters are saved.
* Buttons to launch Livestreamer for the selected stream using a specific quality.

## Screenshot
![Capture of the interface](http://s27.postimg.org/ho8apvdwz/Livestreamer_GUI.png)

## Configuration
This utility does not install, configure, nor update Livestreamer.  
Livestreamer must be added to the path (done by default by its installer).

## Code 
C++ with Qt for GUI, network and JSON.  
Used part of the Qxt library for its QxtCheckComboBox.
