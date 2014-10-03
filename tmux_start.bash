#!/bin/sh
cd ./src/
SESSION="bike-correlations"

tmux -2 new-session -d -s $SESSION

tmux new-window -t $SESSION:1 -k -n main
tmux send-keys -t $SESSION:1 'vim Utils.h' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe Utils.c++' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe Structures.h' C-m

tmux split-window -h
tmux send-keys -t $SESSION:1 'vim getR2.h' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe getR2.c++' C-m
tmux select-pane -t 0

tmux new-window -t $SESSION:2 -n routines1
tmux select-window -t $SESSION:2
tmux send-keys -t $SESSION:2 'vim InOut.h' C-m
tmux send-keys -t $SESSION:2 ':' 'tabe InOut.c++' C-m

tmux split-window -h
tmux send-keys -t $SESSION:2 'vim Calculations.h' C-m
tmux send-keys -t $SESSION:2 ':' 'tabe Calculations.c++' C-m
tmux select-pane -t 0

tmux new-window -t $SESSION:3 -n python
tmux select-window -t $SESSION:3
tmux send-keys -t $SESSION:3 'vim router.py' C-m
tmux send-keys -t $SESSION:3 ':' 'tabe getStDists.py' C-m
tmux split-window -h
tmux select-pane -t 0

cd ..
tmux new-window -t $SESSION:4 -n makefile
tmux select-window -t $SESSION:4
tmux send-keys -t $SESSION:4 'vim makefile' C-m
tmux send-keys -t $SESSION:4 ':' 'tabe .travis.yml' C-m
tmux split-window -h
tmux select-pane -t 0

tmux select-window -t $SESSION:3

tmux attach -t $SESSION
