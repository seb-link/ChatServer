#include "common.h"
#include "crypto.h"

int main(void) {
    printf("TEST : CRYPTO : generate_challenge()\n");
    challenge* a;
    printf("INFO : Generating challenge...\n");
    a = generate_challenge();
    printf("INFO : Challenge generated !\n");
    printf("Hash data : \"%s\"\n",a->hash);
    printf("Rand data : \"%s\"\n",a->rand);
    return 0;
}