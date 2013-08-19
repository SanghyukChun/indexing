#ifndef __INDEX_H__
#define __INDEX_H__
/*****************************************************************************/
#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "flow_manager.h"
#include "qsort.h"
/*****************************************************************************/
#define COMPARE_VALUE(a,b) ((a)->value < (b)->value)
/*****************************************************************************/
typedef struct bloom_filter {
	u_char *saddr;
	u_char *daddr;
	u_char *sport;
	u_char *dport;
} bloom_filter_t;
/*****************************************************************************/
typedef struct array_node {
	uint32_t offset;
	uint32_t value;
} array_node_t;
/*****************************************************************************/
typedef struct index_info {
	uint16_t fileID;    /* fileID of given index                 */
	int      first_idx; /* index of start point of given index   */
	int      last_idx;  /* index of last point of given index    */
	bool     is_backup; /* true if index is backup, false if not */
} index_info_t;
/*****************************************************************************/
enum {
	TYPE_SADDR = 1,
	TYPE_DADDR = 2,
	TYPE_SPORT = 4,
	TYPE_DPORT = 8
};
/*****************************************************************************/
enum {
	SEARCH_EXACT,
	SEARCH_MIN,
	SEARCH_MAX
};
/*****************************************************************************/
inline bool init_index_array(indexer_context_t *ictx);
inline void insert_index(indexer_context_t *ictx, FlowMeta *meta);
inline void sort_array(indexer_context_t *ictx);

#endif /* __INDEX_H__ */
