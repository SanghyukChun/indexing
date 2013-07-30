typedef struct query_context {
	struct bpf_program *bpf;
	int stime;
	int etime;
	int fsaddr;
	int lsaddr;
	int fdaddr;
	int ldaddr;
	int sport;
	int dport;
	char *bpf_query;
} query_context_t;
