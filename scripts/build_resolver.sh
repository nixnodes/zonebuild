#!/bin/bash

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}`"

rm -f ${OUT_PATH}/res/*

mkdir -p ${OUT_PATH}/res

cu_add_hint_zone ${OUT_PATH}/res/named.conf '.' ${OUT_PATH}/res/hints.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 "" "" 3600000 >> ${OUT_PATH}/res/hints.db

root_ips=(`cat ${OUT_PATH}/res/hints.db | egrep 'IN A' | sort -u | cut -d ' ' -f 5`)

unset forwarders

for ip in ${root_ips[@]}; do
	forwarders="${forwarders}${ip};"
done

for zone in ${ARPA_ZONES[@]}; do
	ZNAME=(`${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${zone} \
				-lom "treelevel = 1" --nons --noshadow \
	 			-print '{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\})):(noop)}'`)
	
	cu_add_forwarders_zone ${OUT_PATH}/res/named-forwards.conf ${ZNAME}.in-addr.arpa "${forwarders}"
done

for zone in ${ARPA_IPV6_ZONES[@]}; do
	cu_add_forwarders_zone ${OUT_PATH}/res/named-forwards.conf ${zone}.ip6.arpa "${forwarders}"
done

for zone in ${TIER1_ZONES[@]}; do	
	cu_add_forwarders_zone ${OUT_PATH}/res/named-forwards.conf ${zone} "${forwarders}"
done

exit 0