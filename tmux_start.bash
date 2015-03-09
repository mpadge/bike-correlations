#!/bin/sh
cd ./src/
SESSION="bike-correlations"

tmux -2 new-session -d -s $SESSION

tmux new-window -t $SESSION:1 -k -n main
tmux send-keys -t $SESSION:1 'vim main.h' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe main.c++' C-m
tmux split-window -h
tmux send-keys -t $SESSION:1 'vim StationData.h' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe StationData.c++' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe RideData.h' C-m
tmux send-keys -t $SESSION:1 ':' 'tabe RideData.c++' C-m
tmux select-pane -t 0

tmux new-window -t $SESSION:2 -n routines1
tmux select-window -t $SESSION:2
tmux send-keys -t $SESSION:2 'vim Structures.h' C-m
tmux send-keys -t $SESSION:2 ':' 'tabe Utils.h' C-m
tmux send-keys -t $SESSION:2 ':' 'tabe Utils.c++' C-m
tmux split-window -h
tmux send-keys -t $SESSION:2 'cd ..' C-m
tmux select-pane -t 0

tmux new-window -t $SESSION:3 -n python
tmux select-window -t $SESSION:3
tmux send-keys -t $SESSION:3 'vim router.py' C-m
tmux send-keys -t $SESSION:3 ':' 'tabe getStDists.py' C-m
tmux send-keys -t $SESSION:3 ':' 'tabe Utils.py' C-m
tmux split-window -h
tmux select-pane -t 0

cd ..
tmux new-window -t $SESSION:4 -n makefile
tmux select-window -t $SESSION:4
tmux send-keys -t $SESSION:4 'vim .travis.yml' C-m
tmux send-keys -t $SESSION:4 ':' 'tabe tmux_start.bash' C-m
tmux send-keys -t $SESSION:4 ':' 'tabe makefile' C-m
tmux split-window -h
# Environmental variable only held in local tmux environment = that pane.
tmux send-keys -t $SESSION:4 "GD='https://github.com/mpadge/bike-correlations'" C-m
tmux select-pane -t 0

cd ./R/
tmux new-window -t $SESSION:5 -n analyses
tmux select-window -t $SESSION:5
tmux send-keys -t $SESSION:5 'vim analyses.R' C-m

tmux select-window -t $SESSION:1

tmux attach -t $SESSION
