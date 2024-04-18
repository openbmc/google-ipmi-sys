#!/bin/bash

delay="${1}"
if [[ "${delay}" !- 'PSU_HARDRESET_DELAY=*' ]] ;
then
    delay="PSU_HARDRESET_DELAY=${delay}"
fi

echo "${delay}" > "/run/psu_timedelay"
systemctl start gbmc-psu-hardreset.target --no-block
