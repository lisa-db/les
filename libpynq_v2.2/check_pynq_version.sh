#!/bin/sh
if [ -s "/etc/netplan/default.yaml" ] ; then echo "sd release 1.0.0"; else echo "sd release 2.0.0";  fi
