#ifndef SHM_MACRO_H_

#define SHM_MACRO_H_


#define COLOR(p_node) (NULL == p_node ? 'B' : p_node->color)

#define AT(p_addr, index) (index < 0 ? NULL : p_addr + index)


#endif

