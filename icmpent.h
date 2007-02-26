#ifndef __ICMPENT_H__
#define __ICMPENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/** mapping of icmp types to names */
struct icmpent_s {
    const char * i_type;
    const char * name;
};

struct icmpent_s * geticmpbyname(char * name);

#ifdef __cplusplus
}
#endif

#endif /* __ICMPENT_H__ */
