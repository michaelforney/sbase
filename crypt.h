struct crypt_ops {
	void (*init)(void *);
	void (*update)(void *, const void *, unsigned long);
	void (*sum)(void *, uint8_t *);
	void *s;
};

int cryptsum(struct crypt_ops *ops, FILE *fp, const char *f,
	     uint8_t *md);
void mdprint(const uint8_t *md, const char *f, size_t len);
