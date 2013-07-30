typedef struct query_context {
	struct index_array_context *ictx;
	struct bpf_program *bpf;
	unsigned int stime;
	unsigned int etime;
	unsigned int fsaddr;
	unsigned int lsaddr;
	unsigned int fdaddr;
	unsigned int ldaddr;
	unsigned int fsport;
	unsigned int lsport;
	unsigned int fdport;
	unsigned int ldport;
	char *bpf_query;
	bool no_bpf;
} query_context_t;
