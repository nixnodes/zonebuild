#!/bin/bash

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}`"


mkdir -p ${OUT_PATH}/tier0

generate_soa ${SERVER_NAME_TIER0} "" > ${OUT_PATH}/tier0/root.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ""   >> ${OUT_PATH}/tier0/root.db
#generate_forward_zone ${REGISTRY_PATH}/dns/dn42 dn42 noglue >> ${OUT_PATH}/tier0/root.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 arpa noglue >> ${OUT_PATH}/tier0/root.db

for item in ${TIER1_ZONES[@]}; do	
	generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 ${item} >> ${OUT_PATH}/tier0/root.db
done


generate_soa ${SERVER_NAME_TIER0} root.dn42 > ${OUT_PATH}/tier0/root.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/dn42 root.dn42  >> ${OUT_PATH}/tier0/root.dn42.db

generate_soa ${SERVER_NAME_TIER0} root-servers.dn42 > ${OUT_PATH}/tier0/root-servers.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 root-servers.dn42 >> ${OUT_PATH}/tier0/root-servers.dn42.db

generate_soa ${SERVER_NAME_TIER0} arpa > ${OUT_PATH}/tier0/arpa.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 arpa noglue >> ${OUT_PATH}/tier0/arpa.db
generate_forward_zone ${REGISTRY_PATH}/dns/arpa in-addr.arpa >> ${OUT_PATH}/tier0/arpa.db

generate_soa ${SERVER_NAME_TIER0} in-addr-servers.arpa > ${OUT_PATH}/tier0/in-addr-servers.arpa.db
generate_forward_zone ${REGISTRY_PATH}/dns/arpa in-addr-servers.arpa >> ${OUT_PATH}/tier0/in-addr-servers.arpa.db

generate_soa ${SERVER_NAME_TIER0} zone-servers.dn42 > ${OUT_PATH}/tier0/zone-servers.dn42.db 
generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 zone-servers.dn42  >>  ${OUT_PATH}/tier0/zone-servers.dn42.db 

#rm -f /tmp/build_tier0.$$.tmp
#for item in ${ARPA_ZONES[@]}; do		
#	generate_forward_zone ${REGISTRY_PATH}/inetnum/${item} in-addr.arpa noglue >> /tmp/build_tier0.$$.tmp
#done
#cat /tmp/build_tier0.$$.tmp | sort -u

cu_add_master_zone ${OUT_PATH}/tier0/named.conf "." ${OUT_PATH}/tier0/root.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "arpa" ${OUT_PATH}/tier0/arpa.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root-servers.dn42" ${OUT_PATH}/tier0/root-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "zone-servers.dn42" ${OUT_PATH}/tier0/zone-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root.dn42" ${OUT_PATH}/tier0/root.dn42.db

exit 0