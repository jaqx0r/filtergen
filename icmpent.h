#ifndef __ICMPENT_H__
#define __ICMPENT_H__

struct icmpent_s {
    char * i_type;
    char * name;
};

struct icmpent_s * geticmpbyname(char * name);

#endif /* __ICMPENT_H__ */
