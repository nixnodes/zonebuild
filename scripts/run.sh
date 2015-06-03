#!/bin/bash

BASE_PATH=`dirname ${0}`

. "${BASE_PATH}/config" || exit 2
. "${BASE_PATH}/common" || exit 2

USAGE_STR="USAGE: ./`basename ${0}` <options> .."

[[ -z "${@}" ]] && {
	print_usage_and_exit	
}


rm -f ${OUT_PATH}/tier[0-9]/*.db ${OUT_PATH}/tier[0-9]/*.conf

[[ "${@}" = *root* ]] && {
	echo "${0}: [T0] processing tier0.."	
	${BASE_PATH}/build_tier0.sh dn42	
}

[[ "${@}" = *zone* ]] && {
	for item in ${TIER1_ZONES[@]}; do	
		echo "${0}: [T1] processing '${item}'"	
		${BASE_PATH}/build_tier1.sh ${item}
	done
}

[[ "${@}" = *arpa* ]] && {
	[[ "${ARPA_TIERS}" = *1*  ]] && {
		for item in ${ARPA_ZONES[@]}; do
			echo "${0}: [T1-A]: processing ${item}"
			${BASE_PATH}/build_tier1_arpa.sh ${item} 1 1
		done
	}
	[[ "${ARPA_TIERS}" = *2*  ]] && {
		for item in ${ARPA_ZONES[@]}; do
			echo "${0}: [T2-A]: processing ${item}"
			${BASE_PATH}/build_tier2_arpa.sh ${item}	 
		done
	}
}

