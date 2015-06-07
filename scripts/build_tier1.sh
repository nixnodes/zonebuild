#!/bin/bash
#@VERSION:0
#@REVISION:42

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}` <tld>"

[ -z ${1} ] && {
	print_usage_and_exit	
}

! [ -f "${REGISTRY_PATH}/dns/${1}" ] && {
	echo "${1}: not found"
	exit 2
}

mkdir -p ${OUT_PATH}/tier1

generate_soa ${SERVER_NAME_TIER1} "" > ${OUT_PATH}/tier1/root.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ""  >> ${OUT_PATH}/tier1/root.db

generate_soa ${SERVER_NAME_TIER1} root-servers.dn42 > ${OUT_PATH}/tier1/root-servers.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 root-servers.dn42 >> ${OUT_PATH}/tier1/root-servers.dn42.db

generate_soa ${SERVER_NAME_TIER1} ${1} > ${OUT_PATH}/tier1/${1}.db 

if [[ "${1}" != "dn42" ]]; then
	generate_forward_zone ${REGISTRY_PATH}/dns/${1} ${1} >> ${OUT_PATH}/tier1/${1}.db 
else
	generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 ${1} >> ${OUT_PATH}/tier1/${1}.db 
fi

generate_soa ${SERVER_NAME_TIER1} in-addr-servers.dn42 > ${OUT_PATH}/tier1/in-addr-servers.dn42.db
generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 in-addr-servers.dn42 >> ${OUT_PATH}/tier1/in-addr-servers.dn42.db

#generate_soa ${SERVER_NAME_TIER1} in-addr.arpa > ${OUT_PATH}/tier1/in-addr.arpa.db
#generate_forward_zone ${REGISTRY_PATH}/dns/arpa in-addr.arpa noglue >> ${OUT_PATH}/tier1/in-addr.arpa.db

generate_soa ${SERVER_NAME_TIER1} dn42-servers.dn42 > ${OUT_PATH}/tier1/dn42-servers.dn42.db 
generate_forward_zone ${REGISTRY_PATH}/dns/dn42-servers.dn42 dn42-servers.dn42  >>  ${OUT_PATH}/tier1/dn42-servers.dn42.db 

for file in ${REGISTRY_PATH}/dns/*.${1}; do
	[ -f "${file}" ] || continue
	zone=`basename $file`
	[[ "${TIER1_FORWARD_ZONES_RESTRICT[@]}" = *${zone}* ]] && continue
	printf "%-40s\r" ${zone}
	generate_forward_zone ${file} ${zone} >> ${OUT_PATH}/tier1/${1}.db 
done

generate_soa ${SERVER_NAME_TIER1} zone-servers.dn42 > ${OUT_PATH}/tier1/zone-servers.dn42.db 
generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 zone-servers.dn42  >>  ${OUT_PATH}/tier1/zone-servers.dn42.db 

cu_add_master_zone ${OUT_PATH}/tier1/named.conf "." ${OUT_PATH}/tier1/root.db
#cu_add_master_zone ${OUT_PATH}/tier1/named.conf in-addr.arpa ${OUT_PATH}/tier1/in-addr.arpa.db
cu_add_master_zone ${OUT_PATH}/tier1/named.conf in-addr-servers.dn42 ${OUT_PATH}/tier1/in-addr-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier1/named.conf root-servers.dn42 ${OUT_PATH}/tier1/root-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier1/named.conf zone-servers.dn42 ${OUT_PATH}/tier1/zone-servers.dn42.db 
cu_add_master_zone ${OUT_PATH}/tier1/named.conf dn42-servers.dn42 ${OUT_PATH}/tier1/dn42-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier1/named.conf ${1} ${OUT_PATH}/tier1/${1}.db 
