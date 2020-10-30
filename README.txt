*****Assignment 2*****
Course: CS344
Name: Yu-Wen, Tseng

The adventure game utilizes C to implement a text based adventure game.
http://en.wikipedia.org/wiki/Colossal_Cave_Adventure (Links to an external site.)

You'll write two programs that will introduce you to programming in C on UNIX based systems. The first program will be contained in a file named "tsengyuw.buildrooms.c" and the second program will be "tsengyuw.adventure.c"

In the game, the player will begin in the "START_ROOM" and win the game when going to the "END_ROOM", which causes the game to exit, showing the path taken by the player.
During the game, the player can also enter a command that returns the current time - this feature uses a mutex and multithreading.

The elements that make up an actual room defined inside a room file are listed below, along with some additional specifications:

**A Room Name:
-A room name cannot be assigned to more than one room.
-Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed (thus, no numbers, special characters, or spaces). This restriction is not extended to the room file's filename.
-You must hard code a list of ten different Room Names into your rooms program and have your rooms program randomly assign one of these to each room generated. Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.
**A Room Type:
-The possible room type entries are: START_ROOM, END_ROOM, and MID_ROOM.
-The assignment of which room gets which type should be randomly generated each time the rooms program is run.
-Naturally, only one room should be assigned the START_ROOM type, and only one room should be assigned the END_ROOM type. The rest of the rooms will receive the MID_ROOM type.
**Outbound connections to other rooms:
-There must be at least 3 outbound connections and at most 6 outbound connections from this room to other rooms.
-The oubound connections from one room to other rooms should be assigned randomly each time the rooms program is run.
-Outbound connections must have matching connections coming back: if room A connects to room B, then room B must have a connection back to room A. Because of all of these specs, there will always be at least one path through.
-A room cannot have an outbound connection that points to itself.
-A room cannot have more than one outbound connection to the same room.