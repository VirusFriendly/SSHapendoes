#!/bin/bash

service rsyslog start
service ssh start

tail -f /var/log/auth.log
