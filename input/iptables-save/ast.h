/* iptables-save syntax tree data structure definitions
 *
 * Copyright (c) 2003,2004 Jamie Wilkinson <jaq@spacepants.org>
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

#ifndef __IPTABLES_AST_H__
#define __IPTABLES_AST_H__

struct pkt_count_s {
    char * in;
    char * out;
};

struct identifier_s {
    char * string;
};

struct not_identifier_s {
    int neg;
    struct identifier_s * identifier;
};

struct range_s {
    struct identifier_s * start;
    struct identifier_s * end;
};

struct not_range_s {
    int neg;
    struct range_s * range;
};

enum ipts_option_type { IPTS_OPT_IN_INTERFACE,
			IPTS_OPT_OUT_INTERFACE,
			IPTS_OPT_JUMP,
			IPTS_OPT_SOURCE,
			IPTS_OPT_DESTINATION,
			IPTS_OPT_MATCH,
			IPTS_OPT_PROTOCOL,
			IPTS_OPT_DPORT,
			IPTS_OPT_SPORT,
			IPTS_OPT_TO_PORTS,
			IPTS_OPT_TO_SOURCE,
			IPTS_OPT_STATE,
			IPTS_OPT_LIMIT,
			IPTS_OPT_LOG_PREFIX,
			IPTS_OPT_UID_OWNER,
			IPTS_OPT_TCP_FLAGS,
			IPTS_OPT_REJECT_WITH,
			IPTS_OPT_ICMP_TYPE,
			IPTS_OPT_FRAGMENT,
			IPTS_OPT_CLAMP_MSS_TO_PMTU,
			IPTS_OPT_HELPER,
			IPTS_OPT_SYN,
};

struct option_s {
    enum ipts_option_type type;

    struct not_identifier_s * not_identifier;
    struct identifier_s * identifier;
    struct not_range_s * not_range;
    struct identifier_s * flags;
    struct identifier_s * mask;
};

struct not_option_s {
    int neg;
    struct option_s * option;
};
    
struct option_list_s {
    struct option_list_s * option_list;
    struct not_option_s * not_option;
};

struct rule_s {
    char * chain;
    char * policy;
    struct pkt_count_s * pkt_count;
    struct option_list_s * option_list;
};

struct rule_list_s {
    struct rule_list_s * list;
    struct rule_s * rule;
};

struct table_s {
  char * name;
  struct rule_list_s * rule_list;
};

struct table_list_s {
  struct table_list_s * list;
  struct table_s * table;
};

struct ast_s {
    struct table_list_s * list;
};

#endif /* __IPTABLES_AST_H__ */
