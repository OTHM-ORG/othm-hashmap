#include <stdio.h>
#include <othm_hashmap.h>

int main(void)
{
	char *str = "If you are seeing this, ";
	char *str2 = "you built othm_hashmap! Yay!\n";
	struct othm_hashmap *a = othm_hashmap_new();
	othm_hashmap_add(a, str, str);
	othm_hashmap_add(a, str2, str2);
	printf("%s", (char *) othm_hashmap_get(a, str));
	printf("%s", (char *) othm_hashmap_get(a, str2));
}
