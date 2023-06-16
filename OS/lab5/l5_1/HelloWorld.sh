#!/bin/bash
find $3 -type f -size +$1 -size -$2 -printf 'PATH: %h\nSIZE: %s\nFILE NAME: %f\n\n' | sed -n 1,79p
