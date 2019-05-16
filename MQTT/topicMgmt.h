extern struct list_type *type;
extern struct list_entry *list;

extern struct list_type *init_list();

extern void getAddr(struct list_entry *le, char *McAddr);

extern int add_topic(struct list_type *list, char *topic, char* mcAddr);

extern struct list_entry *find_topic(struct list_type *list, char *topic);

extern int remove_topic(struct list_type *list, char *topic);