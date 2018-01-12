
typedef struct n{
	char *element;
	struct n *next;
} *node;

typedef struct l{
	nodo begining;
	nodo end;
	int length;
} *list;

list create(void);
//adds ath the end
void add(char *e, list l);
//eliminates the first element
char* eliminate(list l);



