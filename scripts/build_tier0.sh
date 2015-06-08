#!/bin/bash
#@VERSION:0
#@REVISION:45

if [ -n "${1}" ]; then
	ucfile="${1}"
else
	ucfile="config.user"
fi

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}`"

mkdir -p ${OUT_PATH}/tier0

generate_soa ${SERVER_NAME_TIER0} "" > ${OUT_PATH}/tier0/root.db
generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ""   >> ${OUT_PATH}/tier0/root.db

#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 arpa noglue >> ${OUT_PATH}/tier0/root.db

for item in ${TIER1_ZONES[@]}; do	
	if [[ "${item}" != "dn42" ]]; then
		generate_forward_zone ${REGISTRY_PATH}/dns/${item} ${item} >> ${OUT_PATH}/tier0/root.db
	else
		generate_forward_zone ${REGISTRY_PATH}/dns/zone-servers.dn42 ${item} >> ${OUT_PATH}/tier0/root.db
	fi
	
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
		
get_icann_root_zone() 
{
	for item in ${ICANN_AXFR_ENABLED_ROOTS[@]}; do
		axfr_zone . ${item} | egrep "${1}" && break
	done
}
		
get_zone_ns_tr() 
{
	for item in ${ICANN_AXFR_ENABLED_ROOTS[@]}; do
		res=`${DIG} @${item} ${1} +trace A` && {
			echo "${res}" | egrep "^${1}." | grep $'IN\tNS\t' | cut -f5 | sed -r 's/\.$//'
			break
		}
		
	done	
}


[ ${MERGE_ICANN_ROOT} -gt 0 ] && get_icann_root_zone '^([^.;]|${MERGE_RESTRICT_ZONES}.)' >> ${OUT_PATH}/tier0/root.db

cu_add_master_zone ${OUT_PATH}/tier0/named.conf "." ${OUT_PATH}/tier0/root.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "in-addr-servers.dn42" ${OUT_PATH}/tier0/in-addr-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root-servers.dn42" ${OUT_PATH}/tier0/root-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "zone-servers.dn42" ${OUT_PATH}/tier0/zone-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "dn42-servers.dn42" ${OUT_PATH}/tier0/dn42-servers.dn42.db
cu_add_master_zone ${OUT_PATH}/tier0/named.conf "root.dn42" ${OUT_PATH}/tier0/root.dn42.db


mkdir -p ${OUT_PATH}/tmp
rm -f ${OUT_PATH}/tmp/*.bt0.tmp

b_path='^('
OCTETS=()

echo "${0}: [T0]: building arpa zones.."

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
		#generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ${o_octet}.in-addr.arpa noglue >> ${OUT_PATH}/tier0/root.db
		cu_add_master_zone ${OUT_PATH}/tier0/named.conf ${o_octet}.in-addr.arpa ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
		
		if [[ "${ZNAME}" != "${o_octet}" ]]; then
			OCTETS=(${OCTETS[@]} ${o_octet})
		else
			OCTETS=(${OCTETS[@]} :${o_octet})
		fi
		
		touch ${OUT_PATH}/tmp/az-${o_octet}.bt0.tmp
	} 
	
	if [[ "${ZNAME}" != "${o_octet}" ]]; then
		generate_forward_zone ${REGISTRY_PATH}/inetnum/${item} ${ZNAME}.in-addr.arpa noglue >> ${OUT_PATH}/tier0/${o_octet}.in-addr.arpa.db
	fi
	
	b_path="${b_path}${ZNAME}|"
	
done

b_path="${b_path}"'1[6-9].172|2[4-9].172|(30|21).172)$'

icann_root=`get_icann_root_zone | egrep '^\.' | \
		egrep 'root-servers.net' | egrep 'IN.*NS'`
		
generate_icann_entries() {
	if ! echo ${1}.${2} | egrep -q "${b_path}"; then		
		ICANN_ITEM_NS=(`get_zone_ns_tr ${1}.${2}.in-addr.arpa`)
	
		if [ ${#ICANN_ITEM_NS[@]} -gt 0 ]; then			
			for ns_item in ${ICANN_ITEM_NS[@]}; do
				echo -e "${1}.${2}.in-addr.arpa.\tIN\tNS\t${ns_item}." >> ${OUT_PATH}/tier0/${2}.in-addr.arpa.db	
			done
		fi
	fi
}

generate_native_entries() {
	generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${1}.${2}.in-addr.arpa noglue >> ${OUT_PATH}/tier0/${2}.in-addr.arpa.db	
}

echo "${0}: [T0]: resolving nameserver information (will take a while).."

for octet in "${OCTETS[@]}"; do
	if [[ "${octet:0:1}" = ":" ]]; then
		octet=${octet:1}
		call=generate_native_entries
	else
		call=generate_icann_entries
	fi
	i=0
	while [ ${i} -lt 256 ]; do
		${call} ${i} ${octet}	
		i=$[i+1]
	done	
done


[ ${TIER0_IPV6} -eq 1 ] && {	

	for zone in ${ARPA_IPV6_ZONES[@]}; do
		zone_parent=`echo ${zone} | sed -r 's/^[0-9a-f]+\.//'`
		generate_soa ${SERVER_NAME_TIER0} ${zone}.ip6.arpa > ${OUT_PATH}/tier0/${zone}.ip6.arpa.db
		generate_forward_zone ${REGISTRY_PATH}/dns/root-servers.dn42 ${zone}.ip6.arpa noglue >> ${OUT_PATH}/tier0/${zone}.ip6.arpa.db
		#generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${zone}.ip6.arpa noglue >> ${OUT_PATH}/tier0/${zone}.ip6.arpa.db		

		i=0
		while [ ${i} -lt 16 ]; do		
			c_zone=`printf %x ${i}`	
			generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${c_zone}.${zone}.ip6.arpa noglue >> ${OUT_PATH}/tier0/${zone}.ip6.arpa.db
			i=$[i+1]
		done

		cu_add_master_zone ${OUT_PATH}/tier0/named.conf ${zone}.ip6.arpa ${OUT_PATH}/tier0/${zone}.ip6.arpa.db

	done		
	
}


exit 0