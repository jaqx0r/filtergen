single: neg
        new filter neg
      | input | output
        parse_group
      | accept | drop | reject | masq | redirect
        new filter target (tok)
      | log
        new log message
      | source | dest
        parse_group host
      | sport | dport
        parse_group ports
      | icmptype
        parse_group icmp
      | proto
        parse_group proto
      | localonly | routedonly
        new filter rtype
      | openbrace
        parse_sibs;
      | openbracket
        parse_subgroup;
      | closebrace | closebracket | semicolon
      
      default parseerror
