#!/bin/bash
#@VERSION:0
#@REVISION:46

if [ -n "${4}" ]; then
	ucfile="${4}"
else
	ucfile="config.user"
fi

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}` <root zone path> [<build glues>] [<process rfc2317 supernets>]"

[ -z ${1} ] && {
	print_usage_and_exit	
}

mkdir -p ${OUT_PATH}/tier1

ROOT_FN=`${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${1} -lom "nslevel <= 1 && nscount > 0"  -preprint "{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr{:n}"`

[ -z "${ROOT_FN}" ] && {
	exit 1
}

#T1_ROOTS=`generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${ROOT_FN}.arpa noglue`

${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${1} --server "${SERVER_NAME_TIER1_ARPA}" --email "${CONTACT_EMAIL}" \
	-lom "nscount > 0 && rfc2317 = 0 && nslevel <= 2  && (pfxsize%8) = 0" \
 	-print "{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr.arpa. {(?P:(?p: )#%-20s)}  {:t} IN {:t}NS  {:t} {?rd:(nserver):([.]+$)}. {:n}" \
 	-preprint "\$TTL ${DEFAULT_TTL}{:n}{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr.arpa. {(?P:(?p: )#%-20s)}  {:t} IN {:t}SOA {:t} {server}. {email}. ({curtime} 14400 3600 9600 ${DEFAULT_TTL}){:n}""{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr.arpa. {(?P:(?p: )#%-20s)} {:t} IN{:t}{:t}TXT{:t}{:t} \"ver=${VERSION},person=${PERSON_HANDLE},rev=`mtn_get_revision ${REGISTRY_PATH}`,ts=`date +%s`\"{:n}" \
 	> ${OUT_PATH}/tier1/${ROOT_FN}.db || rm -f ${OUT_PATH}/tier1/${ROOT_FN}.db
 	
	
cu_add_master_zone ${OUT_PATH}/tier1/named.conf ${ROOT_FN}.arpa ${OUT_PATH}/tier1/${ROOT_FN}.db || {
	! [ $? -eq 1 ] && {
		echo "${ROOT_FN}: configuration update failed : $? | ${OUT_PATH}"
		exit 2
	}
}

case "${2}" in 
	1)
	${ZBUILD} -build inetnum --path  ${REGISTRY_PATH}/inetnum --root ${1} \
		-l: nserver -regex "\.${AVAILABLE_TLDS}$" \
		-lom "nscount > 0 && rfc2317 = 0 && nslevel <= 2 && hasglue = 1 && (pfxsize%8) = 0" \
		-print "{(nserver#%-30s)} {:t} IN{:t}A{?L:nsglueip = 0:(?p:AAA):(noop)}  {:t} {nsglue}{:n}" \
		| sort -n | uniq > ${OUT_PATH}/tier1/${ROOT_FN}.glues.db || rm -f ${OUT_PATH}/tier1/${ROOT_FN}.glues.db
	;;
esac

build_rfc2317_supernet_records()
{
	RFC2317_ALL=(`${ZBUILD} -build inetnum --path ${REGISTRY_PATH}/inetnum --root ${1} \
				-lom "rfc2317 = 1" -lom "([p:nscount]) = 0 || nslevel = 1" \
	 			--noshadow -print '{?Q:({?C:1:startip\}.\{?C:2:startip\}.\{?C:3:startip\})}{:n}' | sort -u`)

	for i in ${RFC2317_ALL[@]}; do		
		generate_forward_zone ${REGISTRY_PATH}/dns/dn42-servers.dn42 ${i}.in-addr.arpa noglue >> ${OUT_PATH}/tier1/${ROOT_FN}.db
		
	done
}

case "${3}" in 
	1)
		build_rfc2317_supernet_records ${1}
 	;;
esac


exit 0