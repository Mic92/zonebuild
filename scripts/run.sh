#!/bin/bash
#@VERSION:0
#@REVISION:43
#
# Read config first, then copy your settings to config.user 
#
# Build options:
#   root   - root (tier0) zone
#   zone   - dn42., hack., etc.. (tier1)
#   arpa   - in-addr.arpa, ip6.arpa, rfc2317 (tier1,tier2)
#   res    - resolver config and hint zone
#
# Options:
#
#  -nosync - disable mtn registry repo pulls
#  -update - perform git-pull before processing and re-enter
#
# Define arpa build tiers in scripts/config

BASE_PATH=`dirname ${0}`

ucfile="config.user"

. "${BASE_PATH}/config" || exit 2
. "${BASE_PATH}/common" || exit 2

USAGE_STR="USAGE: ./`basename ${0}` <build options> .. <options>"

[[ -z "${@}" ]] && {
	print_usage_and_exit	
}

rm -f ${OUT_PATH}/tier[0-9]/*.db ${OUT_PATH}/tier[0-9]/*.conf

[[ "${@}" = *-nosync* ]] && {
	PULL_BEFORE_BUILD=0
}

for hook in "${OPTION_HOOKS[@]}"; do
	eval "${hook}"
done

for hook in "${PRE_BUILD_HOOKS[@]}"; do
	eval "${hook}"
done

[ -n "${NCONF}" ] && {
	shc_append="${NCONF}"
}

[[ "${@}" = *root* ]] && {
	echo "${0}: [T0] processing tier0.."	
	
	eval "${BASE_PATH}/build_tier0.sh ${shc_append}" || {
		echo "${0}: tier 0 failed: ${?}"
	}
}

[[ "${@}" = *zone* ]] && {
	for item in ${TIER1_ZONES[@]}; do	
		echo "${0}: [T1] processing '${item}'"	
		eval "${BASE_PATH}/build_tier1.sh ${item} ${shc_append}" || {
		echo "${0}: tier 1 failed: ${?}"
	}
	done
}

[[ "${@}" = *arpa* ]] && {
	[[ "${ARPA_TIERS}" = *1*  ]] && {
		
		for item in ${ARPA_ZONES[@]}; do
			echo "${0}: [T1-A]: processing ${item}"
			eval "${BASE_PATH}/build_tier1_arpa.sh ${item} ${BUILD_GLUE_RECORDS} ${BUILD_RFC2317_SUPERNETS} ${shc_append}" || {
				echo "${0}: tier 1 arpa failed: ${?}"
			}			
		done
		[ ${TIER1_IPV6} -eq 1 ] && {
			export SUBNETTR_CONTACT=${CONTACT_EMAIL}
			export SUBNETTR_PERSON=${PERSON_HANDLE}
			export SUBNETTR_PRIMARY=${SERVER_NAME_TIER1_ARPA}
			export SUBNETTR_REVISION=$REVISION
			
			mkdir -p ${OUT_PATH}/ipv6
			cp ${BASE_PATH}/ipv6/subnettr.py ${OUT_PATH}/ipv6
			
			run_subnettr || {
				echo ${0}: subnettr failed
				exit 2;
			}				
			
			for zone in ${ARPA_IPV6_ZONES[@]}; do
				sed -i -r "/^${zone}.ip6.arpa\. IN NS.*/d" ${OUT_PATH}/ipv6/db.${zone}.ip6.arpa
				generate_forward_zone ${REGISTRY_PATH}/dns/in-addr-servers.dn42 ${zone}.ip6.arpa noglue >> ${OUT_PATH}/ipv6/db.${zone}.ip6.arpa	
				cu_add_master_zone ${OUT_PATH}/tier1/named.conf "${zone}.ip6.arpa" ${OUT_PATH}/ipv6/db.${zone}.ip6.arpa
			done
		}
	}
	[[ "${ARPA_TIERS}" = *2*  ]] && {
		for item in ${ARPA_ZONES[@]}; do
			echo "${0}: [T2-A]: processing ${item}"
			${BASE_PATH}/build_tier2_arpa.sh ${item} ${shc_append}	
			eval "${BASE_PATH}/build_tier1_arpa.sh ${item} 0 1 ${shc_append}" || {
				echo "${0}: tier 2 arpa failed: ${?}"
			} 
		done
	}
}

[[ "${@}" = *res* ]] && {
	echo "${0}: [R] processing"	
	${BASE_PATH}/build_resolver.sh ${shc_append}
}

for hook in "${POST_BUILD_HOOKS[@]}"; do
	eval "${hook}"
done
