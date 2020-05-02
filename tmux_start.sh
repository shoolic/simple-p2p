#!/bin/bash

SESSION="simple-p2p"
WINDOW="main"
SESSIONEXISTS=$(tmux list-sessions | grep $SESSION)

if [ "$SESSIONEXISTS" = "" ];
then
	tmux new-session -d -s $SESSION 
	tmux rename-window -t "$SESSION:0"  $WINDOW
	tmux split-window -t "$SESSION:$WINDOW" -h 
	tmux split-window -t "$SESSION:$WINDOW" -h 
	tmux select-layout -t "$SESSION:$WINDOW" even-horizontal 
	tmux split-window -t "$SESSION:$WINDOW.1" -v 
	tmux split-window -t "$SESSION:$WINDOW.3" -v

	docker-compose -p simple_p2p up -d --build --scale host=2

	tmux send-keys -t "$SESSION:$WINDOW.1" 'docker attach simple_p2p_host_1' C-m
	tmux send-keys -t "$SESSION:$WINDOW.2" 'docker exec -it simple_p2p_host_1 bash' C-m
	tmux send-keys -t "$SESSION:$WINDOW.3" 'docker attach simple_p2p_host_2' C-m
	tmux send-keys -t "$SESSION:$WINDOW.4" 'docker exec -it simple_p2p_host_2 bash' C-m
	
	tmux attach-session -t $SESSION
	tmux select-pane -t "$SESSION:$WINDOW.0"

fi
