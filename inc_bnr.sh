#!/bin/bash
typeset -i var=$(cat $1)
((var=var+1))
echo "$var" 