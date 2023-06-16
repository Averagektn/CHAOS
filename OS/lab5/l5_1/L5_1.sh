#!/bin/bash
find $3 -type f -size +$1 -size -$2 -printf "PATH: %h SIZE: %s FILE NAME: %f\n" | sed 1,20p
