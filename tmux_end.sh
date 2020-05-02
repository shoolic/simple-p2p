#!/bin/sh

SESSION="simple-p2p"
SESSIONEXISTS=$(tmux list-sessions | grep $SESSION)

if [ "$SESSIONEXISTS" != "" ]
then
  docker-compose -p simple_p2p down
	tmux kill-session -t $SESSION
fi
