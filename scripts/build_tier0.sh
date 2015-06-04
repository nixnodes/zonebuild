#!/bin/bash

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}`"


mkdir -p ${OUT_PATH}/tier0

rm -f ${OUT_PATH}/tier0/*

generate_soa ${SERVER_NAME_TIER0} "" > ${OUT_PATH}/tier0/root.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ""   >> ${OUT_PATH}/tier0/root.db
#generate_forward_zone ${REGISTRY_PATH}/dns/dn42 dn42 noglue >> ${OUT_PATH}/tier0/root.db
#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 arpa noglue >> ${OUT_PATH}/tier0/root.db

for item in ${TIER1_ZONES[@]}; do	
	generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 ${item} >> ${OUT_PATH}/tier0/root.db
done

generate_soa ${SERVER_NAME_TIER0} root.dn42 > ${OUT_PATH}/tier0/root.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/dn42 root.dn42  >> ${OUT_PATH}/tier0/root.dn42.db

generate_soa ${SERVER_NAME_TIER0} root-servers.dn42 > ${OUT_PATH}/tier0/root-servers.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 root-servers.dn42 >> ${OUT_PATH}/tier0/root-servers.dn42.db

#generate_soa ${SERVER_NAME_TIER0} arpa > ${OUT_PATH}/tier0/arpa.db
#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 arpa noglue >> ${OUT_PATH}/tier0/arpa.db
#generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 in-addr.arpa noglue >> ${OUT_PATH}/tier0/arpa.db

generate_soa ${SERVER_NAME_TIER0} in-addr-servers.dn42 > ${OUT_PATH}/tier0/in-addr-servers.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 in-addr-servers.dn42 >> ${OUT_PATH}/tier0/in-addr-servers.dn42.db

generate_soa ${SERVER_NAME_TIER0} zone-servers.dn42 > ${OUT_PATH}/tier0/zone-servers.dn42.db 
generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 zone-servers.dn42  >>  ${OUT_PATH}/tier0/zone-servers.dn42.db 

generate_soa ${SERVER_NAME_TIER0} dn42-servers.dn42 > ${OUT_PATH}/tier0/dn42-servers.dn42.db 
generate_forward_zone ${REGISTRY_PATH}/dns/dn42-servers.dn42 dn42-servers.dn42  >>  ${OUT_PATH}/tier0/dn42-servers.dn42.db 
		
get_icann_root_zone() {
	for item in ${ICANN_AXFR_ENABLED_ROOTS[@]}; do
		axfr_zone . ${item} | egrep '^[^.;]' && break
	done
}

[ ${MERGE_ICANN_ROOT} -gt 0 ] && get_icann_root_zone | egrep -v '^arpa.' >> ${OUT_PATH}/tier0/root.db

cu_add_master_zone ${OUT_PATH}/tier0/named.conf "." ${OUT_PATH}/tier0/root.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "in-addr-servers.dn42" ${OUT_PATH}/tier0/in-addr-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root-servers.dn42" ${OUT_PATH}/tier0/root-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "zone-servers.dn42" ${OUT_PATH}/tier0/zone-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "dn42-servers.dn42" ${OUT_PATH}/tier0/dn42-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root.dn42" ${OUT_PATH}/tier0/root.dn42.db


mkdir -p ${OUT_PATH}/tmp
rm -f ${OUT_PATH}/tmp/*.bt0.tmp

for item in ${ARPA_ZONES[@]}; do	
	ZNAME=(`${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${item} \
				-lom "treelevel = 1" --nons --noshadow \
	 			-print '{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\})):(noop)}'`)
	
	[ -z "${ZNAME}" ] && {
		echo "${item}: could not resolve zone"
		exit 2
	}
	
	o_octet=`echo ${ZNAME} | sed -r 's/^[0-9]+\.//'` 	
	
	! [ -f "${OUT_PATH}/tmp/az-${o_octet}.bt0.tmp" ] && {
		generate_soa ${SERVER_NAME_TIER0} ${o_octet}.in-addr.arpa >> ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
		generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ${o_octet}.in-addr.arpa noglue >> ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
		cu_add_master_zone ${OUT_PATH}/tier0/named.conf ${o_octet}.in-addr.arpa ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
		touch ${OUT_PATH}/tmp/az-${o_octet}.bt0.tmp
	} 
	
	generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${ZNAME}.in-addr.arpa noglue >> ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
	
done

exit 0