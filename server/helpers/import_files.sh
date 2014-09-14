#!/bin/bash

ls -1t /tmp/????????? | tac | while read file; do 
    [ -f "${file}" ] && {
        echo importing "${file}"
        cat $file | perl -T u1 --nosave
    }

done

chown nginx:nginx /var/lib/tracking/*
