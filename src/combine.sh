#!/bin/bash

variables=`cat variables.c`
variables="${variables//$'\n'/$'\\'$'\n'}"
cat main.c | sed "/PRECOMPUTED VALUES/a $variables" > main_generated.c
