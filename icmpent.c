/* argument name resolver
 *
 * Copyright (c) 2004 Jamie Wilkinson <jaq@spacepants.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <string.h>
#include "icmpent.h"

/* icmp codes from RFC1700 */
struct icmpent_s icmpcodes[] = {
    { "0", "echo-reply" },
    /* 1-2 unassigned */
    { "3", "destination-unreachable" },
    { "3/0", "network-unreachable" },
    { "3/1", "host-unreachable" },
    { "3/2", "protocol-unreachable" },
    { "3/3", "port-unreachable" },
    { "3/4", "fragmentation-needed" },
    { "3/5", "source-route-failed" },
    { "3/6", "network-unknown" },
    { "3/7", "host-unknown" },
    { "3/8", "source-host-isolated" },
    { "3/9", "network-prohibited" },
    { "3/10", "host-prohibited" },
    { "3/11", "tos-network-unreachable" },
    { "3/12", "tos-host-unreachable" },
    /* RFC1812 defines the next three */
    { "3/13", "communication-prohibited" },
    { "3/14", "host-precedence-violation" },
    { "3/15", "precedence-cutoff" },
    { "4", "source-quench" },
    { "5", "redirect" },
    { "5/0", "redirect-network" },
    { "5/1", "redirect-host" },
    { "5/2", "redirect-tos-network" },
    { "5/3", "redirect-tos-host" },
    { "6", "alternate-host-address" },
    /* 7 unassigned */
    { "8", "echo-request" },
    { "9", "router-advertisement" },
    { "9/0", "router-advertisement-normal" },
    /* RFC2002 */
    { "9/16", "router-advertisement-uncommon" },
    { "10", "router-selection" },
    { "11", "time-exceeded" },
    { "11/0", "time-exceeded-in-transmit" },
    { "11/1", "time-exceeded-fragment-reassembly" },
    { "12", "parameter-problem" },
    { "12/0", "parameter-problem-pointer" },
    /* RFC1108 */
    { "12/1", "parameter-problem-missing-option" },
    { "12/2", "parameter-problem-bad-length" },
    { "13", "timestamp-request" },
    { "14", "timestamp-reply" },
    { "15", "information-request" },
    { "16", "information-reply" },
    { "17", "address-mask-request" },
    { "18", "address-mask-reply" },
    /* 19-29 reserved */
    { "30", "traceroute" },
    { "31", "datagram-conversion-error" },
    { "32", "mobile-host-redirect" },
    { "33", "ipv6-where-are-you" },
    { "34", "ipv6-i-am-here" },
    { "35", "mobile-registration-request" },
    { "36", "mobile-registration-reply" },
    { "37", "domain-name-request" },
    { "38", "domain-name-reply" },
    { "39", "skip" },
    { "40", "photuris"},
    { "40/0", "photuris-bad-spi" },
    { "40/1", "photuris-authn-failed" },
    { "40/2", "photuris-decompression-failed" },
    { "40/3", "photuris-decryption-failed" },
    { "40/4", "photuris-need-authn" },
    { "40/5", "photuris-need-authz" },
    /* 41-255 reserved */
    { NULL, NULL }
};

/* fake netdb-like function for icmp types */
struct icmpent_s * geticmpbyname(char * name) {
    struct icmpent_s * icmpent;

    for (icmpent = icmpcodes; icmpent->i_type != NULL; icmpent++) {
	if (!strcmp(name, icmpent->name))
	    break;
    }
    if (icmpent->i_type == NULL)
        icmpent = NULL;

    return icmpent;
}
