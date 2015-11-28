#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <othm_hashmap.h>

int str_request_cmp(void *storage, void *data)
{
	return !(strcmp((char *)storage, (char *)data));
}

struct othm_hashmap *hasmap_gen(void)
{
	return malloc(sizeof(struct othm_hashmap));
}

void hashmap_free(struct othm_hashmap *map)
{
	free(map);
}

int main(void)
{
	char *str  = "If you are seeing this, ";
	char *str2 = "you built othm_hashmap! ";
	char *str3 = "Yay!\n";
	struct othm_hashmap *a = othm_hashmap_new(hasmap_gen);


	struct othm_request *b = othm_request_new
		(str_request_cmp, str, strlen(str), str);
	struct othm_request *c = othm_request_new
		(str_request_cmp, str2, strlen(str2), str2);
	struct othm_request *d = othm_request_new
		(str_request_cmp, str3, strlen(str3), str3);

	othm_hashmap_add(a, b, str);
	othm_hashmap_add(a, c, str2);
	othm_hashmap_add(a, d, str3);


	printf("%s", (char *) othm_hashmap_get(a, b));
	printf("%s", (char *) othm_hashmap_get(a, c));
	printf("%s", (char *) othm_hashmap_get(a, d));

	/* othm_hashmap_remove(a, b); */
	/* if(othm_hashmap_get(a, b) == NULL) */
	/* 	printf("It is also fully functional!\n"); */
	free(c);
	free(b);
	othm_hashmap_free(a, hashmap_free);
	return 0;
}
