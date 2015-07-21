#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <othm_hashmap.h>

int str_request_cmp(void *storage, void *data)
{
	return !(strcmp((char *)storage, (char *)data));
}

int main(void)
{
	char *str = "If you are seeing this, ";
	struct othm_request *b = othm_request_new
		(str_request_cmp, str, strlen(str), str);
	char *str2 = "you built othm_hashmap! Yay!\n";
	struct othm_request *c = othm_request_new
		(str_request_cmp, str2, strlen(str2), str2);
	struct othm_hashmap *a = othm_hashmap_new();
	othm_hashmap_add(a, b, str);
	othm_hashmap_add(a, c, str2);
	printf("%s", (char *) othm_hashmap_get(a, b));
	printf("%s", (char *) othm_hashmap_get(a, c));
	othm_hashmap_remove(a, b);
	if(othm_hashmap_get(a, b) == NULL)
		printf("It is also fully functional!\n");
	free(c);
	free(b);
	othm_hashmap_free(a);
}
