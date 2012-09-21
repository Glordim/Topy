#ifndef __GET_INFO_H__
#define __GET_INFO_H__

char		get_state(const char* pid);
unsigned int	get_proc_usage(const char* pid);
unsigned int	get_mem(const char* pid);

#endif

