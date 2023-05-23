#!/bin/bash

follower_port=(8001\
			   8002\
			   8003)
LEADER_IP=${follower_ip[0]}
LEADER_PORT=${follower_port[0]}

printf $LEADER_PORT
