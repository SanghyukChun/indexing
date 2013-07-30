typedef struct query_context {
	struct index_array_context_t *ictx;
	struct bpf_program *bpf;
	unsigned int stime;
	unsigned int etime;
	unsigned int fsaddr;
	unsigned int lsaddr;
	unsigned int fdaddr;
	unsigned int ldaddr;
	unsigned int sport;
	unsigned int dport;
	char *bpf_query;
	bool no_bpf;
} query_context_t;
