#!/bin/bash


STEP=0
 
  if [ -z "$1" ]; then
                  echo No Max_value. You have to execute: runMultTrans Max_Value Step Net 
                  exit 1
  fi



  if  [ -z "$2" ]; then
                  echo  No Step. You have to execute: runMultTrans Max_Value Step Net
                  exit 1
  fi

  if [ -z "$3" ]; then
                  echo   No Net. You have to execute: runMultTrans Max_Value Step Net
                  exit 1
  fi


while [  $STEP -lt $1 ]; 
do
        echo "****************************************"
        echo step $STEP 
	randomTR $3 $STEP
	mv $3.sta $3.sta$STEP
        let STEP=STEP+$2 
        echo "****************************************"
 	echo 
done

