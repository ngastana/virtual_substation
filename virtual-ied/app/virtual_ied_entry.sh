#!/bin/bash
rsyslogd -n &

/app/virtualied $1 $2

