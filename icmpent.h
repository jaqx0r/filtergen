#ifndef __ICMPENT_H__
#define __ICMPENT_H__

struct icmpent_s {
    const char * i_type;
    const char * name;
};

struct icmpent_s * geticmpbyname(char * name);

#endif /* __ICMPENT_H__ */
