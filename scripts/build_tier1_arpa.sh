#!/bin/sh

. `dirname ${0}`/config || exit 2
. `dirname ${0}`/common || exit 2

USAGE_STR="USAGE: ./`basename ${0}` <root zone path> [<build glues>]"

[ -z ${1} ] && {
	print_usage_and_exit	
}

mkdir -p ${OUT_PATH}/tier1

ROOT_FN=`${ZBUILD} --build rev --path ${REGISTRY_PATH} --root ${1} -lom "nslevel <= 1 && nscount > 0"  -preprint "{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr{:n}"`

[ -z "${ROOT_FN}" ] && {
	exit 1
}

${ZBUILD} --build rev --path ${REGISTRY_PATH} --root ${1} --server "${SERVER_NAME_TIER1_ARPA}" --email "${CONTACT_EMAIL}" \
	-lom "nscount > 0 && rfc2317 = 0 && nslevel <= 2" \
 	-print "{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr.arpa. {(?P:(?p: )#%-20s)}  {:t} IN {:t}NS  {:t} {?rd:(nserver):([.]+$)}. {:n}" \
 	-preprint "\$TTL ${DEFAULT_TTL}{:n}{?L:pfxsize >= 24:(?Q:(\{?C:1:startip\}.)):(noop)}{?L:pfxsize >= 16:(?Q:(\{?C:2:startip\}.)):(noop)}{?L:pfxsize >= 8:(?Q:(\{?C:3:startip\}.)):(noop)}in-addr.arpa. {(?P:(?p: )#%-20s)}  {:t} IN {:t}SOA {:t} {server}. {email}. ({curtime} 14400 3600 9600 ${DEFAULT_TTL}){:nl}" \
 	> ${OUT_PATH}/tier1/${ROOT_FN}.db || rm -f ${OUT_PATH}/tier1/${ROOT_FN}.db
 	
 	
cu_add_master_zone ${OUT_PATH}/tier1/named.conf ${ROOT_FN}.arpa. ${OUT_PATH}/tier1/${ROOT_FN}.db >> ${OUT_PATH}/tier1/named.conf || {
	! [ $? -eq 1 ] && {
		echo "${ROOT_FN}: configuration update failed : $? | ${OUT_PATH}"
		exit 2
	}
}
 	
case "${2}" in 
	1)
	${ZBUILD} --build rev --path  ${REGISTRY_PATH} --root ${1} \
		-l: nserver -regex "\.${AVAILABLE_TLDS}$" \
		-lom "nscount > 0 && rfc2317 = 0 && nslevel <= 2 && hasglue = 1" \
		-print "{(nserver#%-30s)} {:t} IN{:t}A{?L:nsglueip = 0:(?p:AAA):(noop)}  {:t} {nsglue}{:n}" \
		| sort -n | uniq > ${OUT_PATH}/tier1/${ROOT_FN}.glues.db || rm -f ${OUT_PATH}/tier1/${ROOT_FN}.glues.db
	;;
esac


 
exit 0