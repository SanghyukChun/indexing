#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "index.h"
#include "flow_manager.h"

static void
index_main(indexer_context_t *ictx)
{
	memset(ictx, 0, sizeof(indexer_context_t));
	ictx->ic_cpu = 0;
	ictx->ic_sockd = 0;
	if (!init_index_array(ictx))
		return;
	ictx->ic_meta = (FlowMeta *)calloc(META_ARRAY_SIZE, sizeof(FlowMeta));
	if (ictx->ic_meta == NULL) {
		return;
	}

	srand(time(NULL));

	FlowMeta *meta = (FlowMeta *)malloc(sizeof(FlowMeta));
	FlowInfo *info = &meta->flowinfo;

	int cnt, i;
	for (cnt = 0; cnt < 150; cnt++)
	{
		for (i = 0; i < 4; i++) {
			info->saddr = rand();
			info->daddr = rand();
			info->sport = rand();
			info->dport = rand();

			insert_index(ictx, meta);
		}
		if (has_enough_index_node(ictx))
			set_next_fcap_index(ictx);
		else
			clean_fcap_index(ictx);
	}
}

int
main(int argc, char *argv[])
{
	indexer_context_t ictx;
	index_main(&ictx);
	return 0;
}
