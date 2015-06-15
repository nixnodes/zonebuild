#!/bin/bash
#@VERSION:0
#@REVISION:45

if [ -n "${2}" ]; then
	ucfile="${2}"
else
	ucfile="config.user"
fi

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}`"

rm -f ${OUT_PATH}/res/*

mkdir -p ${OUT_PATH}/res

generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 "" "" 3600000 > ${OUT_PATH}/res/hints.db
cu_add_hint_zone ${OUT_PATH}/res/named.conf '.' ${OUT_PATH}/res/hints.db

root_ips=(`cat ${OUT_PATH}/res/hints.db | egrep 'IN A' | sort -u | cut -d ' ' -f 5`)

unset forwarders

for ip in ${root_ips[@]}; do
	forwarders="${forwarders}${ip};"
done

for zone in ${ARPA_ZONES[@]}; do
	ZNAME=(`${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${zone} \
				-lom "treelevel = 1" --nons --noshadow \
	 			-print '{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\})):(noop)}'`)
	
	cu_add_hint_zone ${OUT_PATH}/res/named-forwards.conf ${ZNAME}.in-addr.arpa ${OUT_PATH}/res/hints.db
done

for zone in ${ARPA_IPV6_ZONES[@]}; do
	cu_add_hint_zone ${OUT_PATH}/res/named-forwards.conf ${zone}.ip6.arpa ${OUT_PATH}/res/hints.db
done

for zone in ${TIER1_ZONES[@]}; do	
	cu_add_hint_zone ${OUT_PATH}/res/named-forwards.conf ${zone} ${OUT_PATH}/res/hints.db
done



cu_add_hint_zone ${OUT_PATH}/res/named.conf '.' ${OUT_PATH}/res/hints.db


echo "${0}: [R]: generating RFC1918 zones"

i=16
while [ ${i} -lt 32 ]; do
	#generate_soa a.resolvers.dn42 ${i}.172.in-addr.arpa > ${OUT_PATH}/res/${i}.172.in-addr.arpa.db
	#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ${i}.172.in-addr.arpa noglue >> ${OUT_PATH}/res/${i}.172.in-addr.arpa.db
	cu_add_hint_zone ${OUT_PATH}/res/named.conf ${i}.172.in-addr.arpa ${OUT_PATH}/res/hints.db
	i=$[i+1]
done

#generate_soa a.resolvers.dn42 10.in-addr.arpa > ${OUT_PATH}/res/10.in-addr.arpa.db
#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 10.in-addr.arpa noglue >> ${OUT_PATH}/res/10.in-addr.arpa.db
cu_add_hint_zone ${OUT_PATH}/res/named.conf 10.in-addr.arpa ${OUT_PATH}/res/hints.db

exit 0