# 3rdCharmChatbot


This is designed to interface with runelite through the chat logger plugin. It
does not require changes to be made to the client itself, so no runelite
re-compiling needed.


---


Requires runelite plugins / settings:
- Disabled 'Emojis' plugin
- Enable 'Chat Timestamps' plugin
	- 'Timestamp Format' set to "[hh:mm:ss]"
- Install and enable 'Chat Logger' plugin
	- Enable 'Clan  Chat' and 'Folder Per User'
- Enable 'Logout Timer' plugin
	- 'Idle timeout' set to 25 minutes


Required packages:
- libxtst-dev
- [json.h](https://github.com/sheredom/json.h)




