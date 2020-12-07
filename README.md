# Multiplayer Game - Starship

by Aitor Simona & Victor Chen 

Starship is a 2D simple online shooter based in outer space. Use your skills and laser to 
overpower your enemies! It was developed in the context of the Networks subject in CITM/UPC Barcelona.

## Instructions

Use Down Arrow to move in the direction you are facing
Use Left and Right Arrows to shoot lasers
Use A/D to rotate

## Features

### Developed by Aitor:

Completely Achieved:

- The Game accepts 8 players at the same time
- Handles players joining an leaving at runtime
- World state replication ensures all of the players see the same world.
Replication managers take care of writing an object's state in the server and reading it in the client,
successfully replicating the world. 
- Redundancy implemented for input ensures stable gameplay even when packets are lost.
We are storing many input commands and sending packets again if no ACK is received from client.
- Delivery manager notifies about failed replication deliveries, commands are issued again.
All commands are saved and tracked, failure is notified on timeout (if client has not sent ACK), and issued
again. 
- Client side prediction with server reconciliation, avoids laggy user input.
Your ship moves smoothly since commands are executed asap, server may correct them, and client will recalculate 
all unsent input. 
- Entity interpolation makes sure all objects seem to be updated very frequently, smoother gameplay. Other
ships and lasers have their position and angle interpolated so their movement is smooth, even if packets arrive
with some latency movement is smooth as if it was executed locally (no jumps).
